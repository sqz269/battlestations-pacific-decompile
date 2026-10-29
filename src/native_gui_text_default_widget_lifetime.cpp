#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/native_gui_widget_child_links.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error DEFAULT Text widget lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Call = NativeGuiTextDefaultWidgetLifetimeCall;
using Op = Call::Operation;
constexpr Word base_profile = 0x00d5c130;
constexpr Word text_profile = 0x00d5c6c8;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(p) + offset;
}
Word read(void* p, std::size_t offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
struct Region { const volatile void* first; std::size_t bytes; };
bool disjoint(Region a, Region b) noexcept {
    const auto left = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(a.first));
    const auto right = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(b.first));
    return left + a.bytes <= right || right + b.bytes <= left;
}
bool representable(Region r) noexcept {
    return (r.first || !r.bytes) &&
        static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(r.first)) + r.bytes <= 0x100000000ull;
}
} // namespace

NativeGuiTextDefaultWidgetLifetime::NativeGuiTextDefaultWidgetLifetime(
    NativeNodeDestructionRuntime& nodes, NativeRenderActualOwnerRegistry& registry,
    game::GameNativeReadOnlyData& data, game::GameNativeGuiTextProcess& process,
    NativeGuiTextDefaultWidgetLifetimeAcquired& acquired)
    : registry_(registry), data_(data), process_(process), acquired_(acquired),
      base_(static_cast<const volatile Word*>(data.data_at(base_profile, 9 * sizeof(Word)))),
      text_(static_cast<const volatile Word*>(data.data_at(text_profile, 9 * sizeof(Word)))),
      text_lineage_(process.text_lineage_00f8be28()),
      context_{nodes, base_, process.element_lineage_00f8bc88(), text_lineage_[0], *this} {
    require(&process == &game::game_native_gui_text_process() &&
        !acquired.bound && !acquired.active && !acquired.poisoned && !acquired.completed_calls &&
        acquired.current.operation == Op::none && acquired.failure.operation == Op::none,
        "DEFAULT widget metadata requires its canonical ready process and fresh diagnostics");
}
NativeGuiTextDefaultWidgetLifetime::~NativeGuiTextDefaultWidgetLifetime() {
    if (acquired_.active || acquired_.poisoned) std::terminate();
}
void NativeGuiTextDefaultWidgetLifetime::begin(Call call) {
    if (acquired_.poisoned)
        throw std::logic_error("DEFAULT widget provider cannot reuse poisoned metadata");
    if (acquired_.active) {
        acquired_.poisoned = true;
        if (acquired_.failure.operation == Op::none) acquired_.failure = call;
        throw std::logic_error("DEFAULT widget provider rejects reentrant lifetime dispatch");
    }
    if (call.pointer_cell) {
        const Region cell{call.pointer_cell, sizeof(Word)};
        if (!representable(cell) || !disjoint(cell, {this, sizeof(*this)}) ||
            !disjoint(cell, {&acquired_, sizeof(acquired_)})) {
            acquired_.poisoned = true;
            if (acquired_.failure.operation == Op::none) acquired_.failure = call;
            throw std::logic_error("DEFAULT list pointer-cell cannot alias provider diagnostic writes");
        }
    }
    acquired_.current = call;
    acquired_.active = true; // before any receiver/table/predicate/list access
}
void NativeGuiTextDefaultWidgetLifetime::finish() noexcept {
    ++acquired_.completed_calls;
    acquired_.active = false;
}
void NativeGuiTextDefaultWidgetLifetime::fail() noexcept {
    acquired_.poisoned = true;
    if (acquired_.failure.operation == Op::none) acquired_.failure = acquired_.current;
    acquired_.active = false;
}
[[noreturn]] void NativeGuiTextDefaultWidgetLifetime::reject(const char* message) {
    fail();
    throw std::logic_error(message);
}
NativeGuiTextDefaultWidgetLifetimeRecord& NativeGuiTextDefaultWidgetLifetime::record() {
    require(acquired_.bound && record_, "DEFAULT widget lifetime has no prepared record");
    return *record_;
}

