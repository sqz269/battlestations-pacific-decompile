#pragma once

#include "bsp/game_entry.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/profile_persistence.hpp"

#include <cstdint>
#include <string_view>

struct lua_State;

namespace bsp {

// Host projections only. Native allocation sizes are 1Ch and 50h. The refresh
// does not read the manager's +4h LuaObject; the hints service reads only +8h.
// Other manager/hints methods and fields are outside this reconstruction.
struct ProfileManagerOwner { GuiLuaRef lua_object_04{}; };
struct ProfileHintsOwner { std::int32_t field_08{}; };

// ECX-free native singleton accessors, RET/EAX. Share the actual publication
// slots and lifetime manager. That manager's destroy_registered callback owns
// deletion of these host objects; its native destructor dispatch is external.
ProfileManagerOwner* get_profile_manager_00425c20(
    ProfileManagerOwner* volatile& global_00e17680, SingletonLifetimeManager&);
ProfileHintsOwner* get_profile_hints_owner_004c1e90(
    ProfileHintsOwner* volatile& global_00e17664, SingletonLifetimeManager&);

struct ProfileManagerRefreshHost {
    virtual ~ProfileManagerRefreshHost() = default;
    virtual lua_State* game_lua_1a0c() = 0;
    virtual lua_State* storage_lua_38() = 0;
    // The integer kind is essential: this path passes 2 (game storage).
    virtual bool storage_query_1c(std::string_view name, std::uint32_t kind) = 0;
    virtual bool has_storage_buffer_30() = 0;
    virtual void free_and_clear_storage_buffer_30() = 0;
    virtual void close_storage_archive_00b65e80() = 0;
    virtual void request_read_00bd3d70(std::string_view name, std::uint32_t kind) = 0;
};

// Native 00BD53C0 is exactly RET. Its intentionally empty body is evidence,
// not an assumed storage-completion success or a missing implementation.
void profile_refresh_storage_completed_00bd53c0() noexcept;

// 007FC2C0: ECX=profile; NativeString* key plus by-value 14h record, RET18h.
// Upserts by the native case-insensitive C-string comparison, preserving the
// first key spelling. The vector's order does not model the native tree order.
void set_profile_transient_record_007fc2c0(
    ProfileResetState&, ProfileTransientRecord94 record);

// 004374F0: ECX=profile manager, RET. Native ECX is only passed recursively
// through 00436C50, which does not read manager fields. Replaces game globals
// BSP_Chk_Save before the nonempty-name/query gates. Storage may retain the
// native RET continuation at a prompt; import still runs immediately afterward.
// Both Lua states and the storage driver must belong to the actual shared host.
void refresh_profile_manager_004374f0(ProfileResetState&, ProfileManagerRefreshHost&,
    StorageOperationState&, StorageOperationHost&);

class ConcreteProfileArchiveManagerServices final : public ProfileArchiveManagerServices {
public:
    ConcreteProfileArchiveManagerServices(ProfileHintsOwner&, ProfileManagerRefreshHost&,
        StorageOperationState&, StorageOperationHost&) noexcept;
    int hints_owner_field_08_004c1e90() override;
    void refresh_profile_manager_004374f0(ProfileResetState&) override;
private:
    ProfileHintsOwner& hints_;
    ProfileManagerRefreshHost& host_;
    StorageOperationState& storage_;
    StorageOperationHost& storage_host_;
};

} // namespace bsp
