// cc13-cgbuildC: rebuilt constructors/destructors formerly tagged CG_* compiler helpers.
// See include/bsp/cg_rebuild_c_cc13.hpp for the host contract. Evidence for every routine
// is the exported Ghidra listing (bsp.py show <addr> --asm) read instruction by
// instruction on 2026-10-24; offsets, constants and call order below are the listing's.
// Names are hypotheses. Build-tested only: no caller in src, not fixture- or game-validated.

#include "bsp/cg_rebuild_c_cc13.hpp"

#include <cstddef>
#include <cstring>

namespace bsp::cg_rebuild_c_cc13 {
namespace {

std::uint8_t* at(void* p, std::uint32_t offset) { return static_cast<std::uint8_t*>(p) + offset; }
const std::uint8_t* at(const void* p, std::uint32_t offset) {
    return static_cast<const std::uint8_t*>(p) + offset;
}
template <class T> T& field(void* p, std::uint32_t offset) { return *reinterpret_cast<T*>(at(p, offset)); }
template <class T> T read(const void* p, std::uint32_t offset) {
    T value;
    std::memcpy(&value, at(p, offset), sizeof value);
    return value;
}
template <class T> T* pointer_from_dword(std::uint32_t value) {
    return reinterpret_cast<T*>(static_cast<std::uintptr_t>(value));
}

// The inlined NativeString release: header {+0 length, +4 data}. When data is set,
// 00419CC0 gives the pool and 00BD1510 returns (data, length + 1, 1). The header is
// not cleared.
void release_string(void* header, CgRebuildCHost& host) {
    void* data = field<void*>(header, 4);
    if (data != nullptr) {
        void* pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, data, field<std::uint32_t>(header, 0) + 1, 1);
    }
}

// The inlined NativeString copy of 005FE010: zero the header; unless dst == src,
// 0041DD40(dst; src length, 1) and, when the source length is non-zero, memcpy the
// destination's (resized) length from the source data.
void copy_string(void* dst, const void* src, CgRebuildCHost& host) {
    field<std::uint32_t>(dst, 0) = 0;
    field<void*>(dst, 4) = nullptr;
    if (dst == src) return;
    host.native_string_resize_0041dd40(dst, read<std::uint32_t>(src, 0), 1);
    if (read<std::uint32_t>(src, 0) != 0) {
        host.memcpy_00bf7680(field<void*>(dst, 4), read<const void*>(src, 4), field<std::uint32_t>(dst, 0));
    }
}

// The inlined intrusive handle release: InterlockedDecrement(p + 4) and, at zero,
// vtable[0]() on p (ECX = p, no arguments).
void release_handle_object(void* p, CgRebuildCHost& host) {
    if (host.interlocked_decrement_00ce2220(at(p, 4)) == 0) host.virtual_call_0(p, 0);
}

}  // namespace

// ---------------------------------------------------------------------------
// 005FE010 BSP_HintDefinition_CopyConstruct_005fe010 (282 insns)
// ABI: __thiscall(ECX = dst; const src*) RET 4, EAX = dst (005FE35D, 005FE36C).
// Layout from the listing: ten NativeStrings at +00h..+48h, byte +50h, eight 18h-byte
// three-string records at +54h (copied by 005FD790, destroyed by 004C5720), bytes
// +114h/+115h/+116h, NativeString +118h, dwords +120h/+124h; 128h bytes in all.
// Callers: 005FE370, 005FE430, 00601200, 00690B20, 00690FD0 (all already rebuilt
// with this routine as a host call).
// EH (handler 00C76A3B): state k is stored after string k completes (k = 0..9; state 9
// at 005FE2CB, just before the array copy). The array and the +118h string get no state
// of their own, so an exception from the +118h resize does not destroy the array; the
// 00BF9A96 iterator itself unwinds a partially copied array. Not modelled here.
// ---------------------------------------------------------------------------
void* hint_definition_copy_construct_005fe010(void* dst, const void* src, CgRebuildCHost& host) {
    for (std::uint32_t i = 0; i < 10; ++i) copy_string(at(dst, i * 8), at(src, i * 8), host);
    field<std::uint8_t>(dst, 0x50) = read<std::uint8_t>(src, 0x50);
    host.eh_vector_copy_construct_00bf9a96(at(dst, 0x54), at(src, 0x54), 0x18, 8, 0x005FD790u, 0x004C5720u);
    field<std::uint8_t>(dst, 0x114) = read<std::uint8_t>(src, 0x114);
    field<std::uint8_t>(dst, 0x115) = read<std::uint8_t>(src, 0x115);
    field<std::uint8_t>(dst, 0x116) = read<std::uint8_t>(src, 0x116);
    copy_string(at(dst, 0x118), at(src, 0x118), host);
    field<std::uint32_t>(dst, 0x120) = read<std::uint32_t>(src, 0x120);
    field<std::uint32_t>(dst, 0x124) = read<std::uint32_t>(src, 0x124);
    return dst;
}