void NativeGuiTextDefaultWidgetLifetime::bind(NativeGuiTextDefaultWidgetLifetimeRecord& r) {
    Call call; call.operation = Op::bind;
    begin(call);
    try {
        require(!acquired_.bound && !record_, "DEFAULT widget record binding is once only");
        auto& f = r.factory_context;
        auto& p = f.publication;
        auto& c = f.construction;
        auto& s = r.publication;
        require(&f.process == &process_ && &p.data == &data_ && &p.owners == &registry_ &&
            &p.construction == &c && &p.construction_context.lifetime == &context_ &&
            &p.retirement.base == &context_ &&
            &p.retirement.resources == &p.construction_context.sections &&
            &p.retirement.model == &c.sections.model &&
            &p.retirement.glyphs.nodes == &context_.nodes &&
            &p.retirement.glyphs.children == this &&
            &p.construction_context.sections.model.storage.node.trees.point_lights ==
                &context_.nodes.point_lights,
            "DEFAULT widget binding requires EXACT factory/raw93/raw122/188/base/glyph identities");
        require(r.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::fresh &&
            !r.factory.started && !r.factory.complete && !r.factory.raw_slot &&
            !c.started && !c.complete && !c.destination &&
            !s.reference && !s.bind_entered && !s.bind_complete && !s.bind_failed &&
            !s.scalar_entered && !s.scalar_returned && !s.scalar_failed &&
            !s.companion_disposal_entered && !s.disposed,
            "DEFAULT widget record must bind before factory/publication/scalar entry");
        require(f.companion_storage && f.companion_storage_bytes >= sizeof(NativeGuiTextRawModelReference) &&
            f.disposal.retire && ((f.disposal.context != nullptr) == (f.disposal.context_bytes != 0)),
            "DEFAULT widget binding requires the real prepared raw122 storage/disposal");
        const Region writable[] = {{this, sizeof(*this)}, {&acquired_, sizeof(acquired_)},
            {&r, sizeof(r)}, {&s, sizeof(s)}};
        const Region borrowed[] = {{&f, sizeof(f)}, {&r.factory, sizeof(r.factory)},
            {&p, sizeof(p)}, {&c, sizeof(c)},
            {&p.construction_context, sizeof(p.construction_context)},
            {&p.construction_frame, sizeof(p.construction_frame)},
            {&p.construction_frame.sections, sizeof(p.construction_frame.sections)},
            {&p.construction_frame.sections.locals[0], sizeof(p.construction_frame.sections.locals)},
            {&p.construction_frame.sections.arguments[0], sizeof(p.construction_frame.sections.arguments)},
            {&p.construction_frame.sections.parenting, sizeof(p.construction_frame.sections.parenting)},
            {&p.retirement, sizeof(p.retirement)}, {&f.reference_frame, sizeof(f.reference_frame)},
            {f.companion_storage, f.companion_storage_bytes},
            {f.disposal.context, f.disposal.context_bytes}};
        for (std::size_t i = 0; i != 4; ++i) {
            require(representable(writable[i]), "DEFAULT widget caller region has an invalid extent");
            for (std::size_t j = i + 1; j != 4; ++j)
                require(disjoint(writable[i], writable[j]), "DEFAULT widget caller records must be separate");
            for (const auto b : borrowed)
                require(representable(b) && disjoint(writable[i], b),
                    "DEFAULT widget records cannot overlap their independent producer graph");
        }
        record_ = &r;
        acquired_.bound = true;
        finish();
    } catch (...) { fail(); throw; }
}

