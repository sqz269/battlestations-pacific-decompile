// cc15-ob1: the MessageManager [game+21DCh]. See include/bsp/game_hosts_message_manager.hpp
// and docs/CC15_OBJECT_MESSAGEMANAGER.md.

#include "bsp/game_hosts_message_manager.hpp"

#include "bsp/band_0085_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/message_record_resolver.hpp"
#include "bsp/native_string.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_mounts.hpp"

#include <windows.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
#include <utility>

namespace bsp::game {

namespace {

namespace seg = bsp::engine_seg11_35_66_cc13;

// 00E08FCC: the four voice directory templates 00704590 walks (00CFD274, 00CFD250,
// 00CFD22C, 00CFD214).
constexpr std::array<const char*, 4> kVoiceDirectoryTemplates = {
    "sound/messages/LANGUAGE/streamed_dialogs/",
    "sound/messages/LANGUAGE/english/",
    "sound/messages/LANGUAGE/japanese/",
    "sound/messages/other/",
};

// The record map's node is 58h bytes (nil byte +55h, 00702480); the string set's 18h
// (nil byte +15h, 004C26B0). Only the heads are bought: the host keeps the entries in
// the containers below, and the heads carry the constructor's self links.
constexpr std::size_t kRecordNodeBytes = 0x58;
constexpr std::size_t kStringSetNodeBytes = 0x18;

struct CaseInsensitiveLess {
    bool operator()(const std::string& left, const std::string& right) const noexcept {
        return _stricmp(left.c_str(), right.c_str()) < 0;
    }
};

void assign_text(NativeString& to, NativeStringStorage& strings, const std::string& text) {
    to.resize_0041dd40(strings, static_cast<std::uint32_t>(text.size()), false);
    if (!text.empty() && to.data() != nullptr) std::memcpy(to.data(), text.data(), text.size());
}

std::string text_of(const NativeString& s) {
    return s.data() == nullptr ? std::string() : std::string(s.data(), s.length());
}

// The six-slot reader 004425C0 builds over the script's globals (vtable 00CE44FC), as
// bsp::GuiLuaReader models it. GuiLuaReader writes std::string and bool fields; the
// message bodies pass NativeString and byte fields, so those two tags are staged.
class MessageScriptVisitor final : public seg::PropertyVisitor {
public:
    MessageScriptVisitor(GuiLuaHost& lua, const GuiLuaRef& root, const bool& sse2,
        NativeStringStorage& strings)
        : reader(lua, root, sse2), strings_(strings) {}

    void enter_04(const GuiLuaVariant& key) override { reader.enter_00bd8e20(key); }
    void leave_08() override { reader.leave_00bd7a20(); }
    bool read_or_default_0c(const GuiLuaVariant& key, const GuiLuaVariant& field,
        const GuiLuaVariant& fallback) override {
        return staged(field, [&](const GuiLuaVariant& f) {
            return reader.read_or_default_00bd68d0(key, f, fallback);
        });
    }
    bool read_10(const GuiLuaVariant& key, const GuiLuaVariant& field) override {
        return staged(field, [&](const GuiLuaVariant& f) { return reader.read_00bd6830(key, f); });
    }
    bool has_key_14(const GuiLuaVariant& key) override { return reader.has_key_00bd5eb0(key); }

