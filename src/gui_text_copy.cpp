#include "bsp/gui_text_copy.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
NativeModelOwner& actual_model(NativeNodeBinding* node, GuiTextBufferServices& services) {
    require(node != nullptr, "Text cursor requires a nonnull actual Model");
    auto* lifetime = services.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node->storage));
    auto* reference = dynamic_cast<NativeModelReference*>(lifetime);
    require(reference != nullptr, "Text cursor requires the canonical Model reference");
    auto& model = reference->model_owner();
    require(&model.node == node && &model.storage.node == &node->storage &&
        &model.environment.nodes == &services.parenting.nodes &&
        &model.environment.retained_owners == &services.geometry.actual_owners() &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text cursor Model storage and ownership domains must agree");
    return model;
}
float current_sentinel(GuiTextBufferServices& services) noexcept {
    const auto bits = services.widgets.environment().models.constants.unchanged_00d7a260;
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
float current_x87_sentinel(GuiTextBufferServices& services) noexcept {
    const auto* from = &services.widgets.environment().models.constants.unchanged_00d7a260;
    float value;
    __asm {
        mov eax, from
        fld dword ptr [eax]
        fstp dword ptr value
    }
    return value;
}
template<class Pointer> void release_creator(Pointer*& acquired, NativeRenderActualOwners& owners) {
    auto* captured = std::exchange(acquired, nullptr);
    release_native_render_actual_owner(owners, captured);
}
} // namespace

