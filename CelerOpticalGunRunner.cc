//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CelerOpticalGunRunner.cc
//! \brief Implementation of the standalone Celeritas optical-photon gun
//---------------------------------------------------------------------------//

#include "CelerOpticalGunRunner.hh"

#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "G4Threading.hh"
#include "G4ios.hh"

#include "celeritas/Types.hh"
#include "celeritas/optical/CoreParams.hh"
#include "celeritas/optical/CoreState.hh"
#include "celeritas/optical/Transporter.hh"
#include "celeritas/optical/gen/PrimaryGeneratorAction.hh"
#include "celeritas/phys/GeneratorCounters.hh"
#include "celeritas/phys/GeneratorCountersIO.json.hh"
#include "corecel/Assert.hh"
#include "corecel/data/AuxStateVec.hh"
#include "corecel/io/JsonUtils.json.hh"
#include "corecel/io/OutputInterfaceAdapter.hh"
#include "corecel/io/OutputRegistry.hh"
#include "corecel/sys/Device.hh"
#include "corecel/sys/ScopedProfiling.hh"
#include "geocel/GeantUtils.hh"
#include "corecel/io/Logger.hh"

namespace legend
{
    namespace
    {
        //---------------------------------------------------------------------------//
        struct LocalOpticalGunState
        {
            std::shared_ptr<celeritas::optical::CoreStateBase> state;
            std::shared_ptr<celeritas::optical::Transporter> transporter;
            std::shared_ptr<celeritas::optical::PrimaryGeneratorAction const> generator;
        };

        thread_local LocalOpticalGunState local_state;

        //! Counters summed over all worker threads for JSON output
        struct GunDiagnostics
        {
            celeritas::CounterAccumStats counters;
        };

        void to_json(nlohmann::json &j, GunDiagnostics const &v)
        {
            j = {CELER_JSON_PAIR(v, counters)};
        }

        std::shared_ptr<GunDiagnostics> const &diagnostics()
        {
            static auto const result = std::make_shared<GunDiagnostics>();
            return result;
        }

        std::mutex &diagnostics_mutex()
        {
            static std::mutex result;
            return result;
        }

        void accumulate(celeritas::CounterAccumStats &total,
                        celeritas::CounterAccumStats const &add)
        {
            total.steps += add.steps;
            total.step_iters += add.step_iters;
            total.flushes += add.flushes;
            total.num_cut += add.num_cut;
            total.num_errored += add.num_errored;
            if (total.generators.size() < add.generators.size())
            {
                total.generators.resize(add.generators.size());
            }
            for (std::size_t i = 0; i < add.generators.size(); ++i)
            {
                total.generators[i].buffer_size += add.generators[i].buffer_size;
                total.generators[i].num_pending += add.generators[i].num_pending;
                total.generators[i].num_generated += add.generators[i].num_generated;
            }
        }

        //! Print optical counters
        struct StreamableCounters
        {
            celeritas::CounterAccumStats const &counters;
        };
        std::ostream &operator<<(std::ostream &os, StreamableCounters const &sc)
        {
            std::size_t num_generated{};
            std::size_t buffer_size{};
            for (auto const &gen : sc.counters.generators)
            {
                num_generated += gen.num_generated;
                buffer_size += gen.buffer_size;
            }
            os << num_generated << " optical photons from " << buffer_size
               << " generator distributions with a total of "
               << sc.counters.steps << " steps over "
               << sc.counters.step_iters << " step iterations ("
               << sc.counters.num_cut << " cut, "
               << sc.counters.num_errored << " errored)";
            if (sc.counters.step_iters > 0)
            {
                os << ", avg "
                   << static_cast<double>(sc.counters.steps) / static_cast<double>(sc.counters.step_iters)
                   << " active tracks per iteration";
            }
            return os;
        }
    } // namespace

    //---------------------------------------------------------------------------//

    CelerOpticalGunRunner &CelerOpticalGunRunner::Instance()
    {
        static CelerOpticalGunRunner result;
        return result;
    }

    //---------------------------------------------------------------------------//

    void CelerOpticalGunRunner::SetOptionsFactory(OptionsFactory factory)
    {
        CELER_VALIDATE(!options_factory_,
                       << "Celeritas optical-gun options factory was already set");
        CELER_VALIDATE(factory, << "invalid Celeritas optical-gun options factory");
        options_factory_ = std::move(factory);
    }

