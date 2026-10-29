//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalOffloadMode.hh
//! \brief Definition of the LEGEND Celeritas optical-offload mode
//---------------------------------------------------------------------------//
#pragma once

#include <string_view>

namespace legend
{
    //---------------------------------------------------------------------------//
    /*!
     * How optical photons are handed off to Celeritas.
     */
    enum class OpticalOffloadMode
    {
        optical_track, //!< Geant4 creates photons; Celeritas tracks them
        optical_gun    //!< Celeritas generates and tracks the photons itself
    };

    //---------------------------------------------------------------------------//
    /*!
     * Parse an optical-offload mode from user input.
     *
     * \param value Either \c "optical-track" or \c "optical-gun"
     */
    OpticalOffloadMode parse_offload_mode(std::string_view value);

    //! Name of an optical-offload mode (\c "optical-track" or \c "optical-gun").
    char const *to_cstring(OpticalOffloadMode value);

    //---------------------------------------------------------------------------//
} // namespace legend
