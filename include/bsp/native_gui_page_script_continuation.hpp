#pragma once
#include "bsp/native_gui_page_construction.hpp"
#include "bsp/native_gui_script_paths.hpp"
#include "bsp/native_lua_objects.hpp"

namespace bsp {
class NativeLuaServiceBindings;

// Borrow genuine application services. strings MUST be the existing RAW-MANAGER
// ActualNativeStringPoolStorage adapter over the SAME publication/gate/manager
// cells as paths.strings. Its private identity cannot be authenticated here;
// its noexcept release domain is not widened to raw getter exception transport.
// Keep every provider/domain alive through the future tail and shared drain.
struct NativeGuiPageScriptContext {
    ActualNativeStringPoolStorage& strings;
    NativeGuiScriptPathContext& paths;
    NativeLuaServiceBindings& lua;
    const char* actual_common_00d5cb64;
};

class NativeGuiPageScriptContinuation final {
public:
    // User-provided constructor preserves the actual4C8h/raw8h preimages even
    // under value initialization. Typed path materialization restores all8h.
    NativeGuiPageScriptContinuation() noexcept;
    ~NativeGuiPageScriptContinuation();
    NativeGuiPageScriptContinuation(const NativeGuiPageScriptContinuation&) = delete;
    NativeGuiPageScriptContinuation& operator=(const NativeGuiPageScriptContinuation&) = delete;
    enum class Phase { fresh, entered, awaiting_reader, failed };
    Phase phase() const noexcept { return phase_; }
    // Sticky metadata only; no raw reads, providers, callbacks or locks.
    bool native_operation_interrupted() const noexcept {
        return phase_ == Phase::entered || phase_ == Phase::failed;
    }
    struct Observation {
        Phase phase;
        const void* actual_page;
        const NativeLuaStateStorage* lua_owner;
        const void* common_name_header;
        const void* path_header;
        bool owner_constructed, open_returned, common_name_entered, common_name_returned;
        bool common_name_adopted, common_run_returned, page_run_returned;
        bool path_cleanup_armed, path_cleanup_entered;
        NativeGuiScriptPathObservation common_path;
        NativeGuiLayerScriptPathObservation page_path;
    };
    Observation observation() const noexcept;
    // Borrow ONLY at the normal AC67CC boundary. A genuine future reader tail
    // first guards the entire graph, then retains SAME storage/services until
    // actual B669A0 clears state04 and all reader/graph borrows are quiescent.
    // No force-close, settlement, abandon, retry or mark-complete API exists.
    NativeLuaStateStorage& lua_owner_for_reader_tail();
private:
    Phase phase_{Phase::fresh};
    NativeGuiPageConstructionAcquired* prefix_{};
    NativeLuaStateStorage owner_; // no initialization of native-unwritten bytes
    std::uint32_t common_name_[2]; // exact by-value8h input, no default bytes
    union { NativeString path_; }; // SAME raw8h passed to real Lua file services
    NativeGuiScriptPathFrame common_path_;
    NativeGuiLayerScriptPathFrame page_path_;
    bool owner_constructed_{}, open_returned_{}, common_name_entered_{}, common_name_returned_{};
    bool common_name_adopted_{}, common_run_returned_{}, page_run_returned_{};
    bool path_cleanup_armed_{}, path_cleanup_entered_{};
    friend void continue_native_gui_page_scripts_00ac6714(NativeGuiPageConstructionAcquired&,
        NativeGuiPageScriptContinuation&, NativeGuiPageScriptContext&);
};

// Exactly [AC6714,AC67CC),184B of AC6600: actual4C8h owner construct/open65,
// _Common then page scripts, each path returned once after consuming state3/4.
// Input prefix must await AC6714/EH1. Normal return is AC67CC/EH2 with Lua OPEN,
// no reader and an UNFINISHED page. Never publish it or return its pool slot.
// All backing exists before the first native/getter/Lua effect. External GUI
// serialization includes reentry. C++ interruption retains every actual header,
// child cleanup state and interpreter; host must _Exit before dependency drain.
// Source Activation restoration is not restoration of Lua C frames/handlers.
// Destructor first guards sticky interruption, then normal pending state04.
// Eventual disposal after a genuine tail/close/quiescence is a caller contract,
// not authentication that close ran. New Source ABI; not whole-parent FH3/SEH.
void continue_native_gui_page_scripts_00ac6714(NativeGuiPageConstructionAcquired&,
    NativeGuiPageScriptContinuation&, NativeGuiPageScriptContext&);
} // namespace bsp
