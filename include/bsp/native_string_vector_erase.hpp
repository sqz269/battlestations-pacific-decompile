#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector erase requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 004954F0..0049556D. Native ECX=actual10h vector, EDX unused,
// stack=(output,first_owner,first,last_owner,last), RET14h, EAX=output.
// Source keeps those physical inputs and adds the actual raw-pool context.
// Report only null first_owner or unequal owners; a returning CRT report
// continues. The receiver need not equal either owner and no range/bounds
// validation is added. Owners are opaque pointer values, not objects to read.
// Empty first==last still validates and writes output {first_owner,first},
// without reading header+8, copying, destroying or publishing the header end.
// Otherwise capture old end; assign [last,old_end) onto first; read CURRENT
// header end after copy for destruction; derive new end from captured pointers
// with wrapping SAR3 arithmetic; destroy trailing actual8h string headers;
// publish captured new end, THEN output owner+0 followed by first+4.
// Output may alias actual vector/header/data storage; its final store order
// and return address are retained. No synthetic iterator or allocator exists.
void* __fastcall erase_native_string_header_vector_range_004954f0(void* header,
    std::uint32_t unused_edx, void* output, const void* first_owner, void* first,
    const void* last_owner, void* last, NativeStringRawPoolContext& strings);

// Complete 00498FF0..00499028. Native ECX=header, no meaningful EDX/stack
// input, plain RET, no semantic result. Source uses EDX for its extra context.
// Capture end, compare current begin to it, report/continue; capture current
// begin, compare to fresh end, report/continue. Delegate to the exact erase
// above with both owner pointers=header and those captured endpoints. A private
// two-DWORD output matches the native local result; no backing free occurs.
void __fastcall clear_native_string_header_vector_00498ff0(void* header,
    NativeStringRawPoolContext& strings);

// Concrete accepted0048DA50 assignment,00432050 raw-pool destruction and
// BF6713 current-CRT report provider are reused. Exceptions escape; no catch,
// rollback, noexcept/noreturn or error conversion is added. Existing copy-zero,
// actual pool/manager/CRT policy and valid taken backing requirements apply.
// Native argument slots are reread at several sites; Source by-value pointers
// do not expose native private-frame mutation/aliasing. The extra context and
// private result also make these new Source APIs, not original ABI/FH3/SEH or
// fault-address replacements. The earlier storage fragment remains separate.

} // namespace bsp
