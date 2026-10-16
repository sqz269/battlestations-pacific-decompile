#include "bsp/native_gui_frame_box_constructed_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual FrameBox constructed storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiFrameBoxConstructedStorageContext;
using Frame = NativeGuiFrameBoxConstructedStorageFrame;
using Acquired = NativeGuiFrameBoxConstructedStorageAcquired;
using Boundary = Acquired::Boundary;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(std::is_trivially_destructible_v<Acquired>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(void* p, Word offset) noexcept { return reinterpret_cast<void*>(bits(p) + offset); }
Word word(void* p, Word offset = 0) noexcept { return *static_cast<const volatile Word*>(at(p, offset)); }
void require(bool condition, const char* text) {
    if (!condition) throw std::invalid_argument(text);
}
struct Region { const volatile void* begin; std::size_t size; };
bool valid(Region r) noexcept {
    return r.begin && r.size && static_cast<std::uint64_t>(bits(r.begin)) + r.size <= 0x100000000ull;
}
bool disjoint(Region a, Region b) noexcept {
    const auto x = static_cast<std::uint64_t>(bits(a.begin));
    const auto y = static_cast<std::uint64_t>(bits(b.begin));
    return x + a.size <= y || y + b.size <= x;
}
struct Views {
    const volatile Word* mesh;
    const volatile double* half;
};
Views preflight(void* widget, Frame& f, Context& c, Acquired& a) {
    const auto& m = a.mesh;
    const auto& r = a.mesh_release;
    require(!a.started && !a.complete && !a.failed && !a.gate_captured && !a.geometry_enabled &&
        !a.factory_entered && !a.factory_returned && !a.geometry_entered && !a.geometry_returned &&
        !a.position_entered && !a.position_returned && !a.delegated_constructor_action_consumed &&
        !a.raw_mesh_return_entered && !a.raw_mesh_returned && a.native_eh_state == -1 &&
        !a.active_call_site && a.boundary == Boundary::none && !a.destination && !a.captured_model &&
        !a.captured_model_owner && !a.mesh_creator && !m.started && !m.raw_slot && !m.creator &&
        !m.companion && !m.owner_record && !m.native_site && !m.constructor_complete &&
        !m.published && !m.registered && !r.identity && !r.captured_decrement && !r.result &&
        !r.captured_profile && !r.captured_target && !r.entered && !r.decremented &&
        !r.zero_entered && !r.zero_returned && !r.complete,
        "Raw FrameBox constructed requires fresh one-shot diagnostics");
    require(widget && (bits(widget) & 3u) == 0 && (bits(&f.locals[0]) & 3u) == 0,
        "Raw FrameBox constructed requires aligned actual slot and local backing");
    const Region writes[] = {{widget, 0x120}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.locals[0], sizeof(f.locals)}, {&f.transform, sizeof(f.transform)}, {&f.bounds, sizeof(f.bounds)}};
    for (std::size_t i = 0; i != 6; ++i) {
        require(valid(writes[i]), "Raw FrameBox writable extent must be representable");
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Raw FrameBox writable frames/backing must be separate");
    }
    const auto* one = c.data.data_at(0x00d7a24c, 4);
    const auto* negative_zero = c.data.data_at(0x00d7a208, 4);
    const auto* sentinel = c.data.data_at(0x00d7a260, 4);
    const auto* minimum = c.data.data_at(0x00ce4adc, 4);
    const auto* maximum = c.data.data_at(0x00ce4970, 4);
    Views views{static_cast<const volatile Word*>(c.data.data_at(0x00d62d60, 8)),
        static_cast<const volatile double*>(c.data.data_at(0x00d7a280, 8))};
    require(&c.geometry.actual_owners() == &c.owners && &c.models.retained_owners == &c.owners &&
        !c.models.actual_names && c.models.nodes.uses_raw_name_pool() &&
        c.models.vtable_00d62de8 == c.data.data_at(0x00d62de8, 23 * 4) &&
        c.models.vtable_00d62c88 == c.data.data_at(0x00d62c88, 22 * 4) &&
        &c.position.dispatch == &c.dispatch && bits(&c.position.one_00d7a24c) == bits(one) &&
        bits(&c.position.negative_zero_00d7a208) == bits(negative_zero) &&
        bits(&c.models.constants.negative_zero_00d7a208) == bits(negative_zero) &&
        bits(&c.models.constants.unchanged_00d7a260) == bits(sentinel) &&
        bits(&c.models.constants.minimum_00ce4adc) == bits(minimum) &&
        bits(&c.models.constants.maximum_00ce4970) == bits(maximum),
        "Raw FrameBox requires SAME actual registry/raw Model and held scalar/profile aliases");
    const auto mesh_constants = c.geometry.mesh_constants();
    require(bits(&mesh_constants.minimum_00ce4adc) == bits(minimum) &&
        bits(&mesh_constants.maximum_00ce4970) == bits(maximum) &&
        bits(&mesh_constants.lod_00d7a24c) == bits(one),
        "Raw FrameBox mesh constructor must borrow SAME current constants");
    for (const Region borrowed : {Region{&c, sizeof(c)}, {&c.models, sizeof(c.models)},
        {&c.models.nodes, sizeof(c.models.nodes)}, {&c.geometry, sizeof(c.geometry)},
        {&c.owners, sizeof(c.owners)}, {&c.data, sizeof(c.data)}, {&c.position, sizeof(c.position)},
        {&c.dispatch, sizeof(c.dispatch)}, {&c.dispatch.profiles(), sizeof(c.dispatch.profiles())},
        {&c.decrement_00ce2220, sizeof(c.decrement_00ce2220)},
        {&c.position.y_scale_00e12fc4, sizeof(c.position.y_scale_00e12fc4)},
        {one, 4}, {negative_zero, 4}, {sentinel, 4}, {minimum, 4}, {maximum, 4},
        {views.mesh, 8}, {views.half, 8}, {c.models.vtable_00d62de8, 23 * 4},
        {c.models.vtable_00d62c88, 22 * 4}}) {
        require(valid(borrowed), "Raw FrameBox known borrowed extent must be representable");
        for (const auto writable : writes)
            require(disjoint(writable, borrowed), "Raw FrameBox caller writes cannot alias retained providers");
    }
    return views; // No widget74/4C/profile/count read belongs in this preflight.
}
void reached(Acquired& a, Word site, Boundary boundary = Boundary::provider_call) noexcept {
    a.active_call_site = site;
    a.boundary = boundary; // Source provider boundary, never Native-origin proof.
}
NativeModelOwner& captured_model(Context& c, Acquired& a) {
    require(a.captured_model, "Native null FrameBox Model is outside supported actual storage");
    auto* reference = dynamic_cast<NativeModelReference*>(&c.owners.resolve_actual(a.captured_model));
    require(reference && &reference->model_owner().storage.node == a.captured_model &&
        &reference->model_owner().environment == &c.models &&
        reference->model_owner().phase == NativeModelOwner::Phase::live,
        "Captured FrameBox Model requires SAME independently retained canonical owner");
    a.captured_model_owner = &reference->model_owner();
    return *a.captured_model_owner;
}
__declspec(noinline) void geometry_arguments(Frame& f, Context& c, volatile std::int32_t& state) noexcept {
    const volatile void* constant = &c.models.constants.unchanged_00d7a260;
    float* first = &f.scalar_178;
    float* second = &f.scalar_17c;
    volatile std::int32_t* state_slot = &state;
    __asm {
        mov eax, constant
        fld dword ptr [eax]
        mov eax, second
        fst dword ptr [eax]
        mov eax, first
        fstp dword ptr [eax]
        mov eax, state_slot
        mov dword ptr [eax], -1
    }
}
__declspec(noinline) void zero_position(Frame& f) noexcept {
    volatile Word* xyz = &f.locals[1];
    __asm {
        xorps xmm0, xmm0
        mov eax, xyz
        movss dword ptr [eax], xmm0
        movss dword ptr [eax+4], xmm0
        movss dword ptr [eax+8], xmm0
    }
}
void release_mesh(Context& c, Acquired& a, const Views& views) {
    // The reached CURRENT cell read is not an entry/preflight snapshot.
    const auto decrement = c.decrement_00ce2220;
    auto& r = a.mesh_release;
    require(a.mesh_creator && decrement && !r.entered,
        "Raw FrameBox mesh release requires unconsumed nonnull creator/current import");
    void* raw = std::exchange(a.mesh_creator, nullptr); // consume BEFORE actual call
    r.identity = raw;
    r.captured_decrement = decrement;
    r.entered = true;
    r.result = decrement(static_cast<volatile long*>(at(raw, 4)));
    r.decremented = true;
    if (r.result == 0) {
        reached(a, 0x00acf96d, Boundary::host_metadata);
        r.captured_profile = word(raw); // CURRENT, only after observed zero.
        require(r.captured_profile == bits(views.mesh), "Raw FrameBox mesh zero requires current genuine D62D60");
        r.captured_target = views.mesh[0]; // CURRENT slot00 of actual held alias.
        require(r.captured_target == 0x00bd30e0u, "Raw FrameBox mesh zero requires real current slot00 terminal");
        reached(a, 0x00acf973);
        r.zero_entered = true;
        dispatch_native_render_actual_owner_zero(c.owners, raw); // no second decrement
        r.zero_returned = true; // independent metadata only after possible retirement
    }
    r.complete = true;
}
} // namespace

