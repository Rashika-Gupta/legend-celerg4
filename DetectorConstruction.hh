//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.hh
//! \brief Definition of the DetectorConstruction class
//---------------------------------------------------------------------------//
#pragma once

#include <string_view>

// Geant4
#include "G4VUserDetectorConstruction.hh"

class G4GDMLParser;

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Volumes that detect (and absorb) optical photons.
 */
enum class SensitiveDetectorType
{
    sipm,  //!< SiPM channels listed in the GDML RMG optical metadata
    fiber  //!< Outer TPB coating of the fiber shroud
};

//---------------------------------------------------------------------------//
/*!
 * Parse a sensitive-detector type from user input.
 *
 * \param value Either \c "sipm" or \c "fiber"
 */
SensitiveDetectorType parse_sensitive_detector(std::string_view value);

//! Name of a sensitive-detector type (\c "sipm" or \c "fiber").
char const* to_cstring(SensitiveDetectorType value);

//---------------------------------------------------------------------------//
/*!
 * Return a detector geometry loaded by a GDML parser.
 *
 * Only the selected detector type is made sensitive. Celeritas absorbs
 * photons in every sensitive volume, so attaching both would let the outer
 * fibers shadow the SiPMs.
 */
class DetectorConstruction final : public G4VUserDetectorConstruction
{
  public:
    //! Construct using a parser that owns the loaded GDML geometry.
    DetectorConstruction(G4GDMLParser const* parser,
                         SensitiveDetectorType detector_type);

    //! Return the world volume from the imported GDML geometry.
    G4VPhysicalVolume* Construct() final;

    //! Attach the selected optical detector.
    void ConstructSDandField() final;

  private:
    //! Attach the SiPM detector to the GDML optical-detector channels.
    void attach_sipms();

    //! Attach the fiber detector to volumes made of the fiber TPB coating.
    void attach_fibers();

    G4GDMLParser const* parser_;
    SensitiveDetectorType detector_type_;
};

//---------------------------------------------------------------------------//
} // namespace legend
