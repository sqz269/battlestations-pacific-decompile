#include "bsp/point_effect_lookup.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(PointEffectInstanceStorage) == 0x114);

template<class T> T load(const void* owner, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(owner) + offset, sizeof result);
    return result;
}
RenderCommandReference** child_slot(RenderCommandReference** begin,
    std::int32_t index) noexcept {
    return reinterpret_cast<RenderCommandReference**>(
        reinterpret_cast<std::uintptr_t>(begin) + static_cast<std::uint32_t>(index) * 4u);
}
void retain_component(void* owner) noexcept {
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(static_cast<std::byte*>(owner) + 4));
}
void release_component(void* owner, GameplayEffectComponentLifetime& lifetime) {
    if (owner && InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<std::byte*>(owner) + 4)) == 0)
        lifetime.zero_references_slot_00(owner);
}
class CapturedLookupSection final {
public:
    explicit CapturedLookupSection(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedLookupSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
private:
    SystemSingletonCriticalSection* section_;
};
class ComponentUnwind final {
public:
    ComponentUnwind(void*& slot, GameplayEffectComponentLifetime& lifetime) noexcept
        : slot_(slot), lifetime_(lifetime) {}
    ~ComponentUnwind() noexcept {
        if (armed_) release_and_clear();
    }
    void finish() {
        // Native transitions state2 -> state1 BEFORE normal release; a throw
        // must not retry the component cleanup on the way to lock/output unwind.
        armed_ = false;
        release_and_clear();
    }
private:
    void release_and_clear() {
        if (void* const captured = slot_) {
            release_component(captured, lifetime_);
            slot_ = nullptr;
        }
    }
    void*& slot_;
    GameplayEffectComponentLifetime& lifetime_;
    bool armed_{true};
};
} // namespace

bool find_gameplay_effect_component_0086b100(GameplayEffectDefinition& definition,
    const void* name, void*& output, std::int32_t& output_index,
    GameplayEffectComponentLifetime& lifetime) {
    // The count is read before backing at entry; the entire span is captured.
    const auto count = load<std::uint32_t>(definition.native.data(), 0xc);
    auto cursor = reinterpret_cast<std::uintptr_t>(load<void*>(definition.native.data(), 8));
    const auto end = cursor + count * 4u;
    std::uint32_t index = 0;
    while (cursor != end) {
        void* const row = load<void*>(reinterpret_cast<void*>(cursor), 0);
        const auto query_length = load<std::uint32_t>(name, 0);
        const auto row_length = load<std::uint32_t>(row, 8);
        if (row_length == query_length && (row_length == 0 ||
                _stricmp(load<const char*>(row, 0xc), load<const char*>(name, 4)) == 0)) {
            // Native reloads this actual slot after CRT comparison.
            void* const incoming = load<void*>(reinterpret_cast<void*>(cursor), 0);
            void* const previous = output;
            if (previous != incoming) {
                output = incoming;
                if (incoming) retain_component(incoming);
                release_component(previous, lifetime);
            }
            // Written after terminal reentry, and absent if that release throws.
            std::memcpy(&output_index, &index, sizeof index);
            return true;
        }
        cursor += 4u;
        ++index;
    }
    return false;
}

bool find_gameplay_effect_component_0086b100(GameplayEffectDefinition& definition,
    const NativeString& name, void*& output, std::int32_t& output_index,
    GameplayEffectComponentLifetime& lifetime) {
    return find_gameplay_effect_component_0086b100(definition, &name, output, output_index, lifetime);
}

RenderCommandReference** find_or_create_point_effect_child_00866cd0(
    PointEffectInstanceStorage& point, RenderCommandReference** output,
    const void* name, PointEffectNameLookupBindings bindings) {
    bool output_constructed = false;
    try {
        auto* const manager = effect_manager_singleton_00866440(
            bindings.global_00f87650, bindings.lifetime);
        CapturedLookupSection lock(manager->section_04);
        void* component = nullptr;
        ComponentUnwind unwind(component, bindings.components);
        std::int32_t index;
        auto& definition = bindings.definitions.definition(*point.template_84);
        if (!find_gameplay_effect_component_0086b100(definition, name, component,
                index, bindings.components)) {
            *output = nullptr;
        } else {
            if (*child_slot(point.entries_0c.begin, index) == nullptr) {
                RenderCommandReference* const result = bindings.rows.create_virtual_18(component, point);
                RenderCommandReference* temporary = result;
                assign_point_effect_entry_reference_006fbeb0(
                    child_slot(point.entries_0c.begin, index), &temporary);
                if (result) release_render_command_reference(*result);
            }
            auto** const source = child_slot(point.entries_0c.begin, index);
            *output = nullptr;
            if (auto* const current = *source) {
                *output = current;
                retain_render_command_reference(*current);
            }
        }
        output_constructed = true;
        unwind.finish();
    } catch (...) {
        // DC6BCC state0: runs AFTER state1 captured-lock unwind, only after the
        // native result-construction bit was set. No prior-output cleanup.
        if (output_constructed) clear_point_effect_entry_reference_006cf070(output);
        throw;
    }
    return output;
}

RenderCommandReference** find_or_create_point_effect_child_00866cd0(
    PointEffectInstanceStorage& point, RenderCommandReference** output,
    const NativeString& name, PointEffectNameLookupBindings bindings) {
    return find_or_create_point_effect_child_00866cd0(point, output, &name, bindings);
}

RenderCommandReference** find_point_effect_child_by_type_00866e60(
    PointEffectInstanceStorage& point, RenderCommandReference** output, std::uint32_t type,
    PointEffectChildEvents& events, EffectManager* volatile& global,
    EffectManagerLifetimeAccess& lifetime) {
    auto* const manager = effect_manager_singleton_00866440(global, lifetime);
    CapturedLookupSection lock(manager->section_04);
    const auto count = point.entries_0c.count;
    if (count > 0) {
        auto** const begin = point.entries_0c.begin;
        for (std::int32_t index = 0; index < count; ++index) {
            auto** const source = child_slot(begin, index);
            if (*source && events.child_fields(**source).type_18 == type) {
                auto* const selected = *source;
                *output = nullptr;
                if (selected) {
                    *output = selected;
                    retain_render_command_reference(*selected);
                }
                return output;
            }
        }
    }
    *output = nullptr;
    return output;
}

} // namespace bsp
