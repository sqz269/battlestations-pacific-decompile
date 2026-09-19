#include "bsp/native_network_console_runtime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const std::byte* p) noexcept {
    T value; std::memcpy(&value, p, sizeof value); return value;
}
template<class T> void write(std::byte* p, T value) noexcept {
    std::memcpy(p, &value, sizeof value);
}
void enter(TrackedCriticalSection* section) {
    if (section) {
        ::EnterCriticalSection(&section->native);
        auto* depth = reinterpret_cast<std::byte*>(&section->depth);
        write(depth, read<std::uint32_t>(depth) + 1u);
    }
}
void leave(TrackedCriticalSection* section) {
    if (section) {
        auto* depth = reinterpret_cast<std::byte*>(&section->depth);
        write(depth, read<std::uint32_t>(depth) - 1u);
        ::LeaveCriticalSection(&section->native);
    }
}
template<class T> T& member(NativeNetworkConsoleStorage& owner, std::size_t offset) noexcept {
    return *std::launder(reinterpret_cast<T*>(owner.bytes + offset));
}
}
static_assert(sizeof(void*) == 4);
NativeNetworkConsoleSocketImports resolve_native_network_console_socket_imports(HMODULE module) {
    if (!module) throw std::invalid_argument("network console requires the loaded xlive module");
    const auto raw = ::GetProcAddress(module, MAKEINTRESOURCEA(38));
    if (!raw) throw std::runtime_error("xlive ordinal38 XSocketNTOHS is unavailable");
    NativeNetworkConsoleSocketImports::NetworkToHostShort function;
    static_assert(sizeof function == sizeof raw);
    std::memcpy(&function, &raw, sizeof function);
    return {function};
}
const NativeNetworkConsoleThreadImports& native_network_console_thread_imports() noexcept {
    static const NativeNetworkConsoleThreadImports imports{&::Sleep, &::CloseHandle};
    return imports;
}
void write_native_network_console_ack_00a3aab0(
    NativeNetworkConsoleChannelStorage& channel, std::byte* header) noexcept {
    auto* data = channel.bytes;
    if (data[0x29] == std::byte{0}) {
        auto* node = read<std::byte*>(data + 0x48);
        std::uint32_t mask = 0;
        if (node) {
            const auto base = read<std::uint32_t>(data + 0x30);
            do {
                const auto sequence = read<std::uint32_t>(node + 0x1c);
                if (sequence > base) {
                    const auto delta = sequence - base;
                    if (delta <= 24u) mask |= 1u << ((delta - 1u) & 31u);
                }
                node = read<std::byte*>(node);
            } while (node);
        }
        write(data + 0x40, mask);
        data[0x29] = std::byte{1};
    }
    const auto counter = read<std::uint32_t>(data + 0x30);
    const auto mask = read<std::uint32_t>(data + 0x40) & 0xffffffu;
    header[4] = static_cast<std::byte>(counter & 0xffu);
    header[2] |= static_cast<std::byte>((counter >> 4u) & 0x30u);
    header[7] = static_cast<std::byte>((mask >> 16u) & 0xffu);
    header[6] = static_cast<std::byte>((mask >> 8u) & 0xffu);
    header[5] = static_cast<std::byte>(mask & 0xffu);
}
void append_native_network_console_received_00a3ab30(
    NativeNetworkConsoleChannelStorage& channel, NativeNetworkConsoleQueue820Storage& incoming,
    std::uint32_t stream_index) noexcept {
    auto* data = channel.bytes;
    const auto offset = stream_index * 4u;
    auto* tail = read<std::byte*>(data + 0x50 + offset);
    if (tail) write(tail, incoming.bytes);
    else write(data + 0x44 + offset, incoming.bytes);
    const auto expected = read<std::uint32_t>(data + 0x2c + offset) + 1u;
    write(data + 0x50 + offset, incoming.bytes);
    if (read<std::uint32_t>(incoming.bytes + 0x1c) != expected) return;
    auto* const head = read<std::byte*>(data + 0x44 + offset);
    write(data + 0x2c + offset, expected);
    bool changed;
    do {
        changed = false;
        auto* node = head;
        while (node) {
            const auto next = read<std::uint32_t>(data + 0x2c + offset) + 1u;
            if (read<std::uint32_t>(node + 0x1c) == next) {
                write(data + 0x2c + offset, next);
                changed = true;
            }
            node = read<std::byte*>(node);
        }
    } while (changed);
}
std::uint32_t find_native_network_console_channel_00a3bb30(
    NativeNetworkConsoleStorage& owner, const std::byte* address,
    const NativeNetworkConsoleSocketImports& imports) {
    auto* const section = read<TrackedCriticalSection*>(owner.bytes + 0x148);
    enter(section);
    for (std::uint32_t index = 0; index != 16; ++index) {
        const auto* channel = owner.bytes + 0x19c + index * 0x49c;
        if (channel[0] == std::byte{0} &&
            read<std::uint32_t>(channel + 8) == read<std::uint32_t>(address + 4)) {
            leave(section);
            return index;
        }
    }
    (void)imports.network_to_host_short(read<std::uint16_t>(address + 2));
    leave(section);
    return 0xffffffffu;
}
void stop_native_network_console_threads_00a3b6e0(
    NativeNetworkConsoleStorage& owner, const NativeNetworkConsoleStopContext& context) {
    auto* section = read<TrackedCriticalSection*>(owner.bytes + 0x14c);
    enter(section);
    context.quit_00f8abe0 = 1;
    leave(section);
    const auto sleep = context.imports.sleep;
    bool active;
    do {
        sleep(10);
        section = read<TrackedCriticalSection*>(owner.bytes + 0x14c);
        enter(section);
        active = context.send_active_00f8abe1 != 0 || context.receive_active_00f8abe2 != 0;
        leave(section);
    } while (active);
    const auto first = read<HANDLE>(owner.bytes + 0x154);
    const auto close = context.imports.close_handle;
    (void)close(first);
    const auto second = read<HANDLE>(owner.bytes + 0x158);
    (void)close(second);
}
void destroy_native_network_console_00a3d1a0(
    NativeNetworkConsoleStorage& owner, NativeNetworkConsoleBaseContext& context) {
    write(owner.bytes, std::uint32_t{0x00d23f48});
    release_native_tracked_critical_section_0041cc80(
        reinterpret_cast<TrackedCriticalSection**>(owner.bytes + 0x148));
    release_native_tracked_critical_section_0041cc80(
        reinterpret_cast<TrackedCriticalSection**>(owner.bytes + 0x14c));
    for (std::size_t i = 1000; i != 0; --i)
        destroy_native_network_console_queue_00a3a520(
            member<NativeNetworkConsoleQueue820Storage>(owner, 0x2046ec + (i - 1) * 0x820));
    for (std::size_t i = 1000; i != 0; --i)
        destroy_native_network_console_queue_00a3a3e0(
            member<NativeNetworkConsoleQueue82cStorage>(owner, 0x5b08 + (i - 1) * 0x82c));
    for (std::size_t i = 16; i != 0; --i)
        destroy_native_network_console_channel_00a3a770(
            member<NativeNetworkConsoleChannelStorage>(owner, 0x19c + (i - 1) * 0x49c));
    destroy_native_network_console_base_00a3cfa0(owner, context);
}
NativeNetworkConsoleStorage* delete_native_network_console_00a3d260(
    NativeNetworkConsoleStorage& owner, std::uint32_t flags, NativeNetworkConsoleBaseContext& context) {
    auto* const captured = &owner;
    destroy_native_network_console_00a3d1a0(owner, context);
    if (flags & 1u) delete captured;
    return captured;
}
} // namespace bsp