// ---------------------------------------------------------------------------
// 004C8F00 BSP_HintDefinition_Destruct_004c8f00 (153 insns)
// ABI: __thiscall(ECX = record), plain RET; no vtable store, no delete flag.
// Teardown of the 005FE010 layout in reverse: +118h string (state 0Ah), the record array
// through 00BF7C6E(+54h, 18h, 8, 004C5720) (state 9), then strings +48h..+00h (states
// 8..0, and -1 for +00h). Handler 00C653A1. Existing caller model:
// src/cc13_sweepB8.cpp (hint_definition_destruct_004c8f00 host hook).
// ---------------------------------------------------------------------------
void hint_definition_destruct_004c8f00(void* record, CgRebuildCHost& host) {
    release_string(at(record, 0x118), host);
    host.eh_vector_destruct_00bf7c6e(at(record, 0x54), 0x18, 8, 0x004C5720u);
    for (std::uint32_t i = 10; i-- > 0;) release_string(at(record, i * 8), host);
}

// ---------------------------------------------------------------------------
// 004D74E0 BSP_SceneRecord_Destruct_004d74e0 (264 insns)
// ABI: __thiscall(ECX = record), plain RET. The only caller is the scalar deleting
// destructor 004DA5F0 (vtable 00CE7950). Handler 00C663BB; states 14h down to -1 in the
// order below (each comment gives the state the listing stores before the step).
// The two owned blocks at +C18h/+C1Ch are released and freed but their slots are not
// cleared. 004D1A50's arguments are from the pushes at 004D762A..004D7632.
// ---------------------------------------------------------------------------
void scene_record_destruct_004d74e0(void* record, CgRebuildCHost& host) {
    field<std::uint32_t>(record, 0) = 0x00CE7950u;
    // state 14h: owned blocks holding a NativeString at +08h.
    for (std::uint32_t offset : {0xC18u, 0xC1Cu}) {
        void* block = field<void*>(record, offset);
        if (block != nullptr) {
            release_string(at(block, 8), host);
            host.free_00bf65ac(block);
        }
    }
    host.sound_record_set_destruct_004c9870(at(record, 0x1064));            // state 13h
    host.pod_pointer_array_resize_00468050(at(record, 0x1058), 0);          // state 12h
    host.free_00bf6989(field<void*>(record, 0x1058));
    release_string(at(record, 0x104C), host);                               // state 11h
    release_string(at(record, 0x1044), host);                               // state 10h
    unknown_dtor_004ccb70(at(record, 0xD5C), host);                         // state 0Fh
    host.effect_handle_vector_resize_004cb160(at(record, 0xD50), 0);        // state 0Eh
    host.free_00bf6989(field<void*>(record, 0xD50));
    {                                                                       // state 0Dh
        void* tree = at(record, 0xC78);
        void* head = field<void*>(record, 0xC7C);
        std::uint32_t out[2]{};
        host.raw_string_tree_erase_range_004d1a50(tree, out, tree, read<void*>(head, 0), tree, head);
        host.free_00bf65ac(field<void*>(record, 0xC7C));
        field<std::uint32_t>(record, 0xC7C) = 0;
        field<std::uint32_t>(record, 0xC80) = 0;
    }
    host.native_string_vector_resize_00427110(at(record, 0xC6C), 0);        // state 0Ch
    host.free_00bf6989(field<void*>(record, 0xC6C));
    // states 0Bh..06h
    for (std::uint32_t offset : {0xC64u, 0xC54u, 0xC3Cu, 0xC34u, 0xC2Cu, 0xC24u}) release_string(at(record, offset), host);
    host.scene_record_header_destruct_004c6520(at(record, 0x990));          // state 5
    release_string(at(record, 0x980), host);                                // state 4
    host.eh_vector_destruct_00bf7c6e(at(record, 0x928), 8, 0xB, 0x0041DD20u);  // state 3
    // states 2, 1, 0
    for (std::uint32_t offset : {0x920u, 0x918u, 0x90Cu}) release_string(at(record, offset), host);
    host.eh_vector_destruct_00bf7c6e(at(record, 0x4), 0x120, 8, 0x004C9820u);  // state -1
}

