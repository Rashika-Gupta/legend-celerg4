//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/SipmSensitiveDetector.cc
//! \brief Implementation of the LEGEND SiPM sensitive detector
//---------------------------------------------------------------------------//

#include "SipmSensitiveDetector.hh"

#include <utility>

// Geant4
#include "G4Event.hh"
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
 * Construct from identifiers read from the GDML optical-detector metadata.
 */
SipmSensitiveDetector::SipmSensitiveDetector(DetectorIdMap detector_ids)
    : G4VSensitiveDetector("legend/sipm")
    , detector_ids_(std::move(detector_ids))
{
    CELER_EXPECT(!detector_ids_.empty());
}

//---------------------------------------------------------------------------//
/*!
 * Buffer one optical photon detected at a configured SiPM boundary.
 */
G4bool SipmSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    CELER_EXPECT(step);
    auto const* track = step->GetTrack();
    CELER_ASSERT(track);

    if (track->GetParticleDefinition() != G4OpticalPhoton::Definition())
    {
        return false;
    }

    auto const* post_step = step->GetPostStepPoint();
    CELER_ASSERT(post_step);
    if (post_step->GetStepStatus() != fGeomBoundary)
    {
        return false;
    }

    auto const* physical_volume = post_step->GetPhysicalVolume();
    if (!physical_volume)
    {
        return false;
    }

    G4int const channel = this->detector_id(physical_volume->GetName());
    if (channel < 0)
    {
        return false;
    }

    auto const* run_manager = G4RunManager::GetRunManager();
    CELER_ASSERT(run_manager);
    auto const* event = run_manager->GetCurrentEvent();
    CELER_ASSERT(event);

    auto const& position = post_step->GetPosition();
    event_output().add_hit({event->GetEventID(),
                            channel,
                            track->GetTrackID(),
                            physical_volume->GetInstanceID(),
                            physical_volume->GetName(),
                            {position.x() / cm,
                             position.y() / cm,
                             position.z() / cm},
                            post_step->GetGlobalTime() / second,
                            post_step->GetKineticEnergy() / MeV});

    // Absorb the photon, matching Celeritas optical detectors
    step->GetTrack()->SetTrackStatus(fStopAndKill);
    return true;
}

//---------------------------------------------------------------------------//
/*!
 * Look up the GDML channel for a SiPM physical volume.
 */
G4int SipmSensitiveDetector::detector_id(std::string const& volume_name) const
{
    auto const iter = detector_ids_.find(volume_name);
    return iter != detector_ids_.end() ? iter->second : -1;
}

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

