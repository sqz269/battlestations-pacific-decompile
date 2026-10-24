#include "bsp/native_scene_render_traversal.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_group_world_sphere.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_traceline_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRender20Profiles) == 24);
static_assert(offsetof(NativeRender20Profiles, node_00d62c88) == 0);
static_assert(offsetof(NativeRender20Profiles, camera_00d62cf0) == 4);
static_assert(offsetof(NativeRender20Profiles, model_00d62de8) == 8);
static_assert(offsetof(NativeRender20Profiles, traceline_00d0c928) == 12);
static_assert(offsetof(NativeRender20Profiles, group_00d634f8) == 16);
static_assert(offsetof(NativeRender20Profiles, group_environment) == 20);
static_assert(offsetof(NativeTracelineRenderAccess, zero_00d7a218) == 28);
static_assert(offsetof(NativeTracelineRenderAccess, clock_current_14) == 52);
static_assert(offsetof(NativeTracelineRenderAccess, render20_profiles) == 56);
static_assert(sizeof(NativeTracelineRenderAccess) == 60);

NativeRender20Profiles::NativeRender20Profiles(const NativeCameraEnvironment& camera,
    const NativeModelEnvironment& model, const NativeGroupEnvironment* group)
    : node_00d62c88(camera.vtable_00d62c88),
      camera_00d62cf0(camera.vtable_00d62cf0),
      model_00d62de8(model.vtable_00d62de8), traceline_00d0c928(nullptr),
      group_00d634f8(group ? group->vtable_00d634f8 : nullptr), group_environment(group) {
    if (!node_00d62c88 || !camera_00d62cf0 || !model_00d62de8 ||
        model.vtable_00d62c88 != node_00d62c88)
        throw std::invalid_argument("ordinary render traversal requires canonical node/camera/model table views");
    if (group && (!group_00d634f8 || group->vtable_00d62c88 != node_00d62c88 ||
        &group->nodes != &model.nodes || &group->nodes != &camera.nodes))
        throw std::invalid_argument("group render traversal requires the same canonical node/table domain");
}

NativeRender20Profiles::NativeRender20Profiles(const NativeCameraEnvironment& camera,
    const NativeModelEnvironment& model, const NativeTracelineLifetimeAccess& line,
    const NativeModelEnvironment& line_model, const NativeGroupEnvironment* group)
    : node_00d62c88(camera.vtable_00d62c88),
      camera_00d62cf0(camera.vtable_00d62cf0),
      model_00d62de8(model.vtable_00d62de8),
      traceline_00d0c928(line.vtable_00d0c928),
      group_00d634f8(group ? group->vtable_00d634f8 : nullptr), group_environment(group) {
    if (!node_00d62c88 || !camera_00d62cf0 || !model_00d62de8 || !traceline_00d0c928 ||
        model.vtable_00d62c88 != node_00d62c88 ||
        line_model.vtable_00d62c88 != node_00d62c88 ||
        line_model.vtable_00d62de8 != model_00d62de8)
        throw std::invalid_argument("render traversal requires canonical actual node/camera/model/Traceline table views");
    if (group && (!group_00d634f8 || group->vtable_00d62c88 != node_00d62c88 ||
        &group->nodes != &model.nodes || &group->nodes != &camera.nodes))
        throw std::invalid_argument("group render traversal requires the same canonical node/table domain");
}

__declspec(naked) NativeRender20Body __fastcall select_native_render20_body(
    std::uint32_t, const NativeTracelineRenderAccess*) noexcept {
    __asm {
        cmp ecx, 00b748e0h
        je model
        cmp ecx, 00af26a0h
        je traceline
        cmp dword ptr [edx + 38h], 0
        je external
        cmp ecx, 00b6d990h
        je node
        cmp ecx, 00b6fb80h
        je camera
        cmp ecx, 00b8ef10h
        je group
    external:
        xor eax, eax
        ret
    model:
        mov eax, offset render_native_generated_model_00b748e0
        ret
    traceline:
        mov eax, offset render_native_traceline_00af26a0
        ret
    node:
        mov eax, offset render_native_node_children_00b6d990
        ret
    camera:
        mov eax, offset render_native_camera_children_00b6fb80
        ret
    group:
        mov eax, dword ptr [edx + 38h]
        cmp dword ptr [eax + 10h], 0
        je external
        mov eax, offset render_native_group_children_00b8ef10
        ret
    }
}

