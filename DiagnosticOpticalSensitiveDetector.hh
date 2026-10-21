//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/DiagnosticOpticalSensitiveDetector.hh
//! \brief Diagnose Celeritas callbacks for optical-photon steps
//---------------------------------------------------------------------------//
#pragma once

#include <cstddef>

// Geant4
#include "G4Types.hh"
#include "G4VSensitiveDetector.hh"

namespace legend
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Count optical-photon callbacks in a temporary diagnostic volume.
 *
 * This detector observes steps without modifying the transported photon or
 * writing physics-analysis output.
 */
class DiagnosticOpticalSensitiveDetector final : public G4VSensitiveDetector
{
  public:
    //! Construct the optical-step diagnostic detector.
    DiagnosticOpticalSensitiveDetector();

    //! Reset callback counters at the start of an event.
    void Initialize(G4HCofThisEvent*) final;

    /*!
     * Count a reconstructed optical-photon step.
     *
     * \param step Geant4 step supplied to the sensitive detector
     * \return Whether the callback represented an optical photon
     */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) final;

    //! Report callback counters at the end of an event.
    void EndOfEvent(G4HCofThisEvent*) final;

  private:
    std::size_t num_callbacks_{};
    std::size_t num_optical_callbacks_{};
};

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

