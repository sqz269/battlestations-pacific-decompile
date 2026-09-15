#include "bsp/native_resource_postprocess_full.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_material_pass_base.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_node_animator_lifetime.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_skin_model_bindings.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include <Windows.h>
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource postprocessing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeResourcePostprocessContext;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> T read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
template<class T> void write(void* p, Word offset, T value) noexcept {
    *static_cast<volatile T*>(at(p, offset)) = value;
}
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, 4); return result;
}
Word distance(const void* begin, const void* end, unsigned shift) noexcept {
    const auto delta = reinterpret_cast<Word>(end) - reinterpret_cast<Word>(begin);
    return static_cast<Word>(signed_word(delta) >> shift);
}
std::int32_t grown(Word capacity) noexcept {
    const auto result = signed_word(capacity * 2u);
    return result > 1 ? result : 1;
}
void check_index(void* vector, Word index, unsigned shift) {
    const auto begin = read<void*>(vector, 4);
    if (!begin || index >= distance(begin, read<void*>(vector, 8), shift))
        _invalid_parameter_noinfo();
}
void check_head(void* owner, void* node) {
    if (node == read<void*>(owner, 4)) _invalid_parameter_noinfo();
}
void check_range(void* owner, void* node, Word index, bool check_owner) {
    if (check_owner && !owner) _invalid_parameter_noinfo();
    check_head(owner, node);
    check_index(at(node, 0x10), index, 3);
}
void* node_at(void* instance, Word index) {
    check_index(at(instance, 0x10), index, 2);
    return read<void*>(read<void*>(instance, 0x14), index * 4u);
}
void retain(void* object) {
    InterlockedIncrement(static_cast<volatile LONG*>(at(object, 4)));
}
void release(void* object, Context& context) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(object, 4))) == 0) {
        const auto target = context.calls.table(read<Word>(object))[0];
        context.calls.destroy(target, object);
    }
}
void assign_registry(void* animator, void* registry, bool nullable, Context& context) {
    void* const old = read<void*>(animator, 0x2c);
    if (old == registry) return;
    write(animator, 0x2c, registry);
    if (!nullable || registry) retain(registry);
    if (old) release(old, context);
}
void* make_registry() {
    void* const memory = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
    try { return memory ? construct_native_animation_registry_00b79a80(memory) : nullptr; }
    catch (...) { singleton_lifetime_free(memory); throw; }
}
void* make_compact(void* node, Context& context) {
    void* const memory = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x38, 0x38});
    try {
        if (!memory) return nullptr;
        void* compatible = nullptr;
        if (node) {
            const auto table = context.calls.table(read<Word>(node));
            const Word token = native_skin_model_type_00b8f920(context.skin_model_type_01090344);
            if (context.calls.is_type(table[3], node, token)) compatible = node;
        }
        initialize_native_compact_animator_fragment_00b79d44(memory, compatible);
        return memory;
    } catch (...) { singleton_lifetime_free(memory); throw; }
}
void compact_phase(void* instance, NativeResourcePostprocessRange& range, Context& context) {
    void* const registry = make_registry();
    void* const entry = range.node;
    const auto count = range.count;
    for (Word i = 0; i < count; ++i) {
        check_range(range.owner, entry, i, true);
        check_range(range.owner, entry, i, false);
        void* const item = read<void*>(read<void*>(entry, 0x14), i * 8u + 4u);
        register_native_compact_track_names_00b925d0(item, registry, context.registry);
    }
    for (Word i = 0; signed_word(i) < get_native_resource_node_count_00b76510(instance); ++i) {
        void* const node = node_at(instance, i);
        void* const animator = make_compact(node, context);
        for (Word j = 0; j < count; ++j) {
            check_range(range.owner, entry, j, true);
            if (read<void*>(read<void*>(entry, 0x14), j * 8u) != node) continue;
            check_range(range.owner, entry, j, false);
            write(animator, 0x34, read<void*>(read<void*>(entry, 0x14), j * 8u + 4u));
            break;
        }
        assign_native_node_animator_00b6ee80(node, context.calls, animator);
        assign_registry(animator, registry, true, context);
        release(animator, context);
    }
    if (registry) release(registry, context);
}
// The second head comparison precedes the node load, and a returning handler
// therefore sees the node already captured. Item storage is reloaded afterward.
void pair_at(void* owner, void* entry, Word index, void*& node, void*& item, bool owner_check) {
    check_range(owner, entry, index, owner_check);
    const bool invalid_head = entry == read<void*>(owner, 4);
    node = read<void*>(read<void*>(entry, 0x14), index * 8u);
    if (invalid_head) _invalid_parameter_noinfo();
    check_index(at(entry, 0x10), index, 3);
    item = read<void*>(read<void*>(entry, 0x14), index * 8u + 4u);
}
void collect_tracks(NativePostprocessNodeTracksArray& rows,
    NativeResourcePostprocessRange& range, Context& context, void* registry) {
    void* const entry = range.node;
    for (Word i = 0; i < range.count; ++i) {
        void* node; void* item;
        pair_at(range.owner, entry, i, node, item, true);
        const auto count = read<std::int32_t>(&rows, 4);
        auto* cursor = read<NativePostprocessNodeTracks*>(&rows);
        for (std::int32_t j = 0; j < count; ++j, ++cursor) {
            if (read<void*>(cursor) != node) continue;
            auto& header = read<NativePostprocessNodeTracks*>(&rows)[j].items_04;
            volatile auto& current = header;
            if (current.count_04 == current.capacity_08)
                reserve_native_instance_entry_pointers_00b1c500(header, grown(current.capacity_08));
            write(current.data_00, static_cast<Word>(current.count_04) * 4u, item);
            current.count_04 = signed_word(static_cast<Word>(current.count_04) + 1u);
            break;
        }
        register_native_track_names_00b8a330(item, registry, context.registry);
    }
}
void populate_animators(NativePostprocessNodeTracksArray& rows, void* registry, Context& context) {
    auto* row = read<NativePostprocessNodeTracks*>(&rows);
    auto remaining = read<std::int32_t>(&rows, 4);
    while (remaining > 0) {
        void* const node = read<void*>(row);
        void* const animator = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x30, 0x30});
        if (animator) initialize_native_track_animator_fragment_00b7a0ba(animator);
        const auto names = read<std::int32_t>(registry, 0x10);
        auto& tracks = *static_cast<NativeRenderPointerArrayStorage*>(at(animator, 0x20));
        resize_native_instance_entry_pointers_00b1c770(tracks, names);
        for (Word j = 0; signed_word(j) < read<std::int32_t>(animator, 0x24); ++j)
            write(read<void*>(animator, 0x20), j * 4u, static_cast<void*>(nullptr));
        assign_native_node_animator_00b6ee80(node, context.calls, animator);
        assign_registry(animator, registry, false, context);
        release(animator, context);
        for (Word j = 0; signed_word(j) < read<std::int32_t>(row, 8); ++j) {
            void* const item = read<void*>(read<void*>(row, 4), j * 4u);
            for (Word k = 0; signed_word(k) < native_track_item_count_00b8a2d0(item); ++k) {
                void* const current_animator = read<void*>(node, 0x130);
                void* const track = native_track_item_at_00b8a370(item, nullptr, k);
                void* const tree = at(read<void*>(current_animator, 0x2c), 8);
                NativeFileStoreNameIterator found{};
                find_native_file_store_name_00be5a50(tree, &found, at(track, 8), context.registry.invalid_parameters);
                void* const head = read<void*>(tree, 4);
                if (!found.owner_00 || found.owner_00 != tree) _invalid_parameter_noinfo();
                Word index = 0xffffffffu;
                if (found.node_04 != head) {
                    if (!found.owner_00) _invalid_parameter_noinfo();
                    check_head(found.owner_00, found.node_04);
                    index = read<Word>(found.node_04, 0x14);
                }
                write(read<void*>(current_animator, 0x20), index * 4u, track);
            }
        }
        ++row; --remaining;
    }
}
bool equal_name(const void* left, const void* right) {
    if (read<Word>(left) != read<Word>(right)) return false;
    if (!read<Word>(left)) return true;
    return _stricmp(read<const char*>(left, 4), read<const char*>(right, 4)) == 0;
}
Word spill_scalar(const void* item) {
    Word result;
    // Preserve the callee ST0 result and the caller's one float32 argument spill.
    __asm { mov ecx, item }
    __asm { call native_skin_item_scalar_00b8a1b0 }
    __asm { fstp dword ptr result }
    return result;
}
void copy_x87(void* destination, const void* source) {
    __asm { mov eax, source }
    __asm { mov ecx, destination }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [ecx] }
}
void skin_model(void* instance, void* model, NativeResourcePostprocessRange& range, Context& context) {
    NativePostprocessPairArray pairs{nullptr, 0, 0};
    try {
        void* const entry = range.node;
        for (Word i = 0; i < range.count; ++i) {
            check_range(range.owner, entry, i, true);
            void* const initial = read<void*>(read<void*>(entry, 0x14), i * 8u);
            void* const parent = read<void*>(initial, 0x30);
            if (!parent || (parent != model && !native_node_has_ancestor_00b75f00(parent, model))) continue;
            void* node; void* item;
            pair_at(range.owner, entry, i, node, item, false);
            volatile auto& current = pairs;
            if (current.count_04 == current.capacity_08)
                reserve_native_material_pass_pairs_00b40c80(pairs, grown(current.capacity_08));
            const auto offset = static_cast<Word>(current.count_04) * 8u;
            write(current.data_00, offset, node);
            write(current.data_00, offset + 4u, item);
            current.count_04 = signed_word(static_cast<Word>(current.count_04) + 1u);
        }
        resize_native_skin_model_bindings_00b91590(model, pairs.count_04, context.calls);
        void* const geometry = gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(at(model, 0x174)), 0);
        auto* const names = get_native_mesh_weight_names_00b72730(geometry);
        for (Word i = 0; i < native_resource_instance_item_count_00b87200(instance); ++i) {
            void* const item = native_resource_instance_item_at_00b87260(instance, nullptr, i);
            const auto table = context.calls.table(read<Word>(item));
            const Word token = context.skin_item_type_01090278;
            if (!context.calls.is_type(table[3], item, token)) continue;
            void* matched = nullptr;
            for (Word j = 0; signed_word(j) < read<std::int32_t>(&pairs, 4); ++j) {
                void* const backing = read<void*>(&pairs);
                if (read<void*>(backing, j * 8u + 4u) == item) {
                    matched = read<void*>(backing, j * 8u); break;
                }
            }
            if (!matched) continue;
            for (Word j = 0; signed_word(j) < read<std::int32_t>(names, 4); ++j) {
                const void* const name = native_skin_item_name_00b8a180(item);
                const void* const weight = at(read<void*>(names), j * 8u);
                if (!equal_name(weight, name) || read<void*>(read<void*>(model, 0x184), j * 0x60u)) continue;
                const Word scalar = spill_scalar(item);
                const void* const angles = native_skin_item_angles_00b8a1a0(item);
                const void* const translation = native_skin_item_translation_00b8a190(item);
                bind_native_skin_node_00b90c30(model, j, matched, translation, angles, scalar, context.calls);
            }
        }
        finalize_native_skin_bindings_00b91000(model);
    } catch (...) { destroy_native_postprocess_pairs_00b77cd0(pairs); throw; }
    // Native state drops to3 BEFORE normal cleanup, so a cleanup failure must
    // only unwind the outer row array, without destroying this pair array twice.
    destroy_native_postprocess_pairs_00b77cd0(pairs);
}
void pose_phase(void* instance, NativePostprocessNodeTracksArray& rows, Context& context) {
    NativeResourcePostprocessRange range;
    get_native_mesh_binding_range_00b87ce0(&range, &context.skin_item_type_01090278, instance);
    auto* row = read<NativePostprocessNodeTracks*>(&rows);
    auto remaining = read<std::int32_t>(&rows, 4);
    while (remaining > 0) {
        void* const node = read<void*>(row);
        const auto table = context.calls.table(read<Word>(node));
        const Word token = native_skin_model_type_00b8f920(context.skin_model_type_01090344);
        if (context.calls.is_type(table[3], node, token)) {
            skin_model(instance, node, range, context);
        } else if (read<void*>(node, 0x130)) {
            float matrix[16];
            std::memcpy(matrix, get_native_node_local_matrix_00b6db60(node), sizeof(matrix));
            void* const animator = read<void*>(node, 0x130);
            for (Word j = 0; j < 3; ++j) write(animator, 8u + j * 4u, read<Word>(matrix, 0x30u + j * 4u));
            float angles[3];
            extract_native_animator_angles_00b630f0(matrix, &context.axes_crt, angles);
            for (Word j = 0; j < 3; ++j) copy_x87(at(animator, 0x14u + j * 4u), angles + j);
        }
        ++row; --remaining;
    }
}
void track_phase(void* instance, NativeResourcePostprocessRange& range, Context& context) {
    void* const registry = make_registry();
    NativePostprocessNodeTracksArray rows{nullptr, 0, 0};
    try {
        void* const begin = read<void*>(instance, 0x14);
        const auto count = begin ? signed_word(distance(begin, read<void*>(instance, 0x18), 2)) : 0;
        resize_native_postprocess_node_tracks_00b78d70(rows, nullptr, count);
        auto* cursor = read<NativePostprocessNodeTracks*>(&rows);
        for (Word i = 0; signed_word(i) < get_native_resource_node_count_00b76510(instance); ++i, ++cursor)
            write(cursor, 0, node_at(instance, i));
        collect_tracks(rows, range, context, registry);
        populate_animators(rows, registry, context);
        pose_phase(instance, rows, context);
        release(registry, context);
    } catch (...) { destroy_native_postprocess_node_tracks_00b78f60(rows); throw; }
    destroy_native_postprocess_node_tracks_00b78f60(rows);
}
void finalize_phase(void* instance, Context& context) {
    for (Word i = 0; signed_word(i) < get_native_resource_node_count_00b76510(instance); ++i) {
        void* const animator = read<void*>(node_at(instance, i), 0x130);
        if (!animator) continue;
        const auto target = context.calls.table(read<Word>(animator))[8];
        if (target == 0x00b77990u) finalize_native_track_animator_00b77990(animator);
        else if (target == 0x00b75ee0u) finalize_native_compact_animator_00b75ee0(animator);
        else context.calls.finalize_animator(target, animator);
    }
}
void camera_phase(void* instance, Context& context) {
    NativeResourcePostprocessRange range;
    get_native_mesh_binding_range_00b87ce0(&range, &context.camera_type_01090288, instance);
    for (Word i = 0; i < range.count; ++i) {
        void* camera; void* item;
        pair_at(range.owner, range.node, i, camera, item, true);
        struct EmptyName { Word length; char* data; } empty{0, nullptr};
        resize_native_string_header_0041dd40(&empty, context.registry.strings, 0, true);
        if (empty.data) std::memcpy(empty.data, "", empty.length + 1u);
        const void* const target_name = at(item, 0x10);
        // Native comparison here uses empty-string ordering, not length equality.
        bool different;
        if (!read<Word>(target_name)) different = empty.length != 0;
        else if (!empty.length) different = true;
        else different = _stricmp(read<const char*>(target_name, 4), empty.data) != 0;
        destroy_native_string_header_0041dd20(&empty, context.registry.strings);
        if (different) {
            for (Word j = 0; signed_word(j) < get_native_resource_node_count_00b76510(instance); ++j) {
                void* const node = node_at(instance, j);
                const auto& name = native_node_name_00b6d800(*static_cast<NativeNodeStorage*>(node));
                if (!equal_name(target_name, &name)) continue;
                check_index(at(instance, 0x10), j, 2);
                void* const begin = read<void*>(instance, 0x14);
                void* const old = read<void*>(camera, 0x438);
                void* const incoming = read<void*>(begin, j * 4u);
                if (old) {
                    release(old, context);
                    write(camera, 0x438, static_cast<void*>(nullptr));
                }
                write(camera, 0x438, incoming);
                if (incoming) retain(incoming);
                break;
            }
        }
        CameraProjection projection(CameraProjectionBacking{
            *static_cast<float*>(at(camera, 0x1c4)), *static_cast<float*>(at(camera, 0x1c8)),
            *static_cast<float*>(at(camera, 0x1d4)), *static_cast<float*>(at(camera, 0x1d8)),
            *static_cast<CameraMatrix*>(at(camera, 0x1e0)), *static_cast<CameraMatrix*>(at(camera, 0x2a0)),
            *static_cast<Word*>(at(camera, 0x2f0))});
        float fov; copy_x87(&fov, at(item, 8));
        set_camera_fov_00b6fbb0(projection, fov);
        float aspect; copy_x87(&aspect, at(item, 0xc));
        set_camera_aspect_00b6fbd0(projection, aspect);
    }
}
} // namespace

