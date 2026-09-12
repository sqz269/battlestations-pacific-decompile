#include "bsp/xlive_manager_owner.hpp"
#include "bsp/xlive_owner_lifetime.hpp"

#include <algorithm>
#include <memory>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t base_vtable = 0x00d24138;
constexpr std::uint32_t derived_vtable = 0x00d2413c;
constexpr std::uint32_t root_vtable = 0x00ce3818;

void reset_id_vector_slots(OnlineSystemState& online) noexcept {
    // Native constructor writes only the three pointer slots. Placement
    // replacement deliberately does not deallocate a caller-supplied preimage.
    ::new (static_cast<void*>(std::addressof(online.pending_notifications)))
        std::vector<std::uint32_t>();
}

void unwind_derived_members(XLiveManagerOwner& owner) noexcept {
    // A second exception from a destructor during native C++ unwinding is
    // fatal; do not replace the original exception with a cleanup failure.
    destroy_xlive_achievement_ids_00a3f840(owner.context.online);
    destroy_xlive_manager_base_00a3f5d0(owner);
}
}

XLiveManagerLifetimeAccess::XLiveManagerLifetimeAccess(SingletonLifetimeDomain& domain,
    XLiveManagerOwner* volatile& published) noexcept
    : semantic_domain_(&domain), access_(domain), published_(published) {}
XLiveManagerLifetimeAccess::XLiveManagerLifetimeAccess(SoundLifetimeAccess access,
    XLiveManagerOwner* volatile& published) noexcept : access_(access), published_(published) {}
ConcreteSingletonLifetimeManager& XLiveManagerLifetimeAccess::manager_00415350() {
    if (!semantic_domain_)
        throw std::logic_error("legacy XLive manager access requires the semantic-domain constructor");
    auto* manager = semantic_domain_->get_manager_00415350();
    if (!manager) throw std::logic_error("native XLive owner requires its lifetime manager");
    return *manager;
}
SoundLifetimeManagerView XLiveManagerLifetimeAccess::manager_view_00415350() const {
    return access_.get_manager_00415350();
}
void* XLiveManagerLifetimeAccess::published_registration_identity() const {
    auto* const owner = published_;
    if (!owner || !access_.uses_actual_storage()) return owner;
    auto* const allocation = owner->allocation_identity;
    if (!allocation || &allocation->owner() != owner ||
        &allocation->storage() != &owner->storage)
        throw std::logic_error("raw XLive registration requires its canonical owner allocation");
    return allocation->identity();
}
XLiveManagerOwner* XLiveManagerLifetimeAccess::published_owner() const noexcept { return published_; }
void XLiveManagerLifetimeAccess::publish(XLiveManagerOwner* owner) noexcept { published_ = owner; }

std::int32_t initialize_xlive_ipc_slot_00a4c250(void** slot, XLiveManagerOwnerHost& host) {
    if (!slot) return static_cast<std::int32_t>(0x80070057u);
    void* output;
    const auto result = host.create_ipc_00a4c030(output);
    *slot = result < 0 ? nullptr : output;
    return result;
}
void close_xlive_ipc_00a4c280(void* handle, XLiveManagerOwnerHost& host) {
    if (handle && reinterpret_cast<std::uintptr_t>(handle) != static_cast<std::uintptr_t>(-1))
        host.destroy_ipc_00a4bde0(handle);
}

XLiveManagerOwner* construct_xlive_manager_base_00a3f530(XLiveManagerOwner& owner) {
    owner.storage.vtable_00 = base_vtable;
    try {
        CapturedSoundLifetimeSection lock(owner.lifetime.lifetime_access());
        owner.lifetime.publish(&owner);
        auto manager = owner.lifetime.manager_view_00415350();
        manager.register_object(owner.lifetime.published_registration_identity());
    } catch (...) {
        owner.storage.vtable_00 = root_vtable;
        throw;
    }
    return &owner;
}

void destroy_xlive_manager_base_00a3f5d0(XLiveManagerOwner& owner) {
    owner.storage.vtable_00 = base_vtable;
    try {
        CapturedSoundLifetimeSection lock(owner.lifetime.lifetime_access());
        auto manager = owner.lifetime.manager_view_00415350();
        manager.unregister_object(owner.lifetime.published_registration_identity());
        owner.lifetime.publish(nullptr);
    } catch (...) {
        owner.storage.vtable_00 = root_vtable;
        throw;
    }
    owner.storage.vtable_00 = root_vtable;
}

void destroy_xlive_achievement_ids_00a3f840(OnlineSystemState& online) noexcept {
    std::vector<std::uint32_t>().swap(online.pending_notifications);
}

