//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalOffloadMode.cc
//! \brief Implementation of LEGEND optical-offload mode utilities
//---------------------------------------------------------------------------//

#include "OpticalOffloadMode.hh"

// Celeritas
#include "corecel/Assert.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    OpticalOffloadMode parse_transport_mode(std::string_view value)
    {
        if (value == "optical-gun")
        {
            return OpticalOffloadMode::optical_gun;
        }
        if (value == "optical-track")
        {
            return OpticalOffloadMode::optical_track;
        }
        if (value == "optical-distribution")
        {
            return OpticalOffloadMode::optical_distribution;
        }
        if (value == "electron-photon")
        {
            return OpticalOffloadMode::electron_photon;
        }

        CELER_VALIDATE(false,
                       << "invalid LEGEND Celeritas offload mode '" << value
                       << "'");
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//

    char const *to_cstring(OpticalOffloadMode mode)
    {
        switch (mode)
        {
        case OpticalOffloadMode::optical_gun:
            return "optical-gun";
        case OpticalOffloadMode::optical_track:
            return "optical-track";
        case OpticalOffloadMode::optical_distribution:
            return "optical-distribution";
        case OpticalOffloadMode::electron_photon:
            return "electron-photon";
        }
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//

    bool uses_user_action_integration(IntegrationType integration)
    {
        return integration == IntegrationType::user_action;
    }

    //---------------------------------------------------------------------------//
} // namespace legend
