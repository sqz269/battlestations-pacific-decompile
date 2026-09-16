#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Complete263B AF45D0: ECX actual90h resource; stack raw8h name; EAX owner,
// RET4. Initialize only the native fields, deep-copy the name through the
// actual raw pool, preserve all other bytes, and honor name/base EH cleanup.
// The three immutable image defaults are encoded as their verified DWORD bits.
void* construct_native_particle_resource_00af45d0(
    void* actual_resource, const void* actual_name, NativeStringRawPoolContext&);

// Complete50B AF5620: ECX actual1Ch text buffer, RET. Free nonnull buffer+14,
// then return CURRENT raw name+C. Preserve the stale header and buffer fields.
void destroy_native_particle_text_buffer_00af5620(
    void* actual_text_buffer, NativeStringRawPoolContext&);

// Genuine loading prerequisites. AF5850 loading, AF4BA0 parsing and the full
// resource-loader virtual function remain separate dependencies. These source
// interfaces do not provide original thiscall/FH3/SEH or runtime compatibility.
} // namespace bsp
