#pragma once

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Full B19980/C30470: ECX actual storage, EAX same storage, RET. Write only
// the recovered words/byte, in order, retaining original profile identities.
// These are raw construction bodies, not C++ objects or callable vtables.
void* construct_native_texture_resource_base_00b19980(void* actual_storage) noexcept;
void* construct_native_texture_source_00c30470(void* actual_storage) noexcept;

// Full BBC6F0/BBC810: native ECX unused, no stack arguments, EAX resource,
// RET. Allocate exactly34h; call the two constructors above and write final
// D64478/D644B4. Other bytes remain uninitialized. No input factory is read.
// Native CC4B70/CC4BB0 free a captured allocation on constructor unwind.
// Valid raw-storage C++ construction cannot throw here; its optimizer may
// eliminate that cleanup. This interface does not emulate native SEH faults.
void* create_native_caustics_texture_source_00bbc6f0();
void* create_native_shore_wave_texture_source_00bbc810();

// Full B19E40: ECX unused; stack output/requested/options; EAX output; RET0C.
// Capture identity BEFORE zeroing both output words. Self-copy leaves the
// cleared header without freeing its previous block. Copy failure arms no
// new cleanup. Options are unused; all reached storage uses the actual pool.
// Capture both native data pointers, then omit a zero-byte memcpy as in the
// existing actual-header provider; valid nonzero copies require valid ranges.
void* copy_native_cache_requested_name_00b19e40(void* actual_output,
    const void* actual_requested, const void* unused_options,
    ActualNativeStringPoolStorage& actual_strings);

// Full4DDB20: ECX unused; stack resource; EAX captured resource; RET4.
// Actual InterlockedIncrement on resource+4, without null or profile checks.
void* retain_native_cache_resource_004ddb20(void* actual_resource) noexcept;

// Actual caller-owned tree: +4=head, head+4=root. Native1Ch nodes contain
// links+0/+4/+8, name length/data+C/+10, value+14, color+18, nil byte+19.
// No construction, population, ownership or snapshot of that storage.
// B19B90: ECX tree; stack key; EAX node; RET4.
void* lower_bound_native_resource_factory_00b19b90(void* actual_tree,
    const void* actual_key);
// B19D60: ECX tree; stack iterator-output/key; EAX output; RET8.
// Output is two DWORDs {owner,node}; capture both before the first store.
// Lower-bound precedes returning CRT validation. Miss rereads current head.
void* find_native_resource_factory_00b19d60(void* actual_tree,
    void* actual_iterator_output, const void* actual_key,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Valid actual Win32 backing storage and the existing callable CRT boundary
// are caller requirements. Host CRT, Win32 and actual-pool services remain
// explicit boundaries; original binary/SEH ABI and game behavior are unproved.
// B19E90 factory dispatch, registry lifetime/population, resource destruction
// and B1A4F0 are separate unfinished routes. No callback substitutes them here.
} // namespace bsp
