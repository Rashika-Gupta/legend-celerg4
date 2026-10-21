//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/FiberSensitiveDetector.hh
//! \brief Record optical-photon entries into LEGEND fiber coatings
//---------------------------------------------------------------------------//
#pragma once

// Geant4
#include "G4Types.hh"
#include "G4VSensitiveDetector.hh"

namespace legend
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Record optical photons entering the outer TPB fiber coating.
 *
 * The detector observes fiber contacts without terminating or otherwise
 * modifying the transported photon.
 */
class FiberSensitiveDetector final : public G4VSensitiveDetector
{
  public:
    //! Construct the fiber-coating detector.
    FiberSensitiveDetector();

    //! Return the GDML logical-volume name monitored by this detector.
    static char const* logical_volume_name();

    /*!
     * Record the first step after an optical photon enters a fiber coating.
     *
     * \param step Geant4 step within the sensitive fiber-coating volume
     * \return Whether the step produced a fiber-contact record
     */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) final;
};

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

