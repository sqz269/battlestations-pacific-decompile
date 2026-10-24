// Packet cc13-pool18 (see include/bsp/pool_pool18_cc13.hpp). Build-only.
#include "bsp/pool_pool18_cc13.hpp"

#include <cstring>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error cc13-pool18 reconstruction reads Win32 image storage.
#endif

namespace bsp::pool_pool18_cc13 {

namespace {

template <class T>
T& at(void* object, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}
template <class T>
const T& at(const void* object, std::size_t off) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(object) + off);
}

void release_native_string(NativeStringImage& string, Pool18Host& host) {
    if (string.data != nullptr) {
        void* pool = host.sized_pool_get_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, string.data, string.length + 1, 1);
    }
}

// The inlined reference drop: count at +4h, vtable +0h on zero.
void drop_reference(void* object, Pool18Host& host) {
    auto* count = reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(object) + 4);
    if (host.interlocked_decrement_00ce2220(count) == 0) host.refcounted_vtable0_release(object);
}

// The inlined NativeString copy constructor: destination = {0, 0}; unless it
// is the source, resize to the source length and, when the source is
// non-empty, copy destination.length bytes (no terminator byte).
void copy_construct_native_string(NativeStringImage& destination, const NativeStringImage& source,
                                  Pool18Host& host) {
    destination.length = 0;
    destination.data = nullptr;
    if (&destination == &source) return;
    host.native_string_resize_0041dd40(&destination, source.length, 1);
    if (source.length != 0) host.crt_memcpy_00bf7680(destination.data, source.data, destination.length);
}

// An x87 FLD m32 / FSTP m32 pair: bit-exact except that a signalling NaN is
// quieted (FLD raises the masked invalid exception and loads the quiet NaN).
void x87_copy_float(void* destination, const void* source) {
    std::uint32_t bits;
    std::memcpy(&bits, source, 4);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0) bits |= 0x00400000u;
    std::memcpy(destination, &bits, 4);
}

