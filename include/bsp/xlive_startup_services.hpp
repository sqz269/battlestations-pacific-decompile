#pragma once

#include "bsp/xlive_application_callbacks.hpp"
#include "bsp/xlive_ipc.hpp"
#include "bsp/xlive_manager_owner.hpp"
#include "bsp/xlive_startup_adapter.hpp"

#include <d3d9.h>
#include <functional>

namespace bsp {

// Composition of recovered bodies, not an additional native function. The
// application supplies its actual allocation deleter; projected owners borrow
// several independent state objects and cannot be freed as a native 3F0h blob.
class RecoveredXLiveManagerOwnerServices final : public XLiveManagerOwnerHost {
public:
    using FreeOwner = void (*)(XLiveManagerOwner*) noexcept;
    RecoveredXLiveManagerOwnerServices(XLiveIpcPipeHost&, XLiveIpcSystemHost&, FreeOwner);
    std::int32_t create_ipc_00a4c030(void*& output) override;
    void destroy_ipc_00a4bde0(void*) override;
    void free_owner_storage(XLiveManagerOwner*) noexcept override;
private:
    XLiveIpcPipeHost& pipes_;
    XLiveIpcSystemHost& system_;
    FreeOwner free_owner_;
};

// Re-read the single owner slot for every manager access. The application
// profile resolver must likewise read the current game singleton on EACH call.
class BoundXLiveApplicationGlobals final : public XLiveApplicationGlobals {
public:
    using CurrentProfile = std::function<XLiveApplicationProfileBinding()>;
    BoundXLiveApplicationGlobals(XLiveManagerOwner* volatile& published_00f8abe8,
        CurrentProfile);
    XLiveApplicationManagerBinding current_manager_00f8abe8() override;
    XLiveApplicationProfileBinding current_game_profile_00e188a8() override;
private:
    XLiveManagerOwner* volatile& published_;
    CurrentProfile current_profile_;
};

// Every callback returns borrowed, actual renderer storage. In particular the
// SDK receives the real mutable present-parameters object, not a local copy.
// Resolve separately at the two native call sites so singleton changes remain
// observable. All services and returned storage must outlive SDK use.
struct XLiveRendererAccess {
    std::function<IDirect3DDevice9*()> current_device_00b1fef0;
    std::function<D3DPRESENT_PARAMETERS*()> current_present_parameters_1a28;
};

class RecoveredXLiveStartupServices final : public XLiveStartupAdapter {
public:
    RecoveredXLiveStartupServices(const XLiveLibrary&, XLiveRendererAccess,
        XLiveManagerOwnerStorage&, XLiveManagerOwnerHost&,
        XLiveApplicationCallbackInvoker&);
    void* d3d9_device() override;
    void* d3d9_present_parameters() override;
    std::int32_t init_subsystem_3ac() override;
    void invoke_state_callback(const void*) override;
private:
    XLiveRendererAccess renderer_;
    XLiveManagerOwnerStorage& owner_storage_;
    XLiveManagerOwnerHost& owner_services_;
    XLiveApplicationCallbackInvoker& callbacks_;
};

} // namespace bsp
