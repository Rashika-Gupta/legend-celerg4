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
#include "DetectorConstruction.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Initialize Celeritas and analysis output for each Geant4 run.
 */
class RunAction final : public G4UserRunAction
{
  public:
    explicit RunAction(SensitiveDetectorType detector_type);

    //! Initialize Celeritas and open analysis output for a run.
    void BeginOfRunAction(G4Run const* run) final;

    //! Write analysis output and finalize Celeritas for a run.
    void EndOfRunAction(G4Run const* run) final;

  private:
    SensitiveDetectorType detector_type_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    double run_time_{0};
    G4String file_name_{"out.csv"};
    std::unique_ptr<G4GenericMessenger> messenger_;
};

//---------------------------------------------------------------------------//
} // namespace legend
