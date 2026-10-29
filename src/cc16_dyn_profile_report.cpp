#include "bsp/cc16_dyn_profile_report.hpp"

#include "bsp/cc15_dyn_live.hpp"
#include "bsp/native_legacy_sbo_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This native register/x87 reconstruction requires MSVC Win32.
#endif

// Genuine MSVC x86 compiler-runtime symbol __alldiv, present in /MD msvcrt.lib.
// ASM ONLY: four stack dwords, callee RET16, quotient EDX:EAX. This declaration
// selects its C symbol; it is not an ordinary cdecl C++ callable interface.
// No vendor implementation or fallback is supplied here.
extern "C" void __cdecl _alldiv();

namespace bsp::cc16_dyn_profile_report {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(DynProfileNodeStorage) == 0x48);
static_assert(0x10 + offsetof(NativeLegacySboStringStorage, buffer_04) == 0x14);
static_assert(0x10 + offsetof(NativeLegacySboStringStorage, capacity_18) == 0x28);

constexpr char kReportFormat[] = "%s: Calls:%d Time:%d Perc:%.2f%%%%";

U32 address(const void* p) noexcept { return reinterpret_cast<U32>(p); }
volatile U32& word(U32 p) noexcept { return *reinterpret_cast<volatile U32*>(p); }
}  // namespace

using cc15_dyn_live::format_string_1024_0040cfc0;

void __cdecl report_profile_node_00c320d0(
    DynProfileNodeStorage& node, DynProfileReportSink sink,
    volatile U32& first_position, volatile U32& second_position,
    const DynProfileScopeContext& profile_context,
    const volatile double& actual_percent_scale_00d7a220) {
    char text[0x400];  // Native uninitialized local formatting buffer.
    std::uint64_t captured_tick_bits;
    DynProfileNodeStorage* const actual_node = &node;
    void* const* const actual_profile_slot = profile_context.profile_slot_0109e9f8;
    const volatile double* const actual_scale = &actual_percent_scale_00d7a220;
    volatile U32* const actual_first = &first_position;
    volatile U32* const actual_second = &second_position;

    __asm {
        // Root/denominator pointer capture precedes the name-capacity decision.
        mov edx, actual_profile_slot
        mov edx, dword ptr [edx]
        mov edx, dword ptr [edx + 10h]
        mov esi, actual_node
        mov ebx, actual_scale
        cmp dword ptr [esi + 28h], 10h
        mov eax, dword ptr [esi + 38h]
        mov ecx, dword ptr [esi + 3ch]
        mov dword ptr [captured_tick_bits], eax
        mov dword ptr [captured_tick_bits + 4], ecx
        fild qword ptr [captured_tick_bits]
        fmul qword ptr [ebx]
        fild qword ptr [edx + 38h]
        fdivp st(1), st(0)
        // CF is still from the BEFORE-FP unsigned capacity comparison.
        jc report_inline_name
        mov edi, dword ptr [esi + 14h]
        jmp report_name_ready
    report_inline_name:
        lea edi, [esi + 14h]
    report_name_ready:
        // Exactly one floating spill, directly to the printf argument slot.
        // Keep it across real integer division; do not reload it through FP.
        sub esp, 8
        fstp qword ptr [esp]
        push 0
        push 1000
        push ecx
        push eax
        call _alldiv
        // Native consumes only low quotient bits as %d, then reloads calls.
        push eax
        mov eax, dword ptr [esi + 40h]
        push eax
        push edi
        push offset kReportFormat
        lea eax, text
        push eax
        call format_string_1024_0040cfc0
        // Preserve the coordinate/argument-stack epoch too: native still has
        // the printf args while incrementing second and loading second/first.
        mov ebx, actual_second
        add dword ptr [ebx], 14h
        mov eax, actual_first
        mov edx, dword ptr [ebx]
        mov ecx, dword ptr [eax]
        add esp, 1ch
        lea eax, text
        push eax
        call sink  // Genuine fastcall-compatible sink pops its text argument.
    }

    const U32 base = address(&node);
    U32 cursor = word(base + 4u);             // C3215E: after the sink.
    U32 count = word(base + 8u);              // Initial data-before-count.
    U32 end = cursor + count * 4u;
    while (cursor != end) {                  // Equality, not a count/less test.
        U32 child_first = first_position + 20u;
        auto* const child = reinterpret_cast<DynProfileNodeStorage*>(word(cursor));
        report_profile_node_00c320d0(*child, sink, child_first, second_position,
                                    profile_context, actual_percent_scale_00d7a220);
        count = word(base + 8u);             // C3218F: count BEFORE data.
        const U32 data = word(base + 4u);     // C32192.
        cursor += 4u;
        end = data + count * 4u;
    }
}

}  // namespace bsp::cc16_dyn_profile_report
