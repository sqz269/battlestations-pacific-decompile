#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_render_pointer_slot_sort.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/scene_attachment.hpp"
#include <cstddef>
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native instance group upload requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
template<class T> T read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
void* offset(void* p, Word bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + bytes);
}
void* end(void* descriptor) noexcept {
    // Native source loop reloads count before base (B1EA7B/B1EA7E).
    const Word count = read<Word>(descriptor, 4);
    return offset(read<void*>(descriptor), count * 4u);
}
std::int32_t distance(void* first, void* last) noexcept {
    const Word bytes = reinterpret_cast<Word>(last) - reinterpret_cast<Word>(first);
    const Word shifted = (bytes >> 2) | ((bytes & 0x80000000u) ? 0xc0000000u : 0u);
    std::int32_t result;
    std::memcpy(&result, &shifted, 4);
    return result;
}
void* geometry(void* model) noexcept {
    return gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(offset(model, 0x174)), 0);
}
void* stream(void* model) noexcept {
    return native_mesh_vertex_stream_00b73260(*static_cast<NativeMeshStorage*>(geometry(model)), 1);
}
void* __fastcall diffuse_bridge(void* material, void*, Word index) noexcept {
    return native_material_diffuse_00b179f0(*static_cast<NativeMaterialStorage*>(material), index);
}
} // namespace
namespace detail {
static_assert(offsetof(UploadOutputAccess, render) == 0);
static_assert(offsetof(UploadOutputAccess, half) == 4);
static_assert(offsetof(NativeTracelineRenderAccess, one_00d7a24c) == 32);

// B1EACC..B1EB17: retain the original outgoing eight stack words and complete
// MOVSS/FLDZ/FST/FLD/FSTP schedule around camera/model/output reloads. C++ float
// arguments or a float-return helper would introduce extra x87 crossings.
__declspec(naked) void __fastcall initialize_upload_output(void*,
    const UploadOutputAccess*, Word, void*, void*, void*) {
    __asm {
        sub esp,4 // Added private alpha spill outside original outgoing arguments.
        push ebx
        push esi
        push edi
        push ebp
        mov ebx,ecx // Captured actual group.
        mov ebp,edx // Added access.
        mov eax,dword ptr [esp + 24] // Category.
        mov edi,dword ptr [esp + 28] // Captured geometry.
        mov esi,dword ptr [esp + 32] // Captured section.
        mov edx,dword ptr [esp + 36] // Actual command.
        test eax,eax // B1EACC
        jz category_zero
        mov ecx,dword ptr [ebp + 4]
        movss xmm0,dword ptr [ecx] // B1EAD0
        jmp alpha_loaded
    category_zero:
        mov ecx,dword ptr [ebp]
        mov ecx,dword ptr [ecx + 32]
        movss xmm0,dword ptr [ecx] // B1EADA
    alpha_loaded:
        fldz // B1EAE2
        mov edx,dword ptr [edx + 28h] // B1EAE8
        mov ecx,dword ptr [edx + 8] // B1EAEB
        mov edx,dword ptr [ebx + eax*4 + 1ch] // B1EAEE
        push 555h // B1EAF2
        sub esp,8 // B1EAF7
        fst dword ptr [esp + 4] // B1EAFA: keep zero live on x87.
        movss dword ptr [esp + 28],xmm0 // B1EAFE: original float spill.
        fld dword ptr [esp + 28] // B1EB04
        fstp dword ptr [esp] // B1EB08
        push ecx // B1EB0B: camera.
        push edx // B1EB0C: current model.
        push edi // B1EB0D: captured geometry.
        push esi // B1EB0E: captured section.
        push ecx // B1EB0F: overwritten leading float argument.
        mov ecx,dword ptr [ebx + eax*4 + 14h] // B1EB10
        fstp dword ptr [esp] // B1EB14
        mov edx,dword ptr [ebp] // Added B51A20 access only.
        call initialize_native_render_entry_00b51a20 // B1EB17; RET20h.
        pop ebp
        pop edi
        pop esi
        pop ebx
        add esp,4
        ret 10h
    }
}
} // namespace detail

