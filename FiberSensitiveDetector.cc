//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/FiberSensitiveDetector.cc
//! \brief Implementation of the LEGEND fiber sensitive detector
//---------------------------------------------------------------------------//

#include "FiberSensitiveDetector.hh"

// Geant4
#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4OpticalPhoton.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

// Celeritas
#include "corecel/Assert.hh"

// Local
#include "EventOutput.hh"

namespace legend
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Construct the detector used to monitor fiber-coating entries.
 */
FiberSensitiveDetector::FiberSensitiveDetector()
    : G4VSensitiveDetector("legend/fiber")
{
}

//---------------------------------------------------------------------------//
/*!
 * Return the shared logical volume for the outer TPB fiber coating.
 */
char const* FiberSensitiveDetector::logical_volume_name()
{
    return "fiber_coating_tpb_l1349.00_tpb1093";
}

//---------------------------------------------------------------------------//
/*!
 * Buffer an optical photon's entry into a fiber coating.
 */
G4bool FiberSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    CELER_EXPECT(step);
    auto const* track = step->GetTrack();
    CELER_ASSERT(track);

    if (track->GetParticleDefinition() != G4OpticalPhoton::Definition())
    {
        return false;
    }

    auto const* pre_step = step->GetPreStepPoint();
    CELER_ASSERT(pre_step);
    if (pre_step->GetStepStatus() != fGeomBoundary)
    {
        return false;
    }

    auto const* physical_volume = pre_step->GetPhysicalVolume();
    if (!physical_volume)
    {
        return false;
    }

    auto const* logical_volume = physical_volume->GetLogicalVolume();
    CELER_ASSERT(logical_volume);
    if (logical_volume->GetName() != logical_volume_name())
    {
        return false;
    }

    auto const* run_manager = G4RunManager::GetRunManager();
    CELER_ASSERT(run_manager);
    auto const* event = run_manager->GetCurrentEvent();
    CELER_ASSERT(event);

    auto const& position = pre_step->GetPosition();
    event_output().add_fiber_hit({event->GetEventID(),
                                  physical_volume->GetInstanceID(),
                                  physical_volume->GetName(),
                                  {position.x() / cm,
                                   position.y() / cm,
                                   position.z() / cm},
                                  pre_step->GetGlobalTime() / second,
                                  pre_step->GetKineticEnergy() / MeV});
    return true;
}

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

