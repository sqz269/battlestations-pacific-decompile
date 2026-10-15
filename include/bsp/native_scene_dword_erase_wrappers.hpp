#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD erase wrappers require MSVC Win32.
#endif

namespace bsp {

// Explicit ordinary Source backing for the native private output and child
// argument cells. Both regions and this binding metadata are stable/disjoint;
// actual control/payload aliases retain the documented reached-access schedule.
// Do not zero-initialize output. Only reached backing accesses must be valid.
// This is not a claim that these buffers occupy compiler/native private frames.
struct NativeSceneDwordEraseWrappersFrame {
    void* actual_output8;
    void* actual_child_parameters20;
};

// Complete 00468AF0..00468B28: native ECX control, no stacked words, plain RET;
// EDX unused, EBX/ESI/EDI preserved. Capture initial end; compare current begin,
// report if unsigned begin>end. Then capture fresh begin, compare fresh end,
// report again if needed. Preserve original end/current captured begin across
// both returnable reports. Publish last_node,last_owner,first_node,first_owner,
// output into child20 in PUSH order, then perform the real 004685A0 Source.
// Return its actual current output-pointer result, without an owner/lifetime
// interpretation or assumption that it still selects actual_output8.
void* clear_native_scene_dword_vector_00468af0(void* actual_control,
    const NativeSceneDwordEraseWrappersFrame&);

// Complete 00468C90..00468CC9: same native ABI and payload schedule, with the
// captured control equal to receiver+8 modulo32. No nominal receiver class is
// imposed. Both native wrappers independently target BF6713 and 004685A0;
// sharing ordinary Source mechanics does not create another native edge.
void* clear_native_scene_dword_vector_embedded8_00468c90(void* actual_receiver,
    const NativeSceneDwordEraseWrappersFrame&);

// Native entryESP=P has output8 at P-8, child20 at P-40, return slot P-44.
// Saved-register/helper-frame coincidences, Source/native argument homes,
// spill/return/EH aliases, asynchronous mutation and native dangling-frame
// return lifetime are not supplied. Current CRT/SDK and explicit raw backing
// are admitted boundaries; original ABI/CRT/fault/SEH/game identity is unproved.

} // namespace bsp
