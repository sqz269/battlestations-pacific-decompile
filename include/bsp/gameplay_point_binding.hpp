#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/gameplay_point_effect.hpp"
#include "bsp/point_effect_constructor.hpp"
#include "bsp/point_effect_owner.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace bsp {
class GameplayDefinitionReferences;

// Stable HOST companion of one actual24h gameplay definition. Borrow the
// atomic object started at native construction+04, never initialize/retain it.
class NativeGameplayEffectDefinitionReference final : public RenderCommandReference {
public:
    GameplayEffectDefinition& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    friend class GameplayDefinitionReferences;
    NativeGameplayEffectDefinitionReference(GameplayEffectDefinition&,
        GameplayDefinitionReferences&) noexcept;
    GameplayEffectDefinition& storage_;
    GameplayDefinitionReferences& owner_;
};

// One existing application's definition companion domain; no second cache or
// native ownership. Context uses the actual shared singleton/strings/component
// destruction bindings. The supplied table borrows CURRENT D0DA58 words0/1.
// All terminal releases of a bound definition must use this domain. Its raw
// scalar870D00/871440 dependency includes the recovered three-state C++ cleanup;
// intrusive terminal callbacks, including component terminals, must not throw.
// Native exception-dispatch ABI and invalidated storage remain unvalidated.
class GameplayDefinitionReferences final {
public:
    GameplayDefinitionReferences(GameplayEffectDefinitionContext&,
        const volatile std::uint32_t* actual_table_00d0da58);
    ~GameplayDefinitionReferences(); // Requires all bound owners retired.
    // Reuses the canonical companion. Pure metadata allocation on first bind;
    // requires the native constructor already ran and a positive actual count.
    NativeGameplayEffectDefinitionReference& bind(GameplayEffectDefinition&);
    // Pure nonthrowing identity projection of a previously bound companion.
    // Unknown identity violates the native-binding precondition (terminates).
    GameplayEffectDefinition& definition_for(RenderCommandReference&) const noexcept;
    std::size_t binding_count() const noexcept { return references_.size(); }
private:
    friend class NativeGameplayEffectDefinitionReference;
    friend class GameplayPointConstruction;
    void release_zero(NativeGameplayEffectDefinitionReference&) noexcept;
    void require_slot(GameplayEffectDefinition&, std::size_t index,
        std::uint32_t expected) const;
    GameplayEffectDefinitionContext& context_;
    const volatile std::uint32_t* table_;
    std::vector<std::unique_ptr<NativeGameplayEffectDefinitionReference>> references_;
};

struct GameplayPointComponentTable {
    std::uint32_t original_identity;
    const volatile std::uint32_t* actual_words;
    std::size_t word_count;
};
// Remaining CURRENT native virtual implementations. No success/null default.
// Arguments include the function identity just read from the actual table.
class GameplayPointRemainingComponents {
public:
    virtual ~GameplayPointRemainingComponents() = default;
    virtual std::uint8_t admit_current(std::uint32_t native_function, void* actual_row,
        const std::array<float, 3>&, CameraTransform&) = 0;
    virtual RenderCommandReference* create_current(std::uint32_t native_function,
        void* actual_row, PointEffectInstanceStorage&) = 0;
};
using GameplayPointReferenceLookup = std::function<CameraTransform&()>;

// Concrete raw definition/row projections and current-table dispatch for the
// established86B7D0 predicate and86A820 shake factory. Other slots require their
// real implementations above. The immutable table bindings, current table
// words, lookup lvalue and spatial associations must remain alive. The lookup
// reloads actual [E188A8]+19FC; it must not throw or snapshot a transform.
class GameplayPointRows final : public PointEffectRowRuntime, public EffectAdmissionDispatch {
public:
    GameplayPointRows(GameplayDefinitionReferences&, const GameplayPointComponentTable*,
        std::size_t table_count, GameplayPointReferenceLookup&,
        ForceEventSpatialHost&, GameplayPointRemainingComponents&);
    EffectAdmissionTemplateView template_rows(RenderCommandReference&) noexcept override;
    PointEffectFactoryRowView row_fields(void*) noexcept override;
    EffectAdmissionRowView project_row(void*) noexcept override;
    CameraTransform& reference_e188a8_19fc() noexcept override;
    std::uint8_t virtual_1c(void*, const std::array<float, 3>&, CameraTransform&) override;
    RenderCommandReference* create_virtual_18(void*, PointEffectInstanceStorage&) override;
private:
    friend class GameplayPointConstruction;
    std::uint32_t current_virtual(void*, std::size_t word_index) const;
    GameplayDefinitionReferences& definitions_;
    const GameplayPointComponentTable* tables_;
    std::size_t table_count_;
    GameplayPointReferenceLookup& reference_;
    ForceEventSpatialHost& spatial_;
    GameplayPointRemainingComponents& remaining_;
};

// Concrete8689C0 construction dependency: actual866440 lock, actual86A650
// admission, native114h allocator/free and complete8680B0 constructor. The
// constructor bindings must use these SAME rows and the actual application
// singleton/string/node/point domains. No alternate manager or count is created.
class GameplayPointConstruction final : public PointEffectConstruction {
public:
    GameplayPointConstruction(GameplayDefinitionReferences&, GameplayPointRows&,
        PointEffectConstructorBindings&);
    PointEffectManagerView manager_00866440() override;
    CameraTransform& reference_transform_e188a8_19fc() override;
    bool eligible_0086a650(RenderCommandReference&, const std::array<float, 3>&,
        CameraTransform&) override;
    void* allocate_00bf681b(std::size_t native_bytes) override;
    void free_00bf65ac(void*) noexcept override;
    RenderCommandReference& construct_008680b0(void*, RenderCommandReference*,
        CameraTransform*, std::uint32_t, const CameraMatrix&, std::uint8_t,
        std::uint8_t, std::uint32_t) override;
private:
    GameplayDefinitionReferences& definitions_;
    GameplayPointRows& rows_;
    PointEffectConstructorBindings& bindings_;
};
// New C++ application composition; no native vtable/binary ABI overlay.
} // namespace bsp
