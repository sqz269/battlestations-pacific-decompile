#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Original checked STL vector subscript specialization. The vector is the
// actual provider+1Ch header (begin/end/capacity at +4/+8/+Ch, records24h).
// Its invalid-parameter call may return and repair the current backing pointer.
// Keep the original library implementation/contract; no fallback is supplied.
class NativeMpakOpenLibrary {
public:
    virtual ~NativeMpakOpenLibrary() = default;
    virtual void* file_at_00bb4140(void* actual_vector, std::uint32_t index) = 0;
};

// Two explicitly bounded calls into other reconstructed packets. The same
// actual manager/provider/pool domain must be wired at integration.
class NativeMpakOpenDispatch {
public:
    virtual ~NativeMpakOpenDispatch() = default;
    // Original ECX captured manager, stack name/unused, RET8. BB5BB0 passes
    // the SAME temporary8h header for both stack arguments.
    virtual std::int32_t select_device_00bdd850(void* captured_manager,
        const void* actual_name, const void* unused_name) = 0;
    // Original ECX provider, stack signed index, RET4; negative -> null.
    virtual void* materialize_entry_00bb5080(void* actual_provider,
        std::int32_t file_index) = 0;
};

struct NativeMpakOpenContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    void* volatile& actual_manager_publication_0109ceec;
    NativeMpakOpenLibrary& library;
    NativeMpakOpenDispatch& dispatch;
};

// Full BB4A60..BB4B15[182]: ECX provider44h, stack name8h, RET4, EAX signed
// file index or -1. Custom game search over the actual file24h records, not an
// STL routine. Repeats current-vector bounds validation, allows the original
// invalid-parameter callback to return, then reloads begin before reading.
std::int32_t find_native_mpak_file_index_00bb4a60(void* actual_provider,
    const void* actual_name, const SingletonLifetimeCallbacks&);

// Full BB4B20..BB4B35[22]: same inputs, RET4, AL = signed search result >= 0.
bool contains_native_mpak_file_00bb4b20(void* actual_provider,
    const void* actual_name, const SingletonLifetimeCallbacks&);

// Full BB5BB0..BB5CCB[284]: ECX provider44h, stack name8h/flags, RET8, EAX
// stream. Reject flags bit0 before any provider/name access. Optionally resolve
// device using a copied name containing "_0000", release that copy, publish to
// CURRENT manager+18h, then check cached index only when signed +40h > 0.
// Lookup and materialization use the original name and actual provider.
void* open_native_mpak_file_00bb5bb0(void* actual_provider,
    const void* actual_name, std::uint32_t flags, NativeMpakOpenContext&);

// These are source interfaces, not binary replacements. Original STL and CRT,
// x86 SEH/FH3 unwinding, concurrently modified storage, and gameplay remain
// separate evidence domains. The actual pooled-string release is noexcept.
} // namespace bsp