// The inlined sound-slot table loop of the screen constructors: for each entry
// of the .data table [first, end) (signed bound), slot i takes the sample-cache
// reference for the path, or is dropped and cleared when the entry is null.
void load_sound_slots(void* slots, std::uint32_t first, std::uint32_t end, Pool18Host& host) {
    for (std::uint32_t entry = first; static_cast<std::int32_t>(entry) < static_cast<std::int32_t>(end); entry += 4) {
        void*& slot = at<void*>(slots, entry - first);
        const char* path = host.image_cstring_table_entry(entry);
        if (path == nullptr) {
            if (slot != nullptr) {
                drop_reference(slot, host);
                slot = nullptr;
            }
            slot = nullptr;
            continue;
        }
        NativeStringImage name{0, nullptr};
        host.native_string_resize_0041dd40(&name, static_cast<std::uint32_t>(std::strlen(path)), 1);
        if (name.data != nullptr) host.crt_memcpy_00bf7680(name.data, path, name.length + 1);
        void* acquired = nullptr;  // uninitialised in the image; 00A83FD0 writes it
        void** const got = host.sound_sample_cache_acquire_00a83fd0(host.global_ptr(0x00F8BBE8u), &acquired, &name);
        void* const incoming = *got;
        void* const previous = slot;
        if (previous != incoming) {
            slot = incoming;
            if (incoming != nullptr) {
                host.interlocked_increment_00ce221c(
                    reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(incoming) + 4));
            }
            if (previous != nullptr) drop_reference(previous, host);
        }
        if (acquired != nullptr) {
            drop_reference(acquired, host);
            acquired = nullptr;
        }
        release_native_string(name, host);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 0054D550 (90 insns), BSP_HudHintsScreen_Destruct. __thiscall(ECX = screen),
// RET, no delete flag (0054D840 is the scalar deleting wrapper). Evidence:
// assembly 0054D550..0054D653.
// vtable 00CEE290; EH state 2; the GUI page +1Ch removed; then for i = 0, 1:
// when slot +14h+4i is set, its vtable +8h is called with 0 (the image tests
// it as NEG/SBB/TEST 00E19311, i.e. non-null), the slot's reference dropped
// and the slot zeroed (twice, as in the image); then slot +0Ch+4i dropped and
// zeroed (twice). EH state 1: the destructor iterator over +14h (4 x 2, dtor
// 00524180); state 0: over +0Ch (4 x 2, dtor 004C3810); state -1: the base.
// Uncertain: what vtable +8h does on the +14h objects (their constructor is
// 0054D440, their element destructor 00524180).
void hud_hints_screen_destruct_0054d550(void* screen, Pool18Host& host) {
    at<std::uint32_t>(screen, 0) = 0x00CEE290u;                      // 0054D572
    // EH state 2
    host.gui_manager_remove_and_destroy_page_00aa31f0(host.gui_manager_get_or_create_004c12b0(),
                                                      at<void*>(screen, 0x1C));  // 0054D58D
    for (std::size_t i = 0; i < 2; ++i) {                            // 0054D5A0..0054D603
        void*& channel = at<void*>(screen, 0x14 + i * 4);
        if (channel != nullptr) {
            host.object_vtable8_call(channel, 0);                     // 0054D5B6
            if (void* object = channel) {
                drop_reference(object, host);
                channel = nullptr;                                    // 0054D5D4
            }
            channel = nullptr;                                        // 0054D5D6
        }
        void*& sample = at<void*>(screen, 0x0C + i * 4);
        if (void* object = sample) {                                  // 0054D5D8
            drop_reference(object, host);
            sample = nullptr;                                         // 0054D5F5
        }
        sample = nullptr;                                             // 0054D5F8
    }
    // EH state 1
    host.eh_vector_destructor_iterator_00bf7c6e(static_cast<std::uint8_t*>(screen) + 0x14, 4, 2, 0x00524180u);
    // EH state 0
    host.eh_vector_destructor_iterator_00bf7c6e(static_cast<std::uint8_t*>(screen) + 0x0C, 4, 2, 0x004C3810u);
    // EH state -1
    host.front_end_screen_destruct_004f71a0(screen);                 // 0054D63C
}

// ---------------------------------------------------------------------------
// 0054D680 (152 insns), BSP_HudHintsScreen_Construct. Only caller 0068CC70
// BSP_InGameInterface_Init. __thiscall(ECX = screen), RET; EAX = screen.
// Evidence: assembly 0054D680..0054D835. The base; EH state 0; vtable
// 00CEE290; the constructor iterator over the sample slots +0Ch (4 x 2, ctor
// 004BA0D0, dtor 004C3810); state 1; over +14h (4 x 2, ctor 0054D440, dtor
// 00524180); state 2; then the sound table 00E08698..00E086A0 into +0Ch/+10h
// (the same inlined loop as 00525A90; states 3/4 around each acquire).
// +1Ch (the page) is not written here.
void* hud_hints_screen_construct_0054d680(void* screen, Pool18Host& host) {
    host.front_end_screen_construct_004f7180(screen);                 // 0054D6A8
    // EH state 0
    at<std::uint32_t>(screen, 0) = 0x00CEE290u;                      // 0054D6C5
    host.eh_vector_constructor_iterator_00bf7cd1(static_cast<std::uint8_t*>(screen) + 0x0C, 4, 2, 0x004BA0D0u,
                                                 0x004C3810u);       // 0054D6CC
    // EH state 1
    host.eh_vector_constructor_iterator_00bf7cd1(static_cast<std::uint8_t*>(screen) + 0x14, 4, 2, 0x0054D440u,
                                                 0x00524180u);       // 0054D6E8
    // EH state 2
    load_sound_slots(static_cast<std::uint8_t*>(screen) + 0x0C, 0x00E08698u, 0x00E086A0u, host);  // 0054D700..0054D81B
    return screen;
}

// ---------------------------------------------------------------------------
// 005902E0 (129 insns), BSP_MainMenuScreen_Construct. Only caller 00686380
// BSP_MainMenu_Init. __thiscall(ECX = screen), RET; EAX = screen. Evidence:
// assembly 005902E0..00590564. Mirror of the destructor 00590600
// (src/cg_rebuild_b_cc13.cpp):
// base; state 0; +8h = 00CEB0FC; state 1; 00A97260 on +0Ch; +40h = 00CEB110;
// final vtables 00CEFC5C +0, 00CEFC48 +8, 00CEFC24 +0Ch, 00CEFC04 +40h; ten
// 12-byte vectors +74h..+10Ch zeroed; state 0Ch: constructor iterator over
// five 10h-byte vectors at +134h (ctor 004214E0, dtor 0041F5D0); state 0Dh:
// 00683610 on +1D4h; lists +280h, +298h, +2A4h each get a sentinel from
// 00582280 (+4h) and size 0 (states 0Eh, 0Fh between them); vectors +2D0h,
// +2E0h zeroed; state 12h: 00683610 on +354h; +3C4h = 00CECA24 with +3D0h..
// +3D8h and +3E4h zeroed; +414h = 00CEFC00 with +428h..+430h zeroed;
// +480h..+488h and +4F4h..+4FCh zeroed, byte +500h = 0; the two std::strings
// at +504h/+520h set empty (size 0, capacity 0Fh, first byte 0); +570h, +574h
// zeroed; then the globals [00E194C4..00E194D0] = 0 and [00E08874] = 1.
void* main_menu_screen_construct_005902e0(void* screen, Pool18Host& host) {
    auto* const s = static_cast<std::uint8_t*>(screen);
    host.front_end_screen_construct_004f7180(screen);                 // 005902FF
    // EH state 0
    at<std::uint32_t>(s, 0x08) = 0x00CEB0FCu;                        // 0059030A
    // EH state 1
    host.text_listener_base_construct_00a97260(s + 0x0C);            // 0059031B
    at<std::uint32_t>(s, 0x40) = 0x00CEB110u;                        // 00590320
    at<std::uint32_t>(s, 0x00) = 0x00CEFC5Cu;
    at<std::uint32_t>(s, 0x08) = 0x00CEFC48u;
    at<std::uint32_t>(s, 0x0C) = 0x00CEFC24u;
    at<std::uint32_t>(s, 0x40) = 0x00CEFC04u;                        // 0059033A
    for (std::size_t off = 0x74; off <= 0x104; off += 0x10) {        // 00590341..005903E6
        at<std::uint32_t>(s, off) = 0;
        at<std::uint32_t>(s, off + 4) = 0;
        at<std::uint32_t>(s, off + 8) = 0;
    }
    // EH state 0Ch
    host.eh_vector_constructor_iterator_00bf7cd1(s + 0x134, 0x10, 5, 0x004214E0u, 0x0041F5D0u);  // 00590406
    // EH state 0Dh
    host.gui_scroller_construct_00683610(s + 0x1D4);                 // 00590416
    for (std::size_t list : {0x280u, 0x298u, 0x2A4u}) {              // 00590423, 0059043B, 00590453
        at<void*>(s, list + 4) = host.stl_buy_head_node(0x00582280u, s + list);
        at<std::uint32_t>(s, list + 8) = 0;
        // EH states 0Eh, 0Fh, ... after each list
    }
    for (std::size_t off : {0x2D0u, 0x2D4u, 0x2D8u, 0x2E0u, 0x2E4u, 0x2E8u}) at<std::uint32_t>(s, off) = 0;
    // EH state 12h
    host.gui_scroller_construct_00683610(s + 0x354);                 // 0059048D
    at<std::uint32_t>(s, 0x3C4) = 0x00CECA24u;                       // 00590492
    for (std::size_t off : {0x3D0u, 0x3D4u, 0x3D8u, 0x3E4u}) at<std::uint32_t>(s, off) = 0;
    at<std::uint32_t>(s, 0x414) = 0x00CEFC00u;                       // 005904B4
    for (std::size_t off : {0x428u, 0x42Cu, 0x430u, 0x480u, 0x484u, 0x488u, 0x4F4u, 0x4F8u, 0x4FCu}) {
        at<std::uint32_t>(s, off) = 0;
    }
    at<std::uint8_t>(s, 0x500) = 0;                                  // 005904F8
    at<std::uint32_t>(s, 0x518) = 0;                                 // std::string +504h: size,
    at<std::uint32_t>(s, 0x51C) = 0x0F;                              // capacity,
    at<std::uint8_t>(s, 0x508) = 0;                                  // first byte
    at<std::uint32_t>(s, 0x538) = 0x0F;                              // std::string +520h
    at<std::uint32_t>(s, 0x534) = 0;
    at<std::uint8_t>(s, 0x524) = 0;
    at<std::uint32_t>(s, 0x570) = 0;                                 // 00590527
    at<std::uint32_t>(s, 0x574) = 0;
    for (std::uint32_t g : {0x00E194C4u, 0x00E194C8u, 0x00E194CCu, 0x00E194D0u}) host.global_store_u32(g, 0);
    host.global_store_u32(0x00E08874u, 1);                           // 0059054F
    return screen;
}

// ---------------------------------------------------------------------------
// 005C81F0 (459 insns), BSP_MissionRecord_CopyConstruct_Provisional.
// __thiscall(ECX = record; const record* source), RET 4; EAX = record.
// Callers: the STL record constructors 005C8970, 005C8E30 and others.
// Evidence: assembly 005C81F0..005C8877. Member order:
//   NativeStrings +0h..+58h (12, inlined copy; EH states 0..0Ah between them);
//   dwords +60h..+68h; floats +6Ch..+74h (FLD/FSTP); state 0Bh: 00557BC0 on
//   +78h; state 0Ch: 004F6210 on +88h; state 0Dh: NativeString +98h; the
//   picture reference +A0h (zeroed, then the source's taken with an
//   InterlockedIncrement); dwords +A4h..+B4h; state 0Fh: the EH copy
//   iterator over two 154h-byte side blocks at +B8h (copy 005C51F0, dtor
//   005C4D40); byte +360h; floats +364h..+420h (FLD/FSTP, 48 of them); byte
//   +424h; 005C69A0 copies the int tree at +428h.
// The FLD/FSTP copies quiet a signalling NaN (x87_copy_float). Host overlap:
// src/mission_picture_owner.cpp projects only the +A0h..+B0h picture fields
// (its fragment rows stay); this is the whole body.
void* mission_record_copy_construct_005c81f0(void* record, const void* source, Pool18Host& host) {
    auto* const d = static_cast<std::uint8_t*>(record);
    const auto* const src = static_cast<const std::uint8_t*>(source);
    for (std::size_t off = 0; off <= 0x58; off += 8) {               // 005C8212..005C8512
        copy_construct_native_string(at<NativeStringImage>(d, off), at<NativeStringImage>(src, off), host);
        // EH state off / 8
    }
    for (std::size_t off : {0x60u, 0x64u, 0x68u}) at<std::uint32_t>(d, off) = at<std::uint32_t>(src, off);
    for (std::size_t off : {0x6Cu, 0x70u, 0x74u}) x87_copy_float(d + off, src + off);  // 005C8527..005C8542
    // EH state 0Bh
    host.stl_copy_construct(0x00557BC0u, d + 0x78, src + 0x78);      // 005C8545
    // EH state 0Ch
    host.stl_copy_construct(0x004F6210u, d + 0x88, src + 0x88);      // 005C855C
    // EH state 0Dh
    copy_construct_native_string(at<NativeStringImage>(d, 0x98), at<NativeStringImage>(src, 0x98), host);
    at<std::uint32_t>(d, 0xA0) = 0;                                  // 005C85A9
    if (void* picture = at<void*>(src, 0xA0)) {
        at<void*>(d, 0xA0) = picture;
        host.interlocked_increment_00ce221c(reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(picture) + 4));
    }
    for (std::size_t off : {0xA4u, 0xA8u, 0xACu, 0xB0u, 0xB4u}) at<std::uint32_t>(d, off) = at<std::uint32_t>(src, off);
    // EH state 0Fh
    host.eh_vector_copy_constructor_iterator_00bf9a96(d + 0xB8, src + 0xB8, 0x154, 2, 0x005C51F0u,
                                                      0x005C4D40u);  // 005C862D
    at<std::uint8_t>(d, 0x360) = at<std::uint8_t>(src, 0x360);       // 005C8632
    for (std::size_t off = 0x364; off <= 0x420; off += 4) x87_copy_float(d + off, src + off);
    at<std::uint8_t>(d, 0x424) = at<std::uint8_t>(src, 0x424);       // 005C8855
    host.stl_copy_construct(0x005C69A0u, d + 0x428, src + 0x428);    // 005C885E
    return record;
}

