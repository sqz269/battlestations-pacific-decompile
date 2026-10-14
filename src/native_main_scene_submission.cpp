#include "bsp/native_main_scene_submission.hpp"
#include "bsp/native_render_scene_submission.hpp"
#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_render_command_owner.hpp"
#include "bsp/native_render_queue_access.hpp"
#include "bsp/native_shadow_command_queue.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_frame_clock_actual.hpp"
#include "bsp/camera_clip_planes.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native main-scene submission requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t read_word(const void* p, std::size_t offset) noexcept {
    const auto* address = static_cast<const std::byte*>(p) + offset;
    std::uint32_t result;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
std::uint8_t read_byte(const void* p, std::size_t offset) noexcept {
    const auto* address = static_cast<const std::byte*>(p) + offset;
    std::uint8_t result;
    __asm {
        mov eax, address
        mov al, byte ptr [eax]
        mov result, al
    }
    return result;
}
void write_word(void* p, std::size_t offset, std::uint32_t value) noexcept {
    auto* address = static_cast<std::byte*>(p) + offset;
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}
void write_byte(void* p, std::size_t offset, std::uint8_t value) noexcept {
    auto* address = static_cast<std::byte*>(p) + offset;
    __asm {
        mov eax, address
        mov dl, value
        mov byte ptr [eax], dl
    }
}
struct MotionAccess {
    const CameraAxesCrtAccess* crt;
    const volatile double* hysteresis;
};
static_assert(offsetof(MotionAccess, hysteresis) == 4);

// Original B10217..B103D3, with B101DD..B10215's integer-only tracking/timer
// dispatch in the C++ caller. Keep all float32 spill boundaries and FCOMIP/JBE
// unordered behavior. Original scratch offsets remain10/14/18/1C/20/24/28.
__declspec(naked) void __fastcall update_motion(void*, const MotionAccess*, const void*) {
    __asm {
        sub esp, 2ch
        push ebx
        push ebp
        push esi
        push edi
        mov esi, ecx
        mov ebp, edx
        xor ebx, ebx
        mov eax, dword ptr [esp + 40h]
        test eax, eax
        jz hysteresis
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov edi, dword ptr [esi + 1c0h]
        test byte ptr [edi + 5ch], 2
        fdivp st(1), st(0)
        fstp dword ptr [esp + 14h]
        jnz first_world_ready
        mov ecx, edi
        call refresh_native_camera_world_00b6db70
    first_world_ready:
        fld dword ptr [edi + 120h]
        xorps xmm0, xmm0
        fstp dword ptr [esp + 20h]
        lea ecx, [esi + 19ch]
        fld dword ptr [edi + 124h]
        fstp dword ptr [esp + 24h]
        fld dword ptr [edi + 128h]
        fstp dword ptr [esp + 28h]
        fld dword ptr [esp + 20h]
        fsub dword ptr [esi + 1b4h]
        fstp dword ptr [esp + 10h]
        fld dword ptr [esp + 24h]
        fsub dword ptr [esi + 1b8h]
        fstp dword ptr [esp + 18h]
        fld dword ptr [esp + 28h]
        fsub dword ptr [esi + 1bch]
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esp + 10h]
        fld dword ptr [esp + 14h]
        fld st(0)
        fdivp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 14h]
        fld dword ptr [esp + 18h]
        fdiv st(0), st(1)
        fstp dword ptr [esp + 18h]
        fdivr dword ptr [esp + 1ch]
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esi + 244h]
        fadd dword ptr [esp + 14h]
        fstp dword ptr [esp + 14h]
        fld dword ptr [esi + 248h]
        fadd dword ptr [esp + 18h]
        fstp dword ptr [esp + 18h]
        fld dword ptr [esi + 24ch]
        fadd dword ptr [esp + 1ch]
        fstp dword ptr [esp + 1ch]
        fld dword ptr [esp + 14h]
        fstp dword ptr [ecx]
        fld dword ptr [esp + 18h]
        fstp dword ptr [ecx + 4]
        fld dword ptr [esp + 1ch]
        fstp dword ptr [ecx + 8]
        movss dword ptr [esi + 244h], xmm0
        movss dword ptr [esi + 248h], xmm0
        movss dword ptr [esi + 24ch], xmm0
        mov edx, dword ptr [ebp]
        call camera_vector_length_00419440
        fstp dword ptr [esi + 1b0h]
        mov edi, dword ptr [esi + 1c0h]
        test byte ptr [edi + 5ch], 2
        jnz second_world_ready
        mov ecx, edi
        call refresh_native_camera_world_00b6db70
    second_world_ready:
        movss xmm0, dword ptr [edi + 120h]
        movss xmm1, dword ptr [edi + 124h]
        movss xmm2, dword ptr [edi + 128h]
        movss dword ptr [esi + 1b4h], xmm0
        movss dword ptr [esi + 1b8h], xmm1
        movss dword ptr [esi + 1bch], xmm2
    hysteresis:
        mov cl, byte ptr [esi + 22ch]
        cmp cl, bl
        jz blur_off
        fld dword ptr [esi + 238h]
        fld dword ptr [esi + 1b0h]
        fcomip st(0), st(1)
        fstp st(0)
        jbe lower_threshold
        cmp byte ptr [esi + 219h], bl
        jz lower_threshold
        mov byte ptr [esi + 22dh], 1
        jmp visibility
    lower_threshold:
        cmp cl, bl
        jz blur_off
        cmp byte ptr [esi + 22dh], bl
        jz blur_off
        fld dword ptr [esi + 238h]
        fld dword ptr [esi + 1b0h]
        fld st(1)
        mov eax, dword ptr [ebp + 4]
        fmul qword ptr [eax]
        fsubp st(2), st(0)
        fcomip st(0), st(1)
        fstp st(0)
        jbe blur_off
        mov byte ptr [esi + 22dh], 1
        jmp visibility
    blur_off:
        mov byte ptr [esi + 22dh], bl
    visibility:
        cmp byte ptr [esi + 22dh], bl
        jz visible_or_untracked
        fldz
        jmp set_visibility
    visible_or_untracked:
        cmp cl, bl
        jz done
        fld1
    set_visibility:
        push ebx
        push ecx
        mov ecx, dword ptr [esi + 1c0h]
        fstp dword ptr [esp]
        call set_native_node_visibility_factor_00b6da70
    done:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 2ch
        ret 4
    }
}

