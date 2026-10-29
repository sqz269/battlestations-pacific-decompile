#include "bsp/native_model_storage_node_dispatch.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/native_node_destruction.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Private raw Model node dispatch requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Call = NativeModelStorageNodeCall;
using Phase = Call::Phase;
using Kind = Call::Kind;
constexpr Word model_profile = 0x00d62de8, node_profile = 0x00d62c88;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
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
void borrowed(NativeModelStorageNodeDispatchContext& c, Check check) {
    auto& m = c.record;
    auto& n = m.model.storage.node;
    auto& s = c.shared_node;
    for (const Region r : {Region{&c, sizeof(c)}, {&m, sizeof(m)}, {&m.model, sizeof(m.model)},
        {&m.actual, sizeof(m.actual)}, {&m.reference, sizeof(m.reference)},
        {&m.construction, sizeof(m.construction)}, {&m.lifetime_frame, sizeof(m.lifetime_frame)},
        {&m.lifetime, sizeof(m.lifetime)}, {&m.release_frame, sizeof(m.release_frame)},
        {&m.release, sizeof(m.release)}, {&c.tree_dispatch, sizeof(c.tree_dispatch)},
        {&m.model.storage, sizeof(m.model.storage)}, {&n, sizeof(n)},
        {&n.scenes, sizeof(n.scenes)}, {&n.scenes.gates, sizeof(n.scenes.gates)},
        {&n.trees, sizeof(n.trees)}, {&c.parenting, sizeof(c.parenting)},
        {&c.base_nodes, sizeof(c.base_nodes)}, {&c.base_nodes.point_lights, sizeof(c.base_nodes.point_lights)},
        {&s, sizeof(s)}, {&s.scenes, sizeof(s.scenes)}, {&s.scenes.gates, sizeof(s.scenes.gates)},
        {&s.scenes.gates.registry, sizeof(s.scenes.gates.registry)}, {&s.scenes.owners, sizeof(s.scenes.owners)},
        {&s.scenes.increment_00ce221c, sizeof(s.scenes.increment_00ce221c)},
        {&s.scenes.decrement_00ce2220, sizeof(s.scenes.decrement_00ce2220)},
        {&s.trees, sizeof(s.trees)}, {&s.trees.frames, sizeof(s.trees.frames)},
        {&s.strings, sizeof(s.strings)}, {&s.world, sizeof(s.world)}}) check(r);
}
bool same_types(const LightTypeBootstrapStorage& a, const LightTypeBootstrapStorage& b) noexcept {
    return &a.root_guard_0109db80 == &b.root_guard_0109db80 && &a.root_0109db84 == &b.root_0109db84 &&
        &a.node_guard_0108ff54 == &b.node_guard_0108ff54 && &a.node_0108ff90 == &b.node_0108ff90 &&
        &a.light_guard_0109010d == &b.light_guard_0109010d && &a.light_0109018c == &b.light_0109018c &&
        &a.directional_guard_0109010e == &b.directional_guard_0109010e &&
        &a.directional_0109019c == &b.directional_0109019c;
}
} // namespace

NativeModelStorageNodeDispatch::NativeModelStorageNodeDispatch(game::GameNativeReadOnlyData& data,
    game::GameNativeTypeStorage& types, TypeIdCounterLifetime& counter, LightTypeBootstrap& nodes,
    Call* calls, std::size_t count, NativeModelStorageNodeDispatchAcquired& a) noexcept
    : data_(data), types_(types), counter_(counter), node_types_(nodes), model_types_(counter, nodes, types.model_types()),
      calls_(calls), call_count_(count), acquired_(a) {}
