//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.cc
//! \brief Implementation of the PhysicsList class
//---------------------------------------------------------------------------//
#include "PhysicsList.hh"

#include <memory>

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
#include "accel/SharedParams.hh"
#include "accel/gen/CherenkovOffload.hh"
#include "accel/gen/ScintillationOffload.hh"
#include "celeritas/g4/SupportedOpticalPhysics.hh"
#include "corecel/Assert.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    PhysicsList::PhysicsList(OpticalOffloadMode mode) : FTFP_BERT_HP()
    {
        if (mode == OpticalOffloadMode::optical_distribution && celeritas::SharedParams::GetMode() == celeritas::OffloadMode::enabled)
        {
            RegisterPhysics(
                new celeritas::SupportedOpticalPhysics(physics_options()));
        }
        else
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

    celeritas::GeantOpticalPhysicsOptions PhysicsList::optical_options() const
    {
        celeritas::GeantOpticalPhysicsOptions optical;

        optical.cherenkov.emplace();
        optical.cherenkov->custom_cherenkov = []
        { return std::make_unique<celeritas::CherenkovOffload>(); };
        // Generated photons are transported directly by Celeritas.
        optical.cherenkov->stack_photons = false;

        optical.scintillation.emplace();
        optical.scintillation->custom_scintillation = []
        { return std::make_unique<celeritas::ScintillationOffload>(); };
        optical.scintillation->stack_photons = false;

        optical.boundary->invoke_sd = true;
        optical.absorption = true;
        optical.rayleigh_scattering = false;

        return optical;
    }

    //---------------------------------------------------------------------------//

    celeritas::GeantPhysicsOptions PhysicsList::physics_options() const
    {
        celeritas::GeantPhysicsOptions opts;
        opts.optical = this->optical_options();
        return opts;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
