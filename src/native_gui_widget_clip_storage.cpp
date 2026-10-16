#include "bsp/native_gui_widget_clip_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_window_focus_dispatch.hpp"
#include <array>
#include <cstddef>
#include <cstring>
#include <exception>
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI clipping requires the audited MSVC Win32 target.
#endif
static_assert(sizeof(void*) == 4, "Native raw headers contain DWORD pointers");

namespace bsp {
namespace {
using Word = std::uint32_t;
using Phase = NativeGuiWidgetClipPhase;
using Stage = NativeGuiWidgetClipStage;
using Action = NativeGuiWidgetClipNameAction;
struct Region { std::uintptr_t first, last; };
Region region(const volatile void* identity, std::size_t bytes) {
    const auto first = reinterpret_cast<std::uintptr_t>(identity);
    if (first == 0 || (first & 3u) != 0 ||
        bytes > (std::numeric_limits<std::uintptr_t>::max)() - first)
        throw std::invalid_argument("Raw clip requires aligned nonwrapping live regions");
    return {first, first + bytes};
}
bool overlaps(Region left, Region right) noexcept {
    return left.first < right.last && right.first < left.last;
}
Word word(const void* identity, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const std::byte*>(identity) + offset);
}
void store_word(void* identity, std::size_t offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(identity) + offset) = value;
}
void* pointer(const void* identity, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(word(identity, offset));
}
bool active_after_ordered_zero_compare(const void* actual_enabled,
    const volatile Word* actual_zero) noexcept {
    // AA9FCC..AA9FDB: preserve SSE unordered/SNaN/MXCSR behavior. Native JNP
    // skips ordered zero; SETP after the same AH test selects active instead.
    unsigned char active;
    __asm {
        mov eax, actual_enabled
        mov edx, actual_zero
        movss xmm0, dword ptr [eax]
        ucomiss xmm0, dword ptr [edx]
        lahf
        test ah, 44h
        setp active
    }
    return active != 0;
}
} // namespace

__declspec(noinline) NativeGuiWidgetClipOperation::Views NativeGuiWidgetClipOperation::prepare(
    void* widget, NativeMaterialStorage& material, NativeGuiWidgetClipBindings& bindings,
    const void* metadata) {
    // FIRST member initializer: all known placement checks precede metadata
    // stores, and no widget/header/vector/source payload is read here.
    const std::array<Region, 9> known{{region(metadata, sizeof(NativeGuiWidgetClipOperation)),
        region(widget, 0xec), region(&material, sizeof(material)),
        region(&bindings, sizeof(bindings)), region(&bindings.strings, sizeof(bindings.strings)),
        region(&bindings.materials, sizeof(bindings.materials)),
        region(&bindings.actual_one_00d7a24c, sizeof(Word)),
        region(&bindings.actual_zero_00d7a218, sizeof(Word)),
        region(&bindings.actual_aspect_ratio_00e12fc0, sizeof(float))}};
    for (std::size_t i = 0; i != known.size(); ++i)
        for (std::size_t j = i + 1; j != known.size(); ++j)
            if (overlaps(known[i], known[j]))
                throw std::invalid_argument("Raw clip known regions overlap");
    const auto& data = bindings.actual_read_only_data;
    if (static_cast<const volatile void*>(&bindings.actual_one_00d7a24c) !=
            data.data_at(0x00d7a24c, sizeof(Word)) ||
        static_cast<const volatile void*>(&bindings.actual_zero_00d7a218) !=
            data.data_at(0x00d7a218, sizeof(Word)))
        throw std::invalid_argument("Raw clip requires the same actual one/zero views");
    return {static_cast<const char*>(data.data_at(0x00d5c1c4, 6)),
        static_cast<const char*>(data.data_at(0x00d5c1b8, 12)),
        static_cast<const char*>(data.data_at(0x00d5c1ac, 12)),
        static_cast<const char*>(data.data_at(0x00d19804, 13))};
}

NativeGuiWidgetClipOperation::NativeGuiWidgetClipOperation(void* widget,
    NativeMaterialStorage& material, NativeGuiWidgetClipBindings& bindings)
    : views_(prepare(widget, material, bindings, this)), actual_widget_(widget),
      material_(material), bindings_(bindings) {}

NativeGuiWidgetClipOperation::~NativeGuiWidgetClipOperation() {
    if (phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
}

void NativeGuiWidgetClipOperation::return_name() {
    void* const header = name_action_ == Action::aspect ?
        static_cast<void*>(&aspect_name_) : static_cast<void*>(&primary_name_);
    void* const data = pointer(header, 4); // Native capture BEFORE state consumption.
    name_action_ = Action::none;
    if (data) {
        const Word bytes = word(header, 0) + 1u; // Current length BEFORE getter.
        auto& strings = bindings_.strings;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, bytes,
            strings.actual_small_returns_disabled_01090aa4);
    } // Returned header stays unchanged, including provider changes.
}

