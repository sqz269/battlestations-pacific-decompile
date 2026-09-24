#include "bsp/native_gui_widget_model_clone.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
}

NativeModelReference& clone_native_gui_widget_model_00b752b0(
    NativeModelOwner& source, std::uint32_t flags,
    NativeGuiWidgetModelCloneContext& context, NativeGuiWidgetModelCloneAcquired& acquired) {
    auto& access = context.access;
    auto& owners = access.geometry.actual_owners();
    const auto& mesh = acquired.creators.mesh;
    require(!acquired.started && !acquired.complete && !acquired.raw_destination &&
        !acquired.model_owner && !acquired.creators.model && !mesh.mesh &&
        !mesh.section && !mesh.material && !mesh.stream.creator &&
        !mesh.stream.companion && mesh.stream.phase == NativeStreamClonePhase::empty,
        "raw Model clone requires a fresh acquisition frame");
    require(flags == 0x26u || flags == 0x3eu,
        "raw Model clone supports only flags26/3E,parent0");
    require(source.phase == NativeModelOwner::Phase::live &&
        &source.environment == &access.models &&
        &access.models.retained_owners == &owners &&
        &access.materials.retained_owners == &owners &&
        !access.models.actual_names && access.prepare_model &&
        access.retire_failed_model && access.bind_completed_model,
        "raw Model clone requires shared actual owners and raw constructor lifecycle");
    auto* source_reference = dynamic_cast<NativeModelReference*>(
        &owners.resolve_actual(&source.storage.node));
    require(source_reference && &source_reference->model_owner() == &source,
        "raw Model clone requires the same canonical source companion");
    require(source.storage.node.vtable_00 == 0x00d62de8u &&
        access.models.vtable_00d62de8 &&
        access.models.vtable_00d62de8[4] == 0x00b752b0u,
        "raw Model clone has no binding for current source virtual10");
    access.models.nodes.require_raw_name_pool();

    acquired.started = true;
    acquired.active_call_site = 0x00b752d1;
    void* const raw = access.models.pool_01090054.allocate_raw_slot_00b74d00();
    if (!raw) throw std::bad_alloc(); // Native null result later faults in base copy.
    acquired.raw_destination = raw;
    acquired.native_eh_state = 0;
    try {
        acquired.model_owner = &access.prepare_model(access.companion_context, raw, access.models);
        auto& destination = *acquired.model_owner;
        require(&destination.storage.node == raw && &destination.environment == &access.models &&
            destination.phase == NativeModelOwner::Phase::prepared,
            "raw Model preparation must bind the same unused actual pool slot");
        acquired.active_call_site = 0x00b752ea;
        const void* const name = &native_node_name_00b6d800(source.storage.node);
        acquired.active_call_site = 0x00b752f2;
        construct_native_model_00b75030(destination, name, context.node_constants);
    } catch (...) {
        if (auto* destination = acquired.model_owner) {
            acquired.model_owner = nullptr;
            access.retire_failed_model(access.companion_context, *destination);
        }
        acquired.raw_destination = nullptr;
        acquired.native_eh_state = -1;
        // CC1C70 -> B748C0 selects this same canonical01090054 pool.
        access.models.pool_01090054.return_raw_slot_00b74750(raw);
        throw;
    }
    // Native B7530A disarms state0 before B6F150. Host registration may fail
    // after construction, so it must not be covered by the slot-return scope.
    acquired.native_eh_state = -1;
    acquired.creators.model = &access.bind_completed_model(
        access.companion_context, *acquired.model_owner);
    require(&acquired.creators.model->model_owner() == acquired.model_owner &&
        &owners.resolve_actual(raw) == static_cast<RenderCommandReference*>(acquired.creators.model),
        "raw Model registration must borrow the same actual destination+04");
    auto& destination = *acquired.model_owner;
    acquired.active_call_site = 0x00b75312;
    copy_native_gui_text_model_base_00b6f150_fragment(source, destination);

    // Read CURRENT source180 only after every base-copy callback.
    void* const current_geometry = source.storage.model.geometry_180;
    if (current_geometry) {
        acquired.active_call_site = 0x00b7532f;
        auto* current = dynamic_cast<NativeMeshReference*>(&owners.resolve_actual(current_geometry));
        require(current && &current->storage() == current_geometry,
            "raw Model current geometry requires its canonical actual mesh companion");
        NativeStreamCloneServices* streams = nullptr;
        if (flags == 0x3eu) {
            streams = context.streams_3e;
            require(streams && &streams->geometry == &access.geometry &&
                &streams->vertices == &access.streams,
                "raw Model flags3E requires the same actual stream clone services");
        }
        access.geometry.clone_mesh_for_text_00b742a0(current->storage(),
            context.mesh_vtable_00d62d60, access.materials,
            access.material_vtable_00d5e520, acquired.creators.mesh, streams);
        acquired.active_call_site = 0x00b7534e;
        associate_native_gui_text_model_clone_geometry_00b752b0_fragment(
            source, destination, acquired.creators.mesh.mesh);
    }
    acquired.active_call_site = 0x00b75365;
    finish_native_gui_text_model_clone_00b752b0_fragment(source, destination);
    acquired.complete = true;
    return *acquired.creators.model;
}
} // namespace bsp
