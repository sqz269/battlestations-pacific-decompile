#include "bsp/native_shadow_command_queue.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_scene_render_traversal.hpp"
#include <cstddef>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shadow command queue requires MSVC Win32 raw assembly.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeInstanceGroupUploadAccess, render) == 0);
namespace {
// New source ABI adapters only; neither receives native function credit.
// The first retains ECX header + one original requested stack word / RET4.
void __fastcall reserve_command_header(NativeRenderPointerArrayStorage* header,
    void*, std::int32_t requested) {
    reserve_native_render_command_pointers_00b1c6c0(*header, requested);
}
void __fastcall upload_captured_command(void* command,
    NativeInstanceGroupUploadAccess* access) {
    upload_native_instance_groups_00b1e990(command, *access);
}
// Raw DWORD load avoids introducing typed owner/profile alias assumptions.
__declspec(naked) std::uint32_t __fastcall current_word(const volatile void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx]
        _emit 0xc3
    }
}
} // namespace

__declspec(naked) void __fastcall append_native_shadow_command_00b1eb80(
    void*, NativeInstanceGroupUploadAccess*, void*, std::uint32_t) {
    __asm {
        push edx // Source-only access at entry S-4; original words stay S+4/+8.
        mov eax, dword ptr [ecx + 1ch] // B1EB80
        cmp dword ptr [ecx + 18h], eax // B1EB83; equality only
        push esi // B1EB86
        lea esi, [ecx + 14h] // B1EB87; actual array header
        push edi // B1EB8A; no flags change since CMP
        jne append_current_command // B1EB8B
        add eax, eax // B1EB8D; DWORD wrap
        cmp eax, 1 // B1EB8F; signed minimum1
        jg capacity_ready // B1EB92
        mov eax, 1 // B1EB94
    capacity_ready:
        push eax // B1EB99
        mov ecx, esi // B1EB9A
        call reserve_command_header // B1EB9C -> concrete B1C6C0, RET4
    append_current_command:
        mov eax, dword ptr [esi + 4] // B1EBA1; CURRENT count after reserve
        mov ecx, dword ptr [esi] // B1EBA4; CURRENT data
        mov edi, dword ptr [esp + 10h] // B1EBA6; ORIGINAL public command
        lea eax, [ecx + eax*4] // B1EBAA; AFTER command read
        test eax, eax // B1EBAD
        jz command_slot_done // B1EBAF
        mov dword ptr [eax], edi // B1EBB1
    command_slot_done:
        add dword ptr [esi + 4], 1 // B1EBB3; CURRENT count after raw store
        cmp byte ptr [esp + 14h], 0 // B1EBB7; LATE ORIGINAL public flag byte
        je queue_done // B1EBBC
        mov edx, dword ptr [edi + 28h] // B1EBBE; current captured-command context
        mov ecx, dword ptr [edi + 4] // B1EBC1; then current scene
        push edx // B1EBC4; original outgoing context, reused by traversal
        mov edx, dword ptr [esp + 0ch] // Source-only same upload access, S-4
        mov edx, dword ptr [edx] // SAME access.render binding
        call traverse_native_scene_00b72190 // B1EBC5; RET4
        mov ecx, edi // B1EBCA; still captured command
        mov edx, dword ptr [esp + 8] // Source-only same upload access, S-4
        call upload_captured_command // B1EBCC -> actual B1E990, no stack args
    queue_done:
        pop edi // B1EBD1
        pop esi // B1EBD2
        add esp, 4 // Remove only source access word.
        ret 8 // B1EBD3; original two public words
    }
}

__declspec(naked) void __fastcall execute_native_shadow_frame_job_00a8ae50(
    void*, NativeInstanceGroupUploadAccess*, void*) {
    __asm {
        push edx // Source-only access at S-4.
        push esi // A8AE50
        mov esi, dword ptr [esp + 0ch] // A8AE51; original S+4 command once
        mov ecx, esi // A8AE55
        call native_shadow_command_context_00b1bf30 // A8AE57
        push eax // A8AE5C; context BEFORE later scene getter
        mov ecx, esi // A8AE5D
        call native_shadow_command_scene_00b1bf20 // A8AE5F
        mov ecx, eax // A8AE64
        mov edx, dword ptr [esp + 8] // Source-only upload access at S-4
        mov edx, dword ptr [edx] // SAME access.render
        call traverse_native_scene_00b72190 // A8AE66; RET4
        mov ecx, esi // A8AE6B; captured command
        mov edx, dword ptr [esp + 4] // Source-only same upload access
        call upload_captured_command // A8AE6D -> actual B1E990, no stack args
        pop esi // A8AE72
        add esp, 4 // Remove source access only.
        ret 4 // A8AE73
    }
}

__declspec(naked) void* __fastcall native_shadow_command_scene_00b1bf20(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 4] // B1BF20
        _emit 0xc3 // B1BF23; exact one-byte RET, not RET0 encoding.
    }
}
__declspec(naked) void* __fastcall native_shadow_command_context_00b1bf30(const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 28h] // B1BF30
        _emit 0xc3 // B1BF33
    }
}

NativeShadowFrameJobDispatch::NativeShadowFrameJobDispatch(
    NativeInstanceGroupUploadAccess& access, const volatile std::uint32_t* table,
    NativeFrameJobDispatch& remaining)
    : access_(access), table_(table), remaining_(remaining) {
    if (!table_ || current_word(table_) != 0x00a8ae50u)
        throw std::invalid_argument("Shadow jobs require the actual D5B570 primary table");
}
void NativeShadowFrameJobDispatch::execute_current_00(void* owner,
    std::uint32_t argument) {
    const std::uint32_t profile = current_word(owner);
    if (profile != 0x00d5b570u) {
        remaining_.execute_current_00(owner, argument);
        return;
    }
    if (current_word(table_) != 0x00a8ae50u)
        throw std::invalid_argument("Missing current native shadow job callback binding");
    execute_native_shadow_frame_job_00a8ae50(owner, &access_,
        reinterpret_cast<void*>(argument));
}
} // namespace bsp
