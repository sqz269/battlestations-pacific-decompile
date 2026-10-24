// Packet cc13-cgbuildB (see include/bsp/cg_rebuild_b_cc13.hpp). Build-only
// reconstructions; nothing here is wired into a live host path.
#include "bsp/cg_rebuild_b_cc13.hpp"

#include <cstring>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error cc13-cgbuildB reconstruction reads Win32 image storage.
#endif

namespace bsp::cg_rebuild_b_cc13 {

namespace {

// The inlined NativeString release: data non-null -> the pool takes back
// length + 1 bytes with flag 1 (00419CC0 then 00BD1510).
void release_native_string(NativeStringImage& string, CgRebuildBHost& host) {
    if (string.data != nullptr) {
        void* pool = host.sized_pool_get_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, string.data, string.length + 1, 1);
    }
}

// The inlined reference drop: count at +4h, vtable +0h on zero.
void drop_reference(void* object, CgRebuildBHost& host) {
    auto* count = reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(object) + 4);
    if (host.interlocked_decrement_00ce2220(count) == 0) host.refcounted_vtable0_release(object);
}

// The inlined `NativeString tmp = literal` the constructors build: resize to
// the length, then copy length + 1 bytes when storage was granted.
void assign_native_string(NativeStringImage& string, const char* text, std::uint32_t length,
                          CgRebuildBHost& host) {
    string.length = 0;
    string.data = nullptr;
    host.native_string_resize_0041dd40(&string, length, 1);
    if (string.data != nullptr) host.crt_memcpy_00bf7680(string.data, text, string.length + 1);
}

// A 12-byte {first, last, end} vector's inlined tidy: free(first) when set,
// then zero all three.
void tidy_vector12(const Raw& s, std::size_t off, CgRebuildBHost& host) {
    if (void* first = s.ptr(off)) host.free_00bf65ac(first);
    s.at<std::uint32_t>(off) = 0;
    s.at<std::uint32_t>(off + 4) = 0;
    s.at<std::uint32_t>(off + 8) = 0;
}

// An STL tree {alloc, head, size} at `off`: erase [begin, head) with the
// instantiation `callee`, free the head, zero head and size.
void tidy_tree(const Raw& s, std::size_t off, std::uint32_t callee, CgRebuildBHost& host) {
    void* tree = s.p + off;
    void* head = s.ptr(off + 4);
    CheckedIterImage out{};
    host.stl_tree_erase_range(callee, tree, &out, CheckedIterImage{tree, *static_cast<void**>(head)},
                              CheckedIterImage{tree, head});
    host.free_00bf65ac(s.ptr(off + 4));
    s.at<std::uint32_t>(off + 4) = 0;
    s.at<std::uint32_t>(off + 8) = 0;
}

