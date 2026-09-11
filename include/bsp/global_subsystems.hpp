#pragma once

#include "bsp/dialog_config.hpp"
#include "bsp/global_config.hpp"
#include "bsp/gameplay_effect_acquisition.hpp"
#include "bsp/global_script_folders.hpp"
#include "bsp/marker_classes.hpp"
#include "bsp/panel_owner.hpp"
#include "bsp/powerup_config.hpp"
#include "bsp/race_config.hpp"
#include "bsp/recon_values.hpp"
#include "bsp/robot_config.hpp"
#include "bsp/traffic_config.hpp"
#include "bsp/warning_owner.hpp"
#include "bsp/weather_config.hpp"

namespace bsp {

// Borrow the actual publication slots of one game. Opaque owners and the
// effect vector stay in their native layouts; the five configuration owners
// use canonical C++ projections. This is not the native game object ABI.
struct GlobalSubsystemState {
    void*& mission_lua_1a08;
    TrafficConfig*& traffic_21d0;
    VoicePanelState*& panel_21e4;
    PowerupConfigOwner*& powerup_00f88c30;
    WarningOwner*& warnings_21e0;
    WeatherConfig*& weather_21c4;
    void* effects_vector_718c;
};

// Unreconstructed callees remain required services. These are individual
// native calls, not default results or substitutes for entire startup phases.
struct GlobalSubsystemHost {
    virtual ~GlobalSubsystemHost() = default;
    // Opens a distinct stack FileBlock and returns its retained host identity.
    virtual void* construct_file_block_00be0a30(const NativeString&, int flag) = 0;
    virtual void destroy_file_block_00bdcb30(void*) = 0;
    // Binding only: return the real service adapter for this captured mission
    // owner. The recovered folder/file/chunk sequences execute directly.
    virtual MissionLuaHostServices& mission_lua_services_1a08(void* captured_owner) = 0;
    virtual void run_string_006b8ad0(void* lua_instance, const char*,
        int capture_results, int capture_error, int result_mode) = 0;

    // Preserve native refcounted pointer-vector copy/checked-growth semantics.
    virtual void append_effect_004d9c00(void* vector, void* const* source) = 0;
    // Called only after a real InterlockedDecrement at object+4 reaches zero.
    // Dispatch its CURRENT virtual slot+0 with ECX=object and no stack args.
    virtual void effect_zero_references(void* object) = 0;

};

struct GlobalSubsystemContext {
    NativeStringStorage& strings;
    LuaStateOwnerEnvironment lua_environment;
    LuaScriptRuntime& scripts;
    DialogConfigContext& dialog;
    const PanelOwnerAllocationWords& panel_allocation;
    const WeatherConfigAllocationWords& weather_allocation;
    const SingletonLifetimeCallbacks& validation;
    GlobalScriptFolderContext& global_script_folders;
    RaceRecordTable& races_00f87464;
    RaceConfigContext& race_config;
    RobotConfigRegistry& robots_00f89994;
    RobotConfigAliases robot_aliases;
    RobotConfigContext& robot_config;
    // Resolves current embedded game+1A0C independently of mission+1A08.
    MarkerClassContext& marker_classes;
    ReconValuesContext& recon_values;
    const TrafficConfigAllocationWords& traffic_allocation;
    TrafficConfigContext& traffic;
    const PowerupOwnerAllocationWords& powerup_allocation;
    PowerupConfigContext& powerup;
    const WarningOwnerAllocationWords& warning_allocation;
    WarningOwnerContext& warnings;
    GlobalConfigContext& global_config;
    GameplayEffectAcquisitionContext& effect_acquisition;
};

// Complete normal-flow004DC6A0, ECX=game, RET at004DC93F. Allocation and
// refcount operations and recovered script/configuration loaders execute
// directly. Other callees above still require real game implementations.
// C++ exception cleanup is an adaptation, not recovered native SEH/ABI proof.
void construct_global_subsystems_004dc6a0(GlobalSubsystemState,
    GlobalSubsystemHost&, GlobalSubsystemContext&);

struct GlobalSubsystemInvocation {
    GlobalSubsystemState state;
    GlobalSubsystemHost& host;
    GlobalSubsystemContext& context;
};
} // namespace bsp
