#pragma once
// Segments 19 (0056D9A0-00580530) and 21 (005C57D0-005CCE70), cc13-seg19-21:
// front-end session browser and loading screen leftovers (19); mission tree
// groups, the HUD movie screens and the movie-camera debug screen (21).
// docs/CC13_SEG19_21.md has the evidence. Names are hypotheses, not recovered
// symbols. Objects are raw image storage (Raw::at at the image's offsets); every
// engine routine the image calls is a named host method invoked at the same
// point. The native __thiscall/__fastcall conventions are documented per
// routine, not reproduced. Nothing here runs in the game.
#include <cstddef>
#include <cstdint>

namespace bsp::frontend_seg19_21_cc13 {

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};
inline Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

// The eight-byte native string (dword length, dword buffer); see native_string.hpp.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

struct Seg19_21Host {
    virtual ~Seg19_21Host() = default;
    // Address of an image global's storage (00E198C4, 00E188A8, 00E194B4, ...).
    virtual void* global(std::uint32_t address) = 0;

    // --- native strings and the sized pool ---
    // 0041DD40 __thiscall(string, length, 1) RET 8.
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t length, std::int32_t flag) = 0;
    // 00419CC0 (the pool singleton; the three pushed words stay for the next call),
    // then 00BD1510 __thiscall(pool, block, size, 1) RET 0Ch.
    virtual void* pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, std::int32_t flag) = 0;

    // --- CRT ---
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;

    // --- singleton lifetime manager ---
    virtual void* singleton_manager_00415350() = 0;
    virtual void enter_critical_section(void* cs) = 0;   // [00CE2218]
    virtual void leave_critical_section(void* cs) = 0;   // [00CE2210]
    // 00BD0C30 / 00BCFCA0 __thiscall(manager, object) RET 4.
    virtual void singleton_register_00bd0c30(void* manager, void* object) = 0;
    virtual void singleton_unregister_00bcfca0(void* manager, void* object) = 0;

    // --- front-end screens and the GUI manager ---
    virtual void frontend_construct_004f7180(void* screen) = 0;
    virtual void frontend_destruct_004f71a0(void* screen) = 0;
    virtual void frontend_register_004f71d0(void* screen) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    // 00AA5840 __thiscall(manager, const NativeStr* name, 1, 0) RET 0Ch -> page.
    virtual std::uint32_t gui_load_page_00aa5840(void* manager, const NativeStr* name, std::int32_t a, std::int32_t b) = 0;
    // 00AA31F0 __thiscall(manager, page) RET 4.
    virtual void gui_remove_and_destroy_page_00aa31f0(void* manager, std::uint32_t page) = 0;
    // 00AA7E00 __thiscall(parent, const NativeStr* name, 1) RET 8 -> widget.
    virtual void* widget_find_child_00aa7e00(void* parent, const NativeStr* name, std::int32_t flag) = 0;
    // 004D6790 __thiscall(vector, const value*) RET 4: push_back.
    virtual void vector_push_back_004d6790(void* vector, const void* value) = 0;
    // Widget virtuals: +34h(int) show, +44h(float), +50h(const float rgba[4]).
    virtual void widget_vslot34(void* widget, std::int32_t value) = 0;
    virtual void widget_vslot44(void* widget, float value) = 0;
    virtual void widget_vslot50(void* widget, const float (&rgba)[4]) = 0;
    // 00AA8240 __thiscall(widget, const float xyz[3]) RET 4.
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float (&xyz)[3]) = 0;
    // 00ABAED0 __thiscall(text, const NativeStr* source, 1) RET 8.
    virtual void text_set_localised_source_00abaed0(void* text, const NativeStr* source, std::int32_t flag) = 0;
    // object->vtable[4](1) and object->vtable[0](1): deleting destructors.
    virtual void delete_vslot04(void* object, std::int32_t flags) = 0;
    // 006833F0 __fastcall(scroller); 00683610 __thiscall(scroller).
    virtual void scroller_recapture_006833f0(void* scroller) = 0;
    virtual void scroller_construct_00683610(void* scroller) = 0;

    // --- HUD movie screens ---
    // 00652B60 / 00611BF0 __thiscall on [[00E198C4]+BCh] / [[00E198C4]+C0h].
    virtual void hud_hide_info_widget_00652b60(void* screen) = 0;
    virtual void hud_hide_info_page_00611bf0(void* screen) = 0;
    // 0075B430 __thiscall(message, type) RET 4; 0076A9F0 __thiscall(game+1EF0h, message, 0) RET 8.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t type) = 0;
    virtual void session_dispatch_0076a9f0(void* sender, void* message, std::int32_t flag) = 0;
    // 00B0D0B0 __thiscall([00F8D39C], float) RET 4.
    virtual void render_set_value224_00b0d0b0(void* service, float value) = 0;
    // 00926D90 __thiscall(object, 1) RET 4; 00695870 __thiscall(owner).
    virtual void entity_kill_00926d90(void* object, std::int32_t flag) = 0;
    virtual void callback_owner_destroy_00695870(void* owner) = 0;

    // --- movie camera ---
    virtual void* movcam_construct_0079f480(void* block) = 0;   // __thiscall, returns this
    // 00923870 __thiscall(mover, 0, [game+19CCh], const float matrix[16]) RET 0Ch.
    virtual void movcam_place_00923870(void* mover, std::int32_t zero, std::uint32_t parent, const float (&matrix)[16]) = 0;
    // 004BC410 __thiscall(game, mover) RET 4.
    virtual void game_set_camera_mover_004bc410(void* game, void* mover) = 0;
    // 00694A60 __fastcall(ECX = subject, EDX = observer).
    virtual void observer_register_pair_00694a60(void* subject, void* observer) = 0;
    virtual void* input_manager_004bec00() = 0;
    // 00A933F0 __thiscall(manager, index, level) RET 8.
    virtual void input_set_context_level_00a933f0(void* manager, std::uint32_t index, std::uint32_t level) = 0;
    // 00B6FBB0 / 00B6FBF0 __thiscall(camera, float) RET 4.
    virtual void camera_set_fov_00b6fbb0(void* camera, float value) = 0;
    virtual void camera_set_near_plane_00b6fbf0(void* camera, float value) = 0;
    virtual void vehicle_camera_limits_init_0078e230(void* limits) = 0;

    // --- Lua reader (00B66BD0 construct, 00B6A020 open, 00B69D40 run, 00B67980 globals,
    //     00B67800 by name, 00B66270 number, 00B67700 destruct, 00B669A0 close) ---
    virtual void lua_owner_construct_00b66bd0(void* owner) = 0;
    virtual void lua_owner_open_00b6a020(void* owner, std::int32_t libs) = 0;
    virtual void lua_run_script_00b69d40(void* owner, const NativeStr* path, std::int32_t flag) = 0;
    virtual void lua_get_globals_00b67980(void* owner, void* out) = 0;
    virtual void lua_get_by_name_00b67800(void* table, void* out, const char* name) = 0;
    // 00B66270 returns ST0; the caller stores it as a float (FSTP float).
    virtual double lua_get_number_00b66270(void* object) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    virtual void lua_owner_close_00b669a0(void* owner) = 0;

    // --- mission tree containers ---
    virtual void mission_record_vector_tidy_005c9370(void* vector) = 0;
    virtual void mission_group_vector_tidy_005ca520(void* vector) = 0;
    // 005C93C0 / 005C9940 __thiscall(dst vector, const src vector*) RET 4.
    virtual void mission_record_vector_copy_005c93c0(void* dst, const void* src) = 0;
    virtual void mission_record_vector_assign_005c9940(void* dst, const void* src) = 0;

    // --- loading screen ---
    // 00506C80 __thiscall(dst string vector, const src*) RET 4.
    virtual void string_vector_assign_00506c80(void* dst, const void* src) = 0;

    // --- session browser ---
    // 0056F160 __thiscall(dst wstring, const src*, pos, count) RET 0Ch: assign.
    virtual void wstring_assign_0056f160(void* dst, const void* src, std::uint32_t pos, std::uint32_t count) = 0;
    // 00568930 __thiscall(screen).
    virtual void session_browser_layout_filter_rows_00568930(void* screen) = 0;
    // Trees: head-node buyers (__thiscall(tree) -> node), 0056A090 __thiscall(tree, root)
    // RET 4 erases a subtree, 005720E0 __thiscall(tree, out, first, last) erases a range.
    virtual void* tree_buy_head_node(std::uint32_t function, void* tree) = 0;
    virtual void tree_erase_subtree_0056a090(void* tree, void* root) = 0;
    virtual void tree_erase_range_005720e0(void* tree, void* out_iterator, void* first, void* last) = 0;
    // 005070C0 __thiscall(map, const NativeStr* key) RET 4 -> value reference.
    virtual void* case_insensitive_map_index_005070c0(void* map, const NativeStr* key) = 0;
};