namespace {
// PRIVATE REGISTER ENTRY: EAX=captured numeric profile, EDX=access.
// Return EAX=actual borrowed table or null. Only EAX/EDX/EFLAGS are clobbered.
// No calls, FP/SSE, owner reread, target-slot read or x87-capacity requirement.
__declspec(naked) void translate_captured_profile() noexcept {
    __asm {
        mov edx, dword ptr [edx + 38h]
        test edx, edx
        je unavailable
        cmp eax, 00d62c88h
        je node
        cmp eax, 00d62cf0h
        je camera
        cmp eax, 00d62de8h
        je model
        cmp eax, 00d0c928h
        je traceline
        cmp eax, 00d634f8h
        je group
    unavailable:
        xor eax, eax
        ret
    node:
        mov eax, dword ptr [edx]
        ret
    camera:
        mov eax, dword ptr [edx + 4]
        ret
    model:
        mov eax, dword ptr [edx + 8]
        ret
    traceline:
        mov eax, dword ptr [edx + 0ch]
        ret
    group:
        mov eax, dword ptr [edx + 10h]
        ret
    }
}

[[noreturn]] void missing_render_profile() {
    throw std::logic_error("native render traversal requires a bound actual current profile view");
}

// Existing real application boundary, reached only after the native FP spills.
// Raw words cross this adapter; arbitrary service stack/FP/exception behavior
// remains its existing contract, not original native virtual-call ABI proof.
void __fastcall application_captured_target(void* owner,
    const NativeTracelineRenderAccess* access, std::uint32_t target, void* context,
    std::uint32_t lod_word, std::uint32_t visibility_word, std::uint32_t flags) {
    float lod, visibility;
    std::memcpy(&lod, &lod_word, 4);
    std::memcpy(&visibility, &visibility_word, 4);
    access->services->call_virtual20(owner, target, context, lod, visibility, flags);
}

// EAX=captured target, ECX=owner, EDX=access, four ORIGINAL outgoing words.
// Copy only for the explicitly qualified C++ application fallback.
__declspec(naked) void application_target_entry() {
    __asm {
        push dword ptr [esp + 10h] // flags
        push dword ptr [esp + 10h] // visibility
        push dword ptr [esp + 10h] // lod
        push dword ptr [esp + 10h] // context
        push eax // captured target, never another profile/slot lookup
        call application_captured_target // source fastcall RET14
        ret 10h
    }
}

// PRIVATE REGISTER ENTRY: EAX=ALREADY captured target, ECX=owner, EDX=access.
// Known targets tail their source body with the return address and all four
// original outgoing words intact. No target argument shifts those words.
__declspec(naked) void dispatch_captured_target() {
    __asm {
        push ecx
        push edx
        push eax
        mov ecx, eax
        call select_native_render20_body
        test eax, eax
        je application
        mov edx, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        add esp, 0ch
        jmp eax
    application:
        mov eax, dword ptr [esp]
        mov edx, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        add esp, 0ch
        jmp application_target_entry
    }
}

const void* __fastcall application_captured_sphere(void* owner,
    const NativeTracelineRenderAccess* access, std::uint32_t target) {
    return access->services->call_virtual48(owner, target);
}

// EAX=captured current48 target, ECX=actual owner, EDX=access. No profile or
// slot reread, and no FP operation before either complete concrete body.
__declspec(naked) void dispatch_captured_sphere() {
    __asm {
        cmp eax, 00b6e8c0h
        je model
        cmp eax, 00b8f100h
        je group
        push eax
        call application_captured_sphere
        ret
    model:
        jmp get_native_model_world_sphere_00b6e8c0
    group:
        jmp get_native_group_world_sphere_00b8f100
    }
}

const float* current_attachment_sphere(NativeGroupWorldSphereRuntime& runtime,
    SceneNodeAttachment& binding) {
    const auto* access = static_cast<const NativeTracelineRenderAccess*>(runtime.context);
    void* const actual = reinterpret_cast<void*>(binding.pointer_key);
    const auto target = access->services->profile(actual)[0x48 / 4];
    if (target == 0x00b6e8c0u)
        return static_cast<const float*>(get_native_model_world_sphere_00b6e8c0(actual, access));
    if (target == 0x00b8f100u)
        return static_cast<const float*>(get_native_group_world_sphere_00b8f100(actual, access));
    return static_cast<const float*>(access->services->call_virtual48(actual, target));
}

const void* __fastcall dynamic_group_sphere(void* actual,
    const NativeTracelineRenderAccess* access) {
    if (!access || !access->render20_profiles || !access->render20_profiles->group_environment)
        throw std::logic_error("dynamic group sphere requires its canonical Group environment");
    const auto& environment = *access->render20_profiles->group_environment;
    auto* reference = dynamic_cast<NativeGroupReference*>(environment.nodes.attachments.find_actual_node(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual))));
    if (!reference || &reference->group_owner().storage.node != actual ||
        &reference->group_owner().environment != &environment)
        throw std::logic_error("dynamic group sphere requires its same canonical actual Group owner");
    auto& owner = reference->group_owner();
    if ((owner.storage.node.auxiliary_flags_138 & 0x30u) == 0) {
        NativeGroupWorldSphereRuntime runtime{environment.nodes,
            {access->crt, access->half_00d7a280}, const_cast<NativeTracelineRenderAccess*>(access),
            current_attachment_sphere};
        aggregate_native_group_world_sphere_00b8ebe0(runtime, owner);
        owner.storage.node.auxiliary_flags_138 |= 0x30u; // B8F11D: reload after callback.
    }
    return owner.storage.node.world_sphere_13c.data();
}
} // namespace