// ---------------------------------------------------------------------------
// 00A475A0 BSP_XLiveSystem_Destruct_00a475a0 (190 insns)
// ABI: __thiscall(ECX = system), plain RET. The matching constructor is 00A47EE0
// (src/online_cc13.cpp, system_construct_00a47ee0; 2218h-byte object, vtable 00D24878).
// cc16 ruled the XLive system game code; the XLive library work happens inside the bound
// callees (00A46EE0 shutdown, 00A4B020 XNet terminate, 00A493A0 voice, 00A4B410 QoS,
// 00A4B460 matchmaking), none of which is rebuilt here.
// Handler 00CB47FE; states 18h, 16h..13h, 11h, 10h, 0Eh..08h, -1 as commented. The
// std::string at +128h (buffer +12Ch, size +13Ch, capacity +140h) and the std::vectors
// at +1BE8h, +1F1Ch, +174h are torn down inline, as the listing does. The two range
// destroys take ECX = first, EDX = last and push (vector, [ESP+0Ch]); that local holds
// the system pointer (00A475BB).
// ---------------------------------------------------------------------------
void xlive_system_destruct_00a475a0(void* system, CgRebuildCHost& host) {
    field<std::uint32_t>(system, 0) = 0x00D24878u;                          // state 18h
    if (field<std::uint8_t>(system, 0xEC) != 0) host.xlive_system_shutdown_00a46ee0(system);
    host.stl_list_tidy_004c1f70(at(system, 0x2200));
    host.xlive_session_terminate_xnet_00a4b020(at(system, 0x2098));         // state 16h
    host.eh_vector_destruct_00bf7c6e(at(system, 0x2064), 0xC, 4, 0x00A44340u);   // state 15h
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1FFC), 0x18, 4, 0x00A43080u);  // state 14h
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1F3C), 0xC, 0x10, 0x004C8BF0u); // state 13h
    {
        void* vector = at(system, 0x1F1C);
        void* first = field<void*>(system, 0x1F20);
        if (first != nullptr) {
            host.string_range_destroy_004ce520(first, field<void*>(system, 0x1F24), vector, system);
            host.free_00bf65ac(field<void*>(system, 0x1F20));
        }
        field<std::uint32_t>(system, 0x1F20) = 0;
        field<std::uint32_t>(system, 0x1F24) = 0;
        field<std::uint32_t>(system, 0x1F28) = 0;
    }
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1D5C), 0x1C, 0x10, 0x004072D0u); // state 11h
    host.xlive_voice_destroy_00a493a0(at(system, 0x1C24));                  // state 10h
    if (field<void*>(system, 0x1BEC) != nullptr) host.free_00bf65ac(field<void*>(system, 0x1BEC));
    field<std::uint32_t>(system, 0x1BEC) = 0;
    field<std::uint32_t>(system, 0x1BF0) = 0;
    field<std::uint32_t>(system, 0x1BF4) = 0;
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1BC4), 4, 4, 0x00A43030u);  // state 0Eh
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1BB4), 4, 4, 0x00A43030u);  // state 0Dh
    host.eh_vector_destruct_00bf7c6e(at(system, 0x1BA4), 4, 4, 0x00A43030u);  // state 0Ch
    field<std::uint32_t>(system, 0x1BA0) = 0x00D24764u;                     // state 0Bh
    host.trivial_body_00a48f00(at(system, 0x1BA0));
    field<std::uint32_t>(system, 0x1B9C) = 0x00D24764u;                     // state 0Ah
    host.trivial_body_00a48f00(at(system, 0x1B9C));
    host.xlive_host_qos_cancel_00a4b410(at(system, 0x1AE8));                // state 9
    host.xlive_matchmaking_reset_00a4b460(at(system, 0x184));               // state 8
    {
        void* vector = at(system, 0x174);
        void* first = field<void*>(system, 0x178);
        if (first != nullptr) {
            host.session_record_destroy_range_00571af0(first, field<void*>(system, 0x17C), vector, system);
            host.free_00bf65ac(field<void*>(system, 0x178));
        }
        field<std::uint32_t>(system, 0x178) = 0;
        field<std::uint32_t>(system, 0x17C) = 0;
        field<std::uint32_t>(system, 0x180) = 0;
    }
    host.stl_inst_00a45510(at(system, 0x168));
    host.free_00bf65ac(field<void*>(system, 0x16C));
    field<std::uint32_t>(system, 0x16C) = 0;
    if (field<std::uint32_t>(system, 0x140) >= 0x10) host.free_00bf65ac(field<void*>(system, 0x12C));
    field<std::uint32_t>(system, 0x140) = 0xF;
    field<std::uint32_t>(system, 0x13C) = 0;
    field<std::uint8_t>(system, 0x12C) = 0;
    for (std::uint32_t offset : {0x118u, 0x10Cu, 0x100u, 0xF4u}) {
        host.stl_list_clear_strings_00688780(at(system, offset));
        host.free_00bf65ac(field<void*>(system, offset + 4));
        field<std::uint32_t>(system, offset + 4) = 0;
    }
    host.game_participant_pairs_destroy_storage_004c5860(at(system, 0xDC));
    host.xlive_stats_block_destruct_00994180(system);                       // state -1
}

