#pragma once
// Segment 20 (00581CE0-005C5470), cc13-seg1b: front-end main menu, HUD map,
// narrative and minimap screen leftovers. docs/CC13_SEG20.md has the evidence.
// Names are hypotheses, not recovered symbols. Objects are raw image storage
// (Raw::at at the image's offsets); every engine routine the image calls is a
// named host method invoked at the same point. Nothing here runs in the game.
#include "bsp/engine_seg49_64_78_cc13.hpp"
#include "bsp/frontend_screens_cc13.hpp"

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

struct Seg20Host : cc13_fe1::EdgeReferenceHost {
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
    // 008DDF90 __thiscall(szurke, unit) RET 4 -> AL.
    virtual bool szurke_contains_unit_008ddf90(void* szurke, void* unit) = 0;
    // 007788B0 __fastcall(unit) -> AL.
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 00521F10 __thiscall(unit+1E4h, side) RET 4: the side's 34h-byte record.
    virtual std::int32_t unit_side_record_value_00521f10(void* records, std::uint32_t side) = 0;

    // --- models and hangars ---
    virtual void* generated_model_geometry_00b74640(void* model, std::int32_t index) = 0;
    virtual std::uint32_t mesh_count58_00b72b40(void* geometry) = 0;
    virtual void* mesh_element_00b732c0(void* geometry, std::uint32_t index) = 0;
    // 00B172D0 __thiscall(effect) -> const NativeString*; returns its data (+4h), null when empty.
    virtual const char* effect_name_data_00b172d0(void* effect) = 0;
    virtual const char* strstr_00bf9440(const char* text, const char* pattern) = 0;
    virtual void* shipyard_first_live_hangar_object_00845750(void* unit) = 0;
    virtual void* airfield_find_usable_hangar_object_006d2730(void* unit) = 0;
    // 00535110 __thiscall(iterator*): the in-order successor of a map node.
    virtual void* tree_next_00535110(void* node) = 0;
    // 0059E690 __thiscall(array slot, int capacity) RET 4.
    virtual void icon_array_reserve_0059e690(void* slot, std::int32_t capacity) = 0;

    // --- minimap markers ---
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 00AAB4C0 __thiscall(template widget, 0) RET 4 -> the clone.
    virtual void* widget_clone_00aab4c0(void* widget, std::int32_t flags) = 0;
    // 00AAA5A0 __thiscall(parent, child, 0) RET 8: add the child.
    virtual void widget_add_child_00aaa5a0(void* parent, void* child, std::int32_t flags) = 0;
    // 005C0790 __thiscall(vector, void* const* value) RET 4: push_back.
    virtual void pointer_vector_push_back_005c0790(void* vector, void* const* value) = 0;
    // 005BCBD0 __thiscall(vector, index) RET 4 -> &vector[index] (checked).
    virtual void** pointer_vector_at_005bcbd0(void* vector, std::uint32_t index) = 0;

    // cc13-x87.
    virtual void widget_vslot7c(void* widget) = 0;                                 // vtable +7Ch()
    virtual void widget_vslot80(void* widget) = 0;                                 // vtable +80h()
    virtual void widget_vslot50(void* widget, const float (&rect)[4]) = 0;         // vtable +50h(&rect)
    virtual void widget_vslot64(void* widget, float* x0, float* y0, float* x1, float* y1) = 0;  // +64h
    // 00419440 BSP_Vector3f_Length (ECX vector), ST0; every caller here stores it as float at once.
    virtual float vector3_length_00419440(const float* v) = 0;
    // 00419260 BSP_Vector2f_ReciprocalLength (ECX vector), ST0; stored as float at once.
    virtual float vector2_reciprocal_length_00419260(const float* v) = 0;
    // 00BF9940, the CRT's x87-register acos (ST0 in, ST0 out); stored as float at once.
    virtual double crt_acos_00bf9940(double x) = 0;
    // 00AC2DC0 __thiscall(ECX points widget, float x, float y), RET 8: appends
    // the point to the vector at +ECh (004222C0).
    virtual void points_widget_add_point_00ac2dc0(void* widget, float x, float y) = 0;
    // 00AA67F0 BSP_GuiWidget_GetLocalPosition __thiscall(widget): widget + 0Ch.
    virtual float* widget_local_position_00aa67f0(void* widget) = 0;
    // 00554600 BSP_GuiRect_DrawOutline __fastcall(ECX a, EDX b, color), RET 4.
    // `color_address` is the pushed immediate (00E194F0 here).
    virtual void gui_rect_draw_outline_00554600(const float* a, const float* b,
                                                std::uint32_t color_address) = 0;
};

inline constexpr std::uint32_t kMinimapMarkerVtable = 0x00CF1154u;

// 005BCAC0 __thiscall(marker, unit) RET 4 -> marker: vtable 00CF1154; with
// layout = [[00E198C4]+58h], the template is layout+7Ch for no unit, else
// layout+74h (is-a 8), +6Ch (is-a 6), +70h (is-a 0Fh) or +78h; marker+4 =
// 00AAB4C0(template, 0); 00AAA5A0(layout+F0h, marker+4, 0); then
// (marker+4)->vtable[34h](1).
void* minimap_marker_construct_005bcac0(void* marker, void* unit, Seg20Host& host);

// 005C0930 __thiscall(screen, unit) RET 4 -> marker. Category k: 0 for no
// unit, else 1 (is-a 8), 2 (is-a 6), 3 (is-a 0Fh), 4. The use counter at
// screen+154h+k*4 is incremented; when it is below the size of the pointer
// vector at screen+104h+k*10h that element is returned (k = 0 inline with a
// checked index, otherwise 005BCBD0); else a new 8-byte marker
// (00BF681B, 005BCAC0(unit) unless the allocation failed, then null) is
// pushed (005C0790) and returned.
void* minimap_acquire_marker_005c0930(void* screen, void* unit, Seg20Host& host);

