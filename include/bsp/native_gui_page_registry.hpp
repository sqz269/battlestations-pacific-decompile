#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI page registry requires MSVC Win32.
#endif

namespace bsp {
struct NativeGuiPageLifetimeHost;

// 00AA30C0, original ECX actual 88h manager, stack actual page, RET4.
// Remove the first identical pointer from manager+14's raw pointer vector;
// shift following slots, then reduce its end. No page virtual or refcount call.
void remove_native_gui_page_pointer_00aa30c0(void* actual_manager, void* actual_page);

// 00AA52A0, original ECX actual manager, stack actual 124h page, RET4.
// Remove a prior copy of this exact pointer, then insert by signed page+FCh
// priority before the first greater entry. Equal priorities keep arrival order.
// Uses the already-recovered raw pointer-vector insert helper for middle/growth;
// never accepts GuiLayoutPage or any typed projection as actual_page.
void register_native_gui_page_00aa52a0(void* actual_manager, void* actual_page);

// 00AA31F0, original ECX actual manager, stack actual page, RET4. If found,
// remove its first pointer before calling its CURRENT virtual+20, then reload
// the page's CURRENT virtual+04 with deleting flag1. The supplied provider has
// one method for each native call and must resolve the actual page at call time.
// This routine does not decrement page+4's reference count; callers decide
// whether to call it or perform the separate InterlockedDecrement path.
void remove_and_destroy_native_gui_page_00aa31f0(void* actual_manager,
    void* actual_page, NativeGuiPageLifetimeHost& actual_page_virtuals);

// These are source interfaces, not original binary ABIs. Raw storage must be
// the manager constructed by00AA5D70, and page storage must be a live native
// page profile. The original invalid-parameter function is represented by the
// current CRT's handler; FH3/SEH, malformed storage and gameplay are unproved.
} // namespace bsp