// ---------------------------------------------------------------------------
// 006821C0 Unknown_VtCF7184_Construct_006821c0 (163 insns)
// ABI: __thiscall(ECX = screen), EBP frame aligned to 8, plain RET, EAX = screen
// (0068238F). Only caller: 0068CC70 BSP_InGameInterface_Init (0068D6E1).
// Base 004F7180, vtable 00CF7184, two 4-slot handle arrays (+30h: 004BA0D0/004C3810,
// +40h: 0054D440/00524180), +50h..+5Ch and +74h zeroed. Then for each of the four names
// in the table 00E08CC4..00E08CD4: float +08h+4i = 0, byte +28h+i = 0; a null name
// releases slot +30h+4i; otherwise a temporary NativeString (Resize to strlen, memcpy
// length + 1) is looked up through 00A83FD0 on the cache [00F8BBE8] and the result is
// assigned into the slot with retain/release, then the temporary handle and string die.
// Handler 00C7D644; states 0, 1, 2, then 3/4 inside the loop body.
// The second dead `slot = 0` on the null-name path (0068236A) is kept.
// ---------------------------------------------------------------------------
void* vt_cf7184_construct_006821c0(void* screen, CgRebuildCHost& host) {
    host.front_end_screen_construct_004f7180(screen);
    field<std::uint32_t>(screen, 0) = 0x00CF7184u;                          // state 0
    host.eh_vector_construct_00bf7cd1(at(screen, 0x30), 4, 4, 0x004BA0D0u, 0x004C3810u);
    host.eh_vector_construct_00bf7cd1(at(screen, 0x40), 4, 4, 0x0054D440u, 0x00524180u);  // state 1
    for (std::uint32_t offset : {0x50u, 0x54u, 0x58u, 0x5Cu, 0x74u}) field<std::uint32_t>(screen, offset) = 0;  // state 2
    for (std::uint32_t i = 0; i < 4; ++i) {
        field<float>(screen, 0x08 + i * 4) = 0.0f;
        field<std::uint8_t>(screen, 0x28 + i) = 0;
        void*& slot = field<void*>(screen, 0x30 + i * 4);
        const char* name = pointer_from_dword<const char>(host.global_dword(0x00E08CC4u + i * 4));
        if (name == nullptr) {
            if (slot != nullptr) {
                release_handle_object(slot, host);
                slot = nullptr;
            }
            slot = nullptr;
            continue;
        }
        struct { std::uint32_t length; void* data; } text{0, nullptr};
        host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(name)), 1);
        if (text.data != nullptr) host.memcpy_00bf7680(text.data, name, text.length + 1);
        void* acquired = nullptr;                                           // state 3
        void* cache = pointer_from_dword<void>(host.global_dword(0x00F8BBE8u));
        void* value = *host.sound_sample_cache_acquire_00a83fd0(cache, &acquired, &text);
        void* old = slot;                                                   // state 4
        if (old != value) {
            slot = value;
            if (value != nullptr) host.interlocked_increment_00ce221c(at(value, 4));
            if (old != nullptr) release_handle_object(old, host);
        }
        if (acquired != nullptr) {                                          // state 3
            release_handle_object(acquired, host);
            acquired = nullptr;
        }
        release_string(&text, host);                                        // state 2
    }
    return screen;
}

