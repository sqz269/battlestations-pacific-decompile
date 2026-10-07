#pragma once
#include "bsp/native_network_console_send.hpp"
#include "bsp/native_unit_health_message.hpp"

namespace bsp {
struct NativeSessionTransportFlushContext {
    const NativeFrameClockPublicationContext& clock;
    const ClockTimestamp& sample_stack_preimage;
    const volatile std::uint32_t& observer_00f871b4;
    const NativeNetworkConsoleSendContext& send;
};
struct NativeSessionTransportTargetFields {
    volatile float& timestamp_d68;
    const volatile std::uint32_t& counter_index_d6c;
    const std::byte* sockaddr_d74;
};

// Complete ordinary host/client +20 flush, +24 delivery and embedded enqueue
// adapters. The constructors publish D243EC/+20=783E00/+24=A41AB0 at session
// +188, or D24448/+20=783EA0/+24=A419B0 at +18C. This class constructs neither.
// Existing actual table/cursor/route bindings remain required. Table access is
// a pure alias; entries are original dispatch tokens, never executable copies
// or default mappings. Unsupported entries throw logic_error (source contract,
// not original fault/exception ABI). +24 is selected afresh after the clock.
//
// All bindings below only alias actual live backing: no snapshots, callbacks,
// allocation, ownership changes, FP changes or early field-value reads. Target
// storage includes D68/D6C and 16 sockaddr bytes at D74. Transport storage covers
// the reached actual family and native indexed counters. Arithmetic on cursor
// addresses, byte counts and counters is modulo32; indices/ranges are unchecked.
// The enqueue's existing fitting-payload/valid-list/profile contracts apply.
// The native x87 environment is retained; two available stack slots and masked
// exceptions are the verified arithmetic domain. No cursor reset or borrowed
// payload retention occurs here. Owned queue copies are not network delivery.
class NativeUnitHealthMessageTransportCalls : public NativeUnitHealthMessageSerializedCalls {
public:
    NativeUnitHealthMessageTransportCalls(NativeUnitHealthSetterGlobals,
        NativeUnitHealthRouteGlobals, const NativeUnitHealthMessageProfile&,
        const volatile std::uint16_t& actual_tick_low_00f876b0,
        const NativeSessionTransportFlushContext&) noexcept;
protected:
    virtual NativeSessionTransportTargetFields bind_transport_target(void*) noexcept = 0;
    virtual const volatile std::uint8_t& host_suppression_a0(void*) noexcept = 0;
    virtual void* const volatile& client_destination_93c(void*) noexcept = 0;
    // Argument is the genuine embedded host+A4 or client+C8 object.
    virtual const volatile std::uint32_t& embedded_socket_858(void*) noexcept = 0;
    // Alias actual transport+(delivery ? 50h : 4Ch)+index*8, with native32-bit
    // address arithmetic. Called only for delivery0/1, after the enqueue.
    virtual volatile std::uint32_t& transport_sent_counter(void*,
        std::uint32_t index, std::uint32_t delivery) noexcept = 0;
    virtual volatile std::uint32_t& game_flush_count_217c(void*) noexcept = 0;

    void call_transport_flush_20(std::uint32_t actual_entry, void* actual_transport,
        void* actual_target, NativeBitCursor*, std::uint32_t actual_delivery) final;
    // Original ECX transport, stack target/cursor/delivery, RET0C. New source
    // interfaces retain the complete normal bodies, not private native ABI.
    void flush_host_transport_00783e00(void*, void*, NativeBitCursor*, std::uint32_t);
    void flush_client_transport_00783ea0(void*, void*, NativeBitCursor*, std::uint32_t);
    void deliver_host_transport_00a41ab0(void*, void*, NativeBitCursor*, std::uint32_t);
    void deliver_client_transport_00a419b0(void*, void*, NativeBitCursor*, std::uint32_t);
    // Original ECX embedded transport, stack type/target/payload/length, RET10.
    // Direct accepts only type5. Both capture the actual socket then reload
    // F8ABDC, call the existing complete A3C1A0 and discard its Boolean.
    void enqueue_direct_transport_00a39120(void* embedded, std::uint32_t type,
        void* target, const void* payload, std::uint32_t length);
    void enqueue_normal_transport_00a39160(void* embedded, std::uint32_t type,
        void* target, const void* payload, std::uint32_t length);
private:
    void stamp_target_and_count(void*);
    void call_current_delivery_24(void*, void*, NativeBitCursor*, std::uint32_t);
    void account_sent_bytes(void*, void*, NativeBitCursor*, std::uint32_t);
    void* volatile& current_game_00e188a8_;
    const NativeSessionTransportFlushContext& flush_;
};
} // namespace bsp