// An STL list {alloc, head, size} at `off`, cleared inline: unlink the head,
// zero the size, free every node, free the head, zero the head pointer.
void tidy_list(const Raw& s, std::size_t off, CgRebuildBHost& host) {
    auto** head = reinterpret_cast<void**>(s.ptr(off + 4));
    void* node = head[0];
    head[0] = head;
    head = reinterpret_cast<void**>(s.ptr(off + 4));
    head[1] = head;
    s.at<std::uint32_t>(off + 8) = 0;
    if (node != s.ptr(off + 4)) {
        void* next;
        do {
            next = *static_cast<void**>(node);
            host.free_00bf65ac(node);
            node = next;
        } while (next != s.ptr(off + 4));
    }
    host.free_00bf65ac(s.ptr(off + 4));
    s.at<std::uint32_t>(off + 4) = 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00524BA0 (104 insns), was CG_vector_deleting_dtor_00524ba0; ledger name
// Unknown_VtCECCF8_Destruct. __thiscall(ECX = screen), RET, no delete flag
// (00524F90 is the scalar deleting wrapper). Screen 29h, registry slot 41,
// F0h bytes, constructor 00525A90 (src/ingame_interface.cpp row 0xCC).
// Evidence: assembly 00524BA0..00524D04 (exports/bsp/functions/00524ba0).
// Order: the three vtables (00CECCF8 at +0, 00CECCEC at +8, 00CECCD4 at +18h);
// node list +3Ch cleared; +48h = 0; the 18h-byte block at +BCh (its
// NativeString at +10h returned to the pool, the block freed); the two
// reference slots +DCh/+E0h dropped and zeroed (the image stores 0 twice);
// the EH destructor iterator over the same two slots (dtor 004C3810, a no-op on
// null slots); the 18h-stride element vector at +C8h..+D0h (vtable +0 with
// flag 0 on each, free(first), zero three); node list +3Ch again; callback
// owner +18h; observer endpoint +8h; the front-end screen base.
// EH states: 5, 4, 3, 1, 0, -1. The block at +BCh is freed even when its
// string is empty. Uncertain: the class identity of +C0h..+D4h (00525A10's
// container) and the element type at +C8h; names are hypotheses.
void unit_pick_screen_destruct_00524ba0(void* screen, CgRebuildBHost& host) {
    const Raw s = raw(screen);
    s.at<std::uint32_t>(0x00) = 0x00CECCF8u;                          // 00524BC2
    s.at<std::uint32_t>(0x08) = 0x00CECCECu;                          // 00524BC8
    s.at<std::uint32_t>(0x18) = 0x00CECCD4u;                          // 00524BCF
    // EH state 5
    host.node_list_clear_005234f0(s.p + 0x3C);                        // 00524BE1
    void* block = s.ptr(0xBC);                                        // 00524BE6
    s.at<std::uint32_t>(0x48) = 0;                                    // 00524BEE
    if (block != nullptr) {
        release_native_string(raw(block).at<NativeStringImage>(0x10), host);  // 00524BF7..00524C0F
        host.free_00bf65ac(block);                                    // 00524C15
    }
    for (std::size_t i = 0; i < 2; ++i) {                             // 00524C1D..00524C57
        void*& slot = s.ptr(0xDC + i * 4);
        if (slot != nullptr) {
            drop_reference(slot, host);
            slot = nullptr;                                           // 00524C45
        }
        slot = nullptr;                                               // 00524C4B
    }
    // EH state 4
    host.eh_vector_destructor_iterator_00bf7c6e(s.p + 0xDC, 4, 2, 0x004C3810u);  // 00524C6E
    // EH state 3
    if (s.ptr(0xC8) != nullptr) {                                     // 00524C73
        auto* element = static_cast<std::uint8_t*>(s.ptr(0xC8));
        auto* const last = static_cast<std::uint8_t*>(s.ptr(0xCC));
        for (; element != last; element += 0x18) host.object_vtable0_scalar_dtor(element, 0);  // 00524C90
        host.free_00bf65ac(s.ptr(0xC8));                              // 00524CA0
    }
    s.at<std::uint32_t>(0xC8) = 0;                                    // 00524CB2
    s.at<std::uint32_t>(0xCC) = 0;
    s.at<std::uint32_t>(0xD0) = 0;
    host.node_list_clear_005234f0(s.p + 0x3C);                        // 00524CC4
    // EH state 1
    host.callback_owner_destroy_00695870(s.p + 0x18);                 // 00524CD1
    // EH state 0
    host.observer_endpoint_destruct_00695760(s.p + 0x08);             // 00524CDE
    // EH state -1
    host.front_end_screen_destruct_004f71a0(screen);                  // 00524CED
}

// ---------------------------------------------------------------------------
// 00525A90 (242 insns), was CG_array_ctor_helper_00525a90; ledger name
// Unknown_VtCECCF8_Construct (PROVISIONAL). __thiscall(ECX = screen), RET;
// EAX = screen. Only caller 0068CC70 BSP_InGameInterface_Init (registry slot
// 29h, F0h bytes). Evidence: assembly 00525A90..00525D87.
// Order: front-end screen base; +8h gets 00CECCC8 then zeroes +Ch..+14h; +18h
// gets 00CE3CD4 then zeroes +1Ch..+24h and byte +28h (the inlined bases of the
// two subobjects); the final vtables 00CECCF8/00CECCEC/00CECCD4; +3Ch..+44h
// zeroed; operator new(18h) (on success +4h, byte +8h, +10h, +14h zeroed) into
// +BCh, null when new fails; 00525A10 on +C0h; the EH constructor iterator
// over two pointer slots at +DCh (ctor 004BA0D0, dtor 004C3810); then for
// each entry of the table 00E0865C (signed bound 00E08664): a null entry drops
// and clears the slot, otherwise a NativeString of the path is acquired from
// the sample cache [00F8BBE8], assigned to the slot when it differs (add-ref
// new, drop old), the temporary reference dropped, the string returned;
// last "InGameGUI" (00CE7854) -> +E4h by 00A7ACF0 and "Normal" (00CECD20) ->
// +E8h by 00A7B0A0 on the sound manager [00F8BBD8].
// EH states: 0 (after the base), 3, 4, 5, then 6/7 around each acquire, 8 and
// 9 around the two lookups. The out slot of 00A83FD0 is not initialised by the
// image before the call (the callee writes it); the C++ zeroes it first.
// Uncertain: the meaning of +3Ch..+48h and of the 18h-byte block at +BCh.
void* unit_pick_screen_construct_00525a90(void* screen, CgRebuildBHost& host) {
    const Raw s = raw(screen);
    host.front_end_screen_construct_004f7180(screen);                 // 00525AB8
    // EH state 0
    s.at<std::uint32_t>(0x08) = 0x00CECCC8u;                          // 00525ABF
    s.at<std::uint32_t>(0x0C) = 0;
    s.at<std::uint32_t>(0x10) = 0;
    s.at<std::uint32_t>(0x14) = 0;
    s.at<std::uint32_t>(0x18) = 0x00CE3CD4u;                          // 00525AD3
    s.at<std::uint32_t>(0x1C) = 0;
    s.at<std::uint32_t>(0x20) = 0;
    s.at<std::uint32_t>(0x24) = 0;
    s.at<std::uint8_t>(0x28) = 0;
    s.at<std::uint32_t>(0x00) = 0x00CECCF8u;                          // 00525AE6
    s.at<std::uint32_t>(0x08) = 0x00CECCECu;
    s.at<std::uint32_t>(0x18) = 0x00CECCD4u;
    s.at<std::uint32_t>(0x3C) = 0;                                    // 00525AFA
    s.at<std::uint32_t>(0x40) = 0;
    s.at<std::uint32_t>(0x44) = 0;
    // EH state 3
    void* block = host.operator_new_00bf681b(0x18);                   // 00525B0A
    if (block != nullptr) {
        const Raw b = raw(block);
        b.at<std::uint32_t>(0x04) = 0;
        b.at<std::uint8_t>(0x08) = 0;
        b.at<std::uint32_t>(0x10) = 0;
        b.at<std::uint32_t>(0x14) = 0;
    } else {
        block = nullptr;
    }
    s.ptr(0xBC) = block;                                              // 00525B2C
    host.container_init_00525a10(s.p + 0xC0);                         // 00525B32
    // EH state 4
    host.eh_vector_constructor_iterator_00bf7cd1(s.p + 0xDC, 4, 2, 0x004BA0D0u, 0x004C3810u);  // 00525B51
    // EH state 5
    for (std::uint32_t entry = 0x00E0865Cu; static_cast<std::int32_t>(entry) < 0x00E08664; entry += 4) {
        void*& slot = s.ptr(0xDC + (entry - 0x00E0865Cu));
        const char* path = host.image_cstring_table_entry(entry);     // 00525B66
        if (path == nullptr) {
            if (slot != nullptr) {                                    // 00525C53
                drop_reference(slot, host);
                slot = nullptr;                                       // 00525C70
            }
            slot = nullptr;                                           // 00525C73
            continue;
        }
        NativeStringImage name;
        assign_native_string(name, path, static_cast<std::uint32_t>(std::strlen(path)), host);  // 00525B72..00525BAE
        // EH state 6
        void* acquired = nullptr;
        void** const got = host.sound_sample_cache_acquire_00a83fd0(host.global_ptr(0x00F8BBE8u), &acquired,
                                                                    &name);  // 00525BC6
        void* const incoming = *got;
        void* const previous = slot;
        // EH state 7
        if (previous != incoming) {                                   // 00525BD0
            slot = incoming;
            if (incoming != nullptr) {
                host.interlocked_increment_00ce221c(
                    reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(incoming) + 4));
            }
            if (previous != nullptr) drop_reference(previous, host);
        }
        // EH state 6
        if (acquired != nullptr) {                                    // 00525C04
            drop_reference(acquired, host);
            acquired = nullptr;
        }
        // EH state 5
        release_native_string(name, host);                            // 00525C2D
    }
    NativeStringImage name;
    assign_native_string(name, "InGameGUI", 9, host);                 // 00525C8F (00CE7854)
    // EH state 8
    s.at<std::uint32_t>(0xE4) = host.sound_manager_find_class_00a7acf0(host.global_ptr(0x00F8BBD8u), &name);
    // EH state 5
    release_native_string(name, host);                                // 00525CD7
    assign_native_string(name, "Normal", 6, host);                    // 00525D01 (00CECD20)
    // EH state 9
    s.at<std::uint32_t>(0xE8) = host.sound_manager_find_type_00a7b0a0(host.global_ptr(0x00F8BBD8u), &name);
    // EH state 5
    release_native_string(name, host);                                // 00525D49
    return screen;
}

