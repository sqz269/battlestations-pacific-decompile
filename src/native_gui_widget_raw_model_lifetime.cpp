#include "bsp/native_gui_widget_raw_model_lifetime.hpp"
#include "bsp/native_gui_text_font_lifetime_storage.hpp"
#include "bsp/native_node_destruction.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw widget Model lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeGuiWidgetRawModelLifetimeAcquired::Phase;
void require(bool b, const char* message) { if (!b) throw std::logic_error(message); }
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word read(void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
struct Region { const volatile void* p; std::size_t n; };
bool valid(Region r) noexcept {
    return (r.p || !r.n) && static_cast<std::uint64_t>(bits(r.p)) + r.n <= 0x100000000ull;
}
bool disjoint(Region a, Region b) noexcept {
    return static_cast<std::uint64_t>(bits(a.p)) + a.n <= bits(b.p) ||
        static_cast<std::uint64_t>(bits(b.p)) + b.n <= bits(a.p);
}
template<class Check> void borrowed(NativeGuiWidgetRawModelLifetimeContext& c, Check check) {
    auto& f = c.text.factory_context;
    for (const Region r : {Region{&c, sizeof(c)}, {&c.text, sizeof(c.text)},
        {&c.authentication, sizeof(c.authentication)}, {&c.widget, sizeof(c.widget)},
        {&c.text.factory, sizeof(c.text.factory)}, {&c.text.publication, sizeof(c.text.publication)},
        {&f, sizeof(f)}, {&f.publication, sizeof(f.publication)},
        {&f.construction, sizeof(f.construction)}, {&f.reference_frame, sizeof(f.reference_frame)},
        {&f.publication.construction_context, sizeof(f.publication.construction_context)},
        {&f.publication.construction_frame, sizeof(f.publication.construction_frame)},
        {&f.publication.retirement, sizeof(f.publication.retirement)},
        {f.companion_storage, f.companion_storage_bytes}, {f.disposal.context, f.disposal.context_bytes}}) check(r);
    for (auto* route : {&c.primary, &c.internal188}) {
        auto& m = route->record;
        auto& n = m.model.storage.node;
        for (const Region r : {Region{route, sizeof(*route)}, {&m, sizeof(m)},
            {&m.model, sizeof(m.model)}, {&m.model.storage, sizeof(m.model.storage)}, {&n, sizeof(n)},
            {&n.scenes, sizeof(n.scenes)}, {&n.trees, sizeof(n.trees)},
            {&m.actual, sizeof(m.actual)}, {&m.reference, sizeof(m.reference)},
            {&m.construction, sizeof(m.construction)}, {&m.lifetime_frame, sizeof(m.lifetime_frame)},
            {&m.lifetime, sizeof(m.lifetime)}, {&m.release_frame, sizeof(m.release_frame)},
            {&m.release, sizeof(m.release)}, {m.model.actual_profile_00d62de8, 0x58},
            {&route->tree, sizeof(route->tree)},
            {&route->tree_acquired, sizeof(route->tree_acquired)}, {&route->node, sizeof(route->node)},
            {&route->node_acquired, sizeof(route->node_acquired)}}) check(r);
    }
}
void require_fresh(NativeGuiWidgetRawModelRoute& q) {
    auto& m = q.record;
    require(!m.actual && !m.reference && !m.construction.started && !m.construction.complete &&
        !m.construction.metadata_bound && !m.lifetime.started && !m.lifetime.complete &&
        !m.lifetime.scalar_started && !m.lifetime.scalar_complete && !m.lifetime.flags_read &&
        !m.lifetime.pool_returned && !m.lifetime.node.started && !m.lifetime.geometry_unwind.started &&
        !m.release.started && !m.release.complete && q.tree_acquired.bound && !q.tree_acquired.poisoned &&
        q.tree_acquired.current18.phase == NativeModelStorageDispatchCallAcquired::Phase::fresh &&
        q.tree_acquired.current0.phase == NativeModelStorageDispatchCallAcquired::Phase::fresh &&
        q.node_acquired.bound && !q.node_acquired.poisoned && !q.node_acquired.used,
        "raw widget Models require exact fresh records and genuine pre-bound e769/2049 providers");
    auto& n = m.model.storage.node;
    require(&n.trees.dispatch == static_cast<NativeNodeTreeRetirementDispatch*>(&q.tree) &&
        &n.scenes.children == static_cast<NativeNodeSceneChildDispatch*>(&q.node) &&
        n.scenes.gates.derived == static_cast<NativeSceneRegistrationDerivedTypes*>(&q.node),
        "raw widget Model record must retain its concrete genuine private dispatchers");
}
} // namespace

NativeGuiWidgetRawModelSlots::NativeGuiWidgetRawModelSlots(NativeGuiWidgetRawModelLifetimeAcquired& a) noexcept
    : acquired_(a) {}
