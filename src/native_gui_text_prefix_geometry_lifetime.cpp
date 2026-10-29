#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <atomic>
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Prefix = NativeGuiTextPrimaryPrefixLifetimeContext;
using Context = NativeGuiTextPrefixGeometryLifetimeContext;
using Phase = NativeGuiTextMainGeometryStoragePhase;
using AssociationPhase = NativeModelStorageGeometryPhase;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void require(bool b, const char* message) { if (!b) throw std::logic_error(message); }
bool untouched(const NativeGuiTextPrefixGeometryProgress& p) noexcept {
    return !p.entered && !p.returned && !p.failed;
}
bool fresh(const NativeModelStorageGeometryAcquired& a) noexcept {
    const auto& s = a.scalars;
    return a.phase == AssociationPhase::fresh && !a.active_call_site && !a.actual_model && !a.context && !a.frame &&
        !a.captured_requested && !a.captured_previous && !a.requested_captured && !a.previous_captured &&
        !a.same_geometry && !a.geometry_published && !a.increment_completed && !a.decrement_completed &&
        !a.terminal_completed && !a.captured_increment && !a.captured_decrement &&
        !s.first_argument_bits && !s.sentinel_bits && !s.second_argument_bits && !s.first_read &&
        !s.sentinel_read && !s.first_stored && !s.second_read && !s.second_stored;
}
bool fresh(const NativeGuiTextMainGeometryStorageAcquired& a) noexcept {
    using Boundary = NativeGuiTextMainGeometryStorageAcquired::Boundary;
    const auto& m = a.mesh;
    const auto& t = a.transform;
    return a.phase == Phase::fresh && a.boundary == Boundary::none && a.escaped_boundary == Boundary::none &&
        !a.active_call_site && !a.escaped_call_site && !a.destination && !a.captured_primary && !a.captured_mesh &&
        !a.primary_captured && !a.mesh_entered && !a.mesh_returned && !a.arguments_prepared &&
        !m.raw_slot && !m.creator && !m.companion && !m.owner_record && !m.native_site && !m.started &&
        !m.constructor_complete && !m.published && !m.registered && fresh(a.association) &&
        !a.association_entered && !a.association_returned && !a.creator_decrement && !a.creator_result &&
        !a.creator_release_entered && !a.creator_decremented && !a.creator_release_returned &&
        !a.creator_zero_profile && !a.creator_zero_target && !a.creator_profile_captured && !a.creator_target_captured &&
        !a.creator_zero_entered && !a.creator_zero_returned && !a.position_prepared && !a.position_entered &&
        !a.position_returned && !t.receiver && !t.matrix && !t.captured_target && !t.admitted_profile &&
        !t.entered && !t.body_entered && !t.body_returned && !t.failed && !a.failure;
}
bool aliases(const Prefix& p, const Context& c) noexcept {
    const auto& r = p.text.factory_context.publication.construction_context.sections;
    return c.prefix == &p && p.operation.geometry() == &c &&
        &c.construction.primary == &p.widget.primary.record &&
        &c.construction.primary.actual == &p.producer_context.primary.actual &&
        &c.construction.primary.reference == &p.producer_context.primary.reference &&
        &c.construction.primary.construction == &p.producer_context.primary.construction &&
        &c.construction.primary.lifetime_frame == &p.producer_context.primary.lifetime_frame &&
        &c.construction.primary.lifetime == &p.producer_context.primary.lifetime &&
        &c.construction.primary.release_frame == &p.producer_context.primary.release_frame &&
        &c.construction.primary.release == &p.producer_context.primary.release &&
        &c.construction.internal188 == &p.widget.internal188.record &&
        &c.construction.internal188.model == &r.model &&
        &c.construction.association.storage == &p.widget.primary.record.model.storage &&
        &c.construction.geometry == &r.geometry &&
        c.frame.matches_backing(c.construction, c.arguments, c.position, c.transform_scratch, c.bounds_scratch);
}
const Context* graph(const Prefix& p) {
    using Kind = NativeGuiTextPrimaryPrefixOperation::Kind;
    const auto* c = p.operation.geometry();
    if (p.operation.kind() == Kind::none) {
        require(!c, "no-operation prefix cannot carry a geometry shortcut");
        return nullptr;
    }
    require(p.operation.kind() == Kind::geometry_only && c && aliases(p, *c),
        "prefix operation needs the EXACT immutable379 context/backing graph");
    auto& publication = p.text.factory_context.publication;
    auto& construction = c->construction;
    require(&construction.geometry.actual_owners() == &publication.owners &&
        &construction.primary.model.storage.node.scenes.owners == &publication.owners &&
        &construction.internal188.model.storage.node.scenes.owners == &publication.owners &&
        bits(&construction.association.unchanged_00d7a260) ==
            bits(publication.data.data_at(0x00d7a260, sizeof(Word))) &&
        bits(&construction.one_00d7a24c) == bits(publication.data.data_at(0x00d7a24c, sizeof(float))) &&
        bits(&construction.negative_zero_00d7a208) == bits(publication.data.data_at(0x00d7a208, sizeof(float))) &&
        bits(&construction.y_scale_00e12fc4) == bits(publication.data.data_at(0x00e12fc4, sizeof(float))) &&
        bits(&construction.half_00d7a280) == bits(publication.data.data_at(0x00d7a280, sizeof(double))),
        "prefix379 must borrow its SAME canonical owners and actual live constant cells");
    return c;
}
bool mesh_metadata(const GuiNativeMeshAcquired& m, const void* identity) noexcept {
    // Producer history/address identities only, valid after terminal erasure.
    // Do not even poll the now-dangling companion/owner_record for liveness.
    // This existing B73B60 producer consumes/clears raw_slot after construction
    // and does not set the other producer's published marker (geometry.cpp259).
    return identity && !m.raw_slot && m.creator == identity &&
        m.started && m.constructor_complete && !m.published && m.registered;
}
bool associated(const NativeModelStorageGeometryAcquired& a, const void* model,
    const void* mesh, const NativeModelStorageGeometryContext& c) noexcept {
    return a.phase == AssociationPhase::complete && a.actual_model == model && a.context == &c && a.frame &&
        a.requested_captured && a.previous_captured && a.captured_requested == mesh && !a.captured_previous &&
        !a.same_geometry && a.geometry_published && a.increment_completed && a.captured_increment &&
        !a.decrement_completed && !a.terminal_completed;
}
bool complete(const Prefix& p, const Context& c) noexcept {
    const auto& a = c.acquired;
    const auto* receipt = p.producer.publication();
    return aliases(p, c) && c.progress.entered && c.progress.returned && !c.progress.failed &&
        c.frame.matches(c.construction, a, Phase::complete) && a.phase == Phase::complete && !a.failure &&
        receipt && p.producer_frame.matches(p.producer_context, p.producer,
            NativeGuiTextPrimaryModelStoragePhase::complete) &&
        receipt->text_identity() == a.destination && receipt->model_identity() == a.captured_primary &&
        receipt->factory_context() == &p.text.factory_context && receipt->factory_acquisition() == &p.text.factory &&
        receipt->primary_record() == &p.producer_context.primary &&
        receipt->primary_dispatch_record() == &c.construction.primary &&
        receipt->construction() == &p.producer_context.primary.construction &&
        receipt->pool() == p.producer.actual_pool && receipt->pool() == &c.construction.primary.model.storage.pool_01090054 &&
        p.name.construct_entered && p.name.construct_returned && !p.name.construct_failed &&
        p.name.return_entered && p.name.returned && !p.name.return_failed &&
        a.destination == p.text.factory.raw_slot && a.captured_primary == c.construction.primary.actual &&
        a.primary_captured && a.mesh_entered && a.mesh_returned && mesh_metadata(a.mesh, a.captured_mesh) &&
        a.arguments_prepared && a.association_entered && a.association_returned &&
        associated(a.association, a.captured_primary, a.captured_mesh, c.construction.association) &&
        a.creator_release_entered && a.creator_decremented && a.creator_release_returned &&
        a.creator_decrement && a.creator_result > 0 && !a.creator_zero_entered && !a.creator_zero_returned &&
        !a.creator_profile_captured && !a.creator_target_captured &&
        a.position_prepared && a.position_entered && a.position_returned &&
        a.transform.entered && a.transform.body_entered && a.transform.body_returned && !a.transform.failed &&
        a.transform.receiver == a.captured_primary && a.transform.matrix &&
        a.transform.captured_target == 0x00b6db10 && a.transform.admitted_profile == 0x00d62de8;
}
bool shadow_metadata(const Prefix& p) noexcept {
    const auto& s = p.text.factory_context.publication.construction.sections;
    const auto& r = p.text.factory_context.publication.construction_context.sections;
    const auto& release = s.mesh_release;
    return s.started && s.complete && !s.exception_cleanup_started &&
        s.failure == NativeGuiTextRawModelSectionStorageAcquired::Failure::none &&
        s.model_constructed && s.shadow_published && s.model_registered &&
        mesh_metadata(s.mesh, release.identity) && release.entered && release.decremented && release.complete &&
        release.captured_decrement && release.result > 0 && !release.zero_entered && !release.zero_returned &&
        associated(s.model.association, s.model.actual, release.identity, r.association);
}
void live_mesh(const Prefix& p, void* identity, const GuiNativeMeshAcquired& acquired) {
    require(mesh_metadata(acquired, identity), "prefix live Mesh needs its actual completed producer record");
    auto& owners = p.text.factory_context.publication.owners;
    auto* const reference = owners.find(identity); // BEFORE any companion/native payload access.
    require(reference && reference == acquired.companion &&
        static_cast<void*>(&reference->reference_count) == reinterpret_cast<void*>(bits(identity) + 4u) &&
        reference->reference_count.load(std::memory_order_relaxed) > 0,
        "prefix Mesh must still have its SAME live canonical actual+4 companion");
    auto& data = p.text.factory_context.publication.data;
    const auto* table = static_cast<const volatile Word*>(data.data_at(0x00d62d60, 2 * sizeof(Word)));
    require(read(identity) == 0x00d62d60 && table[0] == 0x00bd30e0 && table[1] == 0x00b74280,
        "prefix Mesh requires genuine CURRENT canonical flags1 terminal family");
}
bool model_mesh_returned(const NativeModelStorageDispatchContext& m, const void* mesh) noexcept {
    const auto& a = m.lifetime;
    return mesh && m.reference && m.reference->retired() && m.release.started && m.release.complete &&
        a.started && a.complete && !a.exception_cleanup_started && a.scalar_started && a.scalar_complete &&
        a.flags_read && a.pool_returned && (a.flags_low_byte & 1u) && !a.captured_retained &&
        a.captured_geometry == mesh && a.geometry_decrement_completed && a.geometry_terminal_completed &&
        a.geometry_cleared && !a.geometry_unwind.started && a.node.complete && a.node.base_cleanup_completed;
}
} // namespace

