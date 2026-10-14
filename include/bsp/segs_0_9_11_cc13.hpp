#pragma once
// Segments 0 (00410630-00412FA0) and 9 (004BC410-004D0860), cc13-segA. Segment 11 is leased to
// agent/argon-v2 (argon-v2-seg11, 004E7E40-004F9D30) and is not touched here.
//
// Segment 0: the archive section handles (a ref-counted visitor wrapper and its writer twin),
// the time/freq pair and the bot-state registry's save/load, the bot-state pair vector's STL
// dispatch thunks, the property node pool page, a scoped critical-section guard and a few leaf
// helpers. Segment 9: GGame methods (camera mover, game mode, cutscene playback, generate object,
// border-zone side), the scene record list push_back and the scene record string blocks.
//
// docs/CC13_SEGS_0_9_11.md has the evidence. Names are hypotheses, not recovered symbols. Objects
// are raw image storage (Raw::at at the image's offsets); every engine routine and every virtual
// slot the image calls is a named host method invoked at the same point with the same arguments.
// The native __thiscall/__fastcall/register conventions are documented per routine, not
// reproduced. Nothing here runs in the game.
#include <cstddef>
#include <cstdint>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_land_state_registry.hpp"
#include "bsp/native_legacy_exception_owner.hpp"

namespace bsp::segs_0_9_11_cc13 {

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

struct Segs0911Host {
    virtual ~Segs0911Host() = default;
    // Address of an image global's storage (00E198C4, 00E19894, 00E17628, 00E18680).
    virtual void* global(std::uint32_t address) = 0;

    // --- CRT and Win32 ---
    virtual void invalid_parameter_00bf6713() = 0;          // checked-iterator / range report
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    virtual void enter_critical_section(void* cs) = 0;      // [00CE2218]
    virtual void leave_critical_section(void* cs) = 0;      // [00CE2210]
    // 00411860, the vector length error (_Xlen). The image never returns from it.
    virtual void vector_length_error_00411860() = 0;

    // --- the archive visitor (abstract base vtable 00CE374C, seven slots) and writer (00CE3784,
    // four slots). Each is a virtual call through the object's own vtable, ECX = object.
    virtual void visitor_enter_04(void* visitor, const GuiLuaVariant& key) = 0;           // RET 8
    virtual void visitor_leave_08(void* visitor) = 0;                                      // RET 0
    virtual bool visitor_read_10(void* visitor, const GuiLuaVariant& key,
                                 const GuiLuaVariant& field) = 0;                          // RET 10h
    virtual bool visitor_has_key_14(void* visitor, const GuiLuaVariant& key) = 0;          // RET 8
    virtual std::uint32_t visitor_enumerate_keys_18(void* visitor, GuiLuaKeyArray& keys) = 0;  // RET 4
    virtual void writer_begin_04(void* writer, const GuiLuaVariant& key) = 0;             // RET 8
    virtual void writer_end_08(void* writer) = 0;                                          // RET 0
    virtual void writer_write_0c(void* writer, const GuiLuaVariant& key,
                                 const GuiLuaVariant& value) = 0;                          // RET 10h
    // Vtable +4h with no stack arguments (the bot state's enter slot; 004108A0's attach target).
    virtual void call_vslot_04(void* object) = 0;

