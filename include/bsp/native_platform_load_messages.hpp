#pragma once
#include "bsp/native_input_cursor.hpp"
#include "bsp/native_online_pump.hpp"
#include "bsp/xlive_library.hpp"

namespace bsp {
// Borrow the current actual publications and their existing canonical services.
// The online context must describe the actual manager captured at A409F0.
// Input calls keep their established actual backend/device/COM contracts; this
// adapter never reads their projected platform-manager accessors.
struct NativePlatformLoadMessagesContext {
    NativeOnlineManagerStorage* volatile& actual_online_00f8abe8;
    void* volatile& actual_input_00f8bbf4;
    NativeOnlinePumpContext* online;
    NativeInputCursorContext* input;
    XLiveLibrary& xlive;
};
// Empty means the original unwritten MSG output storage. PeekMessageA is its
// first producer; consumers are reached only after a successful retrieval.
// Explicit captures remain available for qualified byte-preimage observations.
// This pointer is metadata and does not initialize or default any MSG byte.
struct NativePlatformLoadMessagePreimage {
    const MSG* captured{};
    NativePlatformLoadMessagePreimage() noexcept = default;
    NativePlatformLoadMessagePreimage(const MSG& value) noexcept : captured(&value) {}
};
// Borrow one existing pointer cell, including the renderer's const-qualified
// focus publication. Neither overload publishes, copies a receiver, or creates
// another cell. The reached load pump reads platform+41 only; a focus-domain
// receiver must never be passed to other whole-platform bodies.
class NativePlatformLoadPublication final {
public:
    NativePlatformLoadPublication(void* const volatile& cell) noexcept : mutable_(&cell) {}
    NativePlatformLoadPublication(const void* const volatile& cell) noexcept : constant_(&cell) {}
    void* load() const noexcept {
        return mutable_ ? *mutable_ : const_cast<void*>(*constant_);
    }
    const volatile void* identity() const noexcept {
        return mutable_ ? static_cast<const volatile void*>(mutable_)
                        : static_cast<const volatile void*>(constant_);
    }
private:
    void* const volatile* mutable_{};
    const void* const volatile* constant_{};
};
struct NativePlatformLoadMessagesOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    void* captured_platform{};
    NativePlatformLoadMessagesContext* context{};
    NativeOnlineManagerStorage* captured_online{};
    std::uint32_t native_site{};
    MSG message; // Unwritten output storage until successful PeekMessageA.
    NativePlatformLoadMessagesOperation() noexcept {}
    ~NativePlatformLoadMessagesOperation();
    NativePlatformLoadMessagesOperation(const NativePlatformLoadMessagesOperation&)=delete;
    NativePlatformLoadMessagesOperation& operator=(const NativePlatformLoadMessagesOperation&)=delete;
    // Only after resolving this frame and the borrowed online/input obligations.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Null service pointers are permitted only on native guard paths that do not
// reach those services. A reached missing/mismatched service is a host error.
// Actual captured receiver, current raw platform+41/online+3E8, exact byte
// globals and existing actual-input bodies. ECX platform, stack loading, RET4.
void update_native_platform_load_cursor_00becb20(void* actual_platform,
    std::uint8_t loading,NativePlatformLoadMessagesContext&,NativePlatformLoadMessagesOperation&);
// ECX captured actual platform, RET. Real Win32 thread messages and actual
// XLive ordinal5030 (BOOL stdcall(MSG*)); no WM_QUIT special case/frame callback.
// No input MSG is required: the original allocates unwritten local output
// storage and reads it only after PeekMessageA succeeds. No implicit UI owner.
void pump_native_platform_load_messages_00beccd0(void* actual_platform,
    NativePlatformLoadMessagesContext&,NativePlatformLoadMessagesOperation&);
} // namespace bsp