    GuiLuaReader reader;

private:
    template <typename Read>
    bool staged(const GuiLuaVariant& field, Read read) {
        if (field.tag == static_cast<std::int32_t>(GuiLuaFieldType::String)) {
            std::string text;
            const bool wrote = read(gui_lua_field(GuiLuaFieldType::String, &text));
            if (wrote) assign_text(*static_cast<NativeString*>(field.value.pointer), strings_, text);
            return wrote;
        }
        if (field.tag == static_cast<std::int32_t>(GuiLuaFieldType::Bool)) {
            auto* byte = static_cast<std::uint8_t*>(field.value.pointer);
            bool value = *byte != 0;
            const bool wrote = read(gui_lua_field(GuiLuaFieldType::Bool, &value));
            if (wrote) *byte = value ? 1u : 0u;
            return wrote;
        }
        return read(field);
    }
    NativeStringStorage& strings_;
};

// cc15-msg2: the services 00705E00's miss arm reaches through VoiceLineHost. The created
// record holds no sound references (00A83FD0 is a seam), so only the current-manager
// read is reached: [[00E198C4]+A4h], the HUD's narrative screen (slot 33h), whose
// +7Ch / +80h 005BB130 loads from DialogGlobals.lua. The screen itself is not hosted;
// `screen` carries those two words and nothing else is read from it.
class RecordMissPlayback final : public VoiceLineHost {
public:
    VoicePlaybackManager screen;

    VoicePlaybackManager& current_voice_manager_00e198c4_a4() override { return screen; }

    float mission_clock_00f876a4() override { throw unreached(); }
    bool sound_completed_vslot_0c(void*) override { throw unreached(); }
    float sound_duration_00a81860(void*) override { throw unreached(); }
    void stop_sound_vslot_08(void*, std::uint32_t) override { throw unreached(); }
    void release_reference(void*) noexcept override {}  // no reference is ever held
    void stop_auxiliary_vslot_08(void*, std::uint32_t) override { throw unreached(); }
    void reset_auxiliary_0054d510(void*&, void*) override { throw unreached(); }
    void set_sound_flag_00a7d120(std::uint32_t) override { throw unreached(); }
    bool alternate_playing_00a77730() override { throw unreached(); }
    std::int32_t local_side_18cc_18ec() override { throw unreached(); }
    std::int32_t speaker_side_54(void*) override { throw unreached(); }
    bool speaker_kind_vslot_5c(void*, std::uint32_t) override { throw unreached(); }
    void notify_005a2650(std::uint32_t, void*, std::uint32_t) override { throw unreached(); }
    VoiceLine* allocate_line_00bf681b(std::uint32_t) override { throw unreached(); }
    VoiceLineNode& allocate_node_00bf681b(std::uint32_t) override { throw unreached(); }
    void retain_reference(void*) override { throw unreached(); }
    std::uint32_t invalid_target_00e188d8() override { throw unreached(); }
    bool target_valid_00645160(std::uint32_t, bool) override { throw unreached(); }
    VoiceSubtitleContext& subtitle_context() override { throw unreached(); }
    VoiceSlotStartContext& slot_start_context() override { throw unreached(); }
    void log_004254b0(const char*, const char*) override { throw unreached(); }
    std::array<float, 3> camera_position_00b6db70() override { throw unreached(); }
    std::array<float, 3> entity_position_00414db0(void*) override { throw unreached(); }
    double vector_length_0042b2f0(const std::array<float, 3>&) override { throw unreached(); }
    float global_sound_level_6c() override { throw unreached(); }
    VoiceFadeBindings& fade_bindings() override { throw unreached(); }

private:
    static std::logic_error unreached() {
        return std::logic_error("00705E00 miss arm: a voice service the created record never reaches");
    }
};

// cc15-msg2: 008D57A0's globals, bound by the application (bind_voice_language_settings).
const std::vector<LanguageEntry>* g_voice_catalog = nullptr;  // [00F88974], count [00F88978]
const int* g_voice_language_index = nullptr;                  // [00F88980]+4h

// 008D57A0 on [00F88980], read afresh at every call as 00704590 calls it:
// - "english" when [00F88978] < index (JL) or the row's voice_dir +10h is empty;
// - "authentic" when 007F8890 says the profile owns DL_Content_0000062;
// - else the row's voice_dir +14h ("" at 00F88A3C when null).
// The profile's content list (game+720h, written by the marketplace enumeration) is
// not hosted, so ownership is false here. With the installation's single row
// (lockit/englishauthentic.lng: voice_dir authentic) both arms answer "authentic".
// An index the image would read outside the table throws instead.
std::string voice_language_name_from_settings() {
    const std::vector<LanguageEntry>& catalog = *g_voice_catalog;
    std::vector<cc13_band0085::VoiceLanguageRow> rows(catalog.size());
    for (std::size_t i = 0; i != catalog.size(); ++i) {
        rows[i].has_voice_10 = static_cast<std::uint32_t>(catalog[i].voice_dir.size());
        rows[i].voice_folder_14 = catalog[i].voice_dir.empty() ? nullptr : catalog[i].voice_dir.c_str();
    }
    const auto name = cc13_band0085::voice_language_name_008d57a0(rows.data(),
        static_cast<std::int32_t>(rows.size()), *g_voice_language_index, false);
    if (!name) throw std::logic_error("008D57A0: language index outside [00F88974]");
    return std::string(*name);
}

// One open script: 00B66BD0 owner, 00B6A020 open(1), the run, 00B67980 globals and the
// 004425C0 reader; 00B669A0 closes it.
struct ScriptFrame {
    GuiLua51Host lua;
    std::unique_ptr<MessageScriptVisitor> visitor;
};

}  // namespace

struct GameMessageManagerHost::Impl final : seg::MessageManagerCalls {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, std::string language)
        : log(log_in), vfs(vfs_in), voice_language(std::move(language)),
          sse2(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE) {}

