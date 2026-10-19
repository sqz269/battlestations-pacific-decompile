// cc15-prof: the signed-in profile read, run read-only. Evidence, the no-write
// proof and the pairs: docs/CC15_PROFILE_LOAD.md. Every routine is an existing
// typed body (request_profile_read_007ff100, complete_profile_read_007fefe0,
// read_profile_archive_007fdf00, read_mission_progress_00920000, and
// PcStorageBackend's read half); this file only composes them.
//
// NO-WRITE RULE. Of PcStorageBackend this file calls only storage_query_1c,
// immediate_read_00bd4380, has_storage_buffer_30, free_and_clear_storage_buffer_30,
// close_storage_archive_00b65e80 and storage_lua_38. The members that write
// (request_read_00bd3d70's completion stamps <name>\valid in update_00beb9b0;
// request_write_00bd3dc0, request_single_write_00bd3e10, request_delete_00bd3e70)
// are never called, and update_00beb9b0, the only caller of the writers, is never
// run. The archive executes in a Lua state opened with mask 1 (base only: no io
// or os library). 007FA710 and 007F9540 (the profile write) are not referenced.

#include "bsp/game_profile_load.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/native_string.hpp"
#include "bsp/profile_archive.hpp"
#include "bsp/storage_backend.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {

// 0109CECC kind 1 = "player"; 004374F0 asks for kind 2 = "game".
constexpr std::uint32_t kPlayerKind = 1;
constexpr std::uint32_t kCheckpointKind = 2;

// 007FDF00's host calls, on the lane-L2 owner's +64h.
class OwnerArchiveHost final : public ProfileArchiveHost {
public:
    OwnerArchiveHost(GameProfileOwner& owner, GameHostLog& log, PcStorageBackend& backend)
        : owner_(owner), log_(log), backend_(backend) {}

    int hints_owner_field_08_004c1e90() override {
        // [00E17664] is not built; its constructor 00426250 stores +08h = 0 at
        // 00426285. Either value leaves the same list here: 007FDB20 has just
        // cleared profile+08h, so the AllSeenHints rows are the whole list.
        log_.unimplemented("ProfileLoad::hints_owner_field_08", "004c1e90");
        return 0;
    }
    void destroy_mission_progress_007fd780() override {
        owner_.destroy_mission_progress_007fd780();
    }
    bool construct_mission_progress_00920e10() override {
        return owner_.construct_mission_progress_00920e10();
    }
    void read_mission_progress_00920000(
        ProfileResetState& profile, ProfileArchiveReader& reader) override {
        // Called even after a failed new(24h), as natively; the owner has none then.
        MissionProgress* progress = owner_.mission_progress();
        if (progress == nullptr) {
            log_.note("profile load: 00920000 skipped, +64h was not allocated");
            return;
        }
        bsp::read_mission_progress_00920000(*progress, reader, profile.version_e8);
        sync_mission_completion(*progress, profile.unlock_state);
        log_.implemented("ProfileLoad::read_mission_progress", "00920000");
    }
    void write_mission_progress_0090cb40(const ProfileResetState&, ProfileArchiveWriter&) override {
        // Reached only from 007F9540, the profile write, which this file never runs.
        throw std::logic_error("profile load is read-only: 0090CB40 must not run");
    }
    int sum_mission_progress_0090bf50() override {
        const MissionProgress* progress = owner_.mission_progress();
        return progress != nullptr ? bsp::sum_mission_progress_0090bf50(*progress) : 0;
    }
    void refresh_profile_manager_004374f0(ProfileResetState& profile) override {
        // 004374F0 builds the game Lua table BSP_Chk_Save, then reads the
        // checkpoint save (kind 2, "game") only when +1Ch answers true. The
        // queued read it would issue (00BD3D70 + 006ADB50) stamps <name>\valid,
        // so it is not run; the query alone is read-only.
        const bool present = !profile.save_name_34.empty()
            && backend_.storage_query_1c(profile.save_name_34, kCheckpointKind);
        log_.unimplemented("ProfileLoad::refresh_profile_manager", "004374f0");
        log_.notef("profile load: 004374F0 checkpoint save %s\\game %s", profile.save_name_34.c_str(),
            present ? "present (its read is withheld)" : "absent (the native reads nothing either)");
    }

private:
    GameProfileOwner& owner_;
    GameHostLog& log_;
    PcStorageBackend& backend_;
};

