//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/DiagnosticOpticalSensitiveDetector.cc
//! \brief Implementation of the optical-step diagnostic detector
//---------------------------------------------------------------------------//

#include "DiagnosticOpticalSensitiveDetector.hh"

// Geant4
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4Track.hh"

// Celeritas
#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"

namespace legend
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Construct a detector used only to verify optical-step callbacks.
 */
DiagnosticOpticalSensitiveDetector::DiagnosticOpticalSensitiveDetector()
    : G4VSensitiveDetector("legend/optical-diagnostic")
{
}

//---------------------------------------------------------------------------//
/*!
 * Reset per-event callback counters.
 */
void DiagnosticOpticalSensitiveDetector::Initialize(G4HCofThisEvent*)
{
    num_callbacks_ = 0;
    num_optical_callbacks_ = 0;
}

//---------------------------------------------------------------------------//
/*!
 * Count one callback without imposing boundary or energy-deposition filters.
 */
G4bool DiagnosticOpticalSensitiveDetector::ProcessHits(
    G4Step* step, G4TouchableHistory*)
{
    CELER_EXPECT(step);
    ++num_callbacks_;

    auto const* track = step->GetTrack();
    CELER_ASSERT(track);
    if (track->GetParticleDefinition() != G4OpticalPhoton::Definition())
    {
        return false;
    }

    ++num_optical_callbacks_;
    if (num_optical_callbacks_ == 1)
    {
        CELER_LOG(status)
            << "Received the first optical-photon sensitive-detector callback";
    }
    return true;
}

//---------------------------------------------------------------------------//
/*!
 * Report whether Celeritas or Geant4 invoked the detector during the event.
 */
void DiagnosticOpticalSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
    CELER_LOG(status) << "Optical diagnostic SD event summary: callbacks="
                      << num_callbacks_ << ", optical="
                      << num_optical_callbacks_;
}

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

