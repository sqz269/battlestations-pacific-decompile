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

#include "bsp/cc13_seg40.hpp"

namespace bsp::cc13_seg40b {

using bsp::Cc13Seg40Host;

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

// --- functions cc16 found undefined (defined in Ghidra by this packet) ---
// Session-message serializers, __thiscall(ECX = message, cursor), RET 4. Each writes or reads
// the extended header (0075B480 / 0075B4C0), then the message's +1Ch byte as a boolean.
// 00728DE0 / 00728E10: GUN_SET_FIRE_MODE (vtable 00CFDE3C slots +4h / +8h); the second field is
// the dword at +20h, 2 bits.
void gun_set_fire_mode_write_00728de0(Cc13Seg40Host& host, void* message, void* cursor);
void gun_set_fire_mode_read_00728e10(Cc13Seg40Host& host, void* message, void* stream);
// 00728EC0 / 00728EF0: R_T_GUN_SET_CONTFIRING (vtable 00CFDE50 slots +4h / +8h); the second
// field is the byte at +20h as a boolean.
void r_t_gun_set_contfiring_write_00728ec0(Cc13Seg40Host& host, void* message, void* cursor);
void r_t_gun_set_contfiring_read_00728ef0(Cc13Seg40Host& host, void* message, void* stream);

// 0072A090, __thiscall(ECX = object, const PropertyKey* key, arg, uint8_t* out), RET 0Ch. The
// "destroyed" property reader shared by nine vtables (00CF9840, 00CF9AB0, 00CFAC50, 00CFBEB8,
// 00CFC0F0, 00CFC328, 00CFE240, 00CFE4A0, 00CFE6E0 hold it). When key+4h is a non-null name
// and _stricmp(name, "destroyed" @ 00CE44D0) == 0, *out = object+3B8h and it returns true;
// otherwise false. The middle argument is not read.
bool destroyed_property_get_0072a090(Cc13Seg40Host& host, const void* object, const void* key,
                                     std::uint8_t* out);

// 007312E0, __thiscall(ECX = command, writer), RET 4: BSP_TorpedoShotCommand_Write, slot +8h of
// 00CFE740 (the reader 00731270 is slot +4h). Writer slot +0Ch three times:
// ("type", string "torpedo" @ 00CFB414), ("heading", float +4h), ("swimdepth", float +8h).
void torpedo_shot_command_write_007312e0(Cc13Seg40Host& host, const void* command, void* writer);

// 007358C0, __thiscall(ECX = object, flags), RET 4: the scalar deleting destructor in vtable
// 00CFEA08 slot 0. Clears the singleton cell 01090298, restores the SingletonBase vtable
// 00CE3818, frees with 00BF65AC when flags bit 0 is set, returns the object.
void* singleton_01090298_scalar_deleting_destructor_007358c0(Cc13Seg40Host& host, void* object,
                                                             std::uint32_t flags,
                                                             void** singleton_cell_01090298);

// 00735B40, RET, ECX never read (a vtable slot, so callers pass one): returns [00E188A8] + 1A0Ch (the game object's member at
// +1A0Ch). Referenced only by the vtable entry at 00CFEAC4, next to 00735B30
// BSP_Application_IgnoreVfsCallback.
void* game_member_1a0c_00735b40(void* game_00e188a8) noexcept;

}  // namespace bsp::cc13_seg40b

#endif
