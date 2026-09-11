#include "bsp/panel_publication.hpp"

#include "bsp/native_pooled_string_substring.hpp"

#include <utility>

namespace bsp {
namespace {
struct OwnedString {
    NativeString value;
    NativeStringStorage& strings;
    OwnedString(const NativeString& from, NativeStringStorage& storage) : strings(storage) {
        copy_construct_native_string_header_00426060(&value, &from, strings);
    }
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, strings); }
};
//0044F286 captures the temporary data pointer in EBP before the tree call;
//0044F2CC reloads its length afterward. Do not replace this with a destructor
// that reloads a potentially changed pointer from the local string header.
struct CharacterPairKey {
    NativeString value;
    NativeStringStorage& strings;
    char* captured_data;
    CharacterPairKey(const NativeString& from, NativeStringStorage& storage) : strings(storage) {
        copy_construct_native_string_header_00426060(&value, &from, strings);
        captured_data = value.data();
    }
    ~CharacterPairKey() {
        if (captured_data) strings.release(captured_data, value.length() + 1u);
    }
};
} // namespace

std::int32_t& lookup_panel_character_0044f220(PanelCharacterMap& characters,
    const NativeString& name, NativeStringStorage& strings)
{
    const auto found = characters.lower_bound(name); //0044A730
    if (found != characters.end()
        && !native_string_less_case_insensitive_00443d00(name, found->first))
        return found->second;

    CharacterPairKey pair_key(name, strings);
    //0044F29E..2A0 clears the mapped word.0044ED30/0044DEA0 use the existing
    // tree library;0044CD80 proves the independent node-key deep copy.
    OwnedString node_key(pair_key.value, strings);
    auto result = characters.try_emplace(std::move(node_key.value), std::int32_t{0});
    // try_emplace preserves an existing value/identity on duplicate and leaves
    // the unconsumed key owned by node_key. Tree ABI/allocator details remain
    // the standard-library boundary, not a second hand-written STL port.
    return result.first->second; // temporary cleanup precedes caller value read
}

void clear_panel_character_map(PanelCharacterMap& characters, NativeStringStorage& strings)
{
    while (!characters.empty()) {
        auto node = characters.extract(characters.begin());
        destroy_native_string_header_0041dd20(&node.key(), strings);
    }
}

void hide_published_panel_row_005b6910(VoicePlaybackManager& manager, std::uint32_t index,
    std::uint32_t enabled, const NativeString* text, std::int32_t character,
    VoiceSubtitleHost& ui, const SingletonLifetimeCallbacks& validation)
{
    (void)enabled;
    (void)text;
    (void)character;
    if (index >= manager.rows_94.size())
        validation.invalid_parameter(validation.context);
    //005B6944..56 reloads the CURRENT vector after a returning handler. Invalid
    // unrepaired storage is outside this projection, as it faults natively.
    auto* widget = manager.rows_94[index].widget_00;
    ui.set_visible_vslot_34(*widget, false);
}

void publish_panel_rows_00451c90(PanelSequenceContext& sequence,
    PanelPublicationContext& publication)
{
    auto& entry = lookup_panel_entry_00451920(sequence.owner,
        sequence.owner.current_28, sequence.strings);
    for (std::uint32_t index = 0;; ++index) {
        auto& counted_manager = sequence.host.current_voice_manager_00e198c4_a4();
        //00451D05/JGE is signed, unlike the per-entry slot bounds test.
        if (static_cast<std::int32_t>(index)
            >= static_cast<std::int32_t>(counted_manager.rows_94.size()))
            return;
        if (index < entry.slots_2c.size()) {
            auto& slot = entry.slots_2c[index];
            const bool enabled = slot.active_00 != 0
                && sequence.subtitles.calls.subtitles_enabled_00f88989();
            const auto character = lookup_panel_character_0044f220(
                publication.characters_04, slot.palette_0c, sequence.strings);
            // Native retains slot pointer across lookup, but reloads manager.
            auto& dispatched_manager = sequence.host.current_voice_manager_00e198c4_a4();
            hide_published_panel_row_005b6910(dispatched_manager, index, enabled ? 1u : 0u,
                &slot.text_04, character, sequence.subtitles.calls, publication.validation);
        } else {
            // Fresh zero/zero string -> resize(0,true) is a proven no-call
            // path.005B6910 does not read or retain this borrowed argument.
            NativeString empty;
            auto& dispatched_manager = sequence.host.current_voice_manager_00e198c4_a4();
            hide_published_panel_row_005b6910(dispatched_manager, index, 0, &empty, -1,
                sequence.subtitles.calls, publication.validation);
            destroy_native_string_header_0041dd20(&empty, sequence.strings);
        }
    }
}

} // namespace bsp
