#include "bsp/native_window_focus_dispatch.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
volatile unsigned char& byte(void* p, Word offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, offset));
}
Word current_target(void* receiver, Word slot, NativeWindowFocusContext& context) {
    const Word profile = word(receiver);
    return *static_cast<const volatile Word*>(context.actual_tables.data_at(profile + slot, 4));
}
void validate_child(void* cursor, void* header) {
    if (cursor == pointer(header, 4)) _invalid_parameter_noinfo();
}
}

Word read_native_gui_widget_type_00a9e110(void* actual_widget) noexcept {
    return word(actual_widget, 0x60);
}

void pause_native_movie_decoder_00a4cb60(void* actual_shared_base,
    Word pause, NativeWindowFocusContext& context) {
    auto* const captured = static_cast<BinkHandle*>(pointer(actual_shared_base, 4));
    if (!captured) return;
    const auto low_byte = static_cast<unsigned char>(pause);
    if (low_byte == 0) {
        const Word half = *static_cast<const volatile Word*>(
            context.actual_tables.data_at(0x00ce3800, 4));
        word(actual_shared_base, 0x14) = half;
    }
    (void)context.actual_bink_pause(captured, low_byte);
}

void pause_native_gui_movie_decoder_00aac8a0(void* actual_widget,
    Word pause, NativeWindowFocusContext& context) {
    if (!pointer(actual_widget, 0xf0)) return;
    void* const decoder = pointer(actual_widget, 0xf0);
    if (current_target(decoder, 0x14, context) != 0x00a4cb60)
        throw std::logic_error("Current GUI movie decoder pause target is not reconstructed");
    pause_native_movie_decoder_00a4cb60(decoder, pause, context);
}

void pause_native_gui_movie_children_00aa8e40(void* actual_widget,
    Word pause, NativeWindowFocusContext& context) {
    void* const initial_head = pointer(actual_widget, 0x68);
    void* cursor = pointer(initial_head);
    void* const header = at(actual_widget, 0x64);
    for (;;) {
        void* const end = pointer(header, 4);
        // Native CMP EDI,EDI makes its validation call unreachable.
        if (cursor == end) return;
        validate_child(cursor, header);
        void* const typed_child = pointer(cursor, 8);
        if (current_target(typed_child, 0x5c, context) != 0x00a9e110)
            throw std::logic_error("Current GUI widget type target is not reconstructed");
        if (read_native_gui_widget_type_00a9e110(typed_child) == 10) {
            validate_child(cursor, header);
            pause_native_gui_movie_decoder_00aac8a0(pointer(cursor, 8), pause, context);
        }
        validate_child(cursor, header);
        pause_native_gui_movie_children_00aa8e40(pointer(cursor, 8), pause, context);
        validate_child(cursor, header);
        cursor = pointer(cursor);
    }
}

void pause_native_gui_movie_pages_00aa33a0(void* actual_gui,
    Word pause, NativeWindowFocusContext& context) {
    void* const header = at(actual_gui, 0x14);
    Word cursor = word(header, 4);
    if (cursor > word(header, 8)) _invalid_parameter_noinfo();
    for (;;) {
        const Word end = word(header, 8); // Captured before returning validation.
        if (word(header, 4) > end) _invalid_parameter_noinfo();
        // Native CMP ESI,ESI makes its validation call unreachable.
        if (cursor == end) return;
        if (cursor >= word(header, 8)) _invalid_parameter_noinfo();
        pause_native_gui_movie_children_00aa8e40(pointer(reinterpret_cast<void*>(cursor)), pause, context);
        if (cursor >= word(header, 8)) _invalid_parameter_noinfo();
        cursor += 4;
    }
}

void dirty_native_sound_classes_00a7a3f0(void* actual_sound_manager, Word mask) noexcept {
    Word cursor = word(actual_sound_manager, 0x8c);
    const Word count = word(actual_sound_manager, 0x90);
    const Word end = cursor + count * 4u;
    while (cursor != end) {
        void* const entry = pointer(reinterpret_cast<void*>(cursor));
        void* const descriptor = pointer(entry, 0x44);
        const Word class_id = word(descriptor, 8);
        if (mask & (Word{1} << (class_id & 31u))) byte(entry, 0x14) = 1;
        cursor += 4;
    }
}

void pause_native_sound_manager_00a7a480(void* actual_sound_manager) noexcept {
    if (byte(actual_sound_manager, 0x50) == 0) {
        byte(actual_sound_manager, 0x50) = 1;
        dirty_native_sound_classes_00a7a3f0(actual_sound_manager, 0xffff);
    }
}
void resume_native_sound_manager_00a7a4a0(void* actual_sound_manager) noexcept {
    if (byte(actual_sound_manager, 0x50) != 0) {
        byte(actual_sound_manager, 0x50) = 0;
        dirty_native_sound_classes_00a7a3f0(actual_sound_manager, 0xffff);
    }
}
}
