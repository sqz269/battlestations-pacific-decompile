#include "bsp/native_gui_text_default_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/native_wide_string.hpp"
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text default construction requires MSVC Win32.
#endif

namespace bsp {
// TU-local use of private retained negative metadata; no public setter/issuer.
struct NativeGuiTextRawModelDefaultFailureAccess final {
    static const NativeGuiTextFailureOriginRuntime* runtime(NativeGuiTextRawModelDefaultStorageAcquired& a) noexcept { return a.failure_runtime_; }
    static NativeGuiTextFailureOrigin& origin(NativeGuiTextRawModelDefaultStorageAcquired& a) noexcept { return a.failure_origin_; }
};
namespace {
struct FailureTransport {
    const NativeGuiTextFailureOriginRuntime* runtime{};
    NativeGuiTextFailureOrigin* origin{};
    const void* supplier{};
};
FailureTransport failure_transport(NativeGuiTextDefaultStorageAcquired&) noexcept { return {}; }
FailureTransport failure_transport(NativeGuiTextRawModelDefaultStorageAcquired& a) noexcept {
    return {NativeGuiTextRawModelDefaultFailureAccess::runtime(a), &NativeGuiTextRawModelDefaultFailureAccess::origin(a), &a};
}
template<class Acquired, class F> decltype(auto) source_host(Acquired& a, std::uint32_t source, F&& f) {
    const auto t = failure_transport(a);
    if (t.runtime) return t.origin->host(*t.runtime, {0, source}, t.supplier, std::forward<F>(f));
    return std::forward<F>(f)();
}
template<class Acquired, class F> decltype(auto) source_unknown(Acquired& a, std::uint32_t site,
    std::uint32_t source, F&& f) {
    const auto t = failure_transport(a);
    if (t.runtime) return t.origin->unknown(*t.runtime, {site, source}, t.supplier, std::forward<F>(f));
    return std::forward<F>(f)();
}
using Word = std::uint32_t;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* storage, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(storage) + offset;
}
volatile Word& word(void* storage, std::size_t offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(storage, offset));
}
volatile unsigned char& byte(void* storage, std::size_t offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(storage, offset));
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region l, Region r) noexcept {
    const auto left = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(l.begin));
    const auto right = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(r.begin));
    return left + l.size <= right || right + r.size <= left;
}
// Read the existing FLOAT object's physical DWORD without a floating-point
// lvalue conversion or wrong-type C++ alias. MOVSS in Original also transports
// all signaling-NaN bits unchanged; never perform FLD/FSTP on these defaults.
__declspec(noinline) Word shadow_word(const volatile float& channel) noexcept {
    const volatile void* source = &channel;
    Word result;
    __asm {
        mov eax, source
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
void preflight(void* raw, NativeGuiTextDefaultStorageFrame& f,
    NativeGuiTextDefaultStorageContext& c, NativeGuiTextDefaultStorageAcquired& a) {
    auto& s = c.sections;
    auto& actual = s.actual;
    auto& owners = actual.geometry.actual_owners();
    const auto mesh = actual.geometry.mesh_constants();
    require(raw && (reinterpret_cast<std::uintptr_t>(raw) & 3u) == 0 &&
        !a.started && !a.complete && a.native_eh_state == -1 && !a.sections.started,
        "Text default requires fresh aligned storage and one fresh complete retained graph");
    const Region writes[] = {{raw, 0x1f4}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.sections, sizeof(f.sections)}, {&f.sections.locals[0], sizeof(f.sections.locals)},
        {&f.sections.parenting, sizeof(f.sections.parenting)}};
    for (std::size_t i = 0; i != 6; ++i) {
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Text default caller regions must be disjoint");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&s, sizeof(s)},
            {&c.lifetime, sizeof(c.lifetime)}, {&c.state, sizeof(c.state)},
            {&actual, sizeof(actual)}, {&s.node_constants, sizeof(s.node_constants)},
            {&s.strings, sizeof(s.strings)}, {&s.string_process, sizeof(s.string_process)},
            {&s.effects, sizeof(s.effects)}, {&s.parenting, sizeof(s.parenting)},
            {&s.decrement_00ce2220, sizeof(s.decrement_00ce2220)},
            {&actual.unchanged_00d7a260, sizeof(actual.unchanged_00d7a260)},
            {&s.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)},
            {&c.shadow_offset_00d5c5c0, sizeof(Word)}, {&c.normal_rgb_00ce3e18, sizeof(Word)},
            {&c.one_00d7a24c, sizeof(Word)}, {&c.disabled_alpha_00ce3800, sizeof(Word)}})
            require(disjoint(writes[i], borrowed), "Text default writes cannot alias its provider/global graph");
    }
    // No read of fresh unconstructed Text188/4C. These are public provider
    // identity checks before the first AA9390 native effect; 85 rechecks its
    // current reached profile/domain at its own entry after the derived stores.
    require(&c.one_00d7a24c == &s.node_constants.one_00d7a24c &&
        &c.lifetime.nodes == &actual.models.nodes && c.lifetime.actual_base_table_00d5c130 &&
        actual.prepare_model && actual.retire_failed_model && actual.bind_completed_model &&
        !actual.models.actual_names && actual.models.nodes.uses_raw_name_pool() &&
        &actual.models.nodes.require_raw_name_pool() == &s.strings &&
        &s.strings == &s.string_process.raw_context() && &actual.strings == &s.string_process.strings() &&
        &s.parenting.node.strings == &s.strings && &actual.models.retained_owners == &owners &&
        &actual.materials.retained_owners == &owners && &s.parenting.node.scenes.owners == &owners &&
        &s.parenting.node.trees.owners == &owners &&
        &s.parenting.node.scenes.decrement_00ce2220 == &s.decrement_00ce2220 &&
        &s.parenting.node.trees.decrement_00ce2220 == &s.decrement_00ce2220 && s.decrement_00ce2220 &&
        &s.effects.effects.owners.actual_owners() == &owners && &s.effects.strings == &actual.strings &&
        &actual.materials.parameter_names == &actual.strings &&
        &s.effects.effects.construction.strings == &actual.strings && s.effects.effects.cache == &s.effects &&
        s.effects.effects.construction.current_renderer_00f8d394 &&
        word(s.effects.effects.construction.current_renderer_00f8d394) == 0x00d5f0a8u &&
        s.effects.effects.renderer_profile_00d5f0a8 &&
        s.effects.effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0u &&
        s.shadow_00d15ec4 && s.guidefault_00d5c25c && actual.material_vtable_00d5e520 &&
        &actual.models.constants.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &actual.models.constants.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc &&
        &mesh.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &mesh.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc &&
        &actual.models.constants.unchanged_00d7a260 == &actual.unchanged_00d7a260,
        "Text default requires the same genuine raw constants, string, owner and cleanup domains");
}
void raw_base_admission(void* raw, NativeGuiTextRawModelDefaultStorageContext& c) {
    auto& p = game::game_native_gui_text_process();
    require(&c.lifetime.actual_base_lineage_00f8bc88 == &p.element_lineage_00f8bc88() &&
        &c.lifetime.actual_text_descriptor_00f8be28 == &p.text_lineage_00f8be28()[0],
        "Raw Text cleanup requires the CURRENT canonical initialized type cells");
    const Word descriptor = native_gui_text_descriptor_00ab6a30(c.lifetime.actual_text_descriptor_00f8be28);
    require(!native_gui_widget_is_kind_of_00a9e070(descriptor, c.lifetime.actual_base_lineage_00f8bc88),
        "Raw Text DEFAULT cleanup does not admit a CURRENT base-lineage Text match");
    void* const head = reinterpret_cast<void*>(word(raw, 0x68));
    require(!word(raw, 0x4c) && !word(raw, 0x70) && !word(raw, 0x6c) && head &&
        word(head) == reinterpret_cast<Word>(head) && word(head, 4) == reinterpret_cast<Word>(head) &&
        !word(raw, 0x88) && !word(raw, 0x8c) && !word(raw, 0x90) &&
        !word(raw, 0x180) && !word(raw, 0x184) && !word(raw, 0x18c) && !word(raw, 0x190) &&
        !word(raw, 0x198) && !word(raw, 0x19c) && !word(raw, 0x1a0) && !word(raw, 0x1ec) &&
        !word(raw, 0xec) && !word(raw, 0xf0) && !word(raw, 0xf4) && !word(raw, 0xf8) &&
        !word(raw, 0x1a4) && !word(raw, 0x1a8) && !word(raw, 0x1c0) && !word(raw, 0x1c4) &&
        !word(raw, 0x1c8) && !word(raw, 0x1cc),
        "Raw Text DEFAULT base cleanup requires genuine empty parent/children/timed/glyph/shader branches");
}
void preflight(void* raw, NativeGuiTextRawModelDefaultStorageFrame& f,
    NativeGuiTextRawModelDefaultStorageContext& c, NativeGuiTextRawModelDefaultStorageAcquired& a) {
    auto& s = c.sections;
    auto& storage = s.model.storage;
    auto& node = storage.node;
    auto& owners = s.geometry.actual_owners();
    const auto mesh = s.geometry.mesh_constants();
    require(raw && (reinterpret_cast<std::uintptr_t>(raw) & 3u) == 0 &&
        !a.started && !a.complete && a.native_eh_state == -1 && !a.sections.started &&
        a.failure == decltype(a.failure)::none && a.sections.failure == decltype(a.failure)::none &&
        !a.sections.model.actual && !a.sections.model.construction.started && !a.sections.model.reference &&
        !a.sections.model.lifetime.started && !a.sections.model.lifetime.scalar_started &&
        !a.sections.model.release.started && !a.sections.mesh.started &&
        !a.sections.shadow_section.section.started && !a.sections.main_section.section.started &&
        a.sections.shadow_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        a.sections.main_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        !a.sections.parenting.started &&
        a.sections.model.association.phase == NativeModelStorageGeometryPhase::fresh &&
        f.sections.association.phase() == NativeModelStorageGeometryPhase::fresh,
        "Raw Text default requires its WHOLE fresh stable producer graph before the first pool effect");
    const Region writes[] = {{raw, 0x1f4}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.sections, sizeof(f.sections)}, {&f.sections.locals[0], sizeof(f.sections.locals)},
        {&f.sections.arguments[0], sizeof(f.sections.arguments)},
        {&f.sections.parenting, sizeof(f.sections.parenting)}};
    for (std::size_t i = 0; i != 7; ++i) {
        for (std::size_t j = i + 1; j != 7; ++j)
            require(disjoint(writes[i], writes[j]), "Raw Text default caller regions must be disjoint");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&s, sizeof(s)},
            {&c.lifetime, sizeof(c.lifetime)}, {&c.state, sizeof(c.state)},
            {&s.model, sizeof(s.model)}, {&storage, sizeof(storage)}, {&node, sizeof(node)},
            {&storage.pool_01090054, sizeof(storage.pool_01090054)},
            {&storage.pool_01090054.storage(), sizeof(storage.pool_01090054.storage())},
            {&s.association, sizeof(s.association)}, {&s.geometry, sizeof(s.geometry)},
            {&s.materials, sizeof(s.materials)}, {&s.material_strings, sizeof(s.material_strings)},
            {&s.strings, sizeof(s.strings)}, {&s.string_process, sizeof(s.string_process)},
            {&s.effects, sizeof(s.effects)}, {&s.parenting, sizeof(s.parenting)},
            {&storage.constants, sizeof(storage.constants)}, {&c.one_00d7a24c, sizeof(Word)},
            {&c.shadow_offset_00d5c5c0, sizeof(Word)}, {&c.normal_rgb_00ce3e18, sizeof(Word)},
            {&c.disabled_alpha_00ce3800, sizeof(Word)}})
            require(disjoint(writes[i], borrowed), "Raw Text default writes cannot alias its borrowed graph");
    }
    require(&c.state == &game::game_native_gui_text_state() &&
        &c.one_00d7a24c == &storage.constants.one_00d7a24c &&
        &s.association.storage == &storage && &s.parenting.node == &node &&
        c.lifetime.actual_base_table_00d5c130 && c.lifetime.nodes.uses_raw_name_pool() &&
        &c.lifetime.nodes.require_raw_name_pool() == &s.strings &&
        &node.trees.point_lights == &c.lifetime.nodes.point_lights &&
        &node.strings == &s.strings && same_native_string_raw_pool_cells(s.strings, s.string_process.raw_context()) &&
        same_native_string_storage_raw_pool_cells(s.material_strings, s.strings) &&
        &s.materials.retained_owners == &owners &&
        &node.scenes.owners == &owners && &node.trees.owners == &owners &&
        &node.scenes.decrement_00ce2220 == &node.trees.decrement_00ce2220 && node.trees.decrement_00ce2220 &&
        &s.effects.effects.owners.actual_owners() == &owners &&
        &s.effects.strings == &s.material_strings && &s.materials.parameter_names == &s.material_strings &&
        &s.effects.effects.construction.strings == &s.material_strings && s.effects.effects.cache == &s.effects &&
        &mesh.maximum_00ce4970 == &storage.constants.positive_bound_00ce4970 &&
        &mesh.minimum_00ce4adc == &storage.constants.negative_bound_00ce4adc &&
        s.model.actual_profile_00d62de8 && s.model.actual_profile_00d62de8[0] == 0x00bd30e0 &&
        s.model.actual_profile_00d62de8[1] == 0x00b75290 && s.model.actual_profile_00d62de8[3] == 0x006ef860 &&
        node.world.resolve_profile(0x00d62de8) == s.model.actual_profile_00d62de8 &&
        node.trees.dispatch.resolve_profile(0x00d62de8) == s.model.actual_profile_00d62de8 &&
        s.parenting.dispatch.resolve_profile(0x00d62de8) == s.model.actual_profile_00d62de8 &&
        s.effects.effects.construction.current_renderer_00f8d394 &&
        word(s.effects.effects.construction.current_renderer_00f8d394) == 0x00d5f0a8 &&
        s.effects.effects.renderer_profile_00d5f0a8 &&
        s.effects.effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0 &&
        s.shadow_00d15ec4 && s.guidefault_00d5c25c && s.material_vtable_00d5e520,
        "Raw Text default requires the same genuine Model/pool/import/string/type/effect providers");
    auto& p = game::game_native_gui_text_process();
    require(&c.lifetime.actual_base_lineage_00f8bc88 == &p.element_lineage_00f8bc88() &&
        &c.lifetime.actual_text_descriptor_00f8be28 == &p.text_lineage_00f8be28()[0] &&
        !native_gui_widget_is_kind_of_00a9e070(native_gui_text_descriptor_00ab6a30(
            c.lifetime.actual_text_descriptor_00f8be28), c.lifetime.actual_base_lineage_00f8bc88),
        "Raw Text default needs initialized CURRENT canonical lineage and an admitted empty base branch");
}
void before_base(void*, NativeGuiTextDefaultStorageContext&, NativeGuiTextDefaultStorageAcquired&) {}
void before_base(void* raw, NativeGuiTextRawModelDefaultStorageContext& c,
    NativeGuiTextRawModelDefaultStorageAcquired& a) {
    a.failure = decltype(a.failure)::host_metadata;
    source_host(a, __COUNTER__, [&] { raw_base_admission(raw, c); }); // AFTER preceding callbacks
    a.failure = decltype(a.failure)::provider_call;
}
template<class Context, class Acquired>
void unwind(void* raw, Context& c, Acquired& a) {
    a.exception_cleanup_started = true;
    try {
        // Source C++ projection of DEEEAC, regardless of a SOURCE provider's
        // exception-origin diagnostics. Do not interpret provider_call as an
        // Original/native exception classification or discover creators here.
        while (a.native_eh_state >= 0) {
            const auto state = a.native_eh_state;
            if (state == 0) before_base(raw, c, a);
            a.native_eh_state = state - 1;
            a.consumed_cleanup_actions |= Word{1} << state;
            switch (state) {
            case 6: a.active_call_site = 0x00cb8097;
                destroy_native_string_header_0041dd20(at(raw, 0x1c8), c.sections.strings); break;
            case 5: a.active_call_site = 0x00cb8089;
                destroy_native_string_header_0041dd20(at(raw, 0x1c0), c.sections.strings); break;
            case 4: a.active_call_site = 0x00cb807b;
                destroy_native_wide_string_header_00436430(at(raw, 0x1a4), c.sections.strings); break;
            case 3: a.active_call_site = 0x00cb806d;
                tidy_native_gui_text_pointer_vector_005d6cf0(at(raw, 0x194)); break;
            case 2: a.active_call_site = 0x00cb805f;
                destroy_native_string_header_0041dd20(at(raw, 0xf4), c.sections.strings); break;
            case 1: a.active_call_site = 0x00cb8051;
                destroy_native_wide_string_header_00436430(at(raw, 0xec), c.sections.strings); break;
            case 0: a.active_call_site = 0x00cb8043;
                destroy_native_gui_widget_base_00aa9730(raw, c.lifetime);
                a.base_lifetime_ended = true; break;
            default: std::terminate();
            }
        }
    } catch (...) {
        // Extra raw admission failure is retained, never interpreted as a
        // second native cleanup exception. Genuine cleanup escape terminates.
        if constexpr (std::is_same_v<Acquired, NativeGuiTextRawModelDefaultStorageAcquired>) {
            if (a.failure == decltype(a.failure)::host_metadata) throw;
        }
        std::terminate();
    }
}
} // namespace

