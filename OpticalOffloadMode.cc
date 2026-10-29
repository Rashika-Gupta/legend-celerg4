//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalOffloadMode.cc
//! \brief Implementation of the LEGEND Celeritas optical-offload mode
//---------------------------------------------------------------------------//

#include "OpticalOffloadMode.hh"

#include "corecel/Assert.hh"

namespace legend
{
    //---------------------------------------------------------------------------//

    OpticalOffloadMode parse_offload_mode(std::string_view value)
    {
        if (value.empty() || value == "optical-track")
        {
            return OpticalOffloadMode::optical_track;
        }
        if (value == "optical-gun")
        {
            return OpticalOffloadMode::optical_gun;
        }
        CELER_VALIDATE(false,
                       << "invalid LEGEND_CELER_OFFLOAD_MODE '" << value << "'");
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//

    char const *to_cstring(OpticalOffloadMode value)
    {
        switch (value)
        {
        case OpticalOffloadMode::optical_track:
            return "optical-track";
        case OpticalOffloadMode::optical_gun:
            return "optical-gun";
        }
        CELER_ASSERT_UNREACHABLE();
    }

    //---------------------------------------------------------------------------//
} // namespace legend
