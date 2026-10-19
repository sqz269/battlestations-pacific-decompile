#pragma once
#include <cstdint>
#include "bsp/cc13_seg33a.hpp"

namespace bsp::oct18b_nB_mid {
// Exceptions to same-file integration: cc13_seg30.hpp is leased by nA2,
// cc13_seg33a.hpp/.cpp by nA3. No fallback implementations are supplied.
struct MotherShipCreateReadHooks {
    virtual ~MotherShipCreateReadHooks() = default;
    virtual void ship_create_read_00813bd0(void* message, void* stream) = 0;
    virtual void air_ops_stock_read_006d4eb0(void* stock, void* stream) = 0;
};
// ECX=message, stack stream, RET4; base read then stock at message+12Ch.
void read_mothership_create_007593b0(void* message, void* stream, MotherShipCreateReadHooks&);
// Unconditional JMP006E6410; receiver and the ignored stack word retained.
void projectile_set_air_mode_thunk_007b9070(cc13_seg33a::Seg33aHost&, void* shot, void* unused);
// The 00D039CC table candidate clusters slots6/7; that alone does not prove
// class ownership. Only the byte operation is established; meaning is unknown.
void decrement_session_byte12b_0076a710(void* owner) noexcept;
// Identity leaves: 00780800 scan refs D64B1C/D64F1C (RET4),
// 0078EAA0 D28C20/D2C420 (RET). Such matches do not prove callback, descriptor
// or class provenance. Existing descriptive identifiers are provisional.
void* descriptor_identity_00780800(void* receiver, std::uint32_t unused) noexcept;
void* descriptor_identity_0078eaa0(void* receiver) noexcept;
// 00D040B8 slot89, XOR AL,AL; RET4 (upper EAX unspecified natively).
bool session_false_slot_00781080(std::uint32_t unused) noexcept;
// 0078CFB0: ECX=destination holder, stack owner pointer, EAX=destination,
// RET4. Null owner clears the holder; otherwise copy owner+Ch and increment
// held+4 via the actual InterlockedIncrement boundary (return ignored).
// A non-null owner has a valid non-null held pointer, as the image requires.
// Class and authored-vs-template provenance remain unknown: no library credit
// exclusion is inferred from the chance D2CE88 data match or the body shape.
void** copy_ref_holder_from_owner_0078cfb0(void** destination, const void* owner,
    std::int32_t (*interlocked_increment)(volatile std::int32_t*));
}
