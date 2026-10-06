//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TrackingAction.cc
//! \brief Implementation of the TrackingAction class
//---------------------------------------------------------------------------//

#include "TrackingAction.hh"

// Geant4
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Store trajectories only for non-optical tracks.
 */
void TrackingAction::PreUserTrackingAction(G4Track const* track)
{
    CELER_EXPECT(track);
    CELER_ASSERT(fpTrackingManager);

    bool const store_trajectory
        = track->GetParticleDefinition() != G4OpticalPhoton::Definition();
    fpTrackingManager->SetStoreTrajectory(store_trajectory);
}

//---------------------------------------------------------------------------//
} // namespace legend
