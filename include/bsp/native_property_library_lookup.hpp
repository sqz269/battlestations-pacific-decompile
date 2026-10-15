#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native property library lookup requires MSVC Win32.
#endif

namespace bsp {

// Complete ordinary readers with new C++ interfaces, not Original stack/
// register ABI bindings. Borrow actual Win32 eight-byte headers (length0,
// data4), an actual108h map and finite correctly bucketed14h node chains.
// Keys are stable closed NUL-free ASCII with matching lengths, null data only
// when empty, and the current CRT C locale. Supply distinct canonical stable
// NUL roles: GameNativeStringProcess::library_empty_00e18560() and its existing
// query_empty_00e17654(). Never substitute another empty cell or construct a
// NativeString C++ object over the raw header. Keep all storage/bindings live;
// the writable bucket output is nonaliasing. No concurrent mutation/reentry,
// invalid memory, arbitrary locale, allocation, ownership, class/game ABI or
// exceptional/fault transport is supplied by these readers.

// Whole [004689D0,00468A29),89 bytes/39 instructions; Original stack header,
// EAX bucket,RET4. Read physical data4 BEFORE length0. Hash moduloDWORD with
// signed-byte promotion through real current CRT toupper, retaining its low
// byte. Subtract (length|20h)>>5 from a separate counter but advance data by
// ONE byte. Null data selects E18560; empty input never dereferences that cell.
std::uint32_t native_property_library_key_bucket_004689d0(
    const void* actual_query_header, const char* actual_empty_00e18560);

// Whole [00468ED0,00468F2D),93 bytes/41 instructions; Original ECX map,
// stack query and bucket pointer,RET8. Publish bucket BEFORE physical head
// load. Read query length only after a nonnull first head, then compare node
// length before data. Each matching-length candidate rereads query data with
// the E17654 fallback (00419CA0 rule), then calls whole00438E10 comparison.
// Return the first matching mutable borrowed node or null. Never read count,
// mapped+8 or allocator+10; only header0/4, head8+4*bucket and node next+C.
void* find_native_property_library_node_00468ed0(const void* actual_map,
    const void* actual_query_header, std::uint32_t& bucket,
    const char* actual_empty_00e18560, const char* actual_query_empty_00e17654);

} // namespace bsp
