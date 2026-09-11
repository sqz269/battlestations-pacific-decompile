#pragma once

#include "bsp/xlive_pipe_protocol.hpp"
#include <type_traits>

namespace bsp {

// Actual original pre-startup image. These values are overwritten by native
// constructors; they are not substitutes for initialized encoded payloads.
struct XLivePipeGlobalsPreimage {
    std::array<std::uint8_t, 0x1c> acquisition;
    XLivePipeEncodedWideValue value_f8b9a0;
    XLivePipeEncodedWideValue value_f8b858;
    XLivePipeEncodedValue seed_f8b8cc;
    XLivePipeEncodedValue value_f8bba0;
    XLivePipeEncodedValue key_f8badc;
    std::array<std::uint8_t, 72> raw_f8b8f8;
};
// Proven original PE .data loader zero-fill: raw ends RVA A18000, all these
// globals are in the virtual tail. Separate from unknown protocol heap bytes.
XLivePipeGlobalsPreimage original_xlive_pipe_loader_preimage() noexcept;

// Immutable original fixed sources E12AC8..E12D00 (568 bytes), checked SHA-256
// 20436ee57ad0c179e4cde3ffd80ea75b53339dea4d500cda75769177176eae21.
// Borrowed backing bytes must remain alive and unchanged throughout startup.
class XLivePipeGlobalData {
public:
    static constexpr std::uint32_t first_va = 0x00e12ac8;
    static constexpr std::size_t size = 568;
    XLivePipeGlobalData(const std::uint8_t*, std::size_t);
    const std::uint8_t* bytes(std::uint32_t va, std::size_t count) const;
private:
    const std::uint8_t* data_;
};

class XLivePipeGlobalsOwner;
using XLivePipeGlobalCleanup = void (*)(XLivePipeGlobalsOwner&);
struct XLivePipeGlobalsStartupHost {
    virtual ~XLivePipeGlobalsStartupHost() = default;
    // Must allocate the actual value lock (normally the recovered FA5A helper).
    virtual XLivePipeValueLock* create_value_lock_00a5fa5a() = 0;
    // Genuine CRT registration boundary. Bind callback to this retained owner,
    // preserve the real registration result, and invoke it in actual CRT order.
    // No automatic callbacks or successful registration default is supplied.
    virtual int register_cleanup_00bf6ff5(std::uint32_t native_address,
        XLivePipeGlobalCleanup callback, XLivePipeGlobalsOwner&) = 0;
};

// One stable-address owned domain shared by protocol and framing. Storage
// lifetime must include all registered CRT callbacks and active consumers.
// Destruction does not add native cleanup: only recovered callbacks destroy
// locks. Reinitialization/double-cleanup is outside native valid lifetime.
class XLivePipeGlobalsOwner {
public:
    explicit XLivePipeGlobalsOwner(const XLivePipeGlobalsPreimage&);
    XLivePipeGlobalsOwner(const XLivePipeGlobalsOwner&) = delete;
    XLivePipeGlobalsOwner& operator=(const XLivePipeGlobalsOwner&) = delete;
    Win32XLivePipeValueLock& construct_acquisition_object();
    Win32XLivePipeValueLock& acquisition_object(); // only while constructed
    CRITICAL_SECTION& acquisition_lock_f8b778();
    XLivePipeProtocolGlobals protocol_globals();