// ---- native string helpers (the inlined 0041DD40 + memcpy build and the 005CC750 release) ----
NativeStr native_string_build(Seg19_21Host& host, const char* text, std::uint32_t length);
void native_string_release_storage_005cc750(NativeStr& s, Seg19_21Host& host);

// ---- segment 19: session browser ----
void* session_record_copy_construct_00570a30(void* self, const void* source, Seg19_21Host& host);
void* session_record_assign_00571a20(void* self, const void* source, Seg19_21Host& host);
void session_record_destroy_range_00571af0(void* first, void* last, Seg19_21Host& host);
void session_record_swap_005725a0(void* a, void* b, Seg19_21Host& host);
void multi_session_browser_destruct_00573a60(void* screen, Seg19_21Host& host);
std::int32_t multi_session_browser_get_screen_id_00573bb0() noexcept;
void multi_session_browser_listener_current_1c_00573bd0(void* listener, Seg19_21Host& host);
void multi_session_browser_collect_children_00573c00(void* screen, void* children, Seg19_21Host& host);
std::uint8_t session_filter_read_rank_setting_00574050(void* profile, Seg19_21Host& host);
void* multi_session_browser_construct_00574240(void* screen, Seg19_21Host& host);
void* multi_lobby_option_registry_construct_005769e0(void* self, Seg19_21Host& host);
void* multi_lobby_option_registry_get_singleton_00576b10(Seg19_21Host& host);

