#pragma once

#include "bsp/native_cube_texture_owner.hpp"
#include "bsp/native_volume_texture_owner.hpp"

namespace bsp {
class ActualNativeStringPoolStorage;

// The existing volume destruction context does not consume the shared serial.
// Named construction borrows the SAME actual0108D6E8 word used by cube/2D.
struct NativeNamedVolumeTextureContext {
    NativeVolumeTextureOwnerContext& owner;
    std::uint32_t& actual_shared_serial_0108d6e8;
};

// Optional diagnostics for the caller's already-owned raw slot and incoming
// COM reference. No allocation, second count, automatic rollback or canonical
// registration occurs here. On complete, creator is that same raw owner and
// its existing terminal consumes the input COM reference. On failure, the
// caller still owns slot/COM; native base cleanup releases only the name/base.
// The frame is single-use and preserves native cleanup preimages.
struct NativeNamedSpecialTextureAcquired final {
    enum class Phase { fresh, named_base, descriptor, level_count, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    void* raw_owner{};
    void* input_com{};
    void* creator{};
    bool named_base_complete{};
    bool cleanup_started{};
    bool named_base_unwound{};
};

// Complete B34280/B342D0: ECX raw24h prefix; stack name/COM/flags;
// EAX same owner; RET0C. Invoke actual B34120 then install D5F280/D5F2C0.
// One count initialization and one shared serial consumption, no COM AddRef.
void* construct_native_named_cube_texture_base_00b34280(void* actual_owner,
    const void* actual_name, void* input_com, std::uint32_t flags,
    ActualNativeStringPoolStorage&, std::uint32_t& actual_shared_serial_0108d6e8);
void* construct_native_named_volume_texture_base_00b342d0(void* actual_owner,
    const void* actual_name, void* input_com, std::uint32_t flags,
    ActualNativeStringPoolStorage&, std::uint32_t& actual_shared_serial_0108d6e8);

// Complete B3CED0: ECX actual30h owner in34h cube slot, name/COM/flags stack,
// EAX same owner, RET0C. Install D61870 before clearing source+2C. Query
// captured INPUT COM current+44 GetLevelDesc(0), store Width+24, then reload
// that input's current+34 GetLevelCount, store count+14, read/store Format+18.
// Keep owner+28 and slot+30 unchanged. EH state0 runs B34090->B33F50 only.
void* construct_native_named_cube_texture_00b3ced0(void* actual_owner,
    const void* actual_name, IDirect3DCubeTexture9* input_com, std::uint32_t flags,
    NativeCubeTextureOwnerContext&, NativeNamedSpecialTextureAcquired* = nullptr);

// Complete B3CFA0: ECX actual34h owner in38h volume slot, same arguments/RET.
// Install D618B0, clear source+30, query current INPUT COM GetLevelDesc(0),
// capture Width/Height/Depth before storing Height+28, Width+24, Depth+2C.
// Reload input table for GetLevelCount, then read Format before count/format
// stores at+14/+18. Keep slot index+34. EH state0 runs B340F0->B33F50 only.
void* construct_native_named_volume_texture_00b3cfa0(void* actual_owner,
    const void* actual_name, IDirect3DVolumeTexture9* input_com, std::uint32_t flags,
    NativeNamedVolumeTextureContext&, NativeNamedSpecialTextureAcquired* = nullptr);

// Both entry points require notification's ActualNativeStringPoolStorage to
// be the same provider used by construction and later existing destruction.
// Descriptor storage is not initialized and HRESULT is ignored, as native;
// a failed query leaving unwritten fields does not establish deterministic
// metadata. These are new Win32 C++ interfaces, not original binary ABI entries.
} // namespace bsp