    //---------------------------------------------------------------------------//
    /*!
     * Initialize the standalone optical problem for a run.
     *
     * Only active when Celeritas offload is enabled (\c CELER_DISABLE=1 leaves
     * \c shared_ uninitialized, so this and \c GenerateAndTransport are no-ops).
     */
    void CelerOpticalGunRunner::BeginOfRunAction()
    {
        CELER_LOG(debug) << "Beginning of run action for Celeritas optical gun";
        CELER_VALIDATE(options_factory_,
                       << "Celeritas optical-gun options factory has not been set");

        if (celeritas::SharedParams::GetMode() != celeritas::OffloadMode::enabled)
        {
            return;
        }
        celeritas::ScopedProfiling profile_this{"setup"};

        if (G4Threading::IsMasterThread())
        {
            // Evaluate after macro commands have been processed so every run
            // uses the latest optical-gun configuration.
            options_ = options_factory_();
            shared_.Initialize(options_);

            *diagnostics() = {};
            shared_.output_reg()->insert(
                std::make_shared<celeritas::OutputInterfaceAdapter<GunDiagnostics>>(
                    celeritas::OutputInterface::Category::result,
                    "optical_gun",
                    diagnostics()));
        }
        else
        {
            shared_.InitializeWorker(options_);
        }

        // An MT master owns shared parameters but does not process events.
        if (G4Threading::IsMultithreadedApplication() && G4Threading::IsMasterThread())
        {
            return;
        }

        auto const &problem = shared_.optical_problem_loaded();
        local_state.transporter = problem.transporter;
        local_state.generator = std::dynamic_pointer_cast<
            celeritas::optical::PrimaryGeneratorAction const>(problem.generator);

        CELER_VALIDATE(local_state.generator,
                       << "expected a Celeritas optical primary generator");
        CELER_ASSERT(local_state.transporter);
        CELER_ASSERT(local_state.transporter->params());

        auto const &params = *local_state.transporter->params();
        celeritas::validate_geant_threading(params.sizes().streams);

        auto const stream_id = celeritas::id_cast<celeritas::StreamId>(
            celeritas::get_geant_thread_id());
        auto const memspace = celeritas::device() ? celeritas::MemSpace::device : celeritas::MemSpace::host;

        if (memspace == celeritas::MemSpace::device)
        {
            local_state.state = std::make_shared<celeritas::optical::CoreState<celeritas::MemSpace::device>>(
                params, stream_id, params.sizes().tracks);
        }
        else
        {
            local_state.state = std::make_shared<celeritas::optical::CoreState<celeritas::MemSpace::host>>(
                params, stream_id, params.sizes().tracks);
        }

        if (params.aux_reg())
        {
            local_state.state->aux() = std::make_shared<celeritas::AuxStateVec>(
                *params.aux_reg(), memspace, stream_id, params.sizes().tracks);
        }
    }

    //---------------------------------------------------------------------------//
    /*!
     * Sample and transport one event's worth of optical photons.
     */
    void CelerOpticalGunRunner::GenerateAndTransport(int event_id)
    {
        CELER_EXPECT(event_id >= 0);
        CELER_VALIDATE(local_state.state && local_state.transporter && local_state.generator,
                       << "Celeritas optical-gun state is not initialized");

        celeritas::ScopedProfiling profile_this{"run"};
        local_state.state->reseed(local_state.transporter->params()->rng(),
                                  celeritas::id_cast<celeritas::UniqueEventId>(event_id));

        // Celeritas samples the configured position, energy, and direction, then
        // transports every generated photon to completion.
        local_state.generator->insert(*local_state.state);
        (*local_state.transporter)(*local_state.state);

        // Sampling runs in the generate action inside the transporter, not in insert()
        auto const times = local_state.transporter->exchange_action_times(
            *local_state.state->aux());
        for (auto const &[label, seconds] : times)
        {
            if (label.find("generate") != std::string::npos)
            {
                G4cout << "Celeritas event " << event_id << " " << label
                       << " time " << seconds << " s" << G4endl;
            }
        }
    }

    //---------------------------------------------------------------------------//
    /*!
     * Release the standalone optical problem at the end of a run.
     */
    void CelerOpticalGunRunner::EndOfRunAction()
    {
        if (celeritas::SharedParams::GetMode() != celeritas::OffloadMode::enabled)
        {
            return;
        }

        // Release stream-local state before the master destroys shared data.
        if (!G4Threading::IsMultithreadedApplication() || G4Threading::IsWorkerThread())
        {
            if (local_state.state && local_state.generator)
            {
                auto counters = local_state.state->accum();
                counters.generators.push_back(
                    local_state.generator->counters(*local_state.state->aux()).accum);
                CELER_LOG_LOCAL(info) << "Transported " << StreamableCounters{counters};

                std::lock_guard scoped_lock{diagnostics_mutex()};
                accumulate(diagnostics()->counters, counters);
            }

            local_state = {};
        }

        if (G4Threading::IsMasterThread())
        {
            shared_.Finalize();
        }
    }

    //---------------------------------------------------------------------------//
} // namespace legend