// ---------------------------------------------------------------------------
// 005B9D40 (99 insns), was CG_vector_deleting_dtor_005b9d40; ledger name
// Unknown_VtCF0ED8_Destruct. __thiscall(ECX = screen), RET, no delete flag
// (005BA480 is the scalar deleting wrapper). HUD narrative screen, registry
// slot 33h, manager +A4h, E8h bytes (include/bsp/interface_runtime_tail.hpp).
// Evidence: assembly 005B9D40..005B9E8C (005B9E50 MOV ECX,EBP read by
// `ghidra disasm`).
// Order: vtable 00CF0ED8; 005B7840 on the owned-object list +54h; 007026F0
// stops the voice slot +8h; three GUI pages removed in the order +20h, +28h,
// +24h; NativeStrings +E0h then +CCh returned; effect samples +A0h resized to
// 0 and their storage freed through 00BF6989; the vector +94h..+9Ch tidied;
// 005B7570 on the list +64h; 005B77F0 on +54h; the EH destructor iterator
// over the one 18h-byte voice slot at +8h (dtor 005B7FC0); the base.
// EH states: 7, 6, 5, 4, 0, -1.
void hud_narrative_screen_destruct_005b9d40(void* screen, CgRebuildBHost& host) {
    const Raw s = raw(screen);
    s.at<std::uint32_t>(0x00) = 0x00CF0ED8u;                          // 005B9D60
    // EH state 7
    host.owned_object_list_clear_005b7840(s.p + 0x54);                // 005B9D73
    host.voice_slot_stop_007026f0(s.p + 0x08);                        // 005B9D7D
    host.gui_manager_remove_and_destroy_page_00aa31f0(host.gui_manager_get_or_create_004c12b0(),
                                                      s.ptr(0x20));  // 005B9D8D
    host.gui_manager_remove_and_destroy_page_00aa31f0(host.gui_manager_get_or_create_004c12b0(),
                                                      s.ptr(0x28));  // 005B9D9D
    host.gui_manager_remove_and_destroy_page_00aa31f0(host.gui_manager_get_or_create_004c12b0(),
                                                      s.ptr(0x24));  // 005B9DAD
    // EH state 6
    release_native_string(s.at<NativeStringImage>(0xE0), host);       // 005B9DB2
    // EH state 5
    release_native_string(s.at<NativeStringImage>(0xCC), host);       // 005B9DDA
    // EH state 4
    host.effect_sound_samples_resize_005b8fa0(s.p + 0xA0, 0);         // 005B9E11
    host.free_00bf6989(s.ptr(0xA0));                                  // 005B9E19
    tidy_vector12(s, 0x94, host);                                     // 005B9E1E..005B9E45
    host.list_tidy_005b7570(s.p + 0x64);                              // 005B9E4B
    host.list_clear_005b77f0(s.p + 0x54);                             // 005B9E52
    // EH state 0
    host.eh_vector_destructor_iterator_00bf7c6e(s.p + 0x08, 0x18, 1, 0x005B7FC0u);  // 005B9E66
    // EH state -1
    host.front_end_screen_destruct_004f71a0(screen);                  // 005B9E75
}

