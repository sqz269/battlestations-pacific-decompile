#include "bsp/native_resource_instance_bones.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_renderer_indexed_draw.hpp"
#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource bone binding requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Tree = detail::TreeInsertAccess<0x20, 0x21>;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T = Word> T read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
void put(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
Word distance(const void* header, Word shift) noexcept {
    const Word bytes = read(header, 8) - read(header, 4);
    return (bytes >> shift) | ((bytes & 0x80000000u) ? (~0u << (32u - shift)) : 0u);
}
void invalid() { _invalid_parameter_noinfo(); }
void* geometry(void* model) noexcept {
    return gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(at(model, 0x174)), 0);
}
Word slot(void* owner, Word offset, NativeResourceInstanceBoneContext& context) noexcept {
    return context.calls.table(read(owner))[offset / 4u];
}
void* declaration(void* stream, NativeResourceInstanceBoneContext& context) {
    const Word target = slot(stream, 0x24, context);
    if (target == 0x00b48ce0u)
        return native_logical_vertex_stream_get_declaration_00b48ce0(stream);
    return context.calls.stream_declaration(target, stream);
}
void* map(void* stream, NativeResourceInstanceBoneContext& context) {
    const Word target = slot(stream, 0x10, context);
    if (target == 0x00b49980u)
        return lock_native_logical_vertex_stream_00b49980(stream, context.mapping, 0, 0, 1);
    return context.calls.stream_map(target, stream, 0, 0, 1);
}
Word count(void* stream, Word target, NativeResourceInstanceBoneContext& context) {
    if (target == 0x00b48cd0u) return native_logical_vertex_count_00b48cd0(stream);
    return context.calls.stream_count(target, stream);
}
void unmap(void* stream, NativeResourceInstanceBoneContext& context) {
    const Word target = slot(stream, 0x14, context);
    if (target == 0x00b49a80u)
        unlock_native_logical_vertex_stream_00b49a80(stream, context.mapping);
    else context.calls.stream_unmap(target, stream);
}
void bounds(void* node, void* output, NativeResourceInstanceBoneContext& context) {
    const Word target = slot(node, 0x4c, context);
    if (target == 0x00b6dc20u) read_native_node_local_bounds_00b6dc20(node, nullptr, output);
    else context.calls.node_bounds(target, node, output);
}
std::int32_t truncate_bone_index(const void* value) noexcept {
    std::int32_t result;
    __asm { mov eax, value }
    __asm { cvttss2si eax, dword ptr [eax] }
    __asm { mov result, eax }
    return result;
}
} // namespace

NativeResourceMeshBindingRange* __fastcall get_native_mesh_binding_range_00b87ce0(
    NativeResourceMeshBindingRange* output, const volatile Word* current_type, void* instance) {
    output->instance = instance;
    output->owner = nullptr;
    output->node = nullptr;
    const Word key = read_native_mesh_binding_type_00b931b0(*current_type);
    void* const tree = at(instance, 0x30);
    void* selected = detail::lower_bound_tree_node<Tree>(tree,
        [key](void* node) { return read(node, 0x0c) < key; });
    if (selected == Tree::head(tree) || key < read(selected, 0x0c)) selected = Tree::head(tree);
    output->owner = tree;
    output->node = selected;
    void* const head = read<void*>(instance, 0x34);
    if (!output->owner) invalid();
    if (output->owner != tree) invalid();
    if (output->node == head) {
        output->count = 0;
        return output;
    }
    if (!output->owner) invalid();
    if (output->node == Tree::head(output->owner)) invalid();
    output->count = read(output->node, 0x14) ? distance(at(output->node, 0x10), 3) : 0;
    return output;
}

