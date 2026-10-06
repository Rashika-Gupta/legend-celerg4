//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.cc
//! \brief Implementation of the RunAction class
//---------------------------------------------------------------------------//

#include "RunAction.hh"

#include "EventOutput.hh"

#include <cstddef>
#include <string>

// Geant4
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "G4ios.hh"

// Celeritas
#include "accel/LocalTransporter.hh"
#include "accel/TrackingManagerIntegration.hh"
#include "accel/UserActionIntegration.hh"
#include "accel/detail/IntegrationSingleton.hh"
#include "celeritas/global/CoreState.hh"
#include "celeritas/optical/CoreState.hh"
#include "celeritas/optical/OpticalCollector.hh"
#include "corecel/Assert.hh"
#include "corecel/sys/Environment.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Construct the run action for an offload mode.
 */
RunAction::RunAction(OpticalOffloadMode mode, IntegrationType integration)
    : mode_(mode)
    , integration_(integration)
    , messenger_(std::make_unique<G4GenericMessenger>(
          this, "/legend/output/", "LEGEND analysis output"))
{
    messenger_->DeclareProperty("file", file_name_, "File Name to Save");
}

//---------------------------------------------------------------------------//
/*!
 * Initialize the selected Celeritas integration and analysis output.
 */
void RunAction::BeginOfRunAction(G4Run const* run)
{
    CELER_EXPECT(run);

    this->begin_integration(run);

    auto* analysis_manager = G4AnalysisManager::Instance();
    CELER_ASSERT(analysis_manager);

    analysis_manager->SetNtupleMerging(true);
    analysis_manager->SetFileName(file_name_);

    std::string const transport
        = celeritas::getenv("CELER_DISABLE") == "1" ? "geant4" : "celeritas";
    std::string const output_file
        = transport + "-" + to_cstring(mode_) + "-" + file_name_;

    CELER_VALIDATE(analysis_manager->OpenFile(output_file),
                   << "failed to open analysis output '" << output_file << "'");

    G4cout << "Generating " << output_file << G4endl;
    EventOutput::create_ntuples(*analysis_manager);

    start_time_ = std::chrono::high_resolution_clock::now();
    run_time_ = 0;
    G4cout << "### Run started ###" << G4endl;
}

//---------------------------------------------------------------------------//
/*!
 * Write output, report optical generation, and finalize Celeritas.
 */
void RunAction::EndOfRunAction(G4Run const* run)
{
    CELER_EXPECT(run);

    auto const duration = std::chrono::high_resolution_clock::now() - start_time_;
    run_time_ = std::chrono::duration_cast<std::chrono::duration<double>>(duration)
                    .count();

    if (!G4Threading::IsMultithreadedApplication() || G4Threading::IsMasterThread())
    {
        G4cout << "Run time: " << run_time_ << " seconds" << G4endl;
    }

    this->report_optical_generation();

    auto* analysis_manager = G4AnalysisManager::Instance();
    CELER_ASSERT(analysis_manager);

    if (G4Threading::IsMasterThread())
    {
        G4cout << "Saving events to " << analysis_manager->GetFileName()
               << G4endl;
    }
    analysis_manager->Write();
    analysis_manager->CloseFile();

    this->end_integration(run);
}

//---------------------------------------------------------------------------//

void RunAction::begin_integration(G4Run const* run)
{
    CELER_EXPECT(run);
    if (uses_user_action_integration(integration_))
    {
        celeritas::UserActionIntegration::Instance().BeginOfRunAction(run);
    }
    else
    {
        celeritas::TrackingManagerIntegration::Instance().BeginOfRunAction(run);
    }
}

//---------------------------------------------------------------------------//

void RunAction::end_integration(G4Run const* run)
{
    CELER_EXPECT(run);
    if (uses_user_action_integration(integration_))
    {
        celeritas::UserActionIntegration::Instance().EndOfRunAction(run);
    }
    else
    {
        celeritas::TrackingManagerIntegration::Instance().EndOfRunAction(run);
    }
}

//---------------------------------------------------------------------------//

void RunAction::report_optical_generation() const
{
    using CeleritasMode = celeritas::OffloadMode;
    auto& tracking = celeritas::TrackingManagerIntegration::Instance();
    if (mode_ != OpticalOffloadMode::electron_photon
        || tracking.GetMode() != CeleritasMode::enabled
        || (!G4Threading::IsWorkerThread()
            && G4Threading::IsMultithreadedApplication()))
    {
        return;
    }

    auto& integration = celeritas::detail::IntegrationSingleton::instance();
    auto& local = dynamic_cast<celeritas::LocalTransporter&>(
        integration.local_offload());
    auto const& optical_collector
        = integration.shared_params().problem_loaded().optical_collector;
    if (!optical_collector)
    {
        return;
    }

    auto const counters
        = optical_collector->exchange_counters(local.GetState().aux());
    std::size_t total_photons_generated = 0;
    for (auto const& generator : counters.generators)
    {
        total_photons_generated += generator.num_generated;
    }
    G4cout << "Celeritas generated " << total_photons_generated
           << " optical photons" << G4endl;
}

//---------------------------------------------------------------------------//
} // namespace legend
