#include "bsp/native_model_storage_dispatch.hpp"
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Model dispatch requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeModelStorageDispatchCallAcquired::Phase;
constexpr Word model_profile = 0x00d62de8;
constexpr Word release_target = 0x00b6f310;
constexpr Word zero_target = 0x00bd30e0;
constexpr Word scalar_target = 0x00b75290;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word read(void* actual, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(actual) + offset);
}
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
struct Region { const volatile void* data; std::size_t size; };
bool disjoint(Region l, Region r) noexcept {
    const auto left = static_cast<std::uint64_t>(bits(l.data));
    const auto right = static_cast<std::uint64_t>(bits(r.data));
    return left + l.size <= right || right + r.size <= left;
}
template<class Check>
void borrowed(NativeModelStorageDispatchContext& c, Check check) {
    auto& s = c.model.storage;
    auto& n = s.node;
    for (const Region r : {Region{&c, sizeof(c)}, {&c.model, sizeof(c.model)},
        {&c.actual, sizeof(c.actual)}, {&c.reference, sizeof(c.reference)},
        {&c.construction, sizeof(c.construction)}, {&s, sizeof(s)},
        {&s.pool_01090054, sizeof(s.pool_01090054)},
        {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())},
        {&s.constants, sizeof(s.constants)}, {&s.negative_zero_00d7a208, 4},
        {&n, sizeof(n)}, {&n.strings, sizeof(n.strings)}, {&n.world, sizeof(n.world)},
        {&n.scenes, sizeof(n.scenes)}, {&n.scenes.owners, sizeof(n.scenes.owners)},
        {&n.scenes.increment_00ce221c, sizeof(n.scenes.increment_00ce221c)},
        {&n.scenes.decrement_00ce2220, sizeof(n.scenes.decrement_00ce2220)},
        {&n.scenes.children, sizeof(n.scenes.children)}, {&n.scenes.gates, sizeof(n.scenes.gates)},
        {&n.trees, sizeof(n.trees)}, {&n.trees.point_lights, sizeof(n.trees.point_lights)},
        {&n.trees.frames, sizeof(n.trees.frames)}, {c.model.actual_profile_00d62de8, 0x58}})
        check(r);
}
bool fresh(const NativeModelStorageLifetimeAcquired& a) noexcept {
    return !a.started && !a.complete && !a.scalar_started && !a.scalar_complete &&
        !a.flags_read && !a.pool_returned && !a.node.started && !a.geometry_unwind.started;
}
void default_members(void* actual) {
    require(!read(actual, 0x30) && !read(actual, 0x34) && !read(actual, 0x38) &&
        !read(actual, 0x3c) && !read(actual, 0x40) && !read(actual, 0xa0) &&
        !read(actual, 0xa4) && !read(actual, 0x130) && !read(actual, 0x164) &&
        !read(actual, 0x168) && !read(actual, 0x16c) && !read(actual, 0x170) && !read(actual, 0x174),
        "private Model dispatch excludes hierarchy/scene/retained/point-light state");
}
} // namespace

NativeModelStorageDispatch::NativeModelStorageDispatch(NativeModelStorageDispatchAcquired& a) noexcept
    : acquired_(a) {}
