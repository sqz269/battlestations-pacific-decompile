#include "bsp/gui_page_script.hpp"

#include <iterator>
#include <stdexcept>

namespace bsp {

GuiPageLuaEvaluation::GuiPageLuaEvaluation(VfsLuaScriptFiles& files,
    LuaScriptRuntime& runtime, const LuaRuntimeGlobals& globals, const std::string& page_name)
    : owner_(files.owner_environment(runtime, globals)) {
    //00AC6718..00AC672B: frame owner, mask65 (base/table/string/math).
    owner_.open_storage_archive_00b6a020(kGuiLuaLibraryMask);
    auto* state = owner_.storage_lua_38();
    //00AC675A then00AC67A0. Each run includes existing VFS overrides; both
    // pass obfuscation=false. Missing/empty scripts are native silent skips.
    chunks_ = runtime.run_file(state, gui_page_script_path_00ac5600(kGuiCommonScriptName), false);
    auto page_chunks = runtime.run_file(state, gui_page_script_path_00ac5600(page_name), false);
    chunks_.insert(chunks_.end(), std::make_move_iterator(page_chunks.begin()),
        std::make_move_iterator(page_chunks.end()));
    reader_ = std::make_unique<GuiLua51Host>(*state);
    const GuiLuaRef globals_ref = reader_->globals();
    GuiLuaRef screen;
    try {
        //00AC67DA..00AC6804: globals, then key{0,"GuiScreen"}.
        screen = reader_->get_by_name(globals_ref, kGuiScreenTableName.data());
        table_ = reader_->snapshot_table(screen);
    } catch (...) {
        reader_->release(screen);
        reader_->release(globals_ref);
        throw;
    }
    reader_->release(screen);
    reader_->release(globals_ref);
}
GuiPageLuaEvaluation::~GuiPageLuaEvaluation() = default;

std::unique_ptr<GuiPageLuaEvaluation> VfsGuiPageScripts::begin_00ac6600(
    const std::string& page_name) {
    return std::make_unique<GuiPageLuaEvaluation>(files_, runtime_, globals_, page_name);
}
std::shared_ptr<const GuiTable> VfsGuiPageScripts::evaluate_00ac6600(
    const std::string& page_name) {
    return begin_00ac6600(page_name)->table();
}

GuiPageScriptLayoutHost::~GuiPageScriptLayoutHost() = default;
GuiLayoutPage* GuiPageScriptLayoutHost::load_page(GuiPageRegistry& registry,
    const std::string& name, std::int32_t flag, bool add_reference) {
    if (active_) throw std::logic_error("GUI page construction is already active");
    return load_gui_page_00aa5840(registry, *this, name, flag, add_reference);
}
std::shared_ptr<const GuiTable> GuiPageScriptLayoutHost::evaluate_page_script(
    const std::string& name) {
    if (active_) throw std::logic_error("GUI page construction is already active");
    active_ = scripts_.begin_00ac6600(name);
    return active_->table();
}
GuiLua51Host& GuiPageScriptLayoutHost::live_reader_host() {
    if (!active_) throw std::logic_error("GUI page has no active Lua reader");
    return active_->reader_host();
}
void GuiPageScriptLayoutHost::on_page_loaded(GuiLayoutPage& page) {
    clear_page_root_list_00b6d890(page);
    //00AC682A..00AC6857: reader references, then owner, after the root call.
    active_.reset();
}
void GuiPageScriptLayoutHost::on_page_load_failed() noexcept { active_.reset(); }

} // namespace bsp
