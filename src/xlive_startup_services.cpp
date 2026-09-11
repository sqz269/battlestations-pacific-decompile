#include "bsp/xlive_startup_services.hpp"

#include <stdexcept>
#include <utility>

namespace bsp {

RecoveredXLiveManagerOwnerServices::RecoveredXLiveManagerOwnerServices(
    XLiveIpcPipeHost& pipes, XLiveIpcSystemHost& system, FreeOwner free_owner)
    : pipes_(pipes), system_(system), free_owner_(free_owner) {
    if (free_owner_ == nullptr) throw std::invalid_argument("XLive owner requires its actual allocation deleter");
}

std::int32_t RecoveredXLiveManagerOwnerServices::create_ipc_00a4c030(void*& output) {
    XLiveIpc* created;
    const auto result = bsp::create_xlive_ipc_00a4c030(&created, pipes_, system_);
    // The recovered function writes its output on every normal return. Its
    // later failures exit/throw before this publication; output stays intact.
    output = created;
    return result;
}

void RecoveredXLiveManagerOwnerServices::destroy_ipc_00a4bde0(void* ipc) {
    bsp::destroy_xlive_ipc_00a4bde0(static_cast<XLiveIpc*>(ipc));
}

void RecoveredXLiveManagerOwnerServices::free_owner_storage(XLiveManagerOwner* owner) noexcept {
    free_owner_(owner);
}

BoundXLiveApplicationGlobals::BoundXLiveApplicationGlobals(
    XLiveManagerOwner* volatile& published, CurrentProfile current_profile)
    : published_(published), current_profile_(std::move(current_profile)) {
    if (!current_profile_) throw std::invalid_argument("XLive callbacks require current game profile access");
}

XLiveApplicationManagerBinding BoundXLiveApplicationGlobals::current_manager_00f8abe8() {
    auto* owner = published_;
    if (owner == nullptr) throw std::logic_error("XLive callback requires a published manager");
    return {owner->context.online, owner->signin};
}

XLiveApplicationProfileBinding BoundXLiveApplicationGlobals::current_game_profile_00e188a8() {
    return current_profile_();
}

RecoveredXLiveStartupServices::RecoveredXLiveStartupServices(const XLiveLibrary& library,
    XLiveRendererAccess renderer, XLiveManagerOwnerStorage& storage,
    XLiveManagerOwnerHost& services, XLiveApplicationCallbackInvoker& callbacks)
    : XLiveStartupAdapter(library), renderer_(std::move(renderer)),
      owner_storage_(storage), owner_services_(services), callbacks_(callbacks) {
    if (!renderer_.current_device_00b1fef0 || !renderer_.current_present_parameters_1a28)
        throw std::invalid_argument("XLive startup requires actual renderer accessors");
}

void* RecoveredXLiveStartupServices::d3d9_device() {
    return renderer_.current_device_00b1fef0();
}

void* RecoveredXLiveStartupServices::d3d9_present_parameters() {
    return renderer_.current_present_parameters_1a28();
}

std::int32_t RecoveredXLiveStartupServices::init_subsystem_3ac() {
    return initialize_xlive_ipc_slot_00a4c250(&owner_storage_.ipc_3ac, owner_services_);
}

void RecoveredXLiveStartupServices::invoke_state_callback(const void* callback) {
    callbacks_.invoke_state_callback(callback);
}

} // namespace bsp
