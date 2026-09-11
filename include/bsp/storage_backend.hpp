#pragma once

#include "bsp/archive_compression.hpp"
#include "bsp/storage_operation.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct lua_State;

namespace bsp {

// Required owner/environment boundary. 00b6a020 opens base libraries, platform
// and region globals, DoFile, and the real Scripts/fundamentals.lua. An empty
// Lua state is not an implementation of that operation. This owner outlives
// PcStorageBackend, which executes the archive and closes it on failure.
struct PcStorageLuaHost {
    virtual ~PcStorageLuaHost() = default;
    virtual void open_storage_archive_00b6a020(std::uint32_t library_flags) = 0;
    virtual lua_State* storage_lua_38() noexcept = 0;
    virtual void close_storage_archive_00b65e80() noexcept = 0;
};

// Concrete Win32 filesystem, block decoding, Lua execution, and PC vtable+44
// projection. Native object/vtable are 0x540 bytes/00d68c10; this is not their
// layout or ABI. Root is the already initialized Documents/Battlestations-
// Pacific/save directory, or an explicit isolated root for host tools.
class PcStorageBackend {
public:
    PcStorageBackend(std::string root, NativeStringStorage&, PcStorageLuaHost&,
        ArchiveCompressionState&);
    ~PcStorageBackend();
    PcStorageBackend(const PcStorageBackend&) = delete;
    PcStorageBackend& operator=(const PcStorageBackend&) = delete;

    StorageManagerOperation& operation() noexcept { return operation_; }
    const StorageManagerOperation& operation() const noexcept { return operation_; }
    std::int32_t operation_code_04() const noexcept { return operation_code_04_; }
    bool error_20() const noexcept { return error_20_; }
    bool storage_available_21() const noexcept { return ready_21_; }
    std::uint32_t write_kind_508() const noexcept { return write_kind_508_; }
    lua_State* storage_lua_38() noexcept { return lua_.storage_lua_38(); }
    bool has_storage_buffer_30() const noexcept { return buffer_present_30_; }
    const std::vector<std::uint8_t>& storage_buffer_30() const noexcept { return buffer_30_; }
    void free_and_clear_storage_buffer_30() noexcept;
    void close_storage_archive_00b65e80() noexcept;

    // Kind is a DWORD, not bool: 0=quick, 1=player, 2=game. Query additionally
    // requires the valid marker. Requests retain state+08 and existing prompts.
    bool storage_query_1c(std::string_view name, std::uint32_t kind);
    void reset_storage_operation_00bd3450() noexcept;
    void immediate_read_00bd4380(std::string_view name, std::uint32_t kind);
    void request_read_00bd3d70(std::string_view name, std::uint32_t kind);
    void request_write_00bd3dc0(std::string_view name);
    void request_single_write_00bd3e10(std::string_view name, std::uint32_t kind);
    void request_delete_00bd3e70(std::string_view name);
    void update_00beb9b0();

private:
    std::string path(std::string_view name, const char* leaf = nullptr) const;
    bool file_exists(std::string_view name, const char* leaf) const;
    void clear_marker(std::string_view name);
    void stamp_marker(std::string_view name);
    void delete_slot();
    void read_archive_00bd3ec0();
    void open_read_archive_00bd3470();
    void begin_write_00bd34c0();
    void write_archive_00bd37b0();
    void abandon_write_00bd3500() noexcept;
    void prompt_00bd4140(const char* key, bool yes_no, bool accept);
    void complete(std::int32_t state) noexcept;

    std::string root_;
    NativeStringStorage& strings_;
    PcStorageLuaHost& lua_;
    ArchiveCompressionState& compression_;
    StorageManagerOperation operation_;
    std::int32_t operation_code_04_{};
    bool error_20_{};
    bool ready_21_{};
    std::string read_name_24_;
    std::uint32_t read_kind_2c_{};
    std::vector<std::uint8_t> buffer_30_;
    bool buffer_present_30_{};
    std::string write_name_500_;
    std::uint32_t write_kind_508_{};
    std::string delete_name_528_;
};

// Compose the concrete progress operation with the existing real UI/render
// host. Bind StorageOperationState.manager_0109cecc to backend.operation().
class PcStorageOperationHost final : public StorageOperationHost {
public:
    PcStorageOperationHost(PcStorageBackend& backend, StorageOperationHost& ui) noexcept
        : backend_(backend), ui_(ui) {}
    void storage_update_virtual_44(StorageManagerOperation&) override;
    FrontEndPromptScreen& prompt_screen_00425d10() override;
    FrontEndPromptHost& prompt_host() override;
    void try_begin_render_frame_004c6c30() override;
    void render_game_004ca440() override;
    void finish_render_frame_004ca1f0() override;
private:
    PcStorageBackend& backend_;
    StorageOperationHost& ui_;
};

} // namespace bsp
