//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventOutput.cc
//! \brief Implementation of event-level LEGEND optical output
//---------------------------------------------------------------------------//

#include "EventOutput.hh"

#include <utility>

// Geant4
#include "G4AnalysisManager.hh"
#include "G4AutoLock.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
    namespace
    {
        G4Mutex output_mutex = G4MUTEX_INITIALIZER;

        enum class NtupleId : G4int
        {
            optical_hits,
            event_summary
        };

        enum class HitColumn : G4int
        {
            event_id,
            detector_id,
            primary_id,
            volume_instance_id,
            volume_name,
            x_cm,
            y_cm,
            z_cm,
            time_s,
            energy_mev,
            wavelength_nm
        };

        enum class SummaryColumn : G4int
        {
            event_id,
            elapsed_time_s,
            num_optical_hits
        };

        template <class T>
        constexpr G4int to_int(T value)
        {
            return static_cast<G4int>(value);
        }

        //! Convert photon energy in MeV to wavelength in nanometers.
        double energy_to_wavelength(double energy_mev)
        {
            CELER_EXPECT(energy_mev > 0);
            // Planck constant times speed of light in eV nm (CODATA 2018)
            constexpr double hc_ev_nm = 1239.8419843320026;
            return hc_ev_nm / (energy_mev * 1e6);
        }

    } // namespace

    //---------------------------------------------------------------------------//

    void EventOutput::create_ntuples(G4AnalysisManager &manager)
    {
        manager.CreateNtuple("optical_hits", "LEGEND optical detector hits");
        manager.CreateNtupleIColumn("event_id");
        manager.CreateNtupleIColumn("detector_id");
        manager.CreateNtupleIColumn("primary_id");
        manager.CreateNtupleIColumn("volume_instance_id");
        manager.CreateNtupleSColumn("volume_name");
        manager.CreateNtupleDColumn("x_cm");
        manager.CreateNtupleDColumn("y_cm");
        manager.CreateNtupleDColumn("z_cm");
        manager.CreateNtupleDColumn("time_s");
        manager.CreateNtupleDColumn("energy_mev");
        manager.CreateNtupleDColumn("wavelength_nm");
        manager.FinishNtuple();

        manager.CreateNtuple("event_summary", "LEGEND event summary");
        manager.CreateNtupleIColumn("event_id");
        manager.CreateNtupleDColumn("elapsed_time_s");
        manager.CreateNtupleIColumn("num_optical_hits");
        manager.FinishNtuple();
    }

    //---------------------------------------------------------------------------//

    void EventOutput::reset()
    {
        hits_.clear();
        event_time_ = 0;
    }

    //---------------------------------------------------------------------------//

    void EventOutput::set_event_time(double event_time)
    {
        CELER_EXPECT(event_time >= 0);
        event_time_ = event_time;
    }

    //---------------------------------------------------------------------------//

    void EventOutput::add_hit(OpticalDetectorHit hit)
    {
        hits_.push_back(std::move(hit));
    }

    //---------------------------------------------------------------------------//

    void EventOutput::add_hits(std::vector<OpticalDetectorHit> const &hits)
    {
        hits_.insert(hits_.end(), hits.begin(), hits.end());
    }

    //---------------------------------------------------------------------------//

    void EventOutput::write_event()
    {
        G4AutoLock lock(&output_mutex);
        auto *manager = G4AnalysisManager::Instance();
        CELER_ASSERT(manager);

        G4int const hit_ntuple = to_int(NtupleId::optical_hits);
        for (auto const &record : hits_)
        {
            CELER_ASSERT(record.detector_id >= 0);
            CELER_ASSERT(record.primary_id >= 0);
            CELER_ASSERT(record.volume_instance_id >= 0);
            CELER_ASSERT(record.energy_mev > 0);

            manager->FillNtupleIColumn(
                hit_ntuple, to_int(HitColumn::event_id), record.event_id);
            manager->FillNtupleIColumn(hit_ntuple,
                                       to_int(HitColumn::detector_id),
                                       record.detector_id);
            manager->FillNtupleIColumn(hit_ntuple,
                                       to_int(HitColumn::primary_id),
                                       record.primary_id);
            manager->FillNtupleIColumn(hit_ntuple,
                                       to_int(HitColumn::volume_instance_id),
                                       record.volume_instance_id);
            manager->FillNtupleSColumn(
                hit_ntuple, to_int(HitColumn::volume_name), record.volume_name);
            manager->FillNtupleDColumn(
                hit_ntuple, to_int(HitColumn::x_cm), record.position_cm[0]);
            manager->FillNtupleDColumn(
                hit_ntuple, to_int(HitColumn::y_cm), record.position_cm[1]);
            manager->FillNtupleDColumn(
                hit_ntuple, to_int(HitColumn::z_cm), record.position_cm[2]);
            manager->FillNtupleDColumn(
                hit_ntuple, to_int(HitColumn::time_s), record.time_s);
            manager->FillNtupleDColumn(
                hit_ntuple, to_int(HitColumn::energy_mev), record.energy_mev);
            manager->FillNtupleDColumn(hit_ntuple,
                                       to_int(HitColumn::wavelength_nm),
                                       energy_to_wavelength(record.energy_mev));
            manager->AddNtupleRow(hit_ntuple);
        }

        auto const *run_manager = G4RunManager::GetRunManager();
        CELER_ASSERT(run_manager);
        CELER_ASSERT(run_manager->GetCurrentEvent());

        G4int const summary_ntuple = to_int(NtupleId::event_summary);
        manager->FillNtupleIColumn(
            summary_ntuple,
            to_int(SummaryColumn::event_id),
            run_manager->GetCurrentEvent()->GetEventID());
        manager->FillNtupleDColumn(summary_ntuple,
                                   to_int(SummaryColumn::elapsed_time_s),
                                   event_time_);
        manager->FillNtupleIColumn(summary_ntuple,
                                   to_int(SummaryColumn::num_optical_hits),
                                   static_cast<G4int>(hits_.size()));
        manager->AddNtupleRow(summary_ntuple);

        this->reset();
    }

    //---------------------------------------------------------------------------//

    EventOutput &event_output()
    {
        thread_local EventOutput result;
        return result;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
