cmake_minimum_required(VERSION 3.16)

# ============================================================================
# Project
# ============================================================================

project(IFdataGen LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)

# Optional: tune for the build-host CPU
option(USE_NATIVE_OPT "Tune code for the host CPU (-march=native or /arch:AVX2)" ON)

# ============================================================================
# Dependencies
# ============================================================================

find_package(OpenMP QUIET)

# ============================================================================
# Sources and includes
# ============================================================================

file(GLOB SRC CONFIGURE_DEPENDS
     "IFdataGen.cpp"
     "../src/*.cpp")      # Adjust the path if your layout differs

add_executable(IFdataGen ${SRC})
target_include_directories(IFdataGen PRIVATE ../inc)

# ============================================================================
# IPO / LTO support check
# ============================================================================

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_OK OUTPUT IPO_ERR)

# ============================================================================
# Compiler and linker flags
# ============================================================================

if (MSVC)

    # ----- MSVC release and relwithdebinfo flag lists -----
    set(MSVC_RELEASE_FLAGS
        /O2 /Ot /fp:fast /Ob3 /Zi /permissive-)
    if (IPO_OK)
        list(APPEND MSVC_RELEASE_FLAGS /GL)
    endif()

    set(MSVC_RELDBG_FLAGS
        /O2 /Zi)
    if (IPO_OK)
        list(APPEND MSVC_RELDBG_FLAGS /GL)
    endif()

    target_compile_options(IFdataGen PRIVATE
        $<$<CONFIG:Release>:${MSVC_RELEASE_FLAGS}>
        $<$<CONFIG:RelWithDebInfo>:${MSVC_RELDBG_FLAGS}>
        $<$<CONFIG:Debug>:/Od /Zi>)

    # ----- MSVC link options -----
    if (IPO_OK)
        set(MSVC_LINK_LTCG /LTCG /INCREMENTAL:NO)
    else()
        set(MSVC_LINK_LTCG "")
    endif()

    target_link_options(IFdataGen PRIVATE
        $<$<CONFIG:Release>:${MSVC_LINK_LTCG}>
        $<$<CONFIG:RelWithDebInfo>:${MSVC_LINK_LTCG}>)

    # Optional host-CPU tuning
    if (USE_NATIVE_OPT)
        target_compile_options(IFdataGen PRIVATE /arch:AVX2)
    endif()

else()  # GCC / Clang branch

    # ----- GCC / Clang release flags -----
    set(GCC_CLANG_RELEASE_FLAGS
        -O3 -ffast-math -funroll-loops -fstrict-aliasing)
    if (IPO_OK)
        list(APPEND GCC_CLANG_RELEASE_FLAGS -flto=auto)
    endif()

    target_compile_options(IFdataGen PRIVATE
        $<$<CONFIG:Release>:${GCC_CLANG_RELEASE_FLAGS}>
        $<$<CONFIG:RelWithDebInfo>:-O3 -g>  #        $<$<CONFIG:RelWithDebInfo>:-O3 -g -ffast-math>
        $<$<CONFIG:Debug>:-O0 -g>)

    # Enable LTO at link time if supported
    if (IPO_OK)
        set_property(TARGET IFdataGen PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    # Optional host-CPU tuning
    if (USE_NATIVE_OPT)
        target_compile_options(IFdataGen PRIVATE -march=native -mtune=native)
    endif()

endif()

# ============================================================================
# OpenMP
# ============================================================================

if (OpenMP_CXX_FOUND)
    target_link_libraries(IFdataGen PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(IFdataGen PRIVATE USE_OPENMP)
endif()

# ============================================================================
# Hot-reload for MSVC (CMP0141)
# ============================================================================

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,\
        $<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,\
        $<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ============================================================================
# Status output
# ============================================================================

message(STATUS "OpenMP found           : ${OpenMP_CXX_FOUND}")
message(STATUS "IPO / LTO supported    : ${IPO_OK}")
message(STATUS "Host-CPU tuning enabled: ${USE_NATIVE_OPT}")

# TODO: Add tests and install targets if needed.
