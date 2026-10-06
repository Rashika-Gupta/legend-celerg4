//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CeleritasSetup.hh
//! \brief Build Celeritas options for LEGEND optical transport
//---------------------------------------------------------------------------//
#pragma once

#include <string>

// Celeritas
#include "accel/SetupOptions.hh"
#include "corecel/Types.hh"

// Local
#include "OpticalOffloadMode.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Construct Celeritas setup options for one optical-offload mode.
 */
class CeleritasSetup
{
  public:
    //! Configuration required to build Celeritas setup options.
    struct Input
    {
        OpticalOffloadMode mode;
        celeritas::size_type num_track_slots{65536};
        std::string diagnostics_file{"celeritas.out.json"};
    };

    //! Construct from complete validated setup input.
    explicit CeleritasSetup(Input input);

    //! Build complete setup options.
    celeritas::SetupOptions operator()() const;

  private:
    Input input_;
};

//---------------------------------------------------------------------------//
} // namespace legend
