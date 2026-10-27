//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.cc
//! \brief Implementation of the RunAction class
//---------------------------------------------------------------------------//

#include "RunAction.hh"

#include "EventOutput.hh"

#include <string>

// Geant4
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "G4ios.hh"

// Celeritas
#include "accel/TrackingManagerIntegration.hh"
#include "corecel/Assert.hh"
#include "corecel/sys/Environment.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Construct the run action and its output-file command.
 */
RunAction::RunAction(SensitiveDetectorType detector_type)
    : detector_type_(detector_type)
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

    celeritas::TrackingManagerIntegration::Instance().BeginOfRunAction(run);

    auto* analysis_manager = G4AnalysisManager::Instance();
    CELER_ASSERT(analysis_manager);

    analysis_manager->SetNtupleMerging(true);
    analysis_manager->SetFileName(file_name_);

    std::string const transport
        = celeritas::getenv("CELER_DISABLE") == "1" ? "geant4" : "celeritas";
    std::string const detector = to_cstring(detector_type_);
    std::string const output_file
        = transport + "-" + detector + "-" + file_name_;

    CELER_VALIDATE(analysis_manager->OpenFile(output_file),
                   << "failed to open analysis output '" << output_file << "'");

    G4cout << "Generating " << output_file << G4endl;
    EventOutput::create_ntuples(*analysis_manager);
    if (G4Threading::IsMasterThread())
    {
        EventOutput::write_run_info(*analysis_manager, detector, transport);
    }

    start_time_ = std::chrono::high_resolution_clock::now();
    run_time_ = 0;
    G4cout << "### Run started ###" << G4endl;
}

//---------------------------------------------------------------------------//
/*!
 * Write output and finalize Celeritas.
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

    auto* analysis_manager = G4AnalysisManager::Instance();
    CELER_ASSERT(analysis_manager);

    if (G4Threading::IsMasterThread())
    {
        G4cout << "Saving events to " << analysis_manager->GetFileName()
               << G4endl;
    }
    analysis_manager->Write();
    analysis_manager->CloseFile();

    celeritas::TrackingManagerIntegration::Instance().EndOfRunAction(run);
}

//---------------------------------------------------------------------------//
} // namespace legend