NativeGuiFrameBoxConstructedTransformDispatch::NativeGuiFrameBoxConstructedTransformDispatch(
    const NativeNodeMappedRawTransformProfiles& profiles) noexcept : profiles_(profiles) {}
void NativeGuiFrameBoxConstructedTransformDispatch::call_virtual38(Word captured_target,
    void* current_receiver, const void* matrix) {
    require(captured_target == 0x00b6db10u,
        "AA7220 captured virtual38 requires genuine B6DB10 mapped body");
    set_raw_local_matrix_00b6db10(current_receiver, profiles_, matrix);
}

void constructed_native_gui_frame_box_storage_00acf8f0(void* widget, Frame& f, Context& c, Acquired& a) {
    const Views views = preflight(widget, f, c, a);
    a.started = true;
    a.destination = widget;
    try {
        a.geometry_enabled = *(static_cast<const volatile std::uint8_t*>(widget) + 0x74) == 0;
        a.gate_captured = true;
        if (a.geometry_enabled) {
            a.captured_model = reinterpret_cast<void*>(word(widget, 0x4c)); // BEFORE allocation.
            reached(a, 0x00acf91b);
            a.factory_entered = true;
            try { a.mesh_creator = c.geometry.create_native_mesh_00b73b60(a.mesh); }
            catch (...) {
                // This established combined provider owns state0/B72F70. Its
                // marker is a contract boundary, not Native exception proof.
                if (a.mesh.native_site == 0x00b4c761 && !a.mesh.constructor_complete) {
                    a.delegated_constructor_action_consumed = true;
                    a.raw_mesh_return_entered = true;
                    a.native_eh_state = -1;
                    if (a.mesh.raw_slot) std::terminate(); // nested raw return escape
                    a.raw_mesh_returned = true;
                } else if (a.mesh.native_site == 0 || a.mesh.constructor_complete) {
                    a.boundary = Boundary::host_metadata;
                }
                throw;
            }
            a.factory_returned = true;
            f.locals[0] = reinterpret_cast<Word>(a.mesh_creator);
            a.native_eh_state = 0; // factory's ctor state was delegated, not repeated
            geometry_arguments(f, c, a.native_eh_state); // FLD/FST/FSTP then state-1
            reached(a, 0x00acf95a, Boundary::host_metadata);
            auto& model = captured_model(c, a); // retained captured identity, no4C reload
            reached(a, 0x00acf95a);
            a.geometry_entered = true;
            set_native_model_geometry_00b75170(model, 0, a.mesh_creator, f.scalar_178, f.scalar_17c);
            a.geometry_returned = true;
            // Null still reached the real setter; its later Native null access
            // is an explicit Source boundary, never early successful return.
            reached(a, 0x00acf963);
            release_mesh(c, a, views);
        }
        zero_position(f); // EVEN when current74 skipped geometry.
        reached(a, 0x00acf993);
        a.position_entered = true;
        set_native_gui_widget_local_position_00aa7dc0(widget, const_cast<const Word*>(&f.locals[1]),
            f.transform, c.position, f.bounds, *views.half);
        a.position_returned = true;
        a.complete = true;
    } catch (...) {
        a.failed = true; // retain creators/publications/frames; NO setter rollback
        throw;
    }
}
} // namespace bsp