void __fastcall postprocess_native_resource_instance_00b79bc0(void* instance, Context& context) {
    NativeResourcePostprocessRange compact;
    get_native_mesh_binding_range_00b87ce0(&compact, &context.compact_type_0109042c, instance);
    if (compact.count) compact_phase(instance, compact, context);
    NativeResourcePostprocessRange tracks;
    get_native_mesh_binding_range_00b87ce0(&tracks, &context.track_type_01090268, instance);
    if (tracks.count) track_phase(instance, tracks, context);
    finalize_phase(instance, context);
    camera_phase(instance, context);
}
std::uint32_t native_skin_model_type_00b8f920(const volatile Word& current) noexcept { return current; }
__declspec(naked) const void* __fastcall native_skin_item_name_00b8a180(const void*) noexcept {
    __asm { lea eax, [ecx + 8] }
    __asm { ret }
}
__declspec(naked) const void* __fastcall native_skin_item_translation_00b8a190(const void*) noexcept {
    __asm { lea eax, [ecx + 10h] }
    __asm { ret }
}
__declspec(naked) const void* __fastcall native_skin_item_angles_00b8a1a0(const void*) noexcept {
    __asm { lea eax, [ecx + 1ch] }
    __asm { ret }
}
__declspec(naked) float __fastcall native_skin_item_scalar_00b8a1b0(const void*) noexcept {
    __asm { fld dword ptr [ecx + 28h] }
    __asm { ret }
}
void __fastcall destroy_native_postprocess_node_tracks_00b78f60(NativePostprocessNodeTracksArray& rows) {
    resize_native_postprocess_node_tracks_00b78d70(rows, nullptr, 0);
    singleton_lifetime_free(read<void*>(&rows));
}
void __fastcall destroy_native_postprocess_pairs_00b77cd0(NativePostprocessPairArray& pairs) {
    resize_native_material_pass_pairs_00b40e00(pairs, 0);
    singleton_lifetime_free(read<void*>(&pairs));
}
} // namespace bsp
