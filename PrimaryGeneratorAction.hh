//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.hh
//! \brief Definition of the LEGEND primary generator action
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

// Geant4
#include "G4VUserPrimaryGeneratorAction.hh"

// Local
#include "OpticalOffloadMode.hh"

class G4Event;
class G4GeneralParticleSource;

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Generate optical or charged primaries for the selected offload mode.
 */
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
  public:
    //! Construct for a selected optical-offload mode.
    explicit PrimaryGeneratorAction(OpticalOffloadMode mode);
    ~PrimaryGeneratorAction();

    //! Generate primary vertices for one Geant4 event.
    void GeneratePrimaries(G4Event* event) final;

  private:
    //! Generate one event containing directly transported optical photons.
    void generate_optical_photons(G4Event& event);

    //! Generate one electron for optical production or electron offloading.
    void generate_electron(G4Event& event);

    std::unique_ptr<G4GeneralParticleSource> particle_source_;
    OpticalOffloadMode mode_;
};

//---------------------------------------------------------------------------//
} // namespace legend
