//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.hh
//! \brief Definition of the DetectorConstruction class
//---------------------------------------------------------------------------//
#pragma once

// Geant4
#include "G4VUserDetectorConstruction.hh"

class G4GDMLParser;

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Return a detector geometry loaded by a GDML parser.
 */
class DetectorConstruction final : public G4VUserDetectorConstruction
{
  public:
    //! Construct using a parser that owns the loaded GDML geometry.
    explicit DetectorConstruction(G4GDMLParser const* parser);

    //! Return the world volume from the imported GDML geometry.
    G4VPhysicalVolume* Construct() final;

    //! Attach optical detectors to configured SiPMs and the outer fiber coating.
    void ConstructSDandField() final;

  private:
    G4GDMLParser const* parser_;
};

//---------------------------------------------------------------------------//
} // namespace legend