void tidy_native_gui_text_pointer_vector_005d6cf0(void* vector) noexcept {
    void* const begin = reinterpret_cast<void*>(word(vector, 4));
    if (begin) std::free(begin); // 5D6CFB; SAME Source CRT backing, no element destruction
    word(vector, 4) = 0; // 5D6D03; even when begin was null
    word(vector, 8) = 0;
    word(vector, 0xc) = 0;
}

namespace {
void provider(NativeGuiTextDefaultStorageAcquired&) noexcept {}
void provider(NativeGuiTextRawModelDefaultStorageAcquired& a) noexcept {
    a.failure = decltype(a.failure)::provider_call;
}
void completed(void*, NativeGuiTextDefaultStorageContext&, NativeGuiTextDefaultStorageAcquired&) {}
void completed(void* raw, NativeGuiTextRawModelDefaultStorageContext& c,
    NativeGuiTextRawModelDefaultStorageAcquired& a) {
    a.failure = decltype(a.failure)::host_metadata;
    source_host(a, __COUNTER__, [&] { raw_base_admission(raw, c); });
    source_host(a, __COUNTER__, [&] { require(word(raw) == 0x00d5c6c8 && a.sections.complete &&
        a.sections.model.reference && a.sections.model.construction.complete &&
        a.sections.model.association.phase == NativeModelStorageGeometryPhase::complete &&
        word(raw, 0x188) == reinterpret_cast<Word>(a.sections.model.actual),
        "Raw Text DEFAULT completion requires its genuine completed internal Model graph"); });
    a.failure = decltype(a.failure)::none;
}
void failed(void* raw, NativeGuiTextDefaultStorageContext& c, NativeGuiTextDefaultStorageAcquired& a) {
    if (a.native_eh_state >= 0) unwind(raw, c, a);
}
void failed(void* raw, NativeGuiTextRawModelDefaultStorageContext& c,
    NativeGuiTextRawModelDefaultStorageAcquired& a) {
    const auto t = failure_transport(a);
    if (t.runtime) {
        t.origin->retain_current(*t.runtime); // exact private cause BEFORE public diagnostics
        return; // no Native issuer: deny ALL nested outer unwind on this used route
    }
    if (a.failure == decltype(a.failure)::host_metadata ||
        a.sections.failure == decltype(a.failure)::host_metadata) {
        a.failure = decltype(a.failure)::host_metadata;
        return; // NO native ctor unwind/slot-return permission for extra admission
    }
    if (a.native_eh_state >= 0) {
        a.failure = decltype(a.failure)::host_metadata;
        raw_base_admission(raw, c);
        a.failure = decltype(a.failure)::provider_call;
        unwind(raw, c, a);
    }
}
struct NoBodyReturn { void operator()() const {} };
template<class Frame, class Context, class Acquired, class BodyReturned = NoBodyReturn>
void* construct_default(void* raw, Frame& f, Context& c, Acquired& a, BodyReturned body_returned = {}) {
    source_host(a, __COUNTER__, [&] { preflight(raw, f, c, a); });
    a.started = true;
    a.destination = raw;
    try {
        provider(a);
        a.active_call_site = 0x00ab9671;
        source_unknown(a, 0x00ab9671u, __COUNTER__, [&] {
            construct_native_gui_widget_base_00aa9390(raw, 3, c.one_00d7a24c);
        });
        a.base_complete = true;
        word(raw) = 0x00d5c6c8; // AB9678
        a.native_eh_state = 0; // AB967E, AFTER the derived stamp
        word(raw, 0xec) = 0;
        word(raw, 0xf0) = 0;
        word(raw, 0xf4) = 0;
        word(raw, 0xf8) = 0;
        const Word shadow_offset = c.shadow_offset_00d5c5c0; // AB969D
        word(raw, 0x100) = 0;
        word(raw, 0x104) = 0;
        word(raw, 0x108) = 0;
        word(raw, 0x10c) = 0;
        word(raw, 0x110) = 0;
        word(raw, 0x114) = 0;
        byte(raw, 0x118) = 0;
        byte(raw, 0x15c) = 0;
        word(raw, 0x160) = 0;
        word(raw, 0x164) = shadow_offset;
        byte(raw, 0xfc) = 1;
        // Four distinct load/store pairs; no early/copy quartet or float conversion.
        word(raw, 0x168) = shadow_word(c.state.shadow_color_00e12fd8()[0]);
        word(raw, 0x16c) = shadow_word(c.state.shadow_color_00e12fd8()[1]);
        word(raw, 0x170) = shadow_word(c.state.shadow_color_00e12fd8()[2]);
        word(raw, 0x174) = shadow_word(c.state.shadow_color_00e12fd8()[3]);
        word(raw, 0x178) = 0;
        word(raw, 0x17c) = 2;
        word(raw, 0x180) = 0;
        word(raw, 0x184) = 0;
        word(raw, 0x188) = 0;
        word(raw, 0x18c) = 0;
        word(raw, 0x190) = 0;
        word(raw, 0x198) = 0;
        word(raw, 0x19c) = 0;
        word(raw, 0x1a0) = 0;
        word(raw, 0x1a4) = 0;
        word(raw, 0x1a8) = 0;
        word(raw, 0x1ac) = 0;
        word(raw, 0x1b0) = 0;
        byte(raw, 0x1b4) = 0;
        word(raw, 0x1c0) = 0;
        word(raw, 0x1c4) = 0;
        word(raw, 0x1c8) = 0;
        word(raw, 0x1cc) = 0;
        const Word normal = c.normal_rgb_00ce3e18; // AB979B, one capture
        const Word one = c.one_00d7a24c; // AB97A3; distinct from both AA9390 loads
        word(raw, 0x1ec) = 0;
        word(raw, 0x11c) = normal;
        byte(raw, 0x1f0) = 1;
        word(raw, 0x120) = normal;
        word(raw, 0x124) = normal;
        a.native_eh_state = 6; // AB97F1
        word(raw, 0x1d0) = 0xffffffff;
        word(raw, 0x1d8) = one;
        word(raw, 0x128) = one;
        word(raw, 0x12c) = one;
        word(raw, 0x130) = one;
        word(raw, 0x134) = one;
        word(raw, 0x13c) = one;
        word(raw, 0x138) = one;
        const Word disabled_alpha = c.disabled_alpha_00ce3800; // AB987C, BEFORE140
        word(raw, 0x140) = one;
        word(raw, 0x144) = one;
        word(raw, 0x14c) = 0;
        word(raw, 0x148) = one;
        word(raw, 0x150) = 0;
        word(raw, 0x154) = 0;
        word(raw, 0x158) = disabled_alpha;
        a.derived_complete = true;
        a.sections_entered = true;
        a.active_call_site = 0x00ab98d8;
        if constexpr (std::is_same_v<Acquired, NativeGuiTextRawModelDefaultStorageAcquired>) {
            const auto t = failure_transport(a);
            if (t.runtime)
                t.origin->child(*t.runtime, {0x00ab98d8u, __COUNTER__}, &a.sections,
                    0x00ab8530u, a.sections.failure_origin(), [&] {
                        ensure_native_gui_text_sections_storage_00ab8530(
                            raw, f.sections, c.sections, a.sections, *t.runtime);
                    });
            else ensure_native_gui_text_sections_storage_00ab8530(raw, f.sections, c.sections, a.sections);
        } else ensure_native_gui_text_sections_storage_00ab8530(raw, f.sections, c.sections, a.sections);
        body_returned(); // PRIVATE negative frontier IMMEDIATELY after actual ctor85 normal return
        source_host(a, __COUNTER__, [&] { completed(raw, c, a); });
        a.complete = true;
        return raw;
    } catch (...) {
        // AA9390 failure is outer state-1 and its own provider already cleans
        // the failed base prefix. After successful base construction, unwind
        // only the represented Text subobjects/base, never independent creators.
        failed(raw, c, a);
        throw;
    }
}
} // namespace
void* construct_native_gui_text_default_storage_00ab9650(void* raw,
    NativeGuiTextDefaultStorageFrame& f, NativeGuiTextDefaultStorageContext& c,
    NativeGuiTextDefaultStorageAcquired& a) { return construct_default(raw, f, c, a); }