    ~Impl() override {
        clear_records();
        std::free(record_head);
        std::free(string_set_head);
    }

    GameHostLog& log;
    GameVfsHost& vfs;
    std::string voice_language;
    bool voice_language_logged{false};
    bool sse2;  // 0109EEA4, the CRT's double-to-int selector, queried as decals.lua does
    std::unique_ptr<seg::MessageManagerLayout> layout;  // the 70h block, null when not built
    MessageRecordStore records;                         // tree +4h
    std::set<std::string, CaseInsensitiveLess> loaded_maps;  // string set +64h
    void* record_head{nullptr};
    void* string_set_head{nullptr};
    std::vector<std::unique_ptr<ScriptFrame>> frames;
    MessageManagerSummary summary;

    void clear_records() noexcept {
        NativeStringStorage& s = crt_string_storage();
        for (auto& [key, record] : records) {
            // 00704060 order: the timed keys, the four references (always null here,
            // 00A83FD0 being a seam), then the three strings.
            destroy_message_keys_00703b90(record.timed_keys_34, s);
            record.alternate_name_18.release_to(s);
            record.auxiliary_0c.release_to(s);
            record.text_00.release_to(s);
            const_cast<NativeString&>(key).release_to(s);
        }
        records.clear();
    }

    [[noreturn]] static void unreached(const char* what) {
        throw std::logic_error(std::string("MessageManager host: unreached ") + what);
    }

