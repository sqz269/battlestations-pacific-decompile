#pragma once
// Segment 20 (00581CE0-005C5470), cc13-seg1b: front-end main menu, HUD map,
// narrative and minimap screen leftovers. docs/CC13_SEG20.md has the evidence.
// Names are hypotheses, not recovered symbols. Objects are raw image storage
// (Raw::at at the image's offsets); every engine routine the image calls is a
// named host method invoked at the same point. Nothing here runs in the game.
#include "bsp/engine_seg49_64_78_cc13.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp::frontend_seg20_cc13 {

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};
inline Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

struct Seg20Host {
    virtual ~Seg20Host() = default;
    // Image globals (dword storage at the address).
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;

    // --- main menu screen ---
    // 005806A0 __thiscall(screen): the selected mission record (its name first).
    virtual void* selected_mission_005806a0(void* screen) = 0;
    // 007FC490 __thiscall([00E188A8]+650h, out, const NativeString* name) RET 8.
    virtual cc13_seg49_64_78::ProfileCheckpointRecord checkpoint_get_copy_007fc490(void* name) = 0;
    virtual void* vehicle_class_registry_00437f50() = 0;    // singleton
    virtual void play_selected_mission_005922f0(void* screen) = 0;
    virtual void build_mission_detail_page_0058c010(void* screen) = 0;
    // 00590600 __thiscall(screen): the screen's destructor body.
    virtual void main_menu_screen_destruct_00590600(void* screen) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00AA6750 __thiscall(widget, float out[3]) RET 4: resolved position.
    virtual std::array<float, 3> widget_resolved_position_00aa6750(void* widget) = 0;
    // 006833F0 __fastcall(scroller): re-read both base positions.
    virtual void scroller_recapture_006833f0(void* scroller) = 0;
    // 00419330's checked-vector bound tests: _invalid_parameter_noinfo.
    virtual void invalid_parameter_00bf6713() = 0;

    // --- narrative screen ---
    virtual void widget_vslot34(void* widget, std::int32_t value) = 0;             // vtable +34h
    virtual void widget_vslot54(void* widget, std::uint8_t (&out)[16]) = 0;        // vtable +54h(&local16)
    virtual void widget_vslot58(void* widget, const float (&xy)[2]) = 0;           // vtable +58h(&vec2)
    virtual void widget_vslot88(void* widget, void* texture, std::int32_t a, float b) = 0;  // vtable +88h
    // 00ABBE50 __thiscall(text, const char*, int) RET 8.
    virtual void text_set_localised_cstring_00abbe50(void* text, const char* value, std::int32_t flag) = 0;
    // 00ABAED0 __thiscall(text, source, int) RET 8.
    virtual void text_set_localised_source_00abaed0(void* text, std::uint32_t source, std::int32_t flag) = 0;
    // 00AB6BD0 __thiscall(text) -> ST0.
    virtual double text_normalized_height_00ab6bd0(void* text) = 0;

    // --- icon maps (std::map nodes: key +0Ch, value +10h) ---
    // 005A1800 __thiscall(map, out iterator pair, const key*) RET 8 -> the node.
    virtual void* hud_map_icon_find_005a1800(void* map, std::uint32_t key) = 0;
    // 00537F20 / 005BF810 __thiscall(map, out, container, node) RET 0Ch: erase.
    virtual void hud_map_icon_erase_00537f20(void* map, void* node) = 0;
    virtual void minimap_icon_erase_005bf810(void* map, void* node) = 0;
    // object->vtable[0](1): the scalar deleting destructor.
    virtual void delete_object_vslot00(void* object, std::int32_t flags) = 0;

    // --- HUD map icons ---
    // object->vtable[5Ch](kind): the is-a test.
    virtual bool is_a_vslot5c(void* object, std::int32_t kind) = 0;
    // 00AA6740 __thiscall(widget) -> const float* size (x, y).
    virtual std::array<float, 2> widget_size_00aa6740(void* widget) = 0;
    // 00AA8240 __thiscall(widget, const float* xyz) RET 4: set resolved position.
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float (&xyz)[3]) = 0;
    virtual void widget_vslot4c(void* widget, float value) = 0;                    // vtable +4Ch(float)
};

// 0059CEF0 __thiscall(icon, byte invert) RET 4 -> AL. a = +110h, g = float
// [00F876A4]. g >= a: +100h/+104h = +F8h/+FCh and AL = 1, t = 1.0. Else t =
// (float)((float)(g - +114h) / (float)(a - +114h)), +100h/+104h = +108h/+10Ch
// + (float)((+F8h - +108h) * t) / (float)(t * (+FCh - +10Ch)), each truncated
// (CVTTSS2SI) to an even integer ((n / 2) * 2) and stored back as float; AL = 0.
// Then with w = the first widget of the vector at +ECh (checked index 0):
// n = ((float)((+100h + 52.0) / 960.0), (float)((52.0 + +104h) / 720.0)); when
// 00AA6740(w) differs from n in x or y, w->vtable[58h](&n); w->vtable[34h](byte
// +2Ah); w->vtable[4Ch]((float)(double)(invert ? 1 - t : t)).
bool hud_map_icon_update_size_and_visibility_0059cef0(void* icon, bool invert, Seg20Host& host);