// ---- segment 19: loading screen ----
void loading_progress_compute_fraction_0057bf10(void* screen, std::int32_t phase, float fraction, Seg19_21Host& host);
void* loading_screen_register_singleton_0057bff0(void* base, Seg19_21Host& host);
void loading_screen_singleton_destruct_0057c090(void* base, Seg19_21Host& host);
void loading_progress_report_phase_0057c1a0(std::int32_t phase, float fraction, Seg19_21Host& host);
void* loading_screen_construct_0057c1c0(void* screen, Seg19_21Host& host);
std::int32_t loading_screen_get_screen_id_0057c230() noexcept;
void loading_screen_destruct_0057c2e0(void* screen, Seg19_21Host& host);
void loading_screen_collect_children_0057cf10(void* screen, void* children, Seg19_21Host& host);
void loading_screen_publish_config_0057cff0(const void* source, Seg19_21Host& host);
void loading_screen_set_mission_text_0057d060(const void* source, Seg19_21Host& host);
void main_menu_quit_confirm_callback_0057d3b0(std::int32_t result, Seg19_21Host& host);
void loading_screen_hide_overlays_0057d3f0(void* screen, Seg19_21Host& host);
bool game_is_loading_state_0057d430(Seg19_21Host& host);

// ---- segment 21: mission tree groups and screen ----
void* mission_group_copy_construct_005c9830(void* self, const void* source, Seg19_21Host& host);
void mission_group_clear_005c9b10(void* self) noexcept;
void mission_group_destroy_005c9b60(void* self, Seg19_21Host& host);
void* mission_group_assign_005c9cc0(void* self, const void* source, Seg19_21Host& host);
void mission_group_uninitialized_fill_005ca300(void* first, std::uint32_t count, const void* value, Seg19_21Host& host);
void* mission_tree_screen_construct_005ca880(void* screen, Seg19_21Host& host);
std::int32_t mission_tree_screen_get_screen_id_005ca8e0() noexcept;
void mission_tree_screen_destruct_005ca900(void* screen, Seg19_21Host& host);

