//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TrackingAction.hh
//! \brief Definition of the TrackingAction class
//---------------------------------------------------------------------------//
#pragma once

// Geant4
#include "G4UserTrackingAction.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Disable Geant4 trajectory storage for optical photons.
 */
class TrackingAction final : public G4UserTrackingAction
{
  public:
    //! Configure trajectory storage for the incoming track.
    void PreUserTrackingAction(G4Track const* track) final;
};

//---------------------------------------------------------------------------//
} // namespace legend