void* construct_native_gui_text_default_storage_00ab9650(void* raw,
    NativeGuiTextRawModelDefaultStorageFrame& f, NativeGuiTextRawModelDefaultStorageContext& c,
    NativeGuiTextRawModelDefaultStorageAcquired& a) {
    try { return construct_default(raw, f, c, a); }
    catch (...) {
        if (!a.started) a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
void* construct_native_gui_text_default_storage_00ab9650(void* raw,
    NativeGuiTextRawModelDefaultStorageFrame& f, NativeGuiTextRawModelDefaultStorageContext& c,
    NativeGuiTextRawModelDefaultStorageAcquired& a, NativeGuiTextFailureConstructorCall& call) {
    // Unprepared envelopes have no qualified catch service. Never query a CRT
    // to discover that domain, nor run any cleanup on this pre-entry rejection.
    if (!call.runtime_) throw std::invalid_argument("Constructor envelope requires its SAME qualified Source runtime");
    call.bind_once(raw, a.failure_origin_, &a); // before Source preflight/Native work; invalidation is sticky
    if (!a.failure_runtime_) a.failure_runtime_ = call.runtime_; // never replace a retained service
    a.failure_origin_.begin(*a.failure_runtime_, &a, 0x00ab9650u);
    try {
        void* const result = construct_default(raw, f, c, a, [&] {
            call.body_returned(a.failure_origin_, &a); // not whole Source completion or raw122 birth
        });
        a.failure_origin_.complete();
        return result;
    } catch (...) {
        a.failure_origin_.retain_current(*a.failure_runtime_); // sticky HOST/UNKNOWN, before diagnostic flag
        if (!a.started) a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
} // namespace bsp
