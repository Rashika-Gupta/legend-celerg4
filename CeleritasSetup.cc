//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CeleritasSetup.cc
//! \brief Implementation of LEGEND Celeritas optical setup
//---------------------------------------------------------------------------//

#include "CeleritasSetup.hh"

#include <utility>
#include <vector>

// Geant4
#include "G4Electron.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4VPhysicalVolume.hh"

// Celeritas
#include "accel/AlongStepFactory.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/inp/OpticalPhysics.hh"
#include "celeritas/optical/DetectorData.hh"
#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"
#include "geocel/GeantGeoParams.hh"
#include "geocel/g4/Convert.hh"

// Local
#include "EventOutput.hh"

namespace legend
{
    namespace
    {
        //---------------------------------------------------------------------------//
        /*!
         * Particles and optical generator associated with an offload mode.
         */
        struct OffloadConfiguration
        {
            celeritas::SetupOptions::VecG4PD particles;
            celeritas::inp::OpticalGenerator generator;
        };

        //---------------------------------------------------------------------------//
        //! Select particles and the optical generator for an offload mode.
        OffloadConfiguration make_offload_configuration(OpticalOffloadMode mode)
        {
            switch (mode)
            {
            case OpticalOffloadMode::optical_gun:
            case OpticalOffloadMode::optical_track:
                return {{G4OpticalPhoton::Definition()},
                        celeritas::inp::OpticalDirectGenerator{}};

            case OpticalOffloadMode::optical_distribution:
                return {{}, celeritas::inp::OpticalOffloadGenerator{}};

            case OpticalOffloadMode::electron_photon:
                return {{G4Electron::Definition()},
                        celeritas::inp::OpticalEmGenerator{}};
            }
            CELER_ASSERT_UNREACHABLE();
        }

        //---------------------------------------------------------------------------//
        //! Buffer Celeritas optical hits for event-level analysis output.
        void record_optical_hits(
            celeritas::Span<celeritas::optical::DetectorHit const> hits)
        {
            auto const *event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
            CELER_EXPECT(event);

            auto geant_geo = celeritas::global_geant_geo().lock();
            CELER_VALIDATE(geant_geo, << "Geant4 geometry mapping is unavailable");

            std::vector<OpticalDetectorHit> records;
            records.reserve(hits.size());
            for (auto const &hit : hits)
            {
                CELER_VALIDATE(hit.detector, << "optical hit has no detector ID");

                CELER_VALIDATE(hit.volume_instance,
                               << "optical hit has no volume instance ID");

                auto const *physical = geant_geo->id_to_geant(hit.volume_instance);
                CELER_VALIDATE(physical,
                               << "no Geant4 volume corresponds to Celeritas volume "
                               << hit.volume_instance.unchecked_get());

                double const energy_mev = celeritas::value_as<celeritas::units::MevEnergy>(
                    hit.energy);
                records.push_back({event->GetEventID(),
                                   static_cast<G4int>(
                                       hit.detector.unchecked_get()),
                                   static_cast<G4int>(
                                       hit.primary.unchecked_get()),
                                   static_cast<G4int>(
                                       hit.volume_instance.unchecked_get()),
                                   physical->GetName(),
                                   {hit.position[0],
                                    hit.position[1],
                                    hit.position[2]},
                                   hit.time,
                                   energy_mev});
            }

            CELER_LOG(debug) << "Buffering " << records.size()
                             << " LEGEND optical detector hits";
            event_output().add_hits(records);
        }
    } // namespace

    //---------------------------------------------------------------------------//

    CeleritasSetup::CeleritasSetup(Input input) : input_(std::move(input))
    {
        CELER_VALIDATE(input_.num_track_slots > 0,
                       << "optical track capacity must be positive");
        CELER_VALIDATE(!input_.diagnostics_file.empty(),
                       << "Celeritas diagnostics filename is empty");
    }

    //---------------------------------------------------------------------------//

    celeritas::SetupOptions CeleritasSetup::operator()() const
    {
        celeritas::SetupOptions result;
        result.sd.enabled = false;
        result.make_along_step = celeritas::UniformAlongStepFactory();
        result.output_file = input_.diagnostics_file;

        celeritas::OpticalSetupOptions optical;
        optical.capacity.tracks = input_.num_track_slots;
        optical.capacity.primaries = 8 * input_.num_track_slots;
        optical.capacity.generators = 2 * input_.num_track_slots;
        optical.detectors.callback = record_optical_hits;

        auto config = make_offload_configuration(input_.mode);
        result.offload_particles = std::move(config.particles);
        optical.generator = std::move(config.generator);
        result.optical = std::move(optical);
        return result;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