// ---- segment 21: HUD movie screens ----
void hud_movie_camera_debug_update_noop_005caf60(void* screen, float seconds) noexcept;
void hud_movie_camera_debug_on_observed_released_005caf70(void* self, void* object) noexcept;
void movie_camera_set_deviation_factor_005caf90(void* screen, float value) noexcept;
void movie_camera_set_overview_height_005cafa0(void* screen, float value) noexcept;
void movie_camera_reset_deviation_factor_005cafb0(void* screen) noexcept;
void movie_camera_reset_overview_height_005cafc0(void* screen) noexcept;
void hud_movie_screen_destruct_005caff0(void* screen, Seg19_21Host& host);
std::int32_t hud_movie_screen_get_screen_id_005cb070() noexcept;
void hud_movie_screen_on_observed_released_005cb080(void* self, void* object) noexcept;
void* hud_engine_movie_screen_construct_005cb0a0(void* screen, Seg19_21Host& host);
std::int32_t hud_engine_movie_screen_get_screen_id_005cb0d0() noexcept;
void hud_engine_movie_screen_show_black_bars_005cb0f0(void* screen, Seg19_21Host& host);
void hud_engine_movie_screen_hide_black_bars_005cb130(void* screen) noexcept;
void hud_engine_movie_screen_show_black_bars_now_005cb160(void* screen, Seg19_21Host& host);
bool session_message52_is_type_005cb8b0(std::int32_t type) noexcept;
void* session_message52_scalar_delete_005cb8c0(void* message, std::uint8_t flags, Seg19_21Host& host);
void movie_camera_set_speed_limits_005cb900(void* screen, float linear, float angular) noexcept;
void movie_camera_set_accel_time_005cb940(void* screen, float seconds) noexcept;
void movie_camera_reset_speed_limits_005cb970(void* screen) noexcept;
void movie_camera_reset_accel_time_005cb9c0(void* screen) noexcept;
void movie_camera_set_linear_speed_limit_005cb9f0(void* screen, float value) noexcept;
void movie_camera_set_angular_speed_limit_005cba20(void* screen, float value) noexcept;
void movie_camera_show_dump_message_005cba50(void* screen, std::int32_t value, Seg19_21Host& host);
void hud_movie_screen_show_black_bars_005cbc30(void* screen, Seg19_21Host& host);
void hud_movie_screen_hide_black_bars_005cbcc0(void* screen, Seg19_21Host& host);
void hud_engine_movie_screen_update_black_bars_005cbd90(void* screen, float seconds, Seg19_21Host& host);
void movie_camera_debug_ensure_mover_005cc000(void* screen, Seg19_21Host& host);
void* hud_movie_screen_construct_005cc120(void* screen, Seg19_21Host& host);
void movie_camera_debug_enter_005cc590(Seg19_21Host& host);
void movie_camera_show_mode_text_005cc5e0(void* screen, NativeStr text, Seg19_21Host& host);
void* vehicle_camera_array_construct_005cc700(void* self, Seg19_21Host& host);
void movie_camera_debug_register_005cc770(void* screen, Seg19_21Host& host);
void movie_camera_debug_bind_widgets_005cca80(void* screen, Seg19_21Host& host);
void hud_movie_screen_register_005cccb0(void* screen, Seg19_21Host& host);

} // namespace bsp::frontend_seg19_21_cc13
