#pragma once
#include "bsp/native_gui_page_script_continuation.hpp"
#include "bsp/native_lua_reader.hpp"

namespace bsp {
class NativeLuaServiceBindings;

// Required borrowed CURRENT native table metadata, not callable host addresses.
// Supported domain is CE44FC with current +04 == BD8E20 only. Caller supplies
// the genuine table (at least two DWORDs), actual GuiScreen literal and SAME
// Lua services used by the script stage. No table copy/default/provider exists.
struct NativeGuiPageReaderContext {
    NativeLuaServiceBindings& lua;
    const char* actual_gui_screen_00d5cb24;
    const volatile std::uint32_t* actual_reader_table_00ce44fc;
};

class NativeGuiPageReaderContinuation final {
public:
    // Preserve the actual14h globals/reader/scratch preimages, including
    // globals/scratch padding and reader proxy04, even under value initialization.
    NativeGuiPageReaderContinuation() noexcept;
    ~NativeGuiPageReaderContinuation();
    NativeGuiPageReaderContinuation(const NativeGuiPageReaderContinuation&) = delete;
    NativeGuiPageReaderContinuation& operator=(const NativeGuiPageReaderContinuation&) = delete;
    enum class Phase { fresh, entered, awaiting_properties, failed };
    Phase phase() const noexcept { return phase_; }
    bool native_operation_interrupted() const noexcept {
        return phase_ == Phase::entered || phase_ == Phase::failed;
    }
    struct Observation {
        Phase phase;
        const NativeGuiPageConstructionAcquired* prefix;
        const NativeGuiPageScriptContinuation* scripts;
        const void* actual_page;
        const NativeLuaStateStorage* lua_owner;
        const void* globals_argument;
        const void* reader_storage;
        const void* lookup_scratch;
        bool globals_entered, globals_returned, globals_adopted;
        bool reader_constructor_entered, reader_constructor_returned;
        bool key_entered, key_returned;
        std::uint32_t captured_reader_profile, captured_enter_target;
    };
    Observation observation() const noexcept;
    struct PendingProperties {
        NativeLuaReaderStorage& reader;
        NativeLuaStateStorage& lua_owner;
        NativeLuaServiceBindings& lua_services; // SAME script/reader service domain
    };
    // Only at the normal AC6806/EH5 boundary, with SAME reader/Lua still live.
    // Properties tail MUST independently guard the entire borrowed graph
    // before this borrow and through its actual reader teardown/Lua close.
    PendingProperties borrow_for_properties_tail();
private:
    Phase phase_{Phase::fresh};
    NativeGuiPageConstructionAcquired* prefix_{};
    NativeGuiPageScriptContinuation* scripts_{};
    NativeGuiPageReaderContext* context_{};
    void* page_{};
    NativeLuaStateStorage* owner_{};
    NativeLuaObjectStorage globals_; // exact consumed by-value14h, not a copy
    alignas(NativeLuaReaderStorage) std::byte reader_[sizeof(NativeLuaReaderStorage)];
    NativeLuaObjectStorage scratch_;
    bool globals_entered_{}, globals_returned_{}, globals_adopted_{};
    bool reader_constructor_entered_{}, reader_constructor_returned_{};
    bool key_entered_{}, key_returned_{};
    std::uint32_t captured_profile_{}, captured_target_{};
    NativeLuaReaderStorage& live_reader() noexcept;
    std::uint32_t retained_reader_profile() const noexcept;
    friend void continue_native_gui_page_reader_00ac67cc(NativeGuiPageConstructionAcquired&,
        NativeGuiPageScriptContinuation&, NativeGuiPageReaderContinuation&, NativeGuiPageReaderContext&);
};

// Exactly [AC67CC,AC6806),58B of AC6600. B67980 writes the SAME20-byte globals
// argument consumed by the existing4425C0 constructor. Load CURRENT reader
// profile, then CURRENT table+04; arm EH5 before genuine BD8E20(kind0,GuiScreen).
// Return pending properties with reader and interpreter OPEN; unfinished page,
// no current page+18/property dispatch, root propagation or reader/Lua teardown.
//
// Frame and all borrowed backing exist before effects. External GUI serialization
// includes reentry. Sticky interruption is checked BEFORE any raw metadata in
// the destructor; host must _Exit before any dependency/member/manager/CRT drain.
// No destructor repeats consumed argument/helper cleanup. Normal pending disposal
// requires SAME reader's retired CE374C stamp and SAME owner's state04 == null,
// after a genuine future tail and quiescent borrows. Those cells are a caller
// lifetime contract, NOT authentication that the tail/close ran. No forced
// settlement/abandon/retry API. Source Activation restoration is not restoration
// of Lua C frames/handlers; native-private scratch/stack parity and FH3/SEH are
// excluded. This new C++ interface is not the Original parent/constructor ABI.
void continue_native_gui_page_reader_00ac67cc(NativeGuiPageConstructionAcquired&,
    NativeGuiPageScriptContinuation&, NativeGuiPageReaderContinuation&, NativeGuiPageReaderContext&);
} // namespace bsp
