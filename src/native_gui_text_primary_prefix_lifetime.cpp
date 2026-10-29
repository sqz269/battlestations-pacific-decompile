#include "bsp/native_gui_text_primary_prefix_lifetime.hpp"
#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextPrimaryPrefixLifetimeContext;
using ProducerPhase = NativeGuiTextPrimaryModelStoragePhase;
using CallPhase = NativeModelStorageDispatchCallAcquired::Phase;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
void require(bool b, const char* text) { if (!b) throw std::logic_error(text); }
bool untouched_name(const NativeGuiTextPrimaryPrefixNameProgress& n) noexcept {
    return !n.construct_entered && !n.construct_returned && !n.construct_failed &&
        !n.return_entered && !n.returned && !n.return_failed;
}
void calls(const NativeGuiWidgetRawModelRoute& q,
    const NativeModelStorageNodeDispatchContext& node, NativeGuiTextPrimaryPrefixCalls span) {
    const auto bytes = static_cast<std::uint64_t>(span.count) * sizeof(NativeModelStorageNodeCall);
    require(span.data && span.count && bytes <= 0xffffffffu &&
        !(bits(span.data) % alignof(NativeModelStorageNodeCall)) &&
        static_cast<std::uint64_t>(bits(span.data)) + bytes <= 0x100000000ull &&
        q.node.matches(node, q.node_acquired, span.data, span.count) &&
        q.tree.matches(q.record, q.tree_acquired) &&
        q.node_acquired.bound && !q.node_acquired.poisoned && !q.node_acquired.entry_failed &&
        !q.node_acquired.capacity_failed && q.node_acquired.used <= span.count &&
        q.tree_acquired.bound && !q.tree_acquired.poisoned &&
        q.tree_acquired.current18.phase != CallPhase::entered && q.tree_acquired.current18.phase != CallPhase::failed &&
        q.tree_acquired.current0.phase != CallPhase::entered && q.tree_acquired.current0.phase != CallPhase::failed,
        "prefix lifetime needs real retained quiescent Model call backing");
    for (std::size_t i = 0; i != span.count; ++i)
        require(span.data[i].phase != CallPhase::entered && span.data[i].phase != CallPhase::failed,
            "prefix lifetime cannot abandon an entered/failed Model provider invocation");
    // Exact private context/acquired/array affinity is checked above. Full
    // opaque/transitive provider extents remain the caller's obligation.
}
void aliases(const Context& g) {
    auto& f = g.text.factory_context;
    auto& t = f.publication;
    auto& p = g.producer_context;
    auto& primary = g.widget.primary;
    auto& internal = g.widget.internal188;
    const auto& nested = f.construction.sections.model;
    require(t.retirement.primary_prefix == &g && &g.widget.text == &g.text &&
        &g.widget.authentication == &g.authentication && &g.widget.widget == &g.authentication.context() &&
        &t.retirement.base == &g.widget.widget && &t.construction_context.lifetime == &g.widget.widget &&
        &t.construction == &f.construction && &t.retirement.model == &nested &&
        &t.retirement.resources == &t.construction_context.sections &&
        &p.factory_context == &f && &p.factory == &g.text.factory && &p.publication == &g.text.publication &&
        &p.primary.actual == &primary.record.actual &&
        &p.primary.reference == &primary.record.reference && &p.primary.construction == &primary.record.construction &&
        &p.primary.lifetime_frame == &primary.record.lifetime_frame && &p.primary.lifetime == &primary.record.lifetime &&
        &p.primary.release_frame == &primary.record.release_frame && &p.primary.release == &primary.record.release &&
        &p.primary_nodes == &primary.node &&
        &p.primary_node_context.record == &primary.record && &p.primary_node_context.tree_dispatch == &primary.tree &&
        &g.internal_node_context.record == &internal.record &&
        &g.internal_node_context.tree_dispatch == &internal.tree && &p.internal188 == &internal.record &&
        &internal.record.actual == &nested.actual && &internal.record.reference == &nested.reference &&
        &internal.record.construction == &nested.construction &&
        &internal.record.lifetime_frame == &nested.lifetime_frame && &internal.record.lifetime == &nested.lifetime &&
        &internal.record.release_frame == &nested.release_frame && &internal.record.release == &nested.release &&
        &primary.record != &internal.record && &p.primary != &nested &&
        g.authentication_acquired.bound && !g.authentication_acquired.active && !g.authentication_acquired.poisoned &&
        g.widget_acquired.bound && !g.widget_acquired.active && !g.widget_acquired.poisoned,
        "prefix lifetime requires the exact immutable factory/Text/primary/internal/widget provider graph");
    require(g.name.header8 && !(bits(g.name.header8) & 3u) &&
        static_cast<std::uint64_t>(bits(g.name.header8)) + 8u <= 0x100000000ull &&
        p.actual_name_header8 == g.name.header8 &&
        same_native_string_raw_pool_cells(g.name.strings, primary.record.model.storage.node.strings) &&
        same_native_string_raw_pool_cells(g.name.strings, internal.record.model.storage.node.strings) &&
        same_native_string_raw_pool_cells(g.name.strings, t.construction_context.sections.strings),
        "prefix name must bind its same external8B storage and real retained VFS/Model string cells");
    calls(primary, p.primary_node_context, g.primary_calls);
    calls(internal, g.internal_node_context, g.internal_calls);
}
void untouched_primary(const Context& g) {
    const auto& a = g.producer;
    const auto& p = g.producer_context.primary;
    require(g.producer_frame.matches(g.producer_context, a, ProducerPhase::prepared) &&
        a.phase == ProducerPhase::prepared && a.tree_bind_entered && a.tree_bound && a.node_bind_entered && a.node_bound &&
        a.actual_pool == &g.widget.primary.record.model.storage.pool_01090054 &&
        !a.allocation_entered && !a.allocation_returned && !a.actual_text && !a.raw_slot &&
        !a.constructor_entered && !a.constructor_returned && !a.constructor_result &&
        !a.canonical_bind_entered && !a.canonical_bound && !a.canonical_reference &&
        !a.publication_entered && !a.published && !a.flags_cleared && !a.failure &&
        !p.actual && !p.reference && !p.construction.started && !p.construction.complete &&
        !p.construction.metadata_bound && !p.construction.node_constructor_returned && !p.construction.tail_constructed &&
        !p.construction.actual_model && !p.construction.context &&
        !p.lifetime.started && !p.lifetime.complete && !p.lifetime.scalar_started && !p.lifetime.scalar_complete &&
        !p.lifetime.flags_read && !p.lifetime.pool_returned && !p.lifetime.node.started &&
        !p.lifetime.geometry_unwind.started && !p.release.started && !p.release.complete &&
        p.association.phase == NativeModelStorageGeometryPhase::fresh &&
        g.widget.primary.tree_acquired.current18.phase == CallPhase::fresh &&
        g.widget.primary.tree_acquired.current0.phase == CallPhase::fresh &&
        !g.widget.primary.tree_acquired.current04_entered && !g.widget.primary.tree_acquired.current04_returned &&
        !g.widget.primary.node_acquired.used &&
        untouched_name(g.name),
        "no receipt falls back ONLY for wholly untouched prepared primary and unentered name");
}
void stage(void* actual, const Context& g, bool permission,
    const NativeGuiTextRetirementAcquired* progress) {
    const auto& p = g.text.publication;
    auto& f = g.text.factory_context;
    const auto& scalar = f.reference_frame.scalar;
    const auto& d = scalar.destruction;
    require(!p.bind_failed && p.bind_complete && p.reference && !p.scalar_failed && !p.scalar_returned &&
        !p.companion_disposal_entered && !p.disposed && !scalar.complete && !scalar.destructor_returned &&
        !scalar.flags_captured && !scalar.pool_returned && !d.complete && !d.base_returned &&
        !d.exception_cleanup_started &&
        actual == g.text.factory.raw_slot && actual == g.producer.actual_text,
        "prefix stage rejects failed/disposing/returned metadata before companion access");
    if (progress) require(progress == &d, "prefix progress must be EXACT scalar.destruction, never a copy");
    g.authentication.require_raw_model_transport_receiver(actual, g.text);
    // The exact-record wrapper above proves this placement companion is alive.
    require(p.reference->storage() == actual && &p.reference->storage_context() == &f.publication,
        "prefix stage has another canonical raw122 companion");
    if (permission) {
        require(!p.scalar_entered && p.reference->phase() == NativeGuiTextRawModelReference::Phase::bound &&
            !scalar.started && !scalar.destructor_entered && !d.started && !d.base_entered && d.native_eh_state == -1,
            "prefix scalar permission must precede owner/canonical scalar entry");
    } else {
        require(p.scalar_entered && p.reference->phase() == NativeGuiTextRawModelReference::Phase::retiring,
            "prefix raw118 requires actual canonical retiring entry, never manual bound destruction");
        if (!progress) {
            require(!d.started && !d.exception_cleanup_started && !d.destination && !d.base_entered &&
                d.native_eh_state == -1 && d.boundary == NativeGuiTextRetirementAcquired::Boundary::none &&
                !d.normal_actions_entered && !d.normal_actions_returned && !d.unwind_actions_consumed &&
                !d.unwind_actions_returned &&
                ((!scalar.started && !scalar.destructor_entered) || (scalar.started && scalar.destructor_entered)),
                "prefix raw118 admits both genuine pre-start passes, not a replayed destruction");
        } else require(d.started && d.destination == actual && scalar.started && scalar.destructor_entered,
            "prefix retirement boundary requires its SAME actual started destruction");
    }
}
void predicate(const Context& g) {
    auto& base = g.widget.widget;
    auto& data = g.text.factory_context.publication.data;
    auto& process = game::game_native_gui_text_process();
    const auto* text = static_cast<const volatile Word*>(data.data_at(0x00d5c6c8, 9 * sizeof(Word)));
    require(base.actual_base_table_00d5c130 == data.data_at(0x00d5c130, 9 * sizeof(Word)) &&
        &base.actual_base_lineage_00f8bc88 == &process.element_lineage_00f8bc88() &&
        &base.actual_text_descriptor_00f8be28 == &process.text_lineage_00f8be28()[0] &&
        base.actual_base_table_00d5c130[3] == 0x00a9e070 && base.actual_base_table_00d5c130[8] == 0x00aa8320 &&
        text[0] == 0x00bd30e0 && text[1] == 0x00ab8ee0 && text[3] == 0x00ab83d0 && text[8] == 0x00aa8320,
        "prefix scalar needs genuine CURRENT canonical base/Text lineage and selectors");
    const Word descriptor = native_gui_text_descriptor_00ab6a30(base.actual_text_descriptor_00f8be28);
    const bool result = native_gui_widget_is_kind_of_00a9e070(descriptor, base.actual_base_lineage_00f8bc88);
    g.widget.slots.query(0x00a9e070, descriptor, result); // genuine answer, independent diagnostics
    if (result) g.widget.slots.reject("prefix TRUE Text-descriptor branch remains unsupported before188 access");
}
void primary_shape(const Context& g) {
    auto& q = g.widget.primary;
    auto& m = q.record;
    auto& owners = g.text.factory_context.publication.owners;
    require(m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
        m.construction.metadata_bound && m.construction.context == &m.model.storage &&
        !m.lifetime.started && !m.lifetime.scalar_started && !m.lifetime.complete &&
        !m.lifetime.scalar_complete && !m.lifetime.pool_returned && !m.release.started && !m.release.complete &&
        owners.find(m.actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == ptr(bits(m.actual) + 4u),
        "prefix primary payload requires its SAME still-live unretired canonical481 record");
    require(read(m.actual) == 0x00d62de8 && !read(m.actual, 0x30) && !read(m.actual, 0x34) &&
        !read(m.actual, 0x38) && !read(m.actual, 0x3c) && !read(m.actual, 0x40) &&
        !*reinterpret_cast<const volatile unsigned char*>(bits(m.actual) + 0x44u) &&
        !read(m.actual, 0xa0) && !read(m.actual, 0xa4) && !read(m.actual, 0x130) &&
        !(read(m.actual, 0x138) & 3u) && !read(m.actual, 0x164) && !read(m.actual, 0x168) &&
        !read(m.actual, 0x16c) && !read(m.actual, 0x170) && !read(m.actual, 0x174) &&
        (g.operation.kind() != NativeGuiTextPrimaryPrefixOperation::Kind::none || !read(m.actual, 0x180)),
        "prefix primary excludes hierarchy/scene/retained/point-light/geometry/74 state");
    if (g.operation.kind() != NativeGuiTextPrimaryPrefixOperation::Kind::none)
        require_native_gui_text_prefix_geometry_primary(ptr(read(m.actual, 0x180)), g);
}
void shape(void* actual, const Context& g, const NativeGuiTextRetirementAcquired* progress) {
    // Must follow the genuine false descriptor predicate. No dead188 Model
    // prefix/count/geometry is read after the existing raw118 release returns.
    require(read(actual, 0x4c) == bits(g.producer_context.primary.actual) &&
        !read(actual, 0x70) && !read(actual, 0x6c) &&
        !read(actual, 0x88) && !read(actual, 0x8c) && !read(actual, 0x90) &&
        !read(actual, 0x108) && !read(actual, 0x180) && !read(actual, 0x184) &&
        !read(actual, 0x18c) && !read(actual, 0x190) && !read(actual, 0x198) &&
        !read(actual, 0x19c) && !read(actual, 0x1a0) && !read(actual, 0x1ec) &&
        !read(actual, 0xec) && !read(actual, 0xf0) && !read(actual, 0xf4) && !read(actual, 0xf8) &&
        !read(actual, 0x1a4) && !read(actual, 0x1a8) && !read(actual, 0x1c0) && !read(actual, 0x1c4) &&
        !read(actual, 0x1c8) && !read(actual, 0x1cc),
        "prefix Text adds ONLY its genuine primary4C to the empty DEFAULT payload");
    void* const head = ptr(read(actual, 0x68));
    require(head && !(bits(head) & 3u) && read(head) == bits(head) && read(head, 4) == bits(head),
        "prefix Text requires its actual finite empty AA9390 ring");
    if (!progress) {
        const auto& m = g.widget.internal188.record;
        require(m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
            m.construction.metadata_bound && !m.lifetime.started && !m.lifetime.scalar_started &&
            !m.release.started && g.text.factory_context.publication.owners.find(m.actual) == &*m.reference,
            "prefix entry requires the same still-live untouched internal188 metadata");
        require(read(actual, 0x188) == bits(m.actual), "prefix entry needs its SAME genuine internal188 publication");
    } else require(progress->shadow_slot_cleared && progress->captured_shadow == g.widget.internal188.record.actual &&
        progress->shadow.started && progress->shadow.complete && g.widget.internal188.record.release.complete,
        "prefix post-shadow boundary uses independent completed release/clear records only");
    primary_shape(g);
}
} // namespace