NativeGuiWidgetRawModelSlots::~NativeGuiWidgetRawModelSlots() {
    if (acquired_.active || acquired_.poisoned) std::terminate();
}
void NativeGuiWidgetRawModelSlots::bind(NativeGuiWidgetRawModelLifetimeContext& c) {
    require(!context_ && !acquired_.bound && !acquired_.active && !acquired_.poisoned &&
        acquired_.scene == Phase::fresh && acquired_.base == Phase::fresh && acquired_.glyph == Phase::fresh &&
        !acquired_.receiver && !acquired_.query_target && !acquired_.descriptor && !acquired_.query_result,
        "raw widget slots bind once before all calls");
    for (const auto* call : {&acquired_.scene188, &acquired_.scene4c, &acquired_.late4c})
        require(!call->captured && !call->entered && !call->returned &&
            !call->unlink.started && !call->unlink.complete,
            "raw widget outer B6DFA0 records must be independently fresh before binding");
    require(&c.slots == this && &c.authentication.context() == &c.widget &&
        &c.widget.bindings == static_cast<NativeGuiWidgetLifetimeBindings*>(&c.authentication) &&
        &c.text.factory_context.publication.construction_context.lifetime == &c.widget &&
        &c.text.factory_context.publication.retirement.base == &c.widget &&
        !c.text.factory.started && !c.text.factory.raw_slot && !c.text.publication.bind_entered,
        "raw widget slots require SAME prepared Text/private widget history before pool entry");
    auto& a = c.primary.record;
    auto& b = c.internal188.record;
    auto& nested = c.text.factory_context.construction.sections.model;
    auto& sections = c.text.factory_context.publication.construction_context.sections;
    require(&b.model == &sections.model && &b.actual == &nested.actual && &b.reference == &nested.reference &&
        &b.construction == &nested.construction && &b.lifetime_frame == &nested.lifetime_frame &&
        &b.lifetime == &nested.lifetime && &b.release_frame == &nested.release_frame && &b.release == &nested.release,
        "raw widget internal188 must be EXACTLY the independent e75 Text85 Model record");
    require(&a != &b && &a.model.storage != &b.model.storage && &a.actual != &b.actual &&
        &a.reference != &b.reference && &a.construction != &b.construction &&
        &a.lifetime_frame != &b.lifetime_frame && &a.lifetime != &b.lifetime &&
        &a.release_frame != &b.release_frame && &a.release != &b.release,
        "raw widget primary and internal188 cannot share Model lifetime cells");
    require_fresh(c.primary); require_fresh(c.internal188);
    auto& p = a.model.storage.node;
    auto& i = b.model.storage.node;
    require(&a.model.storage.pool_01090054 == &b.model.storage.pool_01090054 &&
        a.model.actual_profile_00d62de8 == b.model.actual_profile_00d62de8 && a.model.actual_profile_00d62de8 &&
        &p.scenes.owners == &sections.geometry.actual_owners() && &i.scenes.owners == &p.scenes.owners &&
        &p.trees.owners == static_cast<NativeRenderActualOwners*>(&p.scenes.owners) &&
        &i.trees.owners == &p.trees.owners && &p.strings == &i.strings &&
        c.widget.nodes.uses_raw_name_pool() && &p.strings == &c.widget.nodes.require_raw_name_pool() &&
        &p.world == &i.world && &p.trees.frames == &i.trees.frames &&
        &p.trees.point_lights == &c.widget.nodes.point_lights && &i.trees.point_lights == &p.trees.point_lights &&
        &p.scenes.increment_00ce221c == &i.scenes.increment_00ce221c &&
        &p.scenes.decrement_00ce2220 == &i.scenes.decrement_00ce2220 &&
        &p.trees.decrement_00ce2220 == &p.scenes.decrement_00ce2220 &&
        &i.trees.decrement_00ce2220 == &p.trees.decrement_00ce2220,
        "raw widget Models require SAME actual pool/owners/string/world/frame/point-light/current import domain");
    const Region writes[] = {{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)}};
    require(valid(writes[0]) && valid(writes[1]) && disjoint(writes[0], writes[1]),
        "raw widget slot metadata must be separate");
    borrowed(c, [&](Region r) {
        require(valid(r) && disjoint(writes[0], r) && disjoint(writes[1], r),
            "raw widget slot writes cannot alias known producer backing");
    });
    context_ = &c; acquired_.bound = true; // no getter, table value or native effect
}
NativeGuiWidgetRawModelLifetimeContext& NativeGuiWidgetRawModelSlots::context() {
    require(context_ && acquired_.bound && !acquired_.poisoned, "raw widget slots are unbound/poisoned");
    return *context_;
}
void NativeGuiWidgetRawModelSlots::authenticate(void* actual) {
    auto& c = context();
    const Region text{actual, 0x1f8};
    require(actual && !(bits(actual) & 3u) && valid(text) &&
        disjoint(text, {this, sizeof(*this)}) && disjoint(text, {&acquired_, sizeof(acquired_)}),
        "raw Text cannot overlap transport diagnostics before receiver authentication");
    borrowed(c, [&](Region r) { require(disjoint(text, r), "raw Text cannot overlap known provider backing"); });
    for (auto* route : {&c.primary, &c.internal188})
        if (route->record.actual) require(disjoint(text, {route->record.actual, 0x188}),
            "Text and independent raw Model slots must be separate");
    c.authentication.require_raw_model_transport_receiver(actual, c.text);
    const auto& d = c.text.factory_context.reference_frame.scalar.destruction;
    require(c.text.publication.scalar_entered && d.started && d.destination == actual &&
        d.base_entered && !d.base_returned && d.native_eh_state == -1,
        "raw widget transport requires genuine current raw118 base-entry records");
}
void NativeGuiWidgetRawModelSlots::require_domain(void* actual) {
    // FIRST domain only. No188 read: raw118 may have retired that Model already.
    require(!read(actual, 0x70) && !read(actual, 0x6c) &&
        !read(actual, 0x88) && !read(actual, 0x8c) && !read(actual, 0x90) &&
        !read(actual, 0x180) && !read(actual, 0x184) &&
        !read(actual, 0x18c) && !read(actual, 0x190) &&
        !read(actual, 0x198) && !read(actual, 0x19c) && !read(actual, 0x1a0) &&
        !read(actual, 0xec) && !read(actual, 0xf0) && !read(actual, 0xf4) && !read(actual, 0xf8) &&
        !read(actual, 0x1a4) && !read(actual, 0x1a8) && !read(actual, 0x1c0) && !read(actual, 0x1c4),
        "raw widget transport excludes parent/child/timed/glyph/content state");
    auto& c = context();
    auto& f = c.text.factory_context;
    auto* const font = f.publication.retirement.font_lifetime;
    if (font) {
        using Font = NativeGuiTextFontLifetimeStorageFrame;
        const auto* const prefix = font->prepared_context().geometry.prefix;
        require(font->phase() == Font::Phase::sealed && prefix &&
            &prefix->widget == &c && &prefix->text == &c.text,
            "raw widget Font boundary needs its exact sealed prefix/helper graph");
        // authenticate() already established the genuine raw118 base entry.
        // Its real bit6 return has ended the owned header: validate retained
        // records only, never reread Text108/1EC/1C8/1CC or ended payloads.
        font->require_live(actual, Font::Stage::base, &f.reference_frame.scalar.destruction);
    } else {
        require(!read(actual, 0x108) && !read(actual, 0x1ec) &&
            !read(actual, 0x1c8) && !read(actual, 0x1cc),
            "raw widget without sealed Font provenance keeps its original empty Font fields");
    }
    void* head = reinterpret_cast<void*>(read(actual, 0x68));
    require(head && !(bits(head) & 3u) && read(head) == bits(head) && read(head, 4) == bits(head),
        "raw widget transport requires the genuine finite empty AA9390 list");
}
void NativeGuiWidgetRawModelSlots::begin_base(NativeGuiWidgetRawModelLifetimeContext& c, void* actual) {
    require(&context() == &c, "raw widget base needs its SAME bound context");
    if (acquired_.active || acquired_.base != Phase::fresh) reject("raw widget base cannot reenter/replay");
    acquired_.active = true; acquired_.base = Phase::entered; acquired_.receiver = actual;
    authenticate(actual); require_domain(actual);
}
void NativeGuiWidgetRawModelSlots::begin_scene(NativeGuiWidgetRawModelLifetimeContext& c,
    void* actual, bool nested) {
    require(&context() == &c, "raw widget scene needs its SAME bound context");
    if (acquired_.scene != Phase::fresh || (nested ?
        (!acquired_.active || acquired_.base != Phase::entered || acquired_.receiver != actual) : acquired_.active))
        reject("raw widget scene cannot reenter/replay or use another base receiver");
    acquired_.active = true; acquired_.scene = Phase::entered; acquired_.receiver = actual;
    authenticate(actual); require_domain(actual);
    require(read(actual) == 0x00d5c130, "raw scene entry is ONLY the genuine active base prefix, never bound Text");
}
void NativeGuiWidgetRawModelSlots::boundary(void* actual) {
    authenticate(actual); require_domain(actual);
}
void NativeGuiWidgetRawModelSlots::finish_scene(bool nested) noexcept {
    acquired_.scene = Phase::complete; if (!nested) acquired_.active = false;
}
void NativeGuiWidgetRawModelSlots::finish_base() noexcept {
    acquired_.base = Phase::complete; acquired_.active = false;
}
void NativeGuiWidgetRawModelSlots::query(Word target, Word descriptor, bool result) noexcept {
    acquired_.query_target = target; acquired_.descriptor = descriptor; acquired_.query_result = result;
}
void NativeGuiWidgetRawModelSlots::release_captured(void* actual, NativeGuiWidgetRawModelReleaseAcquired& call) {
    auto& c = context();
    require(acquired_.active && (&call == &acquired_.scene4c || &call == &acquired_.late4c) &&
        !call.entered && !call.unlink.started && !call.returned, "raw Model release requires its fresh exact native site");
    call.entered = true; call.captured = actual;
    NativeGuiWidgetRawModelRoute* route = nullptr;
    if (actual && c.primary.record.actual == actual) route = &c.primary;
    if (actual && c.internal188.record.actual == actual) {
        require(!route, "raw widget Model identity is ambiguous"); route = &c.internal188;
    }
    require(route, "raw widget captured an unowned Model replacement");
    auto& m = route->record;
    require(m.reference && !m.reference->retired() && m.construction.complete && m.construction.metadata_bound &&
        m.construction.node_constructor_returned && m.construction.tail_constructed &&
        m.construction.actual_model == actual && m.construction.context == &m.model.storage &&
        !m.lifetime.started && !m.lifetime.complete && !m.lifetime.scalar_started && !m.lifetime.scalar_complete &&
        !m.lifetime.flags_read && !m.lifetime.pool_returned && !m.lifetime.node.started &&
        !m.lifetime.node.complete && !m.lifetime.node.base_cleanup_completed && !m.lifetime.geometry_unwind.started &&
        !m.release.started && !m.release.complete && !route->tree_acquired.poisoned && !route->node_acquired.poisoned &&
        route->tree_acquired.current18.phase == NativeModelStorageDispatchCallAcquired::Phase::fresh &&
        route->tree_acquired.current0.phase == NativeModelStorageDispatchCallAcquired::Phase::fresh &&
        m.model.storage.node.scenes.owners.find(actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == reinterpret_cast<void*>(bits(actual) + 4u),
        "raw widget release requires the SAME live canonical481 first-credit records BEFORE payload reads");
    const Region payload{actual, 0x188};
    require(valid(payload) && !(bits(actual) & 3u) && disjoint(payload, {this, sizeof(*this)}) &&
        disjoint(payload, {&acquired_, sizeof(acquired_)}) &&
        disjoint(payload, {acquired_.receiver, 0x1f8}), "raw Model cannot overlap Text/slot diagnostics");
    borrowed(c, [&](Region r) { require(disjoint(payload, r), "raw Model cannot overlap known provider backing"); });
    require(m.reference->reference_count.load(std::memory_order_relaxed) > 0 && read(actual) == 0x00d62de8 &&
        !read(actual, 0x30) && !read(actual, 0x34) && !read(actual, 0x38) && !read(actual, 0x3c) &&
        !read(actual, 0x40) && !read(actual, 0xa0) && !read(actual, 0xa4) && !read(actual, 0x130) &&
        !read(actual, 0x164) && !read(actual, 0x168) && !read(actual, 0x16c) &&
        !read(actual, 0x170) && !read(actual, 0x174) &&
        !*reinterpret_cast<const volatile unsigned char*>(bits(actual) + 0x44u),
        "raw widget release admits only unattached default Model first tree credit");
    unlink_native_node_tree_00b6dfa0(actual, m.model.storage.node.trees, call.unlink);
    require(call.unlink.complete, "raw B6DFA0 returned without complete independent evidence");
    call.returned = true; // metadata ONLY; the Model payload may have ended
}
void NativeGuiWidgetRawModelSlots::fail() noexcept {
    acquired_.poisoned = true;
    if (acquired_.base == Phase::entered) acquired_.base = Phase::failed;
    if (acquired_.scene == Phase::entered) acquired_.scene = Phase::failed;
    if (acquired_.glyph == Phase::entered) acquired_.glyph = Phase::failed;
    // active remains visible after any entered escape. Never auto-dispose.
}
void NativeGuiWidgetRawModelSlots::reject(const char* message) {
    fail(); throw std::logic_error(message);
}
void NativeGuiWidgetRawModelSlots::reject_glyph(
    NativeGuiWidgetRawModelLifetimeContext& c, void* actual) {
    require(&context() == &c, "raw glyph rejection requires SAME bound context");
    acquired_.glyph = Phase::entered;
    acquired_.active = true;
    authenticate(actual);
    reject("TRUE Text188 cleanup is outside this first raw widget transport domain");
}
} // namespace bsp
