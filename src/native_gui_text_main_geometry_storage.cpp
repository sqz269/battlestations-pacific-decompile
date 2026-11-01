#include "bsp/native_render_observed_zero.hpp"
namespace bsp {
class NativeGuiTextMainGeometryStorageObservedZeroIssuer final {
public:
    static void dispatch(NativeRenderActualOwners& owners, void* actual, NativeRenderObservedZeroFrame& outer) {
        NativeRenderObservedZeroAccess::dispatch(owners, actual, outer);
    }
};
} // namespace bsp
#include "bsp/native_gui_text_main_geometry_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_node_destruction.hpp"
#include <atomic>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextMainGeometryStorageContext;
using Acquired = NativeGuiTextMainGeometryStorageAcquired;
using Phase = NativeGuiTextMainGeometryStoragePhase;
constexpr Word text_profile = 0x00d5c6c8, model_profile = 0x00d62de8;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(bits(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
struct Region { const volatile void* data; std::size_t bytes; };
bool representable(Region r) noexcept {
    return r.data && static_cast<std::uint64_t>(bits(r.data)) + r.bytes <= 0x100000000ull;
}
bool disjoint(Region l, Region r) noexcept {
    const auto a = static_cast<std::uint64_t>(bits(l.data));
    const auto b = static_cast<std::uint64_t>(bits(r.data));
    return a + l.bytes <= b || b + r.bytes <= a;
}
template<class Check>
void model_borrowed(NativeModelStorageDispatchContext& m, Check check) {
    auto& s = m.model.storage;
    auto& n = s.node;
    for (const Region r : {Region{&m, sizeof(m)}, {&m.model, sizeof(m.model)},
        {&m.actual, sizeof(m.actual)}, {&m.reference, sizeof(m.reference)},
        {&m.construction, sizeof(m.construction)}, {&m.lifetime_frame, sizeof(m.lifetime_frame)},
        {&m.lifetime, sizeof(m.lifetime)}, {&m.release_frame, sizeof(m.release_frame)},
        {&m.release, sizeof(m.release)}, {&s, sizeof(s)}, {&s.pool_01090054, sizeof(s.pool_01090054)},
        {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())},
        {&s.constants, sizeof(s.constants)}, {&n, sizeof(n)}, {&n.strings, sizeof(n.strings)},
        {&n.world, sizeof(n.world)}, {&n.scenes, sizeof(n.scenes)},
        {&n.scenes.gates, sizeof(n.scenes.gates)}, {&n.scenes.owners, sizeof(n.scenes.owners)},
        {&n.scenes.increment_00ce221c, sizeof(n.scenes.increment_00ce221c)},
        {&n.scenes.decrement_00ce2220, sizeof(n.scenes.decrement_00ce2220)},
        {&n.trees, sizeof(n.trees)}, {&n.trees.frames, sizeof(n.trees.frames)},
        {&n.trees.dispatch, sizeof(n.trees.dispatch)}, {&n.trees.point_lights, sizeof(n.trees.point_lights)},
        {m.model.actual_profile_00d62de8, 0x58}}) check(r);
}
Word live_model(void* actual, NativeModelStorageDispatchContext& m) {
    auto& s = m.model.storage;
    require(actual && !(bits(actual) & 3u) && m.actual == actual && m.reference &&
        !m.reference->retired() && m.construction.started && m.construction.complete &&
        m.construction.metadata_bound && m.construction.actual_model == actual &&
        m.construction.context == &s && !m.lifetime.started && !m.lifetime.complete &&
        !m.lifetime.scalar_started && !m.lifetime.scalar_complete && !m.lifetime.flags_read &&
        !m.lifetime.pool_returned && !m.lifetime.node.started && !m.release.started &&
        s.node.scenes.owners.find(actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == at(actual, 4),
        "Text74 requires the SAME completed, canonical, still-live raw Model record");
    require(m.reference->reference_count.load(std::memory_order_relaxed) > 0,
        "Text74 raw Model must retain its actual native credit through remaining operations");
    // Existing481 private admission cannot be called here. Use only its public
    // actual pool/live-slot view; no companion, slot or count is manufactured.
    const auto& pool = s.pool_01090054.storage();
    const Word index = NativeModelPool::live_slab_index(actual);
    require(pool.slabs_28 && index < pool.slab_count_2c,
        "Text74 Model requires its real allocated pool slab");
    const auto* slab = pool.slabs_28[index];
    const Word offset = bits(actual) - bits(slab);
    require(slab && offset < NativeModelPool::slot_bytes * NativeModelPool::slots_per_slab &&
        offset % NativeModelPool::slot_bytes == 0,
        "Text74 Model requires its exact actual188h pool slot");
    const Word free_count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3140);
    require(free_count <= NativeModelPool::slots_per_slab, "Text74 Model pool free-count is invalid");
    const auto* free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3100);
    for (Word i = 0; i < free_count; ++i)
        require(free_slots[i] < NativeModelPool::slots_per_slab &&
            free_slots[i] != offset / NativeModelPool::slot_bytes,
            "Text74 cannot use returned or invalid Model pool storage");
    const Word profile = read(actual);
    require(profile == model_profile, "Text74 admits completed raw Models, not an active destructor prefix");
    return profile;
}
void same_node_domain(NativeModelStorageDispatchContext& m,
    const NativeGuiTextRawModelReferenceContext& text) {
    auto& r = text.construction_context.sections;
    auto& shared = r.model.storage;
    auto& s = m.model.storage;
    auto& n = s.node;
    require(&s.pool_01090054 == &shared.pool_01090054 && &s.constants == &shared.constants &&
        &s.negative_zero_00d7a208 == &shared.negative_zero_00d7a208 &&
        &n.strings == &r.strings && &n.world == &shared.node.world &&
        &n.scenes.owners == &text.owners && &n.trees.owners == &text.owners &&
        &n.trees.point_lights == &text.retirement.base.nodes.point_lights &&
        &n.scenes.increment_00ce221c == &shared.node.scenes.increment_00ce221c &&
        &n.scenes.decrement_00ce2220 == &shared.node.scenes.decrement_00ce2220 &&
        &n.trees.decrement_00ce2220 == &shared.node.trees.decrement_00ce2220 &&
        &n.scenes.decrement_00ce2220 == &n.trees.decrement_00ce2220 &&
        bits(m.model.actual_profile_00d62de8) == model_profile &&
        m.model.actual_profile_00d62de8 == text.data.data_at(model_profile, 0x58),
        "Text74 Model must borrow SAME real pool/constants/string/world/import/owner/point-light domains");
}
void separate_records(NativeModelStorageDispatchContext& first, NativeModelStorageDispatchContext& second) {
    const Region a[] = {{&first.actual, sizeof(first.actual)}, {&first.reference, sizeof(first.reference)},
        {&first.construction, sizeof(first.construction)}, {&first.lifetime_frame, sizeof(first.lifetime_frame)},
        {&first.lifetime, sizeof(first.lifetime)}, {&first.release_frame, sizeof(first.release_frame)},
        {&first.release, sizeof(first.release)}};
    const Region b[] = {{&second.actual, sizeof(second.actual)}, {&second.reference, sizeof(second.reference)},
        {&second.construction, sizeof(second.construction)}, {&second.lifetime_frame, sizeof(second.lifetime_frame)},
        {&second.lifetime, sizeof(second.lifetime)}, {&second.release_frame, sizeof(second.release_frame)},
        {&second.release, sizeof(second.release)}};
    for (const auto left : a) for (const auto right : b)
        require(disjoint(left, right), "Text74 primary/internal Model record cells cannot overlap");
}
// AB7747/7750/7756: one FLD, FST second scalar, FSTP first scalar.
// Native PUSH geometry/0 occurs afterward. No float-valued C++ snapshot or
// conversion crosses this helper; both actual operand cells stay mutable.
__declspec(noinline) void association_arguments(const volatile Word& sentinel,
    volatile Word (&arguments)[4], void* mesh) noexcept {
    const volatile void* source = &sentinel;
    volatile Word* slots = arguments;
    const Word geometry = bits(mesh);
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov edx, slots
        fst dword ptr [edx+0ch]
        fstp dword ptr [edx+8]
        mov eax, geometry
        mov dword ptr [edx+4], eax
        mov dword ptr [edx], 0
    }
}
__declspec(noinline) void position_arguments(volatile Word (&position)[3]) noexcept {
    volatile Word* slots = position;
    __asm {
        xorps xmm0, xmm0
        mov eax, slots
        movss dword ptr [eax], xmm0
        movss dword ptr [eax+4], xmm0
        movss dword ptr [eax+8], xmm0
    }
}
} // namespace