NativeModelStorageNodeDispatch::~NativeModelStorageNodeDispatch() {
    if (acquired_.poisoned) std::terminate();
    if (!context_) return; // rejected pure binding admitted no calls/backing
    if (acquired_.used > call_count_) std::terminate();
    for (std::size_t i = 0; i < acquired_.used; ++i)
        if (calls_[i].phase == Phase::entered) std::terminate();
}
bool NativeModelStorageNodeDispatch::matches(const NativeModelStorageNodeDispatchContext& c,
    const NativeModelStorageNodeDispatchAcquired& a,
    const NativeModelStorageNodeCall* calls, std::size_t count) const noexcept {
    return context_ == &c && &acquired_ == &a && calls_ == calls && call_count_ == count;
}
void NativeModelStorageNodeDispatch::bind(NativeModelStorageNodeDispatchContext& c) {
    auto& m = c.record;
    auto& n = m.model.storage.node;
    auto& shared = c.shared_node;
    auto& g = n.scenes.gates;
    auto& sg = shared.scenes.gates;
    require(!context_ && !acquired_.bound && !acquired_.poisoned && !acquired_.used &&
        !acquired_.entry_failed && !acquired_.capacity_failed && !m.actual && !m.reference &&
        !m.construction.started && !m.construction.complete && !m.construction.metadata_bound &&
        !m.construction.node_constructor_returned && !m.construction.tail_constructed &&
        !m.construction.actual_model && !m.construction.context &&
        !m.lifetime.started && !m.lifetime.complete && !m.lifetime.scalar_started && !m.lifetime.scalar_complete &&
        !m.lifetime.node.started && !m.lifetime.geometry_unwind.started && !m.release.started && !m.release.complete,
        "Model node dispatch binds once to the SAME fresh pre-pool record");
    const auto bytes = static_cast<std::uint64_t>(call_count_) * sizeof(Call);
    require(calls_ && bits(calls_) % alignof(Call) == 0 && call_count_ && bytes <= 0xffffffffu &&
        static_cast<std::uint64_t>(bits(calls_)) + bytes <= 0x100000000ull,
        "Model node dispatch requires initialized representable prepared call backing");
    require(&c.parenting.node == &n && &c.parenting.dispatch == static_cast<NativeNodeParentingDispatch*>(this) &&
        &n.scenes.children == static_cast<NativeNodeSceneChildDispatch*>(this) &&
        g.derived == static_cast<NativeSceneRegistrationDerivedTypes*>(this) &&
        &n.trees.dispatch == static_cast<NativeNodeTreeRetirementDispatch*>(&c.tree_dispatch) &&
        &n.trees.frames == &shared.trees.frames && &n.trees.point_lights == &c.base_nodes.point_lights &&
        &n.scenes.owners == &shared.scenes.owners &&
        &n.trees.owners == static_cast<NativeRenderActualOwners*>(&shared.scenes.owners) &&
        &n.scenes.increment_00ce221c == &shared.scenes.increment_00ce221c &&
        &n.scenes.decrement_00ce2220 == &shared.scenes.decrement_00ce2220 &&
        &n.trees.decrement_00ce2220 == &shared.scenes.decrement_00ce2220 &&
        &n.strings == &shared.strings && c.base_nodes.uses_raw_name_pool() &&
        &c.base_nodes.require_raw_name_pool() == &shared.strings && &n.world == &shared.world &&
        &g.registry == &sg.registry && &g.light_types == &node_types_ && &sg.light_types == &node_types_ &&
        g.camera_types == sg.camera_types && g.actual_node_profile_00d62c88 == sg.actual_node_profile_00d62c88 &&
        g.actual_camera_profile_00d62cf0 == sg.actual_camera_profile_00d62cf0 &&
        g.actual_light_profile_00d62f58 == sg.actual_light_profile_00d62f58 &&
        g.actual_directional_profile_00d62fb0 == sg.actual_directional_profile_00d62fb0 &&
        &c.parenting.group_type_0109032c == &types_.group_types().group_0109032c.own_id &&
        same_types(node_types_.storage(), types_.light_types()),
        "Model node dispatch requires SAME private Model/node/scene/gate/point-light/string/type/import views");
    const auto* mp = static_cast<const volatile Word*>(data_.data_at(model_profile, 0x58));
    const auto* np = static_cast<const volatile Word*>(data_.data_at(node_profile, 0x58));
    require(m.model.actual_profile_00d62de8 == mp && sg.actual_node_profile_00d62c88 == np,
        "Model node dispatch requires its genuine borrowed Original-VA profile aliases");
    const Region writes[] = {{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)},
        {calls_, static_cast<std::size_t>(bytes)}};
    for (std::size_t i = 0; i != 3; ++i) {
        for (std::size_t j = i + 1; j != 3; ++j)
            require(disjoint(writes[i], writes[j]), "Model node dispatch writable metadata must be separate");
        borrowed(c, [&](Region r) {
            require(disjoint(writes[i], r), "Model node dispatch writes cannot alias known providers");
        });
        for (const Region r : {Region{&data_, sizeof(data_)}, {&types_, sizeof(types_)}, {&counter_, sizeof(counter_)},
            {&node_types_, sizeof(node_types_)}, {mp, 0x58}, {np, 0x58}})
            require(disjoint(writes[i], r), "Model node dispatch writes cannot alias data/type owners");
    }
    for (std::size_t i = 0; i != call_count_; ++i)
        require(calls_[i].phase == Phase::fresh && calls_[i].kind == Kind::none &&
            !calls_[i].body_entered && !calls_[i].body_returned &&
            !calls_[i].attachment.started && !calls_[i].attachment.complete &&
            !calls_[i].scene.started && !calls_[i].scene.complete,
            "Model node dispatch call slots require fresh initialized records");
    context_ = &c;
    model_profile_ = mp;
    node_profile_ = np;
    acquired_.bound = true; // no type getter/init/import value/native body call
}
NativeModelStorageNodeDispatchContext& NativeModelStorageNodeDispatch::context() const {
    require(context_ && acquired_.bound && !acquired_.poisoned,
        "Model node dispatch is unbound or has retained entered failure");
    return *context_;
}
const volatile Word* NativeModelStorageNodeDispatch::resolve_profile(Word captured) const {
    (void)context();
    if (captured == model_profile) return model_profile_;
    if (captured == node_profile) return node_profile_;
    throw std::logic_error("private Model node dispatch rejects another captured profile");
}
Call& NativeModelStorageNodeDispatch::next(Kind kind, Word target, void* actual) {
    (void)context();
    if (acquired_.used >= call_count_) {
        acquired_.entry_failed = acquired_.capacity_failed = acquired_.poisoned = true;
        acquired_.failed_kind = kind;
        acquired_.failed_target = target;
        acquired_.failed_receiver = actual;
        throw std::logic_error("Model node dispatch exhausted retained call backing after caller effects");
    }
    auto& a = calls_[acquired_.used];
    if (a.phase != Phase::fresh || a.kind != Kind::none) {
        acquired_.entry_failed = acquired_.poisoned = true;
        acquired_.failed_kind = kind;
        acquired_.failed_target = target;
        acquired_.failed_receiver = actual;
        throw std::logic_error("Model node dispatch cannot reuse an entered/completed/failed record");
    }
    ++acquired_.used;
    a.phase = Phase::entered;
    a.kind = kind;
    a.target = target;
    a.receiver = actual;
    return a; // active BEFORE any admission read or reached real type/body call
}
Word NativeModelStorageNodeDispatch::require_active_model(void* actual) const {
    auto& c = context();
    auto& m = c.record;
    auto& n = m.model.storage.node;
    require(actual && bits(actual) % 4 == 0 && m.actual == actual && m.reference &&
        !m.reference->retired() && m.construction.complete && m.construction.metadata_bound &&
        m.construction.actual_model == actual && m.construction.context == &m.model.storage &&
        !m.lifetime.complete && !m.lifetime.scalar_complete && !m.lifetime.flags_read && !m.lifetime.pool_returned &&
        !m.lifetime.node.complete && !m.lifetime.node.base_cleanup_completed &&
        n.scenes.owners.find(actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == reinterpret_cast<void*>(bits(actual) + 4u),
        "Model node dispatch needs its SAME canonical still-live Model/base-prefix record");
    const Region payload{actual, 0x188};
    for (const Region r : {Region{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)},
        {calls_, call_count_ * sizeof(Call)}})
        require(disjoint(payload, r), "Model payload cannot overlap node dispatch records");
    borrowed(c, [&](Region r) {
        require(disjoint(payload, r), "Model payload cannot overlap known provider backing");
    });
    for (const Region r : {Region{&data_, sizeof(data_)}, {&types_, sizeof(types_)}, {&counter_, sizeof(counter_)},
        {&node_types_, sizeof(node_types_)}, {model_profile_, 0x58}, {node_profile_, 0x58}})
        require(disjoint(payload, r), "Model payload cannot overlap data/type owners");
    const Word profile = *static_cast<const volatile Word*>(actual); // only AFTER lifetime/identity admission
    require(profile == model_profile || (profile == node_profile && m.lifetime.started &&
        m.lifetime.scalar_started && m.lifetime.node.started && !m.lifetime.node.complete),
        "Model node dispatch requires current Model or genuine active B6F440 base prefix");
    return profile;
}
void NativeModelStorageNodeDispatch::fail(Call& a) noexcept {
    a.phase = Phase::failed;
    acquired_.entry_failed = true;
    acquired_.failed_kind = a.kind;
    acquired_.failed_target = a.target;
    acquired_.failed_receiver = a.receiver;
    acquired_.poisoned = true;
}
std::uint8_t NativeModelStorageNodeDispatch::invoke_type_0c(Word target, void* actual, Word token) {
    auto& a = next(Kind::type_0c, target, actual);
    a.first_argument = token;
    try {
        a.admitted_profile = require_active_model(actual);
        require(target == 0x006ef860 || target == 0x00b6f570,
            "Model type dispatch requires captured genuine Model/base-node predicate target");
        a.body_entered = true;
        // Dispatch the CAPTURED target, not a second current-profile/slot read.
        a.result_al = static_cast<std::uint8_t>(target == 0x006ef860
            ? model_types_.is_type_006ef860(token) : node_types_.node_is_type_00b6f570(token));
        a.body_returned = true;
        a.phase = Phase::complete;
        return a.result_al;
    } catch (...) { fail(a); throw; }
}
void NativeModelStorageNodeDispatch::invoke_attachment_1c(Word target, void* actual, Word attachment) {
    auto& a = next(Kind::attachment_1c, target, actual);
    a.first_argument = attachment;
    try {
        a.admitted_profile = require_active_model(actual);
        require(target == 0x00b6d7b0, "Model attachment dispatch requires captured genuine B6D7B0");
        a.attachment_frame.attachment_argument = attachment; // actual persistent incoming argument cell
        a.body_entered = true;
        set_native_node_attachment_00b6d7b0(actual, a.attachment_frame, *this, a.attachment);
        a.body_returned = true;
        require(a.attachment.complete, "Model attachment returned without complete retained evidence");
        a.phase = Phase::complete; // metadata only; no post-callback actual access
    } catch (...) { fail(a); throw; }
}
void NativeModelStorageNodeDispatch::invoke_child(Word target, void* actual, Word scene, Word recursion) {
    const auto kind = target == 0x00b6ee10 ? Kind::scene_54 :
        target == 0x00b6ed80 ? Kind::scene_50 : Kind::scene_other;
    auto& a = next(kind, target, actual);
    a.first_argument = scene;
    a.second_argument = recursion;
    try {
        a.admitted_profile = require_active_model(actual);
        require(target == 0x00b6ed80 || target == 0x00b6ee10,
            "Model scene-child dispatch requires captured genuine B6ED80/B6EE10");
        a.scene_frame.scene_argument = scene;
        a.scene_frame.recursion_argument = recursion; // real DWORD; callee reads LOW BYTE at native site
        a.body_entered = true;
        auto& scenes = context_->record.model.storage.node.scenes;
        if (target == 0x00b6ed80) set_native_node_scene_storage_00b6ed80(actual, a.scene_frame, scenes, a.scene);
        else remove_native_node_scene_storage_00b6ee10(actual, a.scene_frame, scenes, a.scene);
        a.body_returned = true;
        require(a.scene.complete, "Model scene call returned without complete retained evidence");
        a.phase = Phase::complete; // no post-return actual/profile/count read
    } catch (...) { fail(a); throw; }
}
} // namespace bsp
