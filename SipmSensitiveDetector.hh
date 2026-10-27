//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file detail/SipmSensitiveDetector.hh
//! \brief Record optical photons detected by LEGEND SiPM volumes
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <unordered_map>

// Geant4
#include "G4Types.hh"
#include "G4VSensitiveDetector.hh"

namespace legend
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Forward Geant4 optical-boundary detections to event output.
 *
 * Detector channels are supplied from the GDML optical-detector metadata.
 */
class SipmSensitiveDetector final : public G4VSensitiveDetector
{
  public:
    //! Map a SiPM physical-volume name to its LEGEND detector identifier.
    using DetectorIdMap = std::unordered_map<std::string, G4int>;

    //! Construct with the complete SiPM channel mapping.
    explicit SipmSensitiveDetector(DetectorIdMap detector_ids);

    /*!
     * Record an optical photon detected at a SiPM boundary.
     *
     * \param step Geant4 step passed by the optical boundary process
     * \return Whether the step produced a detector hit
     */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) final;

    //! LEGEND channel for a SiPM physical volume, or -1 if not a SiPM.
    G4int detector_id(std::string const& volume_name) const;

  private:
    DetectorIdMap detector_ids_;
};

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace legend

