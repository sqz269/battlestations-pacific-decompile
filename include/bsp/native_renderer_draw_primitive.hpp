#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Complete B21B40..B21BEE (175 bytes): native ECX renderer, stack primitive
// type/start vertex/primitive count, RET0C. Test DWORD+1D90 then byte+1D8A;
// call the complete viewport address helper on current+1904 before testing
// primitive count. Optional entry precedes current device/table+144 lookup.
// HRESULT is ignored; no draw counters are updated.
void draw_native_renderer_primitive_00b21b40(void* actual_renderer,
    std::uint32_t primitive_type, std::uint32_t start_vertex,
    std::uint32_t primitive_count,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);

// Borrows actual renderer/device/global storage, with the existing actual
// synchronization providers. Current mode is reread before normal leave and
// during source C++ unwind. The native skipped-entry record is uninitialized;
// changing mode to require that record after skipped entry is outside the valid
// source domain. No enum/index validation, device ownership or result policy is
// added. This new C++ interface does not reproduce the original private stack,
// FH3/SEH machinery, original caller-frame aliases or binary entry ABI.

} // namespace bsp