    // --- segment 9 callees ---
    // Camera mover vtable +124h, __thiscall(mover, previous mover) RET 4.
    virtual void mover_link_previous_124(void* mover, void* previous) = 0;
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t reason) = 0;     // ECX, RET 4
    virtual void scene_node_enable_00922f30(void* node, std::int32_t enable) = 0;         // ECX, RET 4
    virtual void camera_mover_set_camera_node_0042a920(void* mover, void* node) = 0;      // ECX, RET 4
    virtual void weather_apply_camera_shake_00445790(void* weather, void* mover) = 0;     // ECX, RET 4
    virtual void clear_all_unit_lists_004bfdf0(void* game) = 0;                           // ECX
    virtual void lua_mw_run_init_script_once_006f7670(void* object) = 0;                  // ECX
    virtual void bit_write_byte_00428ff0(void* cursor, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_write_signed_dword_00429090(void* cursor, std::uint32_t value,
                                                 std::uint32_t bits) = 0;
    virtual void bit_write_bool_004290b0(void* cursor, std::uint32_t value) = 0;
    virtual void bit_write_qword_00429180(void* cursor, std::uint32_t low, std::uint32_t high) = 0;
    // 0041DD40 __thiscall(string, length, 0) RET 8.
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t length,
                                               std::int32_t flag) = 0;
    // 00419CC0 (the sized pool singleton), then 00BD1510 __thiscall(pool, block, size, 1) RET 0Ch.
    virtual void* pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
                                            std::int32_t flag) = 0;
    virtual void* menu_command_screen_00425d10() = 0;                                     // cdecl
    virtual void screen_set_byte_3c_00530630(void* screen, std::uint8_t value) = 0;       // ECX, RET 4
    virtual bool input_action_was_pressed_004c43c0(void* game, std::int32_t action) = 0;  // ECX, RET 4
    virtual void engine_movie_stop_0045f600(void* movie) = 0;                             // ECX
    virtual void cutscene_player_ptr_delete_004bea40(void** slot) = 0;                    // ECX = slot
    virtual void engine_movie_advance_0045f970(void* movie, float delta) = 0;             // ECX, RET 4
    // 0046DC10 __thiscall(scene database [00E18680], a, b, c, float d) RET 10h.
    virtual void* scene_create_hidden_entity_0046dc10(void* scene_db, std::uint32_t a,
                                                      std::uint32_t b, std::uint32_t c,
                                                      float d) = 0;
    virtual void game_assign_party_player_slots_004c3840(void* game, std::int32_t arg) = 0;  // RET 4
};

// ===================================================================================================
// Segment 0: archive section handles. A 12-byte handle {vtable, int* count, visitor*} derived from
// the visitor base 00CE374C. 00BD4EB0 (argon-v2's lease, not reconstructed here) builds it:
// count = operator new(4) = 1 and visitor->enter(key). The last handle to go calls leave.
// ===================================================================================================

// 00410630 __thiscall(handle) RET 0 (callee-destroyed by-value argument). Stamps 00CE3768, drops
// the count; at zero frees the count and calls visitor +8h; leaves base vtable 00CE374C.
void archive_reader_ref_destroy_00410630(Segs0911Host& host, Raw handle);
// 004107B0, the writer twin: vtables 00CE3794 then 00CE3784; writer +8h at zero.
void archive_writer_ref_destroy_004107b0(Segs0911Host& host, Raw handle);

// The reader handle's vtable 00CE3768 slots +4h..+18h, each forwarding to [handle+8].
void archive_reader_ref_enter_004106a0(Segs0911Host& host, Raw handle, const GuiLuaVariant& key);
void archive_reader_ref_leave_004106c0(Segs0911Host& host, Raw handle);
bool archive_reader_ref_read_or_default_00410700(Segs0911Host& host, Raw handle,
                                                 const GuiLuaVariant& key,
                                                 const GuiLuaVariant& field,
                                                 const GuiLuaVariant& fallback);
bool archive_reader_ref_read_004106d0(Segs0911Host& host, Raw handle, const GuiLuaVariant& key,
                                      const GuiLuaVariant& field);
bool archive_reader_ref_has_key_00410730(Segs0911Host& host, Raw handle, const GuiLuaVariant& key);
std::uint32_t archive_reader_ref_enumerate_keys_00410750(Segs0911Host& host, Raw handle,
                                                         GuiLuaKeyArray& keys);
// The writer handle's vtable 00CE3794 slots +4h..+0Ch.
void archive_writer_ref_begin_00410820(Segs0911Host& host, Raw handle, const GuiLuaVariant& key);
void archive_writer_ref_end_00410840(Segs0911Host& host, Raw handle);
void archive_writer_ref_write_00410850(Segs0911Host& host, Raw handle, const GuiLuaVariant& key,
                                       const GuiLuaVariant& value);
