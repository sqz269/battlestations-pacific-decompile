#include "bsp/voice_line_lifetime.hpp"

#include <cstring>
#include <new>
#include <string>
#include <type_traits>

namespace bsp {
namespace {
static_assert(sizeof(void*) == sizeof(std::uint32_t),
    "The recovered subtitle archive scratch-address seed requires Win32.");
static_assert(std::is_trivially_destructible_v<VoiceClip>,
    "The native clip vector frees storage without releasing borrowed records.");

void release_clip_storage(VoiceClips& clips) noexcept {
    // Invoke the canonical vector's allocator instead of freeing a projected
    // std::vector buffer as native raw memory. Destruction frees its backing
    // storage before a new empty header is constructed in the same location.
    clips.~VoiceClips();
    ::new (static_cast<void*>(&clips)) VoiceClips;
}

struct ClipCleanup {
    VoiceClips& clips;
    bool active{true};
    ~ClipCleanup() { if (active) release_clip_storage(clips); }
};

struct ArchiveText {
    NativeString value;
    NativeStringStorage& storage;
    ~ArchiveText() { value.release_to(storage); }
};
} // namespace

void destroy_voice_line_005b9160(VoiceLine& line, VoiceLineLifetimeHost& host) {
    line.native_vtable_00 = 0x00cf0ed4;
    // Native EH funclet00C72200 also destroys the clip vector if the widget
    // callback throws. No sound stop/release occurs in this destructor.
    ClipCleanup cleanup{line.clips_04};
    if (line.widget_14) {
        // Keep the second load and post-callback clear of005B918F..005B919F.
        auto* widget = static_cast<GuiWidgetTransform*>(line.widget_14);
        if (widget) {
            host.delete_widget_vslot_04(*widget, 1);
            line.widget_14 = nullptr;
        }
        host.current_voice_manager_00e198c4_a4().dirty_60 = 1;
    }
    // ClipCleanup releases the current vector after the last GUI/manager call.
    // The shortcut pointer, slot index and layout words are left untouched.
}

VoiceLine* scalar_delete_voice_line_005b9b80(VoiceLine* line, std::uint8_t flags,
    VoiceLineLifetimeHost& host) {
    VoiceLine* const original = line;
    destroy_voice_line_005b9160(*line, host);
    if (flags & 1u) host.free_line_storage_00bf65ac(original);
    return original;
}

VoiceLine& deserialize_voice_line_005baf30(VoiceLine& line, GuiLuaReader& reader,
    GuiLuaHandleResolver& handles, VoiceSubtitleContext& subtitles,
    NativeStringStorage& strings) {
    line.native_vtable_00 = 0x00cf0ed4;
    // A fresh native line only zeroes +8/+C/+10. Reset the already-constructed
    // canonical empty vector to establish the corresponding empty owner.
    release_clip_storage(line.clips_04);
    ClipCleanup failed_construction{line.clips_04};

    GuiLuaVariant shortcut_field;
    auto shortcut = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(&shortcut_field));
    shortcut_field = gui_lua_field(GuiLuaFieldType::Handle, &shortcut);
    //005BAF67 seeds the destination with the field-pair address, not zero.
    // No has-key test, default value, target validation or ignored-value fixup.
    reader.read_00bd6830(gui_lua_key_by_name("shortcut"), shortcut_field, &handles);
    line.target_2c = static_cast<std::uint32_t>(shortcut);

    ArchiveText text{{}, strings};
    std::string serialized_text;
    reader.read_00bd6830(gui_lua_key_by_name("text"),
        gui_lua_field(GuiLuaFieldType::String, &serialized_text));
    // The canonical serializer projects its native string destination as
    // std::string. Bridge that actual result to the existing pooled owner.
    text.value.resize_0041dd40(strings,
        static_cast<std::uint32_t>(serialized_text.size()), true);
    if (text.value.data()) {
        std::memcpy(text.value.data(), serialized_text.data(), serialized_text.size());
    }
    display_voice_subtitles_005b8510(line, text.value, subtitles, strings);
    line.slot_index_18 = -1;
    failed_construction.active = false;
    return line;
}

} // namespace bsp