NativeModelStorageDispatch::~NativeModelStorageDispatch() {
    if (acquired_.poisoned || acquired_.current18.phase == Phase::entered ||
        acquired_.current0.phase == Phase::entered) std::terminate();
}
void NativeModelStorageDispatch::bind(NativeModelStorageDispatchContext& c) {
    auto& s = c.model.storage;
    auto& n = s.node;
    require(!context_ && !acquired_.bound && !acquired_.poisoned &&
        acquired_.current18.phase == Phase::fresh && acquired_.current0.phase == Phase::fresh &&
        !acquired_.current04_entered && !acquired_.current04_returned &&
        !c.actual && !c.reference && !c.construction.started && !c.construction.complete &&
        !c.construction.node_constructor_returned && !c.construction.tail_constructed &&
        !c.construction.metadata_bound && !c.construction.actual_model && !c.construction.context &&
        fresh(c.lifetime) && !c.release.started && !c.release.complete,
        "Model dispatch binds once to a wholly prepared pre-pool record");
    require(c.model.actual_profile_00d62de8 && &n.trees.dispatch == this &&
        &n.trees.owners == static_cast<NativeRenderActualOwners*>(&n.scenes.owners) &&
        &n.trees.decrement_00ce2220 == &n.scenes.decrement_00ce2220,
        "Model dispatch requires its SAME raw tree/actual-owner/import domain");
    const Region writes[] = {{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)},
        {&c.lifetime_frame, sizeof(c.lifetime_frame)}, {&c.lifetime, sizeof(c.lifetime)},
        {&c.release_frame, sizeof(c.release_frame)}, {&c.release, sizeof(c.release)}};
    for (std::size_t i = 0; i != 6; ++i) {
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Model dispatch writable records must be separate");
        borrowed(c, [&](Region r) {
            require(disjoint(writes[i], r), "Model dispatch writes cannot alias known providers");
        });
    }
    context_ = &c;
    profile_ = c.model.actual_profile_00d62de8;
    acquired_.bound = true; // metadata only; no native/table/import value read
}
NativeModelStorageDispatchContext& NativeModelStorageDispatch::context() const {
    require(context_ && acquired_.bound && !acquired_.poisoned,
        "Model dispatch is unbound or has retained entered failure");
    require(context_->model.actual_profile_00d62de8 == profile_,
        "Model dispatch cannot replace its borrowed current table view");
    return *context_;
}
const volatile Word* NativeModelStorageDispatch::resolve_profile(Word captured) const {
    (void)context();
    require(captured == model_profile, "private Model dispatch rejects another captured family");
    return profile_; // actual CURRENT contents; no copied table or target read
}
NativeModelStorageReference& NativeModelStorageDispatch::require_live(void* actual, bool zero) {
    auto& c = context();
    auto& n = c.model.storage.node;
    require(actual && !(bits(actual) & 3u) && c.actual == actual && c.reference &&
        !c.reference->retired() && c.construction.complete && c.construction.metadata_bound &&
        c.construction.node_constructor_returned && c.construction.tail_constructed &&
        c.construction.actual_model == actual && c.construction.context == &c.model.storage &&
        fresh(c.lifetime) && &n.trees.dispatch == this &&
        &n.trees.owners == static_cast<NativeRenderActualOwners*>(&n.scenes.owners) &&
        &n.trees.decrement_00ce2220 == &n.scenes.decrement_00ce2220 &&
        n.scenes.owners.find(actual) == &*c.reference &&
        static_cast<void*>(&c.reference->reference_count) == reinterpret_cast<void*>(bits(actual) + 4u),
        "Model dispatch needs its SAME completed canonical481 identity and retained record");
    const Region payload{actual, 0x188};
    for (const Region r : {Region{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)},
        {&c.lifetime_frame, sizeof(c.lifetime_frame)}, {&c.lifetime, sizeof(c.lifetime)},
        {&c.release_frame, sizeof(c.release_frame)}, {&c.release, sizeof(c.release)}})
        require(disjoint(payload, r), "Model dispatch payload cannot overlap its persistent records");
    borrowed(c, [&](Region r) {
        require(disjoint(payload, r), "Model dispatch payload cannot overlap known providers");
    });
    const auto count = c.reference->reference_count.load(std::memory_order_relaxed);
    require((zero ? count == 0 : count > 0) && read(actual) == model_profile,
        "Model dispatch requires its actual current Model family and reached count domain");
    default_members(actual); // Source domain admission, no cleanup/preclear
    return *c.reference;
}
void NativeModelStorageDispatch::require_completed_scalar() const {
    const auto& c = context();
    require(c.reference && c.reference->retired() && c.lifetime.complete &&
        c.lifetime.scalar_complete && c.lifetime.flags_read &&
        c.lifetime.pool_returned == ((c.lifetime.flags_low_byte & 1u) != 0),
        "Model metadata unbind is not completed cleanup/late-flags/physical-return proof");
}
void NativeModelStorageDispatch::invoke_virtual18(Word target, void* actual) {
    auto& c = context();
    auto& a = acquired_.current18;
    require(a.phase == Phase::fresh && acquired_.current0.phase == Phase::fresh,
        "Model current18 rejects recursive reuse before another native body call");
    a.phase = Phase::entered;
    a.captured_target = target;
    try {
        require(target == release_target && !c.release.started && !c.release.complete,
            "Model current18 requires fresh genuine B6F310 records and its captured target");
        auto& reference = require_live(actual, false);
        require(*reinterpret_cast<const volatile std::uint8_t*>(bits(actual) + 0x44u) == 0,
            "Model current18 needs its first real tree-release credit");
        a.admitted_profile = model_profile;
        a.body_entered = true;
        release_native_node_tree_00b6f310(actual, c.release_frame, c.model.storage.node.trees, c.release);
        a.body_returned = true;
        require(c.release.complete, "Model B6F310 returned without complete retained evidence");
        if (reference.retired()) {
            require(acquired_.current0.phase == Phase::complete,
                "retired Model current18 requires this completed genuine zero dispatch");
            require_completed_scalar();
        } else require(fresh(c.lifetime) && acquired_.current0.phase == Phase::fresh,
            "live Model credits cannot hide entered scalar cleanup");
        a.phase = Phase::complete; // metadata ONLY: actual may have been returned
    } catch (...) {
        a.phase = Phase::failed;
        acquired_.poisoned = true;
        throw;
    }
}
void NativeModelStorageDispatch::invoke_virtual0(Word target, void* actual, RenderCommandReference& owner) {
    auto& a = acquired_.current0;
    // This is an observed-zero Native terminal boundary. Even admission failure
    // can follow the outer decrement; honor481's noexcept retention policy.
    try {
        (void)context();
        require(a.phase == Phase::fresh, "Model zero rejects recursive/replayed terminal dispatch");
        a.phase = Phase::entered;
        a.captured_target = target;
        require(target == zero_target, "Model current0 captured target is not genuine BD30E0");
        auto& reference = require_live(actual, true);
        require(&owner == &reference, "Model zero requires its SAME canonical actual+4 companion");
        a.admitted_profile = model_profile;
        a.body_entered = true;
        invoke_native_ref_counted_delete_00bd30e0(actual, *this); // FRESH profile/current04
        a.body_returned = true;
        require(acquired_.current04_returned, "Model BD30E0 must return from its real current04 body");
        require_completed_scalar();
        a.phase = Phase::complete; // no actual payload/profile/count read
    } catch (...) {
        a.phase = Phase::failed;
        acquired_.poisoned = true;
        std::terminate();
    }
}
void NativeModelStorageDispatch::delete_vslot04(void* actual, Word captured_profile, Word flags) {
    auto& c = context();
    require(acquired_.current0.phase == Phase::entered && acquired_.current0.body_entered &&
        !acquired_.current04_entered, "Model current04 requires this entered BD30E0 invocation");
    acquired_.current04_entered = true;
    acquired_.captured_current04_profile = captured_profile;
    require(captured_profile == model_profile && flags == 1,
        "Model BD30E0 requires its captured actual profile and literal flags1");
    const Word target = profile_[1]; // one CURRENT04 read, AFTER genuine BD30E0 profile capture
    acquired_.captured_current04_target = target;
    require(target == scalar_target, "Model current04 rejects unsupported captured-profile target");
    auto& reference = require_live(actual, true);
    c.lifetime_frame.scalar_flags_argument = flags; // persistent Original argument cell, NOT a late snapshot
    reference.delete_scalar_00b75290(c.lifetime_frame.scalar_flags_argument);
    acquired_.current04_returned = true; // SAME canonical terminal; payload may be dead
}
} // namespace bsp