// 0059EAD0 __thiscall(slot, int count) RET 4: the slot points at {data, size,
// ...}; cur = [slot] ? [[slot]+4] : 0. When count != cur: 0059E690(slot,
// count), then [[slot]+4] = count (the block re-read after the reserve).
void hud_map_icon_array_resize_0059ead0(void* slot, std::int32_t count, Seg20Host& host);

// 005A07B0 __fastcall(ECX = model, EDX = const NativeString* name) -> EAX:
// for i < 00B72B40(00B74640(model, 0)), element = 00B732C0(00B74640(model,
// 0), i), material = [element+20h]; the first material whose effect name
// (00B172D0 on [material+7Ch]) contains the name (both data pointers non-null,
// strstr hit at an offset other than -1) is returned; 0 otherwise. `name_data`
// is the name's +4h data pointer.
void* generated_model_find_material_by_effect_name_005a07b0(void* model, const char* name_data, Seg20Host& host);

// 005A1220 __thiscall(screen, object) RET 4 -> EAX: walks the icon map at
// +4F4h in order; for each icon (node+10h) whose unit ([icon+4]) is-a 46h,
// 00845750(unit), or is-a 45h, 006D2730(unit), equals `object`, returns the
// icon; 0 at the end.
void* hud_map_find_icon_carrying_unit_005a1220(void* screen, void* object, Seg20Host& host);

// 0059D6D0 __thiscall(screen, icon) RET 4 -> AL. p = 00AA6750(first widget of
// the icon's vector at +ECh); n = ((float)((+F8h + 60.0) / 960.0),
// (float)((60.0 + +FCh) / 720.0)); q = (p.x + n.x, n.y + p.y, p.z + 0.0) as
// floats. When 0059D410(p) and 0059D410(q) both hold: icon+120h = 0, AL = 1;
// otherwise icon+120h = 0059A910(icon, screen+54Ch), AL = 0.
bool hud_map_icon_update_offscreen_edge_0059d6d0(void* screen, void* icon, Seg20Host& host);

// 0059D940 __thiscall(screen, unit) RET 4 -> AL, z = screen+2Ch (zoom level):
// is-a 1Ch -> 1; in the local side's grey arrow ([00E188A8]+21A4h+side*4,
// 008DDF90) with z >= 1 -> 1; is-a 19h/1Bh/1Ah/0Fh -> z >= 2; is-a 18h -> z <
// 2; is-a 6 and a formation follower (007788B0) -> z >= 1; neither 2Bh nor
// 34h -> 1; else z >= 2 and 00521F10(unit+1E4h, [[00E188A8]+18CCh+side*4]+
// 28h) >= 2.
bool hud_map_is_unit_shown_at_zoom_0059d940(void* screen, void* unit, Seg20Host& host);

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

// --- cc13-x87: the x87-heavy bodies (docs/CC13_SEG20.md, "x87 bodies"). ---
// Map coordinates: x' = (x - screen+54Ch) / screen+30h + [00E08890],
// y' = (screen+550h - z) / screen+34h + [00E08894].

// 0059DD20 __thiscall(ECX arrow widget, screen, const float* from, const
// float* to, float width), RET 10h. Hides [+4] and the eight parts +8..+24h
// (vtable+34h(0)), [+4]->vtable+7Ch(), then seven map points through
// 00AC2DC0: with n = (to - from) on x/z scaled by 1/|to - from| (0 when the
// length is not positive), A = n.z*w, B = n.x*w, C = -n.x*w:
// (fx-B, fz-A), (fx-A, fz-C), (tx-A, tz-C), (tx+B, tz+A), (tx+A, tz+C),
// (fx+A, fz+C), (fx-B, fz-A). Then vtable+80h(), vtable+50h({0,1,0,1}),
// vtable+34h(1).
void hud_map_arrow_widget_set_line_0059dd20(void* widget, void* screen, const float* from,
                                            const float* to, float width, Seg20Host& host);

// 0059F7C0 __thiscall(ECX screen, const float* world, float* out, float*
// angle), RET 0Ch. out = (x', y', 110), *angle = 0; x' is clamped to
// [left + 1/48, right - 1/48] (left/right 0.05/0.95, or -0.1/1.1 when
// [[0109CF04]+0Dh] is set), y' to [0.0777778, 0.922222]; then *angle =
// acos(dot(n, (0, -1))) with the sign -1 when n.x > 0, else +1, where n is
// the unit vector from the clamped point to the unclamped one.
void hud_map_world_to_clamped_map_point_0059f7c0(void* screen, const float* world, float* out,
                                                 float* angle, Seg20Host& host);

// 00581DF0 (ledger name BSP_MainMenuScreen_ScaleWidget; no callers), one
// stack argument (widget), RET 4; ECX is not read. Draws the widget's bounds
// as a debug outline: p0 = parent (+70h) resolved position + local position
// - size scaled by (+20h*+18h, +24h*+1Ch), p1 = p0 + size, both passed through
// vtable+64h(&x0, &y0, &x1, &y1), then four 00554600 edges (top, left,
// bottom, right) in the colour {1, 0, 0, 1} at 00E194F0 (a static guarded by bit 0
// of [00E19500], initialised before each edge if needed).
void main_menu_draw_widget_outline_00581df0(void* widget, Seg20Host& host);

}  // namespace bsp::frontend_seg20_cc13
