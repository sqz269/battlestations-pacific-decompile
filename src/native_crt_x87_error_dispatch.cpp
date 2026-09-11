#include "bsp/native_crt_x87_error_dispatch.hpp"
#include "bsp/legacy_crt_math.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT x87 error dispatch requires MSVC Win32 raw assembly.
#endif

namespace bsp {

static_assert(sizeof(CameraAxesCrtException) == 32);
static_assert(offsetof(CameraAxesCrtException, type) == 0);
static_assert(offsetof(CameraAxesCrtException, name) == 4);
static_assert(offsetof(CameraAxesCrtException, argument1) == 8);
static_assert(offsetof(CameraAxesCrtException, argument2) == 16);
static_assert(offsetof(CameraAxesCrtException, result) == 24);

// Full physical union C08330..C08383, including both original entry prologues
// and one shared tail. Only the direct C27489 CALL operand is rebound to its
// completed concrete source provider. No context, callback, guard or snapshot.
__declspec(naked) void __cdecl dispatch_native_crt_binary_error_00c08330() {
    __asm {
        push ebp // 00c08330
        mov ebp, esp // 00c08331
        add esp, -20h // 00c08333
        mov dword ptr [ebp - 20h], eax // 00c08336
        mov eax, dword ptr [ebp + 18h] // 00c08339
        mov dword ptr [ebp - 10h], eax // 00c0833c
        mov eax, dword ptr [ebp + 1ch] // 00c0833f
        mov dword ptr [ebp - 0ch], eax // 00c08342
        jmp shared_tail // 00c08345
        // Original unary entry C08347 is exactly at this function's +17h.
        push ebp // 00c08347
        mov ebp, esp // 00c08348
        add esp, -20h // 00c0834a
        mov dword ptr [ebp - 20h], eax // 00c0834d
    shared_tail:
        fstp qword ptr [ebp - 8] // 00c08350
        mov dword ptr [ebp - 1ch], ecx // 00c08353
        mov eax, dword ptr [ebp + 10h] // 00c08356
        mov ecx, dword ptr [ebp + 14h] // 00c08359
        mov dword ptr [ebp - 18h], eax // 00c0835c
        mov dword ptr [ebp - 14h], ecx // 00c0835f
        lea eax, [ebp + 8] // 00c08362
        lea ecx, [ebp - 20h] // 00c08365
        push eax // 00c08368
        push ecx // 00c08369
        push edx // 00c0836a
        call legacy_crt_87except_00c27489 // 00c0836b
        add esp, 0ch // 00c08370
        fld qword ptr [ebp - 8] // 00c08373
        cmp word ptr [ebp + 8], 27fh // 00c08376
        jz finished // 00c0837c
        fldcw word ptr [ebp + 8] // 00c0837e
    finished:
        leave // 00c08381
        ret // 00c08382
    }
}

// Explicit new unary entry thunk; reaches the original unary prologue and
// shared tail inside the physical union without pushing a return address.
__declspec(naked) void __cdecl dispatch_native_crt_unary_error_00c08347() {
    __asm {
        jmp dispatch_native_crt_binary_error_00c08330 + 17h
    }
}

} // namespace bsp
