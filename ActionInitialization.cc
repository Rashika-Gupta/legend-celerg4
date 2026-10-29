//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ActionInitialization.cc
//! \brief Implementation of the ActionInitialization class
//---------------------------------------------------------------------------//

#include "ActionInitialization.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "TrackingAction.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    void ActionInitialization::BuildForMaster() const
    {
        SetUserAction(new RunAction(detector_type_, offload_mode_));
    }

    //---------------------------------------------------------------------------//

    void ActionInitialization::Build() const
    {
        SetUserAction(new PrimaryGeneratorAction(detector_type_, offload_mode_));
        SetUserAction(new RunAction(detector_type_, offload_mode_));
        SetUserAction(new EventAction(offload_mode_));
        SetUserAction(new TrackingAction());
    }

    //---------------------------------------------------------------------------//
} // namespace legend