// ---------------------------------------------------------------------------
// 00682030 Unknown_VtCF7184_Destruct_00682030 (89 insns)
// ABI: __thiscall(ECX = screen), plain RET. Handler 00C7D5F4; state 2, then 1, 0, -1.
// The page [+70h] goes back through 00AA31F0 on the 004C12B0 manager. Each of the four
// +40h slots that is set gets vtable[2](0) (CALL [vtbl+8] after PUSH 0) and is then
// released; the +30h slot is released. The listing's NEG/SBB/TEST 0E19311h is the
// safe-bool test, i.e. slot != null. Both slots are cleared unconditionally.
// ---------------------------------------------------------------------------
void vt_cf7184_destruct_00682030(void* screen, CgRebuildCHost& host) {
    field<std::uint32_t>(screen, 0) = 0x00CF7184u;
    void* manager = host.gui_manager_get_or_create_004c12b0();
    host.gui_manager_remove_and_destroy_page_00aa31f0(manager, field<std::uint32_t>(screen, 0x70));
    for (std::uint32_t i = 0; i < 4; ++i) {
        void*& late = field<void*>(screen, 0x40 + i * 4);
        if (late != nullptr) {
            host.virtual_call_1(late, 8, 0);
            if (late != nullptr) release_handle_object(late, host);
            late = nullptr;
        }
        void*& early = field<void*>(screen, 0x30 + i * 4);
        if (early != nullptr) release_handle_object(early, host);
        early = nullptr;
    }
    host.eh_vector_destruct_00bf7c6e(at(screen, 0x40), 4, 4, 0x00524180u);  // state 1
    host.eh_vector_destruct_00bf7c6e(at(screen, 0x30), 4, 4, 0x004C3810u);  // state 0
    host.front_end_screen_destruct_004f71a0(screen);                        // state -1
}

// ---------------------------------------------------------------------------
// 0064B780 Unknown_VtCF5E30_Construct_0064b780 (50 insns)
// ABI: __thiscall(ECX = screen), plain RET, EAX = screen. HUD screen 14 in
// src/ingame_interface.cpp kInGameHudScreens (slot +78h, 1B0h bytes, vtable 00CF5E30).
// Base 004F7180; state 0; handle array +0Ch (4 x 4, 00415680/0041DE40). The member
// at +170h gets vtable 00CF5C94 with +174h/+178h/+17Ch/+184h = 0 and byte +180h = 1;
// +18Ch..+198h = 0, +1ACh = 1, floats +19Ch..+1A8h = 0.0 (XORPS), +78h/+7Ch/+80h = 0.
// Handler 00C7A9C8; the state stays 0 after the array.
// ---------------------------------------------------------------------------
void* vt_cf5e30_construct_0064b780(void* screen, CgRebuildCHost& host) {
    host.front_end_screen_construct_004f7180(screen);
    field<std::uint32_t>(screen, 0) = 0x00CF5E30u;
    host.eh_vector_construct_00bf7cd1(at(screen, 0xC), 4, 4, 0x00415680u, 0x0041DE40u);
    for (std::uint32_t offset : {0x1Cu, 0x20u, 0x174u, 0x178u, 0x17Cu, 0x184u}) field<std::uint32_t>(screen, offset) = 0;
    field<std::uint32_t>(screen, 0x170) = 0x00CF5C94u;
    field<std::uint8_t>(screen, 0x180) = 1;
    for (std::uint32_t offset : {0x18Cu, 0x190u, 0x194u, 0x198u}) field<std::uint32_t>(screen, offset) = 0;
    field<std::uint32_t>(screen, 0x1AC) = 1;
    for (std::uint32_t offset : {0x19Cu, 0x1A0u, 0x1A4u, 0x1A8u}) field<float>(screen, offset) = 0.0f;
    for (std::uint32_t offset : {0x78u, 0x7Cu, 0x80u}) field<std::uint32_t>(screen, offset) = 0;
    return screen;
}

