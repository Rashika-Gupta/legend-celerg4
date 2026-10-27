//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.cc
//! \brief Implementation of the DetectorConstruction class
//---------------------------------------------------------------------------//

#include "DetectorConstruction.hh"

#include <charconv>
#include <cstddef>
#include <system_error>
#include <unordered_set>
#include <utility>

// Geant4
#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4VPhysicalVolume.hh"

// Celeritas
#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"

// Local
#include "FiberSensitiveDetector.hh"
#include "SipmSensitiveDetector.hh"

namespace legend
{
    //---------------------------------------------------------------------------//
    SensitiveDetectorType parse_sensitive_detector(std::string_view value)
    {
        if (value == "sipm")
        {
            return SensitiveDetectorType::sipm;
        }
        if (value == "fiber")
        {
            return SensitiveDetectorType::fiber;
        }
        CELER_VALIDATE(false,
                       << "invalid LEGEND sensitive detector '" << value
                       << "': expected 'sipm' or 'fiber'");
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//
    char const *to_cstring(SensitiveDetectorType value)
    {
        switch (value)
        {
        case SensitiveDetectorType::sipm:
            return "sipm";
        case SensitiveDetectorType::fiber:
            return "fiber";
        }
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//
    /*!
     * Construct from a parser containing a loaded GDML geometry.
     */
    DetectorConstruction::DetectorConstruction(
        G4GDMLParser const *parser, SensitiveDetectorType detector_type)
        : parser_(parser), detector_type_(detector_type)
    {
        CELER_EXPECT(parser_);
    }

    //---------------------------------------------------------------------------//
    /*!
     * Return the world volume without modifying imported detector metadata.
     */
    G4VPhysicalVolume *DetectorConstruction::Construct()
    {
        auto *world = parser_->GetWorldVolume();
        CELER_ENSURE(world);
        return world;
    }

    //---------------------------------------------------------------------------//

    /*!
     * Attach the selected LEGEND optical detector.
     *
     * Celeritas converts every logical volume with a sensitive detector into
     * an optical detector that absorbs photons on entry, so only attach SDs
     * to volumes that should stop photons.
     */
    void DetectorConstruction::ConstructSDandField()
    {
        switch (detector_type_)
        {
        case SensitiveDetectorType::sipm:
            CELER_LOG(status) << "Sensitive detector: SiPMs";
            this->attach_sipms();
            return;
        case SensitiveDetectorType::fiber:
            CELER_LOG(status) << "Sensitive detector: fibers";
            this->attach_fibers();
            return;
        }
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//
    /*!
     * Attach the SiPM detector to the GDML optical-detector channels.
     */
    void DetectorConstruction::attach_sipms()
    {
        using DetectorIdMap = detail::SipmSensitiveDetector::DetectorIdMap;

        auto const *auxiliary_list = parser_->GetAuxList();
        CELER_VALIDATE(auxiliary_list,
                       << "GDML contains no global auxiliary metadata");

        DetectorIdMap detector_ids;
        for (auto const &auxiliary : *auxiliary_list)
        {
            if (auxiliary.type != "RMG_detector" || auxiliary.value != "optical")
            {
                continue;
            }

            CELER_VALIDATE(auxiliary.auxList,
                           << "RMG optical-detector metadata has no channels");
            for (auto const &channel : *auxiliary.auxList)
            {
                G4int detector_id{};
                char const *const begin = channel.value.data();
                char const *const end = begin + channel.value.size();
                auto const conversion = std::from_chars(begin, end, detector_id);
                CELER_VALIDATE(conversion.ec == std::errc{} && conversion.ptr == end && detector_id >= 0,
                               << "invalid detector ID '" << channel.value
                               << "' for SiPM volume '" << channel.type << "'");

                bool const inserted = detector_ids.emplace(channel.type, detector_id).second;
                CELER_VALIDATE(inserted,
                               << "duplicate SiPM volume metadata for '"
                               << channel.type << "'");
            }
        }
        CELER_VALIDATE(!detector_ids.empty(),
                       << "GDML contains no RMG optical-detector channels");

        auto *physical_store = G4PhysicalVolumeStore::GetInstance();
        CELER_ASSERT(physical_store);

        std::unordered_set<G4LogicalVolume *> logical_volumes;
        std::size_t num_physical_volumes = 0;
        for (auto *physical_volume : *physical_store)
        {
            CELER_ASSERT(physical_volume);
            if (detector_ids.find(physical_volume->GetName()) != detector_ids.end())
            {
                ++num_physical_volumes;
                logical_volumes.insert(physical_volume->GetLogicalVolume());
            }
        }

        CELER_VALIDATE(num_physical_volumes == detector_ids.size(),
                       << "found " << num_physical_volumes << " of "
                       << detector_ids.size()
                       << " GDML optical-detector physical volumes");
        CELER_VALIDATE(!logical_volumes.empty(),
                       << "no logical volume contains a configured SiPM");

        auto *detector_manager = G4SDManager::GetSDMpointer();
        CELER_ASSERT(detector_manager);

        auto *sipm_detector = new detail::SipmSensitiveDetector(std::move(detector_ids));
        detector_manager->AddNewDetector(sipm_detector);

        for (auto *logical_volume : logical_volumes)
        {
            CELER_ASSERT(logical_volume);
            this->SetSensitiveDetector(logical_volume, sipm_detector);
        }
    }

    //---------------------------------------------------------------------------//
    /*!
     * Attach the fiber detector to volumes made of the fiber TPB coating.
     */
    void DetectorConstruction::attach_fibers()
    {
        auto *detector_manager = G4SDManager::GetSDMpointer();
        CELER_ASSERT(detector_manager);

        auto *logical_store = G4LogicalVolumeStore::GetInstance();
        CELER_ASSERT(logical_store);

        std::vector<G4LogicalVolume *> fiber_volumes;
        CELER_LOG(status)
            << "Attaching fiber sensitive detector to logical volumes using "
               "material 'tpb_on_fibers'";
        for (auto *logical_volume : *logical_store)
        {
            CELER_ASSERT(logical_volume);

            auto const *material = logical_volume->GetMaterial();
            if (!material || material->GetName() != "tpb_on_fibers")
            {
                continue;
            }

            CELER_LOG(status)
                << "Fiber SD match: logical volume='"
                << logical_volume->GetName() << "', material='"
                << material->GetName() << "'";
            fiber_volumes.push_back(logical_volume);
        }

        CELER_VALIDATE(!fiber_volumes.empty(),
                       << "no logical volumes use material 'tpb_on_fibers'");

        auto *fiber_detector = new detail::FiberSensitiveDetector;
        detector_manager->AddNewDetector(fiber_detector);

        for (auto *logical_volume : fiber_volumes)
        {
            CELER_VALIDATE(!logical_volume->GetSensitiveDetector(),
                           << "logical volume '" << logical_volume->GetName()
                           << "' already has a sensitive detector");
            this->SetSensitiveDetector(logical_volume, fiber_detector);
        }
    }

    //---------------------------------------------------------------------------//
} // namespace legend