void* __fastcall native_node_point_light_array_00b6dc50(void* node) noexcept {
    return offset(node, 0x164);
}
SceneResource* __fastcall native_scene_lighting_owner_00b72110(const void* scene) noexcept {
    return read<SceneResource*>(scene, 0x1c);
}
void __fastcall set_native_section_instance_count_00b85590(void* section, void*, Word count) noexcept {
    *reinterpret_cast<volatile Word*>(offset(section, 0x1c)) = count;
}

void upload_native_instance_groups_00b1e990(void* command, NativeInstanceGroupUploadAccess& a) {
    auto& services = *a.render.services;
    const detail::UploadOutputAccess output_access{&a.render, &a.half_00ce3800};
    void* group_slot = read<void*>(command, 0x38);
    const Word initial_count = read<Word>(command, 0x3c);
    if (group_slot == offset(group_slot, initial_count * 4u)) return;
    do {
        void* const group = read<void*>(group_slot);
        for (Word category = 0; category < 2u; ++category) {
            const Word category_word = category * 4u;
            if (read<Word>(group, 0x0c + category_word) == 0) continue;
            // Capture model's current v50 table BEFORE the root's zero-argument
            // lighting getter; then reload model, as B1E9CB..B1E9E5 does.
            const auto* const model_table = services.profile(read<void*>(group, 0x1c + category_word));
            SceneResource* const scene = native_scene_lighting_owner_00b72110(read<void*>(command, 4));
            void* const attached_model = read<void*>(group, 0x1c + category_word);
            if (model_table[0x50 / 4] != 0x00b6ed80u) std::terminate();
            auto& binding = a.scenes.resolve_key(reinterpret_cast<Word>(attached_model));
            set_node_scene_00b6ed80(a.scenes, binding, scene, false);
            void* const sources = offset(group, 0x24 + category * 12u);
            if (category != 0) {
                const Word count = read<Word>(sources, 4);
                void* const last = offset(read<void*>(sources), count * 4u);
                void* const first = read<void*>(sources);
                sort_native_render_pointer_slots_00b1dce0(first, last, distance(first, last),
                    native_render_entry_material_depth_less_00b51ab0);
            }
            void* logical = stream(read<void*>(group, 0x1c + category_word));
            const Word count = read<Word>(group, 0x0c + category_word);
            if (services.profile(logical)[0x10 / 4] != 0x00b49980u) std::terminate();
            void* destination = lock_native_logical_vertex_stream_00b49980(logical, *a.render.mapping, count, 0, 0);
            void* source_slot = read<void*>(sources);
            const Word source_count = read<Word>(sources, 4);
            if (source_slot != offset(source_slot, source_count * 4u)) {
                do {
                    void* generator = read<void*>(read<void*>(group), 0x0c);
                    const Word target = services.profile(generator)[8 / 4];
                    void* const entry = read<void*>(source_slot);
                    if (target == 0x00b556f0u)
                        write_native_generic_instance_00b556f0(generator, nullptr, entry, destination);
                    else if (target == 0x00b55780u)
                        write_native_building_instance_00b55780(generator, &a.unsigned_bias_00ce3978, entry, destination);
                    else std::terminate();
                    generator = read<void*>(read<void*>(group), 0x0c);
                    void* const declaration = native_instance_generator_declaration_00b556b0(generator);
                    destination = offset(destination, read<Word>(declaration, 0xcc));
                    void* const current_end = end(sources);
                    source_slot = offset(source_slot, 4);
                    if (source_slot == current_end) break;
                } while (true);
            }
            // Native unmap obtains geometry/stream again. A changed publication
            // intentionally does not reuse the object which was mapped above.
            logical = stream(read<void*>(group, 0x1c + category_word));
            if (services.profile(logical)[0x14 / 4] != 0x00b49a80u) std::terminate();
            unlock_native_logical_vertex_stream_00b49a80(logical, *a.render.mapping);
            void* const mesh = geometry(read<void*>(group, 0x1c + category_word));
            void* const section = gui_geometry_element_00b732c0(mesh, 0);
            set_native_section_instance_count_00b85590(section, nullptr, read<Word>(group, 0x0c + category_word));
            detail::initialize_upload_output(group, &output_access, category, mesh, section, command);
            void* queued_entry;
            NativeRenderBatchStorage* batch;
            if (category != 0) {
                queued_entry = read<void*>(group, 0x14 + category_word);
                batch = read<NativeRenderBatchStorage*>(command, 0x10);
            } else {
                const void* const effect = read<void*>(read<void*>(section, 0x20), 0x7c);
                const Word selector = native_effect_batch_index_00b17300(effect);
                queued_entry = read<void*>(group, 0x14);
                batch = read<NativeRenderBatchStorage*>(command, 0x0c + selector * 4u);
            }
            append_native_render_batch_entry_00b51cb0(*batch, queued_entry);
        }
        const Word count = read<Word>(command, 0x3c);
        void* const base = read<void*>(command, 0x38);
        group_slot = offset(group_slot, 4);
        if (group_slot == offset(base, count * 4u)) break;
    } while (true);
}

