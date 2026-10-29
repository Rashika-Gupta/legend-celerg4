//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventAction.hh
//! \brief Definition of the EventAction class
//---------------------------------------------------------------------------//
#pragma once

#include <chrono>
// Geant4
#include "G4UserEventAction.hh"

#include "OpticalOffloadMode.hh"

namespace legend
{
  //---------------------------------------------------------------------------//
  /*!
   * Coordinate Celeritas transport and analysis output for each event.
   */
  class EventAction final : public G4UserEventAction
  {
  public:
    explicit EventAction(OpticalOffloadMode offload_mode);

    //! Reset analysis state and initialize Celeritas for an event.
    void BeginOfEventAction(G4Event const *event) final;

    //! Flush transport and write analysis output for an event.
    void EndOfEventAction(G4Event const *event) final;

  private:
    OpticalOffloadMode offload_mode_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
  };

  //---------------------------------------------------------------------------//
} // namespace legend