XLiveManagerOwner* construct_xlive_manager_00a40df0(XLiveManagerOwner& owner,
    const void* callback20, const void* callback24) {
    construct_xlive_manager_base_00a3f530(owner);
    auto& c = owner.context;
    auto& o = c.online;
    auto& p = c.pump;
    auto& flags = c.flags;
    auto& s = owner.signin;
    auto& fields = owner.storage;
    auto& startup = c.startup_host;
    // All assignments up to vector construction are nonthrowing native stores.
    fields.vtable_00 = derived_vtable;
    s.debounce_pending_04 = false;
    s.debounce_ticks_08 = 0;
    s.debounce_frequency_high_14 = 0;
    o.field_10 = 1;
    s.callback_18 = nullptr;
    o.callback_20 = callback20;
    o.callback_24 = callback24;
    s.flag_2d = 0;
    s.flag_2e = 0;
    flags.storage_removed = false;
    flags.content_installed = false;
    flags.invite_accepted = false;
    fields.byte_86 = 0;
    fields.byte_87 = 0;
    fields.float_bits_88 = 0;
    o.signin_flag_119 = false;
    p.signin_flag_120 = false;
    reset_id_vector_slots(o);
    try {
        p.signin_flag_3bc = false;
        o.flag_3bd = false;
        // 004254B0 is a verified bare RET. Do not introduce a host callback
        // between these stores or make native diagnostic no-ops throw.
        flags.system_ui_visible = false;
        fields.byte_3e9 = 0;
        static_cast<void>(p.achievements_3a0.release());
        std::fill_n(p.achievements_overlapped_384.words.begin(), 5, 0u);
        p.achievement_count_3a4 = 0;
        p.achievement_result_3a8 = 0;

        XLiveInitializeInfo info;
        info.size_bytes = 0x1c;
        info.d3d_device = startup.d3d9_device();
        info.d3d_present_parameters = startup.d3d9_present_parameters();
        info.language_id = startup.user_default_lang_id();
        o.xlive_initialize_result = startup.xlive_initialize_ex(info, kXLiveInitializeVersion);
        o.subsystem_3ac_result = initialize_xlive_ipc_slot_00a4c250(&fields.ipc_3ac, owner.host);
        startup.x_online_startup();
        // Native ignores XWSAStartup's result and reads its output even on
        // failure. The required adapter must provide that actual output word
        // or report unavailable output; no fabricated zero is supplied here.
        std::uint16_t negotiated;
        startup.x_wsa_startup(kXLiveWinsockVersion, &negotiated);
        o.winsock_started = negotiated == 0x0202;
        if (!o.winsock_started) startup.x_wsa_cleanup();
        o.system_link_port = startup.x_socket_ntohs(kSystemLinkPortLiteral);
        startup.x_net_set_system_link_port(o.system_link_port);
        flags.profile_overlapped_3c0.words.fill(0);
        o.connected_flag = 0;
        flags.link_failure = false;
        o.notification_listener = startup.x_notify_create_listener(kNotifyListenerAreas);
        o.notification_listener_valid = o.notification_listener &&
            reinterpret_cast<std::uintptr_t>(o.notification_listener) != static_cast<std::uintptr_t>(-1);
        reset_online_signin_state(startup, o);
        pump_xlive_system_00a409f0(c);
        static_cast<void>(p.storage_buffer_14c.release());
        p.storage_state_12c = 0;
    } catch (...) {
        unwind_derived_members(owner);
        throw;
    }
    return &owner;
}

void destroy_xlive_manager_00a3f9d0(XLiveManagerOwner& owner) {
    owner.storage.vtable_00 = derived_vtable;
    try {
        close_xlive_ipc_00a4c280(owner.storage.ipc_3ac, owner.host);
        owner.context.pump.storage_buffer_14c.reset();
    } catch (...) {
        unwind_derived_members(owner);
        throw;
    }
    destroy_xlive_achievement_ids_00a3f840(owner.context.online);
    destroy_xlive_manager_base_00a3f5d0(owner);
}

XLiveManagerOwner* delete_xlive_manager_00a3fdc0(XLiveManagerOwner& owner, std::uint8_t flags) {
    auto* const result = &owner;
    destroy_xlive_manager_00a3f9d0(owner);
    if (flags & 1) owner.host.free_owner_storage(result);
    return result;
}
XLiveManagerOwner* delete_xlive_manager_base_00a3f670(XLiveManagerOwner& owner, std::uint8_t flags) {
    auto* const result = &owner;
    destroy_xlive_manager_base_00a3f5d0(owner);
    if (flags & 1) owner.host.free_owner_storage(result);
    return result;
}
} // namespace bsp
