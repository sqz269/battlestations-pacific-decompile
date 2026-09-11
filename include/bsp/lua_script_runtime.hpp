#pragma once

#include "bsp/gui_lua_reader.hpp"

struct lua_State;

namespace bsp {

// Native VFS boundary, not a disk fallback. The caller owns this object and
// keeps it alive while a runtime or one of its Lua closures can be called.
struct LuaScriptFiles {
    virtual ~LuaScriptFiles() = default;
    // Singleton0109ceec virtual+4(path,2), stream+18 open/+30 size/+24 read.
    // Missing, not-open and empty files are silent skips. Host I/O failures
    // that cannot supply the native byte count should throw, not return data.
    virtual bool read_file_00b66ca0(const std::string& path, std::uint32_t mode,
                                  std::vector<char>& bytes) = 0;
    // 00bdef90: existing stem + '_' + registered suffix + extension paths,
    // in manager+48/+4c order. Called AFTER executing the base chunk.
    virtual std::vector<std::string> override_paths_00bdef90(const std::string& path) = 0;
};

// Concrete Lua 5.1.1 calls behind the existing GUI script reconstruction.
// New C++ ABI, names are hypotheses. See docs/LUA_SCRIPT_RUNTIME.md.
class LuaScriptRuntime {
public:
    explicit LuaScriptRuntime(LuaScriptFiles& files) noexcept : files_(files) {}
    // Preserve LUA_MULTRET results on success. Throw on native fatal errors;
    // prior global mutations/results survive. The failed error value is popped.
    std::vector<GuiLuaChunkOutcome> run_file(lua_State*, const std::string& path,
                                            bool obfuscated = false);
    GuiLuaChunkOutcome run_chunk(lua_State*, const std::string& path,
                                 bool obfuscated = false);

    // Assign directly to LuaStateOwnerEnvironment::do_file; context=this.
    // Uses the supplied state non-owningly, first argument coerced with
    // lua_tolstring, obfuscation=false, zero Lua returns. Non-string/non-number
    // arguments raise a host error in place of native0041e870's null dereference.
    static int do_file(lua_State*, void*) noexcept;

private:
    static int do_file_closure(lua_State*);
    bool invoke_do_file(lua_State*, const char* path, char* error,
                        std::size_t error_size) noexcept;
    LuaScriptFiles& files_;
};

} // namespace bsp
