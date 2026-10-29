#include "bsp/native_gui_text_default_factory_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include <initializer_list>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DEFAULT Text factory requires MSVC Win32.
#endif

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Region { std::uintptr_t first; std::size_t bytes; };
Region region(const volatile void* p, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(p);
    require((p || !bytes) && bytes <= (std::numeric_limits<std::uintptr_t>::max)() - first,
        "Raw DEFAULT factory needs valid bounded caller storage");
    return {first, bytes};
}
bool disjoint(Region a, Region b) noexcept {
    return !a.bytes || !b.bytes || a.first + a.bytes <= b.first || b.first + b.bytes <= a.first;
}
void known_regions(NativeGuiTextDefaultFactoryStorageContext& c,
    NativeGuiTextDefaultFactoryStorageAcquired& a) {
    auto& r = c.publication;
    auto& ctor = r.construction_context;
    auto& frame = r.construction_frame;
    require(c.companion_storage && c.companion_storage_bytes >= sizeof(NativeGuiTextRawModelReference) &&
        !(reinterpret_cast<std::uintptr_t>(c.companion_storage) % alignof(NativeGuiTextRawModelReference)),
        "Raw DEFAULT factory requires prepared aligned raw122 companion storage");
    require(c.disposal.retire && ((c.disposal.context == nullptr) == (c.disposal.context_bytes == 0)),
        "Raw DEFAULT factory requires its real sized companion-only disposal context");
    const Region writes[] = {region(&a, sizeof(a)),
        region(c.companion_storage, c.companion_storage_bytes),
        region(&c.reference_frame, sizeof(c.reference_frame))};
    const Region borrowed[] = {region(&c, sizeof(c)), region(&r, sizeof(r)),
        region(&c.process, sizeof(c.process)),
        region(&ctor, sizeof(ctor)), region(&ctor.sections, sizeof(ctor.sections)),
        region(&c.construction, sizeof(c.construction)), region(&frame, sizeof(frame)),
        region(&frame.sections, sizeof(frame.sections)),
        region(&frame.sections.locals[0], sizeof(frame.sections.locals)),
        region(&frame.sections.arguments[0], sizeof(frame.sections.arguments)),
        region(&frame.sections.parenting, sizeof(frame.sections.parenting)),
        region(&r.retirement, sizeof(r.retirement)), region(&ctor.lifetime, sizeof(ctor.lifetime)),
        region(&ctor.state, sizeof(ctor.state))};
    for (std::size_t i = 0; i != 3; ++i) {
        for (std::size_t j = i + 1; j != 3; ++j)
            require(disjoint(writes[i], writes[j]), "Raw DEFAULT factory caller writes must be disjoint");
        for (const Region b : borrowed)
            require(disjoint(writes[i], b), "Raw DEFAULT factory writes cannot alias their prepared graph");
    }
    const Region disposal = region(c.disposal.context, c.disposal.context_bytes);
    require(disjoint(writes[1], disposal) && disjoint(writes[2], disposal),
        "Raw DEFAULT factory disposal cannot overlap companion/scalar-frame writes");
}
NativeGuiTextPool& prepared_pool(NativeGuiTextDefaultFactoryStorageContext& c) {
    auto& r = c.publication;
    auto& ctor = r.construction_context;
    auto& produced = c.construction;
    auto& sections = ctor.sections;
    auto& node = sections.model.storage.node;
    require(&c.process == &game::game_native_gui_text_process(),
        "Raw DEFAULT factory requires the canonical existing Text process");
    NativeGuiTextPool& pool = c.process.pool_00f8bdf0(); // ready-only borrow, no startup
    require(&r.construction == &produced && &r.retirement.resources == &sections &&
        &r.retirement.model == &produced.sections.model && &r.retirement.base == &ctor.lifetime &&
        &r.retirement.actual_pool_00f8bdf0 == &pool &&
        &r.owners == &sections.geometry.actual_owners() &&
        &ctor.lifetime.actual_base_lineage_00f8bc88 == &c.process.element_lineage_00f8bc88() &&
        &ctor.lifetime.actual_text_descriptor_00f8be28 == &c.process.text_lineage_00f8be28()[0],
        "Raw DEFAULT factory requires the SAME prepared93/122/188/pool/type domains");
    require(ctor.lifetime.nodes.uses_raw_name_pool() &&
        &ctor.lifetime.nodes.require_raw_name_pool() == &sections.strings &&
        &node.strings == &sections.strings &&
        same_native_string_raw_pool_cells(sections.strings, sections.string_process.raw_context()) &&
        same_native_string_storage_raw_pool_cells(sections.material_strings, sections.strings) &&
        &node.trees.point_lights == &ctor.lifetime.nodes.point_lights,
        "Raw DEFAULT factory requires genuine shared raw strings and base/Model point-light identity");
    const auto& model = produced.sections.model;
    const auto& scalar = c.reference_frame.scalar;
    require(!produced.started && !produced.complete && !produced.base_complete &&
        !produced.derived_complete && !produced.sections_entered &&
        !produced.exception_cleanup_started && !produced.base_lifetime_ended &&
        !produced.active_call_site && !produced.consumed_cleanup_actions &&
        !produced.destination && produced.native_eh_state == -1 &&
        produced.failure == NativeGuiTextRawModelDefaultStorageAcquired::Failure::none &&
        !produced.sections.started && !produced.sections.complete &&
        produced.sections.failure == NativeGuiTextRawModelDefaultStorageAcquired::Failure::none &&
        !model.actual && !model.reference && !model.construction.started &&
        !model.lifetime.started && !model.lifetime.scalar_started && !model.release.started &&
        model.association.phase == NativeModelStorageGeometryPhase::fresh &&
        r.construction_frame.sections.association.phase() == NativeModelStorageGeometryPhase::fresh &&
        !scalar.started && !scalar.complete && !scalar.destructor_entered &&
        !scalar.destructor_returned && !scalar.flags_captured &&
        !scalar.pool_return_entered && !scalar.pool_returned &&
        !scalar.destruction.started && !scalar.destruction.complete,
        "Raw DEFAULT factory requires fresh persistent producer/scalar frames before allocation");
    return pool;
}
} // namespace

