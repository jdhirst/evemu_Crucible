# MIT License
#
# Copyright (c) 2019 REGoth-project
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if(WIN32)
    set(VCPKG_FALLBACK_ROOT ${CMAKE_CURRENT_BINARY_DIR}/vcpkg CACHE STRING "vcpkg configuration directory to use if vcpkg was not installed on the system before")
else()
    set(VCPKG_FALLBACK_ROOT ${CMAKE_CURRENT_BINARY_DIR}/.vcpkg CACHE STRING  "vcpkg configuration directory to use if vcpkg was not installed on the system before")
endif()

# Find out whether the user supplied their own VCPKG toolchain file
if(NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})
    if(NOT DEFINED ENV{VCPKG_ROOT})
        if(WIN32)
            set(VCPKG_ROOT ${VCPKG_FALLBACK_ROOT})
        else()
            set(VCPKG_ROOT ${VCPKG_FALLBACK_ROOT})
        endif()
    else()
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    endif()

    set(AUTOMATE_VCPKG_USE_SYSTEM_VCPKG OFF)
else()
    # VCPKG_ROOT has been defined by the toolchain file already
    set(AUTOMATE_VCPKG_USE_SYSTEM_VCPKG ON)
endif()

# Installs a new copy of Vcpkg or updates an existing one
macro(vcpkg_bootstrap)
    if(NOT AUTOMATE_VCPKG_USE_SYSTEM_VCPKG)
        _install_or_update_vcpkg()

        set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

        # Just setting vcpkg.cmake as toolchain file does not seem to actually pull in the code
        include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
    else()
        message(STATUS "Automate VCPKG using System VCPKG installation.")
    endif()

    message(STATUS "Automate VCPKG status:")
    message(STATUS "  VCPKG_ROOT.....: ${VCPKG_ROOT}")
    message(STATUS "  VCPKG_EXEC.....: ${VCPKG_EXEC}")
    message(STATUS "  VCPKG_BOOTSTRAP: ${VCPKG_BOOTSTRAP}")
endmacro()

macro(_install_or_update_vcpkg)
    if(NOT EXISTS ${VCPKG_ROOT})
        message(STATUS "Cloning vcpkg in ${VCPKG_ROOT}")
        execute_process(COMMAND git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT})

        # If a reproducible build is desired (and potentially old libraries are # ok), uncomment the
        # following line and pin the vcpkg repository to a specific githash.
        # execute_process(COMMAND git checkout 745a0aea597771a580d0b0f4886ea1e3a94dbca6 WORKING_DIRECTORY ${VCPKG_ROOT})
    else()
        # Check if vcpkg is a git repository before attempting git operations
        execute_process(
            COMMAND git rev-parse --git-dir
            WORKING_DIRECTORY ${VCPKG_ROOT}
            RESULT_VARIABLE GIT_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )
        
        if(GIT_RESULT EQUAL 0)
            # It's a git repository, safe to update
            if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
                # The following command has no effect if the vcpkg repository is in a detached head state.
                message(STATUS "Auto-updating vcpkg in ${VCPKG_ROOT}")
                execute_process(COMMAND git pull WORKING_DIRECTORY ${VCPKG_ROOT})
            else()
                message(STATUS "Using Visual Studio generator - skipping vcpkg auto-update")
            endif()
        else()
            # Not a git repository (e.g., Visual Studio's built-in vcpkg)
            message(STATUS "vcpkg at ${VCPKG_ROOT} is not a git repository - skipping auto-update (assuming system-managed vcpkg)")
        endif()
    endif()

    if(WIN32)
        set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg.exe)
        set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.bat)
    else()
        set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg)
        set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.sh)
    endif()

    # Verify vcpkg executable exists or can be bootstrapped
    if(NOT EXISTS ${VCPKG_EXEC})
        # Check if we have a README.md (indicating a proper git clone that can be bootstrapped)
        if(EXISTS ${VCPKG_ROOT}/README.md)
            message("Bootstrapping vcpkg in ${VCPKG_ROOT}")
            execute_process(COMMAND ${VCPKG_BOOTSTRAP} WORKING_DIRECTORY ${VCPKG_ROOT})
        else()
            # System-managed vcpkg without README.md - check if executable exists somewhere
            if(NOT EXISTS ${VCPKG_EXEC})
                message(FATAL_ERROR "***** FATAL ERROR: vcpkg executable not found at ${VCPKG_EXEC} and cannot bootstrap *****")
            endif()
        endif()
    endif()

    if(NOT EXISTS ${VCPKG_EXEC})
        message(FATAL_ERROR "***** FATAL ERROR: Could not find or bootstrap vcpkg *****")
    endif()

endmacro()

# Installs the list of packages given as parameters using Vcpkg
macro(vcpkg_install_packages)

    message(STATUS "Installing/Updating the following vcpkg-packages: ${ARGN}")

    if (VCPKG_TRIPLET)
        set(ENV{VCPKG_DEFAULT_TRIPLET} "${VCPKG_TRIPLET}")
    endif()

    execute_process(
            COMMAND ${VCPKG_EXEC} install ${ARGN}
            WORKING_DIRECTORY ${VCPKG_ROOT}
    )
endmacro()
