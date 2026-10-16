#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This native x87 leaf requires MSVC Win32.
#endif

// Complete native body 00B40040..00B40046. Provisional address-based name.
// ECX is actual backing storage, read as m32real at +0x1f8; no EDX or stacked
// argument is consumed. The single FLD pushes its result onto x87 ST0 and RET
// consumes only the actual return address. No object layout is inferred.
// The naked definition preserves the native load and inherited x87 state;
// this declaration describes the floating result without an extra store/load.
extern "C" float __fastcall bsp_tagged_x87_b4_00b40040(const void* backing);
