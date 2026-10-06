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
#include "G4PhysicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4VPhysicalVolume.hh"

// Celeritas
#include "corecel/Assert.hh"

// Local
#include "SipmSensitiveDetector.hh"

namespace legend
{
    //---------------------------------------------------------------------------//
    /*!
     * Construct from a parser containing a loaded GDML geometry.
     */
    DetectorConstruction::DetectorConstruction(G4GDMLParser const *parser)
        : parser_(parser)
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
     * Attach the LEGEND SiPM detector using the imported GDML channel metadata.
     */
    void DetectorConstruction::ConstructSDandField()
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

        auto *sensitive_detector = new detail::SipmSensitiveDetector(std::move(detector_ids));
        auto *detector_manager = G4SDManager::GetSDMpointer();
        CELER_ASSERT(detector_manager);
        detector_manager->AddNewDetector(sensitive_detector);

        for (auto *logical_volume : logical_volumes)
        {
            CELER_ASSERT(logical_volume);
            this->SetSensitiveDetector(logical_volume, sensitive_detector);
        }
    }

    //---------------------------------------------------------------------------//
} // namespace legend
