//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.hh
//! \brief Definition of the RunAction class
//---------------------------------------------------------------------------//
#pragma once

#include <chrono>
#include <memory>

// Geant4
#include "G4GenericMessenger.hh"
#include "G4String.hh"
#include "G4UserRunAction.hh"

// Local
#include "OpticalOffloadMode.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Initialize Celeritas and analysis output for each Geant4 run.
 */
class RunAction final : public G4UserRunAction
{
  public:
    //! Construct for the selected Celeritas offload mode.
    RunAction(OpticalOffloadMode mode, IntegrationType integration);

    //! Initialize Celeritas and open analysis output for a run.
    void BeginOfRunAction(G4Run const* run) final;

    //! Write analysis output and finalize Celeritas for a run.
    void EndOfRunAction(G4Run const* run) final;

  private:
    //! Forward run initialization to the selected Celeritas integration.
    void begin_integration(G4Run const* run);

    //! Forward run finalization to the selected Celeritas integration.
    void end_integration(G4Run const* run);

    //! Report generated optical photons when collector data are available.
    void report_optical_generation() const;

    OpticalOffloadMode mode_;
    IntegrationType integration_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    double run_time_{0};
    G4String file_name_{"out.csv"};
    std::unique_ptr<G4GenericMessenger> messenger_;
};

//---------------------------------------------------------------------------//
} // namespace legend
