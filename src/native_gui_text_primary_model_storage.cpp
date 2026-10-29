#include "bsp/native_gui_text_primary_model_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/native_node_destruction.hpp"
#include <atomic>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextPrimaryModelStorageContext;
using Acquired = NativeGuiTextPrimaryModelStorageAcquired;
using Phase = NativeGuiTextPrimaryModelStoragePhase;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(const void* p, Word offset = 0) noexcept { return reinterpret_cast<void*>(bits(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void write(void* p, Word offset, Word value) noexcept { *static_cast<volatile Word*>(at(p, offset)) = value; }
void require(bool value, const char* message) { if (!value) throw std::logic_error(message); }
struct Region { const volatile void* data; std::size_t bytes; };
bool valid(Region r) noexcept {
    return r.data && static_cast<std::uint64_t>(bits(r.data)) + r.bytes <= 0x100000000ull;
}
bool disjoint(Region a, Region b) noexcept {
    const auto left = static_cast<std::uint64_t>(bits(a.data));
    const auto right = static_cast<std::uint64_t>(bits(b.data));
    return left + a.bytes <= right || right + b.bytes <= left;
}
void fresh_primary(const NativeGuiTextRawModelRecord& p) {
    require(!p.actual && !p.reference && !p.construction.started && !p.construction.complete &&
        !p.construction.metadata_bound && !p.construction.node_constructor_returned &&
        !p.construction.tail_constructed && !p.construction.actual_model && !p.construction.context &&
        !p.lifetime.started && !p.lifetime.complete && !p.lifetime.scalar_started &&
        !p.lifetime.scalar_complete && !p.lifetime.flags_read && !p.lifetime.pool_returned &&
        !p.lifetime.node.started && !p.lifetime.geometry_unwind.started &&
        !p.release.started && !p.release.complete &&
        p.association.phase == NativeModelStorageGeometryPhase::fresh,
        "primary Model producer requires its SAME unused complete record");
}
template<class Check>
void model_regions(NativeModelStorageDispatchContext& m, Check check) {
    auto& s = m.model.storage;
    auto& n = s.node;
    for (const Region r : {Region{&m, sizeof(m)}, {&m.model, sizeof(m.model)}, {&s, sizeof(s)},
        {&s.pool_01090054, sizeof(s.pool_01090054)}, {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())},
        {&s.constants, sizeof(s.constants)}, {&n, sizeof(n)}, {&n.scenes, sizeof(n.scenes)},
        {&n.scenes.gates, sizeof(n.scenes.gates)}, {&n.scenes.owners, sizeof(n.scenes.owners)},
        {&n.scenes.increment_00ce221c, sizeof(n.scenes.increment_00ce221c)},
        {&n.scenes.decrement_00ce2220, sizeof(n.scenes.decrement_00ce2220)},
        {&n.trees, sizeof(n.trees)}, {&n.trees.frames, sizeof(n.trees.frames)},
        {&n.trees.point_lights, sizeof(n.trees.point_lights)}, {&n.world, sizeof(n.world)},
        {&n.strings, sizeof(n.strings)}, {&m.actual, sizeof(m.actual)},
        {&m.reference, sizeof(m.reference)}, {&m.construction, sizeof(m.construction)},
        {&m.lifetime_frame, sizeof(m.lifetime_frame)}, {&m.lifetime, sizeof(m.lifetime)},
        {&m.release_frame, sizeof(m.release_frame)}, {&m.release, sizeof(m.release)},
        {m.model.actual_profile_00d62de8, 0x58}}) check(r);
}
template<class Check>
void borrowed(Context& c, Check check) {
    auto& f = c.factory_context;
    auto& t = f.publication;
    for (const Region r : {Region{&c, sizeof(c)}, {&c.model_pools, sizeof(c.model_pools)},
        {&f, sizeof(f)}, {&c.factory, sizeof(c.factory)}, {&c.publication, sizeof(c.publication)},
        {&f.construction, sizeof(f.construction)}, {&f.reference_frame, sizeof(f.reference_frame)},
        {&t, sizeof(t)}, {&t.owners, sizeof(t.owners)}, {&t.data, sizeof(t.data)},
        {&t.construction_context, sizeof(t.construction_context)},
        {&t.construction_frame, sizeof(t.construction_frame)}, {&t.retirement, sizeof(t.retirement)},
        {f.companion_storage, f.companion_storage_bytes}, {f.disposal.context, f.disposal.context_bytes},
        {&c.primary, sizeof(c.primary)}, {&c.primary_node_context, sizeof(c.primary_node_context)},
        {&c.primary_node_context.parenting, sizeof(c.primary_node_context.parenting)},
        {&c.primary_node_context.tree_dispatch, sizeof(c.primary_node_context.tree_dispatch)},
        {&c.primary_nodes, sizeof(c.primary_nodes)}, {&c.primary_node_context.shared_node, sizeof(c.primary_node_context.shared_node)},
        {&c.primary_node_context.base_nodes, sizeof(c.primary_node_context.base_nodes)}, {c.actual_name_header8, 8}})
        check(r);
    model_regions(c.primary_node_context.record, check);
    model_regions(c.internal188, check);
}
void aliases(Context& c) {
    auto& f = c.factory_context;
    auto& t = f.publication;
    auto& nested = f.construction.sections.model;
    auto& r = t.construction_context.sections;
    auto& p = c.primary;
    auto& a = c.primary_node_context.record;
    auto& b = c.internal188;
    require(&t.construction == &f.construction && &t.retirement.model == &nested &&
        &t.retirement.resources == &r && &t.construction_context.sections == &r &&
        &b.model == &r.model && &b.actual == &nested.actual && &b.reference == &nested.reference &&
        &b.construction == &nested.construction && &b.lifetime_frame == &nested.lifetime_frame &&
        &b.lifetime == &nested.lifetime && &b.release_frame == &nested.release_frame && &b.release == &nested.release &&
        &a.actual == &p.actual && &a.reference == &p.reference && &a.construction == &p.construction &&
        &a.lifetime_frame == &p.lifetime_frame && &a.lifetime == &p.lifetime &&
        &a.release_frame == &p.release_frame && &a.release == &p.release &&
        &a.model.storage != &b.model.storage && &p != &nested &&
        disjoint({&p, sizeof(p)}, {&nested, sizeof(nested)}),
        "primary producer needs exact distinct primary/internal188 record aliases");
    auto& pn = a.model.storage.node;
    auto& in = b.model.storage.node;
    require(&a.model.storage.pool_01090054 == &b.model.storage.pool_01090054 &&
        &a.model.storage.constants == &b.model.storage.constants &&
        &a.model.storage.negative_zero_00d7a208 == &b.model.storage.negative_zero_00d7a208 &&
        a.model.actual_profile_00d62de8 == b.model.actual_profile_00d62de8 &&
        bits(a.model.actual_profile_00d62de8) == 0x00d62de8 &&
        a.model.actual_profile_00d62de8 == t.data.data_at(0x00d62de8, 0x58) &&
        &pn.scenes.owners == &t.owners && &in.scenes.owners == &t.owners &&
        &pn.trees.owners == &t.owners && &in.trees.owners == &t.owners &&
        &pn.strings == &r.strings && &in.strings == &r.strings && &pn.world == &in.world &&
        &pn.trees.frames == &in.trees.frames && &pn.trees.point_lights == &t.retirement.base.nodes.point_lights &&
        &in.trees.point_lights == &pn.trees.point_lights &&
        &pn.scenes.increment_00ce221c == &in.scenes.increment_00ce221c &&
        &pn.scenes.decrement_00ce2220 == &in.scenes.decrement_00ce2220 &&
        &pn.trees.decrement_00ce2220 == &pn.scenes.decrement_00ce2220 &&
        &in.trees.decrement_00ce2220 == &pn.trees.decrement_00ce2220 &&
        &c.primary_node_context.base_nodes == &t.retirement.base.nodes,
        "primary producer requires SAME actual pool/constants/profile/owner/string/world/frame/import/point-light domain");
}
void* bound_text(Context& c) {
    auto& f = c.factory_context;
    const auto& a = c.factory;
    const auto& p = c.publication;
    auto& construction = f.construction;
    require(p.bind_entered && p.bind_complete && !p.bind_failed && p.reference &&
        !p.scalar_entered && !p.scalar_returned && !p.scalar_failed &&
        !p.companion_disposal_entered && !p.disposed &&
        a.started && a.complete && a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete &&
        !a.failure && a.allocation_entered && a.allocation_returned && a.constructor_entered && a.constructor_returned &&
        a.prepared_context == &f && a.construction_record == &construction &&
        a.actual_pool == &f.publication.retirement.actual_pool_00f8bdf0 && a.raw_slot && a.constructor_result == a.raw_slot &&
        static_cast<void*>(p.reference) == f.companion_storage && f.publication.owners.find(a.raw_slot) == p.reference &&
        !f.reference_frame.scalar.started && !f.reference_frame.scalar.complete &&
        !f.reference_frame.scalar.destructor_entered && !f.reference_frame.scalar.destruction.started &&
        construction.started && construction.complete && construction.base_complete && construction.derived_complete &&
        construction.sections_entered && construction.destination == a.raw_slot &&
        construction.sections.started && construction.sections.complete &&
        !construction.sections.exception_cleanup_started &&
        construction.failure == decltype(construction.failure)::none &&
        construction.sections.failure == decltype(construction.sections.failure)::none &&
        !construction.exception_cleanup_started && !construction.base_lifetime_ended && !construction.consumed_cleanup_actions,
        "primary producer requires SAME real ba2/raw93/raw122 success BEFORE any scalar/disposal entry");
    // Independent markers above precede any access to the placement companion.
    auto& reference = *p.reference;
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound &&
        reference.storage() == a.raw_slot && &reference.storage_context() == &f.publication &&
        f.publication.owners.find(a.raw_slot) == &reference &&
        static_cast<void*>(&reference.reference_count) == at(a.raw_slot, 4) &&
        reference.reference_count.load(std::memory_order_relaxed) > 0,
        "primary producer needs the SAME live canonical Text+4 reference, never a substitute identity");
    auto& m = c.internal188;
    require(m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
        m.construction.metadata_bound && m.construction.actual_model == m.actual &&
        m.construction.context == &m.model.storage && !m.lifetime.started && !m.lifetime.scalar_started &&
        !m.lifetime.complete && !m.lifetime.scalar_complete && !m.lifetime.flags_read && !m.lifetime.pool_returned &&
        !m.lifetime.node.started && !m.release.started && !m.release.complete &&
        f.publication.owners.find(m.actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == at(m.actual, 4) &&
        m.reference->reference_count.load(std::memory_order_relaxed) > 0,
        "primary producer requires its SAME genuine completed still-live internal188 record");
    void* const raw = a.raw_slot;
    require(read(raw) == 0x00d5c6c8 && read(raw, 0x188) == bits(m.actual) &&
        !read(raw, 0x4c) && !read(raw, 0x70) && !read(raw, 0x6c) &&
        !read(raw, 0x88) && !read(raw, 0x8c) && !read(raw, 0x90) &&
        !read(raw, 0x108) && !read(raw, 0x180) && !read(raw, 0x184) && !read(raw, 0x18c) && !read(raw, 0x190) &&
        !read(raw, 0x198) && !read(raw, 0x19c) && !read(raw, 0x1a0) && !read(raw, 0x1ec) &&
        !read(raw, 0xec) && !read(raw, 0xf0) && !read(raw, 0xf4) && !read(raw, 0xf8) &&
        !read(raw, 0x1a4) && !read(raw, 0x1a8) && !read(raw, 0x1c0) && !read(raw, 0x1c4) &&
        !read(raw, 0x1c8) && !read(raw, 0x1cc),
        "primary prefix admits fresh4C childless/unparented/no-timed DEFAULT Text only");
    void* const head = reinterpret_cast<void*>(read(raw, 0x68));
    require(head && !(bits(head) & 3u) && read(head) == bits(head) && read(head, 4) == bits(head),
        "primary prefix requires the real finite empty AA9390 ring");
    return raw;
}
} // namespace

NativeGuiTextPrimaryModelStorageFrame::NativeGuiTextPrimaryModelStorageFrame(Context& c, Acquired& a) noexcept
    : context_(c), acquired_(a) {}
bool NativeGuiTextPrimaryModelStorageFrame::matches(const Context& c, const Acquired& a,
    Phase expected) const noexcept {
    return &context_ == &c && &acquired_ == &a && phase_ == expected;
}
NativeGuiTextPrimaryModelStorageFrame::~NativeGuiTextPrimaryModelStorageFrame() noexcept {
    if (phase_ == Phase::preparing || phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
}
void NativeGuiTextPrimaryModelStorageFrame::fail() noexcept {
    acquired_.escaped_boundary = acquired_.boundary;
    acquired_.escaped_call_site = acquired_.active_call_site;
    acquired_.failure = std::current_exception();
    acquired_.phase = Phase::failed;
    phase_ = Phase::failed;
}
void NativeGuiTextPrimaryModelStorageFrame::require_graph() {
    aliases(context_);
    fresh_primary(context_.primary);
    const Region writes[] = {{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)}};
    require(valid(writes[0]) && valid(writes[1]) && disjoint(writes[0], writes[1]),
        "primary producer frame/acquisition must be separate representable storage");
    borrowed(context_, [&](Region r) {
        require(valid(r) && disjoint(writes[0], r) && disjoint(writes[1], r),
            "primary producer metadata cannot overlap known provider/name/record backing");
    });
    require(!(bits(context_.actual_name_header8) & 3u), "primary producer needs an aligned real raw8B name header");
}
void NativeGuiTextPrimaryModelStorageFrame::prepare_before_text_allocation() {
    auto& c = context_;
    auto& a = acquired_;
    try {
        require(phase_ == Phase::fresh && a.phase == Phase::fresh && !pool_ && !a.receipt_.issued_ &&
            !a.tree_bind_entered && !a.node_bind_entered && !a.allocation_entered && !a.constructor_entered &&
            !a.canonical_bind_entered && !a.publication_entered && !a.actual_pool && !a.raw_slot && !a.failure,
            "primary producer prepares one fresh retained graph exactly once");
        phase_ = Phase::preparing; a.phase = Phase::preparing; a.boundary = Acquired::Boundary::preparation;
        require(!c.factory.started && c.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::fresh &&
            !c.factory.raw_slot && !c.factory_context.construction.started &&
            !c.publication.reference && !c.publication.bind_entered && !c.publication.bind_complete &&
            !c.publication.bind_failed && !c.publication.scalar_entered && !c.publication.scalar_returned &&
            !c.publication.scalar_failed && !c.publication.companion_disposal_entered && !c.publication.disposed &&
            !c.internal188.actual && !c.internal188.reference && !c.internal188.construction.started &&
            !c.internal188.lifetime.started && !c.internal188.release.started,
            "primary graph preparation must precede the real Text factory/native pool attempt");
        require_graph();
        // Existing private process constructor/canonical service; this ready-only
        // accessor NEVER initializes a pool. No global lazy getter is called.
        auto& pool = c.model_pools.model_pool_01090054();
        require(&pool == &c.primary_node_context.record.model.storage.pool_01090054,
            "primary producer must use the already-ready canonical process Model pool");
        pool_ = &pool; a.actual_pool = &pool;
        a.boundary = Acquired::Boundary::tree_bind; a.tree_bind_entered = true;
        c.primary_node_context.tree_dispatch.bind(c.primary_node_context.record);
        a.tree_bound = true;
        a.boundary = Acquired::Boundary::node_bind; a.node_bind_entered = true;
        c.primary_nodes.bind(c.primary_node_context);
        a.node_bound = true;
        a.phase = Phase::prepared; phase_ = Phase::prepared;
    } catch (...) { fail(); throw; }
}

void produce_native_gui_text_primary_model_00aaad95(Context& c,
    NativeGuiTextPrimaryModelStorageFrame& f, Acquired& a) {
    // Mismatched objects are outside this prepared API's caller contract.
    require(&f.context_ == &c && &f.acquired_ == &a, "primary producer requires its SAME prepared context/frame/acquisition");
    try {
        require(f.phase_ == Phase::prepared && a.phase == Phase::prepared && a.tree_bound && a.node_bound &&
            f.pool_ && a.actual_pool == f.pool_ && !a.allocation_entered && !a.receipt_.issued_,
            "primary prefix cannot enter an unprepared, replayed or failed graph");
        f.phase_ = Phase::entered; a.phase = Phase::entered;
        a.boundary = Acquired::Boundary::text_admission;
        f.require_graph();
        void* const text = bound_text(c);
        a.actual_text = text;
        require(f.pool_ == &c.primary_node_context.record.model.storage.pool_01090054,
            "primary pool binding changed after preparation");
        a.boundary = Acquired::Boundary::allocation; a.active_call_site = 0x00aaad95;
        a.allocation_entered = true; // BEFORE B74D00, including held-lock allocation failure.
        void* const raw = f.pool_->allocate_raw_slot_00b74d00();
        a.raw_slot = raw; a.allocation_returned = true;
        require(raw && !(bits(raw) & 3u), "primary prefix excludes the native null/fault allocation continuation");
        const Region payload{raw, NativeModelPool::slot_bytes};
        require(valid(payload) && disjoint(payload, {&f, sizeof(f)}) && disjoint(payload, {&a, sizeof(a)}) &&
            disjoint(payload, {text, 0x1f8}) && disjoint(payload, {c.internal188.actual, 0x188}),
            "real primary allocation cannot overlap Text/internal188 or producer metadata");
        borrowed(c, [&](Region r) { require(disjoint(payload, r), "primary payload cannot overlap known provider/name/record backing"); });
        c.primary.actual = raw;
        a.boundary = Acquired::Boundary::constructor_call; a.active_call_site = 0x00aaadb1;
        a.constructor_entered = true;
        void* const result = construct_native_model_storage_00b75030(raw, c.actual_name_header8,
            c.primary_node_context.record.model.storage, c.primary.construction);
        a.constructor_returned = true; // NORMAL return FIRST, before postcheck/bind/publication.
        a.constructor_result = result;
        require(result == raw && c.primary.construction.complete,
            "primary constructor must return its SAME real completed Model");
        a.boundary = Acquired::Boundary::canonical_bind; a.active_call_site = 0; // extra Source metadata boundary
        a.canonical_bind_entered = true;
        auto& reference = c.primary.reference.emplace(raw, c.primary_node_context.record.model,
            c.primary.construction, c.primary.lifetime_frame, c.primary.lifetime);
        a.canonical_reference = &reference; a.canonical_bound = true;
        require(c.factory_context.publication.owners.find(raw) == &reference &&
            static_cast<void*>(&reference.reference_count) == at(raw, 4),
            "primary canonical binding must borrow this constructor's existing actual+4");
        a.boundary = Acquired::Boundary::publication; a.active_call_site = 0x00aaadc4;
        a.publication_entered = true;
        write(text, 0x4c, bits(raw)); a.published = true; // transfers custody; NO retain
        a.boundary = Acquired::Boundary::flags_clear; a.active_call_site = 0x00aaadc9;
        write(raw, 0x138, read(raw, 0x138) & 0xfffffffcu); a.flags_cleared = true;
        auto& receipt = a.receipt_;
        receipt.text_ = text; receipt.model_ = raw; receipt.factory_context_ = &c.factory_context;
        receipt.factory_ = &c.factory; receipt.record_ = &c.primary;
        receipt.dispatch_ = &c.primary_node_context.record; receipt.pool_ = f.pool_;
        receipt.construction_ = &c.primary.construction;
        receipt.issued_ = true; // ONLY this producer can issue the provenance receipt.
        a.phase = Phase::complete; f.phase_ = Phase::complete; // completed PREFIX only
    } catch (...) {
        f.fail();
        throw; // ALL escapes retain: NO B74750, destructor, name return, pool unlock or rollback.
    }
}
} // namespace bsp