NativeGuiTextMainGeometryStorageFrame::NativeGuiTextMainGeometryStorageFrame(Context& c,
    volatile Word (&arguments)[4], volatile Word (&position)[3],
    NativeGuiWidgetTransformScratch& scratch, GuiWidgetBounds& bounds) noexcept
    : context_(c), arguments_(arguments), position_(position), transform_scratch_(scratch),
      bounds_scratch_(bounds), association_(arguments[0], arguments[1], arguments[2], arguments[3]),
      transform_bindings_{c.one_00d7a24c, c.negative_zero_00d7a208, c.y_scale_00e12fc4, *this} {}
NativeGuiTextMainGeometryStorageFrame::~NativeGuiTextMainGeometryStorageFrame() noexcept {
    if (phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
}
bool NativeGuiTextMainGeometryStorageFrame::matches(const Context& c, const Acquired& a,
    Phase expected) const noexcept {
    return &context_ == &c && acquired_ == &a && phase_ == expected;
}
bool NativeGuiTextMainGeometryStorageFrame::matches_backing(const Context& c,
    const volatile Word (&arguments)[4], const volatile Word (&position)[3],
    const NativeGuiWidgetTransformScratch& scratch, const GuiWidgetBounds& bounds) const noexcept {
    return &context_ == &c && &arguments_[0] == &arguments[0] && &position_[0] == &position[0] &&
        &transform_scratch_ == &scratch && &bounds_scratch_ == &bounds;
}
void NativeGuiTextMainGeometryStorageFrame::preflight(NativeGuiTextRawModelReference& reference,
    Acquired& a) {
    require(phase_ == Phase::fresh && !acquired_ && association_.phase() == NativeModelStorageGeometryPhase::fresh &&
        a.phase == Phase::fresh && a.boundary == Acquired::Boundary::none && !a.destination &&
        !a.primary_captured && !a.mesh.started && !a.mesh_entered && !a.association_entered &&
        a.association.phase == NativeModelStorageGeometryPhase::fresh && !a.creator_release_entered &&
        !a.position_entered && !a.transform.entered && !a.failure,
        "Text74 requires one fresh independently retained frame and acquisition");
    auto& c = context_;
    const auto& t = reference.storage_context();
    void* const raw = reference.storage();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        !(bits(raw) & 3u) && t.owners.find(raw) == &reference &&
        static_cast<void*>(&reference.reference_count) == at(raw, 4),
        "Text74 requires SAME canonical live raw122 Text identity");
    const auto& construction = t.construction;
    const auto& internal = construction.sections.model;
    auto& r = t.construction_context.sections;
    require(construction.started && construction.complete && construction.base_complete &&
        construction.derived_complete && construction.sections_entered && construction.destination == raw &&
        !construction.exception_cleanup_started && !construction.base_lifetime_ended &&
        !construction.consumed_cleanup_actions && construction.failure == decltype(construction.failure)::none &&
        construction.sections.complete && !construction.sections.exception_cleanup_started &&
        &c.internal188.model == &r.model && &c.internal188.actual == &internal.actual &&
        &c.internal188.reference == &internal.reference && &c.internal188.construction == &internal.construction &&
        &c.internal188.lifetime_frame == &internal.lifetime_frame && &c.internal188.lifetime == &internal.lifetime &&
        &c.internal188.release_frame == &internal.release_frame && &c.internal188.release == &internal.release &&
        &t.retirement.model == &internal && &t.retirement.resources == &r &&
        &c.association.storage == &c.primary.model.storage && &c.geometry == &r.geometry &&
        &c.geometry.actual_owners() == &t.owners && c.primary.actual != c.internal188.actual &&
        &c.primary.actual != &c.internal188.actual && &c.primary.reference != &c.internal188.reference &&
        &c.primary.construction != &c.internal188.construction &&
        &c.primary.lifetime_frame != &c.internal188.lifetime_frame &&
        &c.primary.lifetime != &c.internal188.lifetime && &c.primary.release_frame != &c.internal188.release_frame &&
        &c.primary.release != &c.internal188.release,
        "Text74 requires exact nested188 aliases and a distinct same-domain primary481 record");
    require(bits(&c.one_00d7a24c) == bits(t.data.data_at(0x00d7a24c, 4)) &&
        bits(&c.negative_zero_00d7a208) == bits(t.data.data_at(0x00d7a208, 4)) &&
        bits(&c.y_scale_00e12fc4) == bits(&t.construction_context.state.widget_y_scale_00e12fc4()) &&
        bits(&c.half_00d7a280) == bits(t.data.data_at(0x00d7a280, 8)) &&
        bits(&c.association.unchanged_00d7a260) == bits(t.data.data_at(0x00d7a260, 4)),
        "Text74 readonly constants and shared mutable Y-scale must keep their canonical domains");
    same_node_domain(c.primary, t);
    same_node_domain(c.internal188, t);
    separate_records(c.primary, c.internal188);
    const Region writes[] = {{this, sizeof(*this)}, {&a, sizeof(a)},
        {arguments_, sizeof(arguments_)}, {position_, sizeof(position_)},
        {&transform_scratch_, sizeof(transform_scratch_)}, {&bounds_scratch_, sizeof(bounds_scratch_)}};
    for (std::size_t i = 0; i < sizeof(writes) / sizeof(writes[0]); ++i) {
        require(representable(writes[i]) && !(bits(writes[i].data) & 3u),
            "Text74 writable frame regions must be aligned representable storage");
        for (std::size_t j = 0; j < i; ++j)
            require(disjoint(writes[i], writes[j]), "Text74 scratch/operands/records cannot overlap");
    }
    auto separate = [&](Region borrowed) {
        require(representable(borrowed), "Text74 requires valid known borrowed extents");
        for (const auto write : writes)
            require(disjoint(write, borrowed), "Text74 writable backing cannot overlap borrowed graph");
    };
    for (const Region borrowed : {Region{&reference, sizeof(reference)}, {raw, 0x1f8},
        {&c, sizeof(c)}, {&t, sizeof(t)}, {&t.data, sizeof(t.data)}, {&t.owners, sizeof(t.owners)},
        {&t.construction_context, sizeof(t.construction_context)}, {&t.construction_frame, sizeof(t.construction_frame)},
        {&construction, sizeof(construction)}, {&t.retirement, sizeof(t.retirement)}, {&r, sizeof(r)},
        {&r.materials, sizeof(r.materials)}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.effects, sizeof(r.effects)}, {&r.parenting, sizeof(r.parenting)},
        {&c.association, sizeof(c.association)}, {&c.geometry, sizeof(c.geometry)},
        {&c.transforms, sizeof(c.transforms)}, {&c.one_00d7a24c, 4}, {&c.negative_zero_00d7a208, 4},
        {&c.y_scale_00e12fc4, 4}, {&c.half_00d7a280, 8}, {&c.association.unchanged_00d7a260, 4},
        {c.primary.actual, 0x188}, {c.internal188.actual, 0x188}, {t.data.data_at(text_profile, 0x84), 0x84}})
        separate(borrowed);
    model_borrowed(c.primary, separate);
    model_borrowed(c.internal188, separate);
    require(disjoint({raw, 0x1f8}, {c.primary.actual, 0x188}) &&
        disjoint({raw, 0x1f8}, {c.internal188.actual, 0x188}) &&
        disjoint({c.primary.actual, 0x188}, {c.internal188.actual, 0x188}),
        "Text74 native Text/primary/internal payloads must be independent allocations");
    live_model(c.primary.actual, c.primary);
    live_model(c.internal188.actual, c.internal188);
    require(read(raw) == text_profile && read(raw, 0x188) == bits(c.internal188.actual) &&
        static_cast<const volatile Word*>(t.data.data_at(text_profile, 0x84))[0x74 / 4] == 0x00ab7700,
        "Text74 requires completed raw Text and its genuine current74 selector");
}
void NativeGuiTextMainGeometryStorageFrame::call_virtual38(Word target, void* actual,
    const void* matrix) {
    require(phase_ == Phase::entered && acquired_ && acquired_->phase == Phase::entered &&
        acquired_->position_entered && !acquired_->position_returned && !acquired_->transform.entered,
        "Text74 transform cannot recursively reuse its already-entered invocation");
    auto& call = acquired_->transform;
    call.receiver = actual;
    call.matrix = matrix;
    call.captured_target = target;
    call.entered = true; // BEFORE receiver admission and any reached raw body.
    try {
        auto& c = context_;
        NativeModelStorageDispatchContext* m = actual == c.primary.actual ? &c.primary :
            actual == c.internal188.actual ? &c.internal188 : nullptr;
        require(m, "Text74 current transform receiver is outside explicit canonical Model aliases");
        call.admitted_profile = live_model(actual, *m);
        require(target == 0x00b6db10, "Text74 transform requires captured genuine B6DB10");
        call.body_entered = true;
        set_raw_local_matrix_00b6db10(actual, c.transforms, matrix);
        call.body_returned = true; // metadata only after body/callbacks
    } catch (...) { call.failed = true; throw; }
}