NativeGuiTextDefaultWidgetLifetime::Admission
NativeGuiTextDefaultWidgetLifetime::authenticate(void* actual) {
    auto& r = record();
    auto& f = r.factory_context;
    const auto& a = r.factory;
    const auto& c = f.construction;
    const auto& s = r.publication;
    const auto& scalar = f.reference_frame.scalar;
    // Independent records FIRST. In particular never inspect the diagnostic
    // reference pointer after placement destruction, nor an ended raw payload.
    require(!s.bind_failed && !s.scalar_failed && !s.companion_disposal_entered && !s.disposed &&
        !s.scalar_returned && !c.base_lifetime_ended && !scalar.complete &&
        !scalar.pool_returned && !scalar.destructor_returned &&
        !scalar.destruction.complete && !scalar.destruction.base_returned &&
        a.phase != NativeGuiTextDefaultFactoryStorageAcquired::Phase::failed && !a.failure,
        "DEFAULT widget dispatch rejects failed/disposing/ended/returned metadata");
    require(actual && a.started && a.allocation_returned && a.raw_slot == actual &&
        a.prepared_context == &f && a.construction_record == &c &&
        a.actual_pool == &f.publication.retirement.actual_pool_00f8bdf0 &&
        c.started && c.destination == actual && c.base_complete && c.derived_complete &&
        context_.actual_base_table_00d5c130 == base_ && &context_.bindings == this,
        "DEFAULT widget receiver is not its SAME actual factory/raw93 allocation");
    if (!s.bind_entered) {
        require(!s.bind_complete && !s.reference && !s.scalar_entered &&
            a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::constructing &&
            a.boundary == NativeGuiTextDefaultFactoryStorageAcquired::Boundary::constructor_call &&
            a.constructor_entered && !a.constructor_returned && !c.complete &&
            registry_.find(actual) == nullptr,
            "DEFAULT widget constructor prefix requires genuine pre-raw122 factory/raw93 entry");
        return Admission::construction;
    }
    require(s.bind_complete && s.reference &&
        a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete && a.complete &&
        a.constructor_returned && a.constructor_result == actual && c.complete &&
        !c.exception_cleanup_started && !c.consumed_cleanup_actions &&
        static_cast<void*>(s.reference) == f.companion_storage &&
        registry_.find(actual) == s.reference,
        "DEFAULT widget dispatch requires its SAME completed canonical raw122 binding");
    // The companion is known alive by the owner's independent disposal marker.
    const auto phase = s.reference->phase();
    require(phase == NativeGuiTextRawModelReference::Phase::bound ||
        (s.scalar_entered && phase == NativeGuiTextRawModelReference::Phase::retiring),
        "DEFAULT widget reference is not in an admitted bound/scalar phase");
    require(s.reference->storage() == actual && &s.reference->storage_context() == &f.publication,
        "DEFAULT widget reference borrows a different producer/lifetime graph");
    if (phase == NativeGuiTextRawModelReference::Phase::bound && !s.scalar_entered) {
        require(!scalar.started && !scalar.destructor_entered && !scalar.destruction.started,
            "DEFAULT widget bound metadata hides an entered scalar operation");
        return Admission::bound;
    }
    require(s.scalar_entered, "DEFAULT widget retiring prefix lacks its owner scalar entry");
    return Admission::retiring;
}
Word NativeGuiTextDefaultWidgetLifetime::active_profile(void* actual, Admission admission) {
    auto& r = record();
    const auto& c = r.factory_context.construction;
    const auto& d = r.factory_context.reference_frame.scalar.destruction;
    const Word profile = read(actual);
    require(profile == text_profile || profile == base_profile,
        "DEFAULT widget dispatch has no live Text/base prefix");
    if (profile == base_profile) {
        const bool ctor_unwind = admission == Admission::construction &&
            c.exception_cleanup_started && !c.base_lifetime_ended &&
            (c.consumed_cleanup_actions & 1u) && c.native_eh_state == -1 &&
            c.active_call_site == 0x00cb8043;
        const bool scalar_base = admission == Admission::retiring &&
            d.started && d.destination == actual && d.base_entered &&
            !d.base_returned && d.native_eh_state == -1;
        require(ctor_unwind || scalar_base,
            "DEFAULT base prefix is not its actual93 unwind or118 base destruction");
    }
    return profile;
}
void NativeGuiTextDefaultWidgetLifetime::require_empty_branches(void* actual) {
    // No188 read here: its lifetime can be partial in constructor cleanup.
    require(!read(actual, 0x4c) && !read(actual, 0x70) && !read(actual, 0x6c) &&
        !read(actual, 0x88) && !read(actual, 0x8c) && !read(actual, 0x90) &&
        !read(actual, 0x180) && !read(actual, 0x184) && !read(actual, 0x18c) && !read(actual, 0x190) &&
        !read(actual, 0x198) && !read(actual, 0x19c) && !read(actual, 0x1a0) && !read(actual, 0x1ec) &&
        !read(actual, 0xec) && !read(actual, 0xf0) && !read(actual, 0xf4) && !read(actual, 0xf8) &&
        !read(actual, 0x1a4) && !read(actual, 0x1a8) &&
        !read(actual, 0x1c0) && !read(actual, 0x1c4) && !read(actual, 0x1c8) && !read(actual, 0x1cc),
        "DEFAULT widget excludes primary/parent/children/timed/glyph/content/shader arms");
    void* const head = reinterpret_cast<void*>(read(actual, 0x68));
    require(head && !(reinterpret_cast<std::uintptr_t>(head) & 3u) &&
        read(head) == reinterpret_cast<Word>(head) && read(head, 4) == reinterpret_cast<Word>(head),
        "DEFAULT widget requires its genuine finite empty AA9390 list ring");
}

