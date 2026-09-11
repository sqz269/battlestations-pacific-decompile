#include "bsp/panel_sequence.hpp"

#include "bsp/mission_lua_host.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/voice_sequence_update.hpp"

#include <cstring>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Panel sequence reconstruction requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
struct ScopedString {
    NativeString value;
    NativeStringStorage& storage;
    explicit ScopedString(NativeStringStorage& s) : storage(s) {}
    ScopedString(const NativeString& from, NativeStringStorage& s) : storage(s) {
        copy_construct_native_string_header_00426060(&value, &from, storage);
    }
    ~ScopedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
float float_bits(std::uint32_t bits) noexcept {
    float result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}
float subtract(float first, float second) noexcept {
    float result;
    __asm {
        fld first
        fsub second
        fstp result
    }
    return result;
}
float text_hold(std::int32_t count, float per_character, float base) noexcept {
    float result;
    __asm {
        fild count
        fmul per_character
        fadd base
        fstp result
    }
    return result;
}
void reset_current_rows(PanelSequenceContext& context) {
    auto& manager = context.host.current_voice_manager_00e198c4_a4();
    reset_voice_panel_rows_005b9490(manager, context.subtitles, context.strings);
}
void erase_current(PanelSequenceContext& context) {
    const auto where = find_panel_entry_0044ba00(context.owner, context.owner.current_28);
    (void)erase_panel_entry_00451020(context.owner, where, context.strings);
    //0041E350 assigns the empty literal to the existing header.
    context.owner.current_28.resize_0041dd40(context.strings, 0, true);
    if (context.owner.current_28.data())
        std::memcpy(context.owner.current_28.data(), "", context.owner.current_28.length() + 1u);
}
} // namespace

bool native_string_less_case_insensitive_00443d00(const NativeString& left,
    const NativeString& right)
{
    if (!left.length()) return right.length() != 0;
    if (!right.length()) return false;
    return _stricmp(left.data(), right.data()) < 0;
}
bool native_string_not_equal_case_insensitive_00449af0(const NativeString& left,
    const NativeString& right)
{
    if (!left.length()) return right.length() != 0;
    if (!right.length()) return true;
    return _stricmp(left.data(), right.data()) != 0;
}

void destroy_panel_slot_0044b530(PanelSequenceSlot& slot, NativeStringStorage& strings)
{
    destroy_native_string_header_0041dd20(&slot.palette_0c, strings);
    destroy_native_string_header_0041dd20(&slot.text_04, strings);
}
void destroy_panel_entry_0044fff0(PanelSequenceEntry& entry, NativeStringStorage& strings)
{
    entry.native_vtable_00 = 0x00ce4b40;
    for (std::uint32_t i = 0; i < entry.commands_18.size(); ++i) {
        auto& captured_cell = entry.commands_18.at(i);
        auto* command = captured_cell;
        if (command) {
            command->destroy_vslot_00(1);
            captured_cell = nullptr; // captured vector cell; after callback
        }
    }
    // Native captures begin/end once for slot destruction. Reallocation or
    // destruction of that storage from a callback is invalid in both models.
    auto* slot = entry.slots_2c.data();
    const auto count = entry.slots_2c.size();
    for (std::size_t i = 0; i < count; ++i)
        destroy_panel_slot_0044b530(slot[i], strings);
    std::vector<PanelSequenceSlot>{}.swap(entry.slots_2c);
    std::vector<PanelSequenceCommand*>{}.swap(entry.commands_18);
}
void destroy_panel_pair_00450110(NativeString& key, PanelSequenceEntry& entry,
    NativeStringStorage& strings)
{
    destroy_panel_entry_0044fff0(entry, strings);
    destroy_native_string_header_0041dd20(&key, strings);
}

PanelSequenceIterator find_panel_entry_0044ba00(PanelSequenceView owner, const NativeString& key)
{
    auto where = owner.queued_1c.lower_bound(key);
    if (where != owner.queued_1c.end()
        && native_string_less_case_insensitive_00443d00(key, where->first))
        where = owner.queued_1c.end();
    return {&owner.queued_1c, where};
}
PanelSequenceIterator erase_panel_entry_00451020(PanelSequenceView owner,
    PanelSequenceIterator where, NativeStringStorage& strings)
{
    // Native diagnoses end() with out_of_range, then advances the by-value
    // iterator before unlink/rebalance. Standard containers own that library
    // work; this is not a reimplementation of MSVC's red-black tree.
    if (where.owner != &owner.queued_1c || where.position == owner.queued_1c.end())
        throw std::out_of_range("invalid map/set<T> iterator");
    const auto next = std::next(where.position);
    {
        auto node = owner.queued_1c.extract(where.position);
        destroy_panel_pair_00450110(node.key(), node.mapped(), strings);
    } // native free is BEFORE count+8 reread/decrement, including reentry
    if (owner.queued_count_24) --owner.queued_count_24;
    return {&owner.queued_1c, next};
}
PanelSequenceEntry& lookup_panel_entry_00451920(PanelSequenceView owner,
    const NativeString& key, NativeStringStorage& strings)
{
    const auto lower = owner.queued_1c.lower_bound(key);
    if (lower != owner.queued_1c.end()
        && !native_string_less_case_insensitive_00443d00(key, lower->first))
        return lower->second;
    // The native constructs a zero/default entry, pair temporary and node.
    // Preserve both key deep copies and temporary cleanup. Vector allocators,
    // debug iterator proxies and internal tree node ABI are library boundaries.
    ScopedString pair_key(key, strings);
    ScopedString node_key(pair_key.value, strings);
    auto where = owner.queued_1c.emplace_hint(lower, std::move(node_key.value), PanelSequenceEntry{});
    ++owner.queued_count_24;
    return where->second;
}