void ensure_gui_text_cursor_00ab8910(GuiTextLifetime& lifetime,
    GuiTextCursorServices& services, GuiTextCursorAcquired& acquired) {
    require(acquired.phase == GuiTextCursorPhase::not_started,
        "Text cursor creation cannot replay a completed or interrupted frame");
    auto binding = lifetime.content_binding();
    auto& buffers = services.buffers;
    auto& widget = binding.widget;
    require(widget.text_lifetime() == &lifetime &&
        widget.layout().type == GuiWidgetType::Text && widget.layout().transform.type_id == 3 &&
        &buffers.widgets.owner(widget.layout()) == &widget,
        "Text cursor requires its same canonical Text lifetime");
    auto& cursor = lifetime.fields().pointer_184;
    if (cursor) { acquired.phase = GuiTextCursorPhase::complete; return; }
    require(&buffers.widgets.environment().models.nodes == &buffers.parenting.nodes &&
        &buffers.widgets.environment().models.retained_owners == &buffers.geometry.actual_owners() &&
        &buffers.materials.retained_owners == &buffers.geometry.actual_owners(),
        "Text cursor services must share actual node and resource domains");
    acquired.phase = GuiTextCursorPhase::running;
    acquired.temporary_name_storage = &buffers.strings;
    acquired.native_site = 0x00ab8935;
    try {
        // SAME pool/map as Shadow, with AB8910's own name/FH3/call sites.
        buffers.widgets.create_auxiliary_model_00ab8910_fragment(cursor, buffers.strings, acquired);
        acquired.native_site = 0x00ab89bd;
        actual_model(cursor, buffers).storage.node.auxiliary_flags_138 &= ~std::uint32_t{3};
        auto* parent = widget.node_binding();
        acquired.native_site = 0x00ab89ce;
        set_native_node_parent_00b6e680(buffers.parenting,
            actual_model(cursor, buffers).node.transform, parent ? &parent->transform : nullptr);

        CameraMatrix matrix{};
        const float one = services.one_00d7a24c; // One MOVSS reused four times.
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = one;
        matrix[14] = current_sentinel(buffers); // Separate live read from later x87.
        auto& positioned = actual_model(cursor, buffers);
        require(positioned.storage.node.vtable_00 == 0x00d62de8 &&
            positioned.environment.vtable_00d62de8 &&
            positioned.environment.vtable_00d62de8[0x38 / 4] == 0x00b6db10,
            "Text cursor requires the actual Model current38 B6DB10 profile");
        acquired.native_site = 0x00ab8a56;
        set_transform_local_matrix_00b6db10(positioned.node.transform, matrix);
        acquired.native_site = 0x00ab8a65;
        buffers.widgets.set_node_visibility_factor_00b6da70(
            actual_model(cursor, buffers).node, 0.0f, false);
        acquired.native_site = 0x00ab8a6f;
        // Raw-slot state3/CB7FB4 remains inside this composed factory. A
        // returned mesh is already constructed and receives no raw rollback.
        acquired.mesh = buffers.geometry.create_mesh();
        const float sentinel = current_x87_sentinel(buffers);
        acquired.native_site = 0x00ab8aaa;
        set_native_model_geometry_00b75170(actual_model(cursor, buffers), 0,
            acquired.mesh, sentinel, sentinel);

        acquired.native_site = 0x00ab8ab8;
        acquired.name_constructing = true;
        acquired.temporary_name.assign_0041e870(buffers.strings, "simplecolor.mvfm");
        acquired.name_constructing = false;
        acquired.name_live = true;
        acquired.name_cleanup_armed = true;
        acquired.native_unwind_state = 4;
        require(buffers.native_renderer != nullptr, "Text cursor requires concrete native renderer services");
        auto& native = *buffers.native_renderer;
        require_gui_text_native_renderer_domain(native, buffers.current_renderer_00f8d394,
            buffers.strings, buffers.geometry.actual_owners());
        void* renderer = buffers.current_renderer_00f8d394;
        acquired.native_site = 0x00ab8ad5;
        acquired.declaration = load_gui_text_native_declaration_current38(renderer,
            acquired.temporary_name, native, acquired.declaration_factory);
        acquired.native_unwind_state = -1;
        acquired.name_cleanup_armed = false;
        acquired.name_live = false;
        acquired.native_site = 0x00ab8afb;
        destroy_native_string_header_0041dd20(&acquired.temporary_name, buffers.strings);
        renderer = buffers.current_renderer_00f8d394; // Reload after name-release callbacks.
        acquired.vertex_factory.native_site = 0x00ab8b10;
        acquired.native_site = 0x00ab8b10;
        acquired.vertex = create_gui_text_native_vertex_current5c(renderer, 4, 1,
            acquired.declaration, native, acquired.vertex_factory);
        require(acquired.declaration && acquired.vertex,
            "Text cursor renderer factories must return actual owned resources");
        auto& owners = buffers.geometry.actual_owners();
        acquired.native_site = 0x00ab8b19;
        set_native_mesh_vertex_stream_00b73bb0(*acquired.mesh, owners, 0, acquired.vertex);
        // AB8910 order differs from AB8400. There is no index-stream factory.
        acquired.declaration_factory = {};
        acquired.native_site = 0x00ab8b22;
        release_creator(acquired.declaration, owners);
        acquired.vertex_factory.creator = nullptr;
        acquired.vertex_factory.companion = nullptr;
        acquired.vertex_factory.phase = NativeStreamClonePhase::consumed;
        acquired.native_site = 0x00ab8b38;
        release_creator(acquired.vertex, owners);
        acquired.native_site = 0x00ab8b4a;
        acquired.section = buffers.geometry.create_section();
        acquired.section->primitive_08 = 5;
        acquired.section->range_words_0c[0] = 0;
        acquired.section->range_words_0c[2] = 0;
        acquired.section->range_words_0c[1] = 4;
        acquired.section->range_words_0c[3] = 2;
        acquired.native_site = 0x00ab8b78;
        acquired.name_constructing = true;
        acquired.temporary_name.assign_0041e870(buffers.strings, "GuiCursor.mshd");
        acquired.name_constructing = false;
        acquired.name_live = true;
        acquired.name_cleanup_armed = true;
        acquired.native_unwind_state = 5;
        acquired.native_site = 0x00ab8b85;
        acquired.material = buffers.geometry.create_material_for_effect_00535320(acquired.temporary_name,
            buffers.current_renderer_00f8d394, buffers.materials, buffers.material_vtable_00d5e520);
        acquired.native_unwind_state = -1;
        acquired.name_cleanup_armed = false;
        acquired.name_live = false;
        acquired.native_site = 0x00ab8bae;
        destroy_native_string_header_0041dd20(&acquired.temporary_name, buffers.strings);
        acquired.native_site = 0x00ab8bb8;
        services.parameter_owner.bind_retained_text_00b18a40(*acquired.material, widget, owners);
        acquired.native_site = 0x00ab8bc0;
        set_native_mesh_section_material_00b864c0(*acquired.section, owners, acquired.material);
        acquired.native_site = 0x00ab8bcf;
        release_creator(acquired.material, owners);
        acquired.native_site = 0x00ab8be0;
        rebuild_native_mesh_section_vertex_layout_00b865a0(*acquired.section, owners,
            acquired.mesh, services.layouts);
        acquired.native_site = 0x00ab8be8;
        append_native_mesh_draw_section_00b73c60(*acquired.mesh, acquired.section);
        acquired.native_site = 0x00ab8bf1;
        release_creator(acquired.section, owners);
        acquired.native_site = 0x00ab8c03;
        release_creator(acquired.mesh, owners);
        acquired.native_site = 0x00ab8c24;
        acquired.phase = GuiTextCursorPhase::complete;
    } catch (...) {
        if (!acquired.failure_site) acquired.failure_site = acquired.native_site;
        // AB8910's own FH3 map (DEEDE0), not the Shadow caller's table.
        // The prefix handles states0/1; retain state1 if only host registration
        // failed after construction, because no native failure site exists.
        if ((acquired.native_unwind_state == 4 || acquired.native_unwind_state == 5) &&
            acquired.name_cleanup_armed) {
            acquired.cleanup_site = acquired.native_unwind_state == 4 ? 0x00cb7fbc : 0x00cb7fc4;
            acquired.name_cleanup_armed = false;
            acquired.name_live = false;
            acquired.native_unwind_state = -1;
            destroy_native_string_header_0041dd20(&acquired.temporary_name, buffers.strings);
        }
        acquired.phase = GuiTextCursorPhase::failed;
        throw; // Preserve publications and completed creator references; no replay.
    }
}

