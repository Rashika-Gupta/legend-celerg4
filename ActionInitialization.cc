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

ActionInitialization::ActionInitialization(OpticalOffloadMode mode,
                                           IntegrationType integration)
    : mode_(mode)
    , integration_(integration)
{
}

//---------------------------------------------------------------------------//

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction(mode_, integration_));
}

//---------------------------------------------------------------------------//

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction(mode_));
    SetUserAction(new RunAction(mode_, integration_));
    SetUserAction(new EventAction(integration_));
    SetUserAction(new TrackingAction());
}

//---------------------------------------------------------------------------//
} // namespace legend
