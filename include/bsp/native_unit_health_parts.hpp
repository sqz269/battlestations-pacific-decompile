#pragma once
#include "bsp/native_unit_scene_initialization.hpp"
#include "bsp/unit_parts.hpp"

namespace bsp {

// Borrowed fields of the existing checked pointer vector at unit+344. Its
// begin/end are +4/+8, unlike NativeRenderPointerArrayStorage's data/count.
// No copied vector, native-layout cast, allocation or implicit initialization.
struct NativeUnitPartPointerVectorView {
    void* identity;
    void**& begin_04;
    void**& end_08;
};
struct NativeUnitClassPartsView {
    const float& hp_48;
    const float& armour_4c;
    const std::byte* const& parts_begin_1c;
    const std::byte* const& parts_end_20;
    void* const& part_set_50;
};
struct NativeUnitSavedPartsView {
    const std::int32_t& state_58;
    const std::int32_t& numbering_5c;
};
struct NativeUnitHealthPartsView {
    NativeUnitObserverAlias unit;
    void*& parent_3c;
    void*& property_holder_c0; // SAME cell as NativeUnitSceneInitializationView
    float& marker_164;
    NativeUnitPartPointerVectorView parts_344;
    void*& descriptor_354;
    std::int32_t& numbering_35c;
    void*& model_360; // SAME cell as NativeUnitSceneInitializationView
    float& armour_368;
    float& maximum_health_36c;
    float& health_370;
    std::uint8_t& condition_378;
};
struct NativeUnitHealthPartsGlobals {
    const float& one_00d7a24c;
    const float& detail_00ced9e0; // observed bits 3f733333 (0.95f)
};

class NativeUnitHealthPartsBindings {
public:
    virtual ~NativeUnitHealthPartsBindings() = default;
    // Pure, nonthrowing access to live actual backing. These must not mutate,
    // allocate, callback, cache owners or alter the floating-point environment.
    // Every reached nonnull native identity must have a valid binding.
    virtual NativeUnitClassPartsView class_parts(void*) noexcept = 0;
    virtual void* parent_3c(void*) noexcept = 0;
    virtual NativeUnitScenePropertyHolderView property_holder(void*) noexcept = 0;
    virtual NativeUnitSavedPartsView saved_parts(void*) noexcept = 0;
    virtual std::int32_t property_integer_0c(void*) noexcept = 0;
    virtual const volatile std::uint32_t* primary_table(void*) noexcept = 0;

    // Whole existing operations, not projections or permissive defaults. Saved
    // table entries are dispatch tokens resolved by the owning runtime, never
    // cast to process addresses. Indirect callees are not identified by this
    // packet; names intentionally retain the evidenced slot only.
    virtual void call_0077f0e0(void* unit) = 0;
    // Concrete default: canonical actual10h checked-vector storage. Valid,
    // consistently owned header; allocation callbacks must not mutate it.
    // Existing equivalent ownership-domain overrides remain supported.
    virtual void call_0087b460(void* vector, std::uint32_t count,
        std::uint32_t fill); // native ECX=vector, RET8
    virtual void call_00bf6713() = 0; // may return; preserve subsequent reloads
    virtual std::uint32_t call_parent_b0(std::uint32_t entry, void* parent) = 0;
    virtual std::uint8_t call_unit_5c(std::uint32_t entry, void* unit,
        std::int32_t kind) = 0;
    // +190 takes NO stack argument: checked profiles use 0080DF80 (RET).
    // The pre-pushed detail survives it and is argument TWO of part-set +8.
    virtual std::uint32_t call_unit_190(std::uint32_t entry, void* unit) = 0;
    virtual void* call_part_set_08(std::uint32_t entry, void* part_set,
        std::uint32_t selector, float detail) = 0; // native RET8
    virtual void* call_00bf681b(std::size_t bytes) = 0;
    virtual void* call_007135c0(void* allocation, void* unit, void* part_set) = 0;
    virtual void call_00bf65ac(void* allocation) = 0;
    virtual void call_00876ec0(void* unit, std::int32_t state) = 0;
    virtual void* call_008f2260(void* bag, const char* key) = 0;
    virtual void call_00711be0(void* model, std::int32_t numbering) = 0;
};

// Complete caller 0087BCC0..0087BF73 (692 bytes), original ECX=unit, RET.
// Preserves reads/stores, callback rebindings and source allocation cleanup.
// Requires the complete providers above and stable actual borrowed storage.
// New C++ ABI; no native FH3/SEH, runtime unit binding or gameplay proof.
// unit_initial_condition_0087bcc0 and unit_part_detail_0087bcc0 remain partial
// value projections and do not substitute for this caller.
void initialize_native_unit_health_parts_0087bcc0(NativeUnitHealthPartsView,
    NativeUnitHealthPartsGlobals, NativeUnitHealthPartsBindings&);

} // namespace bsp
