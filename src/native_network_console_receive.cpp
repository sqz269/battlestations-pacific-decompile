#include "bsp/native_network_console_receive.hpp"
#include <cstring>

namespace bsp {
namespace {
template<class T> T read(const std::byte* p) noexcept {
    T value; std::memcpy(&value, p, sizeof value); return value;
}
template<class T> void write(std::byte* p, T value) noexcept {
    std::memcpy(p, &value, sizeof value);
}
}
std::uint32_t decode_native_network_console_sequence_00a3a6b0(
    const std::byte* header, std::uint32_t reference) noexcept {
    auto sequence = (reference & 0xfffffc00u) |
        ((std::to_integer<std::uint32_t>(header[2]) & 0xc0u) << 2u) |
        std::to_integer<std::uint32_t>(header[3]);
    if (sequence >= 0x400u && sequence > reference + 0x200u) return sequence - 0x400u;
    if (reference >= 0x200u && sequence < reference - 0x200u) sequence += 0x400u;
    return sequence;
}
NativeNetworkConsoleQueue82cStorage* process_native_network_console_ack_00a3b420(
    NativeNetworkConsoleChannelStorage& channel, const std::byte* header,
    const NativeNetworkConsoleReceiveContext& context) {
    const auto mask = std::to_integer<std::uint32_t>(header[5]) |
        (std::to_integer<std::uint32_t>(header[6]) << 8u) |
        (std::to_integer<std::uint32_t>(header[7]) << 16u);
    const auto sent = read<std::uint32_t>(channel.bytes + 0x70);
    auto counter = (sent & 0xfffffc00u) |
        ((std::to_integer<std::uint32_t>(header[2]) & 0x30u) << 4u) |
        std::to_integer<std::uint32_t>(header[4]);
    if (counter >= 0x400u && counter > sent) counter -= 0x400u;
    const auto previous_counter = read<std::uint32_t>(channel.bytes + 0x60);
    if (counter < previous_counter || (counter == previous_counter &&
        (mask & read<std::uint32_t>(channel.bytes + 0x68)) != mask)) return nullptr;
    ClockTimestamp output = context.ack_stack_preimage;
    const auto* sampled = sample_published_native_frame_clock(context.clock, &output);
    auto* destination = &channel;
    const auto* maximum = &context.maximum_sample_00d049a8;
    const auto* capped = &context.capped_sample_00cf4848;
    NativeNetworkConsoleQueue82cStorage* result;
    std::uint32_t previous, bit;
    float temporary;
    // A3B4A2..A3B5C9: retain the sampled float and double limit on the x87
    // stack throughout traversal. No C++ double/SSE arithmetic substitution.
    // EBX/EBP stay available to MSVC's possible aligned-stack frame.
    __asm {
        mov esi, destination
        xor edi, edi
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov edx, dword ptr [esi + 78h]
        mov eax, mask
        test edx, edx
        fdivp st(1), st(0)
        mov previous, 0
        mov ecx, counter
        mov dword ptr [esi + 60h], ecx
        mov dword ptr [esi + 68h], eax
        fstp temporary
        jz ack_done
        fld temporary
        mov eax, capped
        movss xmm0, dword ptr [eax]
        mov eax, maximum
        fld qword ptr [eax]
    ack_next:
        mov eax, dword ptr [edx + 828h]
        cmp eax, counter
        jbe ack_remove
        mov ecx, eax
        sub ecx, counter
        sub ecx, 1
        cmp ecx, 18h
        jae ack_skip
        mov bit, 1
        shl bit, cl
        mov ecx, mask
        test ecx, bit
        jz ack_skip
    ack_remove:
        mov ecx, eax
        and ecx, 1fh
        shl ecx, 4
        cmp dword ptr [ecx + esi + 290h], eax
        lea ecx, [ecx + esi + 28ch]
        jne ack_unlink
        cmp byte ptr [ecx], 0
        jne ack_unlink
        fld st(1)
        fsub dword ptr [ecx + 8]
        fstp temporary
        fld temporary
        fcomip st(0), st(1)
        jbe ack_record_sample
        movss temporary, xmm0
    ack_record_sample:
        movss xmm1, temporary
        mov byte ptr [ecx], 1
        movss dword ptr [ecx + 0ch], xmm1
        fld dword ptr [esi + 490h]
        fadd temporary
        add dword ptr [esi + 48ch], 1
        fstp dword ptr [esi + 490h]
    ack_unlink:
        mov ecx, previous
        test ecx, ecx
        jnz ack_not_head
        mov eax, dword ptr [edx]
        mov dword ptr [esi + 78h], eax
        mov dword ptr [edx], edi
        mov edi, edx
        mov edx, dword ptr [esi + 78h]
        test edx, edx
        jnz ack_next
        fstp st(1)
        fstp st(0)
        mov dword ptr [esi + 84h], edx
        jmp ack_done
    ack_not_head:
        cmp edx, dword ptr [esi + 84h]
        je ack_tail
        mov eax, dword ptr [edx]
        mov dword ptr [edx], edi
        mov edi, edx
        mov dword ptr [ecx], eax
        mov edx, eax
        jmp ack_continue
    ack_skip:
        mov previous, edx
        mov edx, dword ptr [edx]
    ack_continue:
        test edx, edx
        jnz ack_next
        jmp ack_pop
    ack_tail:
        mov dword ptr [edx], edi
        mov dword ptr [esi + 84h], ecx
        mov edi, edx
        mov dword ptr [ecx], 0
    ack_pop:
        fstp st(1)
        fstp st(0)
    ack_done:
        mov result, edi
    }
    return result;
}
NativeNetworkConsoleQueue82cStorage* classify_native_network_console_received_00a3b5f0(
    NativeNetworkConsoleChannelStorage& channel, const std::byte* header,
    std::int32_t& stream, std::uint32_t& sequence, const NativeNetworkConsoleReceiveContext& context) {
    ClockTimestamp output = context.classify_stack_preimage;
    const auto* sampled = sample_published_native_frame_clock(context.clock, &output);
    auto* destination = &channel;
    __asm {
        mov ecx, destination
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp dword ptr [ecx + 20h]
    }
    auto* const released = process_native_network_console_ack_00a3b420(channel, header, context);
    sequence = 0;
    const auto type = std::to_integer<std::uint32_t>(header[2]) & 0xfu;
    if (context.sequenced_types_00e0e360[type]) {
        const auto index = context.stream_indices_00e0e370[type];
        sequence = decode_native_network_console_sequence_00a3a6b0(
            header, read<std::uint32_t>(channel.bytes + 0x2c + index * 4u));
        if (sequence > read<std::uint32_t>(channel.bytes + 0x2c + index * 4u)) {
            auto* node = read<std::byte*>(channel.bytes + 0x44 + index * 4u);
            while (node) {
                if (sequence == read<std::uint32_t>(node + 0x1c)) {
                    stream = -1;
                    return released;
                }
                node = read<std::byte*>(node);
            }
            channel.bytes[0x28 + index] = std::byte{0};
            stream = static_cast<std::int32_t>(index);
            return released;
        }
    } else if (type != 8u) {
        stream = 2;
        return released;
    }
    stream = -1;
    return released;
}
void activate_native_network_console_channel_00a3baa0(NativeNetworkConsoleStorage& owner,
    const std::byte* address, const NativeNetworkConsoleSocketImports& imports) {
    auto* const section = read<TrackedCriticalSection*>(owner.bytes + 0x148);
    auto* depth = section ? reinterpret_cast<std::byte*>(&section->depth) : nullptr;
    if (section) {
        ::EnterCriticalSection(&section->native);
        write(depth, read<std::uint32_t>(depth) + 1u);
    }
    for (std::uint32_t index = 0; index != 16; ++index) {
        auto* channel = owner.bytes + 0x19c + index * 0x49c;
        if (channel[0] != std::byte{0}) {
            channel[0] = std::byte{0};
            for (std::size_t word = 0; word != 4; ++word)
                write(channel + 4 + word * 4, read<std::uint32_t>(address + word * 4));
            (void)imports.host_to_network_short(read<std::uint16_t>(address + 2));
            break;
        }
    }
    if (section) {
        write(depth, read<std::uint32_t>(depth) - 1u);
        ::LeaveCriticalSection(&section->native);
    }
}
void request_native_network_console_disconnect_00a3adf0(NativeNetworkConsoleChannelStorage& channel,
    float delay, const NativeFrameClockPublicationContext& clock, const ClockTimestamp& preimage) {
    const std::uint32_t was_active = channel.bytes[0x18] != std::byte{0};
    if (!was_active) channel.bytes[0x18] = std::byte{1};
    ClockTimestamp output = preimage;
    const auto* sampled = sample_published_native_frame_clock(clock, &output);
    auto* destination = &channel;
    float candidate, old_deadline;
    __asm {
        mov esi, destination
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp candidate
        fld candidate
        fadd delay
        cmp was_active, 0
        jne deadline_active
        fstp dword ptr [esi + 1ch]
        jmp deadline_done
    deadline_active:
        fstp candidate
        fld dword ptr [esi + 1ch]
        fstp old_deadline
        fld old_deadline
        fld candidate
        fcomip st(0), st(1)
        fstp st(0)
        jbe deadline_candidate
        movss xmm0, old_deadline
        movss dword ptr [esi + 1ch], xmm0
        jmp deadline_done
    deadline_candidate:
        movss xmm0, candidate
        movss dword ptr [esi + 1ch], xmm0
    deadline_done:
    }
}
} // namespace bsp
