#pragma once

#include "bsp/audio_online_startup.hpp"

#include <cstdint>
#include <stdexcept>

namespace bsp {
class XLiveLibrary;

// Native A40F77 reads wVersion even if XWSAStartup returned an error. The
// adapter cannot establish output validity on that path, so it does not feed
// invented bytes into the version check. The actual error remains available.
class XLiveWsaStartupOutputError final : public std::runtime_error {
public:
    explicit XLiveWsaStartupOutputError(std::int32_t result);
    std::int32_t sdk_result() const noexcept { return result_; }
private:
    std::int32_t result_;
};

// Audited DLL/Win32 forwarding for the existing startup interface. Renderer
// pointers, init_subsystem_3ac and actual state callback invocation remain
// required inherited methods. No library load or initialization occurs in the
// constructor. The borrowed XLiveLibrary must outlive this adapter, all startup
// state and every listener/SDK object obtained through it.
class XLiveStartupAdapter : public XLiveStartupHost {
public:
    explicit XLiveStartupAdapter(const XLiveLibrary& library);
    ~XLiveStartupAdapter() override = default;
    XLiveStartupAdapter(const XLiveStartupAdapter&) = delete;
    XLiveStartupAdapter& operator=(const XLiveStartupAdapter&) = delete;

    std::uint16_t user_default_lang_id() override;
    std::int32_t xlive_initialize_ex(const XLiveInitializeInfo& info,
        std::uint32_t version) override;
    void x_online_startup() override;
    // Supplies the SDK with the complete Win32 WSADATA, then projects its
    // wVersion only on result0. Nonzero result throws the typed guard above;
    // out_version is untouched. out_version must be nonnull.
    std::int32_t x_wsa_startup(std::uint16_t requested_version,
        std::uint16_t* out_version) override;
    void x_wsa_cleanup() override;
    std::uint16_t x_socket_ntohs(std::uint16_t value) override;
    void x_net_set_system_link_port(std::uint16_t port) override;
    void* x_notify_create_listener(std::uint64_t areas) override;

    // Native diagnostic target004254B0 is exactly RET. These empty bodies
    // reconstruct that proven behavior; they are not missing SDK defaults.
    void log(const char* message) override;
    void log_state_change(std::uint32_t state) override;

private:
    void* module_; // borrowed HMODULE, never freed here
};
} // namespace bsp