// Complete generic writer; source/destination overlap follows each original FLD/FSTP.
__declspec(naked) void __fastcall write_native_generic_instance_00b556f0(void*,void*,const void*,void*) {
    __asm {
        mov eax, dword ptr [esp + 4] // 00b556f0
        push esi // 00b556f4
        mov esi, dword ptr [eax + 0xc] // 00b556f5
        test byte ptr [esi + 0x5c], 2 // 00b556f8
        jne loc_00b55705 // 00b556fc
        mov ecx, esi // 00b556fe
        call refresh_native_camera_world_00b6db70 // 00b55700
    loc_00b55705:
        fld dword ptr [esi + 0xf0] // 00b55705
        mov eax, dword ptr [esp + 0xc] // 00b5570b
        fstp dword ptr [eax] // 00b5570f
        fld dword ptr [esi + 0x100] // 00b55711
        fstp dword ptr [eax + 4] // 00b55717
        fld dword ptr [esi + 0x110] // 00b5571a
        fstp dword ptr [eax + 8] // 00b55720
        fld dword ptr [esi + 0x120] // 00b55723
        fstp dword ptr [eax + 0xc] // 00b55729
        fld dword ptr [esi + 0xf4] // 00b5572c
        fstp dword ptr [eax + 0x10] // 00b55732
        fld dword ptr [esi + 0x104] // 00b55735
        fstp dword ptr [eax + 0x14] // 00b5573b
        fld dword ptr [esi + 0x114] // 00b5573e
        fstp dword ptr [eax + 0x18] // 00b55744
        fld dword ptr [esi + 0x124] // 00b55747
        fstp dword ptr [eax + 0x1c] // 00b5574d
        fld dword ptr [esi + 0xf8] // 00b55750
        fstp dword ptr [eax + 0x20] // 00b55756
        fld dword ptr [esi + 0x108] // 00b55759
        fstp dword ptr [eax + 0x24] // 00b5575f
        fld dword ptr [esi + 0x118] // 00b55762
        fstp dword ptr [eax + 0x28] // 00b55768
        fld dword ptr [esi + 0x128] // 00b5576b
        pop esi // 00b55771
        fstp dword ptr [eax + 0x2c] // 00b55772
        ret 8 // 00b55775
    }
}

