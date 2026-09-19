#include "bsp/native_network_console_storage.hpp"
#include <cstring>
#include <new>
#include <type_traits>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeNetworkConsoleSlotStorage) == 0x10);
static_assert(sizeof(NativeNetworkConsoleChannelStorage) == 0x49c);
static_assert(sizeof(NativeNetworkConsoleQueue82cStorage) == 0x82c);
static_assert(sizeof(NativeNetworkConsoleQueue820Storage) == 0x820);
static_assert(std::is_trivially_default_constructible_v<NativeNetworkConsoleChannelStorage>);
namespace {
void word(void* owner, std::size_t offset, std::uint32_t value) noexcept {
    std::memcpy(static_cast<std::byte*>(owner) + offset, &value, sizeof value);
}
NativeNetworkConsoleSlotStorage& slot(NativeNetworkConsoleChannelStorage& channel,
    std::size_t index) noexcept {
    return *std::launder(reinterpret_cast<NativeNetworkConsoleSlotStorage*>(
        channel.bytes + 0x8c + index * 0x10));
}
}
NativeNetworkConsoleSlotStorage* construct_native_network_console_slot_00a3a330(
    NativeNetworkConsoleSlotStorage& owner) noexcept {
    owner.bytes[0] = std::byte{0};
    word(&owner, 4, 0);
    return &owner;
}
NativeNetworkConsoleQueue82cStorage* construct_native_network_console_queue_00a3a3d0(
    NativeNetworkConsoleQueue82cStorage& owner) noexcept {
    word(&owner, 0, reinterpret_cast<std::uint32_t>(owner.bytes + sizeof owner));
    return &owner;
}
NativeNetworkConsoleQueue820Storage* construct_native_network_console_queue_00a3a500(
    NativeNetworkConsoleQueue820Storage& owner) noexcept {
    word(&owner, 0, reinterpret_cast<std::uint32_t>(owner.bytes + sizeof owner));
    word(&owner, 0x1c, 0);
    return &owner;
}
void destroy_native_network_console_slot_00a3a340(NativeNetworkConsoleSlotStorage&) noexcept {}
void destroy_native_network_console_queue_00a3a3e0(NativeNetworkConsoleQueue82cStorage&) noexcept {}
void destroy_native_network_console_queue_00a3a520(NativeNetworkConsoleQueue820Storage&) noexcept {}

void reset_native_network_console_channel_00a3ad10(NativeNetworkConsoleChannelStorage& owner,
    const NativeNetworkConsoleChannelContext& context) {
    // Explicit source stack input, not an original-game stack identity claim.
    ClockTimestamp output = context.sample_stack_preimage;
    owner.bytes[0] = std::byte{1};
    owner.bytes[0x18] = std::byte{0};
    const auto* sampled = sample_published_native_frame_clock(context.clock, &output);
    auto* destination = &owner;
    const auto* reset_bits = &context.reset_interval_bits_00d7a260;
    // A3AD31..A3ADE4: keep arithmetic, exception timing and the scalar-store
    // sequence in one block. In particular, do not round through a double or
    // move the +20 float store ahead of the intervening twelve DWORD clears.
    __asm {
        mov esi, destination
        xor edx, edx
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        xorps xmm0, xmm0
        mov byte ptr [esi + 29h], 1
        mov byte ptr [esi + 28h], 1
        fdivp st(1), st(0)
        mov dword ptr [esi + 38h], edx
        mov dword ptr [esi + 34h], edx
        mov dword ptr [esi + 30h], edx
        mov dword ptr [esi + 2ch], edx
        mov dword ptr [esi + 40h], edx
        mov dword ptr [esi + 3ch], edx
        mov dword ptr [esi + 60h], edx
        mov dword ptr [esi + 5ch], edx
        mov dword ptr [esi + 68h], edx
        mov dword ptr [esi + 64h], edx
        mov dword ptr [esi + 70h], edx
        mov dword ptr [esi + 6ch], edx
        lea eax, [esi + 90h]
        mov ecx, 20h
        fstp dword ptr [esi + 20h]
        mov dword ptr [esi + 74h], edx
        mov dword ptr [esi + 80h], edx
        mov dword ptr [esi + 44h], edx
        mov dword ptr [esi + 50h], edx
        mov dword ptr [esi + 78h], edx
        mov dword ptr [esi + 84h], edx
        mov dword ptr [esi + 48h], edx
        mov dword ptr [esi + 54h], edx
        mov dword ptr [esi + 7ch], edx
        mov dword ptr [esi + 88h], edx
        mov dword ptr [esi + 4ch], edx
        mov dword ptr [esi + 58h], edx
        movss dword ptr [esi + 490h], xmm0
        mov dword ptr [esi + 48ch], edx
        mov dword ptr [esi + 494h], edx
        movss dword ptr [esi + 498h], xmm0
    reset_slots:
        mov byte ptr [eax - 4], dl
        mov dword ptr [eax], edx
        mov byte ptr [eax + 1fch], dl
        mov dword ptr [eax + 200h], edx
        add eax, 10h
        sub ecx, 1
        jnz reset_slots
        mov eax, reset_bits
        movss xmm0, dword ptr [eax]
        mov dword ptr [esi + 14h], edx
        movss dword ptr [esi + 24h], xmm0
    }
}
NativeNetworkConsoleChannelStorage* construct_native_network_console_channel_00a3ce60(
    NativeNetworkConsoleChannelStorage& owner, const NativeNetworkConsoleChannelContext& context) {
    for (std::size_t i = 0; i < 64; ++i) {
        auto* element = ::new (owner.bytes + 0x8c + i * 0x10) NativeNetworkConsoleSlotStorage;
        construct_native_network_console_slot_00a3a330(*element);
    }
    try { reset_native_network_console_channel_00a3ad10(owner, context); }
    catch (...) { destroy_native_network_console_channel_00a3a770(owner); throw; }
    return &owner;
}
void destroy_native_network_console_channel_00a3a770(NativeNetworkConsoleChannelStorage& owner) noexcept {
    for (std::size_t i = 64; i != 0; --i) destroy_native_network_console_slot_00a3a340(slot(owner, i - 1));
}
} // namespace bsp
