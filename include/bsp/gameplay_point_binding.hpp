#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/gameplay_point_effect.hpp"
#include "bsp/point_effect_constructor.hpp"
#include "bsp/point_effect_owner.hpp"
#include "bsp/native_gamepad_force_event.hpp"
#include "bsp/point_effect_children.hpp"

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
// The explicitly selected typed or raw context, its manager/cells, string and
// component providers, and actual table must outlive this domain and every
// bound reference owner. A raw context installs no actual callable vtable.
// All terminal releases of a bound definition must use this domain. Its raw
// scalar870D00/871440 dependency includes the recovered three-state C++ cleanup;
// intrusive terminal callbacks, including component terminals, must not throw.
// Native exception-dispatch ABI and invalidated storage remain unvalidated.
class GameplayDefinitionReferences final {
public:
    GameplayDefinitionReferences(GameplayEffectDefinitionContext&,
        const volatile std::uint32_t* actual_table_00d0da58);
    GameplayDefinitionReferences(NativeGameplayEffectDefinitionContext&,
        const volatile std::uint32_t* actual_table_00d0da58);
    ~GameplayDefinitionReferences(); // Requires all bound owners retired.
    // Reuses the canonical companion. Pure metadata allocation on first bind;
    // requires the native constructor already ran and a positive actual count.
    NativeGameplayEffectDefinitionReference& bind(GameplayEffectDefinition&);
    // Borrow an existing canonical companion by exact raw address, or null.
    // Does not read the raw owner/count, allocate, retain, bind or retire.
    // The result expires when that companion's metadata is retired.
    NativeGameplayEffectDefinitionReference* find_bound(
        const GameplayEffectDefinition*) const noexcept;
    // Explicit raw-domain terminal entry after one genuine actual1->0 release.
    // Requires an existing binding, live count0 owner, current D0DA58/slot0
    // BD30E0 and fresh current slot4=871440. Calls the genuine Source providers
    // with flags1, then retires metadata only after successful scalar return.
    // Invalid/unknown inputs or cleanup failure terminate. Providers must not
    // throw; no concurrent/reentrant same-owner terminal or retry is admitted.
    // Adds no decrement, binding, Native table or automatic raw-caller route.
    void dispatch_bound_definition_zero(GameplayEffectDefinition&) noexcept;
    // Pure nonthrowing identity projection of a previously bound companion.
    // Unknown identity violates the native-binding precondition (terminates).
    GameplayEffectDefinition& definition_for(RenderCommandReference&) const noexcept;
    std::size_t binding_count() const noexcept { return references_.size(); }
private:
    friend class NativeGameplayEffectDefinitionReference;
    friend class GameplayPointConstruction;
    class BoundZeroDeleteCalls;
    void release_zero(NativeGameplayEffectDefinitionReference&) noexcept;
    void retire(NativeGameplayEffectDefinitionReference&) noexcept;
    void require_slot(GameplayEffectDefinition&, std::size_t index,
        std::uint32_t expected) const;
    NativeStringStorage& string_storage() const noexcept;
    union {
        GameplayEffectDefinitionContext* typed_context_;
        NativeGameplayEffectDefinitionContext* raw_context_;
    };
    const volatile std::uint32_t* table_;
    std::vector<std::unique_ptr<NativeGameplayEffectDefinitionReference>> references_;
    const bool raw_context_domain_;
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
        EffectPointView, CameraTransform&) = 0;
    virtual RenderCommandReference* create_current(std::uint32_t native_function,
        void* actual_row, PointEffectInstanceStorage&) = 0;
    virtual std::uint8_t restart_current(std::uint32_t native_function, void* actual_row) = 0;
};
// Compose the three established rumble factories with other actual component
// implementations. Unknown functions still require the remaining dispatch.
class GameplayPointRumbleComponents final : public GameplayPointRemainingComponents {
public:
    GameplayPointRumbleComponents(NativeGamepadForceEvents&, GameplayPointRemainingComponents&) noexcept;
    std::uint8_t admit_current(std::uint32_t, void*, EffectPointView,
        CameraTransform&) override;
    RenderCommandReference* create_current(std::uint32_t, void*, PointEffectInstanceStorage&) override;
    std::uint8_t restart_current(std::uint32_t, void*) override;
private:
    NativeGamepadForceEvents& events_;
    GameplayPointRemainingComponents& remaining_;
};
using GameplayPointReferenceLookup = std::function<CameraTransform&()>;

// Concrete raw definition/row projections and current-table dispatch for the
// established86B7D0 predicate and86A820 shake factory. Other slots require their
// real implementations above. The immutable table bindings, current table
// words, lookup lvalue and spatial associations must remain alive. The lookup
// reloads actual [E188A8]+19FC; it must not throw or snapshot a transform.
class GameplayPointRows final : public PointEffectRowRuntime, public EffectAdmissionDispatch,
    public PointEffectChildRows {
public:
    GameplayPointRows(GameplayDefinitionReferences&, const GameplayPointComponentTable*,
        std::size_t table_count, GameplayPointReferenceLookup&,
        ForceEventSpatialHost&, GameplayPointRemainingComponents&);
    EffectAdmissionTemplateView template_rows(RenderCommandReference&) noexcept override;
    PointEffectFactoryRowView row_fields(void*) noexcept override;
    EffectAdmissionRowView project_row(void*) noexcept override;
    CameraTransform& reference_e188a8_19fc() noexcept override;
    std::uint8_t virtual_1c(void*, EffectPointView, CameraTransform&) override;
    RenderCommandReference* create_virtual_18(void*, PointEffectInstanceStorage&) override;
    PointEffectRestartRowView restart_fields(void*) noexcept override;
    std::uint8_t restart_virtual_08(void*) override;
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

// Routes existing native rumble companions to their actual fields/current
// virtuals. All other event owners require the supplied remaining runtime.
// Domain lookup allocates nothing and never creates another reference count.
class GameplayPointChildEvents final : public PointEffectChildEvents {
public:
    GameplayPointChildEvents(NativeGamepadForceEvents&, PointEffectChildEvents&) noexcept;
    PointEffectChildView child_fields(RenderCommandReference&) noexcept override;
    void update_virtual_28(RenderCommandReference&, float, void*) override;
    std::uint8_t complete_virtual_08(RenderCommandReference&) override;
    void deactivate_virtual_30(RenderCommandReference&) override;
private:
    NativeGamepadForceEvents& events_;
    PointEffectChildEvents& remaining_;
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
    bool eligible_0086a650(RenderCommandReference&, EffectPointView,
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
