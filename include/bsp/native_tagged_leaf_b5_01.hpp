#pragma once

// Original bodies at these addresses consist solely of RET. They consume no
// input and prescribe no result. __cdecl(void) is the new Source interface;
// an original class, formal signature, and vtable slot are not recovered.
extern "C" void __cdecl bsp_tagged_b5_00b540a0() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b542b0() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b544e0() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b546e0() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b548b0() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b5fd50() noexcept;
extern "C" void __cdecl bsp_tagged_b5_00b5fd60() noexcept;
