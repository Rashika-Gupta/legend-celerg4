//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.hh
//! \brief Definition of the LEGEND optical-photon gun
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <vector>

// Geant4
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4UImessenger.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"

class G4Event;

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Fire fixed-energy isotropic optical photons from a list of source points.
 *
 * Event \em i uses source position <em>i mod N</em>, so a run of
 * <em>k N</em> events visits every position \em k times. Positions are added
 * with \c /legend/gun/addPosition; with none, \c /legend/gun/position is used.
 * Its default depends on the sensitive detector: inside the shroud at the
 * fiber middle for fibers, or just below top SiPM S0102T for SiPMs.
 */
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
  public:
    explicit PrimaryGeneratorAction(SensitiveDetectorType detector_type);

    //! Generate the optical photons for one Geant4 event.
    void GeneratePrimaries(G4Event* event) final;

    //! Append a source point (messenger command).
    void add_position(G4ThreeVector position);

    //! Remove all added source points (messenger command).
    void clear_positions();

  private:
    //! Source point used by an event.
    G4ThreeVector const& position(G4int event_id) const;

    G4int num_photons_{1000};
    G4double energy_{9.69 * eV};  // 128 nm LAr scintillation
    G4ThreeVector default_position_;
    std::vector<G4ThreeVector> positions_;
    std::unique_ptr<G4GenericMessenger> messenger_;
    std::unique_ptr<G4UImessenger> add_position_messenger_;
};

//---------------------------------------------------------------------------//
} // namespace legend