__declspec(naked) NativeMeshWeightNamesStorage* __fastcall get_native_mesh_weight_names_00b72730(void*) noexcept {
    __asm { lea eax, [ecx + 0b0h] }
    __asm { ret }
}
Word read_native_mesh_binding_type_00b931b0(const volatile Word& current) noexcept {
    return current;
}
std::int32_t native_mesh_vertex_stream_count_00b72b20(const NativeMeshStorage& mesh) noexcept {
    return read<std::int32_t>(&mesh, 0x7c);
}
__declspec(naked) std::int32_t __fastcall get_native_resource_node_count_00b76510(const void*) noexcept {
    __asm {
        mov edx, dword ptr [ecx + 14h]
        test edx, edx
        jnz nonempty
        xor eax, eax
        ret
    nonempty:
        mov eax, dword ptr [ecx + 18h]
        sub eax, edx
        sar eax, 2
        ret
    }
}
__declspec(naked) void __fastcall set_native_model_bone_node_00b90600(void*, void*, Word, void*) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, dword ptr [ecx + 184h]
        mov ecx, dword ptr [esp + 8]
        mov dword ptr [eax + edx * 4], ecx
        ret 8
    }
}
__declspec(naked) void* __fastcall get_native_model_bone_node_00b90620(void*, void*, Word) noexcept {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [ecx + 184h]
        mov eax, dword ptr [ecx + eax * 4]
        ret 4
    }
}
void resize_native_model_bone_nodes_00b90e30(void* model, std::int32_t requested) {
    resize_native_instance_entry_pointers_00b1c770(*static_cast<NativeRenderPointerArrayStorage*>(at(model, 0x184)), requested);
}

__declspec(naked) void* __fastcall read_native_node_local_bounds_00b6dc20(const void*, void*, void*) noexcept {
    __asm {
        fld dword ptr [ecx + 0x18]
        mov eax, dword ptr [esp + 4]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 0x1c]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 0x20]
        fstp dword ptr [eax + 8]
        fld dword ptr [ecx + 0x24]
        fstp dword ptr [eax + 0xc]
        fld dword ptr [ecx + 0x28]
        fstp dword ptr [eax + 0x10]
        fld dword ptr [ecx + 0x2c]
        fstp dword ptr [eax + 0x14]
        ret 4
    }
}

__declspec(naked) void* __fastcall include_native_aabb3_point_00427d10(void*, void*, const void*) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        fld dword ptr [edx]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [ecx]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427d30
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx], xmm0
    native_00427d30:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [ecx + 4]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427d4f
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 4], xmm0
    native_00427d4f:
        fld dword ptr [edx + 8]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [ecx + 8]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427d6e
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 8], xmm0
    native_00427d6e:
        fld dword ptr [edx]
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 0xc]
        fld dword ptr [esp + 4]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427d8c
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 0xc], xmm0
    native_00427d8c:
        fld dword ptr [edx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 0x10]
        fld dword ptr [esp + 4]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427dab
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 0x10], xmm0
    native_00427dab:
        fld dword ptr [edx + 8]
        mov eax, ecx
        fstp dword ptr [esp + 4]
        fld dword ptr [ecx + 0x14]
        fld dword ptr [esp + 4]
        fcomip st, st(1)
        fstp st(0)
        jbe short native_00427dcc
        movss xmm0, dword ptr [esp + 4]
        movss dword ptr [ecx + 0x14], xmm0
    native_00427dcc:
        ret 4
    }
}

