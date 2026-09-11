#include "bsp/pc_profile_manager.hpp"

namespace bsp {
lua_State* PcProfileManagerRefreshHost::game_lua_1a0c() { return game_lua_slot_; }
lua_State* PcProfileManagerRefreshHost::storage_lua_38() { return backend_.storage_lua_38(); }
bool PcProfileManagerRefreshHost::storage_query_1c(std::string_view name, std::uint32_t kind) {
    return backend_.storage_query_1c(name, kind);
}
bool PcProfileManagerRefreshHost::has_storage_buffer_30() { return backend_.has_storage_buffer_30(); }
void PcProfileManagerRefreshHost::free_and_clear_storage_buffer_30() {
    backend_.free_and_clear_storage_buffer_30();
}
void PcProfileManagerRefreshHost::close_storage_archive_00b65e80() {
    backend_.close_storage_archive_00b65e80();
}
void PcProfileManagerRefreshHost::request_read_00bd3d70(std::string_view name, std::uint32_t kind) {
    backend_.request_read_00bd3d70(name, kind);
}
} // namespace bsp
