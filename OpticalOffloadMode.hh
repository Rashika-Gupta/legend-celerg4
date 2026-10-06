//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalOffloadMode.hh
//! \brief Celeritas optical-offload modes used by the LEGEND application
//---------------------------------------------------------------------------//
#pragma once

#include <string_view>

namespace legend
{
    //---------------------------------------------------------------------------//
    /*!
     * Select how optical transport is handed from Geant4 to Celeritas.
     */
    enum class OpticalOffloadMode
    {
        optical_gun,
        optical_track,
        optical_distribution,
        electron_photon
    };

    //---------------------------------------------------------------------------//
    /*!
     * Select the Geant4 integration mechanism used for Celeritas transport.
     */
    enum class IntegrationType
    {
        tracking_manager,
        user_action
    };

    //---------------------------------------------------------------------------//
    /*!
     * Parse an optical-offload mode from user input.
     *
     * \param value Hyphenated mode name
     * \return Corresponding strongly typed mode
     */
    OpticalOffloadMode parse_transport_mode(std::string_view value);

    //---------------------------------------------------------------------------//
    /*!
     * Return the stable user-facing name of an optical-offload mode.
     */
    char const *to_cstring(OpticalOffloadMode mode);

    //---------------------------------------------------------------------------//
    /*!
     * Whether optical generation is integrated through Geant4 user actions.
     */
    bool uses_user_action_integration(IntegrationType integration);

    //---------------------------------------------------------------------------//
} // namespace legend