// ---------------------------------------------------------------------------
// 005C89B0 (81 insns), BSP_MissionRecord_Construct_Provisional. Callers
// 005C9F70 BSP_MissionTree_ReadGroupFromLua and 005CAAF0
// BSP_MissionTreeScreen_LoadTables. __thiscall(ECX = record), RET; EAX =
// record. Evidence: assembly 005C89B0..005C8ADC.
// Zeroes +0h..+5Ch (the twelve NativeStrings; EH state 0 after the first),
// +7Ch..+84h, +8Ch..+94h, +98h/+9Ch, +A0h; +A4h/+A8h = 0.0f and +ACh/+B0h =
// [00D7A24C] (1.0f) through SSE stores; state 0Fh: the constructor iterator
// over the two 154h-byte side blocks at +B8h (ctor 005C4BE0, dtor 005C4D40);
// state 10h: the int tree at +428h gets its head from 0056A710 (+4h), the
// head's isnil byte (+1Dh) = 1, its links = itself, size +8h = 0. +60h..+78h,
// +B4h and +360h..+424h are not written. Host overlap: the
// MissionRecordPicture_Default fragment in src/mission_picture_owner.cpp
// models +A0h..+B0h only.
void* mission_record_construct_005c89b0(void* record, Pool18Host& host) {
    auto* const r = static_cast<std::uint8_t*>(record);
    for (std::size_t off = 0; off <= 0x5C; off += 4) at<std::uint32_t>(r, off) = 0;  // 005C89D1..005C8A19
    for (std::size_t off : {0x7Cu, 0x80u, 0x84u, 0x8Cu, 0x90u, 0x94u, 0x98u, 0x9Cu, 0xA0u}) at<std::uint32_t>(r, off) = 0;
    at<float>(r, 0xA4) = 0.0f;                                       // 005C8A69 (XORPS)
    at<float>(r, 0xA8) = 0.0f;
    at<std::uint32_t>(r, 0xAC) = 0x3F800000u;                        // [00D7A24C] = 1.0f
    at<std::uint32_t>(r, 0xB0) = 0x3F800000u;
    // EH state 0Fh
    host.eh_vector_constructor_iterator_00bf7cd1(r + 0xB8, 0x154, 2, 0x005C4BE0u, 0x005C4D40u);  // 005C8A97
    // EH state 10h
    void* const tree = r + 0x428;
    void* const head = host.stl_buy_head_node(0x0056A710u, tree);    // 005C8AA9
    at<void*>(tree, 4) = head;
    at<std::uint8_t>(head, 0x1D) = 1;
    at<void*>(at<void*>(tree, 4), 4) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 0) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 8) = at<void*>(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;                                  // 005C8ACA
    return record;
}

}  // namespace bsp::pool_pool18_cc13