// ---------------------------------------------------------------------------
// 005BA7B0 (328 insns), was CG_array_ctor_helper_005ba7b0; ledger name
// Unknown_VtCF0ED8_Construct (PROVISIONAL). __thiscall(ECX = screen), RET;
// EAX = screen. Only caller 0068CC70 BSP_InGameInterface_Init (registry slot
// 33h). Evidence: assembly 005BA7B0.. and the five string immediates.
// Order: the base; vtable 00CF0ED8; the EH constructor iterator over one
// 18h-byte voice slot at +8h (ctor 005B7FA0, dtor 005B7FC0); +54h..+5Ch
// zeroed; 005B7550 buys the head of the list at +64h (stored at +68h, size
// +6Ch = 0); +94h..+9Ch, +A0h..+A8h, +CCh, +D0h, +E0h, +E4h zeroed; then five
// warning radio banks, each: NativeString of the path, acquired from the
// sample cache [00F8BBE8], appended to the effect samples at +A0h (005B9AF0
// receives the out slot 00A83FD0 returned), the temporary dropped, the
// string returned. Order fighter, sub, land, ship, enemy.
// EH states: 0, 2, 7, then 8/9 (fighter), 10/11, 12/13, 14/15, 16/17. The out
// slot is uninitialised in the image before 00A83FD0; the C++ zeroes it.
void* hud_narrative_screen_construct_005ba7b0(void* screen, CgRebuildBHost& host) {
    const Raw s = raw(screen);
    host.front_end_screen_construct_004f7180(screen);                 // 005BA7D2
    // EH state 0
    s.at<std::uint32_t>(0x00) = 0x00CF0ED8u;                          // 005BA7EF
    host.eh_vector_constructor_iterator_00bf7cd1(s.p + 0x08, 0x18, 1, 0x005B7FA0u, 0x005B7FC0u);  // 005BA7F5
    s.at<std::uint32_t>(0x54) = 0;                                    // 005BA7FA
    s.at<std::uint32_t>(0x58) = 0;
    s.at<std::uint32_t>(0x5C) = 0;
    // EH state 2
    s.ptr(0x68) = host.list_buy_head_node_005b7550(s.p + 0x64);      // 005BA80D
    s.at<std::uint32_t>(0x6C) = 0;
    for (std::size_t off : {0x94u, 0x98u, 0x9Cu, 0xA0u, 0xA4u, 0xA8u, 0xCCu, 0xD0u, 0xE0u, 0xE4u}) {
        s.at<std::uint32_t>(off) = 0;                                 // 005BA818..005BA84B
    }
    // EH state 7
    struct Bank {
        const char* path;
        std::uint32_t length;  // the Resize immediate
    };
    static constexpr char kFighter[] = "sound/messages/warning/warning_fighter_radio.fsb";  // 00CF0FC0
    static constexpr char kSub[] = "sound/messages/warning/warning_sub_radio.fsb";
    static constexpr char kLand[] = "sound/messages/warning/warning_land_radio.fsb";
    static constexpr char kShip[] = "sound/messages/warning/warning_ship_radio.fsb";
    static constexpr char kEnemy[] = "sound/messages/warning/warning_enemy_radio.fsb";
    static_assert(sizeof(kFighter) - 1 == 0x30 && sizeof(kSub) - 1 == 0x2C && sizeof(kLand) - 1 == 0x2D &&
                  sizeof(kShip) - 1 == 0x2D && sizeof(kEnemy) - 1 == 0x2E, "Resize immediates");
    static constexpr Bank kBanks[] = {
        {kFighter, 0x30}, {kSub, 0x2C}, {kLand, 0x2D}, {kShip, 0x2D}, {kEnemy, 0x2E},
    };
    for (const Bank& bank : kBanks) {
        NativeStringImage name;
        assign_native_string(name, bank.path, bank.length, host);     // 005BA851..005BA886
        // EH state 8 + 2i
        void* acquired = nullptr;
        void** const got = host.sound_sample_cache_acquire_00a83fd0(host.global_ptr(0x00F8BBE8u), &acquired,
                                                                    &name);  // 005BA89F
        // EH state 9 + 2i
        host.effect_sound_samples_append_005b9af0(s.p + 0xA0, got);   // 005BA8AC
        // EH state 8 + 2i
        if (acquired != nullptr) {                                    // 005BA8B1
            drop_reference(acquired, host);
            acquired = nullptr;
        }
        // EH state 7
        release_native_string(name, host);
    }
    return screen;
}