void construct_native_gui_text_main_geometry_storage_00ab7700(NativeGuiTextRawModelReference& reference,
    Context& c, NativeGuiTextMainGeometryStorageFrame& f, Acquired& a) {
    require(&f.context_ == &c, "Text74 frame must retain its SAME prepared context");
    f.preflight(reference, a);
    f.acquired_ = &a;
    f.phase_ = Phase::entered;
    a.phase = Phase::entered;
    a.destination = reference.storage();
    try {
        a.boundary = Acquired::Boundary::model_capture;
        a.active_call_site = 0x00ab771d;
        void* const captured = reinterpret_cast<void*>(read(a.destination, 0x4c));
        a.captured_primary = captured;
        a.primary_captured = true;
        require(captured == c.primary.actual, "Text74 preallocation4C must be its explicit primary Model");
        a.boundary = Acquired::Boundary::mesh;
        a.active_call_site = 0x00ab7725;
        a.mesh_entered = true;
        auto* const mesh = c.geometry.create_native_mesh_00b73b60(a.mesh);
        a.captured_mesh = mesh;
        a.mesh_returned = true;
        require(mesh && a.mesh.constructor_complete && a.mesh.registered && a.mesh.companion,
            "Text74 native null-mesh/fault continuation is outside successful-allocation Source domain");
        a.boundary = Acquired::Boundary::association;
        a.active_call_site = 0x00ab7747;
        association_arguments(c.association.unchanged_00d7a260, f.arguments_, mesh);
        a.arguments_prepared = true;
        a.active_call_site = 0x00ab7764;
        a.association_entered = true;
        set_native_model_geometry_storage_00b75170(captured, f.association_, c.association, a.association);
        a.association_returned = true;
        a.boundary = Acquired::Boundary::creator_release;
        a.active_call_site = 0x00ab776d;
        const auto decrement = c.primary.model.storage.node.trees.decrement_00ce2220;
        a.creator_decrement = decrement; // FRESH current import AFTER B75170 callbacks.
        require(decrement != nullptr, "Text74 creator release requires genuine CURRENT CE2220");
        a.creator_release_entered = true;
        const long result = decrement(static_cast<volatile long*>(at(mesh, 4)));
        a.creator_result = result;
        a.creator_decremented = true;
        if (result == 0) {
            a.boundary = Acquired::Boundary::creator_zero;
            a.active_call_site = 0x00ab7777;
            a.creator_zero_profile = read(mesh);
            a.creator_profile_captured = true;
            const auto& t = reference.storage_context();
            require(a.creator_zero_profile == 0x00d62d60,
                "Text74 already-zero creator has unsupported current family; retain its obligation");
            const auto* table = static_cast<const volatile Word*>(t.data.data_at(a.creator_zero_profile, 8));
            require(bits(table) == a.creator_zero_profile,
                "Text74 already-zero creator requires the genuine captured current table alias");
            a.creator_zero_target = table[0];
            a.creator_target_captured = true;
            a.active_call_site = 0x00ab777d;
            require(a.creator_zero_target == 0x00bd30e0 && t.owners.find(mesh) == a.mesh.companion,
                "Text74 already-zero creator requires its SAME genuine current0 mesh companion");
            a.creator_zero_entered = true;
            NativeGuiTextMainGeometryStorageObservedZeroIssuer::dispatch(t.owners, mesh, a.cpp_zero_frame);
            a.creator_zero_returned = true;
        }
        a.creator_release_returned = true; // No mesh payload/reference access after terminal.
        a.boundary = Acquired::Boundary::position;
        a.active_call_site = 0x00ab777f;
        position_arguments(f.position_);
        a.position_prepared = true;
        a.active_call_site = 0x00ab779b;
        a.position_entered = true;
        set_native_gui_widget_local_position_00aa7dc0(a.destination,
            const_cast<const Word*>(&f.position_[0]), f.transform_scratch_, f.transform_bindings_,
            f.bounds_scratch_, c.half_00d7a280);
        a.position_returned = true;
        a.phase = Phase::complete;
        f.phase_ = Phase::complete; // ONLY independent frame metadata after the body.
    } catch (...) {
        a.escaped_boundary = a.boundary;
        a.escaped_call_site = a.active_call_site;
        a.failure = std::current_exception();
        a.phase = Phase::failed;
        f.phase_ = Phase::failed;
        throw; // No native cleanup, creator release, rollback, retry or payload rediscovery.
    }
}

