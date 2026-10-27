//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.cc
//! \brief Implementation of the PhysicsList class
//---------------------------------------------------------------------------//
#include "PhysicsList.hh"

// Geant4
#include "G4Electron.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpticalPhysics.hh"
#include "G4ParticleDefinition.hh"
#include "G4Positron.hh"
#include "G4ProcessManager.hh"
#include "G4ios.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    PhysicsList::PhysicsList() : FTFP_BERT_HP()
    {
        auto *params = G4OpticalParameters::Instance();
        params->SetProcessActivation("Cerenkov", false);
        params->SetProcessActivation("Scintillation", true);
        params->SetProcessActivation("OpAbsorption", true);
        params->SetProcessActivation("OpRayleigh", false);
        params->SetProcessActivation("OpBoundary", true);
        params->SetBoundaryInvokeSD(true);
        params->SetProcessActivation("OpWLS", false);
        params->SetProcessActivation("OpMieHG", false);
        RegisterPhysics(new G4OpticalPhysics());
    }

    //---------------------------------------------------------------------------//

    void PhysicsList::ConstructProcess()
    {
        FTFP_BERT_HP::ConstructProcess();

        for (G4ParticleDefinition *particle :
             {static_cast<G4ParticleDefinition *>(G4Electron::Definition()),
              static_cast<G4ParticleDefinition *>(G4Positron::Definition())})
        {
            auto *pm = particle->GetProcessManager();

            for (G4int i = pm->GetProcessListLength() - 1; i >= 0; --i)
            {
                auto *proc = (*pm->GetProcessList())[i];

                if (proc->GetProcessName() == "CoulombScat")
                {
                    G4cout << "Removing CoulombScat from "
                           << particle->GetParticleName() << G4endl;

                    pm->RemoveProcess(proc);
                }
            }
        }

        // Optical photons cannot generate scintillation from energy deposition.
        // Remove the incorrectly attached process before tracking begins.
        auto *optical_process_manager = G4OpticalPhoton::Definition()->GetProcessManager();
        CELER_ASSERT(optical_process_manager);

        auto *optical_processes = optical_process_manager->GetProcessList();
        CELER_ASSERT(optical_processes);

        for (G4int process_index = optical_process_manager->GetProcessListLength() - 1;
             process_index >= 0;
             --process_index)
        {
            auto *process = (*optical_processes)[process_index];
            CELER_ASSERT(process);

            if (process->GetProcessName() == "Scintillation")
            {
                optical_process_manager->RemoveProcess(process);
            }
        }
    }

    //---------------------------------------------------------------------------//
} // namespace legend
