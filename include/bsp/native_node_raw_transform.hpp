#pragma once

#include <cstdint>

namespace bsp {

class NativeTracelineRenderServices;

void __fastcall invalidate_raw_descendants_00b6da30(void* actual);
void __fastcall notify_raw_bounds_00b6dbc0(
    void* actual, NativeTracelineRenderServices* profiles);
void __fastcall set_raw_local_matrix_00b6db10(
    void* actual, NativeTracelineRenderServices* profiles, const void* source);

} // namespace bsp