// 0059D150 __thiscall(icon, byte* b, byte* c, byte* d, byte* a) RET 10h. All
// four are cleared (a, d, c, b). unit = [icon+4]: *a = unit is-a 8 ?
// [unit+122Ch] == 2 : byte [unit+9E5h]; *d = 1 when a member of the list at
// [unit+48h] (next +44h) is-a 20h with byte +5Dh set; *b = 1 when float
// [unit+A54h] (00939F80 on unit+A20h) > 0.9f; *c = 1 when float [unit+A58h]
// (00939F70) > 0.9f.
void hud_map_icon_query_unit_status_0059d150(void* icon, std::uint8_t* b, std::uint8_t* c, std::uint8_t* d, std::uint8_t* a,
                                             Seg20Host& host);

// 0059D350 __fastcall(label): text [+124h], frame [+120h]. w = (float)([text+
// 114h] / 960.0), h = (float)00AB6BD0(text); dx = (float)((size.x - w) * 0.5),
// dy = (float)((size.y - h) * 0.5) with size = 00AA6740(frame) (called twice);
// p = 00AA6750(frame); 00AA8240(text, {p.x + dx, p.y + dy, p.z - 10.0}).
void hud_map_label_center_on_widget_0059d350(void* label, Seg20Host& host);

// 00581CE0 __thiscall(screen, int list) RET 4. With i = [00E194DC] and the
// 12-byte point vector at screen+134h+list*10h (begin +4h): anchor +184h..+18Ch
// = point[i] * zoom (+130h), each product stored to float; +18Ch = 0.0
// (XMM0); then anchor += (+1A4h + +190h, +1A8h + +194h, +1ACh + +198h), every
// sum stored to float. The checked index test (00BF6713 when the vector is null
// or i >= size) is a host call.
void main_menu_layout_mission_row_00581ce0(void* screen, std::uint32_t list, Seg20Host& host);

// 00588280 __fastcall(screen): the widget listener's +1Ch slot. +E4h..+ECh =
// 00AA6750 of +2E8h when set; +518h..+520h = 00AA6750 of +178h when set; the
// five 12-byte vectors at +F4h, +104h, +114h, +124h, +134h are cleared
// (erase(begin, end) through 00419330, which over an empty source returns
// begin); then 006833F0 on +194h and on +314h (tail jump).
void main_menu_listener_current_1c_00588280(void* screen, Seg20Host& host);

// 00590D60 __thiscall(screen, byte flags) RET 4 -> EAX = screen: 00590600,
// then free(screen) when bit 0 is set.
void* main_menu_screen_scalar_deleting_dtor_00590d60(void* screen, std::uint8_t flags, Seg20Host& host);

// 00592640 __thiscall(screen, int result) RET 4: the play-mission prompt.
// result 1: +5Ch = 1, record = 007FC490(005806A0(screen)), [00E188A8]+6ACh =
// record word 0, the vehicle class index maps (registry +10h / +2010h, 800h
// entries) reset to the identity, forward[word 1] = word 2 and inverse[word 2]
// = word 1, then 005922F0; the record's string is released. Otherwise +5Ch = 0
// and 0058C010.
void main_menu_on_play_mission_prompt_result_00592640(void* screen, std::int32_t result, Seg20Host& host);

// 005B5C10 __thiscall(screen, a, texture) RET 8: when texture != 0,
// [screen+40h]->vtable[88h](texture, 0, 1.0f). The first argument is unused.
void narrative_set_portrait_005b5c10(void* screen, void* texture, Seg20Host& host);

// 005B5D20 __thiscall(screen, unused) RET 4: [screen+B0h]->vtable[54h] into a
// 16-byte local that is then dropped.
void narrative_query_text_005b5d20(void* screen, Seg20Host& host);

// 005B7390 __thiscall(screen, source, float* x, float* y, float* w) RET 10h:
// [+ACh]->vtable[34h](1); text [+B0h]: 00ABBE50("" at 00CE3A0C, 1),
// 00ABAED0(source, 1); t = (float)([text+114h] / 960.0); [+B4h]->vtable[58h]
// ({(float)(t + 0.13333), (float)(00AB6BD0(text) + 0.011111)}); text
// vtable[54h] into a dropped local; then with g = [[00E198C4]+A4h]: *x =
// (float)(g[78h] * 0.5), *y = (float)((int)[text+ECh] * g[7Ch] + g[80h]),
// *w = g[78h].
void narrative_show_text_005b7390(void* screen, std::uint32_t source, float* x, float* y, float* w,
                                  Seg20Host& host);

// 005A5920 __thiscall(screen, key) RET 4, observer slot 0 (vtable 00CF011C):
// 005A1800 finds key in the icon map at +490h; a hit (node != head [+494h])
// deletes the icon at node+10h (vtable[0](1)), clears node+10h and erases the
// node (00537F20). Always sets byte +534h = 1.
void hud_map_icon_observer_on_released_005a5920(void* screen, std::uint32_t key, Seg20Host& host);

// 005BFDF0 __thiscall(screen, key) RET 4, observer slot 0 (vtable 00CF13F0):
// 005BE110 on the minimap icon map at +F0h; a hit (node != head [+F4h])
// deletes the icon at node+10h (vtable[0](1), node+10h not cleared) and erases
// the node (005BF810).
void minimap_icon_observer_on_released_005bfdf0(void* screen, std::uint32_t key, Seg20Host& host);

}  // namespace bsp::frontend_seg20_cc13