bool native_gui_text_primary_prefix_selected(const Context& g) {
    aliases(g);
    require_native_gui_text_prefix_geometry_reservation(g);
    const auto* const r = g.producer.publication();
    if (!r) { untouched_primary(g); return false; }
    const auto& a = g.producer;
    const auto& p = g.producer_context.primary;
    require(g.producer_frame.matches(g.producer_context, a, ProducerPhase::complete) &&
        a.phase == ProducerPhase::complete && !a.failure && a.tree_bound && a.node_bound &&
        a.allocation_entered && a.allocation_returned && a.constructor_entered && a.constructor_returned &&
        a.canonical_bind_entered && a.canonical_bound && a.publication_entered && a.published && a.flags_cleared &&
        a.raw_slot && a.raw_slot == a.constructor_result && a.raw_slot == p.actual &&
        p.reference && a.canonical_reference == &*p.reference &&
        a.actual_text == g.text.factory.raw_slot && r->text_identity() == a.actual_text &&
        r->model_identity() == a.raw_slot && r->factory_context() == &g.text.factory_context &&
        r->factory_acquisition() == &g.text.factory && r->primary_record() == &p &&
        r->primary_dispatch_record() == &g.widget.primary.record && r->construction() == &p.construction &&
        r->pool() == a.actual_pool && r->pool() == &g.widget.primary.record.model.storage.pool_01090054 &&
        g.name.construct_entered && g.name.construct_returned && !g.name.construct_failed &&
        g.name.return_entered && g.name.returned && !g.name.return_failed,
        "prefix selection requires exact private db55 provenance and completed real outer name return");
    return true; // Provenance only: do not inspect retired Model/Text payloads.
}
void require_native_gui_text_primary_prefix_before_scalar(const Context& g) {
    if (!native_gui_text_primary_prefix_selected(g)) { g.authentication.require_default_before_scalar(); return; }
    void* const actual = g.text.factory.raw_slot;
    stage(actual, g, true, nullptr);
    predicate(g); // TRUE must fail before the following first188 slot read.
    shape(actual, g, nullptr);
}
void require_native_gui_text_primary_prefix_retirement(void* actual, const Context& g,
    const NativeGuiTextRetirementAcquired* progress) {
    require(native_gui_text_primary_prefix_selected(g), "raw118 primary route requires completed provenance");
    stage(actual, g, false, progress);
    predicate(g);
    shape(actual, g, progress);
}
void destroy_native_gui_text_primary_prefix_base(void* actual, const Context& g,
    const NativeGuiTextRetirementAcquired& progress) {
    require_native_gui_text_primary_prefix_retirement(actual, g, &progress);
    require(progress.base_entered && !progress.base_returned && progress.native_eh_state == -1 &&
        (progress.normal_actions_returned & 0x7eu) == 0x7eu,
        "prefix base runs only at the real completed-member raw118 base boundary");
    destroy_native_gui_widget_base_00aa9730(actual, g.widget);
}
} // namespace bsp