void NativeGuiTextDefaultWidgetLifetime::require_default_before_scalar() {
    Call call; call.operation = Op::scalar_admission;
    begin(call);
    try {
        auto& r = record();
        void* const actual = r.factory.raw_slot;
        acquired_.current.receiver = actual;
        require(authenticate(actual) == Admission::bound && !r.publication.scalar_entered,
            "DEFAULT scalar admission must precede the owner's canonical scalar entry");
        require(active_profile(actual, Admission::bound) == text_profile,
            "DEFAULT scalar admission requires its completed Text prefix");
        require_empty_branches(actual);
        require(base_[3] == 0x00a9e070 && base_[8] == 0x00aa8320 &&
            text_[0] == 0x00bd30e0 && text_[1] == 0x00ab8ee0 &&
            text_[3] == 0x00ab83d0 && text_[8] == 0x00aa8320,
            "DEFAULT scalar admission requires current genuine base/Text query/scene/scalar selectors");
        const Word descriptor = native_gui_text_descriptor_00ab6a30(context_.actual_text_descriptor_00f8be28);
        acquired_.current.descriptor = descriptor;
        acquired_.current.body_entered = true;
        const bool result = native_gui_widget_is_kind_of_00a9e070(
            descriptor, context_.actual_base_lineage_00f8bc88);
        acquired_.current.result = result;
        acquired_.current.body_returned = true;
        if (result) reject("DEFAULT base Text-descriptor branch is outside the raw188 lifetime domain");
        finish();
    } catch (...) { fail(); throw; }
}
void NativeGuiTextDefaultWidgetLifetime::require_raw_model_transport_receiver(
    void* actual, NativeGuiTextDefaultWidgetLifetimeRecord& expected_record) {
    Call call; call.operation = Op::raw_model_authentication; call.receiver = actual;
    begin(call);
    try {
        require(&record() == &expected_record,
            "Raw Model transport requires this EXACT bound widget lifetime record");
        const auto admission = authenticate(actual);
        (void)active_profile(actual, admission); // admission only; NEVER a selector snapshot
        finish();
    } catch (...) { fail(); throw; }
}
const volatile Word* NativeGuiTextDefaultWidgetLifetime::current_table(void* actual, Word profile) {
    Call call; call.operation = Op::table; call.receiver = actual; call.profile = profile;
    begin(call);
    try {
        const auto admission = authenticate(actual);
        (void)active_profile(actual, admission);
        require_empty_branches(actual);
        require(profile == base_profile || profile == text_profile,
            "DEFAULT widget lookup rejects foreign captured profiles");
        // Return the ACTUAL table alias selected by the captured profile.
        // AA8320 captures its class CELL address, reads descriptor, THEN target.
        const volatile Word* const result = profile == base_profile ? base_ : text_;
        finish();
        return result;
    } catch (...) { fail(); throw; }
}
bool NativeGuiTextDefaultWidgetLifetime::call_class_query_0c(void* actual, Word target, Word descriptor) {
    Call call; call.operation = Op::class_query; call.receiver = actual;
    call.target = target; call.descriptor = descriptor;
    begin(call);
    try {
        const auto admission = authenticate(actual);
        (void)active_profile(actual, admission);
        require_empty_branches(actual);
        require(target == 0x00a9e070 || target == 0x00ab83d0,
            "DEFAULT widget query rejects an unsupported captured target");
        acquired_.current.body_entered = true;
        // No current table-target reload, ID snapshot, fixed tag or zero case.
        const bool result = target == 0x00a9e070 ?
            native_gui_widget_is_kind_of_00a9e070(descriptor, context_.actual_base_lineage_00f8bc88) :
            gui_text_matches_type0c_00ab83d0(descriptor, text_lineage_);
        acquired_.current.result = result;
        acquired_.current.body_returned = true;
        // Separate admission AFTER the genuine predicate. The caller's Text
        // descriptor provenance is this canonical cell; no188 read is safe or
        // needed to reject the unsupported legacy AA8320 Text-release branch.
        if (result && descriptor ==
            native_gui_text_descriptor_00ab6a30(context_.actual_text_descriptor_00f8be28))
            reject("TRUE canonical Text-descriptor lifetime dispatch would enter legacy188 cleanup");
        finish();
        return result; // Other descriptor/base queries keep the genuine answer.
    } catch (...) { fail(); throw; }
}
void NativeGuiTextDefaultWidgetLifetime::call_child_scene_20(void* actual, Word target) {
    Call call; call.operation = Op::child_scene; call.receiver = actual; call.target = target;
    begin(call);
    reject("DEFAULT widget child-scene dispatch is outside its admitted lifetime");
}
void NativeGuiTextDefaultWidgetLifetime::call_child_delete_04(void* actual, Word target, Word flags) {
    Call call; call.operation = Op::child_delete; call.receiver = actual; call.target = target; call.flags = flags;
    begin(call);
    reject("DEFAULT widget child scalar dispatch is unsupported; owner uses canonical raw122 directly");
}
void NativeGuiTextDefaultWidgetLifetime::call_timed_delete_00(void* actual, Word target, Word flags) {
    Call call; call.operation = Op::timed_delete; call.receiver = actual; call.target = target; call.flags = flags;
    begin(call);
    reject("DEFAULT widget timed-entry dispatch is outside its admitted lifetime");
}
void NativeGuiTextDefaultWidgetLifetime::remove_all_00a9bd50(void* list, const void* pointer_cell) {
    Call call; call.operation = Op::remove_all; call.receiver = list; call.pointer_cell = pointer_cell;
    begin(call);
    try {
        auto& r = record();
        void* const actual = r.factory.raw_slot;
        const auto admission = authenticate(actual);
        (void)active_profile(actual, admission);
        require(list == at(actual, 0x64) && pointer_cell &&
            !(reinterpret_cast<std::uintptr_t>(pointer_cell) & 3u),
            "DEFAULT remove-all requires its CAPTURED owned AA9390 list and real pointer-cell");
        require_empty_branches(actual);
        auto* const captured_list = static_cast<NativeEffectDeletionListStorage*>(list);
        const void* const captured_cell = pointer_cell;
        acquired_.current.body_entered = true;
        // The real body captures initial head THEN payload, preserves native
        // reload/unlink/free/count order and the opaque allocator word. Never
        // reselect Text+64 or inspect list/payload after this call.
        remove_native_gui_widget_list_value_00a9bd50(*captured_list, captured_cell);
        acquired_.current.body_returned = true;
        finish();
    } catch (...) { fail(); throw; }
}
} // namespace bsp