// ---------------------------------------------------------------------------
// 0064D740 Unknown_VtCF5E30_Destruct_0064d740 (133 insns)
// ABI: __thiscall(ECX = screen), plain RET. Handler 00C7AC22; states 4, 3, 2, 1, 0, -1.
// Pages +38h, +3Ch, +34h, +40h (that order) go back through 00AA31F0, each on a fresh
// 004C12B0 manager. The owned blocks +78h/+7Ch/+80h are {?, first, last, end}
// vectors: free first when set, zero +4/+8/+0Ch, free the block, clear the slot.
// ---------------------------------------------------------------------------
void vt_cf5e30_destruct_0064d740(void* screen, CgRebuildCHost& host) {
    field<std::uint32_t>(screen, 0) = 0x00CF5E30u;                          // state 4
    for (std::uint32_t offset : {0x38u, 0x3Cu, 0x34u, 0x40u}) {
        void* manager = host.gui_manager_get_or_create_004c12b0();
        host.gui_manager_remove_and_destroy_page_00aa31f0(manager, field<std::uint32_t>(screen, offset));
    }
    for (std::uint32_t offset : {0x78u, 0x7Cu, 0x80u}) {
        void* block = field<void*>(screen, offset);
        if (block == nullptr) continue;
        if (field<void*>(block, 4) != nullptr) host.free_00bf65ac(field<void*>(block, 4));
        field<std::uint32_t>(block, 4) = 0;
        field<std::uint32_t>(block, 8) = 0;
        field<std::uint32_t>(block, 0xC) = 0;
        host.free_00bf65ac(block);
        field<std::uint32_t>(screen, offset) = 0;
    }
    host.observed_owner_base_destroy_0064a610(at(screen, 0x170));          // state 3
    for (std::uint32_t offset : {0x20u, 0x1Cu}) {                           // states 2, 1
        void* handle = field<void*>(screen, offset);
        if (handle != nullptr) {
            release_handle_object(handle, host);
            field<std::uint32_t>(screen, offset) = 0;
        }
    }
    host.eh_vector_destruct_00bf7c6e(at(screen, 0xC), 4, 4, 0x0041DE40u);  // state 0
    host.front_end_screen_destruct_004f71a0(screen);                        // state -1
}

// ---------------------------------------------------------------------------
// 0086E5A0 BSP_EffectFlare_Destruct_0086e5a0 (65 insns)
// ABI: __thiscall(ECX = flare), plain RET. Handler 00C95B3B; states 1, 0, 2, -1 (the
// base part numbers its own state 2). Vtable 00D0D614; each non-null element of the
// pointer array {+28h data, +2Ch count} has its two NativeStrings at +48h destroyed
// (00BF7C6E with 0041DD20) and is freed; the count and data are re-read each pass.
// Then the array is resized to 0 (0086AA10) and its data freed; base vtable 00D0D570,
// the NativeString at +08h is released, and 00BD30F0 finishes the base.
// ---------------------------------------------------------------------------
void effect_flare_destruct_0086e5a0(void* flare, CgRebuildCHost& host) {
    field<std::uint32_t>(flare, 0) = 0x00D0D614u;                           // state 1
    for (std::uint32_t i = 0; i < field<std::uint32_t>(flare, 0x2C); ++i) {
        void* element = field<void**>(flare, 0x28)[i];
        if (element == nullptr) continue;
        host.eh_vector_destruct_00bf7c6e(at(element, 0x48), 8, 2, 0x0041DD20u);
        host.free_00bf65ac(element);
    }
    host.native_dword_vector_resize_0086aa10(at(flare, 0x28), 0);          // state 0
    host.free_00bf6989(field<void*>(flare, 0x28));
    field<std::uint32_t>(flare, 0) = 0x00D0D570u;                           // state 2
    release_string(at(flare, 0x8), host);
    host.ref_counted_destroy_base_00bd30f0(flare);                          // state -1
}

