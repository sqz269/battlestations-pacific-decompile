#include "bsp/native_session_target_lifetime.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session target lifetime reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
NativeSessionTargetHistoryStorage* __fastcall history_scalar_bridge(
    NativeSessionTargetHistoryStorage* history, void*, std::uint32_t flags) {
    return scalar_delete_native_session_target_history_00783970(history, flags);
}
}
const NativeSessionTargetHistoryProfile native_session_target_history_profile_00d04268{
    &history_scalar_bridge};

NativeSessionTargetStorage* construct_native_session_target_007839d0(
    NativeSessionTargetStorage* target, const void* original_metadata,
    std::uint32_t original_length, const NativeSessionTargetLifetimeBindings& bindings) {
    volatile auto& t = *target;
    t.profile_00 = 0x00d0426cu;
    // Native unwind state0 starts only AFTER this member returns successfully.
    construct_native_session_transport_buffers_00783080(&target->buffers_10);
    try {
        t.field_d72 = 0;
        t.field_d70 = 0;
        t.section_04 = create_native_tracked_critical_section_00bd1860();
        if (original_metadata == nullptr) {
            t.metadata_08 = nullptr;
        } else {
            void* const metadata = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x98, 0x98});
            t.metadata_08 = metadata;
            std::memmove(metadata, original_metadata, original_length); // BF7680
        }
        t.field_d50 = 0;
        auto* first = static_cast<NativeSessionTargetHistoryStorage*>(
            singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, 0x18}));
        try {
            if (first != nullptr) {
                std::uint32_t zero_word;
                __asm { fldz }
                __asm { fstp dword ptr zero_word }
                static_cast<volatile NativeSessionTargetHistoryStorage*>(first)->profile_00 =
                    reinterpret_cast<std::uint32_t>(&native_session_target_history_profile_00d04268);
                initialize_native_session_target_history_00782f40(first, 50, zero_word);
            }
        } catch (...) {
            singleton_lifetime_free(first); // state1: only current 18h object
            throw;
        }
        t.history_d54 = first;
        auto* second = static_cast<NativeSessionTargetHistoryStorage*>(
            singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, 0x18}));
        try {
            if (second != nullptr) {
                volatile auto& h = *second;
                h.count_04 = 3;
                h.profile_00 = reinterpret_cast<std::uint32_t>(
                    &native_session_target_history_profile_00d04268);
                h.initial_float_word_08 = 0;
                auto* const array = static_cast<std::uint32_t*>(singleton_lifetime_allocate(
                    {SingletonAllocationKind::object, 12, 12})); // BF55BE -> BF681B
                const double* const three = &bindings.three_d7a2b0;
                const double* const zero = &bindings.zero_d7a258;
                // Preserve the live x87 result across publication and all three
                // sample stores: returned array first, then two fresh reloads.
                __asm {
                    mov edx, second
                    mov eax, array
                    mov ecx, three
                    fld qword ptr [ecx]
                    xorps xmm0, xmm0
                    mov ecx, zero
                    fmul qword ptr [ecx]
                    mov dword ptr [edx + 0ch], eax
                    movss dword ptr [eax], xmm0
                    mov eax, dword ptr [edx + 0ch]
                    movss dword ptr [eax + 4], xmm0
                    mov ecx, dword ptr [edx + 0ch]
                    movss dword ptr [ecx + 8], xmm0
                    fstp dword ptr [edx + 14h]
                    mov dword ptr [edx + 10h], 0
                }
            }
        } catch (...) {
            singleton_lifetime_free(second); // state2: only current 18h object
            throw;
        }
        const float* const timeout = &bindings.timeout_ce4bc4;
        const float* const negative_one = &bindings.negative_one_d7a260;
        // Native timeout store and negative-one load precede D58 publication.
        __asm {
            mov eax, target
            mov ecx, timeout
            mov edx, negative_one
            xorps xmm0, xmm0
            movss xmm1, dword ptr [ecx]
            movss dword ptr [eax + 0d60h], xmm1
            movss xmm1, dword ptr [edx]
            mov ecx, second
            mov dword ptr [eax + 0d58h], ecx
            movss dword ptr [eax + 0d5ch], xmm0
            mov dword ptr [eax + 0ch], 7
            movss dword ptr [eax + 0d64h], xmm1
            movss dword ptr [eax + 0d68h], xmm0
        }
        const std::uint16_t captured_mask = bindings.slots_f871b0;
        std::uint32_t index = 0;
        std::uint32_t bit = 1;
        while ((captured_mask & bit) != 0 && index < 16) {
            ++index;
            bit += bit;
        }
        if (index == 16) index = 0xffffffffu;
        else {
            volatile std::uint16_t* const slots = &bindings.slots_f871b0;
            // The native instruction modifies the fresh word, not the scan's
            // captured value; retain its single unprefixed memory RMW.
            __asm {
                mov ecx, slots
                mov eax, bit
                or word ptr [ecx], ax
            }
        }
        t.index_d6c = index;
        t.field_d4c = 0;
    } catch (...) {
        destroy_native_session_transport_buffers_007830f0(&target->buffers_10);
        throw;
    }
    return target;
}

void destroy_native_session_target_00783b90(NativeSessionTargetStorage* target,
    volatile std::uint16_t& actual_slots_f871b0) {
    volatile auto& t = *target;
    t.profile_00 = 0x00d0426cu;
    const std::uint32_t index = t.index_d6c;
    try {
        if (index <= 15) {
            volatile std::uint16_t* const slots = &actual_slots_f871b0;
            __asm {
                mov ecx, index
                mov eax, 1
                shl eax, cl
                not eax
                mov edx, slots
                and word ptr [edx], ax
            }
        }
        if (void* const metadata = t.metadata_08) {
            singleton_lifetime_free(metadata);
            t.metadata_08 = nullptr;
        }
        if (auto* const first = t.history_d54) {
            const auto* const profile = reinterpret_cast<const volatile NativeSessionTargetHistoryProfile*>(
                static_cast<volatile NativeSessionTargetHistoryStorage*>(first)->profile_00);
            const auto scalar = profile->scalar_00;
            scalar(first, nullptr, 1);
        }
        if (auto* const second = t.history_d58) {
            const auto* const profile = reinterpret_cast<const volatile NativeSessionTargetHistoryProfile*>(
                static_cast<volatile NativeSessionTargetHistoryStorage*>(second)->profile_00);
            const auto scalar = profile->scalar_00;
            scalar(second, nullptr, 1);
        }
        if (auto* const section = t.section_04) {
            volatile auto& depth = section->depth;
            while (depth > 0) {
                depth = depth - 1;
                ::LeaveCriticalSection(&section->native);
            }
            ::DeleteCriticalSection(&section->native);
            singleton_lifetime_free(section);
        }
    } catch (...) {
        destroy_native_session_transport_buffers_007830f0(&target->buffers_10);
        throw;
    }
    destroy_native_session_transport_buffers_007830f0(&target->buffers_10);
}

NativeSessionTargetStorage* scalar_delete_native_session_target_00784320(
    NativeSessionTargetStorage* target, std::uint32_t flags,
    volatile std::uint16_t& actual_slots_f871b0) {
    destroy_native_session_target_00783b90(target, actual_slots_f871b0);
    if ((flags & 1u) != 0) singleton_lifetime_free(target);
    return target;
}
} // namespace bsp
