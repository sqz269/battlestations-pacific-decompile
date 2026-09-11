#pragma once

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {

// Script half of00AC6600, __thiscall(screen, NativeString* name, model*, byte
// flag), RET0C; final instruction00AC6876. Names/new C++ ABI are provisional.
// A page owns a private Lua5.1.1 state until traversal completes. Destruction
// retires registry references before closing it. The retained data table can
// outlive the state; functions/metatables/cycles are rejected, not flattened.
class GuiPageLuaEvaluation {
public:
    GuiPageLuaEvaluation(VfsLuaScriptFiles&, LuaScriptRuntime&,
        const LuaRuntimeGlobals&, const std::string& page_name);
    ~GuiPageLuaEvaluation();
    GuiPageLuaEvaluation(const GuiPageLuaEvaluation&) = delete;
    GuiPageLuaEvaluation& operator=(const GuiPageLuaEvaluation&) = delete;
    const std::shared_ptr<const GuiTable>& table() const noexcept { return table_; }
    GuiLua51Host& reader_host() noexcept { return *reader_; }
    const std::vector<GuiLuaChunkOutcome>& chunks() const noexcept { return chunks_; }
private:
    // Member order is intentional: reader dies before owner.
    PcStorageLuaOwner owner_;
    std::unique_ptr<GuiLua51Host> reader_;
    std::shared_ptr<const GuiTable> table_;
    std::vector<GuiLuaChunkOutcome> chunks_;
};

class VfsGuiPageScripts {
public:
    VfsGuiPageScripts(VfsLuaScriptFiles& files, LuaScriptRuntime& runtime,
        const LuaRuntimeGlobals& globals) noexcept
        : files_(files), runtime_(runtime), globals_(globals) {}
    std::unique_ptr<GuiPageLuaEvaluation> begin_00ac6600(const std::string& page_name);
    // Data-only convenience: closes after snapshot. Runtime widget traversal
    // uses begin_00ac6600 and keeps its result until00AC6825 completes.
    std::shared_ptr<const GuiTable> evaluate_00ac6600(const std::string& page_name);
private:
    VfsLuaScriptFiles& files_;
    LuaScriptRuntime& runtime_;
    const LuaRuntimeGlobals& globals_;
};

// Real evaluator plus explicit unresolved scene/widget contracts. Abstract on
// purpose: a game adapter must implement GuiLayoutHost's root/model/child-node,
// parenting, widescreen, derived-property and+74/+78 callbacks. No fabricated
// node IDs or no-op class hooks are supplied. It may use live_reader_host()
// while projecting properties. Both the free loader and load_page perform
// deterministic private-state cleanup on every exception after a cache miss.
class GuiPageScriptLayoutHost : public GuiLayoutHost {
public:
    explicit GuiPageScriptLayoutHost(VfsGuiPageScripts& scripts) noexcept : scripts_(scripts) {}
    ~GuiPageScriptLayoutHost() override;
    GuiLayoutPage* load_page(GuiPageRegistry&, const std::string& name,
        std::int32_t screen_flag, bool add_reference);
    std::shared_ptr<const GuiTable> evaluate_page_script(const std::string&) final;
    void on_page_loaded(GuiLayoutPage&) final;
    void on_page_load_failed() noexcept final;
protected:
    GuiLua51Host& live_reader_host();
    //00AC6825 calls00B6D890(node,0), recursively clears the root-list binding;
    // does not unlink the node's parent. The Lua state is still open here.
    virtual void clear_page_root_list_00b6d890(GuiLayoutPage&) = 0;
private:
    VfsGuiPageScripts& scripts_;
    std::unique_ptr<GuiPageLuaEvaluation> active_;
};

} // namespace bsp
