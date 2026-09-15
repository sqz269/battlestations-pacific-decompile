#include "bsp/native_scene_render_traversal.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_traceline_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRender20Profiles) == 16);
static_assert(offsetof(NativeRender20Profiles, node_00d62c88) == 0);
static_assert(offsetof(NativeRender20Profiles, camera_00d62cf0) == 4);
static_assert(offsetof(NativeRender20Profiles, model_00d62de8) == 8);
static_assert(offsetof(NativeRender20Profiles, traceline_00d0c928) == 12);
static_assert(offsetof(NativeTracelineRenderAccess, zero_00d7a218) == 28);
static_assert(offsetof(NativeTracelineRenderAccess, clock_current_14) == 52);
static_assert(offsetof(NativeTracelineRenderAccess, render20_profiles) == 56);
static_assert(sizeof(NativeTracelineRenderAccess) == 60);

NativeRender20Profiles::NativeRender20Profiles(const NativeCameraEnvironment& camera,
    const NativeModelEnvironment& model, const NativeTracelineLifetimeAccess& line,
    const NativeModelEnvironment& line_model)
    : node_00d62c88(camera.vtable_00d62c88),
      camera_00d62cf0(camera.vtable_00d62cf0),
      model_00d62de8(model.vtable_00d62de8),
      traceline_00d0c928(line.vtable_00d0c928) {
    if (!node_00d62c88 || !camera_00d62cf0 || !model_00d62de8 || !traceline_00d0c928 ||
        model.vtable_00d62c88 != node_00d62c88 ||
        line_model.vtable_00d62c88 != node_00d62c88 ||
        line_model.vtable_00d62de8 != model_00d62de8)
        throw std::invalid_argument("render traversal requires canonical actual node/camera/model/Traceline table views");
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
    }
}

[[noreturn]] void missing_render_profile() {
    throw std::logic_error("native render traversal requires a bound actual four-profile view");
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
} // namespace

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
} // namespace bsp