__declspec(naked) const void* __fastcall get_native_group_world_sphere_00b8f100(
    void*, const NativeTracelineRenderAccess*) {
    __asm {
        cmp byte ptr [ecx + 175h], 0 // B8F106, before cache test.
        jne dynamic
        // B8F12F..B8F18B is instruction-identical to B6E8C6..B6E922.
        // The existing raw body owns the same 10h scratch and ordered x87 copies.
        jmp get_native_model_world_sphere_00b6e8c0
    dynamic:
        jmp dynamic_group_sphere // existing canonical companion/array projection.
    }
}

__declspec(naked) void __fastcall traverse_native_scene_00b72190(
    void*, const NativeTracelineRenderAccess*, void*) {
    __asm {
        movss xmm0, dword ptr [ecx + 18h] // B72190, BEFORE first node
        push ebx
        push esi
        mov esi, dword ptr [ecx + 0ch]
        test esi, esi
        push edi
        // Two private words: access, captured table. Original entry S+4 stays.
        lea esp, [esp - 8] // preserve the native first-node TEST flags
        mov dword ptr [esp], edx
        mov edi, dword ptr [esp + 18h]
        mov ebx, dword ptr [edi + 8]
        movss dword ptr [esp + 18h], xmm0 // B721A4, ORIGINAL context word
        je done
    next_node:
        mov eax, dword ptr [esi + 48h]
        test dword ptr [ebx + 19ch], eax
        je advance
        fld1 // B721BB
        mov eax, dword ptr [esi] // B721BD: captured current profile
        mov edx, dword ptr [esp]
        call translate_captured_profile // INTEGER ONLY while x87 live
        mov dword ptr [esp + 4], eax
        test eax, eax
        je no_target
        mov eax, dword ptr [eax + 20h] // B721BF, current target after FLD1
    no_target:
        push 0
        sub esp, 8
        fstp dword ptr [esp + 4] // B721C7
        mov ecx, esi
        fld dword ptr [esp + 24h] // B721CD, original entry S+4
        fstp dword ptr [esp]
        push edi
        cmp dword ptr [esp + 14h], 0 // private table, not target value
        jne scene_profile_ready
        call missing_render_profile // FP spills complete; source-only error
    scene_profile_ready:
        mov edx, dword ptr [esp + 10h]
        call dispatch_captured_target // B721D5; captured EAX and4words
    advance:
        mov esi, dword ptr [esi + 3ch] // B721D7, AFTER callback
        test esi, esi
        jne next_node
    done:
        add esp, 8
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}

__declspec(naked) void __fastcall render_native_node_children_00b6d990(
    void*, const NativeTracelineRenderAccess*, void*, float, float, std::uint32_t) {
    __asm {
        sub esp, 8 // private access and captured table; original words stay
        mov dword ptr [esp], edx
        push edi
        mov edi, ecx
        movss xmm0, dword ptr [edi + 0ach] // B6D993
        mov eax, dword ptr [esp + 4]
        mov eax, dword ptr [eax + 1ch] // SAME borrowed D7A218 cell
        comiss xmm0, dword ptr [eax] // B6D99B
        jbe done // <= OR unordered
        push esi
        mov esi, dword ptr [edi + 34h]
        test esi, esi
        je no_children
        push ebx
        mov ebx, dword ptr [esp + 24h] // captured original S+10h FLAGS
        push ebp
        mov ebp, dword ptr [esp + 1ch] // captured original S+4 context
    next_child:
        fld dword ptr [esp + 24h] // B6D9C0 ORIGINAL S+0Ch visibility
        mov eax, dword ptr [esi] // B6D9C4 captured child profile
        mov edx, dword ptr [esp + 10h]
        call translate_captured_profile // INTEGER ONLY, no slot20 read
        mov dword ptr [esp + 14h], eax
        fmul dword ptr [edi + 0ach] // B6D9C6 CURRENT parent visibility
        test eax, eax
        je no_target
        mov eax, dword ptr [eax + 20h] // B6D9CC AFTER FMUL
    no_target:
        push ebx
        sub esp, 8
        fstp dword ptr [esp + 34h] // B6D9D3 ORIGINAL S+10h FLAGS spill
        mov ecx, esi
        fld dword ptr [esp + 34h]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 2ch] // B6D9E1 ORIGINAL S+8 lod
        fstp dword ptr [esp]
        push ebp
        cmp dword ptr [esp + 24h], 0 // private table, not target value
        jne node_profile_ready
        call missing_render_profile // original flags spill already occurred
    node_profile_ready:
        mov edx, dword ptr [esp + 20h]
        call dispatch_captured_target // B6D9E9, no late profile lookup
        mov esi, dword ptr [esi + 3ch] // B6D9EB, AFTER callback
        test esi, esi
        jne next_child
        pop ebp
        pop ebx
    no_children:
        pop esi
    done:
        pop edi
        add esp, 8
        ret 10h
    }
}

