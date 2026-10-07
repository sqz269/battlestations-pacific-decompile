#include "bsp/native_session_transport_flush.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
U partial_byte(U bits) noexcept { return (bits & 7u) != 0 ? 1u : 0u; }
void* embedded(void* transport, U offset) noexcept {
    return static_cast<std::byte*>(transport) + offset;
}
} // namespace

NativeUnitHealthMessageTransportCalls::NativeUnitHealthMessageTransportCalls(
    NativeUnitHealthSetterGlobals globals, NativeUnitHealthRouteGlobals route,
    const NativeUnitHealthMessageProfile& profile, const volatile std::uint16_t& tick,
    const NativeSessionTransportFlushContext& flush) noexcept
    : NativeUnitHealthMessageSerializedCalls(globals, route, profile, tick),
      current_game_00e188a8_(globals.current_game_00e188a8), flush_(flush) {}

void NativeUnitHealthMessageTransportCalls::call_transport_flush_20(U entry,
    void* transport, void* target, NativeBitCursor* cursor, U delivery) {
    if (entry == 0x00783e00u)
        flush_host_transport_00783e00(transport, target, cursor, delivery);
    else if (entry == 0x00783ea0u)
        flush_client_transport_00783ea0(transport, target, cursor, delivery);
    else
        throw std::logic_error("unsupported current native session transport +20 target");
}

void NativeUnitHealthMessageTransportCalls::stamp_target_and_count(void* target) {
    ClockTimestamp output = flush_.sample_stack_preimage;
    const ClockTimestamp* const returned = sample_published_native_frame_clock(flush_.clock, &output);
    volatile float* const destination = &bind_transport_target(target).timestamp_d68;
    //783E27..783E33 / 783EC8..783ED4. Consume the returned pointer, retain
    //ambient x87 PC/RC/TOP, and perform the ONLY binary32 spill at actual D68.
    __asm {
        mov eax, returned
        mov edx, destination
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp dword ptr [edx]
    }
    void* const game = current_game_00e188a8_;
    auto& counter = game_flush_count_217c(game);
    counter = counter + 1u;
}

void NativeUnitHealthMessageTransportCalls::call_current_delivery_24(void* transport,
    void* target, NativeBitCursor* cursor, U delivery) {
    const volatile U* const table = transport_primary_table(transport);
    const U entry = table[0x24 / 4];
    if (entry == 0x00a41ab0u)
        deliver_host_transport_00a41ab0(transport, target, cursor, delivery);
    else if (entry == 0x00a419b0u)
        deliver_client_transport_00a419b0(transport, target, cursor, delivery);
    else
        throw std::logic_error("unsupported current native session transport +24 target");
}

void NativeUnitHealthMessageTransportCalls::flush_host_transport_00783e00(
    void* transport, void* target, NativeBitCursor* cursor, U delivery) {
    if (delivery == 1) stamp_target_and_count(target);
    else if (delivery == 0 && host_suppression_a0(transport) != 0) return;

    //The native test of constant004B4FE0 is always nonzero. 786CA0 itself is
    //RET0Ch, but the ordered cursor/global loads preparing its arguments occur.
    const volatile auto& c = *cursor;
    const U bit = static_cast<U>(c.bit_0c);
    const auto* const base = c.base_00;
    const U observer = flush_.observer_00f871b4;
    const U current = reinterpret_cast<U>(c.current_08);
    const U length = partial_byte(bit) + current - reinterpret_cast<U>(base);
    (void)observer;
    (void)length;
    call_current_delivery_24(transport, target, cursor, delivery);
}

void NativeUnitHealthMessageTransportCalls::flush_client_transport_00783ea0(
    void* transport, void* target, NativeBitCursor* cursor, U delivery) {
    if (delivery == 1) stamp_target_and_count(target);

    const volatile auto& c = *cursor;
    const U bit = static_cast<U>(c.bit_0c);
    const auto* const base = c.base_00;
    const U current = reinterpret_cast<U>(c.current_08);
    const U length = partial_byte(bit) + current - reinterpret_cast<U>(base);
    const U observer = flush_.observer_00f871b4; //783EF9, AFTER current-byte read
    (void)length;
    (void)observer;
    call_current_delivery_24(transport, target, cursor, delivery);
}

