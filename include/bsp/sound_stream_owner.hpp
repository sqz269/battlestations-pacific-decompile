#pragma once
#include "bsp/sound_dialog_table.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct GameplayEffectComponentLifetime;

// Actual native storage, without implicit construction or destruction. +10 is
// unwritten by A877D0. +24/+2C/+34 are strings; +3C is a retained table; +40/+44
// belong to runtime FMOD state; +48 is a 0Ch vector of 0Ch control records.
struct alignas(4) NativeSoundStreamStorage { std::byte bytes[0x54]; };
struct alignas(4) NativeSoundStreamTableStorage { std::byte bytes[0x20]; };
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeSoundStreamStorage) == 0x54);
static_assert(sizeof(NativeSoundStreamTableStorage) == 0x20);

class SoundStreamOwnerHost {
public:
    virtual ~SoundStreamOwnerHost() = default;
    // BDF4C0 normalizes/resolves the mutable actual8h name through the live VFS.
    // The original caller ignores AL, including failed lookup with mutation.
    virtual void resolve_stream_name_00bdf4c0(void* actual_name) = 0;
    // Required actual54h stop body, including current FMOD release/state writes.
    virtual void stop_stream_00a86bf0(void* actual_stream) = 0;
};
struct SoundStreamOwnerContext {
    SoundDialogTableContext& table; // sole string allocator is table.strings
    GameplayEffectComponentLifetime& references;
    SoundStreamOwnerHost& host;
};

// ECX actual54h; stack owned8h name then retained4h table; EAX=this, RET0C.
// Consumes BOTH incoming owned arguments on success and exception, preserving
// their native dangling header bytes. The caller must not destroy them again.
// Members copy the name and retain a supplied table exactly as native. Null
// table allocates/loads an actual20h table from resolved-name-minus4 + ".def".
// Full normal and native member/argument EH ordering in the documented Win32
// throwing-allocation domain; this interface is not a binary ABI replacement.
void* construct_sound_stream_00a877d0(void* actual_stream,
    NativeString& owned_name, void* owned_table, SoundStreamOwnerContext&);
void destroy_sound_stream_00a87390(void* actual_stream, SoundStreamOwnerContext&);
void* scalar_delete_sound_stream_00a87b30(void* actual_stream,
    std::uint8_t flags, SoundStreamOwnerContext&);

// ECX actual20h; stack borrowed filename header; EAX=this, RET4. Preserves
// padding +1D..1F. Loader failure destroys the current vector and root base.
void* construct_sound_stream_table_00a79150(void* actual_table,
    const NativeString& filename, SoundDialogTableContext&);

// Actual0Ch vector header {data,signed count,signed capacity}. Reserve copies
// all three words forward; resize writes only floats +0/+4 of newly added
// records, leaving +8 untouched. No copied header, hidden ownership or bounds
// fallback. Destruction resizes0 then frees current data, leaving its bytes.
void reserve_sound_stream_controls_00a86540(void* actual_vector, std::int32_t capacity);
void resize_sound_stream_controls_00a868b0(void* actual_vector, std::int32_t count);
void destroy_sound_stream_controls_00a86990(void* actual_vector);
} // namespace bsp
