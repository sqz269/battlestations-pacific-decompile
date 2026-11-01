#include "bsp/native_gui_text_active_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_gui_widget_child_lookup_storage.hpp"
#include "bsp/native_string_raw_header_return.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

__forceinline Word word(const volatile void* location) noexcept {
    Word value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }

void* find_arrow(void* captured_text, NativeGuiTextActiveNameStorage& name,
    Word length, const void* literal, Word resize_site, Word copy_site,
    Word lookup_site, Word return_site, std::int32_t state,
    NativeGuiTextActiveStorageContext& c, NativeGuiTextActiveStorageAcquired& a) {
    name.header[0] = 0;
    name.header[1] = 0;
    name.preserve_home = 1; // genuine reached PUSH1 home, not caller flag input
    a.active_call_site = resize_site;
    name.resize_entered = true;
    resize_native_string_header_0041dd40_exact(name.header, c.strings, length,
        const_cast<const Word*>(&name.preserve_home), c.feature);
    name.resize_returned = true;
    void* const data = pointer(word(name.header + 1));
    name.captured_data = data;
    if (data) {
        const Word bytes = word(name.header) + 1u;
        a.active_call_site = copy_site;
        name.copy_entered = true;
        (void)copy_native_crt_bytes_00bf7680(data, literal, bytes, c.feature);
        name.copy_returned = true;
    }
    // Native arms state0/1 only after its copy, before the actual lookup.
    a.native_state = state;
    name.lookup_homes[0] = reinterpret_cast<Word>(name.header);
    name.lookup_homes[1] = 0; // Original PUSH0; AA7E00 does not read HOME1
    a.active_call_site = lookup_site;
    name.lookup_entered = true;
    void* const child = find_native_gui_widget_child_storage_00aa7e00(
        captured_text, name.lookup_homes);
    name.captured_child = child;
    name.lookup_returned = true;
    a.native_state = -1; // BEFORE current DATA/LEN/getter/normal return
    a.active_call_site = return_site;
    name.header_return_entered = true;
    return_native_string_header_buffer_0041dd20_exact(name.header, c.strings);
    name.header_returned = true;
    return child; // no subsequent header/DATA access
}

Word slot34(Word captured_profile, NativeGuiTextActiveStorageContext& c) {
    const auto* const captured_table = static_cast<const Word*>(
        c.data.data_at(captured_profile, 0x38));
    return word(captured_table + 0x34 / sizeof(Word));
}
void call_child(void* receiver, Word profile, Word target, Word captured_flags,
    NativeGuiTextActiveChild34Call& call, NativeGuiTextActiveStorageContext& c) {
    call.receiver = receiver;
    call.profile = profile;
    call.target = target;
    call.flags_dword = captured_flags;
    call.entered = true; // before throwing admission or any provider effect
    if (target != 0x00aa8530u)
        throw std::invalid_argument("raw Text active child34 requires its genuine typed target provider");
    c.children.call_visible_34_00aa8530(receiver, profile, target, captured_flags);
    call.returned = true;
}
} // namespace

NativeGuiTextActiveStorageContext::NativeGuiTextActiveStorageContext(
    NativeStringRawPoolContext& actual_strings, const volatile Word& actual_feature,
    const game::GameNativeReadOnlyData& actual_data, NativeGuiTextActiveChild34Bindings& calls)
    : strings(actual_strings), feature(actual_feature), data(actual_data), children(calls),
      left_literal_00d5c764(actual_data.data_at(0x00d5c764, 0x13)),
      right_literal_00d5c750(actual_data.data_at(0x00d5c750, 0x14)) {}

void propagate_native_gui_text_active_storage_00ab87d0(void* actual_text,
    const volatile Word& flags_home, NativeGuiTextActiveStorageContext& c,
    NativeGuiTextActiveStorageFrame& frame, NativeGuiTextActiveStorageAcquired& a) {
    if (a.phase_ != NativeGuiTextActiveStorageAcquired::Phase::fresh || frame.invocation_)
        throw std::logic_error("raw Text active call requires fresh retained frame/acquired storage");
    frame.invocation_ = &a;
    a.phase_ = NativeGuiTextActiveStorageAcquired::Phase::entered;
    void* const captured_text = actual_text; // Native ESI before first allocation
    a.captured_text = captured_text;
    a.flags_home = &flags_home; // address only, not the incoming DWORD value
    try {
        void* const left = find_arrow(captured_text, frame.left, 0x12, c.left_literal_00d5c764,
            0x00ab87ff, 0x00ab881a, 0x00ab882e, 0x00ab8850, 0, c, a);
        void* const right = find_arrow(captured_text, frame.right, 0x13, c.right_literal_00d5c750,
            0x00ab886c, 0x00ab8887, 0x00ab889f, 0x00ab88c1, 1, c, a);
        if (left && right) {
            a.active_call_site = 0x00ab88d5;
            const Word left_profile = word(left);              // AB88D5
            const Word captured_flags = word(&flags_home);     // AB88D7, FULL DWORD once
            a.captured_flags_dword = captured_flags;
            a.flags_captured = true;
            a.active_call_site = 0x00ab88db;
            const Word left_target = slot34(left_profile, c);   // AB88DB
            a.active_call_site = 0x00ab88e1;
            call_child(left, left_profile, left_target, captured_flags, a.left, c);
            a.active_call_site = 0x00ab88e3;
            const Word right_profile = word(right);             // AB88E3, AFTER left call
            a.active_call_site = 0x00ab88e5;
            const Word right_target = slot34(right_profile, c); // AB88E5
            a.active_call_site = 0x00ab88eb;
            call_child(right, right_profile, right_target, captured_flags, a.right, c);
        }
        a.phase_ = NativeGuiTextActiveStorageAcquired::Phase::complete;
    } catch (...) {
        a.failure = std::current_exception();
        a.phase_ = NativeGuiTextActiveStorageAcquired::Phase::failed;
        throw; // retain reached headers/providers; no guessed Native EH cleanup
    }
}
} // namespace bsp
