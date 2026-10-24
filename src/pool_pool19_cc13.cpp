#include "bsp/pool_pool19_cc13.hpp"

#include <cstddef>
#include <cstring>

// cc13 pool19. Evidence is the disk listing (bsp.py disasm-raw) checked
// against Ghidra's pseudocode where the decompiler lost register arguments;
// literals and constants were read from the image bytes (2026-10-24).
// Descriptive names are hypotheses.

namespace bsp::cc13_pool19 {
namespace {

template <typename T>
T& field(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

void* at(void* base, std::size_t offset) { return static_cast<std::uint8_t*>(base) + offset; }

template <typename T>
T* pointer_from_dword(std::uint32_t value) {
    return reinterpret_cast<T*>(static_cast<std::uintptr_t>(value));
}

struct NativeString {
    std::uint32_t length;
    void* data;
};

// The inlined release: 00419CC0 then ReturnBlock(data, length + 1, 1) when
// data is non-null. The header is not cleared.
void release_string_at(void* header, Pool19Host& host) {
    auto& text = *static_cast<NativeString*>(header);
    if (text.data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, text.data, text.length + 1, 1);
    }
}

// The inlined NativeString assignment: skipped when both are the same object;
// Resize(source length, 1); the copy runs only for a non-empty source and
// copies the destination's new length.
void assign_string(void* dst, void* src, Pool19Host& host) {
    if (dst == src) return;
    host.native_string_resize_0041dd40(dst, field<std::uint32_t>(src, 0), 1);
    if (field<std::uint32_t>(src, 0) != 0) {
        host.memcpy_00bf7680(field<void*>(dst, 4), field<void*>(src, 4), field<std::uint32_t>(dst, 0));
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 005C8AE0 BSP_MissionRecord_Destroy_Provisional (240 insns, 005C8AE0..005C8DAC)
// ABI: __thiscall(ECX = record), plain RET; SEH handler 00C73161. Element
// destructor of the mission record arrays (the 14 callers include the scalar
// deleting destructor 005C8E80 and the range helpers around 005C8EF0).
// Members are destroyed in reverse layout order:
//  1. state 10h: the std::map at +428h (head +42Ch, size +430h). It erases
//     [head->left, head) through 00576740 into a dead iterator local, then
//     frees the head (00BF65AC) and clears +42Ch and +430h.
//  2. state 0Fh: 00BF7C6E(+B8h, 154h, 2, 005C4D40) destroys the two
//     MissionSideBlocks at +B8h and +20Ch.
//  3. state 0Eh: the picture handle +A0h. When non-null, InterlockedDecrement
//     ([00CE2220]) on its +4h, its vtable[0]() at zero, then +A0h = 0 (still
//     inside the non-null branch). The UV floats at +A4h are not touched.
//     src/mission_picture_owner.cpp already models this member
//     (MissionPictureOwner::release_then_clear); this is the whole body.
//  4. state 0Dh: the NativeString +98h. Then the POD vector {+88h proxy,
//     +8Ch first, +90h last, +94h end}: _free(first) when non-null, then
//     first/last/end = 0. No state is stored for it (0Ch is never written).
//  5. state 0Bh: the NativeString vector at +78h: when first (+7Ch) is
//     non-null, 00432050(first, last, &vector, record) and _free(first); then
//     first/last/end = 0 either way.
//  6. states 0Ah..1, 0 and -1: the NativeStrings at +58h, +50h, +48h, +40h,
//     +38h, +30h, +28h, +20h, +18h, +10h, +08h and +00h, each released and
//     not cleared.
// Plain data the destructor leaves alone: the 18h bytes +60h..+77h between the
// strings and the vectors (three dwords and three floats), and the C4h-byte
// block +360h..+423h after the two side blocks plus the byte at +424h.
// Uncertainty: what each string and the map hold. The record is at least
// 434h bytes.
// ---------------------------------------------------------------------------
void mission_record_destroy_005c8ae0(void* record, Pool19Host& host) {
    void* const tree = at(record, 0x428);
    void* const head = field<void*>(record, 0x42C);
    void* dead_iterator[2] = {nullptr, nullptr};
    host.tree_erase_range_00576740(tree, dead_iterator, tree, field<void*>(head, 0), tree, head);
    host.free_00bf65ac(field<void*>(tree, 4));
    field<void*>(tree, 4) = nullptr;
    field<std::uint32_t>(tree, 8) = 0;

    host.eh_vector_destruct_00bf7c6e(at(record, 0xB8), 0x154, 2, 0x005C4D40u);

    void* const picture = field<void*>(record, 0xA0);
    if (picture != nullptr) {
        if (host.interlocked_decrement_00ce2220(at(picture, 4)) == 0) host.virtual_call_0(picture, 0);
        field<void*>(record, 0xA0) = nullptr;
    }

    release_string_at(at(record, 0x98), host);

    if (field<void*>(record, 0x8C) != nullptr) host.free_00bf65ac(field<void*>(record, 0x8C));
    field<void*>(record, 0x8C) = nullptr;
    field<void*>(record, 0x90) = nullptr;
    field<void*>(record, 0x94) = nullptr;

    void* const strings = at(record, 0x78);
    if (field<void*>(strings, 4) != nullptr) {
        host.native_string_range_destroy_00432050(field<void*>(strings, 4), field<void*>(strings, 8), strings,
                                                   record);
        host.free_00bf65ac(field<void*>(strings, 4));
    }
    field<void*>(strings, 4) = nullptr;
    field<void*>(strings, 8) = nullptr;
    field<void*>(strings, 0xC) = nullptr;

    for (int offset = 0x58; offset >= 0; offset -= 8) release_string_at(at(record, offset), host);
}

// ---------------------------------------------------------------------------
// 005D2F90 BSP_MultiErrorScreen_Construct (363 insns, 005D2F90..005D3405)
// ABI: __thiscall(ECX = screen), EBP frame aligned to 8, plain RET, EAX =
// screen. SEH handler 00C73E69. Only caller 00689540 BSP_MultiMenu_Init; the
// 5Ch-byte screen goes to manager 00E198B4 + 50h there.
// Body: base 004F7180; +08h = 00CEB0FC, +0Ch = 00CEB110 (the intermediate
// class); state 1; then +00h = 00CF1BF4, +08h = 00CF1BE0, +0Ch = 00CF1BC0.
// 00BF7CD1(+30h, 10h, 2, ctor 004322C0, dtor 004324A0) builds two checked
// NativeString vectors at +30h and +40h (state 2). (The ledger's earlier note
// named 004324A0 the constructor; the push order makes it the destructor.)
// Then ten labels: for each a temporary NativeString {0, 0} is resized to the
// literal's length with preserve 1, the literal is copied with length + 1
// bytes when the data is non-null, it is appended with 00450540 (state 3..0Ch
// for the ten appends, back to 2 after each) and released. The first five go
// to +30h (00CED4F0 "globals.yes", 00CED4E4 "globals.no", 00CED4D4
// "globals.accept", 00CF1C3C "globals.marketplace_pc", 00CEB3A8
// "globals.back"); the second five to +40h (00CED480, 00CED46C, 00CED454,
// 00CF1C1C, 00CED3EC: the "globals.dialog_*" keys).
// +50h..+58h are not written here (the register virtual 005D2440 clears +54h).
// Uncertainty: which vector holds button labels and which dialog texts is a
// reading of the keys.
// ---------------------------------------------------------------------------
void* multi_error_screen_construct_005d2f90(void* screen, Pool19Host& host) {
    host.front_end_screen_construct_004f7180(screen);
    field<std::uint32_t>(screen, 0x08) = 0x00CEB0FCu;
    field<std::uint32_t>(screen, 0x0C) = 0x00CEB110u;
    field<std::uint32_t>(screen, 0x00) = 0x00CF1BF4u;  // state 1
    field<std::uint32_t>(screen, 0x08) = 0x00CF1BE0u;
    field<std::uint32_t>(screen, 0x0C) = 0x00CF1BC0u;
    host.eh_vector_construct_00bf7cd1(at(screen, 0x30), 0x10, 2, 0x004322C0u, 0x004324A0u);  // state 2

    auto push_label = [&](std::size_t vector_offset, const char* literal) {
        NativeString text{0, nullptr};
        host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(literal)), 1);
        if (text.data != nullptr) host.memcpy_00bf7680(text.data, literal, text.length + 1);
        host.native_string_vector_append_00450540(at(screen, vector_offset), &text);  // states 3..0Ch
        release_string_at(&text, host);                                              // state 2
    };
    for (const char* key : kMultiErrorButtonKeys) push_label(0x30, key);
    for (const char* key : kMultiErrorDialogKeys) push_label(0x40, key);
    return screen;
}

// ---------------------------------------------------------------------------
// 00618980 BSP_HudPowerupCarousel_RefreshSelectedLabel (367 insns, 00618980..00618E5C)
// ABI: __thiscall(ECX = carousel), plain RET; SEH handler 00C77CF3. Callers
// 00619C50, 0061A400, 0061BEA0. The HUD screen is hud = [[00E198C4]+C0h],
// re-read at every use. Carousel fields: +00h slot position, +04h selected
// list index, +10h listener, +1Ch/+20h a pointer vector, +3Dh/+3Eh/+3Fh flags.
// The powerup list is hud's checked vector {+144h proxy, +148h first, +14Ch
// last}; each entry's +4h is a record whose +0h is its NativeString name and
// whose +0Ch + team*38h is its localised name.
//  1. Leave unless the carousel's own vector (+1Ch..+20h) is non-empty and
//     (+3Fh == 0 or +3Eh != 0). Clear +3Eh.
//  2. Hide the four icons [hud+48h..54h] with vtable[34h](0).
//  3. When +04h >= the list count (0 for a null list): +00h = +04h = 0.
//  4. Checked element +04h (00BF6713 when out of range, then the access
//     anyway). [hud+D4h] label: 00ABAED0(entry->record + 0Ch + team*38h, 1),
//     team = [[[00E188A8] + 18CCh + [[00E188A8]+18ECh]*4] + 28h].
//  5. q = (float)([[hud+D4h]+114h] / 960.0). When q > [hud+254h] (FCOMIP/JBE:
//     an unordered compare keeps the old width) +254h = q and [hud+D4h]
//     vtable[34h](0). Then +234h = (float)([hud+254h] + 0.013) and +238h =
//     00AA6740([hud+ECh])[1].
//  6. Four temporary NativeStrings built by 00BF7CD1(ctor 00415270, dtor
//     0041DD20) (state 0). Backward: for slot k = +00h-1 down to 0 the index
//     wraps below zero (idx == 0 -> count, then idx-1) and slot k takes that
//     record's name. Centre: slot +00h takes element +04h's name. Forward:
//     while slot <= 3, the list is non-null and slot < count, idx+1 wraps at
//     count to 0 and the slot takes that name. filled = (+00h + 1) + forward.
//  7. For i < filled: id = *(uint16*)00617870(hud+1B4h, &slot[i]) (the map's
//     operator[] inserts a missing key); icon i vtable[88h](id, 0, 1.0f) then
//     vtable[34h](1). The image reads icon i's vtable+88h slot address before
//     the 00617870 call (00618D8B..00618D97) and re-reads the icon pointer
//     after it (00618DB4). The C++ reads the icon once before the call, which
//     is equivalent because 00617870 does not touch the HUD icons.
//  8. [hud + 78h + (+00h)*4] vtable[34h](1).
//  9. When +10h is non-null, +3Dh is set and the carousel's own vector is
//     non-empty: +3Dh = 0 and [+10h]->vtable[0](+04h).
// 10. State -1, 00BF7C6E destroys the four strings.
// x87: main-thread precision is 24-bit (docs/CC13_FPU_PRECISION.md); both
// results are stored to floats, and 0.013 is exactly 0.013f, so float
// division and addition reproduce them.
// Uncertainty: the icon slot meaning of vtable[88h]'s arguments; a +00h above
// 3 indexes past the four temporaries in the image as well.
// ---------------------------------------------------------------------------
void hud_powerup_carousel_refresh_selected_label_00618980(void* carousel, Pool19Host& host) {
    auto hud = [&]() -> void* {
        return field<void*>(pointer_from_dword<void>(host.global_dword(0x00E198C4u)), 0xC0);
    };
    auto list_first = [&]() -> void** { return field<void**>(hud(), 0x148); };
    auto list_count = [&]() -> std::uint32_t {
        void** const first = list_first();
        if (first == nullptr) return 0;
        return static_cast<std::uint32_t>(field<void**>(hud(), 0x14C) - first);
    };
    auto checked_entry = [&](std::uint32_t index) -> void* {
        if (list_first() == nullptr || index >= list_count()) host.invalid_parameter_00bf6713();
        return list_first()[index];
    };

    // SUB / SAR 2 / JE on the carousel's own vector: a signed element count.
    auto own_vector_count = [&]() -> std::int32_t {
        const auto bytes = static_cast<std::int32_t>(field<std::uint32_t>(carousel, 0x20) - field<std::uint32_t>(carousel, 0x1C));
        return bytes >> 2;
    };
    void* const own_first = field<void*>(carousel, 0x1C);
    if (own_first == nullptr) return;
    if (own_vector_count() == 0) return;
    if (field<std::uint8_t>(carousel, 0x3F) != 0 && field<std::uint8_t>(carousel, 0x3E) == 0) return;
    if (field<std::uint8_t>(carousel, 0x3E) != 0) field<std::uint8_t>(carousel, 0x3E) = 0;

    for (std::size_t offset = 0x48; offset < 0x58; offset += 4) {
        host.virtual_call_1(field<void*>(hud(), offset), 0x34, 0);
    }
    auto& position = field<std::int32_t>(carousel, 0);
    auto& selected = field<std::uint32_t>(carousel, 4);
    if (selected >= list_count()) {
        position = 0;
        selected = 0;
    }

    void* const game = pointer_from_dword<void>(host.global_dword(0x00E188A8u));
    void* const entry = checked_entry(selected);
    void* const player = field<void*>(game, 0x18CC + field<std::int32_t>(game, 0x18EC) * 4);
    const std::int32_t team = field<std::int32_t>(player, 0x28);
    host.gui_text_set_localised_source_00abaed0(field<void*>(hud(), 0xD4),
                                                at(field<void*>(entry, 4), 0x0C + team * 0x38), 1);

    const float q = static_cast<float>(field<float>(field<void*>(hud(), 0xD4), 0x114) /
                                       static_cast<float>(kLabelWidthDivisor_00cec380));
    if (q > field<float>(hud(), 0x254)) {
        field<float>(hud(), 0x254) = q;
        host.virtual_call_1(field<void*>(hud(), 0xD4), 0x34, 0);
    }
    const float* const size = host.gui_widget_get_size_00aa6740(field<void*>(hud(), 0xEC));
    const float size_y = size[1];
    field<float>(hud(), 0x234) = field<float>(hud(), 0x254) + kLabelPad_00ced090;
    field<float>(hud(), 0x238) = size_y;

    NativeString slots[4];
    host.eh_vector_construct_00bf7cd1(slots, 8, 4, 0x00415270u, 0x0041DD20u);  // state 0

    std::int32_t filled = 1;
    std::uint32_t index = selected;
    if (position - 1 >= 0) {
        filled = position + 1;
        for (std::int32_t k = position - 1; k >= 0; --k) {
            if (index == 0) index = list_count();
            index -= 1;
            assign_string(&slots[k], field<void*>(checked_entry(index), 4), host);
        }
    }
    assign_string(&slots[position], field<void*>(checked_entry(selected), 4), host);

    index = selected;
    for (std::int32_t slot = position + 1; slot <= 3; ++slot) {
        if (list_first() == nullptr || static_cast<std::uint32_t>(slot) >= list_count()) break;
        index += 1;
        if (index == list_count()) index = 0;
        assign_string(&slots[slot], field<void*>(checked_entry(index), 4), host);
        ++filled;
    }

    for (std::int32_t i = 0; i < filled; ++i) {
        void* const icon = field<void*>(hud(), 0x48 + static_cast<std::size_t>(i) * 4);
        const std::uint16_t id =
            *static_cast<std::uint16_t*>(host.string_map_ci_subscript_00617870(at(hud(), 0x1B4), &slots[i]));
        host.virtual_call_icon_set_88(icon, id, 0, 1.0f);
        host.virtual_call_1(field<void*>(hud(), 0x48 + static_cast<std::size_t>(i) * 4), 0x34, 1);
    }
    host.virtual_call_1(field<void*>(hud(), 0x78 + static_cast<std::size_t>(position) * 4), 0x34, 1);

    void* const listener = field<void*>(carousel, 0x10);
    if (listener != nullptr && field<std::uint8_t>(carousel, 0x3D) != 0) {
        void* const first = field<void*>(carousel, 0x1C);
        if (first != nullptr && own_vector_count() != 0) {
            field<std::uint8_t>(carousel, 0x3D) = 0;
            host.virtual_call_1(listener, 0, selected);
        }
    }
    host.eh_vector_destruct_00bf7c6e(slots, 8, 4, 0x0041DD20u);  // state -1
}

}  // namespace bsp::cc13_pool19