    // ---- MessageManagerCalls -------------------------------------------------
    NativeStringStorage& strings() override { return crt_string_storage(); }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("MessageManager: 00BF6713 invalid parameter");
    }
    seg::NativeTreeLinks* buy_record_head_00702480() override {
        record_head = std::calloc(1, kRecordNodeBytes);
        if (record_head == nullptr) throw std::bad_alloc();
        return static_cast<seg::NativeTreeLinks*>(record_head);
    }
    seg::NativeTreeLinks* buy_string_set_head_004c26b0() override {
        string_set_head = std::calloc(1, kStringSetNodeBytes);
        if (string_set_head == nullptr) throw std::bad_alloc();
        return static_cast<seg::NativeTreeLinks*>(string_set_head);
    }
    void erase_string_set_004d1a50(seg::NativeTreeHeader& set) override {
        loaded_maps.clear();
        set.size = 0;
    }
    void erase_record_map_00705d30(seg::NativeTreeHeader& map) override {
        clear_records();
        map.size = 0;
    }
    void destroy_string_range_00432050(void*, void*) override {
        // +14h..+1Ch: no writer of the actor-name vector is known; it stays null.
        unreached("00432050 (the +14h name vector is never filled)");
    }
    void free_00bf65ac(void* block) override {
        if (block == record_head) record_head = nullptr;
        if (block == string_set_head) string_set_head = nullptr;
        std::free(block);
    }
    VoiceClipRecord& lookup_record_00705c50(const NativeString& name) override {
        // 00705C50 operator[]: an existing node is returned; otherwise a record built by
        // 00703FF0 is inserted at the hint. Its +08h / +14h come from the caller's
        // uninitialized stack in the image; zero here, and 00704590 writes both next.
        const auto lower = lower_bound_message_record_00702300(records, name);
        if (lower != records.end() && !PanelSequenceNameLess{}(name, lower->first)) return lower->second;
        NativeString key;
        key.copy_from_00be0a30_fragment(crt_string_storage(), name);
        auto where = records.emplace_hint(lower, std::move(key), VoiceClipRecord{});
        construct_message_record_00703ff0(where->second);
        if (layout) layout->records_04.size = static_cast<std::uint32_t>(records.size());
        return where->second;
    }
    const NativeString* lower_bound_key_00702300(const NativeString& name) override {
        const auto lower = lower_bound_message_record_00702300(records, name);
        return lower == records.end() ? nullptr : &lower->first;
    }
    const NativeString& record_key_007030d0(const VoiceClipRecord* record) override {
        for (const auto& [key, value] : records) {
            if (&value == record) return key;
        }
        unreached("007030D0 for a record outside the map");
    }
    const VoiceClipRecord* resolve_record_00705e00(const NativeString& name) override {
        const MessageRecordIterator found = find_message_record_00702910(records, name);
        if (found.position != records.end()) {
            ++summary.resolve_hits;
            return &found.position->second;
        }
        ++summary.resolve_misses;
        if constexpr (kMessageRecordMissCc15Bound) {
            // 00705E00's miss arm, the typed body: 00705C50 inserts a 00703FF0 record at
            // the hint, then text = name, sound id -1, one timed key with text "<name>",
            // flag 1, start 0 and end = len * [+7Ch] + [+80h] on the narrative screen.
            // The 00705C50 temporary's +08h / +14h are the caller's uninitialized stack
            // in the image; zero here, as lookup_record_00705c50 above does. +08h is then
            // overwritten with -1, so only +14h (the alternate flag) carries that choice.
            // 00976B00 passes 0 as the unused second word.
            MessageRecordResolverContext context{crt_string_storage(), miss_playback(), {}};
            VoiceClipRecord& record = resolve_message_record_00705e00(records, name, 0, context);
            if (layout) layout->records_04.size = static_cast<std::uint32_t>(records.size());
            ++summary.records_created;
            return &record;
        }
        return nullptr;
    }
    std::vector<std::string> loaded_map_keys() override {
        return {loaded_maps.begin(), loaded_maps.end()};
    }
    bool loaded_map_contains(const std::string& key) override { return loaded_maps.count(key) != 0; }
    void loaded_map_erase_004cf8a0(const std::string& key) override {
        loaded_maps.erase(key);
        if (layout) layout->loaded_maps_64.size = static_cast<std::uint32_t>(loaded_maps.size());
    }
    void loaded_map_insert(const std::string& key) override {
        loaded_maps.insert(key);
        if (layout) layout->loaded_maps_64.size = static_cast<std::uint32_t>(loaded_maps.size());
    }
    // The base file of a script from the VFS (00BDF4C0 / 00BDF310); error on failure.
    std::string read_script_text(const std::string& path, std::string& error) {
        std::string text;
        std::string resolved = path;
        if (!vfs.ready() || !resolve_existing_resource_00bdf4c0_fragment(
                vfs.context(), vfs.search_registrations(), resolved)) {
            error = "cannot resolve " + path;
        } else {
            VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(vfs.context(), resolved, 2);
            if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
                error = opened.error.empty() ? "cannot open " + path : opened.error;
            } else {
                text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
                    static_cast<std::size_t>(opened.stream->size_00bef600()));
            }
        }
        return text;
    }

    // cc15-msg2: the narrative screen's timing pair for 00705E00's miss arm, loaded once
    // the way 005BB130 loads it (005BB80B..005BB901): a Lua owner runs
    // Scripts/datatables/DialogGlobals.lua, and +7Ch = TempCharWait, +80h = TempWaitBase,
    // each GetNumber FSTP'd to a float. As for Messages.lua, only the base file is run
    // (00BDEF90's override files are a seam). A global that is not a number reads 0,
    // which is what lua_tonumber answers.
    RecordMissPlayback& miss_playback() {
        if (miss_playback_) return *miss_playback_;
        auto playback = std::make_unique<RecordMissPlayback>();
        const char* const path = "Scripts/datatables/DialogGlobals.lua";
        std::string error;
        const std::string text = read_script_text(path, error);
        GuiLua51Host lua;
        if (error.empty() && !lua.execute_archive(text, error) && error.empty()) {
            error = "the script did not run";
        }
        if (!error.empty()) throw std::runtime_error(std::string("00705E00 miss arm: ") + path + ": " + error);
        const GuiLuaRef globals = lua.globals();
        auto number = [&](const char* name) {
            const GuiLuaRef value = lua.get_by_name(globals, name);
            const double out = lua.type_of(value) == GuiLuaType::Number ? lua.to_number(value) : 0.0;
            lua.release(value);
            return static_cast<float>(out);
        };
        playback->screen.per_character_7c = number("TempCharWait");
        playback->screen.base_80 = number("TempWaitBase");
        lua.release(globals);
        summary.char_wait_7c = playback->screen.per_character_7c;
        summary.wait_base_80 = playback->screen.base_80;
        log.notef("message manager: record-miss timing from %s: TempCharWait=%.9g TempWaitBase=%.9g "
            "(005BB130's narrative screen +7Ch/+80h)", path,
            static_cast<double>(summary.char_wait_7c), static_cast<double>(summary.wait_base_80));
        miss_playback_ = std::move(playback);
        return *miss_playback_;
    }
    std::unique_ptr<RecordMissPlayback> miss_playback_;

    seg::PropertyVisitor* open_script_reader(const std::string& path) override {
        // 00706160 runs the path through 00B69D40 in a state of its own. As for
        // Warnings.lua (009870A0) and decals.lua, the base file is read from the VFS
        // and run by the repository's Lua 5.1.1; 00BDEF90's override files are not run.
        std::string error;
        const std::string text = read_script_text(path, error);
        auto frame = std::make_unique<ScriptFrame>();
        if (error.empty() && !frame->lua.execute_archive(text, error) && error.empty()) {
            error = "the script did not run";
        }
        if (!error.empty()) {
            // The image's reader would walk nils over an empty globals table; the
            // host reports instead (00706160 then enters "Messages" regardless).
            ++summary.scripts_failed;
            summary.script_error = error;
            log.notef("message manager: %s not read: %s", path.c_str(), error.c_str());
            return nullptr;
        }
        ++summary.scripts_opened;
        frame->visitor = std::make_unique<MessageScriptVisitor>(frame->lua, frame->lua.globals(),
            sse2, crt_string_storage());
        seg::PropertyVisitor* visitor = frame->visitor.get();
        frames.push_back(std::move(frame));
        return visitor;
    }
    void close_script_reader(seg::PropertyVisitor* reader) override {
        if (frames.empty() || frames.back()->visitor.get() != reader) unreached("reader close order");
        frames.pop_back();
    }
    std::vector<GuiLuaVariant> enumerate_keys_00bd5f50(seg::PropertyVisitor& reader) override {
        auto& visitor = static_cast<MessageScriptVisitor&>(reader);
        auto keys = std::make_unique<GuiLuaKeyArray>();
        gui_lua_clear_key_array_00aaa710(*keys);
        const std::size_t count = visitor.reader.enumerate_keys_00bd5f50(*keys);
        return {keys->keys, keys->keys + count};
    }
    void report_loading_progress_0057c1a0(float) override {}
    void retain_reference(void*) override { unreached("a sound bank reference (00A83FD0 is a seam)"); }
    void release_reference(void*) override { unreached("a sound bank reference (00A83FD0 is a seam)"); }
    bool file_store_exists_vslot8(const std::string& path) override {
        const bool present = vfs.exists(path);  // 00BDD440
        ++(present ? summary.voice_probe_hits : summary.voice_probe_misses);
        return present;
    }
    bool file_store_exists_vslotc(const std::string&) override {
        // 00BDD520 BSP_VFS_ExistsInFlaggedMounts: not reachable through GameVfsHost.
        ++summary.streamed_probes_not_evaluated;
        return false;
    }
    void* load_sound_bank_00a83fd0(const std::string&) override {
        // [00F8BBE8] 00A83FD0 loads the .fsb through FMOD's sample cache; not hosted.
        ++summary.sound_banks_not_loaded;
        return nullptr;
    }
    std::string language_name_008d57a0() override {
        if constexpr (kMessageVoiceLanguageCc15Bound) {
            if (g_voice_catalog != nullptr) {
                std::string name = voice_language_name_from_settings();
                if (!voice_language_logged) {
                    // The load line's language= is still the lockit name the frame host holds.
                    voice_language_logged = true;
                    log.notef("message manager: 008D57A0 voice language \"%s\" (settings+4h index %d of "
                        "%zu catalog rows; lockit name \"%s\")", name.c_str(), *g_voice_language_index,
                        g_voice_catalog->size(), voice_language.c_str());
                }
                return name;
            }
        }
        return voice_language;
    }
    const char* voice_directory_template_00e08fcc(std::size_t slot) override {
        return slot < kVoiceDirectoryTemplates.size() ? kVoiceDirectoryTemplates[slot] : nullptr;
    }
    // 00706760's profile helpers. The host keeps no profile-hints singleton (004C1E90)
    // or profile content list (007F8890), so mode 2's language sweep and the
    // "englishauthentic" bank name are not taken; the FileBlock is not built.
    std::int32_t profile_hints_mode_plus8() override { return 0; }
    std::vector<std::string> collect_language_names() override { return {}; }
    void select_language_by_name(const std::string&) override {}
    bool profile_owns_content(const std::string&) override { return false; }
    void* construct_file_block_00be0a30(const std::string&) override { return nullptr; }
    void delete_file_block(void*) override {}
    std::int32_t atol_00bf8417(const char* text) override { return std::atol(text); }
    void resize_timed_keys_00704190(VoiceTimedKeys& keys, std::uint32_t count) override {
        resize_message_keys_00704190(keys, count, VoiceTimedKey{}, crt_string_storage());
    }
};

