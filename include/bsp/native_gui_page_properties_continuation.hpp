#pragma once
#include "bsp/native_gui_page_reader_continuation.hpp"

namespace bsp {
class NativeNodeSceneChildDispatch;

// Pure metadata lookups return the SAME current table represented by the raw
// profile. No numeric VA is invoked as a host function and no table is invented.
// Page metadata spans at least7 DWORDs; reader metadata spans at least3 DWORDs.
// Provider/table backing outlives every callback, including reader retirement.
class NativeGuiPagePropertiesBindings {
public:
    virtual ~NativeGuiPagePropertiesBindings() = default;
    virtual const volatile std::uint32_t* current_page_table(void* actual_page,
        std::uint32_t captured_profile) = 0;
    virtual const volatile std::uint32_t* current_reader_table(
        NativeLuaReaderStorage& actual_reader, std::uint32_t captured_profile) = 0;
    // Mandatory genuine full reached current+18 implementation on this SAME
    // raw124h Page/reader. It owns property/child dispatch and its acquisitions;
    // no typed GuiLayoutPage receiver, stub visitor or alternate child tree.
    virtual void call_page_properties_18(void* actual_page,
        std::uint32_t captured_target, NativeLuaReaderStorage& actual_reader) = 0;
};

struct NativeGuiPagePropertiesContext {
    NativeGuiPagePropertiesBindings& properties;
    NativeNodeSceneChildDispatch& nodes;
};

// Sideband observation only: owns no Page, reader, Lua state, node or reference.
// Keep this guard and all borrowed graph/provider backing alive before entering
// and through genuine retirement. Interruption is sticky and not restartable.
struct NativeGuiPagePropertiesContinuation final {
    enum class Phase { fresh, entered, constructed, failed };
    ~NativeGuiPagePropertiesContinuation();
    NativeGuiPagePropertiesContinuation() = default;
    NativeGuiPagePropertiesContinuation(const NativeGuiPagePropertiesContinuation&) = delete;
    NativeGuiPagePropertiesContinuation& operator=(const NativeGuiPagePropertiesContinuation&) = delete;
    Phase phase{Phase::fresh};
    const NativeGuiPageConstructionAcquired* prefix{};
    const NativeGuiPageReaderContinuation* reader_stage{};
    void* actual_page{};
    const NativeLuaReaderStorage* actual_reader{}; // identity only after tidy
    const NativeLuaStateStorage* actual_lua_owner{};
    std::uint32_t page_profile{}, properties_target{};
    std::uint32_t reader_profile{}, leave_target{};
    void* captured_node{};
    bool properties_entered{}, properties_returned{}, leave_returned{};
    bool root_returned{}, reader_retired{}, lua_close_returned{};
    bool native_operation_interrupted() const noexcept {
        return phase == Phase::entered || phase == Phase::failed;
    }
};

// Normal AC6600 tail [AC6806,AC6878): CURRENT Page+18(reader), CURRENT reader+08,
// CURRENT Page+4C -> B6D890(0), exact raw reader destructor, B669A0 on SAME owner.
// The genuine reader leave domain is CURRENT CE44FC/+08 BD7A20 only, reusing
// native reader+4 vector pop; other profiles/targets reject without fallback.
// Root propagation uses existing actual scene-child dispatch, not a host scene.
// Success appends the constructed prefix phase; no page registration/publication,
// scene/store acquisition, native retain or terminal destruction is added.
//
// External serialization covers the entire graph and reentry. All acquisitions
// and provider backing survive failures; host must _Exit before dependency drain
// when interrupted. No retry, unwind/rollback, catch-to-Lua transport, native
// FH3/private-stack restoration or original constructor ABI is claimed. Normal
// success is source reconstruction/build proof until composed and game-validated.
void continue_native_gui_page_properties_00ac6806(NativeGuiPageConstructionAcquired&,
    NativeGuiPageReaderContinuation&, NativeGuiPagePropertiesContinuation&,
    NativeGuiPagePropertiesContext&);
} // namespace bsp