void* make_native_gui_text_default_factory_storage_00aa1380(
    NativeGuiTextDefaultFactoryStorageContext& c, NativeGuiTextDefaultFactoryStorageAcquired& a) {
    using Phase = NativeGuiTextDefaultFactoryStorageAcquired::Phase;
    using Boundary = NativeGuiTextDefaultFactoryStorageAcquired::Boundary;
    require(!a.started && !a.complete && a.phase == Phase::fresh && a.boundary == Boundary::none &&
        !a.prepared_context && !a.construction_record && !a.actual_pool && !a.raw_slot &&
        !a.constructor_result && !a.allocation_entered && !a.allocation_returned &&
        !a.constructor_entered && !a.constructor_returned && !a.failure && !a.active_native_site,
        "Raw DEFAULT factory attempts cannot be reset, copied, reentered or replayed");
    known_regions(c, a); // no caller-record write or native effect before this succeeds
    a.prepared_context = &c;
    a.construction_record = &c.construction;
    a.started = true;
    a.phase = Phase::admission;
    a.boundary = Boundary::admission;
    try {
        a.actual_pool = &prepared_pool(c);
        a.phase = Phase::allocating;
        a.boundary = Boundary::allocation;
        a.active_native_site = 0x00aa139e;
        a.allocation_entered = true; // visible to AB79E0 allocation callbacks
        a.raw_slot = allocate_gui_text_raw_slot_00ab79e0(*a.actual_pool);
        a.allocation_returned = true;
        a.active_native_site = 0;
        if (!a.raw_slot) {
            a.phase = Phase::empty;
            a.boundary = Boundary::none;
            a.complete = true; // operation complete, no Text/188/reference exists
            return nullptr;
        }
        a.boundary = Boundary::admission;
        const Region text = region(a.raw_slot, NativeGuiTextPool::payload_bytes);
        require(disjoint(text, region(&a, sizeof(a))) && disjoint(text, region(&c, sizeof(c))) &&
            disjoint(text, region(c.companion_storage, c.companion_storage_bytes)) &&
            disjoint(text, region(&c.reference_frame, sizeof(c.reference_frame))),
            "Raw DEFAULT factory allocated Text cannot overlap caller factory/publication storage");
        a.phase = Phase::constructing;
        a.boundary = Boundary::constructor_call;
        a.active_native_site = 0x00aa13dd;
        a.constructor_entered = true;
        a.constructor_result = construct_native_gui_text_default_storage_00ab9650(a.raw_slot,
            c.publication.construction_frame, c.publication.construction_context, c.construction);
        a.constructor_returned = true;
        a.active_native_site = 0;
        a.boundary = Boundary::result_admission;
        require(a.constructor_result == a.raw_slot && c.construction.complete &&
            c.construction.sections.complete && c.construction.sections.model.actual &&
            c.construction.sections.model.reference && c.construction.sections.model.construction.complete &&
            c.construction.sections.model.association.phase == NativeModelStorageGeometryPhase::complete,
            "Raw DEFAULT factory completion requires SAME Text and its mandatory genuine188 graph");
        a.phase = Phase::complete;
        a.boundary = Boundary::none;
        a.complete = true;
        return a.constructor_result; // separate existing raw122 bind; NO retain/publication here
    } catch (...) {
        a.failure = std::current_exception();
        a.phase = Phase::failed;
        throw; // keep pool/Text/93/188/frames/providers; never infer AB76F0 cleanup
    }
}
} // namespace bsp