struct ClipAccess {
    const volatile float* threshold;
    const volatile float* negative_one;
    const volatile float* distance;
};
static_assert(offsetof(ClipAccess, distance) == 8);
// Original COMISS/JBE clears on equality or unordered, unlike a negated C++
// comparison. MOVSS preserves the source payloads without x87 arithmetic.
__declspec(naked) bool __fastcall prepare_clip(void*, const ClipAccess*, CameraPlane*) {
    __asm {
        mov eax, dword ptr [edx]
        movss xmm0, dword ptr [eax]
        comiss xmm0, dword ptr [ecx + 124h]
        jbe clear_plane
        xorps xmm0, xmm0
        mov eax, dword ptr [edx + 4]
        movss xmm1, dword ptr [eax]
        mov ecx, dword ptr [esp + 4]
        movss dword ptr [ecx], xmm0
        movss dword ptr [ecx + 8], xmm0
        mov eax, dword ptr [edx + 8]
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx + 4], xmm1
        movss dword ptr [ecx + 0ch], xmm0
        mov al, 1
        ret 4
    clear_plane:
        xor eax, eax
        ret 4
    }
}
} // namespace

NativeRenderCommandStorage* submit_native_main_scene_00b101c0(void* service,
    void* scene, NativeCameraOwner& camera, NativeMainSceneSubmissionContext& c,
    NativeMainSceneSubmissionOperation& op) {
    void* const tracked = reinterpret_cast<void*>(read_word(service, 0x1c0));
    write_byte(service, 0x22c, tracked && read_byte(tracked, 0x44) == 0 ? 1 : 0);
    const void* interval = nullptr;
    if (read_byte(service, 0x22c) != 0) {
        void* const clock = c.actual_clock_01090ab0;
        const auto profile = read_word(clock, 0);
        if (profile != 0x00d68d50u || c.actual_clock_profile_00d68d50[7] != 0x00bee070u)
            throw std::logic_error("main scene submission requires actual D68D50:BEE070 frame clock");
        interval = get_raw_timer_interval_00bee070(clock);
    }
    const MotionAccess motion{&c.crt, &c.actual_hysteresis_00d5e1c8};
    update_motion(service, &motion, interval);

    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x44, sizeof(NativeRenderCommandStorage)});
    op.allocation = allocation;
    void* const camera_storage = &camera.storage.node;
    NativeRenderCommandStorage* command = nullptr;
    if (allocation) {
        try {
            // Load +1D4 after allocation; reentrant allocator changes are live.
            void* const target = reinterpret_cast<void*>(read_word(service, 0x1d4));
            command = construct_native_render_command_00b1f170(allocation,
                c.submission.commands, c.submission.associations, scene, camera_storage, target);
            write_word(allocation, 0, 0x00d5e13c);
        } catch (...) {
            // CBBC60: sole state0 funclet is free(saved allocation) only.
            singleton_lifetime_free(allocation);
            op.allocation = nullptr;
            throw;
        }
    }
    op.command = command;
    if ((read_byte(camera_storage, 0x5c) & 2) == 0)
        refresh_native_camera_world_00b6db70(camera_storage);
    CameraPlane plane;
    const ClipAccess clip{&c.actual_clip_threshold_00cfbc84,
        &c.actual_negative_one_00d7a260, &c.actual_clip_distance_00ce3958};
    CameraClipPlaneState clip_state{camera.frame, camera.storage.camera.extra_plane_2e0};
    if (prepare_clip(camera_storage, &clip, &plane))
        set_camera_extra_clip_plane_00b70410(clip_state, plane);
    else
        clear_camera_extra_clip_plane_00b70470(clip_state);

    auto* const append_queue = get_native_render_command_queue_004c11f0(c.queue);
    op.append_queue = append_queue;
    op.append_started = true;
    append_native_shadow_command_00b1eb80(append_queue, &c.submission.instances, command, 1);
    op.append_returned = true;
    const std::uint32_t metadata[3]{0, 0x24, 0x5c};
    auto* const metadata_queue = get_native_render_command_queue_004c11f0(c.queue);
    op.metadata_queue = metadata_queue;
    set_native_last_render_command_metadata_00b1cb50(*metadata_queue, metadata);

    if (read_byte(service, 0xb0) != 0) {
        void* const previous = reinterpret_cast<void*>(read_word(service, 0x10));
        if (previous != camera_storage) {
            write_word(service, 0x10, reinterpret_cast<std::uint32_t>(camera_storage));
            c.actual_increment_00ce221c(reinterpret_cast<volatile long*>(
                static_cast<std::byte*>(camera_storage) + 4));
            if (previous && c.actual_decrement_00ce2220(reinterpret_cast<volatile long*>(
                    static_cast<std::byte*>(previous) + 4)) == 0)
                dispatch_native_render_actual_owner_zero(c.submission.commands.owners, previous);
        }
    }
    op.completed = true;
    return command;
}
} // namespace bsp