GameMessageManagerHost::GameMessageManagerHost(GameHostLog& log, GameVfsHost& vfs,
    std::string voice_language)
    : impl_(std::make_unique<Impl>(log, vfs, std::move(voice_language))) {}

void GameMessageManagerHost::bind_voice_language_settings(
    const std::vector<LanguageEntry>& catalog, const int& language_index_04) {
    g_voice_catalog = &catalog;
    g_voice_language_index = &language_index_04;
}

GameMessageManagerHost::~GameMessageManagerHost() {
    if (impl_->layout) destruct_007071f0();
}

void GameMessageManagerHost::construct_00707480() {
    if (impl_->layout) destruct_007071f0();
    impl_->layout = std::make_unique<seg::MessageManagerLayout>();
    seg::construct_message_manager_00707480(*impl_->layout, *impl_);
    ++impl_->summary.constructed;
    impl_->log.implemented("MessageManager::construct", "00707480");
}

void GameMessageManagerHost::destruct_007071f0() {
    if (!impl_->layout) return;
    seg::destruct_message_manager_007071f0(*impl_->layout, *impl_);
    impl_->layout.reset();
    ++impl_->summary.destructed;
    impl_->log.implemented("MessageManager::destruct", "007071f0");
}

bool GameMessageManagerHost::constructed() const noexcept { return impl_->layout != nullptr; }