// ---------------------------------------------------------------------------
// 0071D480 BSP_CommandControllerBase_Destruct_0071d480 (55 insns)
// ABI: __thiscall(ECX = controller), plain RET. Handler 00C85095; states 2, 1, 0, -1.
// Vtables 00CFDA40 (+0) and 00CFDA28 (+1Ch). The ten owned objects at +1A4h and the
// one at +38h get vtable[0](1) (scalar deleting destructor, delete) and their slots are
// cleared; then 00BF7C6E(+54h, 1Ch, 0Ah, 005F9640), 00695870 on +1Ch, 00875B30.
// Existing caller model: src/cc13_seg52.cpp (weapon_director_base_destruct_0071d480).
// ---------------------------------------------------------------------------
void command_controller_base_destruct_0071d480(void* controller, CgRebuildCHost& host) {
    field<std::uint32_t>(controller, 0) = 0x00CFDA40u;
    field<std::uint32_t>(controller, 0x1C) = 0x00CFDA28u;                   // state 2
    for (std::uint32_t i = 0; i < 10; ++i) {
        void*& owned = field<void*>(controller, 0x1A4 + i * 4);
        if (owned != nullptr) {
            host.virtual_call_1(owned, 0, 1);
            owned = nullptr;
        }
    }
    if (void*& owned = field<void*>(controller, 0x38); owned != nullptr) {
        host.virtual_call_1(owned, 0, 1);
        owned = nullptr;
    }
    host.eh_vector_destruct_00bf7c6e(at(controller, 0x54), 0x1C, 0xA, 0x005F9640u);  // state 1
    host.callback_owner_destroy_00695870(at(controller, 0x1C));             // state 0
    host.tick_subnode_base_cleanup_00875b30(controller);                    // state -1
}

// ---------------------------------------------------------------------------
// 00A37130 Unknown_VtD232C4_Construct_00a37130 (27 insns)
// ABI: __thiscall(ECX = object), plain RET, EAX = object. The FD8h-byte singleton kept
// at [00F8AAFC] (src/engine_seg4_6_9_cc13.cpp row for 004C1C50). Handler 00CB3930;
// state 0, vtable 00D232C4, three 10h-byte records at +FA8h (00A33470/00A32FF0),
// state 1, then 00A335D0 BSP_Ai_LoadHighLevelGlobals on the object.
// ---------------------------------------------------------------------------
void* vt_d232c4_construct_00a37130(void* object, CgRebuildCHost& host) {
    field<std::uint32_t>(object, 0) = 0x00D232C4u;                          // state 0
    host.eh_vector_construct_00bf7cd1(at(object, 0xFA8), 0x10, 3, 0x00A33470u, 0x00A32FF0u);
    host.ai_load_high_level_globals_00a335d0(object);                       // state 1
    return object;
}

// ---------------------------------------------------------------------------
// 00A33020 Unknown_VtD232C4_Destruct_00a33020 (51 insns)
// ABI: __thiscall(ECX = object), plain RET. Handler 00CB2DC8; state 0 throughout.
// Vtable 00D232C4; the eight globals 00F8AB60, 00F8AB0C, 00F8AB18, ..., 00F8AB54 are
// cleared when non-zero (compare-then-store, kept); the records at +FA8h die through
// 00A32FF0; the singleton pointer [00F8AAFC] is cleared; the base vtable 00CE3818 is
// stored last (the base destructor is inlined and otherwise empty).
// ---------------------------------------------------------------------------
void vt_d232c4_destruct_00a33020(void* object, CgRebuildCHost& host) {
    field<std::uint32_t>(object, 0) = 0x00D232C4u;
    for (std::uint32_t address : {0x00F8AB60u, 0x00F8AB0Cu, 0x00F8AB18u, 0x00F8AB24u,
                                  0x00F8AB30u, 0x00F8AB3Cu, 0x00F8AB48u, 0x00F8AB54u}) {
        std::uint32_t& global = host.global_dword(address);
        if (global != 0) global = 0;
    }
    host.eh_vector_destruct_00bf7c6e(at(object, 0xFA8), 0x10, 3, 0x00A32FF0u);
    host.global_dword(0x00F8AAFCu) = 0;
    field<std::uint32_t>(object, 0) = 0x00CE3818u;
}