NativeGuiTextPageMainGeometryStorageFrame::NativeGuiTextPageMainGeometryStorageFrame(
    NativeGuiTextPageMainGeometryStorageContext& c, volatile Word (&arguments)[4],
    volatile Word (&position)[3], NativeGuiWidgetTransformScratch& transform, GuiWidgetBounds& bounds) noexcept
    : context_(c), arguments_(arguments), position_(position), transform_scratch_(transform),
      bounds_scratch_(bounds), association_(arguments[0], arguments[1], arguments[2], arguments[3]),
      transform_bindings_{c.one_00d7a24c, c.negative_zero_00d7a208, c.y_scale_00e12fc4, *this} {}
NativeGuiTextPageMainGeometryStorageFrame::~NativeGuiTextPageMainGeometryStorageFrame() noexcept {
    if (phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
}
void* NativeGuiTextPageMainGeometryStorageFrame::require_primary_metadata() const {
    const auto& c = context_;
    const auto& reference = c.primary;
    const auto& owner = reference.owner_;
    const auto& facts = owner.raw_facts_;
    auto& lifetime = c.primary_lifetime;
    auto* const actual = &owner.storage.node;
    // These are private SOURCE return facts and addresses, not actual word
    // loads. In particular do not call require_checked_raw_live here: its
    // pool/count/profile reads belong to the existing setter's later stage.
    require(reference.phase_ == NativeModelReference::Phase::bound &&
        !reference.ordinary_post_effect_ && !owner.ordinary_post_effect_ &&
        owner.phase == NativeModelOwner::Phase::live && owner.raw_lifetime == &lifetime &&
        &owner.environment == &c.primary_environment && facts.entered && facts.returned &&
        !facts.failed && !facts.destruction_entered && !facts.typed_end_entered &&
        facts.node == actual && facts.tail == &owner.storage.model &&
        facts.count == &owner.storage.node.references_04 &&
        static_cast<const void*>(&reference.reference_count) == static_cast<const void*>(facts.count) &&
        facts.environment == &c.primary_environment && facts.binding == &lifetime &&
        facts.constants == &lifetime.storage.constants && facts.sentinel == &c.unchanged_00d7a260 &&
        &c.primary_environment.constants.unchanged_00d7a260 == &c.unchanged_00d7a260 &&
        !lifetime.host.failed && !lifetime.host.typed_storage_ended &&
        !lifetime.terminal.started && !lifetime.terminal.scalar_started &&
        !lifetime.terminal.complete && !lifetime.terminal.pool_returned &&
        &c.primary_environment.pool_01090054 == &lifetime.storage.pool_01090054 &&
        &c.primary_environment.retained_owners == &lifetime.storage.node.scenes.owners &&
        lifetime.storage.node.scenes.owners.find(actual) == &reference,
        "Page Text74 requires its SAME canonical private raw constructor-return record");
    return actual;
}
void NativeGuiTextPageMainGeometryStorageFrame::preflight(NativeGuiTextRawModelReference& reference) {
    auto& c = context_;
    const auto& t = reference.storage_context();
    const auto& construction = t.construction;
    auto& r = t.construction_context.sections;
    const auto& internal = construction.sections.model;
    auto& m = c.internal188;
    auto& s = c.primary_lifetime.storage;
    auto& n = s.node;
    void* const raw = reference.storage(); // SOURCE canonical identity only.
    void* const primary = require_primary_metadata();
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && raw &&
        !(bits(raw) & 3u) && t.owners.find(raw) == &reference &&
        static_cast<void*>(&reference.reference_count) == at(raw, 4) &&
        construction.started && construction.complete && construction.base_complete &&
        construction.derived_complete && construction.sections_entered && construction.destination == raw &&
        !construction.exception_cleanup_started && !construction.base_lifetime_ended &&
        !construction.consumed_cleanup_actions && construction.failure == decltype(construction.failure)::none &&
        construction.sections.complete && !construction.sections.exception_cleanup_started &&
        &m.model == &r.model && &m.actual == &internal.actual && &m.reference == &internal.reference &&
        &m.construction == &internal.construction && &m.lifetime_frame == &internal.lifetime_frame &&
        &m.lifetime == &internal.lifetime && &m.release_frame == &internal.release_frame &&
        &m.release == &internal.release && &t.retirement.model == &internal &&
        &t.retirement.resources == &r && &c.geometry == &r.geometry && &c.geometry.actual_owners() == &t.owners &&
        m.actual && m.actual != primary && m.reference && !m.reference->retired() &&
        m.construction.started && m.construction.complete && m.construction.metadata_bound &&
        m.construction.actual_model == m.actual && m.construction.context == &m.model.storage &&
        !m.lifetime.started && !m.lifetime.scalar_started && !m.release.started &&
        t.owners.find(m.actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == at(m.actual, 4),
        "Page Text74 requires SAME canonical raw Text and distinct completed internal188 metadata");
    same_node_domain(m, t); // Reference/address comparisons; no Native payload.
    const auto& shared = r.model.storage;
    require(&s.pool_01090054 == &shared.pool_01090054 && &s.constants == &shared.constants &&
        &s.negative_zero_00d7a208 == &shared.negative_zero_00d7a208 &&
        &n.strings == &r.strings && &n.world == &shared.node.world &&
        &n.scenes.owners == &t.owners && &n.trees.owners == &t.owners &&
        &n.trees.point_lights == &t.retirement.base.nodes.point_lights &&
        &n.scenes.increment_00ce221c == &shared.node.scenes.increment_00ce221c &&
        &n.scenes.decrement_00ce2220 == &shared.node.scenes.decrement_00ce2220 &&
        &n.trees.decrement_00ce2220 == &shared.node.trees.decrement_00ce2220 &&
        &n.scenes.decrement_00ce2220 == &n.trees.decrement_00ce2220 &&
        &c.primary_environment.nodes.point_lights == &n.trees.point_lights &&
        &c.primary_environment.constants.minimum_00ce4adc == &s.constants.negative_bound_00ce4adc &&
        &c.primary_environment.constants.maximum_00ce4970 == &s.constants.positive_bound_00ce4970 &&
        bits(&c.one_00d7a24c) == bits(t.data.data_at(0x00d7a24c, 4)) &&
        bits(&c.negative_zero_00d7a208) == bits(t.data.data_at(0x00d7a208, 4)) &&
        bits(&c.y_scale_00e12fc4) == bits(&t.construction_context.state.widget_y_scale_00e12fc4()) &&
        bits(&c.half_00d7a280) == bits(t.data.data_at(0x00d7a280, 8)) &&
        bits(&c.unchanged_00d7a260) == bits(t.data.data_at(0x00d7a260, 4)),
        "Page Text74 requires SAME real pool/owner/string/world/import/constant domains");
    const Region writes[] = {{this, sizeof(*this)}, {acquired_, sizeof(*acquired_)},
        {&arguments_[0], sizeof(arguments_)}, {&position_[0], sizeof(position_)},
        {&transform_scratch_, sizeof(transform_scratch_)}, {&bounds_scratch_, sizeof(bounds_scratch_)}};
    auto separate = [&](Region borrowed) {
        require(representable(borrowed), "Page Text74 borrowed backing must have a representable extent");
        for (const auto w : writes)
            require(disjoint(w, borrowed), "Page Text74 writable backing overlaps its borrowed graph");
    };
    for (std::size_t i = 0; i != sizeof(writes) / sizeof(*writes); ++i) {
        require(representable(writes[i]), "Page Text74 requires real retained writable backing");
        for (std::size_t j = i + 1; j != sizeof(writes) / sizeof(*writes); ++j)
            require(disjoint(writes[i], writes[j]), "Page Text74 writable regions cannot overlap");
    }
    const auto& owner = c.primary.owner_;
    for (const Region b : {Region{raw, 0x1f8}, {&reference, sizeof(reference)}, {&t, sizeof(t)},
        {&t.owners, sizeof(t.owners)}, {&t.data, sizeof(t.data)}, {&t.construction, sizeof(t.construction)},
        {&t.construction_context, sizeof(t.construction_context)},
        {&t.construction_frame, sizeof(t.construction_frame)}, {&t.retirement, sizeof(t.retirement)},
        {&r, sizeof(r)}, {&c, sizeof(c)}, {&c.primary, sizeof(c.primary)}, {&owner, sizeof(owner)},
        {&c.primary_environment, sizeof(c.primary_environment)}, {&c.primary_lifetime, sizeof(c.primary_lifetime)},
        {&c.primary_lifetime.terminal_frame, sizeof(c.primary_lifetime.terminal_frame)},
        {&c.primary_lifetime.terminal, sizeof(c.primary_lifetime.terminal)},
        {&c.primary_lifetime.host, sizeof(c.primary_lifetime.host)}, {&s, sizeof(s)}, {&n, sizeof(n)},
        {&n.scenes, sizeof(n.scenes)}, {&n.scenes.owners, sizeof(n.scenes.owners)},
        {&n.trees, sizeof(n.trees)}, {&n.trees.frames, sizeof(n.trees.frames)},
        {&n.trees.point_lights, sizeof(n.trees.point_lights)}, {&n.world, sizeof(n.world)},
        {&n.strings, sizeof(n.strings)}, {&s.pool_01090054, sizeof(s.pool_01090054)},
        {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())}, {&s.constants, sizeof(s.constants)},
        {&c.geometry, sizeof(c.geometry)}, {&c.transforms, sizeof(c.transforms)},
        {&c.one_00d7a24c, 4}, {&c.negative_zero_00d7a208, 4}, {&c.y_scale_00e12fc4, 4},
        {&c.half_00d7a280, 8}, {&c.unchanged_00d7a260, 4}, {primary, 0x188}, {m.actual, 0x188}})
        separate(b);
    model_borrowed(m, separate);
    require(disjoint({raw, 0x1f8}, {primary, 0x188}) &&
        disjoint({raw, 0x1f8}, {m.actual, 0x188}) && disjoint({primary, 0x188}, {m.actual, 0x188}),
        "Page Text74 actual Text/primary/internal188 must be distinct allocations");
}
void NativeGuiTextPageMainGeometryStorageFrame::call_virtual38(Word target, void* actual,
    const void* matrix) {
    require(phase_ == Phase::entered && acquired_ && acquired_->phase == Phase::entered &&
        acquired_->position_entered && !acquired_->position_returned && !acquired_->transform.entered,
        "Page Text74 transform requires its SAME active independent invocation");
    auto& call = acquired_->transform;
    call.receiver = actual;
    call.matrix = matrix;
    call.captured_target = target;
    call.entered = true;
    try {
        void* const primary = require_primary_metadata();
        require(actual == primary || (actual == context_.internal188.actual &&
            context_.internal188.reference && !context_.internal188.reference->retired()),
            "Page Text74 current transform requires its SAME admitted primary/internal188");
        require(target == 0x00b6db10, "Page Text74 transform requires captured genuine B6DB10");
        // AA7220 already captured the actual current target. Do not add another
        // profile/count/pool read; admitted_profile remains unobserved (zero).
        call.body_entered = true;
        set_raw_local_matrix_00b6db10(actual, context_.transforms, matrix);
        call.body_returned = true;
    } catch (...) { call.failed = true; throw; }
}
void construct_native_gui_text_main_geometry_storage_00ab7700(NativeGuiTextRawModelReference& reference,
    NativeGuiTextPageMainGeometryStorageContext& c, NativeGuiTextPageMainGeometryStorageFrame& f, Acquired& a) {
    try {
        require(&f.context_ == &c && f.phase_ == Phase::fresh && !f.acquired_ &&
            f.association_.phase() == NativeModelStorageGeometryPhase::fresh &&
            a.phase == Phase::fresh && a.boundary == Acquired::Boundary::none && !a.destination &&
            !a.primary_captured && !a.mesh.started && !a.mesh_entered && !a.association_entered &&
            a.association.phase == NativeModelStorageGeometryPhase::fresh && !a.creator_release_entered &&
            !a.position_entered && !a.transform.entered && !a.failure,
            "Page Text74 requires one fresh SAME context/frame/acquisition");
        f.acquired_ = &a;
        f.phase_ = Phase::entered; a.phase = Phase::entered; // BEFORE all admission/effects.
        a.destination = reference.storage(); // independent SOURCE identity.
        f.preflight(reference);
        a.boundary = Acquired::Boundary::model_capture; a.active_call_site = 0x00ab771d;
        void* const captured = reinterpret_cast<void*>(read(a.destination, 0x4c));
        a.captured_primary = captured; a.primary_captured = true;
        require(captured == f.require_primary_metadata(), "Page Text74 actual4C differs from its SAME primary");
        a.boundary = Acquired::Boundary::mesh; a.active_call_site = 0x00ab7725;
        a.mesh_entered = true;
        auto* const mesh = c.geometry.create_native_mesh_00b73b60(a.mesh);
        a.captured_mesh = mesh; a.mesh_returned = true;
        require(mesh && a.mesh.constructor_complete && a.mesh.registered && a.mesh.companion,
            "Page Text74 native null-mesh/fault continuation is outside successful-allocation Source domain");
        a.boundary = Acquired::Boundary::association; a.active_call_site = 0x00ab7747;
        association_arguments(c.unchanged_00d7a260, f.arguments_, mesh);
        a.arguments_prepared = true; a.active_call_site = 0x00ab7764; a.association_entered = true;
        c.primary.set_raw_geometry_00b75170(f.association_, a.association);
        a.association_returned = true;
        (void)f.require_primary_metadata(); // Source-only caught-failure/identity check.
        a.boundary = Acquired::Boundary::creator_release; a.active_call_site = 0x00ab776d;
        const auto decrement = c.primary_lifetime.storage.node.trees.decrement_00ce2220;
        a.creator_decrement = decrement; // CURRENT import AFTER genuine setter callbacks.
        require(decrement != nullptr, "Page Text74 creator release requires genuine CURRENT CE2220");
        a.creator_release_entered = true;
        const long result = decrement(static_cast<volatile long*>(at(mesh, 4)));
        a.creator_result = result; a.creator_decremented = true;
        if (result == 0) {
            a.boundary = Acquired::Boundary::creator_zero; a.active_call_site = 0x00ab7777;
            a.creator_zero_profile = read(mesh); a.creator_profile_captured = true;
            const auto& t = reference.storage_context();
            require(a.creator_zero_profile == 0x00d62d60,
                "Page Text74 already-zero creator has unsupported current family");
            const auto* table = static_cast<const volatile Word*>(t.data.data_at(a.creator_zero_profile, 8));
            require(bits(table) == a.creator_zero_profile, "Page Text74 zero requires SAME captured current table");
            a.creator_zero_target = table[0]; a.creator_target_captured = true; a.active_call_site = 0x00ab777d;
            require(a.creator_zero_target == 0x00bd30e0 && t.owners.find(mesh) == a.mesh.companion,
                "Page Text74 already-zero creator requires its SAME genuine mesh companion");
            a.creator_zero_entered = true;
            NativeGuiTextMainGeometryStorageObservedZeroIssuer::dispatch(t.owners, mesh, a.cpp_zero_frame);
            a.creator_zero_returned = true;
        }
        a.creator_release_returned = true; // No further mesh payload/reference access.
        a.boundary = Acquired::Boundary::position; a.active_call_site = 0x00ab777f;
        position_arguments(f.position_); a.position_prepared = true;
        a.active_call_site = 0x00ab779b; a.position_entered = true;
        set_native_gui_widget_local_position_00aa7dc0(a.destination,
            const_cast<const Word*>(&f.position_[0]), f.transform_scratch_, f.transform_bindings_,
            f.bounds_scratch_, c.half_00d7a280);
        a.position_returned = true; a.phase = Phase::complete; f.phase_ = Phase::complete;
    } catch (...) {
        a.escaped_boundary = a.boundary; a.escaped_call_site = a.active_call_site;
        a.failure = std::current_exception(); a.phase = Phase::failed; f.phase_ = Phase::failed;
        throw; // Retain entire reached graph; no Native cleanup, retry or extra credit.
    }
}
} // namespace bsp
