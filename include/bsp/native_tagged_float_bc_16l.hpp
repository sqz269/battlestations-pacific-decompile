#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These raw x87 getters require MSVC Win32.
#endif

// New ECX-backing / x87-ST(0) Source interfaces, not recovered Original class
// or float-versus-double declaration spellings. Each complete body performs
// FLD m32real in the inherited x87 environment, then RET. No bit normalization,
// environment reset, stack-empty guard or extra read is introduced.
extern "C" float __fastcall bsp_tagged_float_bc_16l_00bc3010(const void* backing); // +2Ch
extern "C" float __fastcall bsp_tagged_float_bc_16l_00bc7aa0(const void* backing); // +34h
extern "C" float __fastcall bsp_tagged_float_bc_16l_00bc7ab0(const void* backing); // +1Ch
