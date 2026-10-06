//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.hh
//! \brief Definition of the LEGEND Geant4 physics list
//---------------------------------------------------------------------------//
#pragma once

// Geant4
#include "FTFP_BERT_HP.hh"

// Celeritas
#include "celeritas/ext/GeantOpticalPhysicsOptions.hh"
#include "celeritas/ext/GeantPhysicsOptions.hh"

// Local
#include "OpticalOffloadMode.hh"

namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Configure Geant4 physics for Celeritas optical offloading.
 */
class PhysicsList final : public FTFP_BERT_HP
{
  public:
    //! Construct physics for the selected offload mode.
    explicit PhysicsList(OpticalOffloadMode mode);

    //! Construct and reconcile Geant4 physics processes.
    void ConstructProcess() final;

  private:
    //! Build Celeritas-supported optical physics options.
    celeritas::GeantOpticalPhysicsOptions optical_options() const;

    //! Build the complete Celeritas Geant4 physics configuration.
    celeritas::GeantPhysicsOptions physics_options() const;
};

//---------------------------------------------------------------------------//
} // namespace legend