// 007FF100 / 007FEFE0's host calls.
class ReadOnlyProfileIoHost final : public ProfileIoHost {
public:
    ReadOnlyProfileIoHost(GameProfileOwner& owner, GameHostLog& log,
        PcStorageBackend& backend, OwnerArchiveHost& archive, const bool& crt_sse2)
        : owner_(owner), log_(log), backend_(backend), archive_(archive), crt_sse2_(crt_sse2) {}

    GameProfileLoadReport report{};
    ProfileIoState io{};
    GameSettingsBlock discarded_settings{};

    // ProfileResetHost: the owner's, so 007FDB20 behaves as lane L2 has it.
    void destroy_mission_progress_007fd780() override { owner_.destroy_mission_progress_007fd780(); }
    bool construct_mission_progress_00920e10() override { return owner_.construct_mission_progress_00920e10(); }
    bool xenon_state_is_two() override { return owner_.xenon_state_is_two(); }
    bool xenon_user_selected() override { return owner_.xenon_user_selected(); }
    void restore_selected_user_controls_008d45d0() override { owner_.restore_selected_user_controls_008d45d0(); }

    bool storage_query_1c(std::string_view name, bool kind) override {
        // GENERIC_READ opens of <name>\valid and <name>\player (00BEB1E0).
        report.queried = backend_.storage_query_1c(name, kind ? kPlayerKind : 0u);
        log_.implemented("ProfileLoad::storage_query_1c", "00beb1e0");
        return report.queried;
    }
    void request_read_00bd3d70(std::string_view name, bool kind) override {
        // Natively this queues operation 2; 00BEB9B0 then reads and, on success,
        // re-stamps <name>\valid (CREATE_ALWAYS + SetFileTime). The host keeps
        // the name and kind and reads in run_storage_operation_006adb50 instead.
        read_name_.assign(name);
        read_kind_ = kind ? kPlayerKind : 0u;
        log_.implemented("ProfileLoad::request_read (immediate, no valid stamp)", "00bd3d70");
    }
    void request_write_00bd3dc0(std::string_view) override {
        throw std::logic_error("profile load is read-only: 00BD3DC0 must not run");
    }
    void run_storage_operation_006adb50(ProfileIoTask task) override {
        if (task != ProfileIoTask::ReadCompleted) {
            throw std::logic_error("profile load is read-only: only 007FEFE0 may be driven");
        }
        // 00BD4380: CreateFileA(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING),
        // ReadFile, block decode, then 00B6A020(1) + DoBuffer. No prompt, no
        // marker write, no operation-code change.
        backend_.immediate_read_00bd4380(read_name_, read_kind_);
        report.read = backend_.operation().state_08 == 0;
        log_.implemented("ProfileLoad::read_archive", "00bd3ec0");
        if (!report.read) {
            // 00BEB9B0 would raise "globals.loadfailed" here; nothing is shown.
            log_.notef("profile load: reading %s\\player failed; 007FEFE0 resets", read_name_.c_str());
        }
        complete_profile_read_007fefe0(owner_.profile(), discarded_settings, io, *this);
        log_.implemented("ProfileLoad::complete_read", "007fefe0");
    }
    int storage_state_08() override { return backend_.operation().state_08; }

    void begin_profile_reader_004425c0() override {
        lua_State* state = backend_.storage_lua_38();
        if (state == nullptr) throw std::logic_error("profile reader requires the open storage Lua");
        readers_.push_back(std::make_unique<ReaderFrame>(*state, crt_sse2_));
    }
    void deserialize_profile_007fdf00(ProfileResetState& profile) override {
        read_profile_archive_007fdf00(profile, readers_.back()->reader, archive_);
        log_.implemented("ProfileLoad::read_profile_archive", "007fdf00");
    }
    bool has_storage_buffer_30() override { return backend_.has_storage_buffer_30(); }
    void free_and_clear_storage_buffer_30() override { backend_.free_and_clear_storage_buffer_30(); }
    void close_storage_archive_00b65e80() override { backend_.close_storage_archive_00b65e80(); }
    bool has_manager_00f8a2fc() override {
        log_.unimplemented("ProfileLoad::manager_00f8a2fc", "00f8a2fc");
        return false;
    }
    void notify_manager_virtual_a0() override {}
    void restore_settings_008d7a50(ProfileCompletion completion) override {
        // Withheld, as lane L2 withholds 007FDB20's settings half: 008D7A50 would
        // read <name>\quick into the live settings. The completion slot is the
        // sign-in's 0067CA40, whose title skip the host's own path performs.
        log_.unimplemented("ProfileLoad::restore_settings (withheld)", "008d7a50");
        static_cast<void>(completion);
    }
    void apply_settings_008d5b50() override {
        log_.unimplemented("ProfileLoad::apply_settings (withheld)", "008d5b50");
    }
    void commit_profile_007fae70(ProfileResetState&) override {
        log_.unimplemented("ProfileLoad::commit_content_state", "007fae70");
    }
    void destroy_profile_reader_00441a20() override { readers_.pop_back(); }

private:
    struct ReaderFrame {
        GuiLua51Host lua;
        GuiLuaReader reader;
        ReaderFrame(lua_State& state, const bool& crt_sse2)
            : lua(state), reader(lua, lua.globals(), crt_sse2) {}
    };

