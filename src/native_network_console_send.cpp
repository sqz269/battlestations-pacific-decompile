#include "bsp/native_network_console_send.hpp"
#include <cstring>
#include <new>

namespace bsp {
namespace {
template<class T> T read(const std::byte* p) noexcept {
    T value; std::memcpy(&value, p, sizeof value); return value;
}
template<class T> void write(std::byte* p, T value) noexcept {
    std::memcpy(p, &value, sizeof value);
}
class CapturedSection {
    TrackedCriticalSection* section_;
public:
    explicit CapturedSection(TrackedCriticalSection* section) : section_(section) {
        if (section_) {
            ::EnterCriticalSection(&section_->native);
            auto* depth = reinterpret_cast<std::byte*>(&section_->depth);
            write(depth, read<std::uint32_t>(depth) + 1u);
        }
    }
    ~CapturedSection() {
        if (section_) {
            auto* depth = reinterpret_cast<std::byte*>(&section_->depth);
            write(depth, read<std::uint32_t>(depth) - 1u);
            ::LeaveCriticalSection(&section_->native);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
};
NativeNetworkConsoleChannelStorage& channel(NativeNetworkConsoleStorage& owner, std::uint32_t index) {
    return *std::launder(reinterpret_cast<NativeNetworkConsoleChannelStorage*>(
        owner.bytes + 0x19c + index * 0x49c));
}
}
void set_native_network_console_latency_profile_00a3a890(NativeNetworkConsoleStorage& owner,
    std::uint32_t index, float maximum, float minimum, float drop, float send_weight,
    const volatile double& half_00d7a280) {
    auto* destination = &owner;
    const auto* half = &half_00d7a280;
    __asm {
        mov eax, index
        fld maximum
        fadd minimum
        movss xmm0, maximum
        lea eax, [eax + eax * 4]
        mov ecx, destination
        lea eax, [ecx + eax * 4 + 8]
        mov ecx, half
        fmul qword ptr [ecx]
        movss dword ptr [eax], xmm0
        movss xmm0, minimum
        movss dword ptr [eax + 4], xmm0
        movss xmm0, drop
        fstp dword ptr [eax + 8]
        movss dword ptr [eax + 0ch], xmm0
        movss xmm0, send_weight
        movss dword ptr [eax + 10h], xmm0
    }
}
float estimate_native_network_console_latency_00a3aef0(NativeNetworkConsoleChannelStorage& owner,
    std::uint32_t& level, const NativeNetworkConsoleTimingContext& context) {
    level = read<std::uint32_t>(owner.bytes + 0x494);
    const auto sent = read<std::uint32_t>(owner.bytes + 0x70);
    if (!sent) return context.no_samples_00d7a2f0;
    auto count = sent - read<std::uint32_t>(owner.bytes + 0x60);
    if (count < 4u) count = 4u;
    if (sent < count) count = sent;
    if (!count) return context.no_samples_00d7a2f0;
    const auto start = sent & 31u;
    float largest = 0, second = 0, third = 0, fourth = 0, temporary, result;
    ClockTimestamp output = context.sample_stack_preimage;
    const auto* sampled = sample_published_native_frame_clock(context.clock, &output);
    auto* destination = &owner;
    const auto* half = &context.half_00d7a280;
    const auto* three = &context.three_00d7a2b0;
    const auto* quarter = &context.quarter_00d7a348;
    // A3AF73..A3B0D8. The two retained x87 values and the SSE bit moves are
    // intentional; replacing the ordered insertion with std::sort changes NaNs.
    __asm {
        mov esi, destination
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov ecx, start
        mov edx, count
        fdivp st(1), st(0)
        fstp temporary
        fld temporary
        movss xmm1, second
        fld largest
        movss xmm3, largest
        movss xmm0, third
    metric_next:
        mov eax, ecx
        shl eax, 4
        cmp byte ptr [eax + esi + 28ch], 0
        lea eax, [eax + esi + 28ch]
        je metric_pending
        movss xmm2, dword ptr [eax + 0ch]
        movss temporary, xmm2
        jmp metric_insert
    metric_pending:
        fld st(1)
        fsub dword ptr [eax + 8]
        fstp temporary
        movss xmm2, temporary
    metric_insert:
        fld temporary
        fcomi st(0), st(1)
        jbe metric_second
        movss fourth, xmm0
        fstp st(0)
        movaps xmm0, xmm1
        fstp st(0)
        movaps xmm1, xmm3
        movaps xmm3, xmm2
        movss largest, xmm3
        fld largest
        movss third, xmm0
        movss second, xmm1
        jmp metric_advance
    metric_second:
        fld second
        fxch
        fcomi st(0), st(1)
        fstp st(1)
        jbe metric_third
        movss fourth, xmm0
        fstp st(0)
        movaps xmm0, xmm1
        movaps xmm1, xmm2
        movss third, xmm0
        movss second, xmm1
        jmp metric_advance
    metric_third:
        fld third
        fxch
        fcomi st(0), st(1)
        fstp st(1)
        jbe metric_fourth
        movss fourth, xmm0
        fstp st(0)
        movaps xmm0, xmm2
        movss third, xmm0
        jmp metric_advance
    metric_fourth:
        fld fourth
        fxch
        fcomip st(0), st(1)
        fstp st(0)
        jbe metric_advance
        movss fourth, xmm2
    metric_advance:
        test ecx, ecx
        jne metric_decrement
        mov ecx, 20h
    metric_decrement:
        sub edx, 1
        sub ecx, 1
        test edx, edx
        ja metric_next
        fstp st(1)
        cmp count, 1
        je metric_result
        fadd second
        cmp count, 2
        jne metric_sum_three
        mov eax, half
        fmul qword ptr [eax]
        jmp metric_result
    metric_sum_three:
        fadd third
        cmp count, 3
        jne metric_sum_four
        mov eax, three
        fdiv qword ptr [eax]
        jmp metric_result
    metric_sum_four:
        fadd fourth
        mov eax, quarter
        fmul qword ptr [eax]
    metric_result:
        fstp result
    }
    return result;
}
__declspec(naked) NativeNetworkConsoleQueue82cStorage* __fastcall
select_native_network_console_packet_00a3a8f0(NativeNetworkConsoleChannelStorage*, void*,
    std::uint8_t*, std::uint8_t*, float, float) noexcept {
    // Complete register/stack ABI and x87 body; no hidden calls or global cells.
    __asm {
        push ecx
        mov eax, dword ptr [esp + 8]
        fld dword ptr [esp + 14h]
        fld dword ptr [esp + 10h]
        push ebx
        push ebp
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 18h]
        push edi
        mov byte ptr [eax], 0
        mov dword ptr [esp + 10h], esi
        mov byte ptr [ecx], 0
        xor ebp, ebp
        lea edi, [esi + 74h]
    select_stream:
        mov edx, dword ptr [edi]
        cmp ebp, 2
        sete cl
        or cl, byte ptr [esi + 18h]
        cmp edx, dword ptr [edi + 0ch]
        jne select_many
        test edx, edx
        jz select_next_stream
        test cl, cl
        mov bl, 1
        jnz select_single_time
        mov edx, dword ptr [edx + 828h]
        mov eax, dword ptr [edi - 18h]
        cmp edx, eax
        jae select_single_window
        mov edx, dword ptr [esp + 1ch]
        mov byte ptr [edx], bl
        jmp select_single_time
    select_single_window:
        add eax, 18h
        cmp edx, eax
        jbe select_single_time
        xor bl, bl
    select_single_time:
        mov eax, dword ptr [edi]
        fld st(0)
        fsub dword ptr [eax + 24h]
        fcomip st(0), st(2)
        jc select_single_forced
        test bl, bl
        jnz select_single_ready
    select_single_forced:
        test cl, cl
        jnz select_single_ready
        mov edx, dword ptr [esp + 1ch]
        cmp byte ptr [edx], cl
        je select_next_stream
    select_single_ready:
        test cl, cl
        fstp st(1)
        mov eax, dword ptr [esi + ebp * 4 + 74h]
        fstp st(0)
        jnz select_single_remove
        mov ecx, dword ptr [esp + 1ch]
        cmp byte ptr [ecx], 0
        je select_single_stamp
    select_single_remove:
        mov edx, dword ptr [esp + 18h]
        xor ecx, ecx
        mov dword ptr [esi + ebp * 4 + 74h], ecx
        mov dword ptr [esi + ebp * 4 + 80h], ecx
        mov byte ptr [edx], 1
    select_single_stamp:
        movss xmm0, dword ptr [esp + 20h]
        pop edi
        pop esi
        pop ebp
        movss dword ptr [eax + 24h], xmm0
        pop ebx
        pop ecx
        ret 10h
    select_many:
        test edx, edx
        mov dword ptr [esp + 24h], 0
        jz select_next_stream
    select_node:
        test cl, cl
        mov bl, 1
        jnz select_node_time
        mov eax, dword ptr [edx + 828h]
        mov esi, dword ptr [edi - 18h]
        cmp eax, esi
        jae select_node_window
        mov eax, dword ptr [esp + 1ch]
        mov byte ptr [eax], bl
        jmp select_restore_owner
    select_node_window:
        add esi, 18h
        cmp eax, esi
        jbe select_restore_owner
        xor bl, bl
    select_restore_owner:
        mov esi, dword ptr [esp + 10h]
    select_node_time:
        fld st(0)
        fsub dword ptr [edx + 24h]
        fcomip st(0), st(2)
        jc select_node_forced
        test bl, bl
        jnz select_node_ready
    select_node_forced:
        test cl, cl
        jnz select_node_ready
        mov eax, dword ptr [esp + 1ch]
        cmp byte ptr [eax], cl
        jne select_node_ready
        mov dword ptr [esp + 24h], edx
        mov edx, dword ptr [edx]
        test edx, edx
        jnz select_node
    select_next_stream:
        add ebp, 1
        add edi, 4
        cmp ebp, 3
        jl select_stream
        pop edi
        fstp st(1)
        pop esi
        fstp st(0)
        pop ebp
        xor eax, eax
        pop ebx
        pop ecx
        ret 10h
    select_node_ready:
        movss xmm0, dword ptr [esp + 20h]
        fstp st(1)
        movss dword ptr [edx + 24h], xmm0
        fstp st(0)
        cmp edx, dword ptr [esi + ebp * 4 + 74h]
        jne select_check_tail
        mov eax, dword ptr [edx]
        mov dword ptr [esi + ebp * 4 + 74h], eax
        jmp select_unlink_previous
    select_check_tail:
        cmp edx, dword ptr [esi + ebp * 4 + 80h]
        jne select_unlink_previous
        mov eax, dword ptr [esp + 24h]
        mov dword ptr [esi + ebp * 4 + 80h], eax
    select_unlink_previous:
        mov eax, dword ptr [esp + 24h]
        test eax, eax
        jz select_rotate_or_remove
        mov edi, dword ptr [edx]
        mov dword ptr [eax], edi
    select_rotate_or_remove:
        mov eax, dword ptr [esp + 1ch]
        cmp byte ptr [eax], 0
        jne select_remove
        test cl, cl
        jnz select_remove
        cmp dword ptr [esi + ebp * 4 + 74h], edx
        jne select_append_tail
        mov ecx, dword ptr [edx]
        mov dword ptr [esi + ebp * 4 + 74h], ecx
    select_append_tail:
        mov eax, dword ptr [esi + ebp * 4 + 80h]
        mov dword ptr [eax], edx
        pop edi
        mov dword ptr [esi + ebp * 4 + 80h], edx
        pop esi
        pop ebp
        mov dword ptr [edx], 0
        mov eax, edx
        pop ebx
        pop ecx
        ret 10h
    select_remove:
        mov ecx, dword ptr [esp + 18h]
        pop edi
        pop esi
        pop ebp
        mov byte ptr [ecx], 1
        mov eax, edx
        pop ebx
        pop ecx
        ret 10h
    }
}
void return_native_network_console_channel_queues_00a3b150(NativeNetworkConsoleChannelStorage& owner,
    std::byte* send_pool_slot, std::byte* receive_pool_slot,
    const NativeNetworkConsoleChannelContext& reset) {
    for (std::uint32_t family = 0; family != 2; ++family) {
        auto* const pool = family ? receive_pool_slot : send_pool_slot;
        const auto heads = family ? 0x44u : 0x74u;
        const auto tails = family ? 0x50u : 0x80u;
        for (std::uint32_t stream = 0; stream != 3; ++stream) {
            if (read<std::byte*>(owner.bytes + heads + stream * 4u)) {
                auto* const tail = read<std::byte*>(owner.bytes + tails + stream * 4u);
                write(tail, read<std::byte*>(pool));
                write(pool, read<std::byte*>(owner.bytes + heads + stream * 4u));
                write(owner.bytes + heads + stream * 4u, std::uint32_t{0});
                write(owner.bytes + tails + stream * 4u, std::uint32_t{0});
            }
        }
    }
    reset_native_network_console_channel_00a3ad10(owner, reset);
}
void remove_native_network_console_channel_00a3bc60(NativeNetworkConsoleStorage& owner,
    const std::byte* address, const NativeNetworkConsoleChannelContext& reset) {
    const CapturedSection section(read<TrackedCriticalSection*>(owner.bytes + 0x148));
    for (std::uint32_t index = 0; index != 16; ++index) {
        auto& current = channel(owner, index);
        if (current.bytes[0] == std::byte{0} &&
            read<std::uint32_t>(current.bytes + 8) == read<std::uint32_t>(address + 4)) {
            return_native_network_console_channel_queues_00a3b150(
                current, owner.bytes + 0x2046e8, owner.bytes + 0x4003ec, reset);
            return;
        }
    }
}
bool queue_native_network_console_packet_00a3b210(NativeNetworkConsoleChannelStorage& owner,
    std::byte* header, std::uint32_t packet_type, NativeNetworkConsoleQueue82cStorage& record,
    const NativeNetworkConsoleSendContext& context) {
    write(owner.bytes + 0x14, read<std::uint32_t>(record.bytes + 4));
    std::uint32_t ignored_level;
    const float score = estimate_native_network_console_latency_00a3aef0(owner, ignored_level, context.timing);
    auto* destination = &owner;
    auto* const publication = &context.current_owner_00f8abdc;
    __asm {
        mov esi, destination
        mov ecx, dword ptr [esi + 494h]
        mov eax, publication
        mov eax, dword ptr [eax]
        lea edx, [ecx + ecx * 4]
        fld dword ptr [eax + edx * 4 + 8]
        lea edx, [eax + edx * 4]
        fld score
        fcomi st(0), st(1)
        fstp st(1)
        jbe packet_no_raise
        add ecx, 1
        mov dword ptr [esi + 494h], ecx
    packet_pop_score:
        fstp st(0)
        jmp packet_adapted
    packet_no_raise:
        test ecx, ecx
        jbe packet_pop_score
        fld dword ptr [edx + 0ch]
        fcomip st(0), st(1)
        fstp st(0)
        jbe packet_adapted
        add ecx, -1
        mov dword ptr [esi + 494h], ecx
    packet_adapted:
    }
    ClockTimestamp output = context.enqueue_stack_preimage;
    const auto* sampled = sample_published_native_frame_clock(context.timing.clock, &output);
    auto* node = &record;
    const auto* types = context.sequenced_types_00e0e360;
    const auto* indices = context.stream_indices_00e0e370;
    const auto* age = &context.initial_packet_age_00ce47a0;
    const auto* unit = &context.packet_drop_unit_00d7a210;
    float now, accumulator;
    bool accepted;
    __asm {
        mov esi, destination
        mov edi, node
        mov eax, sampled
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov eax, packet_type
        mov edx, header
        mov cl, al
        fdivp st(1), st(0)
        and cl, 0fh
        mov byte ptr [edx + 2], cl
        mov ecx, types
        cmp byte ptr [ecx + eax], 0
        fstp now
        jz packet_unsequenced
        mov ecx, indices
        mov eax, dword ptr [ecx + eax * 4]
        fld now
        add dword ptr [esi + eax * 4 + 6ch], 1
        mov ecx, age
        fsub qword ptr [ecx]
        mov ecx, dword ptr [esi + eax * 4 + 6ch]
        mov byte ptr [edx + 3], cl
        shr ecx, 2
        and cl, 0c0h
        or byte ptr [edx + 2], cl
        mov edx, dword ptr [esi + eax * 4 + 6ch]
        fstp dword ptr [edi + 24h]
        mov dword ptr [edi + 828h], edx
        cmp dword ptr [esi + eax * 4 + 74h], 0
        jne packet_link_previous
        mov dword ptr [esi + eax * 4 + 74h], edi
        jmp packet_set_tail
    packet_link_previous:
        mov ecx, dword ptr [esi + eax * 4 + 80h]
        mov dword ptr [ecx], edi
    packet_set_tail:
        mov dword ptr [esi + eax * 4 + 80h], edi
        mov dword ptr [edi], 0
        mov edx, dword ptr [esi + eax * 4 + 6ch]
        and edx, 1fh
        mov ecx, eax
        shl ecx, 5
        add edx, ecx
        shl edx, 4
        cmp byte ptr [edx + esi + 8ch], 0
        lea ecx, [edx + esi + 8ch]
        je packet_record_slot
        fld dword ptr [esi + 490h]
        fsub dword ptr [ecx + 0ch]
        add dword ptr [esi + 48ch], -1
        fstp dword ptr [esi + 490h]
    packet_record_slot:
        movss xmm0, now
        mov byte ptr [ecx], 0
        mov edx, dword ptr [esi + eax * 4 + 6ch]
        mov dword ptr [ecx + 4], edx
        movss dword ptr [ecx + 8], xmm0
        jmp packet_success
    packet_unsequenced:
        mov eax, dword ptr [esi + 494h]
        mov ecx, publication
        mov ecx, dword ptr [ecx]
        add eax, 1
        lea eax, [eax + eax * 4]
        fld dword ptr [ecx + eax * 4]
        fadd dword ptr [esi + 498h]
        fstp accumulator
        fld accumulator
        fst dword ptr [esi + 498h]
        fld1
        fxch
        fcomi st(0), st(1)
        fstp st(1)
        jc packet_below_drop_threshold
        mov eax, unit
        fsub qword ptr [eax]
        fstp dword ptr [esi + 498h]
        mov eax, publication
        mov eax, dword ptr [eax]
        cmp byte ptr [eax + 150h], 0
        je packet_append_unsequenced
        cmp dword ptr [edi + 20h], 20h
        jbe packet_append_unsequenced
        mov accepted, 0
        jmp packet_done
    packet_below_drop_threshold:
        fstp st(0)
    packet_append_unsequenced:
        fld now
        mov byte ptr [edx + 3], 0
        mov eax, age
        fsub qword ptr [eax]
        mov dword ptr [edi], 0
        mov dword ptr [edi + 828h], 0
        fstp dword ptr [edi + 24h]
        mov eax, dword ptr [esi + 88h]
        test eax, eax
        jz packet_empty_unsequenced
        mov dword ptr [eax], edi
        mov dword ptr [esi + 88h], edi
        jmp packet_success
    packet_empty_unsequenced:
        mov dword ptr [esi + 7ch], edi
        mov dword ptr [esi + 88h], edi
    packet_success:
        mov accepted, 1
    packet_done:
    }
    return accepted;
}
bool enqueue_native_network_console_packet_00a3c1a0(NativeNetworkConsoleStorage& owner,
    std::uint32_t socket, const std::byte* address, std::uint32_t type,
    const void* payload, std::uint32_t length, std::uint8_t direct,
    const NativeNetworkConsoleSendContext& context) {
    const CapturedSection section(read<TrackedCriticalSection*>(owner.bytes + 0x148));
    std::uint32_t socket_index = 0;
    while (socket_index < read<std::uint32_t>(owner.bytes + 0x15c)) {
        if (read<std::uint32_t>(owner.bytes + 0x160 + socket_index * 4u) == socket) break;
        ++socket_index;
    }
    if (socket_index >= read<std::uint32_t>(owner.bytes + 0x15c)) return false;
    auto* const record = read<NativeNetworkConsoleQueue82cStorage*>(owner.bytes + 0x2046e8);
    if (!record) return false;
    auto* const data = record->bytes;
    write(owner.bytes + 0x2046e8, read<std::byte*>(data));
    write(data + 4, socket);
    for (std::size_t word = 0; word != 4; ++word)
        write(data + 8 + word * 4, read<std::uint32_t>(address + word * 4));
    write(data + 0x18, type);
    data[0x1c] = static_cast<std::byte>(direct);
    if (direct) {
        if (owner.bytes[0x4b5c] == std::byte{0}) {
            data[0x2a] = static_cast<std::byte>(type & 0xfu);
            data[0x29] = std::byte{0};
            data[0x28] = std::byte{3};
            std::memcpy(data + 0x2b, payload, length);
            write(data + 0x20, length + 3u);
            const auto count = read<std::uint32_t>(owner.bytes + 0x4b60);
            const auto delta = reinterpret_cast<std::uint32_t>(record) -
                reinterpret_cast<std::uint32_t>(&owner) - 0x5b08u;
            const auto index = static_cast<std::int32_t>(delta) / 0x82c;
            write(owner.bytes + 0x4b68 + count * 2u, static_cast<std::uint16_t>(index));
            write(owner.bytes + 0x4b60, read<std::uint32_t>(owner.bytes + 0x4b60) + 1u);
            return true;
        }
    } else {
        const auto index = find_native_network_console_channel_00a3bb30(owner, address, context.sockets);
        if (static_cast<std::int32_t>(index) >= 0) {
            if (length) std::memcpy(data + 0x30, payload, length);
            write(data + 0x20, length + 8u);
            data[0x29] = static_cast<std::byte>(((length + 6u) >> 8u) & 0xffu);
            data[0x28] = static_cast<std::byte>((length + 6u) & 0xffu);
            if (queue_native_network_console_packet_00a3b210(channel(owner, index), data + 0x28,
                    type, *record, context)) return true;
        }
    }
    write(data, read<std::byte*>(owner.bytes + 0x2046e8));
    write(owner.bytes + 0x2046e8, record);
    return true;
}
} // namespace bsp