// 004108A0 __thiscall(self, object) RET 4: self+4 = object, then object vtable +4h. No callers.
void attach_and_call_slot4_004108a0(Segs0911Host& host, Raw self, void* object);

// 00410D80 / 00410E00 __thiscall(pair, 12-byte handle by value) RET 0Ch. pair = {float +0 under
// the key "freq" (00CE37B0), float +4 under "time" (00CE37A8)}. The calls go through the
// handle's own vtable (+10h read, +0Ch write); the handle is destroyed on the way out.
void time_freq_pair_read_00410d80(Segs0911Host& host, float* pair, Raw handle);
void time_freq_pair_write_00410e00(Segs0911Host& host, const float* pair, Raw handle);

// ===================================================================================================
// Segment 0: the bot-state registry (NativeBotStateRegistryStorage: vector begin at +8h, end +0Ch)
// ===================================================================================================

// 00411170 __thiscall(registry, machine, reader*) RET 8. Reads "stateIndex" (00CE37B8) as an Int,
// checks it against the vector, stores the state at machine+4h and calls state vtable +4h.
void bot_state_registry_read_current_00411170(Segs0911Host& host,
                                              NativeBotStateRegistryStorage& registry,
                                              Raw machine, void* reader);
// 00411350 __thiscall(registry, state) RET 4. Always returns -1 (see the cpp).
std::int32_t bot_state_registry_index_of_00411350(Segs0911Host& host,
                                                  NativeBotStateRegistryStorage& registry,
                                                  const void* state);
// 004113C0 __thiscall(registry, state) RET 4: name of the first entry holding state, else null.
const char* bot_state_registry_find_name_004113c0(Segs0911Host& host,
                                                  NativeBotStateRegistryStorage& registry,
                                                  const void* state);
// 00411430 __thiscall(registry, state) RET 4: 004113C0 or "undefined" (00CE37C4).
const char* bot_state_registry_name_or_undefined_00411430(Segs0911Host& host,
                                                         NativeBotStateRegistryStorage& registry,
                                                         const void* state);
// 00411450 __thiscall(registry, machine, writer*) RET 8: writes "stateIndex" = 00411350(machine+4).
void bot_state_registry_write_current_00411450(Segs0911Host& host,
                                               NativeBotStateRegistryStorage& registry,
                                               Raw machine, void* writer);

// ===================================================================================================
// Segment 0: bot-state pair vector STL thunks (they all reach the existing 00410400/00410C00/00410FE0)
// ===================================================================================================

// 004109C0 __stdcall(count) RET 4 -> 00410400(ECX = count, EDX = 0).
NativeBotStatePair* allocate_pairs_thunk_004109c0(std::uint32_t count);
// 00411140 / 004115E0 (ECX first, EDX last, stack dest, unused) RET 8 and 00411320 (same, two more
// unused stack words) RET 10h -> 00410C00(first, last, dest); EAX is 00410C00's destination end.
NativeBotStatePair* copy_pairs_dispatch_00411140(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest);
NativeBotStatePair* copy_pairs_dispatch_00411320(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest);
NativeBotStatePair* copy_pairs_dispatch_004115e0(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest);
// 004115B0 (ECX dest, stack first{container, ptr}, last{container, ptr}) RET 10h and 00411640
// (stack first{container, ptr}, last{container, ptr}, dest) RET 14h: the pointers only.
NativeBotStatePair* copy_pairs_dispatch_004115b0(NativeBotStatePair* dest,
                                                 const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last);
NativeBotStatePair* copy_pairs_dispatch_00411640(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest);
// 004112B0 (ECX dest, EDX count, stack pair*, unused) RET 8 -> 00410FE0(dest, count, pair).
void fill_pairs_dispatch_004112b0(NativeBotStatePair* dest, std::uint32_t count,
                                  const NativeBotStatePair& pair);
