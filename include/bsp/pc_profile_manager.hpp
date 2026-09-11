#pragma once

#include "bsp/profile_manager.hpp"
#include "bsp/storage_backend.hpp"

namespace bsp {

// Connects the recovered refresh to the concrete PC storage owner. The game
// Lua publication slot and backend must outlive this host and any retained
// storage continuation. Interpreter creation and ownership remain explicit.
class PcProfileManagerRefreshHost final : public ProfileManagerRefreshHost {
public:
    PcProfileManagerRefreshHost(PcStorageBackend& backend, lua_State*& game_lua_slot) noexcept
        : backend_(backend), game_lua_slot_(game_lua_slot) {}
    lua_State* game_lua_1a0c() override;
    lua_State* storage_lua_38() override;
    bool storage_query_1c(std::string_view, std::uint32_t kind) override;
    bool has_storage_buffer_30() override;
    void free_and_clear_storage_buffer_30() override;
    void close_storage_archive_00b65e80() override;
    void request_read_00bd3d70(std::string_view, std::uint32_t kind) override;
private:
    PcStorageBackend& backend_;
    lua_State*& game_lua_slot_;
};

} // namespace bsp
