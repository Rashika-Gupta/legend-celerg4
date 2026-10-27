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
    SetUserAction(new RunAction(detector_type_));
}

//---------------------------------------------------------------------------//

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction(detector_type_));
    SetUserAction(new RunAction(detector_type_));
    SetUserAction(new EventAction);
    SetUserAction(new TrackingAction());
}

//---------------------------------------------------------------------------//
} // namespace legend