void NativeGuiWidgetClipOperation::unwind_name() noexcept {
    const auto action = name_action_;
    if (action == Action::none) return;
    name_action_ = Action::none; // Consume before potentially throwing 41DD20/getter.
    stage_ = Stage::unwind_name;
    void* const header = action == Action::aspect ?
        static_cast<void*>(&aspect_name_) : static_cast<void*>(&primary_name_);
    try { destroy_native_string_header_0041dd20(header, bindings_.strings); }
    catch (...) { std::terminate(); }
}

void register_native_gui_widget_clip_storage_00aa9f10(void* widget,
    NativeMaterialStorage& material, NativeGuiWidgetClipBindings& bindings,
    NativeGuiWidgetClipOperation& operation) {
    if (operation.phase_ != Phase::fresh || operation.actual_widget_ != widget ||
        &operation.material_ != &material || &operation.bindings_ != &bindings)
        throw std::logic_error("Raw clip requires its fresh exact operation");
    operation.phase_ = Phase::entered;
    try {
        operation.stage_ = Stage::ancestor_selection;
        void* cursor = widget;
        while (cursor) {
            const Word profile = word(cursor, 0);
            const auto* const slot = static_cast<const volatile Word*>(
                bindings.actual_read_only_data.data_at(static_cast<Word>(profile + 0x5cu), 4));
            if (*slot != 0x00a9e110u)
                throw std::logic_error("Raw clip has no genuine current type-slot binding");
            if (read_native_gui_widget_type_00a9e110(cursor) == 16) break;
            cursor = pointer(cursor, 0x70); // Current parent AFTER the type call.
        }
        operation.captured_ancestor_ = cursor;
        const Word enabled = cursor ? bindings.actual_one_00d7a24c : 0u;
        auto* const actual_enabled = static_cast<std::byte*>(widget) + 0xe8;
        operation.stage_ = Stage::enabled_store;
        store_word(actual_enabled, 0, enabled); // MOVSS bit move, no conversion.

        operation.stage_ = Stage::clip_construct;
        void* const primary = &operation.primary_name_;
        store_word(primary, 0, 0);
        store_word(primary, 4, 0);
        resize_native_string_header_0041dd40(primary, bindings.strings, 5, true);
        if (void* const data = pointer(primary, 4)) {
            const Word bytes = word(primary, 0) + 1u;
            if (bytes) std::memmove(data, operation.views_.clip, bytes);
        } // Existing raw BF7680 boundary: overlap admitted, zero copy omitted.
        operation.name_action_ = Action::primary; // Native state0, ONLY after construction.
        operation.stage_ = Stage::clip_register;
        register_native_material_parameter_00b17e10(material, primary, actual_enabled,
            1, 0, bindings.materials);
        ++operation.returned_registrations_;
        operation.stage_ = Stage::clip_name_return;
        operation.return_name();
        operation.stage_ = Stage::enabled_reload;
        if (active_after_ordered_zero_compare(actual_enabled, &bindings.actual_zero_00d7a218)) {
            operation.stage_ = Stage::center_construct;
            construct_native_string_header_0041e870(primary, bindings.strings, operation.views_.center);
            operation.name_action_ = Action::primary;
            operation.stage_ = Stage::center_register;
            register_native_material_parameter_00b17e10(material, primary,
                static_cast<std::byte*>(cursor) + 0xec, 2, 0, bindings.materials);
            ++operation.returned_registrations_;
            operation.stage_ = Stage::center_name_return;
            operation.return_name();

            operation.stage_ = Stage::border_construct;
            construct_native_string_header_0041e870(primary, bindings.strings, operation.views_.border);
            operation.name_action_ = Action::primary;
            operation.stage_ = Stage::border_register;
            register_native_material_parameter_00b17e10(material, primary,
                static_cast<std::byte*>(cursor) + 0xf4, 4, 0, bindings.materials);
            ++operation.returned_registrations_;
            operation.stage_ = Stage::border_name_return;
            operation.return_name();

            operation.stage_ = Stage::aspect_construct;
            void* const aspect = &operation.aspect_name_;
            construct_native_string_header_0041e870(aspect, bindings.strings, operation.views_.aspect);
            operation.name_action_ = Action::aspect;
            operation.stage_ = Stage::aspect_register;
            register_native_material_parameter_00b17e10(material, aspect,
                &bindings.actual_aspect_ratio_00e12fc0, 1, 0, bindings.materials);
            ++operation.returned_registrations_;
            operation.stage_ = Stage::aspect_name_return;
            operation.return_name();
        }
        operation.phase_ = Phase::returned;
    } catch (...) {
        operation.failed_stage_ = operation.stage_;
        operation.phase_ = Phase::failed;
        operation.unwind_name();
        throw;
    }
}
} // namespace bsp
