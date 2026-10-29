//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CeleritasSetup.cc
//! \brief Implementation of LEGEND Celeritas optical setup
//---------------------------------------------------------------------------//

#include "CeleritasSetup.hh"

#include <array>
#include <utility>
#include <vector>

// Geant4
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4LogicalVolume.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"

// Celeritas
#include "accel/AlongStepFactory.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/inp/OpticalPhysics.hh"
#include "celeritas/optical/DetectorData.hh"
#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"
#include "geocel/GeantGeoParams.hh"
#include "geocel/g4/Convert.hh"

// Local
#include "EventOutput.hh"
#include "FiberSensitiveDetector.hh"
#include "PrimaryGeneratorAction.hh"
#include "SipmSensitiveDetector.hh"

namespace legend
{
    namespace
    {
        //---------------------------------------------------------------------------//
        //! Buffer Celeritas optical hits for event-level analysis output.
        void record_optical_hits(
            celeritas::Span<celeritas::optical::DetectorHit const> hits)
        {
            auto const *event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
            CELER_EXPECT(event);

            auto geant_geo = celeritas::global_geant_geo().lock();
            CELER_VALIDATE(geant_geo, << "Geant4 geometry mapping is unavailable");

            std::vector<OpticalDetectorHit> records;
            std::vector<OpticalFiberHit> fiber_records;
            records.reserve(hits.size());
            fiber_records.reserve(hits.size());
            for (auto const &hit : hits)
            {
                CELER_VALIDATE(hit.detector, << "optical hit has no detector ID");

                CELER_VALIDATE(hit.volume_instance,
                               << "optical hit has no volume instance ID");

                auto const *physical = geant_geo->id_to_geant(hit.volume_instance);
                CELER_VALIDATE(physical,
                               << "no Geant4 volume corresponds to Celeritas volume "
                               << hit.volume_instance.unchecked_get());

                double const energy_mev = celeritas::value_as<celeritas::units::MevEnergy>(
                    hit.energy);

                auto const *logical = physical->GetLogicalVolume();
                CELER_ASSERT(logical);
                auto const *sd = logical->GetSensitiveDetector();
                std::array<double, 3> const position{
                    hit.position[0], hit.position[1], hit.position[2]};
                G4int const instance_id = static_cast<G4int>(
                    hit.volume_instance.unchecked_get());

                if (dynamic_cast<detail::FiberSensitiveDetector const *>(sd))
                {
                    fiber_records.push_back({event->GetEventID(),
                                             instance_id,
                                             physical->GetName(),
                                             position,
                                             hit.time,
                                             energy_mev});
                    continue;
                }

                auto const *sipm = dynamic_cast<detail::SipmSensitiveDetector const *>(sd);
                CELER_VALIDATE(sipm,
                               << "optical hit in volume '" << physical->GetName()
                               << "' has no LEGEND sensitive detector");

                // Store the GDML channel rather than the Celeritas detector
                // index so Geant4 and Celeritas output are comparable
                G4int const channel = sipm->detector_id(physical->GetName());
                CELER_VALIDATE(channel >= 0,
                               << "no GDML channel for SiPM volume '"
                               << physical->GetName() << "'");

                records.push_back({event->GetEventID(),
                                   channel,
                                   hit.primary ? static_cast<G4int>(
                                                     hit.primary.unchecked_get())
                                               : -1,
                                   instance_id,
                                   physical->GetName(),
                                   position,
                                   hit.time,
                                   energy_mev});
            }

            CELER_LOG(debug) << "Buffering " << records.size()
                             << " LEGEND optical detector hits";
            CELER_LOG(debug) << "Buffering " << fiber_records.size()
                             << " LEGEND optical fiber contacts";
            auto &output = event_output();
            output.add_hits(records);
            output.add_fiber_hits(fiber_records);
        }
    } // namespace

    //---------------------------------------------------------------------------//

    CeleritasSetup::CeleritasSetup(Input input) : input_(std::move(input))
    {
        CELER_VALIDATE(input_.num_track_slots > 0,
                       << "optical track capacity must be positive");
        CELER_VALIDATE(!input_.diagnostics_file.empty(),
                       << "Celeritas diagnostics filename is empty");
    }

    //---------------------------------------------------------------------------//

    celeritas::SetupOptions CeleritasSetup::operator()() const
    {
        celeritas::SetupOptions result;
        result.sd.enabled = true;
        result.sd.ignore_zero_deposition = false;
        result.make_along_step = celeritas::UniformAlongStepFactory();
        result.output_file = input_.diagnostics_file;

        celeritas::OpticalSetupOptions optical;
        optical.capacity.tracks = input_.num_track_slots;
        optical.capacity.primaries = 8 * input_.num_track_slots;
        optical.capacity.generators = 2 * input_.num_track_slots;
        optical.detectors.callback = record_optical_hits;

        // Geant4 creates optical photons; Celeritas transports their tracks
        result.offload_particles = {G4OpticalPhoton::Definition()};
        optical.generator = celeritas::inp::OpticalDirectGenerator{};
        result.optical = std::move(optical);
        return result;
    }

    //---------------------------------------------------------------------------//
    /*!
     * Build setup options for a standalone Celeritas optical-photon gun.
     *
     * No Geant4 particles are offloaded; Celeritas generates the photons
     * itself from the current LEGEND gun configuration.
     */
    celeritas::SetupOptions make_optical_gun_options(CeleritasSetup::Input const &input)
    {
        celeritas::SetupOptions result;
        result.sd.enabled = false;
        result.make_along_step = celeritas::UniformAlongStepFactory();
        result.output_file = input.diagnostics_file;

        celeritas::OpticalSetupOptions optical;
        optical.capacity.tracks = input.num_track_slots;
        optical.capacity.primaries = 8 * input.num_track_slots;
        optical.capacity.generators = 2 * input.num_track_slots;
        optical.detectors.callback = record_optical_hits;

        auto &gun = primary_gun();
        celeritas::inp::OpticalPrimaryGenerator gen;
        gen.primaries = static_cast<celeritas::size_type>(gun.num_photons());
        gen.energy = celeritas::inp::MonoenergeticDistribution{gun.energy() / MeV};
        gen.angle = celeritas::inp::IsotropicDistribution{};
        // Celeritas builds one fixed source per run, so only the first
        // configured gun position (not an added-position list) is used
        auto const &pos = gun.position(0);
        gen.shape = celeritas::inp::PointDistribution{
            {pos.x() / cm, pos.y() / cm, pos.z() / cm}};

        optical.generator = std::move(gen);
        result.optical = std::move(optical);
        return result;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
