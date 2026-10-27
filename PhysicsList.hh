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


namespace legend
{
//---------------------------------------------------------------------------//
/*!
 * Configure Geant4 optical physics whose photon tracks Celeritas transports.
 */
class PhysicsList final : public FTFP_BERT_HP
{
  public:
    PhysicsList();

    //! Construct and reconcile Geant4 physics processes.
    void ConstructProcess() final;
};

//---------------------------------------------------------------------------//
} // namespace legend
