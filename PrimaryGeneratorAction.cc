//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.cc
//! \brief Implementation of the LEGEND optical-photon gun
//---------------------------------------------------------------------------//

#include "PrimaryGeneratorAction.hh"

#include <cmath>

// Geant4
#include "G4Event.hh"
#include "G4OpticalPhoton.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4ios.hh"
#include "Randomize.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
    namespace
    {
        //---------------------------------------------------------------------------//
        //! Sample an isotropic unit direction.
        G4ThreeVector sample_isotropic()
        {
            G4double const cos_theta = 2 * G4UniformRand() - 1;
            G4double const sin_theta = std::sqrt(1 - cos_theta * cos_theta);
            G4double const phi = CLHEP::twopi * G4UniformRand();
            return {sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta};
        }

        //---------------------------------------------------------------------------//
        //! Sample a random polarization perpendicular to a direction.
        G4ThreeVector sample_polarization(G4ThreeVector const &direction)
        {
            G4ThreeVector const perpendicular = direction.orthogonal().unit();
            G4ThreeVector const parallel = perpendicular.cross(direction).unit();
            G4double const angle = CLHEP::twopi * G4UniformRand();
            return std::cos(angle) * perpendicular + std::sin(angle) * parallel;
        }

        //---------------------------------------------------------------------------//
        //! Default source point that illuminates the selected detector.
        G4ThreeVector default_position(SensitiveDetectorType detector_type)
        {
            switch (detector_type)
            {
            case SensitiveDetectorType::sipm:
                // LAr gap between the fiber tops (z = 349.09 mm) and the
                // face of top SiPM S0102T (z = 349.14 mm), at its center
                return {52.75 * mm, 0, 349.12 * mm};
            case SensitiveDetectorType::fiber:
                // Inside the fiber shroud at the fiber middle; points on the
                // axis lie outside the ORANGE geometry
                return {30 * mm, 0, -325 * mm};
            }
            CELER_ASSERT_UNREACHABLE();
        }

        //---------------------------------------------------------------------------//
        /*!
         * Messenger for \c /legend/gun/addPosition.
         *
         * G4GenericMessenger cannot declare a method taking a 3-vector with
         * a unit, so this command uses its own messenger.
         */
        class AddPositionMessenger final : public G4UImessenger
        {
          public:
            explicit AddPositionMessenger(PrimaryGeneratorAction *gun)
                : gun_(gun),
                  command_(std::make_unique<G4UIcmdWith3VectorAndUnit>(
                      "/legend/gun/addPosition", this))
            {
                command_->SetGuidance(
                    "Append a source point; event i uses point i mod N");
                command_->SetParameterName("x", "y", "z", false);
                command_->SetDefaultUnit("mm");
            }

            void SetNewValue(G4UIcommand *command, G4String value) final
            {
                CELER_EXPECT(command == command_.get());
                gun_->add_position(command_->GetNew3VectorValue(value));
            }

          private:
            PrimaryGeneratorAction *gun_;
            std::unique_ptr<G4UIcmdWith3VectorAndUnit> command_;
        };
    } // namespace

    //---------------------------------------------------------------------------//

    PrimaryGeneratorAction::PrimaryGeneratorAction(
        SensitiveDetectorType detector_type)
        : default_position_(default_position(detector_type)), messenger_(std::make_unique<G4GenericMessenger>(
              this, "/legend/gun/", "LEGEND optical-photon gun"))
    {
        messenger_->DeclareProperty(
            "photons", num_photons_, "Optical photons generated per event");
        messenger_->DeclarePropertyWithUnit(
            "energy", "eV", energy_, "Fixed optical photon energy");
        messenger_->DeclarePropertyWithUnit(
            "position", "mm", default_position_,
            "Source point used when no positions were added");
        messenger_->DeclareMethod("clearPositions",
                                  &PrimaryGeneratorAction::clear_positions,
                                  "Remove all added source points");
        add_position_messenger_ = std::make_unique<AddPositionMessenger>(this);
    }

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::add_position(G4ThreeVector position)
    {
        positions_.push_back(position);
    }

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::clear_positions()
    {
        positions_.clear();
    }

    //---------------------------------------------------------------------------//

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
    {
        CELER_EXPECT(event);
        CELER_VALIDATE(num_photons_ > 0,
                       << "optical gun photon count must be positive");
        CELER_VALIDATE(energy_ > 0,
                       << "optical gun energy must be positive");

        auto const &pos = this->position(event->GetEventID());
        auto *vertex = new G4PrimaryVertex(pos, 0);
        for (G4int i = 0; i < num_photons_; ++i)
        {
            G4ThreeVector const direction = sample_isotropic();
            auto *primary = new G4PrimaryParticle(G4OpticalPhoton::Definition());
            primary->SetKineticEnergy(energy_);
            primary->SetMomentumDirection(direction);
            primary->SetPolarization(sample_polarization(direction));
            vertex->SetPrimary(primary);
        }
        event->AddPrimaryVertex(vertex);

        G4cout << "Event " << event->GetEventID() << ": " << num_photons_
               << " photons of " << energy_ / eV << " eV at " << pos / mm
               << " mm" << G4endl;
    }

    //---------------------------------------------------------------------------//

    G4ThreeVector const &PrimaryGeneratorAction::position(G4int event_id) const
    {
        if (positions_.empty())
        {
            return default_position_;
        }
        return positions_[static_cast<std::size_t>(event_id) % positions_.size()];
    }

    //---------------------------------------------------------------------------//
} // namespace legend
