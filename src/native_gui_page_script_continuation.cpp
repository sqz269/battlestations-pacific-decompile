#include "bsp/native_gui_page_script_continuation.hpp"
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <array>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI page script continuation requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8 && alignof(NativeString) == 4);
static_assert(std::is_standard_layout_v<NativeString> && std::is_trivially_destructible_v<NativeString>);
static_assert(sizeof(NativeLuaStateStorage) == 0x4c8 && alignof(NativeLuaStateStorage) == 4);
namespace {
void* path_data(const NativeString& header) noexcept {
    return *reinterpret_cast<void* const volatile*>(reinterpret_cast<const unsigned char*>(&header) + 4);
}
std::uint32_t path_length(const NativeString& header) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(&header);
}
void return_captured_path(void* captured, std::uint32_t bytes, NativeStringRawPoolContext& strings,
    NativeGuiPageConstructionAcquired& prefix, std::uint32_t getter_site, std::uint32_t return_site) {
    prefix.native_site = getter_site;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    prefix.native_site = return_site;
    return_native_string_pool_00bd1510(pool, captured, bytes,
        strings.actual_small_returns_disabled_01090aa4);
}
}

NativeGuiPageScriptContinuation::NativeGuiPageScriptContinuation() noexcept {
    // The union prevents NativeString's default zeroing before this snapshot.
    // This Source-only lifetime materialization adds no externally observed
    // preimage change; native-private scratch aliases/transient stores excluded.
    std::array<std::byte, sizeof(NativeString)> preimage;
    std::memcpy(preimage.data(), &path_, preimage.size());
    ::new (static_cast<void*>(&path_)) NativeString;
    std::memcpy(&path_, preimage.data(), preimage.size());
}
NativeGuiPageScriptContinuation::~NativeGuiPageScriptContinuation() {
    // Do NOT read possibly interrupted native metadata before the sticky guard.
    if (native_operation_interrupted()) std::terminate();
    if (phase_ == Phase::awaiting_reader && owner_.state_04 != nullptr) std::terminate();
    // Null state after a genuine tail's actual close/quiescence is the caller's
    // contract, not proof/authentication of its history. No cleanup is added.
    path_.~NativeString();
}
NativeGuiPageScriptContinuation::Observation NativeGuiPageScriptContinuation::observation() const noexcept {
    return {phase_, prefix_ ? prefix_->actual_page : nullptr, &owner_, common_name_, &path_,
        owner_constructed_, open_returned_, common_name_entered_, common_name_returned_,
        common_name_adopted_, common_run_returned_, page_run_returned_,
        path_cleanup_armed_, path_cleanup_entered_, common_path_.observation(), page_path_.observation()};
}
NativeLuaStateStorage& NativeGuiPageScriptContinuation::lua_owner_for_reader_tail() {
    if (phase_ != Phase::awaiting_reader || !prefix_ ||
        prefix_->phase != NativeGuiPageConstructionAcquired::Phase::awaiting_reader_continuation ||
        prefix_->continuation != 0x00ac67ccu || prefix_->native_eh_state != 2 || !owner_.state_04)
        throw std::logic_error("GUI Lua owner borrow requires the normal AC67CC reader boundary");
    return owner_;
}
void continue_native_gui_page_scripts_00ac6714(NativeGuiPageConstructionAcquired& prefix,
    NativeGuiPageScriptContinuation& frame, NativeGuiPageScriptContext& context) {
    if (frame.phase_ != NativeGuiPageScriptContinuation::Phase::fresh ||
        prefix.phase != NativeGuiPageConstructionAcquired::Phase::awaiting_lua_continuation ||
        prefix.continuation != 0x00ac6714u || prefix.native_eh_state != 1 || !prefix.actual_page ||
        !context.actual_common_00d5cb64 || !context.paths.actual_interface_prefix_00d5cb18 ||
        !context.paths.actual_lua_suffix_00cfd2c8 || !context.lua.bootstrap().get_fundamentals_00884770 ||
        !context.lua.bootstrap().do_file_00b69e00 || !context.lua.files().append_overrides_00bdef90 ||
        !context.lua.files().do_file_00b69e00)
        throw std::invalid_argument("GUI scripts require a fresh real prefix, path/Lua services and actual literals");
    frame.prefix_ = &prefix;
    frame.phase_ = NativeGuiPageScriptContinuation::Phase::entered;
    prefix.phase = NativeGuiPageConstructionAcquired::Phase::running;
    try {
        NativeLuaServiceBindings::Activation active(context.lua);
        prefix.native_site = 0x00ac6718;
        construct_native_lua_state_00b66bd0(&frame.owner_);
        frame.owner_constructed_ = true;
        prefix.native_eh_state = 2; // AC6723, BEFORE B6A020.
        prefix.native_site = 0x00ac672b;
        open_native_lua_state_00b6a020(frame.owner_, 0x65, context.strings, context.lua.bootstrap());
        frame.open_returned_ = true;

        frame.common_name_entered_ = true;
        prefix.native_site = 0x00ac673f;
        construct_native_string_header_0041e870(frame.common_name_, context.paths.strings,
            context.actual_common_00d5cb64);
        frame.common_name_returned_ = true;
        prefix.native_site = 0x00ac6748;
        compose_native_gui_script_path_00ac5600(&frame.path_, frame.common_name_,
            context.paths, frame.common_path_);
        frame.common_name_adopted_ = true;
        frame.path_cleanup_armed_ = true;
        prefix.native_eh_state = 3;
        prefix.native_site = 0x00ac675a;
        run_native_lua_file_00b69d40(frame.owner_, frame.path_, 0, context.strings, context.lua.files());
        frame.common_run_returned_ = true;
        void* const common_path = path_data(frame.path_); // AC675F, BEFORE state2.
        prefix.native_eh_state = 2;
        frame.path_cleanup_armed_ = false; // Consume BEFORE current length/getter.
        if (common_path) {
            const auto bytes = path_length(frame.path_) + 1u;
            frame.path_cleanup_entered_ = true;
            return_captured_path(common_path, bytes, context.paths.strings, prefix, 0x00ac677a, 0x00ac6781);
            frame.path_cleanup_entered_ = false;
        }

        prefix.native_site = 0x00ac678d;
        compose_native_gui_layer_script_path_00ac5770(prefix.actual_page, &frame.path_,
            context.paths, frame.page_path_);
        frame.path_cleanup_armed_ = true;
        prefix.native_eh_state = 4;
        prefix.native_site = 0x00ac67a0;
        run_native_lua_file_00b69d40(frame.owner_, frame.path_, 0, context.strings, context.lua.files());
        frame.page_run_returned_ = true;
        void* const page_path = path_data(frame.path_); // AC67A5, BEFORE state2.
        prefix.native_eh_state = 2;
        frame.path_cleanup_armed_ = false;
        if (page_path) {
            const auto bytes = path_length(frame.path_) + 1u;
            frame.path_cleanup_entered_ = true;
            return_captured_path(page_path, bytes, context.paths.strings, prefix, 0x00ac67c0, 0x00ac67c7);
            frame.path_cleanup_entered_ = false;
        }
        prefix.native_site = 0x00ac67cc;
        prefix.continuation = 0x00ac67cc;
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::awaiting_reader_continuation;
        frame.phase_ = NativeGuiPageScriptContinuation::Phase::awaiting_reader;
    } catch (...) {
        frame.common_name_adopted_ = frame.common_path_.observation().phase != NativeGuiScriptPathPhase::fresh;
        frame.phase_ = NativeGuiPageScriptContinuation::Phase::failed;
        prefix.phase = NativeGuiPageConstructionAcquired::Phase::failed;
        throw; // Retain SAME owner/headers/child cleanup states, no retry or close.
    }
}
} // namespace bsp
