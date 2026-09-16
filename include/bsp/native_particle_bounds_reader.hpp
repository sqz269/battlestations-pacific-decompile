#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete AF4700[1176]. Native ECX actual destination (writes five float
// fields +7C,+80,+84,+88,+8C); stack actual1Ch text buffer; RET4. Borrow the
// application's SAME raw string-pool/manager/gate cells and shared F8C2C8
// scratch used by AF5740. No separate owner, allocator or parser is supplied.
//
// Scan until BoundSphere, null token or EOF, then search for an exact "{" line
// and process lines until exact "}" or EOF. Param/Coords uses the native space
// count and suffix/token helpers, with no five-token validation or rollback.
// Each atof ST0 result is stored directly to its binary32 destination before
// that token's current pool return. Other destination bytes remain untouched.
// Native missing/null tokens, scratch aliases and malformed inputs receive no
// sanitization. Current CRT parsing/comparison remains a provider boundary.
//
// True native unwind states own only line, conditional first scan token,
// parameter name and suffix. Keyword/coordinate temporaries are not added to
// cleanup. Normal getter failures propagate; secondary unwind failures
// terminate. This source interface does not preserve native stack-slot aliases,
// register calling convention, hardware SEH or gameplay compatibility.
void read_native_particle_bounds_00af4700(void* actual_destination,
    void* actual_text_buffer, NativeStringRawPoolContext& strings,
    char* actual_shared_scratch_00f8c2c8);
} // namespace bsp