void select_panel_sequence_0044c390(PanelSequenceContext& context)
{
    auto owner = context.owner;
    // The first local string is zero/zero and resize(0) returns immediately;
    // its comparison and destructor reduce to this length gate without calls.
    if (owner.current_28.length()) return;
    ScopedString chosen(context.strings);
    float best_priority = -1.0e10f; //00CE4ADC = D01502F9
    std::optional<float> best_tie;
    for (auto item = owner.queued_1c.begin(); item != owner.queued_1c.end(); ++item) {
        const float rank = item->second.priority_04;
        bool choose = rank > best_priority;
        if (!choose && rank == best_priority) {
            const float previous_tie = best_tie ? *best_tie
                : float_bits(context.selection_scratch_18);
            choose = previous_tie > item->second.tie_08;
        }
        // Ordered comparisons match the FCOMI/FUCOMIP parity gates for quiet
        // NaNs. Native signaling-NaN exception/status behavior is not promised.
        if (choose) {
            chosen.value.copy_from_00be0a30_fragment(context.strings, item->first);
            best_priority = item->second.priority_04; // reload after allocation
            best_tie = item->second.tie_08;
        }
    }
    owner.current_28.copy_from_00be0a30_fragment(context.strings, chosen.value);
}

void reset_voice_panel_row_005b8a00(VoicePlaybackManager& manager, std::uint32_t index,
    VoiceSubtitleContext& subtitles, NativeStringStorage& strings)
{
    (void)manager.rows_94.at(index);
    ScopedString empty(strings);
    // Native initializes local length/data=0 then resize(0,true); no allocator
    // can run, so its display branch is unreachable. Preserve GUI reloads.
    auto& calls = subtitles.calls;
    calls.set_visible_vslot_34(*manager.group_30, false);
    calls.set_visible_vslot_34(*manager.background_3c, false);
    calls.set_visible_vslot_34(*manager.decoration_40, false);
    calls.set_literal_00abbe50(*manager.text_34, "", true);
    calls.set_literal_00abbe50(*manager.text_38, "", true);
}
void reset_voice_panel_rows_005b9490(VoicePlaybackManager& manager,
    VoiceSubtitleContext& subtitles, NativeStringStorage& strings)
{
    for (std::uint32_t index = 0; index < manager.rows_94.size(); ++index)
        reset_voice_panel_row_005b8a00(manager, index, subtitles, strings);
}

void begin_panel_voice_record_005b94d0(ScheduledVoiceContext& scheduled,
    std::uint32_t index, const VoiceClipRecord& record, float delay,
    VoiceSubtitleContext& subtitles)
{
    auto& manager = scheduled.manager;
    manager.selected_row_84 = index;
    auto& row = manager.rows_94.at(index);
    row.fade_remaining_0c = manager.initial_78;
    row.color_10 = manager.base_color_44;
    admit_scheduled_voice_record_005b93d0(scheduled, record);
    manager.delay_88 = delay;
    const auto& keys = scheduled.records.timed_keys_34(record);
    if (!keys.empty()) {
        manager.hold_8c = keys.back().end_14;
        return;
    }
    // Same GUI sequence and constants as005B6710, force byte false.
    set_voice_panel_text_005b6710(manager, record.text_00, false, subtitles);
    if (poll_scheduled_voice_005b9420(scheduled)) {
        manager.hold_8c = 0.0f;
    } else {
        const auto count = static_cast<std::int32_t>(manager.text_34->text.size());
        manager.hold_8c = text_hold(count, manager.per_character_7c, manager.base_80);
    }
}
void request_panel_voice_004483f0(PanelSequenceContext& context,
    PanelSequenceEntry& entry, std::uint32_t slot, const NativeString& name)
{
    const float delay = entry.elapsed_0c; // captured before either game call
    entry.selected_slot_3c = slot;
    const auto& record = context.host.resolve_message_00705e00(name, 0);
    auto& manager = context.host.current_voice_manager_00e198c4_a4();
    auto scheduled = context.host.scheduled_context(manager);
    begin_panel_voice_record_005b94d0(scheduled, slot, record, delay, context.subtitles);
}

