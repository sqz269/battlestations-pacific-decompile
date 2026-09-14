#pragma once

#include <cstdint>

namespace bsp {
// Complete C0DC9A[28] and C0DD00[23] native instruction entries, MSVC Win32.
// These adapters call the actual __local_unwind4 source provider with its
// three native stack words and inherited EBP. They supply no frame, cookie,
// scope table, native funclet, FS-chain or canonical mutable-data owner.
// Valid current native storage, the real DP/DJ/DK/CU provider chain and
// admission of the nested handler to the consuming image's SafeSEH table
// remain required. The source is not placed at the original code addresses.
//
// C0DC9A: entry ESP=S; the actual context pointer is [S+4]. After saving
// incoming EBP, load EBP=context[0], then read/push DWORDs at +1C, +18, +28
// in that order. They become target level, registration and cookie pointer
// respectively. The context is neither copied nor validated. Current x86
// _JUMP_BUFFER offsets match these observations; its original symbol/type
// has not been recovered. On normal return the adapter restores incoming
// EBP and removes its one context word (RET 4).
void __stdcall unwind_native_crt_context_00c0dc9a(const void* context);

// C0DD00 retains the confirmed _EH4_LocalUnwind library contract: ECX is
// registration, EDX is target level; [S+4] is the actual inherited frame and
// [S+8] is the actual scope-decoding cookie pointer. Load EBP from [S+4],
// push target/registration/cookie pointer, call the same real loop, restore
// incoming EBP and remove the two original stack words (RET 8). Fastcall's
// register words are not duplicated on the stack.
void __fastcall unwind_native_crt_eh4_local_00c0dd00(
    void* registration, std::uint32_t target_level, void* inherited_frame,
    const volatile std::uint32_t* scope_cookie);

// The loop's deliberate omission of its saved-EBP pop is retained; these
// adapters perform their own original EBP restore. EBX/ESI/EDI follow the
// real provider contract; EAX/ECX/EDX carry its raw normal-return effects.
// Final arithmetic flags are those of ADD ESP,0C; POP/RET do not change
// them. DF is neither normalized nor changed by these adapters. Reached
// native providers retain their own DF, frame, register and fault contracts.
// Native aliasing/read order is preserved, including any stack aliases;
// callers must supply storage valid for every reached access and frame.
// Faults, termination and nonlocal exits receive no catch, rollback or new
// return policy. Exact source/COFF/link evidence is not native exception or
// gameplay validation. See docs/NATIVE_CRT_LOCAL_UNWIND_ADAPTERS_EA.md.
} // namespace bsp
