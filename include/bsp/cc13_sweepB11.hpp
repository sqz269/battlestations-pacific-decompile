#pragma once

// cc13_sweepB11: eleventh pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB11, docs/CC13_SWEEPB11.md). Every name is a hypothesis, not a
// recovered symbol. These are new C++ interfaces over the native records, not
// drop-in replacements of the original ABI. Nothing here is wired into the
// game.
//
// Same conventions as cc13_sweepB..B10: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the same
// arguments. Cc13SweepB11Host extends Cc13SweepB10Host, so the earlier hooks
// (sized pool, native and wide strings, LuaStateOwner / LuaObject, the CRT,
// image_global, ...) are reused as they are.

#include "bsp/cc13_sweepB10.hpp"

#include <cstdint>

namespace bsp {

// The 8-byte native wide string: length in UTF-16 units, then data (released
// to the sized pool with length * 2 + 2 bytes).
struct Cc13SweepB11WideString {
    std::uint32_t length;  // +0h
    std::uint16_t* data;   // +4h
};
static_assert(sizeof(void*) != 4 || sizeof(Cc13SweepB11WideString) == 8);

class Cc13SweepB11Host : public Cc13SweepB10Host {
public:
    // ---- session DLC check (segment 43, 00776930) ----
    // The two std::list instantiations of 00776930 (both lists are 0Ch-byte
    // objects: allocator byte, +4h head node, +8h size):
    //   groups: std::list of record lists, node = {next, prev, record list
    //     (0Ch bytes at +8h: allocator, +0Ch head, +10h size)};
    //   records: std::list of {NativeString name (+8h length, +0Ch data),
    //     u64 mask (+10h low, +14h high)} nodes.
    // 0076E970 / 0076E990 (_Buyheadnode), __thiscall(ECX = list), RET; EAX =
    // the new head node (the caller stores it at list+4h and zeroes the size).
    virtual void* dlc_group_list_buy_head_0076e970(void* list) = 0;
    virtual void* dlc_record_list_buy_head_0076e990(void* list) = 0;
    // 007767F0 / 007740B0 (_Buynode: allocate a node and copy-construct the
    // value), __thiscall(ECX = list; next, prev, const value*), RET 0Ch; EAX =
    // node.
    virtual void* dlc_group_list_buy_node_007767f0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void* dlc_record_list_buy_node_007740b0(void* list, void* next, void* prev, const void* value) = 0;
    // 007736A0 / 00773740 (_Incsize: length check, then size += n),
    // __thiscall(ECX = list; n), RET 4.
    virtual void dlc_group_list_incsize_007736a0(void* list, std::uint32_t count) = 0;
    virtual void dlc_record_list_incsize_00773740(void* list, std::uint32_t count) = 0;
    // 00776540 / 00774050 (clear: destroy and free every node, relink the
    // head to itself, size 0), __thiscall(ECX = list), RET.
    virtual void dlc_group_list_clear_00776540(void* list) = 0;
    virtual void dlc_record_list_clear_00774050(void* list) = 0;
    // 007765A0 (std::list::sort of the records, the 26-bin merge sort; Ghidra
    // name CG_vector_deleting_dtor_007765a0 is a tag artefact), __thiscall(ECX
    // = list), RET.
    virtual void dlc_record_list_sort_007765a0(void* list) = 0;
    // 00774280 (std::list::unique of the records, comparing the names),
    // __thiscall(ECX = list), RET.
    virtual void dlc_record_list_unique_00774280(void* list) = 0;
    // 0076E7F0 BSP_WideString_MatchesAt, __thiscall(ECX = string; const
    // WideString* needle, pos), RET 8; AL = 1 when the needle's units match
    // the string from pos.
    virtual bool wide_string_matches_at_0076e7f0(void* string, const void* needle, std::uint32_t pos) = 0;
    // 00770E50 BSP_WideString_Substring, __thiscall(ECX = string; WideString*
    // out, pos, count), RET 0Ch; EAX = out.
    virtual void* wide_string_substring_00770e50(void* string, void* out, std::int32_t pos, std::int32_t count) = 0;
    // 005CF570 BSP_WideString_Concat, __thiscall(ECX = lhs; WideString* out,
    // const WideString* rhs), RET 8; EAX = out.
    virtual void* wide_string_concat_005cf570(void* lhs, void* out, const void* rhs) = 0;
};

// 00776930 BSP_SessionHostInfo_CheckDlc (vtable 00D039CC slot 4),
// __thiscall(ECX = host_info; client, unused, WideString* out), RET 0Ch.
// missing = host DLC mask (+170h / +174h) & ~client mask (+48h / +4Ch). Runs
// SCRIPTS/datatables/DLC.lua in a private LuaStateOwner and, for every missing
// bit i, gathers the DLCTable entries whose 64-character bit string has '1' at
// [63 - i] (name = key, mask = the string read MSB first). Each group gives
// one record: the only one when it has fewer than two, else the first or the
// second, by the bits where their masks differ. The records are sorted and
// made unique; only the FIRST names the packs: "globals.<name>_<language>"
// (language = the client's C string at +32h) unless its localization through
// [00F8BC4C] starts with the key itself (unresolved), in which case
// "globals.<name>_english"; either way followed by "|.,|". The
// list (minus the trailing separator) is resolved into out, then cut to 90
// units + "..." when longer than 100.
void cc13_sweepB11_session_check_dlc_00776930(void* host_info, void* client, std::uint32_t unused,
                                              Cc13SweepB11WideString* out, Cc13SweepB11Host& host);

}  // namespace bsp