void consume_panel_commands_00452360(PanelSequenceContext& context)
{
    auto& entry = lookup_panel_entry_00451920(context.owner,
        context.owner.current_28, context.strings);
    auto* command = entry.commands_18.at(entry.cursor_24); // native checks even initially empty
    for (;;) {
        if (command->kind_vslot_04() != 1 && command->kind_vslot_04() != 2
            && command->kind_vslot_04() != 4) {
            context.host.publish_panel_rows_00451c90(context.owner);
            return;
        }
        // Repeated virtual calls are intentional; do not cache the kind.
        if (command->kind_vslot_04() == 1) {
            if (entry.slots_2c.size() <= command->word_04)
                entry.slots_2c.resize(command->word_04 + 1u);
            entry.slots_2c.at(command->word_04).active_00 = 1;
            entry.slots_2c.at(command->word_04).text_04.copy_from_00be0a30_fragment(
                context.strings, command->string_08);
            entry.slots_2c.at(command->word_04).palette_0c.copy_from_00be0a30_fragment(
                context.strings, command->string_10);
            const auto index = command->word_04;
            auto& manager = context.host.current_voice_manager_00e198c4_a4();
            reset_voice_panel_row_005b8a00(manager, index, context.subtitles, context.strings);
        } else if (command->kind_vslot_04() == 2) {
            const auto index = command->word_04;
            if (index < entry.slots_2c.size()) entry.slots_2c.at(index).active_00 = 0;
        } else if (command->kind_vslot_04() == 4) {
            //004525B5 captures game owner before guard store. Accessors only
            // project real storage; no synthetic callbacks are introduced.
            auto& lua = context.host.current_mission_lua_1a08();
            context.host.callback_guard_00e17bfa() = 1;
            const auto& name = command->string_08;
            const std::string text = name.length() ? std::string(name.data(), name.length()) : std::string{};
            (void)call_named_entry_point_threadsafe(lua, text, {});
            context.host.callback_guard_00e17bfa() = 0; // not an RAII restoration
        }
        ++entry.cursor_24; // reread current value AFTER callbacks
        if (entry.cursor_24 >= entry.commands_18.size()) return;
        command = entry.commands_18.at(entry.cursor_24);
    }
}
void step_panel_sequence_00452740(PanelSequenceContext& context)
{
    auto& entry = lookup_panel_entry_00451920(context.owner,
        context.owner.current_28, context.strings);
    consume_panel_commands_00452360(context); // may select a different entry in a callback
    if (entry.cursor_24 >= entry.commands_18.size()) return;
    auto* command = entry.commands_18.at(entry.cursor_24);
    if (command->kind_vslot_04() == 0) {
        request_panel_voice_004483f0(context, entry, command->word_04, command->string_08);
        ++entry.cursor_24;
        return;
    }
    if (command->kind_vslot_04() == 3) {
        const float delay = subtract(command->time_08, entry.elapsed_0c);
        context.host.current_voice_manager_00e198c4_a4().delay_88 = delay;
    }
    ++entry.cursor_24;
}
bool advance_panel_sequence_004527f0(PanelSequenceContext& context)
{
    auto owner = context.owner;
    const auto state = owner.state_34;
    if (state == 0) {
        if (!owner.queued_count_24) return false;
    } else if (state == 2) {
        reset_current_rows(context);
    } else if (state == 1) {
        auto& entry = lookup_panel_entry_00451920(owner, owner.current_28, context.strings);
        if (entry.cursor_24 >= entry.commands_18.size()) {
            erase_current(context);
        } else if (entry.erase_38) {
            erase_current(context);
            return true; // state remains1, no selection/reset this turn
        }
        ScopedString previous(owner.current_28, context.strings); // AFTER clearing on erase
        select_panel_sequence_0044c390(context);
        if (!owner.current_28.length()) {
            reset_current_rows(context);
            owner.state_34 = 0;
            return false;
        }
        if (!previous.value.length()) {
            reset_current_rows(context);
            auto& selected = lookup_panel_entry_00451920(owner, owner.current_28, context.strings);
            if (static_cast<std::int32_t>(selected.cursor_24) > 0)
                consume_panel_commands_00452360(context);
            else
                step_panel_sequence_00452740(context);
            return true;
        }
        if (native_string_not_equal_case_insensitive_00449af0(previous.value, owner.current_28))
            owner.state_34 = 2;
        else
            step_panel_sequence_00452740(context);
        return true;
    } else {
        return false;
    }
    select_panel_sequence_0044c390(context);
    step_panel_sequence_00452740(context);
    owner.state_34 = 1; // AFTER nested callbacks, including reset in state2
    return true;
}

} // namespace bsp
