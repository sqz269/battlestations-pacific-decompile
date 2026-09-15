#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-entry cache requires MSVC Win32.
#endif

namespace bsp {
// Actual raw14h owner: profile00, record pointer04, used count08, capacity0C,
// tracked critical-section pointer10. Records occupy28h bytes. Borrow the
// application's existing cells/literal; no replacement singleton or container.
struct NativeRenderEntryCacheContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_cache_0108fe88;
    const volatile std::uint32_t& actual_one_00d7a24c;
};

// BEBF00[39]: native ECX record, EAX same record, RET. EDX is a new source
// binding to the actual one literal. Set00/14=+0, capture one, clear08/0C/10,
// store captured one at18; leave04/1C/20/24 untouched. Preserve SSE store order.
void* __fastcall initialize_native_render_entry_00bebf00(
    void* actual_28h_record, const volatile std::uint32_t* actual_one_00d7a24c);

// BEC240[10]: native ECX raw0Ch array header, RET. Free current data pointer;
// leave all header fields untouched. No element destruction occurs.
void destroy_native_render_entry_array_00bec240(void* actual_header) noexcept;

// BEC3E0[154]: native ECX raw0Ch header, stack requested DWORD, RET4.
// Compare requested with USED count+4, not capacity+8. On inequality clear
// count/capacity, free old data, then allocate requested*28h saturated at FFFFFFFF
// on unsigned overflow. Initialize records through the signed403560 iteration
// schedule. Zero request retains the stale data pointer; count stays zero.
void resize_native_render_entry_array_00bec3e0(void* actual_header,
    std::uint32_t requested, const volatile std::uint32_t& actual_one_00d7a24c);

// BEC590[145]/BEC630[153]: native ECX owner, no stack arguments, RET.
// Constructor returns original owner. Publication/register and removal/clear
// use live actual cells and the first manager's captured section throughout.
void* publish_native_render_entry_cache_base_00bec590(
    void* actual_owner, NativeRenderEntryCacheContext& context);
void destroy_native_render_entry_cache_base_00bec630(
    void* actual_owner, NativeRenderEntryCacheContext& context);

// BEC6F0[30]: native ECX owner, flags stack slot, EAX original owner, RET4.
void* delete_native_render_entry_cache_base_00bec6f0(void* actual_owner,
    std::uint32_t flags, NativeRenderEntryCacheContext& context);

// BEC870[110]: native ECX raw14h owner, EAX original owner, RET. Capture old
// count08, clear data04, and only for nonzero old count clear08/0C and free the
// now-current data pointer. Old capacity survives when old count is zero.
// Allocate the actual tracked section last. FH3 failure cleanup frees current
// array storage before base removal once the array state has been armed.
void* construct_native_render_entry_cache_00bec870(
    void* actual_owner, NativeRenderEntryCacheContext& context);

// BEC8E0[37]/BEC910[58]: native ECX owner; destructor RET, scalar delete
// consumes flags/RET4 and returns original owner. Restore derived profile,
// release section, free current record array, then destroy base; flags&1 frees
// the owner. No new cleanup is added if one of those operations fails.
void destroy_native_render_entry_cache_00bec8e0(
    void* actual_owner, NativeRenderEntryCacheContext& context);
void* delete_native_render_entry_cache_00bec910(void* actual_owner,
    std::uint32_t flags, NativeRenderEntryCacheContext& context);

// BECEE0 fragment BED1E8..BED222[59], after device startup. Allocate raw14h,
// construct when nonnull, then reread the actual publication and resize its
// header to10000. Constructor failure frees the captured owner after constructor
// cleanup. Disarm that owner-free state before resize; resize failure retains
// the registered owner. A null allocation still follows the publication path.
// This source helper is not a separate original function or all of BECEE0.
void initialize_native_window_render_entry_cache_00bed1e8_fragment(
    NativeRenderEntryCacheContext& context);

// New context interfaces are not original register/stack/FH3/SEH ABI. Numeric
// profiles are identity values, not host vtables. C++ unwind follows reviewed
// states; second cleanup exceptions terminate. Invalid storage, hardware-fault
// unwind and concurrent mutation are outside the proved domain. Reusing a stale
// pointer after resize(0), or exceeding capacity on producer increment, receives
// no invented safety policy. Generic singleton-deletion routing is separate.
} // namespace bsp
