#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.16...3.21)
project(legend-celerg4)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available Vis drivers by default
# You can set WITH_GEANT4_VIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(LEGEND_USE_GEANT4_VIS "Enable Geant4 visualization" ON)
if(LEGEND_USE_GEANT4_VIS)
  find_package(Geant4 REQUIRED gdml vis_all)
else()
  find_package(Geant4 REQUIRED gdml)
endif()

#----------------------------------------------------------------------------
# Setup Celeritas package
#
# TODO(LEGEND): point at your Celeritas build/install, e.g.
#   cmake -B build -DCeleritas_DIR=/path/to/celeritas/build/lib/cmake/Celeritas
if(NOT DEFINED Celeritas_DIR AND DEFINED ENV{Celeritas_DIR})
  set(Celeritas_DIR "$ENV{Celeritas_DIR}")
endif()
find_package(Celeritas 0.7 REQUIRED)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Define application sources explicitly so obsolete files cannot be linked
# accidentally.
#
set(legend_celer_sources
  ActionInitialization.cc
  CeleritasSetup.cc
  DetectorConstruction.cc
  EventAction.cc
  EventOutput.cc
  OpticalOffloadMode.cc
  PhysicsList.cc
  PrimaryGeneratorAction.cc
  RunAction.cc
  TrackingAction.cc
  SipmSensitiveDetector.cc
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(legend_celer legend_celer.cc ${legend_celer_sources})
target_include_directories(legend_celer PRIVATE ${PROJECT_SOURCE_DIR})

#----------------------------------------------------------------------------
# Set Celeritas offload mode
set(LEGEND_CELER_OFFLOAD_MODE "optical-track" CACHE STRING
    "Default Celeritas offload mode")

set_property(CACHE LEGEND_CELER_OFFLOAD_MODE PROPERTY STRINGS
    optical-track
    optical-distribution
    electron-photon
    optical-gun)

#----------------------------------------------------------------------------
# Link Celeritas libraries
target_compile_definitions(legend_celer PRIVATE
    LEGEND_DEFAULT_OFFLOAD_MODE="${LEGEND_CELER_OFFLOAD_MODE}")
if(LEGEND_USE_GEANT4_VIS)
  target_compile_definitions(legend_celer PRIVATE LEGEND_USE_GEANT4_VIS=1)
endif()
if(NOT CELERITAS_USE_Geant4)
  message(FATAL_ERROR "This application requires Geant4 support "
    "to be enabled in Celeritas")
endif()

target_link_libraries(legend_celer PRIVATE
  Celeritas::celeritas
  Celeritas::G4
  ${Geant4_LIBRARIES}
)
#----------------------------------------------------------------------------
# Copy runtime inputs to the build directory.
#
# TODO(LEGEND): add any additional run macros/geometry files needed
set(RUN_SCRIPTS
    input/legend-optical.mac
    input/celeritas-legendl1000-no-tesselated.gdml)
foreach(_script ${RUN_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()


#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS legend_celer DESTINATION bin)
