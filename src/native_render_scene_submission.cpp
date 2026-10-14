#include "bsp/native_render_scene_submission.hpp"
#include "bsp/native_render_command_owner.hpp"
#include "bsp/native_render_queue_access.hpp"
#include "bsp/native_scene_render_traversal.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene submission requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeRenderCommandStorage) == 0x44);
static_assert(offsetof(NativeTracelineRenderAccess, render20_profiles) == 0x38);

std::uint32_t read_word(const void* base, std::size_t offset) noexcept {
    const auto* address = static_cast<const std::byte*>(base) + offset;
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}

NativeRenderCommandStorage* acquire_command(void* scene, void* camera, void* target,
    NativeRenderSceneSubmissionContext& c, NativeRenderSceneSubmissionOperation& op) {
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x44, sizeof(NativeRenderCommandStorage)});
    op.allocation = allocation;
    NativeRenderCommandStorage* command = nullptr;
    // The actual BF681B helper throws after its new-handler loop. Preserve the
    // caller's explicit null arm nevertheless; subsequent native uses may fault.
    if (allocation) {
        try {
            command = construct_native_render_command_00b1f170(allocation,
                c.commands, c.associations, scene, camera, target);
        } catch (...) {
            // CBCC78 / CBCC90: free the failed NEW allocation, not its fields.
            singleton_lifetime_free(allocation);
            op.allocation = nullptr;
            throw;
        }
    }
    op.command = command;
    return command;
}

void append_command(NativeRenderCommandQueueStorage& queue,
    NativeRenderCommandStorage* command, NativeRenderSceneSubmissionOperation& op) {
    // B1F55A..B1F58B / B1F631..B1F66A are exactly this existing specialized
    // reserve/append schedule: signed doubling/min1; reload current fields;
    // skip null computed destination, then increment the live count.
    append_native_render_command_pointer_00b1cc20(queue.commands_14, &command);
    op.appended = true;
}

void return_captured_label(char* captured, void* label, NativeStringRawPoolContext& c) {
    if (!captured) return;
    const auto bytes = read_word(label, 0) + 1u;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        c.actual_published_01090aa8, c.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, captured, bytes,
        c.actual_small_returns_disabled_01090aa4);
}

[[noreturn]] void missing_profile() {
    throw std::logic_error("native drawable submission requires its canonical actual render profile");
}

// This fallback is the existing real application boundary, not a generic
// success adapter. It is reached after the original two x87 stores are done.
void __fastcall application_target(void* node, const NativeTracelineRenderAccess* a,
    std::uint32_t target, void* context, std::uint32_t lod_word,
    std::uint32_t visibility_word, std::uint32_t flags) {
    float lod, visibility;
    std::memcpy(&lod, &lod_word, 4);
    std::memcpy(&visibility, &visibility_word, 4);
    a->services->call_virtual20(node, target, context, lod, visibility, flags);
}

// Source helper for original B1F679..B1F694, not a native function. ECX=node,
// EDX=access, stack=actual command. Integer-only profile translation preserves
// FLD1 -> current table -> current command context -> slot20 -> FSTP -> FLDZ
// -> FSTP. The normal original four outgoing words reach concrete bodies.
__declspec(naked) void __fastcall collect_drawable(void*,
    const NativeTracelineRenderAccess*, void*) {
    __asm {
        push ebx
        push esi
        push edi
        mov esi, ecx
        mov edi, edx
        fld1
        mov eax, dword ptr [esi]
        mov ebx, dword ptr [edi + 38h]
        test ebx, ebx
        je unbound
        cmp eax, 00d62c88h
        je node
        cmp eax, 00d62cf0h
        je camera
        cmp eax, 00d62de8h
        je model
        cmp eax, 00d0c928h
        je traceline
    unbound:
        xor ebx, ebx
        jmp profile_ready
    node:
        mov ebx, dword ptr [ebx]
        jmp profile_ready
    camera:
        mov ebx, dword ptr [ebx + 4]
        jmp profile_ready
    model:
        mov ebx, dword ptr [ebx + 8]
        jmp profile_ready
    traceline:
        mov ebx, dword ptr [ebx + 0ch]
    profile_ready:
        mov edx, dword ptr [esp + 10h]
        mov edx, dword ptr [edx + 28h]
        test ebx, ebx
        je no_target
        mov ebx, dword ptr [ebx + 20h]
    no_target:
        push 0
        sub esp, 8
        fstp dword ptr [esp + 4]
        fldz
        fstp dword ptr [esp]
        push edx
        test ebx, ebx
        je missing
        mov ecx, ebx
        mov edx, edi
        call select_native_render20_body
        test eax, eax
        je external
        mov ecx, esi
        mov edx, edi
        call eax
        jmp done
    external:
        // The original four words are already at the top of the stack.
        // Add captured target for the source-only fastcall adapter.
        push ebx
        mov ecx, esi
        mov edx, edi
        call application_target
        jmp done
    missing:
        // No FP value is live across a C++ diagnostic. Native-valid domains
        // never enter this source-only missing-profile failure.
        add esp, 10h
        call missing_profile
    done:
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}
} // namespace

NativeRenderCommandStorage* submit_native_scene_00b1f4d0(
    NativeRenderCommandQueueStorage& queue, void* scene, void* camera, void* targets,
    void* label, NativeRenderSceneSubmissionContext& c,
    NativeRenderSceneSubmissionOperation& op) {
    bool argument_owned = true; // Native EH state0 owns the by-value argument.
    try {
        auto* const command = acquire_command(scene, camera, targets, c, op);
        // EBP captures the argument DATA only after command construction.
        auto* const captured_data = reinterpret_cast<char*>(read_word(label, 4));
        auto* const destination = reinterpret_cast<std::byte*>(command) + 0x14;
        if (destination != label) {
            resize_native_string_header_0041dd40(destination, c.strings,
                read_word(label, 0), true);
            if (read_word(label, 0) != 0) {
                const auto bytes = read_word(destination, 0);
                void* const data = reinterpret_cast<void*>(read_word(destination, 4));
                // BF7680 supports overlap. The source pointer is the earlier
                // EBP capture, not a reload after the resize/allocation.
                if (bytes) std::memmove(data, captured_data, bytes);
            }
        }
        append_command(queue, command, op);
        auto* const context = reinterpret_cast<void*>(read_word(command, 0x28));
        traverse_native_scene_00b72190(scene, &c.instances.render, context);
        upload_native_instance_groups_00b1e990(command, c.instances);
        argument_owned = false; // B1F5A1: disarm BEFORE the returning getter.
        return_captured_label(captured_data, label, c.strings);
        op.completed = true;
        return command;
    } catch (...) {
        if (argument_owned) {
            argument_owned = false;
            // CBCC70 reloads the current by-value header, unlike normal EBP.
            destroy_native_string_header_0041dd20(label, c.strings);
        }
        throw;
    }
}

NativeRenderCommandStorage* submit_native_drawable_00b1f5e0(
    NativeRenderCommandQueueStorage& queue, void* scene, void* camera, void* targets,
    void* drawable, NativeRenderSceneSubmissionContext& c,
    NativeRenderSceneSubmissionOperation& op) {
    auto* const command = acquire_command(scene, camera, targets, c, op);
    append_command(queue, command, op);
    const auto camera_mask = read_word(camera, 0x19c);
    if ((read_word(drawable, 0x48) & camera_mask) != 0) {
        collect_drawable(drawable, &c.instances.render, command);
        upload_native_instance_groups_00b1e990(command, c.instances);
    }
    op.completed = true;
    return command;
}
} // namespace bsp
