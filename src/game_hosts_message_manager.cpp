// cc15-ob1: the MessageManager [game+21DCh]. See include/bsp/game_hosts_message_manager.hpp
// and docs/CC15_OBJECT_MESSAGEMANAGER.md.

#include "bsp/game_hosts_message_manager.hpp"

#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_lua_runtime.hpp"
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
    seg::PropertyVisitor* open_script_reader(const std::string& path) override {
        // 00706160 runs the path through 00B69D40 in a state of its own. As for
        // Warnings.lua (009870A0) and decals.lua, the base file is read from the VFS
        // and run by the repository's Lua 5.1.1; 00BDEF90's override files are not run.
        std::string text;
        std::string resolved = path;
        std::string error;
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
    std::string language_name_008d57a0() override { return voice_language; }
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
    const VoiceClipRecord* record = impl_->resolve_record_00705e00(key);
    key.release_to(crt_string_storage());
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