// Complete building writer. Added EDX binding is held outside native scratch;
// register, x87/SSE load/store order, light-list reloads and branch widths remain.
__declspec(naked) void __fastcall write_native_building_instance_00b55780(void*,const volatile float*,const void*,void*) {
    __asm {
        push edx // Added actual unsigned-bias address; original argument offsets adjusted.
        sub esp,0x30 // 00b55780
        push ebx // 00b55783
        mov ebx,dword ptr [esp + 0x3c] // 00b55784
        push esi // 00b55788
        push edi // 00b55789
        mov edi,dword ptr [ebx + 0xc] // 00b5578a
        test byte ptr [edi + 0x5c],0x2 // 00b5578d
        jnz loc_00b5579a // 00b55791
        mov ecx,edi // 00b55793
        call refresh_native_camera_world_00b6db70 // 00b55795
    loc_00b5579a:
        fld dword ptr [edi + 0xf0] // 00b5579a
        mov esi,dword ptr [esp + 0x48] // 00b557a0
        fstp dword ptr [esi] // 00b557a4
        fld dword ptr [edi + 0x100] // 00b557a6
        fstp dword ptr [esi + 0x4] // 00b557ac
        fld dword ptr [edi + 0x110] // 00b557af
        fstp dword ptr [esi + 0x8] // 00b557b5
        fld dword ptr [edi + 0x120] // 00b557b8
        fstp dword ptr [esi + 0xc] // 00b557be
        fld dword ptr [edi + 0xf4] // 00b557c1
        fstp dword ptr [esi + 0x10] // 00b557c7
        fld dword ptr [edi + 0x104] // 00b557ca
        fstp dword ptr [esi + 0x14] // 00b557d0
        fld dword ptr [edi + 0x114] // 00b557d3
        fstp dword ptr [esi + 0x18] // 00b557d9
        fld dword ptr [edi + 0x124] // 00b557dc
        fstp dword ptr [esi + 0x1c] // 00b557e2
        fld dword ptr [edi + 0xf8] // 00b557e5
        fstp dword ptr [esi + 0x20] // 00b557eb
        fld dword ptr [edi + 0x108] // 00b557ee
        fstp dword ptr [esi + 0x24] // 00b557f4
        fld dword ptr [edi + 0x118] // 00b557f7
        fstp dword ptr [esi + 0x28] // 00b557fd
        fld dword ptr [edi + 0x128] // 00b55800
        fstp dword ptr [esi + 0x2c] // 00b55806
        mov ecx,dword ptr [ebx + 0xc] // 00b55809
        call native_node_point_light_array_00b6dc50 // 00b5580c
        mov ecx,dword ptr [eax + 0x4] // 00b55811
        cmp ecx,0x3 // 00b55814
        xorps xmm0,xmm0 // 00b55817
        jge loc_00b55896 // 00b5581a
        test ecx,ecx // 00b5581c
        mov edx,ecx // 00b5581e
        jbe loc_00b5589d // 00b55820
    loc_00b55822:
        mov ecx,dword ptr [eax] // 00b55822
        mov ecx,dword ptr [ecx] // 00b55824
        movss xmm1,dword ptr [ecx + 0x1ec] // 00b55826
        movss xmm2,dword ptr [ecx + 0x1f0] // 00b5582e
        movss xmm3,dword ptr [ecx + 0x1f4] // 00b55836
        movss xmm4,dword ptr [ecx + 0x1f8] // 00b5583e
        movss dword ptr [esp + 0xc],xmm1 // 00b55846
        mov edi,dword ptr [esp + 0xc] // 00b5584c
        mov dword ptr [esi + 0x30],edi // 00b55850
        movss dword ptr [esp + 0x10],xmm2 // 00b55853
        mov edi,dword ptr [esp + 0x10] // 00b55859
        mov dword ptr [esi + 0x34],edi // 00b5585d
        movss dword ptr [esp + 0x14],xmm3 // 00b55860
        mov edi,dword ptr [esp + 0x14] // 00b55866
        mov dword ptr [esi + 0x38],edi // 00b5586a
        add ecx,0x184 // 00b5586d
        movss dword ptr [esp + 0x18],xmm4 // 00b55873
        mov edi,dword ptr [esp + 0x18] // 00b55879
        mov dword ptr [esi + 0x3c],edi // 00b5587d
        mov edi,dword ptr [ecx] // 00b55880
        mov dword ptr [esi + 0x60],edi // 00b55882
        mov edi,dword ptr [ecx + 0x4] // 00b55885
        mov dword ptr [esi + 0x64],edi // 00b55888
        mov edi,dword ptr [ecx + 0x8] // 00b5588b
        mov dword ptr [esi + 0x68],edi // 00b5588e
        mov ecx,dword ptr [ecx + 0xc] // 00b55891
        jmp loc_00b55902 // 00b55894
    loc_00b55896:
        mov edx,0x3 // 00b55896
        jmp loc_00b55822 // 00b5589b
    loc_00b5589d:
        movss dword ptr [esp + 0x1c],xmm0 // 00b5589d
        mov ecx,dword ptr [esp + 0x1c] // 00b558a3
        mov dword ptr [esi + 0x30],ecx // 00b558a7
        movss dword ptr [esp + 0x20],xmm0 // 00b558aa
        mov ecx,dword ptr [esp + 0x20] // 00b558b0
        mov dword ptr [esi + 0x34],ecx // 00b558b4
        movss dword ptr [esp + 0x24],xmm0 // 00b558b7
        mov ecx,dword ptr [esp + 0x24] // 00b558bd
        mov dword ptr [esi + 0x38],ecx // 00b558c1
        movss dword ptr [esp + 0x28],xmm0 // 00b558c4
        mov ecx,dword ptr [esp + 0x28] // 00b558ca
        mov dword ptr [esi + 0x3c],ecx // 00b558ce
        movss dword ptr [esp + 0x2c],xmm0 // 00b558d1
        mov ecx,dword ptr [esp + 0x2c] // 00b558d7
        mov dword ptr [esi + 0x60],ecx // 00b558db
        movss dword ptr [esp + 0x30],xmm0 // 00b558de
        mov ecx,dword ptr [esp + 0x30] // 00b558e4
        mov dword ptr [esi + 0x64],ecx // 00b558e8
        movss dword ptr [esp + 0x34],xmm0 // 00b558eb
        mov ecx,dword ptr [esp + 0x34] // 00b558f1
        movss dword ptr [esp + 0x38],xmm0 // 00b558f5
        mov dword ptr [esi + 0x68],ecx // 00b558fb
        mov ecx,dword ptr [esp + 0x38] // 00b558fe
    loc_00b55902:
        cmp edx,0x1 // 00b55902
        mov dword ptr [esi + 0x6c],ecx // 00b55905
        jbe loc_00b5597f // 00b55908
        mov ecx,dword ptr [eax] // 00b5590a
        mov ecx,dword ptr [ecx + 0x4] // 00b5590c
        movss xmm1,dword ptr [ecx + 0x1ec] // 00b5590f
        movss xmm2,dword ptr [ecx + 0x1f0] // 00b55917
        movss xmm3,dword ptr [ecx + 0x1f4] // 00b5591f
        movss xmm4,dword ptr [ecx + 0x1f8] // 00b55927
        movss dword ptr [esp + 0xc],xmm1 // 00b5592f
        mov edi,dword ptr [esp + 0xc] // 00b55935
        mov dword ptr [esi + 0x40],edi // 00b55939
        movss dword ptr [esp + 0x10],xmm2 // 00b5593c
        mov edi,dword ptr [esp + 0x10] // 00b55942
        mov dword ptr [esi + 0x44],edi // 00b55946
        movss dword ptr [esp + 0x14],xmm3 // 00b55949
        mov edi,dword ptr [esp + 0x14] // 00b5594f
        mov dword ptr [esi + 0x48],edi // 00b55953
        add ecx,0x184 // 00b55956
        movss dword ptr [esp + 0x18],xmm4 // 00b5595c
        mov edi,dword ptr [esp + 0x18] // 00b55962
        mov dword ptr [esi + 0x4c],edi // 00b55966
        mov edi,dword ptr [ecx] // 00b55969
        mov dword ptr [esi + 0x70],edi // 00b5596b
        mov edi,dword ptr [ecx + 0x4] // 00b5596e
        mov dword ptr [esi + 0x74],edi // 00b55971
        mov edi,dword ptr [ecx + 0x8] // 00b55974
        mov dword ptr [esi + 0x78],edi // 00b55977
        mov ecx,dword ptr [ecx + 0xc] // 00b5597a
        jmp loc_00b559e4 // 00b5597d
    loc_00b5597f:
        movss dword ptr [esp + 0x1c],xmm0 // 00b5597f
        mov ecx,dword ptr [esp + 0x1c] // 00b55985
        mov dword ptr [esi + 0x40],ecx // 00b55989
        movss dword ptr [esp + 0x20],xmm0 // 00b5598c
        mov ecx,dword ptr [esp + 0x20] // 00b55992
        mov dword ptr [esi + 0x44],ecx // 00b55996
        movss dword ptr [esp + 0x24],xmm0 // 00b55999
        mov ecx,dword ptr [esp + 0x24] // 00b5599f
        mov dword ptr [esi + 0x48],ecx // 00b559a3
        movss dword ptr [esp + 0x28],xmm0 // 00b559a6
        mov ecx,dword ptr [esp + 0x28] // 00b559ac
        mov dword ptr [esi + 0x4c],ecx // 00b559b0
        movss dword ptr [esp + 0x2c],xmm0 // 00b559b3
        mov ecx,dword ptr [esp + 0x2c] // 00b559b9
        mov dword ptr [esi + 0x70],ecx // 00b559bd
        movss dword ptr [esp + 0x30],xmm0 // 00b559c0
        mov ecx,dword ptr [esp + 0x30] // 00b559c6
        mov dword ptr [esi + 0x74],ecx // 00b559ca
        movss dword ptr [esp + 0x34],xmm0 // 00b559cd
        mov ecx,dword ptr [esp + 0x34] // 00b559d3
        movss dword ptr [esp + 0x38],xmm0 // 00b559d7
        mov dword ptr [esi + 0x78],ecx // 00b559dd
        mov ecx,dword ptr [esp + 0x38] // 00b559e0
    loc_00b559e4:
        cmp edx,0x2 // 00b559e4
        mov dword ptr [esi + 0x7c],ecx // 00b559e7
        jbe loc_00b55a69 // 00b559ea
        mov eax,dword ptr [eax] // 00b559ec
        mov eax,dword ptr [eax + 0x8] // 00b559ee
        movss xmm0,dword ptr [eax + 0x1ec] // 00b559f1
        movss xmm1,dword ptr [eax + 0x1f0] // 00b559f9
        movss xmm2,dword ptr [eax + 0x1f4] // 00b55a01
        movss xmm3,dword ptr [eax + 0x1f8] // 00b55a09
        movss dword ptr [esp + 0xc],xmm0 // 00b55a11
        mov ecx,dword ptr [esp + 0xc] // 00b55a17
        mov dword ptr [esi + 0x50],ecx // 00b55a1b
        movss dword ptr [esp + 0x10],xmm1 // 00b55a1e
        mov ecx,dword ptr [esp + 0x10] // 00b55a24
        mov dword ptr [esi + 0x54],ecx // 00b55a28
        movss dword ptr [esp + 0x14],xmm2 // 00b55a2b
        mov ecx,dword ptr [esp + 0x14] // 00b55a31
        mov dword ptr [esi + 0x58],ecx // 00b55a35
        add eax,0x184 // 00b55a38
        movss dword ptr [esp + 0x18],xmm3 // 00b55a3d
        mov ecx,dword ptr [esp + 0x18] // 00b55a43
        mov dword ptr [esi + 0x5c],ecx // 00b55a47
        mov ecx,dword ptr [eax] // 00b55a4a
        mov dword ptr [esi + 0x80],ecx // 00b55a4c
        mov ecx,dword ptr [eax + 0x4] // 00b55a52
        mov dword ptr [esi + 0x84],ecx // 00b55a55
        mov ecx,dword ptr [eax + 0x8] // 00b55a5b
        mov dword ptr [esi + 0x88],ecx // 00b55a5e
        mov eax,dword ptr [eax + 0xc] // 00b55a64
        jmp loc_00b55ad7 // 00b55a67
    loc_00b55a69:
        movss dword ptr [esp + 0x1c],xmm0 // 00b55a69
        mov ecx,dword ptr [esp + 0x1c] // 00b55a6f
        mov dword ptr [esi + 0x50],ecx // 00b55a73
        movss dword ptr [esp + 0x20],xmm0 // 00b55a76
        mov eax,dword ptr [esp + 0x20] // 00b55a7c
        mov dword ptr [esi + 0x54],eax // 00b55a80
        movss dword ptr [esp + 0x24],xmm0 // 00b55a83
        mov ecx,dword ptr [esp + 0x24] // 00b55a89
        mov dword ptr [esi + 0x58],ecx // 00b55a8d
        movss dword ptr [esp + 0x28],xmm0 // 00b55a90
        mov eax,dword ptr [esp + 0x28] // 00b55a96
        movss dword ptr [esp + 0x2c],xmm0 // 00b55a9a
        mov ecx,dword ptr [esp + 0x2c] // 00b55aa0
        mov dword ptr [esi + 0x5c],eax // 00b55aa4
        mov dword ptr [esi + 0x80],ecx // 00b55aa7
        movss dword ptr [esp + 0x30],xmm0 // 00b55aad
        mov eax,dword ptr [esp + 0x30] // 00b55ab3
        movss dword ptr [esp + 0x34],xmm0 // 00b55ab7
        mov ecx,dword ptr [esp + 0x34] // 00b55abd
        mov dword ptr [esi + 0x84],eax // 00b55ac1
        movss dword ptr [esp + 0x38],xmm0 // 00b55ac7
        mov eax,dword ptr [esp + 0x38] // 00b55acd
        mov dword ptr [esi + 0x88],ecx // 00b55ad1
    loc_00b55ad7:
        test edx,edx // 00b55ad7
        mov dword ptr [esi + 0x8c],eax // 00b55ad9
        fld dword ptr [ebx + 0x18] // 00b55adf
        mov dword ptr [esp + 0x44],edx // 00b55ae2
        fstp dword ptr [esi + 0x6c] // 00b55ae6
        fild dword ptr [esp + 0x44] // 00b55ae9
        jge loc_00b55af5 // 00b55aed
        mov eax,dword ptr [esp + 0x3c] // Added saved actual constant pointer.
        fadd dword ptr [eax] // 00b55aef
    loc_00b55af5:
        fstp dword ptr [esi + 0x7c] // 00b55af5
        mov ecx,dword ptr [ebx + 0x4] // 00b55af8
        mov ecx,dword ptr [ecx + 0x20] // 00b55afb
        push 0x0 // 00b55afe
        call diffuse_bridge // 00b55b00
        fld dword ptr [eax + 0xc] // 00b55b05
        pop edi // 00b55b08
        fstp dword ptr [esi + 0x8c] // 00b55b09
        pop esi // 00b55b0f
        pop ebx // 00b55b10
        add esp,0x34 // 00b55b11
        ret 0x8 // 00b55b14
    }
}
} // namespace bsp