// ---------------------------------------------------------------------------
// 007B4030 BSP_BotTask_DestructBase_007b4030 (33 insns)
// ABI: __thiscall(ECX = task), plain RET. Handler 00C8C095; states 1, 0, -1.
// Vtable 00D05708; 007B3F70 on +38Ch; sub-object vtable 00D05704 at +30Ch; when +310h
// is set, its vtable[2]() (CALL [vtbl+8], no stack arguments); three C8h-byte plan
// elements at +04h die through 007B3BA0. Existing caller models: cc13_seg47/seg67a/b.
// ---------------------------------------------------------------------------
void bot_task_destruct_base_007b4030(void* task, CgRebuildCHost& host) {
    field<std::uint32_t>(task, 0) = 0x00D05708u;                            // state 1
    host.bot_observer_holder_destruct_full_007b3f70(at(task, 0x38C));
    field<std::uint32_t>(task, 0x30C) = 0x00D05704u;                        // state 0
    if (void* member = field<void*>(task, 0x310); member != nullptr) host.virtual_call_0(member, 8);
    host.eh_vector_destruct_00bf7c6e(at(task, 0x4), 0xC8, 3, 0x007B3BA0u);  // state -1
}

// ---------------------------------------------------------------------------
// 00808230 Unknown_Dtor_00808230 (31 insns)
// ABI: __thiscall(ECX = object), plain RET; no vtable. Handler 00C906C3; states 0, -1.
// The range list at +54h is resized to 0 (00807E60) and its data freed; then seven
// 0Ch-byte entries at +00h die through 00808080 BSP_SensorEntryList_Destroy.
// The element destructor of 00808D20's array.
// ---------------------------------------------------------------------------
void unknown_dtor_00808230(void* object, CgRebuildCHost& host) {
    host.sensor_gui_range_list_resize_00807e60(at(object, 0x54), 0);
    host.free_00bf6989(field<void*>(object, 0x54));
    host.eh_vector_destruct_00bf7c6e(object, 0xC, 7, 0x00808080u);
}

// ---------------------------------------------------------------------------
// 004CCB70 Unknown_Dtor_004ccb70 (27 insns)
// ABI: __thiscall(ECX = object), plain RET; no vtable. Handler 00C65898; states 0, -1.
// 12h 18h-byte records at +3Ch (004C87F0), then three 14h-byte records at +00h
// (004C7FA0). Called by 004D74E0 on its +D5Ch member.
// ---------------------------------------------------------------------------
void unknown_dtor_004ccb70(void* object, CgRebuildCHost& host) {
    host.eh_vector_destruct_00bf7c6e(at(object, 0x3C), 0x18, 0x12, 0x004C87F0u);
    host.eh_vector_destruct_00bf7c6e(object, 0x14, 3, 0x004C7FA0u);
}

// ---------------------------------------------------------------------------
// 00808D20 Unknown_Dtor_00808d20 (24 insns)
// ABI: __thiscall(ECX = object), plain RET. Handler 00C907D8; states 0, -1.
// Seven 60h-byte entries at +08h die through 00808230 (rebuilt above, passed by its
// native address as the iterator callback); then 00BD30F0 BSP_RefCounted_DestroyBase.
// ---------------------------------------------------------------------------
void unknown_dtor_00808d20(void* object, CgRebuildCHost& host) {
    host.eh_vector_destruct_00bf7c6e(at(object, 0x8), 0x60, 7, 0x00808230u);
    host.ref_counted_destroy_base_00bd30f0(object);
}

// ---------------------------------------------------------------------------
// 00B04B10 Unknown_OwnedArray_Release_00b04b10 (21 insns)
// ABI: __thiscall(ECX = holder), plain RET; no EH frame. The holder {+0 array, +4 ?}
// owns a new[] array of 8-byte rows with the count cookie at array-4: when set, the
// rows die through 00B04910 BSP_NativeParticleEmitterRow_Destroy and the block
// (array - 4) is freed (00BF6989). +4 and +0 are cleared either way.
// ---------------------------------------------------------------------------
void owned_array_release_00b04b10(void* holder, CgRebuildCHost& host) {
    std::uint8_t* array = field<std::uint8_t*>(holder, 0);
    if (array != nullptr) {
        host.eh_vector_destruct_00bf7c6e(array, 8, read<std::uint32_t>(array - 4, 0), 0x00B04910u);
        host.free_00bf6989(array - 4);
    }
    field<std::uint32_t>(holder, 4) = 0;
    field<std::uint32_t>(holder, 0) = 0;
}

}  // namespace bsp::cg_rebuild_c_cc13
