#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// 00770E50 borrows the caller's UTF-16 backing: length +0/4, data +4/4.
// The source and destination headers may alias; the temporary is separate.
struct WideStringSubstringHeader {
    std::uint32_t length;
    wchar_t* data;
};
static_assert(sizeof(wchar_t) == 2);
static_assert(offsetof(WideStringSubstringHeader, length) == 0);
static_assert(offsetof(WideStringSubstringHeader, data) == 4);
static_assert(sizeof(WideStringSubstringHeader) == 8);

// Only the five dependencies used by the existing sweepB2 substring body.
// Host storage must outlive both the temporary and the returned header.
struct WideStringSubstringHost {
    virtual ~WideStringSubstringHost() = default;
    virtual void wide_string_resize_004c53e0(void*, std::uint32_t, std::int32_t) = 0;
    virtual void* wide_string_copy_construct_004c8dd0(void*, const void*) = 0;
    virtual wchar_t* crt_wcsncpy_00bfaeef(wchar_t*, const wchar_t*, std::uint32_t) = 0;
    virtual void* sized_pool_get_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void*, void*, std::uint32_t, std::int32_t) = 0;
};

// The single existing body, now typed over the caller's header/storage.
// Native thiscall(source, out, position, count), RET 0Ch; Source ABI only.
// The pre-existing sweepB2 EH limitation is unchanged.
WideStringSubstringHeader* wide_string_substring_view_00770e50(
    const WideStringSubstringHeader& source, WideStringSubstringHeader& out,
    std::int32_t position, std::uint32_t count, WideStringSubstringHost& host);

} // namespace bsp
