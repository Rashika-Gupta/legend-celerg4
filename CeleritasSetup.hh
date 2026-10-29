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

namespace legend
{
  //---------------------------------------------------------------------------//
  /*!
   * Construct Celeritas setup options for Geant4 optical-photon track offload.
   */
  class CeleritasSetup
  {
  public:
    //! Configuration required to build Celeritas setup options.
    struct Input
    {
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
  /*!
   * Build Celeritas setup options for a standalone optical-photon gun.
   *
   * Celeritas samples and transports the photons itself, using the same
   * photon count, energy, and position as the LEGEND Geant4 gun (see
   * \c legend::primary_gun).
   */
  celeritas::SetupOptions make_optical_gun_options(CeleritasSetup::Input const &input);

  //---------------------------------------------------------------------------//
} // namespace legend