void require_native_gui_text_prefix_geometry_reservation(const Prefix& p) {
    const auto* const c = graph(p);
    if (!c) return;
    if (untouched(c->progress)) {
        require(c->frame.phase() == Phase::fresh && fresh(c->acquired),
            "unentered installed379 graph requires wholly fresh retained records");
        return; // No Text, Model, Mesh, operand/preimage or control-byte read.
    }
    require(complete(p, *c), "entered/pending/failed379 cannot fall back to prefix-only lifetime");
}

void require_native_gui_text_prefix_geometry_primary(void* geometry, const Prefix& p) {
    const auto* const c = graph(p);
    require(c && complete(p, *c) && shadow_metadata(p),
        "geometry scalar permission requires BOTH real completed379/db55 and raw85 Mesh producers");
    const auto& model = c->construction.primary;
    require(model.reference && !model.reference->retired() && !model.lifetime.started &&
        !model.lifetime.pool_returned && geometry == c->acquired.captured_mesh,
        "prefix Model180 cannot be replaced, retired, pending or empty");
    live_mesh(p, geometry, c->acquired.mesh);
    const auto& shadow = c->construction.internal188;
    if (!shadow.lifetime.started) {
        require(shadow.actual && shadow.reference && !shadow.reference->retired() &&
            shadow.construction.complete && shadow.construction.metadata_bound && !shadow.lifetime.pool_returned &&
            p.text.factory_context.publication.owners.find(shadow.actual) == &*shadow.reference,
            "prefix internal Mesh needs its SAME still-live canonical Model metadata");
        require(read(shadow.actual) == 0x00d62de8 &&
            *reinterpret_cast<const volatile Word*>(bits(shadow.actual) + 0x180u) ==
                bits(p.text.factory_context.publication.construction.sections.mesh_release.identity),
            "prefix internal180 must retain the genuine raw85 Shadow Mesh publication");
        live_mesh(p, p.text.factory_context.publication.construction.sections.mesh_release.identity,
            p.text.factory_context.publication.construction.sections.mesh);
    } else require(model_mesh_returned(shadow,
        p.text.factory_context.publication.construction.sections.mesh_release.identity),
        "post-shadow permission needs durable actual Shadow Mesh return evidence");
    // An observed zero is only a boundary guard, NEVER proof of the late AA70E0
    // epoch inside379. Source-audited closed callbacks/imports must establish it.
    require(*(static_cast<const volatile unsigned char*>(p.text.factory.raw_slot) + 0x74) == 0,
        "geometry lifetime excludes nonzero Text bounds control; late-epoch proof remains external");
}

bool native_gui_text_prefix_geometry_meshes_settled(const Prefix& p) noexcept {
    using Kind = NativeGuiTextPrimaryPrefixOperation::Kind;
    if (p.operation.kind() == Kind::none) return !p.operation.geometry();
    const auto* const c = p.operation.geometry();
    if (p.operation.kind() != Kind::geometry_only || !c || !complete(p, *c) || !shadow_metadata(p)) return false;
    // Only retained C++ metadata and saved address identities below: MeshEntry,
    // Mesh companion, native header/vptr/count and returned Model slots are gone.
    return model_mesh_returned(c->construction.primary, c->acquired.captured_mesh) &&
        model_mesh_returned(c->construction.internal188,
            p.text.factory_context.publication.construction.sections.mesh_release.identity);
}
} // namespace bsp
