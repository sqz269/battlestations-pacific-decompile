// Segment 40 leftovers (00728A90..00735EC0), packet cc13-seg40b (marker cc13_seg40b).
//
// After cc13-seg40 / cc9's gunnery host the segment's only non-library rows without a body were
// eleven trivial bodies: seven default virtuals of the gun-bot base vtable 00CFDFA0 (constructor
// 0072BBD0, docs/GUN_BOT_TICKS.md section 2) and four base-destructor bodies that only restore
// a root vtable and are reached only from exception-unwind funclets. docs/CC13_SEG40B.md has the
// census (segment 58 included) and the evidence.
//
// New C++ interfaces, not drop-in binary replacements; nothing is bound into bsp_game. Names are
// hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG40B_HPP
#define BSP_CC13_SEG40B_HPP

#include <cstdint>

namespace bsp::cc13_seg40b {

// The empty C string literal the name getter returns (00CE3A0C, four zero bytes).
inline constexpr std::uint32_t kEmptyStringLiteral = 0x00CE3A0Cu;
// BSP_SessionMessageRoot's vtable (docs/CC13_FRONT6.md, 007F7D90).
inline constexpr std::uint32_t kSessionMessageRootVtable = 0x00CE4974u;
// The torpedo shot command's base vtable (src/cc13_seg40.cpp 00731220).
inline constexpr std::uint32_t kShotCommandBaseVtable = 0x00CFE2B8u;
// The gun-bot base vtable these defaults sit in. Derived vtables keep some of them at the same
// slots: 00CFE000, 00D0D360 (the unit gunnery AI), 00D180A0, 00D18140 and 00D18238 (absolute
// scan of .rdata; each start is the slot +0Ch tick entry less 0Ch, docs/BOT_FIRE_TARGET.md).
inline constexpr std::uint32_t kGunBotBaseVtable = 0x00CFDFA0u;

// --- gun-bot base default virtuals (__thiscall, ECX = bot) ---
// 0072BC70, slot +14h: MOV EAX,00CE3A0C / RET. Returns the empty literal "".
const char* gun_bot_default_name_0072bc70(const void* bot) noexcept;
// 0072BC90, slot +1Ch: XOR AL,AL / RET 4. One stack argument, returns false.
bool gun_bot_default_false_0072bc90(const void* bot, std::uint32_t arg) noexcept;
// 0072BCA0, slot +20h, and 0072BCB0, slot +24h: FLD1 / RET. Return 1.0f.
float gun_bot_default_one_0072bca0(const void* bot) noexcept;
float gun_bot_default_one_0072bcb0(const void* bot) noexcept;
// 0072BCC0, slot +28h: RET 8. Two stack arguments, no effect.
void gun_bot_default_noop2_0072bcc0(void* bot, std::uint32_t arg0, std::uint32_t arg1) noexcept;
// 0072BCD0, slot +2Ch: XOR EAX,EAX / RET. Returns 0 (null / zero).
std::uint32_t gun_bot_default_zero_0072bcd0(const void* bot) noexcept;
// 0072BD00, slot +3Ch: RET 4. One stack argument, no effect.
void gun_bot_default_noop1_0072bd00(void* bot, std::uint32_t arg) noexcept;

// --- base-destructor bodies, __thiscall(ECX = object), RET; [ECX] = vtable ---
// 00729090 / 007290A0 / 007293B0 store 00CE4974; reached only by JMP from the unwind funclets
// 00C856D0, 00C856B0 and 00C8587B (ECX = a stack message object).
void session_message_root_store_vtable_00729090(void* object) noexcept;
void session_message_root_store_vtable_007290a0(void* object) noexcept;
void session_message_root_store_vtable_007293b0(void* object) noexcept;
// 00730B50 stores 00CFE2B8; reached only by JMP from 00C85890, 00C858B0 and 00C858D0
// (ECX = [ebp-10h], a heap shot command under construction).
void shot_command_base_store_vtable_00730b50(void* object) noexcept;

}  // namespace bsp::cc13_seg40b

#endif