// 00410A30 __stdcall(dest, src) RET 8: copies the eight bytes when dest is not null.
void construct_pair_at_00410a30(NativeBotStatePair* dest, const NativeBotStatePair* src);
// 00410B50 / 00411010 __cdecl(first, last, dest_end): backward copy, returns the new dest begin.
NativeBotStatePair* copy_pairs_backward_00410b50(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest_end);
NativeBotStatePair* copy_pairs_backward_00411010(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest_end);
// 00411D30 __thiscall(vector, n) RET 4 (vector::_Buy): false for n == 0, else 00411860 past
// 1FFFFFFFh, allocates n pairs and returns true.
bool vector_buy_00411d30(Segs0911Host& host, NativeBotStateVectorStorage& vector, std::uint32_t n);
// 004114A0 __thiscall(vector {+4 first, +8 last, +0Ch end}): frees first, zeroes all three.
void pointer_vector_tidy_004114a0(Segs0911Host& host, Raw vector);
// 00411760 __thiscall(storage, message) RET 4: 00411700, then the length_error vtable 00D69260.
NativeLegacyExceptionStorage& construct_length_error_00411760(
    NativeLegacyExceptionStorage& storage, const NativeLegacySboStringStorage& message);

// ===================================================================================================
// Segment 0: leaves
// ===================================================================================================

// Property node pool page: 64 slots of 14h bytes, a free-index array at +500h, count word +580h.
void node_page_fill_slot_field_10_00410af0(Raw page, std::uint32_t value);   // ECX, RET 4
bool node_page_count_is_64_00410b10(Raw page);                               // ECX
bool node_page_count_is_zero_00410b20(Raw page);                             // ECX
// 00411EB0 __thiscall(guard, owner) RET 4 and its scalar deleting destructor 00411F00 (vtable
// 00CE37FC). owner is a CRITICAL_SECTION followed by a dword depth at +18h.
void* scoped_lock_construct_00411eb0(Segs0911Host& host, Raw guard, void* owner);
void* scoped_lock_scalar_delete_00411f00(Segs0911Host& host, Raw guard, std::uint32_t flags);
void float_record5_defaults_00411f40(float* record);                         // ECX
void float_record4_defaults_00411f80(float* record);                         // ECX
std::int32_t float_equals_00411ff0(const float* self, float value);          // ECX, RET 4
void* flagged_global_if_set_00412270(Segs0911Host& host, Raw self);          // ECX
void list_iterator_next_004122a0(void** iterator);                           // ECX
std::int32_t c_string_length_00412500(const char* text);                     // ECX

// ===================================================================================================
// Segment 9: GGame methods and scene records
// ===================================================================================================

void game_set_camera_mover_004bc410(Segs0911Host& host, Raw game, void* mover);   // RET 4
void game_set_game_mode_004bc890(Raw game, std::int32_t mode, std::uint8_t forced);  // RET 8
void* game_singleton_construct_base_004bf9a0(Segs0911Host& host, Raw self);      // ECX
void scene_record_list_push_back_004c2c80(Segs0911Host& host, Raw list, void* payload);  // RET 4
void game_reset_local_player_units_004c3c90(Segs0911Host& host, Raw game);       // ECX
void session_message_e4_write_004c60e0(Segs0911Host& host, Raw message, void* cursor);  // RET 4
void* scene_texture_layer_construct_a_004c62a0(Segs0911Host& host, Raw layer);   // ECX
void* scene_texture_layer_construct_b_004c63e0(Segs0911Host& host, Raw layer);   // ECX
void scene_record_header_destruct_004c6520(Segs0911Host& host, Raw header);      // ECX
void game_update_cutscene_playback_004c6b20(Segs0911Host& host, Raw game, float delta);  // RET 4
void* game_generate_object_004c6be0(Segs0911Host& host, Raw game, std::uint32_t a,
                                    std::uint32_t b, std::uint32_t c, float d);   // RET 10h
void world_set_border_zone_side_004c7150(Segs0911Host& host, Raw world, std::int32_t edge,
                                         std::int32_t index, std::uint32_t side);  // RET 0Ch

} // namespace bsp::segs_0_9_11_cc13
