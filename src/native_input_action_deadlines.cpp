#include "bsp/native_input_action_deadlines.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input action deadlines require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
template<class T> T read(void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}

bool edge(void* record) noexcept {
    std::uint8_t result;
    // Current must be ordered-positive; previous only blocks an edge when
    // ordered-positive. In particular, unordered previous permits an edge.
    __asm {
        mov eax, record
        cmp byte ptr [eax+28h], 0
        jz no_edge
        movss xmm1, dword ptr [eax+24h]
        xorps xmm0, xmm0
        comiss xmm1, xmm0
        jbe no_edge
        cmp byte ptr [eax+20h], 0
        jz has_edge
        movss xmm1, dword ptr [eax+1ch]
        comiss xmm1, xmm0
        ja no_edge
    has_edge:
        mov result, 1
        jmp edge_done
    no_edge:
        mov result, 0
    edge_done:
    }
    return result != 0;
}
} // namespace

void record_native_input_action_deadlines_004d8cd0(void* game,
    NativeInputActionDeadlineContext& context) {
    constexpr std::int32_t indices[] = {
        0x4a, 0x4b, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
        0x54, 0x55, 0x46, 0x47, 0x4c, 0x4d, 0x57, 0x58
    };
    for (const auto index : indices) {
        void* const owner = get_native_input_action_owner_004bec00(context.actions);
        void* const record = at(read<void*>(owner, 4),
            static_cast<std::uint32_t>(index) * 0x30u);
        if (!edge(record)) continue;

        float deadline;
        const volatile double* const delay = &context.delay_00ce65d0;
        __asm {
            mov eax, game
            fld dword ptr [eax+64ch]
            mov eax, delay
            fadd qword ptr [eax]
            fstp deadline
        }
        const std::int32_t key = index;
        float* const destination = context.calls.call_004d6900(
            context.map_header_00e18a7c, &key);
        // The native binary32 spill precedes subscript; this separate x87
        // load/store uses the returned address even if the provider mutated
        // game clock, delay, action publication or later action storage.
        __asm {
            mov eax, destination
            fld deadline
            fstp dword ptr [eax]
        }
    }
}

void invoke_native_input_action_deadline_callback_006965a0(
    void* volatile& game_publication_00e188a8,
    NativeInputActionDeadlineContext& context) {
    void* const game = game_publication_00e188a8;
    record_native_input_action_deadlines_004d8cd0(game, context);
}
} // namespace bsp
