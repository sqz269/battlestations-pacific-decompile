#include "bsp/input_script_startup.hpp"
#include <stdexcept>

namespace bsp {

InputScriptStartup::InputScriptStartup(VfsLuaScriptFiles& files, LuaScriptRuntime& runtime,
    const LuaRuntimeGlobals& globals)
    : files_(files), runtime_(runtime), globals_(globals) {
    // 006ab7dd..006ab7e3: both flags begin clear, then call the guarded loader.
    load_data_tables_006a7be0();
}

InputScriptStartup::~InputScriptStartup() = default;

lua_State* InputScriptStartup::control_presets_lua() noexcept {
    return control_presets_ ? control_presets_->storage_lua_38() : nullptr;
}

void InputScriptStartup::load_data_tables_006a7be0() {
    if (data_tables_started_) return; // 006a7c09..006a7c10
    data_tables_started_ = true;     // 006a7c21, before00b6a020

    control_presets_ = std::make_unique<PcStorageLuaOwner>(
        files_.owner_environment(runtime_, globals_));
    control_presets_->open_storage_archive_00b6a020(1);
    runtime_.run_file(control_presets_lua(), "Scripts/datatables/ControlPresets.lua", false);

    // 006a7c74..006a7cb7: construct a second owner, also mask1. It survives both
    // script executions and schema reads, then closes at006a9829. Keep path
    // spelling/separators because00bdef90's override splitting is observable.
    PcStorageLuaOwner tables(files_.owner_environment(runtime_, globals_));
    tables.open_storage_archive_00b6a020(1);
    lua_State* state = tables.storage_lua_38();
    runtime_.run_file(state, "Scripts\\datatables\\KeyboardSetup.lua", false);

    std::string error;
    if (!load_keyboard_setup(state, settings_, error))
        throw std::runtime_error("Input settings KeyboardSetup: " + error);

    // 006a947a commits DEVINPUTS before the controller script runs. Its globals
    // can depend on KeyboardSetup, but cannot retroactively change parsed data.
    runtime_.run_file(state, "Scripts\\datatables\\ControllerInputNames.lua", false);
    if (!load_controller_input_names(state, settings_, error))
        throw std::runtime_error("Input settings ControllerInputNames: " + error);
}

} // namespace bsp
