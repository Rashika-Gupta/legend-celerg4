//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.cc
//! \brief Implementation of the LEGEND primary generator action
//---------------------------------------------------------------------------//

#include "PrimaryGeneratorAction.hh"

#include <cmath>
#include <iostream>
#include <memory>

// Geant4
#include "G4Electron.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4OpticalPhoton.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SingleParticleSource.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4ios.hh"
#include "Randomize.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
    namespace
    {
        //---------------------------------------------------------------------------//
        //! Set a random polarization perpendicular to a primary direction.
        void set_random_polarization(G4PrimaryParticle &primary)
        {
            G4ThreeVector const direction = primary.GetMomentumDirection().unit();
            G4ThreeVector const perpendicular = direction.orthogonal().unit();
            G4ThreeVector const parallel = perpendicular.cross(direction).unit();
            G4double const angle = CLHEP::twopi * G4UniformRand();
            primary.SetPolarization(std::cos(angle) * perpendicular + std::sin(angle) * parallel);
        }
    } // namespace

    //---------------------------------------------------------------------------//

    PrimaryGeneratorAction::PrimaryGeneratorAction(OpticalOffloadMode mode)
        : particle_source_(std::make_unique<G4GeneralParticleSource>()), mode_(mode)
    {
    }

    //---------------------------------------------------------------------------//

    PrimaryGeneratorAction::~PrimaryGeneratorAction() = default;

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
    {
        CELER_EXPECT(event);
        if (mode_ == OpticalOffloadMode::optical_gun)
        {
            this->generate_optical_photons(*event);
            return;
        }

        this->generate_electron(*event);
    }

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::generate_optical_photons(G4Event &event)
    {
        auto *source = particle_source_->GetCurrentSource();
        CELER_ASSERT(source);

        constexpr G4int num_photons = 1;
        constexpr G4double min_optical_energy = 1.8 * eV;
        constexpr G4double max_optical_energy = 14 * eV;
        particle_source_->SetParticleDefinition(
            G4OpticalPhoton::OpticalPhotonDefinition());
        source->GetAngDist()->SetAngDistType("iso");
        source->GetAngDist()->SetMinTheta(0);
        source->GetAngDist()->SetMaxTheta(CLHEP::pi);
        source->GetAngDist()->SetMinPhi(0);
        source->GetAngDist()->SetMaxPhi(CLHEP::twopi);
        particle_source_->SetNumberOfParticles(1);

        for (G4int photon_id = 0; photon_id < num_photons; ++photon_id)
        {
            G4double const energy = min_optical_energy + G4UniformRand() * (max_optical_energy - min_optical_energy);
            G4cout << "Generating optical photon " << photon_id << " of "
                   << num_photons << " with energy " << energy << G4endl;

            source->GetEneDist()->SetEnergyDisType("Mono");
            source->GetEneDist()->SetMonoEnergy(energy);
            particle_source_->GeneratePrimaryVertex(&event);

            auto *vertex = event.GetPrimaryVertex(event.GetNumberOfPrimaryVertex() - 1);
            CELER_ASSERT(vertex);
            auto *primary = vertex->GetPrimary();
            CELER_ASSERT(primary);
            set_random_polarization(*primary);
        }
    }

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::generate_electron(G4Event &event)
    {
        auto *source = particle_source_->GetCurrentSource();
        CELER_ASSERT(source);

        constexpr G4double electron_energy = 50 * MeV;
        particle_source_->SetParticleDefinition(G4Electron::Definition());
        source->GetEneDist()->SetEnergyDisType("Mono");
        source->GetEneDist()->SetMonoEnergy(electron_energy);
        source->GetAngDist()->SetAngDistType("planar");
        source->GetAngDist()->SetParticleMomentumDirection(
            G4ThreeVector(0, 0, -1));
        particle_source_->GeneratePrimaryVertex(&event);
    }

    //---------------------------------------------------------------------------//
} // namespace legend
