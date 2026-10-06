//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventOutput.hh
//! \brief Event-level optical detector output for LEGEND
//---------------------------------------------------------------------------//
#pragma once

#include <array>
#include <string>
#include <vector>

// Geant4
#include "G4AnalysisManager.hh"
#include "G4Types.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Store one backend-independent optical-detector hit.
 */
struct OpticalDetectorHit
{
    G4int event_id{};
    G4int detector_id{};
    G4int primary_id{};
    G4int volume_instance_id{};
    std::string volume_name;
    std::array<double, 3> position_cm{};
    double time_s{};
    double energy_mev{};
};

//---------------------------------------------------------------------------//
/*!
 * Accumulate and write event-level LEGEND optical output.
 *
 * Each worker thread owns one complete instance through \c event_output.
 */
class EventOutput
{
  public:
    //! Create all analysis ntuples in their stable output order.
    static void create_ntuples(G4AnalysisManager& manager);

    //! Clear all state at the beginning of an event.
    void reset();

    //! Set the event wall-clock duration in seconds.
    void set_event_time(double event_time);

    //! Append one detected optical photon to the current event.
    void add_hit(OpticalDetectorHit hit);

    //! Append detected optical photons to the current event.
    void add_hits(std::vector<OpticalDetectorHit> const& hits);

    //! Write and clear the current event.
    void write_event();

  private:
    std::vector<OpticalDetectorHit> hits_;
    double event_time_{0};
};

//---------------------------------------------------------------------------//
/*!
 * Access output state belonging to the current Geant4 worker thread.
 */
EventOutput& event_output();

//---------------------------------------------------------------------------//
} // namespace legend