void GameMessageManagerHost::load_messages_007065e0(bool load, const MessageManagerLoadInputs& inputs) {
    if (!impl_->layout) throw std::logic_error("007065E0 before 00707480");
    seg::MessageLoadOptions options{};
    options.local_side_kind = inputs.local_side_kind;
    options.filters_enabled = inputs.filters_enabled;
    options.ids_1068 = {inputs.ids_1068.data(), static_cast<std::int32_t>(inputs.ids_1068.size())};
    options.ids_1074 = {inputs.ids_1074.data(), static_cast<std::int32_t>(inputs.ids_1074.size())};
    options.ids_1080 = {inputs.ids_1080.data(), static_cast<std::int32_t>(inputs.ids_1080.size())};
    const std::size_t before = impl_->records.size();
    seg::load_messages_007065e0(*impl_->layout, options, load, *impl_);
    ++(load ? impl_->summary.loads : impl_->summary.unloads);
    impl_->summary.records_added += impl_->records.size() - before;
    impl_->log.implemented("MessageManager::load_messages", "007065e0");
}

void GameMessageManagerHost::load_message_map_00706760(const std::string& name, std::int32_t side) {
    if (!impl_->layout) throw std::logic_error("00706760 before 00707480");
    seg::load_message_map_00706760(*impl_, name, side);
}