// ---------------------------------------------------------------------------
// 00590600 (260 insns), was CG_vector_deleting_dtor_00590600; ledger name
// BSP_MainMenuScreen_Destruct. __thiscall(ECX = screen), RET, no delete flag
// (00590D60 BSP_MainMenuScreen_ScalarDeletingDtor wraps it; that wrapper is
// built in src/frontend_seg20_cc13.cpp and reaches this body only through
// Seg20Host::main_menu_screen_destruct_00590600). Evidence: assembly
// 00590600..005909F0 (exports/bsp/functions/00590600/assembly.txt).
// The binary's order, exactly (cc16 asked that it be recorded):
//   1. vtables 00CEFC5C +0, 00CEFC48 +8, 00CEFC24 +Ch, 00CEFC04 +40h;
//   2. 00543E00 on +3C4h; 00544A70 on +414h;
//   3. GUI pages removed: +470h, +2F0h, +244h, +248h;
//   4. 005830A0 (mission preview) on the screen;
//   5. reference slots +574h then +570h dropped;
//   6. std::string tidy (MSVC SSO: free when capacity >= 10h, capacity = 0Fh,
//      size 0, first byte 0) at +520h then +504h;
//   7. the NativeString range vector +4F0h (00432050 over [first, last), free
//      first, zero three);
//   8. 00427110 resizes +480h to 0, then 00BF6989 frees its storage;
//   9. 00590250 on +414h; 0051F910 on +3C4h;
//  10. vectors +2E0h, +2D0h tidied; 005822A0 on lists +2A4h, +298h, +280h;
//  11. the EH destructor iterator over five 10h-byte vectors at +134h (dtor
//      0041F5D0);
//  12. vectors +104h, +F4h, +E4h, +D4h, +C4h, +B4h, +A4h, +94h, +84h, +74h;
//  13. 00A97140 on +Ch; vtable 00CEB0FC stored at +8h; the front-end base.
// EH states: 1Ah, 19h, 18h, 15h, 14h, 13h, 12h, 0Ch, 1, -1.
void main_menu_screen_destruct_00590600(void* screen, CgRebuildBHost& host) {
    const Raw s = raw(screen);
    s.at<std::uint32_t>(0x00) = 0x00CEFC5Cu;                          // 00590622
    s.at<std::uint32_t>(0x08) = 0x00CEFC48u;
    s.at<std::uint32_t>(0x0C) = 0x00CEFC24u;
    s.at<std::uint32_t>(0x40) = 0x00CEFC04u;                          // 00590636
    // EH state 1Ah
    host.debug_text_list_destroy_rows_00543e00(s.p + 0x3C4);          // 0059064D
    host.member_teardown_00544a70(s.p + 0x414);                       // 00590658
    for (std::size_t page : {0x470u, 0x2F0u, 0x244u, 0x248u}) {       // 0059065D..005906A4
        host.gui_manager_remove_and_destroy_page_00aa31f0(host.gui_manager_get_or_create_004c12b0(),
                                                          s.ptr(page));
    }
    host.main_menu_release_mission_preview_005830a0(screen);          // 005906AB
    // EH state 19h, then 18h
    for (std::size_t off : {0x574u, 0x570u}) {                        // 005906B0..00590702
        if (void* object = s.ptr(off)) {
            drop_reference(object, host);
            s.ptr(off) = nullptr;
        }
    }
    for (std::size_t base : {0x520u, 0x504u}) {                       // 00590708..00590761
        if (s.at<std::uint32_t>(base + 0x18) >= 0x10) host.free_00bf65ac(s.ptr(base + 4));
        s.at<std::uint32_t>(base + 0x18) = 0x0F;
        s.at<std::uint32_t>(base + 0x14) = 0;
        s.at<std::uint8_t>(base + 4) = 0;
    }
    // EH state 15h
    if (s.ptr(0x4F4) != nullptr) {                                    // 00590767
        host.native_string_range_destroy_00432050(s.ptr(0x4F4), s.ptr(0x4F8), s.p + 0x4F0, screen);
        host.free_00bf65ac(s.ptr(0x4F4));                             // 00590785
    }
    s.at<std::uint32_t>(0x4F4) = 0;
    s.at<std::uint32_t>(0x4F8) = 0;
    s.at<std::uint32_t>(0x4FC) = 0;
    // EH state 14h
    host.native_string_vector_resize_00427110(s.p + 0x480, 0);        // 005907A4
    host.free_00bf6989(s.ptr(0x480));                                 // 005907AC
    // EH state 13h
    host.debug_text_list_destruct_00590250(s.p + 0x414);              // 005907BF
    // EH state 12h
    host.cheat_menu_destroy_members_0051f910(s.p + 0x3C4);            // 005907CB
    tidy_vector12(s, 0x2E0, host);                                    // 005907D0
    tidy_vector12(s, 0x2D0, host);                                    // 005907F5
    host.list_tidy_005822a0(s.p + 0x2A4);                             // 00590820
    host.list_tidy_005822a0(s.p + 0x298);                             // 0059082B
    host.list_tidy_005822a0(s.p + 0x280);                             // 00590836
    // EH state 0Ch
    host.eh_vector_destructor_iterator_00bf7c6e(s.p + 0x134, 0x10, 5, 0x0041F5D0u);  // 00590850
    for (std::size_t off : {0x104u, 0xF4u, 0xE4u, 0xD4u, 0xC4u, 0xB4u, 0xA4u, 0x94u, 0x84u, 0x74u}) {
        tidy_vector12(s, off, host);                                  // 00590855..005909BB
    }
    // EH state 1
    host.text_listener_base_destroy_00a97140(s.p + 0x0C);             // 005909C3
    s.at<std::uint32_t>(0x08) = 0x00CEB0FCu;                          // 005909CA
    // EH state -1
    host.front_end_screen_destruct_004f71a0(screen);                  // 005909D9
}