void __fastcall bind_native_resource_mesh_bones_00b87e80(void* instance,
    NativeResourceInstanceBoneContext& context) {
    NativeResourceMeshBindingRange range;
    get_native_mesh_binding_range_00b87ce0(&range, &context.mesh_type_01090468, instance);
    for (Word ordinal = 0; ordinal < range.count; ++ordinal) {
        if (!range.owner) invalid();
        if (range.node == Tree::head(range.owner)) invalid();
        if (!read(range.node, 0x14) || ordinal >= distance(at(range.node, 0x10), 3)) invalid();
        void* const model = read<void*>(read<void*>(range.node, 0x14), ordinal * 8u);
        auto* const names = get_native_mesh_weight_names_00b72730(geometry(model));
        resize_native_model_bone_nodes_00b90e30(model, read<std::int32_t>(names, 4));
        // B87F14/B87FFF re-read count; B87F21 re-reads backing for EACH name.
        for (std::int32_t n = 0; n < read<std::int32_t>(names, 4); ++n) {
            const void* const wanted = at(read<void*>(names), static_cast<Word>(n) * 8u);
            for (std::int32_t index = 0; index < get_native_resource_node_count_00b76510(instance); ++index) {
                if (!read(instance, 0x14) || static_cast<Word>(index) >= distance(at(instance, 0x10), 2)) invalid();
                void* const node = read<void*>(read<void*>(instance, 0x14), static_cast<Word>(index) * 4u);
                const auto& name = native_node_name_00b6d800(*static_cast<NativeNodeStorage*>(node));
                const Word candidate_length = read(&name);
                const Word wanted_length = read(wanted);
                if (candidate_length != wanted_length) continue;
                if (candidate_length != 0) {
                    const char* const wanted_data = read<const char*>(wanted, 4);
                    const char* const candidate_data = read<const char*>(&name, 4);
                    if (context.compare_names_00bf7fbf(candidate_data, wanted_data) != 0) continue;
                }
                set_native_model_bone_node_00b90600(model, nullptr, static_cast<Word>(n), node);
                const volatile Word* const predicate_cell = context.calls.table(read(node)) + 3;
                const Word token = context.model_type_01090034.own_id;
                const Word predicate = *predicate_cell;
                if (context.calls.node_type(predicate, node, token) == 0) {
                    const Word minimum = context.empty_minimum_00ce4970;
                    put(node, 0x18, minimum); put(node, 0x1c, minimum); put(node, 0x20, minimum);
                    const Word maximum = context.empty_maximum_00ce4adc;
                    put(node, 0x24, maximum); put(node, 0x28, maximum); put(node, 0x2c, maximum);
                }
                break;
            }
        }
        auto* const mesh = static_cast<NativeMeshStorage*>(geometry(model));
        if (!mesh) continue;
        const std::int32_t streams = native_mesh_vertex_stream_count_00b72b20(*mesh);
        for (std::int32_t index = 0; index < streams; ++index) {
            void* const stream = native_mesh_vertex_stream_00b73260(*mesh, static_cast<Word>(index));
            if (!native_vertex_declaration_has_semantic_00b47c90(declaration(stream, context), 2, 0)) continue;
            const Word bone_offset = native_vertex_declaration_semantic_offset_00b47c40(declaration(stream, context), 2, 0);
            void* const mapped = map(stream, context);
            const volatile Word* const profile = context.calls.table(read(stream)); // B8808B, before stride.
            const Word stride = read(stream, 0x0c);
            const Word count_target = profile[0x20 / 4];
            const Word vertices = count(stream, count_target, context);
            void* cursor = at(mapped, bone_offset);
            for (std::int32_t vertex = 0; vertex < static_cast<std::int32_t>(vertices); ++vertex) {
                const auto bone_index = static_cast<Word>(truncate_bone_index(cursor));
                void* const bone = get_native_model_bone_node_00b90620(model, nullptr, bone_index);
                float box[6];
                bounds(bone, box, context);
                float position[3];
                if (!read_native_vertex_position_004768d0(stream, static_cast<Word>(vertex), position, context.half_import))
                    throw std::logic_error("Bone bounds reached an uninitialized native position-reader format");
                include_native_aabb3_point_00427d10(box, nullptr, position);
                cursor = at(cursor, stride);
                for (Word lane = 0; lane < 6; ++lane) put(bone, 0x18 + lane * 4u, read(box, lane * 4u));
            }
            unmap(stream, context);
        }
    }
}
} // namespace bsp
