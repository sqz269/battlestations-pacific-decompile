#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Complete 00B24A40..00B24AF7: original ECX actual renderer, stack stream/value,
// RET8. New C++ interface borrows the actual synchronization globals. Storage
// and index arithmetic are raw Win32 values; no stream validation is added.
void set_native_renderer_stream_frequency_00b24a40(void* actual_renderer,
    std::uint32_t stream, std::uint32_t value,
    NativeRendererSynchronizationGlobals& actual_synchronization);

} // namespace bsp
