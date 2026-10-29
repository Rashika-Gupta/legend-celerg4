//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file legend_celer.cc
//! \brief Run the LEGEND Geant4-Celeritas optical integration
//---------------------------------------------------------------------------//

#include <cstring>
#include <memory>
#include <string>

// Geant4
#include "G4GDMLParser.hh"
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4String.hh"
#include "G4Threading.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#if LEGEND_USE_GEANT4_VIS
#include "G4VisExecutive.hh"
#endif
#include "G4ios.hh"

// Celeritas
#include "accel/TrackingManagerConstructor.hh"
#include "accel/TrackingManagerIntegration.hh"
#include "corecel/sys/Environment.hh"

// Local
#include "ActionInitialization.hh"
#include "CelerOpticalGunRunner.hh"
#include "CeleritasSetup.hh"
#include "DetectorConstruction.hh"
#include "OpticalOffloadMode.hh"
#include "PhysicsList.hh"

namespace
{
    //---------------------------------------------------------------------------//
    //! Print command-line usage.
    void print_usage(char const *executable)
    {
        G4cout << "Usage:" << G4endl
               << "  " << executable << " <geometry.gdml> <commands.mac>"
               << G4endl
               << "  " << executable
               << " i <geometry.gdml> <commands.mac>" << G4endl
               << "Environment:" << G4endl
               << "  LEGEND_SENSDET=sipm|fiber  sensitive detector (default "
                  "sipm)"
               << G4endl
               << "  LEGEND_CELER_OFFLOAD_MODE=optical-track|optical-gun "
                  "(default optical-track)"
               << G4endl
               << "  LEGEND_NUM_THREADS=<n>    worker threads (default: "
                  "hardware concurrency)"
               << G4endl
               << "  LEGEND_NUM_TRACK_SLOTS=<n> Celeritas optical track "
                  "capacity (default 65536)"
               << G4endl
               << "  CELER_DISABLE=1            transport photons in Geant4"
               << G4endl;
    }
} // namespace

//---------------------------------------------------------------------------//

int main(int argc, char **argv)
{
    bool const interactive = argc > 1 && std::strcmp(argv[1], "i") == 0;
    int const required_arguments = interactive ? 4 : 3;
    if (argc < required_arguments)
    {
        print_usage(argv[0]);
        return 1;
    }

    char const *geometry_file = argv[interactive ? 2 : 1];
    char const *macro_file = argv[interactive ? 3 : 2];

    // SDs are attached during initialization, before the macro runs, so the
    // detector is chosen from the environment
    std::string detector_name = celeritas::getenv("LEGEND_SENSDET");
    if (detector_name.empty())
    {
        detector_name = "sipm";
    }
    auto const detector_type = legend::parse_sensitive_detector(detector_name);

    auto const offload_mode = legend::parse_offload_mode(celeritas::getenv("LEGEND_CELER_OFFLOAD_MODE"));

    legend::CeleritasSetup::Input setup_input;
    std::string const slots_env = celeritas::getenv("LEGEND_NUM_TRACK_SLOTS");
    if (!slots_env.empty())
    {
        setup_input.num_track_slots =
            static_cast<celeritas::size_type>(std::stoul(slots_env));
    }

    auto parser = std::make_unique<G4GDMLParser>();
    parser->Read(geometry_file, false);

    std::unique_ptr<G4UIExecutive> ui;
    if (interactive)
    {
        ui = std::make_unique<G4UIExecutive>(argc, argv);
    }

    std::unique_ptr<G4RunManager> run_manager{
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT)};
    // Default to hardware concurrency; override with LEGEND_NUM_THREADS for tuning
    std::string const num_threads_env = celeritas::getenv("LEGEND_NUM_THREADS");
    int const num_threads = num_threads_env.empty()
                                ? static_cast<int>(G4Threading::G4GetNumberOfCores())
                                : std::stoi(num_threads_env);
    run_manager->SetNumberOfThreads(num_threads);

    auto *physics = new legend::PhysicsList;

    if (offload_mode == legend::OpticalOffloadMode::optical_gun)
    {
        // Celeritas generates and transports the optical photons itself
        legend::CelerOpticalGunRunner::Instance().SetOptionsFactory(
            [setup_input]
            { return legend::make_optical_gun_options(setup_input); });
    }
    else
    {
        // Geant4 optical-photon tracks are handed to Celeritas (CELER_DISABLE=1
        // keeps them in Geant4)
        auto &integration = celeritas::TrackingManagerIntegration::Instance();
        physics->RegisterPhysics(
            new celeritas::TrackingManagerConstructor(&integration));
        integration.SetOptions(legend::CeleritasSetup({})(setup_input));
    }

    run_manager->SetUserInitialization(physics);
    run_manager->SetUserInitialization(
        new legend::DetectorConstruction(parser.get(), detector_type));
    run_manager->SetUserInitialization(
        new legend::ActionInitialization(detector_type, offload_mode));
    run_manager->Initialize();

#if LEGEND_USE_GEANT4_VIS
    auto visualization = std::make_unique<G4VisExecutive>();
    visualization->Initialize();
#endif

    auto *ui_manager = G4UImanager::GetUIpointer();
    G4String const execute = "/control/execute ";
    ui_manager->ApplyCommand(execute + macro_file);

    if (interactive)
    {
#if LEGEND_USE_GEANT4_VIS
        ui_manager->ApplyCommand(execute + "vis.mac");
#endif
        ui->SessionStart();
    }
    return 0;
}