void NativeUnitHealthMessageTransportCalls::account_sent_bytes(void* transport,
    void* original_target, NativeBitCursor* cursor, U delivery) {
    //Both +24 bodies reread in this order AFTER enqueue, regardless of its
    //Boolean/drop result. Client accounting still indexes the original target.
    const volatile auto& c = *cursor;
    const U bit = static_cast<U>(c.bit_0c);
    const U index = bind_transport_target(original_target).counter_index_d6c;
    const U base = reinterpret_cast<U>(c.base_00);
    const U current = reinterpret_cast<U>(c.current_08);
    const U length = partial_byte(bit) - base + current;
    auto& counter = transport_sent_counter(transport, index, delivery);
    counter = counter + length;
}

void NativeUnitHealthMessageTransportCalls::deliver_host_transport_00a41ab0(
    void* transport, void* target, NativeBitCursor* cursor, U delivery) {
    if (delivery > 2) return;
    const volatile auto& c = *cursor;
    const U bit = static_cast<U>(c.bit_0c);
    const auto* const base = c.base_00;
    const U current = reinterpret_cast<U>(c.current_08);
    if (delivery == 2) {
        const U length = partial_byte(bit) + current - reinterpret_cast<U>(base);
        enqueue_direct_transport_00a39120(embedded(transport, 0xa4), 5, target, base, length);
        return;
    }
    const U length = partial_byte(bit) - reinterpret_cast<U>(base) + current;
    enqueue_normal_transport_00a39160(embedded(transport, 0xa4), delivery == 1 ? 6u : 5u,
        target, base, length);
    account_sent_bytes(transport, target, cursor, delivery);
}

void NativeUnitHealthMessageTransportCalls::deliver_client_transport_00a419b0(
    void* transport, void* target, NativeBitCursor* cursor, U delivery) {
    if (delivery > 2) return;
    const volatile auto& c = *cursor;
    if (delivery == 2) {
        const U bit = static_cast<U>(c.bit_0c);
        const auto* const base = c.base_00;
        const U current = reinterpret_cast<U>(c.current_08);
        const U length = partial_byte(bit) + current - reinterpret_cast<U>(base);
        enqueue_direct_transport_00a39120(embedded(transport, 0xc8), 5, target, base, length);
        return;
    }
    if (delivery == 1) {
        const U bit = static_cast<U>(c.bit_0c);
        const auto* const base = c.base_00;
        void* const destination = client_destination_93c(transport); //A41A08
        const U current = reinterpret_cast<U>(c.current_08); //A41A19
        const U length = partial_byte(bit) - reinterpret_cast<U>(base) + current;
        enqueue_normal_transport_00a39160(embedded(transport, 0xc8), 6, destination, base, length);
    } else {
        const U bit = static_cast<U>(c.bit_0c);
        const auto* const base = c.base_00;
        const U current = reinterpret_cast<U>(c.current_08); //A41A6A
        const U length = partial_byte(bit) - reinterpret_cast<U>(base) + current;
        void* const destination = client_destination_93c(transport); //A41A6F
        enqueue_normal_transport_00a39160(embedded(transport, 0xc8), 5, destination, base, length);
    }
    account_sent_bytes(transport, target, cursor, delivery);
}

void NativeUnitHealthMessageTransportCalls::enqueue_direct_transport_00a39120(
    void* object, U type, void* target, const void* payload, U length) {
    if (type != 5) return;
    const U socket = embedded_socket_858(object); //A3912F
    const auto* const address = bind_transport_target(target).sockaddr_d74; //A3913F
    auto* const owner = flush_.send.current_owner_00f8abdc; //A39146
    (void)enqueue_native_network_console_packet_00a3c1a0(*owner, socket, address,
        5, payload, length, 1, flush_.send);
}

void NativeUnitHealthMessageTransportCalls::enqueue_normal_transport_00a39160(
    void* object, U type, void* target, const void* payload, U length) {
    const U socket = embedded_socket_858(object); //A39175
    auto* const owner = flush_.send.current_owner_00f8abdc; //A3917B
    const auto* const address = bind_transport_target(target).sockaddr_d74; //A39181
    (void)enqueue_native_network_console_packet_00a3c1a0(*owner, socket, address,
        type, payload, length, 0, flush_.send);
}
} // namespace bsp