    XLivePipeEncodedWideValue value_f8b9a0;
    XLivePipeEncodedWideValue value_f8b858;
    XLivePipeEncodedValue seed_f8b8cc;
    XLivePipeEncodedValue value_f8bba0;
    XLivePipeEncodedValue key_f8badc;
    std::array<std::uint8_t, 72> raw_f8b8f8;
private:
    std::aligned_storage_t<sizeof(Win32XLivePipeValueLock), alignof(Win32XLivePipeValueLock)> acquisition_storage_;
};

// Native ECX=destination, stack source pointer, RET4/EAX=destination. Allocate
// and store lock before the complete forward byte copy. Preserve null results,
// aliasing, callbacks and exceptions; do not clear/replace old preimage first.
XLivePipeEncodedValue* construct_xlive_pipe_seed_global_00a60015(
    XLivePipeEncodedValue&, const std::uint8_t*, XLivePipeGlobalsStartupHost&);
XLivePipeEncodedValue* construct_xlive_pipe_key_global_00a5ffcf(
    XLivePipeEncodedValue&, const std::uint8_t*, XLivePipeGlobalsStartupHost&);
XLivePipeEncodedValue* construct_xlive_pipe_frame_value_00a5fff2(
    XLivePipeEncodedValue&, const std::uint8_t*, XLivePipeGlobalsStartupHost&);
XLivePipeEncodedWideValue* construct_xlive_pipe_frame_value_00a60038(
    XLivePipeEncodedWideValue&, const std::uint8_t*, XLivePipeGlobalsStartupHost&);
XLivePipeEncodedWideValue* construct_xlive_pipe_frame_value_00a5ffac(
    XLivePipeEncodedWideValue&, const std::uint8_t*, XLivePipeGlobalsStartupHost&);
// A60095 ECX=raw72, RET; A600AB ECX=raw72, stacksource, RET4/EAX=raw72.
void initialize_xlive_pipe_raw_constant_00a60095(std::uint8_t*, const XLivePipeProtocolTables&);
std::uint8_t* construct_xlive_pipe_raw_global_00a600ab(
    std::uint8_t*, const std::uint8_t*, const XLivePipeProtocolTables&);

void cleanup_xlive_pipe_acquisition_00ce099a(XLivePipeGlobalsOwner&);
void cleanup_xlive_pipe_frame_value_00ce09a4(XLivePipeGlobalsOwner&);
void cleanup_xlive_pipe_frame_value_00ce09b5(XLivePipeGlobalsOwner&);
void cleanup_xlive_pipe_seed_00ce09c6(XLivePipeGlobalsOwner&);
void cleanup_xlive_pipe_frame_value_00ce09d7(XLivePipeGlobalsOwner&);
void cleanup_xlive_pipe_key_00ce09e8(XLivePipeGlobalsOwner&);

// Native no-argument initializer bodies: constructor then actual CRT register;
// RET/EAX=registration result. Failure does not undo construction or stop CRT.
int initialize_xlive_pipe_acquisition_global_00cd6e16(XLivePipeGlobalsOwner&, XLivePipeGlobalsStartupHost&);
int initialize_xlive_pipe_frame_global_00cd6e2c(XLivePipeGlobalsOwner&, const XLivePipeGlobalData&, XLivePipeGlobalsStartupHost&);
int initialize_xlive_pipe_frame_global_00cd6e47(XLivePipeGlobalsOwner&, const XLivePipeGlobalData&, XLivePipeGlobalsStartupHost&);
int initialize_xlive_pipe_seed_global_00cd6e62(XLivePipeGlobalsOwner&, const XLivePipeGlobalData&, XLivePipeGlobalsStartupHost&);
int initialize_xlive_pipe_frame_global_00cd6e7d(XLivePipeGlobalsOwner&, const XLivePipeGlobalData&, XLivePipeGlobalsStartupHost&);
int initialize_xlive_pipe_key_global_00cd6e98(XLivePipeGlobalsOwner&, const XLivePipeGlobalData&, XLivePipeGlobalsStartupHost&);
// Native RET/EAX=raw destination; no registration for this object.
std::uint8_t* initialize_xlive_pipe_raw_global_00cd6f03(XLivePipeGlobalsOwner&,
    const XLivePipeGlobalData&, const XLivePipeProtocolTables&);

// Projection convenience: execute this owned subset in original relative CRT
// order. Does not claim the intervening unrelated global initializer sequence.
std::array<int, 6> initialize_xlive_pipe_globals(XLivePipeGlobalsOwner&,
    const XLivePipeGlobalData&, const XLivePipeProtocolTables&, XLivePipeGlobalsStartupHost&);
} // namespace bsp