    GameProfileOwner& owner_;
    GameHostLog& log_;
    PcStorageBackend& backend_;
    OwnerArchiveHost& archive_;
    const bool& crt_sse2_;
    std::string read_name_;
    std::uint32_t read_kind_{kPlayerKind};
    std::vector<std::unique_ptr<ReaderFrame>> readers_;
};

} // namespace

std::string read_only_save_root() {
    if (const char* root = std::getenv("BSP_PROFILE_LOAD_ROOT"); root != nullptr && *root != '\0') {
        return root;
    }
    char personal[MAX_PATH]{};
    // fCreate FALSE: unlike 00BEB2C0, nothing is created.
    if (!SHGetSpecialFolderPathA(nullptr, personal, CSIDL_PERSONAL, FALSE)) return {};
    std::string path = personal;
    path += "\\Battlestations-Pacific\\save";
    return path;
}

GameProfileLoadReport load_signed_in_profile_read_only(GameProfileOwner& owner,
    GameHostLog& log, const std::string& save_root, std::string_view save_name,
    LuaStateOwnerEnvironment lua)
{
    // 0109EEA4, bound as GameMenuHost binds it.
    static const bool crt_sse2 = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;

    PcStorageLuaOwner storage_lua(std::move(lua));
    ArchiveCompressionState unused_compression; // the write half's buffer, never touched
    PcStorageBackend backend(save_root, crt_string_storage(), storage_lua, unused_compression);
    backend.reset_storage_operation_00bd3450(); // 0067CC60's 00BD3450; memory only
    OwnerArchiveHost archive(owner, log, backend);
    ReadOnlyProfileIoHost host(owner, log, backend, archive, crt_sse2);

    // 0067CC60: 007FDB20 first, then (PC: [0109CECC]+21h is 1) 007FF100.
    owner.reset_to_defaults_007fdb20();
    log.implemented("ProfileLoad::reset_to_defaults", "007fdb20");
    log.notef("profile load: root %s save %.*s", save_root.c_str(),
        static_cast<int>(save_name.size()), save_name.data());
    request_profile_read_007ff100(owner.profile(), host.discarded_settings, host.io, host,
        save_name, [] {});
    log.implemented("ProfileLoad::request_read", "007ff100");

    GameProfileLoadReport report = host.report;
    const ProfileResetState& profile = owner.profile();
    report.seen_hints = profile.string_list_08.size();
    report.score_total = profile.score_total_30;
    if (const MissionProgress* progress = owner.mission_progress()) {
        report.mission_records = progress->mission_scores_00.size();
        for (const auto& [key, record] : progress->mission_scores_00) {
            if (record.mission_completed_00 != 0) {
                ++report.completed_records;
                log.notef("profile load: completed record \"%s\"", key.c_str());
            }
        }
    }
    log.notef("profile load: queried=%d read=%d version=%u selected_mission=%s seen_hints=%zu "
        "unlocks=%zu achievements=%zu mission_records=%zu completed=%zu score_total=%d",
        report.queried ? 1 : 0, report.read ? 1 : 0, profile.version_e8,
        profile.selected_mission_id_68.c_str(), report.seen_hints,
        profile.unlock_state.unlocks.size(), profile.unlock_state.named_counters.size(),
        report.mission_records, report.completed_records, report.score_total);
    return report;
}

} // namespace bsp::game