// ---------------------------------------------------------------------------
// 00989A80 (435 insns), was CG_vector_deleting_dtor_00989a80; ledger name
// BSP_WarningManager_Destruct. __thiscall(ECX = manager), RET, no delete flag
// (0098A2A0 is the scalar deleting wrapper). The manager is 1B0h bytes at
// game+21E0h (src/game_hosts_global_subsystems.cpp). No base class: the only
// vtable store is D1B968 and no base destructor is called.
// Evidence: assembly 00989A80..00989FEA (exports/bsp/functions/00989a80).
// The binary's order, exactly (cc16 asked that it be recorded):
//   1. vtable 00D1B968; 0096BF70 clears the waiting queue;
//   2. list +ECh: each node's object at node+8h deleted by vtable +0(1), slot zeroed;
//   3. map +F8h: for each node, its inner tree at node+14h: each inner node's
//      object at +14h deleted by vtable +0(1), slot zeroed (iterators advanced
//      by 00966960 inner and 0096A280 outer);
//   4. [[00F8BBCC]+230h] = 0, [00F8BF4C] = 0 (the GUI layer hook installed by
//      009870A0), [00F8A0C4] = 0;
//   5. the recursive lock at +24h: while its count (+18h) > 0, decrement and
//      LeaveCriticalSection; DeleteCriticalSection; free; +24h = 0;
//   6. tree +1A4h (005A5E40); NativeString +19Ch; references +194h, +190h;
//      trees +184h (009735D0), +168h (00988440), +15Ch (00971B70), +150h
//      (00444B10), +144h (0058D860); 0097A190 on +130h then +11Ch;
//      NativeString +114h; trees +108h (008EC550), +F8h (00980270);
//   7. lists +ECh and +E0h cleared inline; tree +D4h (00971770);
//   8. the EH destructor iterator over 13h NativeStrings at +38h (dtor 0041DD20);
//   9. list +28h cleared inline.
// EH states: 12h, 11h, 10h, 0Fh, 0Eh, 0Dh, 0Ch, 0Bh, 0Ah, 9, 8, 7, 6, 5, 4,
// 1, 0. Checked-iterator failures call 00BF6713; the loops keep every test
// the image makes except the always-false CMP EBP,EBP at 00989AD0 (Ghidra
// removed that block as unreachable).
void warning_manager_destruct_00989a80(void* manager, CgRebuildBHost& host) {
    const Raw s = raw(manager);
    s.at<std::uint32_t>(0x00) = 0x00D1B968u;                          // 00989AA8
    // EH state 12h
    host.warning_manager_clear_waiting_queue_0096bf70(manager);       // 00989AB6

    // 00989ABB..00989B0E: the list at +ECh.
    {
        const Raw list = raw(s.p + 0xEC);
        auto** node = static_cast<void**>(*static_cast<void**>(s.ptr(0xF0)));
        for (;;) {
            if (static_cast<void*>(node) == list.ptr(4)) break;       // 00989AE0
            if (static_cast<void*>(node) == list.ptr(4)) host.invalid_parameter_00bf6713();  // 00989AE6
            if (void* object = node[2]) {                             // 00989AF0
                host.object_vtable0_scalar_dtor(object, 1);
                node[2] = nullptr;
            }
            if (static_cast<void*>(node) == list.ptr(4)) host.invalid_parameter_00bf6713();  // 00989B02
            node = static_cast<void**>(node[0]);                      // 00989B0C
        }
    }

    // 00989B10..00989BFC: the map at +F8h and each node's inner tree at +14h.
    {
        void* const outer_tree = s.p + 0xF8;
        CheckedIterImage outer{outer_tree, *static_cast<void**>(s.ptr(0xFC))};
        for (;;) {
            void* const outer_end = s.ptr(0xFC);                      // 00989B32
            if (outer.container == nullptr || outer.container != outer_tree) host.invalid_parameter_00bf6713();
            if (outer.node == outer_end) break;                       // 00989B4D
            if (outer.container == nullptr) host.invalid_parameter_00bf6713();
            if (outer.node == raw(outer.container).ptr(4)) host.invalid_parameter_00bf6713();
            const Raw onode = raw(outer.node);
            CheckedIterImage inner{onode.p + 0x14, *static_cast<void**>(onode.ptr(0x18))};  // 00989B6A
            for (;;) {
                if (outer.node == raw(outer.container).ptr(4)) host.invalid_parameter_00bf6713();  // 00989B88
                const Raw cur = raw(outer.node);
                void* const inner_end = cur.ptr(0x18);                // 00989B96
                void* const inner_tree = cur.p + 0x14;
                if (inner.container == nullptr || inner.container != inner_tree) host.invalid_parameter_00bf6713();
                if (inner.node == inner_end) break;                   // 00989BAD
                if (inner.container == nullptr) host.invalid_parameter_00bf6713();
                if (inner.node == raw(inner.container).ptr(4)) host.invalid_parameter_00bf6713();
                const Raw inode = raw(inner.node);
                if (void* object = inode.ptr(0x14)) {                 // 00989BC6
                    host.object_vtable0_scalar_dtor(object, 1);
                    inode.ptr(0x14) = nullptr;
                }
                host.stl_tree_iterator_increment(0x00966960u, &inner);  // 00989BDC
            }
            host.stl_tree_iterator_increment(0x0096A280u, &outer);      // 00989BEF
        }
    }

    raw(host.global_ptr(0x00F8BBCCu)).at<std::uint32_t>(0x230) = 0;  // 00989C01
    host.global_store_u32(0x00F8BF4Cu, 0);                            // 00989C0C
    host.global_store_u32(0x00F8A0C4u, 0);                            // 00989C12

    if (void* lock = s.ptr(0x24)) {                                   // 00989C18
        auto& count = raw(lock).at<std::int32_t>(0x18);
        while (count > 0) {                                           // 00989C1F..00989C3A
            count += -1;
            host.leave_critical_section_00ce2210(lock);
        }
        host.delete_critical_section_00ce2214(lock);                  // 00989C3D
        host.free_00bf65ac(lock);                                     // 00989C44
        s.at<std::uint32_t>(0x24) = 0;
    }
    // EH state 11h
    tidy_tree(s, 0x1A4, 0x005A5E40u, host);                           // 00989C4F..00989C7E
    // EH state 10h
    release_native_string(s.at<NativeStringImage>(0x19C), host);      // 00989C81
    // EH state 0Fh, then 0Eh
    for (std::size_t off : {0x194u, 0x190u}) {                        // 00989CAC..00989CFA
        if (void* object = s.ptr(off)) {
            drop_reference(object, host);
            s.ptr(off) = nullptr;
        }
    }
    // EH states 0Dh, 0Ch, 0Bh, 0Ah, 9
    tidy_tree(s, 0x184, 0x009735D0u, host);                           // 00989D1E
    tidy_tree(s, 0x168, 0x00988440u, host);                           // 00989D53
    tidy_tree(s, 0x15C, 0x00971B70u, host);                           // 00989D88
    tidy_tree(s, 0x150, 0x00444B10u, host);                           // 00989DBD
    tidy_tree(s, 0x144, 0x0058D860u, host);                           // 00989DF2
    // EH state 8
    host.member_teardown_0097a190(s.p + 0x130);                       // 00989E14
    // EH state 7
    host.member_teardown_0097a190(s.p + 0x11C);                       // 00989E24
    // EH state 6
    release_native_string(s.at<NativeStringImage>(0x114), host);      // 00989E29
    // EH states 5, 4
    tidy_tree(s, 0x108, 0x008EC550u, host);                           // 00989E6F
    tidy_tree(s, 0xF8, 0x00980270u, host);                            // 00989EA4
    tidy_list(s, 0xEC, host);                                         // 00989EB8..00989F01
    tidy_list(s, 0xE0, host);                                         // 00989F07..00989F5A
    // EH state 1
    tidy_tree(s, 0xD4, 0x00971770u, host);                            // 00989F75
    // EH state 0
    host.eh_vector_destructor_iterator_00bf7c6e(s.p + 0x38, 8, 0x13, 0x0041DD20u);  // 00989F9D
    tidy_list(s, 0x28, host);                                         // 00989FA2..00989FD9
}

}  // namespace bsp::cg_rebuild_b_cc13