void GameMessageManagerHost::unload_message_map_00706fc0(const std::string& name, std::int32_t side) {
    if (!impl_->layout) throw std::logic_error("00706FC0 before 00707480");
    seg::unload_message_map_00706fc0(*impl_, name, side);
}

const VoiceClipRecord* GameMessageManagerHost::resolve_record_00705e00(const std::string& name) {
    if (!impl_->layout) throw std::logic_error("00705E00 before 00707480");
    NativeString key;
    assign_text(key, crt_string_storage(), name);
    const unsigned long long created = impl_->summary.records_created;
    const VoiceClipRecord* record = impl_->resolve_record_00705e00(key);
    key.release_to(crt_string_storage());
    if (impl_->summary.records_created != created) {
        // A name is created once; later lookups of it hit the new record.
        const VoiceTimedKey& timed = record->timed_keys_34.front();
        impl_->log.notef("message manager record created: \"%s\" key=\"%s\" end=%.9g (00705E00 miss arm)",
            name.c_str(), text_of(timed.text_00).c_str(), static_cast<double>(timed.end_14));
    }
    return record;
}

bool GameMessageManagerHost::record_map_contains_00702300(const std::string& key_text) {
    if (!impl_->layout) throw std::logic_error("00702300 before 00707480");
    NativeString key;
    assign_text(key, crt_string_storage(), key_text);
    const MessageRecordIterator found = find_message_record_00702910(impl_->records, key);
    key.release_to(crt_string_storage());
    ++impl_->summary.key_tests;
    const bool hit = found.position != impl_->records.end();
    if (hit) ++impl_->summary.key_hits;
    return hit;
}

std::size_t GameMessageManagerHost::record_count() const noexcept { return impl_->records.size(); }

const MessageManagerSummary& GameMessageManagerHost::summary() const noexcept { return impl_->summary; }

}  // namespace bsp::game
