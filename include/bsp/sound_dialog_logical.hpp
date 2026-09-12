#pragma once
#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Required services for the independently reconstructed actual54h stream.
// A877D0 consumes the caller-constructed8h name and retained4h table argument
// on both normal return and unwind. It does not free the receiver allocation
// on failure: that is A783F0's state0 cleanup. No extra copy or retain on entry.
class SoundDialogLogicalDependencies {
public:
    virtual ~SoundDialogLogicalDependencies() = default;
    virtual void* construct_stream_00a877d0(void* actual_storage,
        NativeString& consumed_name, void* consumed_table) = 0;
    virtual void start_stream_00a867b0(void* actual_stream,
        const void* actual_name_header) = 0;
    virtual void set_stream_row_gain_00a86670(void* actual_stream,
        std::int32_t row, float gain) = 0;
    virtual void set_stream_gain_00a864f0(void* actual_stream, float gain) = 0;
    virtual void update_stream_00a874d0(void* actual_stream, float dt) = 0;
};
struct SoundDialogLogicalBindings {
    NativeStringStorage& strings;
    GameplayEffectComponentLifetime& references;
    void* volatile& global_00f8bbcc;
    SoundDialogLogicalDependencies& streams;
};

// Full normal bodies, new C++ interfaces over the actual constructor-produced
//5Ch logical and1Ch configuration storage. Strings remain actual8h headers;
// references use actual+4 Interlocked counts. No copied canonical state.
// ECX configuration, RET. Clears refs+10 and+18 after their callbacks; strings
// and gain survive. This is not the configuration destructor.
void clear_sound_dialog_configuration_references_00a77bb0(void*,
    GameplayEffectComponentLifetime&);
// ECX destination, stack source, EAX destination, RET4. String0, string8,
// publish/retain/release ref10, then gain14. Identity still loads/stores gain.
void* copy_sound_dialog_record_00a78230(void*, const void*,
    NativeStringStorage&, GameplayEffectComponentLifetime&);
// Same ABI: copy18h base first, then publish/retain/release ref18.
void* copy_sound_dialog_configuration_00a785d0(void*, const void*,
    NativeStringStorage&, GameplayEffectComponentLifetime&);

// ECX actual20h table, stack actual8h name, EAX signed index, RET4. Captures
// vector endpoints once; equal lengths precede current-CRT _stricmp. Missing
// name returns0, as does a match at row0. Requires a valid native row vector.
std::int32_t find_sound_dialog_record_00a865f0(void* actual_table,
    const void* actual_name_header) noexcept;

// Originally no arguments, ST0 float, RET; read CURRENT F8BBCC+218/+21C.
// Not yet Ghidra functions: RET at A7782B/A7783B, end exclusive2C/3C.
float sound_dialog_primary_gain_00a77820(void* volatile& global_00f8bbcc) noexcept;
float sound_dialog_secondary_gain_00a77830(void* volatile& global_00f8bbcc) noexcept;

// A783F0: ECX logical, no stack arguments, RET. Replaces a requested stream or
// restarts an existing state0/3 stream. Its actual by-value ctor arguments and
// EH cleanup are explicit in the dependency contract above.
void start_sound_dialog_logical_00a783f0(void*, SoundDialogLogicalBindings&);
// A78820: ECX logical, stack dt/gain, RET8. Rounds countdown before comparison;
// NaN does not expire. Retains x87 intermediate precision for the gain product.
// Slot0 dispatch covers the two existing A78150/A781C0 constructor profiles
// D58E60/D58E64. Other logical vtables are outside this interface's domain and
// raise logic_error instead of inventing a gain. No binary-ABI/gameplay claim.
void update_sound_dialog_logical_00a78820(void*, float dt, float gain,
    SoundDialogLogicalBindings&);
} // namespace bsp
