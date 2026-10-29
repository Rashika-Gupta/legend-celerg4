//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CelerOpticalGunRunner.hh
//! \brief Drive a standalone Celeritas optical-photon gun from Geant4
//---------------------------------------------------------------------------//
#pragma once

#include <functional>

#include "accel/SetupOptions.hh"
#include "accel/SharedParams.hh"

namespace legend
{
    //---------------------------------------------------------------------------//
    /*!
     * Drive Celeritas' optical-primary generator from the Geant4 event loop.
     *
     * This is separate from the Geant4 track-offload integration in
     * \c CeleritasSetup because an \c inp::OpticalPrimaryGenerator creates an
     * optical-only Celeritas problem: Celeritas samples and transports the
     * photons itself instead of receiving tracks created by Geant4. Ported from
     * OpticalSims-upstream's \c CelerOpticalPrimaryRunner.
     */
    class CelerOpticalGunRunner
    {
    public:
        using OptionsFactory = std::function<celeritas::SetupOptions()>;

        static CelerOpticalGunRunner &Instance();

        void SetOptionsFactory(OptionsFactory factory);
        void BeginOfRunAction();
        void GenerateAndTransport(int event_id);
        void EndOfRunAction();

    private:
        CelerOpticalGunRunner() = default;

        OptionsFactory options_factory_;
        celeritas::SetupOptions options_;
        celeritas::SharedParams shared_;
    };

    //---------------------------------------------------------------------------//
} // namespace legend