GuiTextCopyContinuation::GuiTextCopyContinuation(GuiTextLifetime& lifetime,
    GuiTextCopyServices& services) : lifetime_(lifetime), services_(services) {
    require(lifetime.after_base_copy_ && !lifetime.copy_continuation_claimed_ &&
        lifetime.phase_ == GuiTextLifetime::Phase::constructing &&
        lifetime.scalar_phase_ == GuiTextScalarDeletionPhase::not_started &&
        lifetime.widget_.text_lifetime() == &lifetime &&
        &services.cursor.buffers == &lifetime.buffers_ &&
        &services.submit.content.content.buffers == &lifetime.buffers_ &&
        &services.submit.content.content.calls.glyph_child_calls() == &lifetime.child_calls_ &&
        &services.cursor.layouts == &services.submit.content.nonempty.layouts,
        "Text copy continuation requires one after-base admission and its same content/resource services");
    lifetime.copy_continuation_claimed_ = true;
}

GuiTextCopyPhase GuiTextCopyContinuation::run_derived_00abb2c0() {
    require(phase_ == GuiTextCopyPhase::admitted, "Text copy native calls must run exactly once");
    try {
        phase_ = GuiTextCopyPhase::cursor;
        ensure_gui_text_cursor_00ab8910(lifetime_, services_.cursor, cursor_);
        phase_ = GuiTextCopyPhase::sections;
        auto binding = lifetime_.content_binding();
        ensure_gui_text_draw_sections_00ab8530(binding.widget, binding.text,
            binding.shadow_188, services_.cursor.buffers, lifetime_.section_operation());
        phase_ = GuiTextCopyPhase::rebuilding;
        auto result = rebuild_gui_text_content_00abb1d0(lifetime_, services_.submit);
        pending_ = std::move(result.pending);
        phase_ = result.status == GuiTextSubmitStatus::pending_content
            ? GuiTextCopyPhase::pending_content : GuiTextCopyPhase::complete;
        lifetime_.copy_continuation_complete_ = phase_ == GuiTextCopyPhase::complete;
        if (lifetime_.copy_continuation_complete_) lifetime_.phase_ = GuiTextLifetime::Phase::live;
        return phase_;
    } catch (...) { phase_ = GuiTextCopyPhase::failed; throw; }
}

GuiTextCopyPhase GuiTextCopyContinuation::resume_after_child() {
    require(phase_ == GuiTextCopyPhase::pending_content && pending_,
        "Text copy resume requires its retained pending content frame");
    // The inner operation checks the exact pending reason before consuming it.
    // No AB8910, AB8530 or ABB1D0 prefix is repeated here.
    try {
        const auto result = resume_gui_text_submit_after_child(pending_);
        if (result == GuiTextSubmitStatus::complete) {
            phase_ = GuiTextCopyPhase::complete;
            lifetime_.copy_continuation_complete_ = true;
            lifetime_.phase_ = GuiTextLifetime::Phase::live;
        }
        return phase_;
    } catch (...) {
        // This entry is permitted only AFTER the actual child completed.
        // A throwing native suffix may already have effects; retain its frame
        // for inspection, never advertise the moved/partially consumed builder
        // as a reusable pending child. No automatic retry or rollback.
        phase_ = GuiTextCopyPhase::failed;
        throw;
    }
}
} // namespace bsp
