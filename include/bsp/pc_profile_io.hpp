#pragma once
#include "bsp/profile_commit.hpp"
#include "bsp/profile_persistence.hpp"
#include "bsp/settings_archive_reader.hpp"
#include "bsp/storage_backend.hpp"

namespace bsp {
// Actual game/platform effects still required by the composed persistence path.
struct PcProfileGameHost {
    virtual ~PcProfileGameHost() = default;
    virtual bool game_present() = 0;
    virtual bool xenon_state_is_two() = 0;
    virtual bool xenon_user_selected() = 0;
    virtual void restore_selected_user_controls_008d45d0() = 0;
    virtual bool has_manager_00f8a2fc() = 0;
    virtual void notify_manager_virtual_a0() = 0;
};
struct PcProfileIoContext {
    ProfileResetState& profile;
    GameSettingsBlock& settings;
    ProfileIoState& io;
    MissionProgressOwner& scores;
    InputSettings& input;
    ProfileArchiveHost& archive;
};
struct PcProfileIoServices {
    PcProfileGameHost& game;
    SettingsArchiveReadHost& archive_read;
    KeyboardRuntimeHost& keyboard;
    ProfileCommitHost& commit;
    SettingsApplyHost& apply;
    SettingsApplyEnvironment& apply_environment;
    SettingsCommitLatches& apply_latches;
    const std::vector<LanguageEntry>& languages;
};
// Composes the real PC backend, Lua archive readers, score owner, settings
// traversal, keyboard restore and recovered callback/commit sequences.
// Bind operation.manager_0109cecc to backend.operation() before construction.
// The caller supplies actual UI/platform/renderer/content services and output
// routing. This host and every referenced object must outlive retained callbacks.
// A new C++ interface, not a native object layout or binary replacement.
class PcProfileIoHost final : public ProfileIoHost, public ProfileSettingsRestoreHost {
public:
    PcProfileIoHost(PcProfileIoContext, PcStorageBackend&, StorageOperationState&,
        StorageOperationHost& ui, PcProfileIoServices, ArchiveTextOutput&,
        ArchiveSettingsServices&);
    ~PcProfileIoHost() override;
    void read(std::string_view name, ProfileCompletion completion);
    void write(std::string_view name, ProfileCompletion completion, bool force);
    const ProfileCommitState& commit_state() const noexcept { return commit_state_; }

    void destroy_mission_progress_007fd780() override;
    bool construct_mission_progress_00920e10() override;
    bool xenon_state_is_two() override;
    bool xenon_user_selected() override;
    void restore_selected_user_controls_008d45d0() override;
    bool storage_query_1c(std::string_view, bool) override;
    void request_read_00bd3d70(std::string_view, bool) override;
    void request_write_00bd3dc0(std::string_view) override;
    void run_storage_operation_006adb50(ProfileIoTask) override;
    void run_storage_operation_006adb50(ProfileCompletion) override;
    int storage_state_08() override;
    void begin_profile_reader_004425c0() override;
    void deserialize_profile_007fdf00(ProfileResetState&) override;
    bool has_storage_buffer_30() override;
    void free_and_clear_storage_buffer_30() override;
    void close_storage_archive_00b65e80() override;
    bool has_manager_00f8a2fc() override;
    void notify_manager_virtual_a0() override;
    void restore_settings_008d7a50(ProfileCompletion) override;
    void apply_settings_008d5b50() override;
    void commit_profile_007fae70(ProfileResetState&) override;
    void destroy_profile_reader_00441a20() override;
    void reset_storage_operation_00bd3450() override;
    bool storage_available_21() override;
    bool game_present() override;
    bool profile_has_save_name_007f8ca0() override;
    std::string copy_profile_save_name_00425f40() override;
    void immediate_read_00bd4380(std::string_view, bool) override;
    void begin_settings_reader_004425c0() override;
    void deserialize_settings_008d6dc0(GameSettingsBlock&) override;
    void destroy_settings_reader_00441a20() override;
private:
    struct ReaderFrame;
    PcProfileIoContext context_;
    PcStorageBackend& backend_;
    StorageOperationState& operation_;
    PcStorageOperationHost storage_host_;
    PcProfileIoServices services_;
    ProfileSettingsRestoreState restore_state_;
    ProfileCommitState commit_state_;
    TextProfileWriteHost writer_;
    std::vector<std::unique_ptr<ReaderFrame>> profile_readers_;
    std::vector<std::unique_ptr<ReaderFrame>> settings_readers_;
};
}
