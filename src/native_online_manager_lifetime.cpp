#include "bsp/native_online_manager_lifetime.hpp"

#include "bsp/native_shader_device_reset.hpp"

#include <cstring>
#include <exception>
#include <initializer_list>

namespace bsp {
namespace {
template<class T> T read(const void* object, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(object) + offset, sizeof(T));
    return result;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(T));
}
void zero32(void* object, std::size_t offset) noexcept {
    write(object, offset, std::uint32_t{0});
}
void zero8(void* object, std::size_t offset) noexcept {
    write(object, offset, std::uint8_t{0});
}
void* vector_header(NativeOnlineManagerStorage& owner) noexcept {
    return reinterpret_cast<std::byte*>(&owner) + 0x360;
}

// The original FH3 states retain base destruction after construction returns,
// then add the embedded vector cleanup. C++ cleanup may not throw while an
// exception is already unwinding; that source limitation is explicit here.
struct BaseCleanup final {
    NativeOnlineManagerStorage& owner;
    NativeOnlineManagerBaseContext& context;
    bool armed = true;
    ~BaseCleanup() noexcept {
        if (armed) {
            try { destroy_native_online_manager_base_00a3f5d0(owner, context); }
            catch (...) { std::terminate(); }
        }
    }
};
struct VectorCleanup final {
    NativeOnlineManagerStorage& owner;
    const NativeOnlineStorageMemory& memory;
    bool armed = true;
    ~VectorCleanup() noexcept {
        if (armed) {
            try { destroy_native_online_vector_00a3f840(vector_header(owner), memory); }
            catch (...) { std::terminate(); }
        }
    }
};
} // namespace

void destroy_native_online_vector_00a3f840(void* header,
    const NativeOnlineStorageMemory& memory) {
    void* const buffer = read<void*>(header, 4);
    if (buffer) memory.release(buffer);
    zero32(header, 4);
    zero32(header, 8);
    zero32(header, 0xc);
}

NativeOnlineManagerStorage* construct_native_online_manager_00a40df0(
    NativeOnlinePumpContext& pump, std::uint32_t callback20, std::uint32_t callback24,
    NativeOnlineManagerStartupContext& c) {
    auto& owner = pump.notifications.manager;
    construct_native_online_manager_base_00a3f530(owner, c.lifetime.base);
    write(&owner, 0, std::uint32_t{0x00d2413c});
    zero8(&owner, 4);
    zero32(&owner, 8);
    zero32(&owner, 0xc);
    zero32(&owner, 0x14);
    write(&owner, 0x10, std::uint32_t{1});
    zero32(&owner, 0x18);
    write(&owner, 0x20, callback20);
    write(&owner, 0x24, callback24);
    for (const auto offset : {0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x86, 0x87})
        zero8(&owner, offset);
    zero32(&owner, 0x88); // MOVSS of the XORPS zero bits.
    zero8(&owner, 0x119);
    zero8(&owner, 0x120);
    BaseCleanup base_cleanup{owner, c.lifetime.base}; // A40E7D: state0.
    zero32(&owner, 0x364);
    zero32(&owner, 0x368);
    zero32(&owner, 0x36c);
    VectorCleanup vector_cleanup{owner, c.lifetime.memory}; // A40E9B: state1.
    zero8(&owner, 0x3bc);
    zero8(&owner, 0x3bd);
    // 004254B0 is a verified bare RET diagnostic sink.
    zero8(&owner, 0x3e8);
    zero8(&owner, 0x3e9);
    zero32(&owner, 0x3a0);
    for (const auto offset : {0x384, 0x388, 0x38c, 0x390, 0x394, 0x3a4, 0x3a8})
        zero32(&owner, offset);

    void* const renderer_for_device = c.current_renderer_00f8d394;
    auto& info = c.initialize_info;
    for (std::size_t offset = 4; offset != 0x1c; offset += 4) zero32(&info, offset);
    info.size_00 = 0x1c;
    info.d3d_device_08 = get_native_renderer_device_00b1fef0(renderer_for_device);
    info.present_parameters_0c = static_cast<std::byte*>(c.current_renderer_00f8d394) + 0x1a28;
    info.language_id_10 = c.sdk.user_default_lang_id();
    (void)c.sdk.xlive_initialize_ex(&info, 0x20029900);
    (void)initialize_native_online_ipc_slot_00a4c250(
        reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::byte*>(&owner) + 0x3ac),
        c.lifetime.ipc);
    (void)c.sdk.x_online_startup();
    (void)c.sdk.x_wsa_startup(0x202, &c.wsadata);
    const auto version = c.wsadata.version_00;
    const auto low = static_cast<std::uint8_t>(version);
    if (low != 2 || static_cast<std::uint8_t>(version >> 8) != low)
        (void)c.sdk.x_wsa_cleanup();
    const auto port = c.sdk.x_socket_ntohs(0x0c02);
    (void)c.sdk.x_net_set_system_link_port(port);
    for (const auto offset : {0x3c0, 0x3c4, 0x3c8, 0x3cc, 0x3d0, 0x3d4, 0x3d8})
        zero32(&owner, offset);
    zero32(&owner, 0x8c);
    zero8(&owner, 0x128);
    write(&owner, 0x1c, c.sdk.x_notify_create_listener(0x2f, 0));
    reset_native_online_signin_state_00a40020(owner, pump.notifications.signin, pump.ui_crt);
    pump_native_online_00a409f0(pump);
    // These stores occur only after the pump returns. They abandon any prior
    // +14C value, including a value just installed by a nested pump callback.
    zero32(&owner, 0x14c);
    zero32(&owner, 0x12c);
    vector_cleanup.armed = false;
    base_cleanup.armed = false;
    return &owner;
}

void destroy_native_online_manager_00a3f9d0(NativeOnlineManagerStorage& owner,
    NativeOnlineManagerLifetimeContext& c) {
    BaseCleanup base_cleanup{owner, c.base};
    VectorCleanup vector_cleanup{owner, c.memory};
    write(&owner, 0, std::uint32_t{0x00d2413c});
    close_native_online_ipc_handle_00a4c280(read<std::uint32_t>(&owner, 0x3ac), c.ipc);
    if (read<void*>(&owner, 0x14c)) {
        void* const buffer = read<void*>(&owner, 0x14c);
        if (buffer) {
            c.memory.release(buffer);
            zero32(&owner, 0x14c);
        }
    }
    destroy_native_online_vector_00a3f840(vector_header(owner), c.memory);
    vector_cleanup.armed = false;
    base_cleanup.armed = false; // A3FA51: state -1 before the base call.
    destroy_native_online_manager_base_00a3f5d0(owner, c.base);
}

NativeOnlineManagerStorage* delete_native_online_manager_00a3fdc0(
    NativeOnlineManagerStorage& owner, std::uint32_t flags,
    NativeOnlineManagerLifetimeContext& c) {
    auto* const captured = &owner;
    destroy_native_online_manager_00a3f9d0(owner, c);
    if ((flags & 1u) != 0) delete captured;
    return captured;
}
} // namespace bsp
