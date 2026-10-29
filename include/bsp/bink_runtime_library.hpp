#pragma once

#include "bsp/movie_decoder.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace bsp {

// Public Bink sound callback types only. Bink owns this opaque state; this
// adapter neither allocates it nor projects its vendor-private representation.
struct BinkSoundState;
using BinkSoundOpen = std::int32_t (__stdcall*)(BinkSoundState*,
    std::uint32_t frequency, std::int32_t bits, std::int32_t channels,
    std::uint32_t flags, BinkHandle*);
using BinkSoundSystemOpen = BinkSoundOpen (__stdcall*)(std::uint32_t argument);

struct BinkFactoryImports {
    // _BinkSetSoundSystem@8; the original factory ignores the signed result.
    std::int32_t (__stdcall* set_sound_system)(BinkSoundSystemOpen,
        std::uint32_t argument);
    // _BinkOpenDirectSound@4, passed to set_sound_system, not called here.
    BinkSoundSystemOpen open_direct_sound;
};

// Owns one caller-selected installed Win32 Bink module and its complete import
// table. Construction binds all thirteen exports or throws before publication;
// it does not configure sound, create a decoder or call any imported function.
class BinkRuntimeLibrary final {
public:
    explicit BinkRuntimeLibrary(const std::wstring& absolute_dll_path);
    // Precondition: all decoder handles, sound callbacks, pending calls and
    // native owners borrowing this library have completed normal teardown.
    // If native teardown is interrupted, the composition must retain this
    // entire owner AND its dependent owner aggregate until process exit.
    ~BinkRuntimeLibrary();
    BinkRuntimeLibrary(const BinkRuntimeLibrary&) = delete;
    BinkRuntimeLibrary& operator=(const BinkRuntimeLibrary&) = delete;
    BinkRuntimeLibrary(BinkRuntimeLibrary&&) = delete;
    BinkRuntimeLibrary& operator=(BinkRuntimeLibrary&&) = delete;

    // Borrowed stable storage: the owner must outlive references, copied
    // function pointers and every vendor object/callback created through them.
    const BinkImports& imports() const noexcept;
    const BinkFactoryImports& factory_imports() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp
