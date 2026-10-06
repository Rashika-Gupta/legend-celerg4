//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventAction.cc
//! \brief Implementation of the EventAction class
//---------------------------------------------------------------------------//

#include "EventAction.hh"

// Geant4
#include "G4Event.hh"
#include "G4ios.hh"

// Celeritas
#include "accel/UserActionIntegration.hh"
#include "corecel/Assert.hh"

// Local
#include "EventOutput.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Construct the event action for an offload mode.
 */
EventAction::EventAction(IntegrationType integration)
    : integration_(integration)
{
}

//---------------------------------------------------------------------------//
/*!
 * Reset event output and initialize distribution offloading.
 */
void EventAction::BeginOfEventAction(G4Event const* event)
{
    CELER_EXPECT(event);

    event_output().reset();

    if (uses_user_action_integration(integration_))
    {
        celeritas::UserActionIntegration::Instance().BeginOfEventAction(event);
    }

    start_time_ = std::chrono::high_resolution_clock::now();
    G4cout << "Begin event " << event->GetEventID() << G4endl;
}

//---------------------------------------------------------------------------//
/*!
 * Flush transport and write all hit collections for the event.
 */
void EventAction::EndOfEventAction(G4Event const* event)
{
    CELER_EXPECT(event);
    if (uses_user_action_integration(integration_))
    {
        celeritas::UserActionIntegration::Instance().EndOfEventAction(event);
    }

    auto const duration
        = std::chrono::high_resolution_clock::now() - start_time_;
    auto const event_time
        = std::chrono::duration_cast<std::chrono::duration<double>>(duration)
              .count();

    auto& output = event_output();
    output.set_event_time(event_time);
    output.write_event();

    G4cout << "Event " << event->GetEventID() << ", End Time " << event_time
           << " seconds" << G4endl;
}

//---------------------------------------------------------------------------//
} // namespace legend
