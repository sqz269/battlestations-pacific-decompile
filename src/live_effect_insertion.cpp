#include "bsp/live_effect_insertion.hpp"

#include <atomic>
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Live effect insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
void* read_cell(const void* cell) noexcept {
    void* result;
    std::memcpy(&result, cell, sizeof(result));
    return result;
}
void write_cell(void* cell, void* value) noexcept {
    std::memcpy(cell, &value, sizeof(value));
}
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
void retain_raw(void* owner) noexcept {
    auto* actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    actual->fetch_add(1, std::memory_order_seq_cst);
}
class CapturedEffectSection final {
public:
    explicit CapturedEffectSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedEffectSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedEffectSection(const CapturedEffectSection&) = delete;
    CapturedEffectSection& operator=(const CapturedEffectSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
class InsertionTemporary final {
public:
    InsertionTemporary(void*& cell, NativeRenderActualOwners& owners) noexcept
        : cell_(cell), owners_(owners) {}
    ~InsertionTemporary() {
        if (armed_) {
            if (void* captured = cell_) {
                release_native_render_actual_owner(owners_, captured);
                cell_ = nullptr;
            }
        }
    }
    void disarm() noexcept { armed_ = false; }
    InsertionTemporary(const InsertionTemporary&) = delete;
    InsertionTemporary& operator=(const InsertionTemporary&) = delete;
private:
    void*& cell_;
    NativeRenderActualOwners& owners_;
    bool armed_{true};
};
}

void reserve_live_effect_references_004c5ce0(NativeRenderPointerArrayStorage& input,
    std::int32_t requested, NativeRenderActualOwners& owners) {
    volatile auto& array = input;
    if (requested < 1) requested = 1;
    if (array.capacity_08 >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 4u;
    void* const replacement = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    void* destination = replacement;
    for (std::int32_t index = 0; index < array.count_04; index = signed_word(static_cast<std::uint32_t>(index) + 1u)) {
        if (destination) {
            void* const source = at(array.data_00, static_cast<std::uint32_t>(index) * 4u);
            write_cell(destination, nullptr);
            if (void* value = read_cell(source)) {
                write_cell(destination, value);
                retain_raw(value);
            }
        }
        destination = at(destination, 4);
    }
    for (std::int32_t index = 0; index < array.count_04; index = signed_word(static_cast<std::uint32_t>(index) + 1u)) {
        void* const cell = at(array.data_00, static_cast<std::uint32_t>(index) * 4u);
        if (void* captured = read_cell(cell)) {
            release_native_render_actual_owner(owners, captured);
            write_cell(cell, nullptr); // clear captured old slot after terminal reentry
        }
    }
    singleton_lifetime_free(array.data_00);
    array.data_00 = static_cast<void**>(replacement);
    array.capacity_08 = requested;
}

void append_live_effect_reference_0074d780(NativeRenderPointerArrayStorage& input,
    const void* source, NativeRenderActualOwners& owners) {
    volatile auto& array = input;
    if (array.count_04 == array.capacity_08) {
        auto grown = signed_word(static_cast<std::uint32_t>(array.capacity_08) * 2u);
        if (grown <= 1) grown = 1;
        reserve_live_effect_references_004c5ce0(input, grown, owners);
    }
    void* const destination = at(array.data_00, static_cast<std::uint32_t>(array.count_04) * 4u);
    if (destination) {
        write_cell(destination, nullptr);
        if (void* value = read_cell(source)) {
            write_cell(destination, value);
            retain_raw(value);
        }
    }
    array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) + 1u);
}

void resize_live_effect_references_004c9550(NativeRenderPointerArrayStorage& input,
    std::int32_t requested, NativeRenderActualOwners& owners) {
    volatile auto& array = input;
    if (requested > array.capacity_08)
        reserve_live_effect_references_004c5ce0(input, requested, owners);
    for (std::int32_t index = array.count_04; index < requested;
        index = signed_word(static_cast<std::uint32_t>(index) + 1u)) {
        if (void* cell = at(array.data_00, static_cast<std::uint32_t>(index) * 4u))
            write_cell(cell, nullptr);
    }
    while (requested < array.count_04) {
        array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) - 1u);
        void* const cell = at(array.data_00, static_cast<std::uint32_t>(array.count_04) * 4u);
        if (void* captured = read_cell(cell)) {
            release_native_render_actual_owner(owners, captured);
            write_cell(cell, nullptr);
        }
    }
    array.count_04 = requested;
}

void insert_live_effect_00867500(NativeRenderPointerArrayStorage& array,
    void* effect, EffectManager* volatile& global, EffectManagerLifetimeAccess& access,
    NativeRenderActualOwners& owners) {
    auto* const manager = effect_manager_singleton_00866440(global, access);
    CapturedEffectSection section(manager->section_04);
    if (effect) {
        void* temporary = effect;
        InsertionTemporary unwind(temporary, owners);
        append_live_effect_reference_0074d780(array, &temporary, owners);
        unwind.disarm();
        release_native_render_actual_owner(owners, effect);
        retain_raw(effect);
    }
}

} // namespace bsp
