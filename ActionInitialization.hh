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
//! \file ActionInitialization.hh
//! \brief Definition of LEGEND user-action initialization
//---------------------------------------------------------------------------//
#pragma once

// Geant4
#include "G4VUserActionInitialization.hh"

// Local
#include "OpticalOffloadMode.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Install Geant4 user actions for a Celeritas optical-offload mode.
 */
class ActionInitialization final : public G4VUserActionInitialization
{
  public:
    //! Construct for the selected Celeritas optical-offload mode.
    ActionInitialization(OpticalOffloadMode mode,
                         IntegrationType integration);

    //! Install actions used by the master thread.
    void BuildForMaster() const final;

    //! Install actions used by a transport thread.
    void Build() const final;

  private:
    OpticalOffloadMode mode_;
    IntegrationType integration_;
};

//---------------------------------------------------------------------------//
} // namespace legend
