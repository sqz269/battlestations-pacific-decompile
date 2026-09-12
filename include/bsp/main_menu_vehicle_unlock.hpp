#pragma once

#include <cstdint>

namespace bsp {
struct NativeLuaStateStorage;
class NativeStringStorage;
struct ProfileResetState;

// Required association with the CURRENT global game owner. These accessors
// resolve storage only; they must return the existing owners without copying
// Lua state, the profile, or its unlock containers. 584750 reads the Lua owner
// first and reloads the game for the profile only after both string constructs.
struct MainMenuVehicleUnlockStorage {
    virtual ~MainMenuVehicleUnlockStorage() = default;
    virtual NativeLuaStateStorage& lua_owner_00e188a8_1a0c() = 0;
    virtual ProfileResetState& profile_00e188a8_650() = 0;
};

// Complete normal 00584750..00584A21 control flow. Native callee consumes two
// stack DWORDs (RET8), returns Boolean AL, and does not read incoming ECX or
// first_argument. Class -1 succeeds without loading either global owner.
// Uses actual LuaObject/NativeString storage and borrows the existing profile
// unlock projection for 7FC4C0; no native profile-tree ABI or SEH claim.
// Evidence and inherited predicate limits: docs/MAIN_MENU_VEHICLE_UNLOCK.md.
bool main_menu_vehicle_class_unlocked_00584750(bool first_argument,
    std::int32_t vehicle_class, MainMenuVehicleUnlockStorage& storage,
    NativeStringStorage& strings);

} // namespace bsp
