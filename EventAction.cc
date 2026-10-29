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
#include "accel/SharedParams.hh"
#include "corecel/Assert.hh"

// Local
#include "CelerOpticalGunRunner.hh"
#include "EventOutput.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    EventAction::EventAction(OpticalOffloadMode offload_mode)
        : offload_mode_(offload_mode)
    {
    }

    //---------------------------------------------------------------------------//
    /*!
     * Reset event output.
     */
    void EventAction::BeginOfEventAction(G4Event const *event)
    {
        CELER_EXPECT(event);

        event_output().reset();

        if (offload_mode_ == OpticalOffloadMode::optical_gun && celeritas::SharedParams::GetMode() == celeritas::OffloadMode::enabled)
        {
            // Celeritas samples and transports this event's optical photons
            CelerOpticalGunRunner::Instance().GenerateAndTransport(event->GetEventID());
        }

        start_time_ = std::chrono::high_resolution_clock::now();
        G4cout << "Begin event " << event->GetEventID() << G4endl;
    }

    //---------------------------------------------------------------------------//
    /*!
     * Write all hits for the event.
     *
     * The tracking manager flushes Celeritas before this is called, so both
     * Geant4 and Celeritas hits are already buffered.
     */
    void EventAction::EndOfEventAction(G4Event const *event)
    {
        CELER_EXPECT(event);

        auto const duration = std::chrono::high_resolution_clock::now() - start_time_;
        auto const event_time = std::chrono::duration_cast<std::chrono::duration<double>>(duration)
                                    .count();

        auto &output = event_output();
        output.set_event_time(event_time);
        output.write_event();

        G4cout << "Event " << event->GetEventID() << ", End Time " << event_time
               << " seconds" << G4endl;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
