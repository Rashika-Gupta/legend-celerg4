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
#include "CeleritasSetup.hh"
#include "DetectorConstruction.hh"
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

    auto parser = std::make_unique<G4GDMLParser>();
    parser->Read(geometry_file, false);

    std::unique_ptr<G4UIExecutive> ui;
    if (interactive)
    {
        ui = std::make_unique<G4UIExecutive>(argc, argv);
    }

    std::unique_ptr<G4RunManager> run_manager{
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial)};
    auto *physics = new legend::PhysicsList;

    // Geant4 optical-photon tracks are handed to Celeritas (CELER_DISABLE=1
    // keeps them in Geant4)
    auto &integration = celeritas::TrackingManagerIntegration::Instance();
    physics->RegisterPhysics(
        new celeritas::TrackingManagerConstructor(&integration));
    integration.SetOptions(legend::CeleritasSetup({})());

    run_manager->SetUserInitialization(physics);
    run_manager->SetUserInitialization(
        new legend::DetectorConstruction(parser.get(), detector_type));
    run_manager->SetUserInitialization(
        new legend::ActionInitialization(detector_type));
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