__declspec(naked) void __fastcall render_native_camera_children_00b6fb80(
    void*, const NativeTracelineRenderAccess*, void*, float, float, std::uint32_t) {
    __asm {
        push edx // private access, original four words remain in place
        mov eax, dword ptr [esp + 14h] // original flags
        fld dword ptr [esp + 10h] // original visibility
        mov edx, dword ptr [esp + 8] // original context
        push eax
        sub esp, 8
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 18h] // original lod
        fstp dword ptr [esp]
        push edx
        mov edx, dword ptr [esp + 10h] // saved source access
        call render_native_node_children_00b6d990 // B6FB9C
        add esp, 4
        ret 10h
    }
}

__declspec(naked) void __fastcall render_native_group_children_00b8ef10(
    void*, const NativeTracelineRenderAccess*, void*, float, float, std::uint32_t) {
    __asm {
        sub esp, 8 // private access/table, outside the original 18h local frame.
        mov dword ptr [esp], edx
        sub esp, 18h // B8EF10
        push ebx
        mov ebx, dword ptr [esp + 28h] // B8EF14: ORIGINAL context.
        mov eax, dword ptr [ebx + 0ch]
        push esi
        mov esi, dword ptr [ebx + 8]
        push edi
        mov edi, ecx
        movss xmm0, dword ptr [edi + 0ach]
        mov edx, dword ptr [esp + 24h]
        mov edx, dword ptr [edx + 1ch]
        comiss xmm0, dword ptr [edx] // B8EF2A, SAME D7A218.
        mov dword ptr [esp + 10h], esi
        mov dword ptr [esp + 0ch], eax
        jbe done // <= OR unordered, before EBP is saved.
        push ebp
        mov ebp, dword ptr [esp + 40h] // B8EF40: ORIGINAL flags.
        mov ecx, ebp
        and ecx, 555h
        cmp ecx, 555h
        mov eax, ebp
        je classified
        test byte ptr [edi + 138h], 3
        je classified
        mov eax, dword ptr [edi] // B8EF5F: capture CURRENT group profile.
        mov edx, dword ptr [esp + 28h]
        call translate_captured_profile
        test eax, eax
        jne cull_profile_ready
        call missing_render_profile
    cull_profile_ready:
        mov eax, dword ptr [eax + 48h] // B8EF61: capture CURRENT target.
        mov ecx, edi
        mov edx, dword ptr [esp + 28h]
        call dispatch_captured_sphere
        push eax
        mov ecx, esi
        mov edx, dword ptr [esp + 2ch]
        call classify_native_camera_sphere_00b71530 // B8EF6B.
    classified:
        cmp eax, 0aaah
        je no_children
        mov ecx, dword ptr [esp + 10h] // view camera captured on entry.
        call get_native_camera_view_projection_00b70490
        mov edx, dword ptr [edi] // B8EF84: capture profile BEFORE matrix push.
        push eax // original matrix word.
        mov eax, edx
        mov edx, dword ptr [esp + 2ch]
        call translate_captured_profile
        test eax, eax
        jne projection_profile_ready
        call missing_render_profile
    projection_profile_ready:
        mov eax, dword ptr [eax + 48h] // B8EF87, before destination LEA.
        lea ecx, [esp + 1ch] // SAME projected sphere in native local frame.
        push ecx
        mov ecx, edi
        mov edx, dword ptr [esp + 30h]
        call dispatch_captured_sphere
        mov ecx, eax
        mov edx, dword ptr [esp + 30h]
        call transform_native_sphere_007c1180 // B8EF95, consumes ORIGINAL 2words.
        movss xmm0, dword ptr [esp + 20h]
        mov eax, dword ptr [esp + 28h]
        mov eax, dword ptr [eax + 24h]
        movss xmm1, dword ptr [eax] // SAME D7A238.
        comiss xmm0, xmm1
        jbe use_epsilon
        movss dword ptr [esp + 34h], xmm0 // ORIGINAL context word, B8EFAD.
        jmp divide_radius
    use_epsilon:
        movss dword ptr [esp + 34h], xmm1
    divide_radius:
        fld dword ptr [esp + 24h]
        mov edx, dword ptr [esp + 10h]
        fdiv dword ptr [esp + 34h]
        fstp dword ptr [esp + 34h]
        fld dword ptr [edx + 4ch]
        mov ecx, dword ptr [esp + 34h]
        and ecx, 7fffffffh // native integer ABS preserves exact float encoding.
        mov dword ptr [esp + 34h], ecx
        fmul dword ptr [esp + 34h]
        fld1
        fsubrp st(1), st(0)
        fstp dword ptr [esp + 34h]
        fld dword ptr [esp + 34h]
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        jbe upper_clamp // B8EFF2 includes unordered.
        xorps xmm0, xmm0
    store_lod:
        movss dword ptr [esp + 34h], xmm0
    test_threshold:
        fld dword ptr [esp + 34h]
        fld dword ptr [edi + 184h]
        fcomip st(0), st(1)
        fstp st(0)
        jbe no_children
        mov esi, dword ptr [edi + 34h]
        test esi, esi
        je no_children
    next_child:
        mov eax, dword ptr [esi + 48h]
        mov ecx, dword ptr [esp + 14h] // original captured cull camera.
        test dword ptr [ecx + 19ch], eax
        je advance
        fld dword ptr [edi + 0ach] // B8F02F CURRENT parent visibility.
        mov eax, dword ptr [esi] // B8F035 CURRENT child profile.
        mov edx, dword ptr [esp + 28h]
        call translate_captured_profile // integer-only with live x87 value.
        mov dword ptr [esp + 2ch], eax
        fmul dword ptr [esp + 3ch] // ORIGINAL incoming alpha, never accumulated.
        test eax, eax
        je no_child_target
        mov eax, dword ptr [eax + 20h] // B8F03B AFTER FMUL.
    no_child_target:
        push ebp
        sub esp, 8
        fstp dword ptr [esp + 4ch] // B8F042 ORIGINAL incoming FLAGS word.
        mov ecx, esi
        fld dword ptr [esp + 4ch]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 40h] // ORIGINAL context word now contains LOD.
        fstp dword ptr [esp]
        push ebx
        cmp dword ptr [esp + 3ch], 0 // captured table, not target.
        jne child_profile_ready
        call missing_render_profile // original native FP spills already finished.
    child_profile_ready:
        mov edx, dword ptr [esp + 38h]
        call dispatch_captured_target // B8F058 with ORIGINAL four outgoing words.
    advance:
        mov esi, dword ptr [esi + 3ch] // B8F05A AFTER callback.
        test esi, esi
        jne next_child
    no_children:
        pop ebp
    done:
        pop edi
        pop esi
        pop ebx
        add esp, 18h
        add esp, 8
        ret 10h
    upper_clamp:
        movss xmm0, dword ptr [esp + 34h]
        mov eax, dword ptr [esp + 28h]
        mov eax, dword ptr [eax + 20h]
        movss xmm1, dword ptr [eax] // SAME D7A24C.
        comiss xmm0, xmm1
        jbe store_lod
        movss dword ptr [esp + 34h], xmm1
        jmp test_threshold
    }
}
} // namespace bsp
