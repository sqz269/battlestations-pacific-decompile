#pragma once
// Packet cc13-left61: the leftover functions of segment 61 (0092E0B0..00951F20)
// and segment 1 (00413D10..0042A830) that the first cc13 wave skipped.
// docs/CC13_LEFT61.md lists every function with its ABI, uncertainty, and the
// ones skipped with the reason.
//
// Objects are reached through Raw (byte offsets into the Win32 layout). Every
// call the image makes goes through a Left61Host method named for its target.
// The MSVC9 container helpers that exist in several identical instantiations
// take the callee address as their first argument (`target`), so one hook
// serves every instantiation; the address is always the image's call target.
// Descriptive names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_left61 {

static_assert(sizeof(void*) == 4, "the left61 objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
    std::uint8_t* field(std::size_t off) const noexcept { return p + off; }
};

// An MSVC9 std::vector: +0 allocator (empty), +4 first, +8 last, +0Ch end.
constexpr std::size_t kVecFirst = 4;
constexpr std::size_t kVecLast = 8;
constexpr std::size_t kVecEnd = 0x0c;

struct Left61Host {
    virtual ~Left61Host() = default;

    // --- CRT ------------------------------------------------------------
    // 00BF65AC operator delete(void*).
    virtual void operator_delete_00bf65ac(void* block) = 0;
    // 00BF6885 _CxxThrowException(0, 0): the rethrow at the end of a catch block.
    // The image never returns from it; the reconstruction rethrows itself after the hook.
    virtual void cxx_rethrow_00bf6885() = 0;

    // --- math -----------------------------------------------------------
    // 004142E0, ECX = source xyz; stack = destination xyz, matrix; RET 8.
    virtual void transform_affine_point_004142e0(const float* source, float* destination,
                                                 const float* matrix) = 0;
    // 0042D0D0, ECX = out xyz, EDX = vector; stack = matrix, char normalize; RET 8.
    virtual void transform_direction_0042d0d0(float* out, const float* vector, const float* matrix,
                                              char normalize) = 0;

    // --- MSVC9 vector helpers (target = the image's call target) ----------
    // _Buy's allocator: __fastcall(ECX = count, EDX = 0 hint), EAX = storage
    // (0092C810, 0092C870, 0092C750, 0092C7B0).
    virtual void* vector_allocate(std::uint32_t target, std::uint32_t count) = 0;
    // _Xlen: throws length_error (009302A0, 00930310, 009301C0, 00930230).
    virtual void vector_xlen(std::uint32_t target) = 0;
    // size(), __thiscall(ECX = vector), EAX = count (0092C060, 0092C0C0).
    virtual std::uint32_t vector_size(std::uint32_t target, Raw vector) = 0;
    // clear(), __thiscall(ECX = vector) (0092FF80, 0092FFC0).
    virtual void vector_clear(std::uint32_t target, Raw vector) = 0;
    // copy(first, last, dest), __cdecl, EAX = end of the copy (0092F010, 0092F080).
    virtual void* range_copy(std::uint32_t target, void* first, void* last, void* dest) = 0;
    // _Ucopy(first, last, dest), __thiscall(ECX = vector), RET 0Ch, EAX = end (0092F8D0, 0092F900).
    virtual void* vector_ucopy(std::uint32_t target, Raw vector, void* first, void* last,
                               void* dest) = 0;
    // vector copy constructor, __thiscall(ECX = memory, const vector* source), RET 4
    // (00930CE0, 00930DD0).
    virtual void vector_copy_construct(std::uint32_t target, void* memory, const void* source) = 0;
    // allocator::destroy(p), __thiscall(ECX = allocator, p), RET 4: the inner vector's
    // _Tidy (00930EC0, 00930EF0).
    virtual void allocator_destroy(std::uint32_t target, void* allocator, void* element) = 0;
    // The nested-vector _Insert_n helpers (see insert_n_nested below):
    // _Umove into raw storage, __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h
    // (00931CB0, 00931DE0); EAX = end.
    virtual void* nested_uninit_move(std::uint32_t target, void* first, void* last, void* dest,
                                     void* allocator) = 0;
    // _Ufill(ptr, count, const val*), __thiscall(ECX = vector), RET 0Ch, EAX = end (009320D0, 00932150).
    virtual void* nested_ufill(std::uint32_t target, Raw vector, void* ptr, std::uint32_t count,
                               const void* value) = 0;
    // _Umove(first, last, dest), __thiscall(ECX = vector), RET 0Ch, EAX = end (00932190, 00932200).
    virtual void* nested_umove(std::uint32_t target, Raw vector, void* first, void* last, void* dest) = 0;
    // _Destroy_range, __fastcall(ECX = first, EDX = last; allocator, tag), RET 8 (009316F0, 00931760).
    virtual void nested_destroy_range(std::uint32_t target, void* first, void* last, void* allocator) = 0;
    // vector::_Destroy(first, last), __thiscall(ECX = vector), RET 8 (00931F10, 00931F30).
    virtual void nested_destroy(std::uint32_t target, Raw vector, void* first, void* last) = 0;
    // fill(first, last, const val*), __cdecl (00931AF0, 00931B40).
    virtual void nested_fill(std::uint32_t target, void* first, void* last, const void* value) = 0;
    // _Move_backward(first, last, dest), __cdecl (009321C0, 00932230).
    virtual void nested_move_backward(std::uint32_t target, void* first, void* last, void* dest) = 0;
    // --- native strings {+0 length, +4 data} ------------------------------
    // 00419CC0 returns the sized string pool; 00BD1510 (ECX = pool) takes a block back
    // (block, bytes, count).
    virtual void* string_pool_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                             std::int32_t count) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string; size, flag), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t size, std::int32_t flag) = 0;

    // --- engine objects --------------------------------------------------
    // Image dword globals.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
    // InterlockedDecrement through the import 00CE2220.
    virtual long interlocked_decrement_ce2220(volatile long* value) = 0;
    // InterlockedIncrement through the import 00CE221C.
    virtual long interlocked_increment_ce221c(volatile long* value) = 0;
    // Vtable slot 0 called with ECX = object and no arguments (the reference release).
    virtual void object_release_slot0(void* object) = 0;
    // 0092E5B0 BSP_UnitController_ApplyNetworkCorrection, __thiscall(controller, char), RET 4.
    virtual void unit_controller_network_correction_0092e5b0(Raw controller, char advance) = 0;
    // 00B6DFA0 BSP_Node_UnlinkAndRelease (ECX = node); 00B72250 BSP_NativeScene_Release (ECX = scene).
    virtual void node_unlink_release_00b6dfa0(void* node) = 0;
    virtual void native_scene_release_00b72250(void* scene) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(ECX = string; const char*), RET 4.
    virtual void native_string_assign_0041e870(void* string, const char* text) = 0;
    // 004C11F0 BSP_RenderCommandQueue_GetSingleton, EAX = queue.
    virtual void* render_queue_singleton_004c11f0() = 0;
    // 00B0D0D0, __thiscall(ECX = object), EAX = value (a trivial accessor).
    virtual std::uint32_t render_targets_00b0d0d0(void* object) = 0;
    // 00B1F4D0 BSP_RenderCommandQueue_SubmitScene, __thiscall(queue; scene, camera, targets,
    // NativeString label by value {length, data}), RET 14h; the callee frees the label.
    virtual void render_queue_submit_scene_00b1f4d0(void* queue, void* scene, void* camera,
                                                    std::uint32_t targets, const std::uint32_t (&label)[2]) = 0;
    // 00860030 BSP_DebugDraw_ScreenLineRgbaFloats, __thiscall(ECX = float rgba[4]; x0, y0, x1, y1), RET 10h.
    virtual void debug_screen_line_00860030(const float* rgba, float x0, float y0, float x1, float y1) = 0;
    // 00419260 BSP_Vector2f_ReciprocalLength, ECX = const float v[2], ST0 = 1 / |v|.
    virtual float vector2_reciprocal_length_00419260(const float* v) = 0;
    // 0093E9A0 BSP_MapOverlay_DrawEllipse, __cdecl(const float centre[2], const float radius[2]).
    virtual void map_overlay_ellipse_0093e9a0(const float* centre, const float* radius) = 0;
    // 0093EEE0 BSP_Math_RoundToStep, __stdcall(value, step), RET 8, ST0 = result.
    virtual float round_to_step_0093eee0(float value, float step) = 0;
    // Vtable +1Ch called with ECX = object and no arguments, ST0 = result.
    virtual float object_float_slot1c(void* object) = 0;
    // Vtable +3Ch called with ECX = object and no arguments.
    virtual void object_call_slot3c(void* object) = 0;
    // 0093F590, __thiscall(ECX = 0Ch-element array; capacity), RET 4: reserve.
    virtual void array12_reserve_0093f590(Raw array, std::int32_t capacity) = 0;
    // 0093FDF0 BSP_SoundSampleOptions_CopyConstruct, __thiscall(ECX = memory; const options*), RET 4.
    virtual void sound_options_copy_construct_0093fdf0(void* memory, const void* source) = 0;
    // 00A83120 BSP_SoundSample_ApplySettings, __thiscall(ECX = sample; 48h-byte options by value),
    // RET 48h; `options` is the by-value copy the callee owns and destroys.
    virtual void sound_sample_apply_settings_00a83120(void* sample, void* options) = 0;
    // 0093F950 BSP_SoundSampleParameters_Resize, __thiscall(ECX = parameter array; count), RET 4.
    virtual void sound_parameters_resize_0093f950(void* parameters, std::int32_t count) = 0;
    // 00BF6989 _free.
    virtual void free_00bf6989(void* block) = 0;

    // 00BF681B operator new(bytes).
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 008F41A0 BSP_ScenePropertyBag_Construct, __thiscall(ECX = memory; int), RET 4, EAX = bag.
    virtual void* property_bag_construct_008f41a0(void* memory, std::int32_t arg) = 0;
    // 008F3710 BSP_ScenePropertyBag_AddIntProperty, __thiscall(ECX = bag; const char* key, int), RET 8.
    virtual void property_bag_add_int_008f3710(void* bag, const char* key, std::int32_t value) = 0;
    // 008F2260 BSP_ScenePropertyBag_Find, __thiscall(ECX = bag; const char* key), RET 4, EAX = property.
    virtual void* property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 008F0DF0 BSP_SceneProperty_GetStringValue, __thiscall(ECX = property), EAX = text.
    virtual const char* property_string_value_008f0df0(void* property) = 0;
    // 0043D8F0 BSP_LuaTable_CopyIntoPropertyBag, __fastcall(ECX = Lua table, EDX = bag).
    virtual void lua_table_copy_into_bag_0043d8f0(void* table, void* bag) = 0;
    // 00964790 BSP_VehicleClass_GetOrCreate, __fastcall(ECX = value, DL = flag), EAX = class.
    virtual void* vehicle_class_get_or_create_00964790(std::uint32_t value, char flag) = 0;

    // 00943B90, ECX = list (unused): list::_Buyheadnode, EAX = a new 0Ch-byte head node.
    virtual void* spawn_list_buy_head_node_00943b90(Raw list) = 0;
    // 00660540 BSP_IntStringIntRecord_CopyConstruct, __thiscall(ECX = memory; const record*), RET 4.
    virtual void record_copy_construct_00660540(void* memory, const void* source) = 0;

    // 00BF6713 _invalid_parameter_noinfo: the checked-iterator failure report.
    virtual void invalid_parameter_00bf6713() = 0;
    // 0048E9F0 BSP_ScenePropertyBag_Contains, __thiscall(ECX = bag; const char* key), RET 4, AL.
    virtual bool property_bag_contains_0048e9f0(void* bag, const char* key) = 0;
    // Vtable +5Ch called with ECX = unit and one argument (the kind test), AL = result.
    virtual bool unit_is_kind_slot5c(void* unit, std::int32_t kind) = 0;
    // 007B8AD0 BSP_Unit_IsSquadronFlightLeader, ECX = unit, AL.
    virtual bool unit_is_squadron_flight_leader_007b8ad0(void* unit) = 0;
    // 009FFD20 BSP_Unit_LossCountingSlot, ECX = unit, EAX = player slot.
    virtual std::int32_t unit_loss_counting_slot_009ffd20(void* unit) = 0;
    // 00946970, __thiscall(ECX = spawn manager; player), RET 4, ST0 = the player's queued
    // resource usage.
    virtual float spawn_manager_player_usage_00946970(void* manager, std::int32_t player) = 0;

    // Lua objects and the table reader. A LuaObject is 14h bytes (passed by value).
    // 00B65FB0 BSP_LuaObject_IsNil, ECX = object, AL.
    virtual bool lua_object_is_nil_00b65fb0(void* object) = 0;
    // 00B66FA0 BSP_LuaObject_CopyConstruct, __thiscall(ECX = memory; const object*), RET 4.
    virtual void lua_object_copy_construct_00b66fa0(void* memory, const void* source) = 0;
    // 00B67700 BSP_LuaObject_Destruct, ECX = object.
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 004425C0 BSP_LuaReader_Construct, __thiscall(ECX = reader; LuaObject by value), RET 14h;
    // `object` is the by-value copy the callee owns.
    virtual void lua_reader_construct_004425c0(void* reader, void* object) = 0;
    // 00BD68D0 BSP_LuaReader_ReadFieldOrDefault, __thiscall(ECX = reader; key variant,
    // destination variant, default variant (8 bytes each, by value)), RET 18h.
    virtual void lua_reader_read_field_or_default_00bd68d0(void* reader, const std::uint32_t (&key)[2],
                                                           const std::uint32_t (&destination)[2],
                                                           const std::uint32_t (&fallback)[2]) = 0;
    // 00441A20 BSP_LuaReader_DestroyStackInstance, ECX = reader.
    virtual void lua_reader_destroy_00441a20(void* reader) = 0;

    // 00695870 BSP_CallbackOwner_Destroy, ECX = callback owner.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 00946D70, __fastcall(ECX = first, EDX = last; allocator, tag), RET 8: destroy the
    // member range.
    virtual void member_destroy_range_00946d70(void* first, void* last, void* allocator) = 0;
    // 009442A0, ECX = spawn request: the pre-delete pass over the request's members.
    virtual void spawn_request_release_members_009442a0(void* request) = 0;

    // 00947BC0, __thiscall(ECX = message (5Ch bytes); 7 dwords), RET 1Ch: the session
    // message 42h (spawn request) constructor.
    virtual void spawn_message_construct_00947bc0(void* message, const std::uint32_t (&args)[7]) = 0;
    // 0094B600, __thiscall(ECX = spawn manager; const message*, dword), RET 8: the local
    // handler of message 42h.
    virtual void spawn_manager_handle_message_0094b600(void* manager, void* message, std::uint32_t arg) = 0;
    // 00770AF0 BSP_Session_SendMessageToSecondaryFirstPeer, __thiscall(ECX = session; message*), RET 4.
    virtual void session_send_to_secondary_00770af0(void* session, void* message) = 0;
    // 00766DA0 BSP_SessionMessage66_Destroy, ECX = message.
    virtual void session_message_destroy_00766da0(void* message) = 0;
    // Vtable slot 0 with argument 1 (the scalar deleting destructor), ECX = object.
    virtual void object_delete_slot0(void* object) = 0;

    // Archive reader/writer vtable calls (ECX = archive). Variants are {tag, value} by value.
    // +4 open(variant section), RET 8.
    virtual void archive_open_slot04(void* archive, const std::uint32_t (&section)[2]) = 0;
    // +8 close().
    virtual void archive_close_slot08(void* archive) = 0;
    // +0Ch write(int, int index, variant value), RET 10h.
    virtual void archive_write_slot0c(void* archive, std::int32_t kind, std::int32_t index,
                                      const std::uint32_t (&value)[2]) = 0;
    // +10h read(int, int index, variant destination), RET 10h.
    virtual void archive_read_slot10(void* archive, std::int32_t kind, std::int32_t index,
                                     const std::uint32_t (&destination)[2]) = 0;
    // +14h has(variant key), RET 8, AL.
    virtual bool archive_has_slot14(void* archive, const std::uint32_t (&key)[2]) = 0;
    // 00951720 BSP_EntityIdTable_RebuildFreeList, ECX = table.
    virtual void entity_id_table_rebuild_free_list_00951720(void* table) = 0;

    // 00947FD0 / 00442190: vector push_back, __thiscall(ECX = vector; const value*), RET 4.
    virtual void vector_push_back_00947fd0(void* vector, const void* value) = 0;
    virtual void vector_push_back_00442190(void* vector, const void* value) = 0;
    // 00408720 std::string assign(const char*, count), __thiscall(ECX = string), RET 8.
    virtual void std_string_assign_00408720(void* string, const char* text, std::uint32_t count) = 0;
    // 00849170 vector<std::string>::push_back, __thiscall(ECX = vector; const string*), RET 4.
    virtual void string_vector_push_back_00849170(void* vector, const void* string) = 0;
    // 0049F9B0, ECX = object: the base constructor 009510F0 runs first.
    virtual void base_construct_0049f9b0(void* object) = 0;
    // 00951160 BSP_TrafficRecordVector_Clear, ECX = vector.
    virtual void traffic_record_vector_clear_00951160(void* vector) = 0;

    // Effect entities.
    // 00867B10 BSP_PointEffect_StopChildren, ECX = effect.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // 009287B0 BSP_GameEntity_Destruct, ECX = entity.
    virtual void game_entity_destruct_009287b0(void* entity) = 0;
    // 009277E0 BSP_Entity_InheritRaceAndActivateThunk, ECX = entity.
    virtual void entity_activate_009277e0(void* entity) = 0;
    // 00870CD0 BSP_EffectHandle_Acquire, __fastcall(ECX = out handle, EDX = value; flag),
    // RET 4, EAX = out (the handle holds one reference).
    virtual void** effect_handle_acquire_00870cd0(void** out, std::uint32_t value, std::int32_t flag) = 0;
    // 00868420 BSP_PointEffect_CreateFromMatrix, __fastcall(ECX = out handle, EDX = kind;
    // definition handle by value (the callee owns that reference), const float* matrix,
    // option, tail), RET 10h, EAX = out.
    virtual void** point_effect_create_00868420(void** out, std::uint32_t kind, void* definition,
                                                const float* matrix, std::uint32_t option,
                                                std::uint32_t tail) = 0;
    // 004845D0, __thiscall(ECX = handle slot; const handle*), RET 4: counted assignment.
    virtual void handle_assign_004845d0(void* slot, void* const* source) = 0;

    // Segment 1.
    // 00415350, EAX = the singleton lifetime manager (critical section pointer at +10h).
    virtual void* singleton_manager_00415350() = 0;
    // EnterCriticalSection / LeaveCriticalSection through the imports 00CE2218 / 00CE2210.
    virtual void enter_critical_section_ce2218(void* section) = 0;
    virtual void leave_critical_section_ce2210(void* section) = 0;
    // 00424A10, ECX = memory: the 76Ch-byte gameplay settings constructor, EAX = object.
    virtual void* game_settings_construct_00424a10(void* memory) = 0;
    // 00BD0C30, __thiscall(ECX = manager; instance), RET 4: register a singleton.
    virtual void singleton_register_00bd0c30(void* manager, void* instance) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, ECX = pose owner.
    virtual void entity_pose_refresh_00414db0(void* entity) = 0;
    // 00BF55BE operator new(bytes) (the second entry point).
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;

    // 00931530 resize(count, value), __thiscall(ECX = vector; count, 14h-byte value by value), RET 18h.
    virtual void debris_entry_vector_resize_00931530(Raw vector, std::uint32_t count,
                                                     const std::uint32_t (&value)[5]) = 0;
};

// --- segment 61: unit parts and MSVC9 vector instantiations --------------

// 0092E250, __thiscall(ECX = const float box[6] (min xyz, max xyz); float out[6],
// const float matrix[12]), RET 8: the box transformed by an affine matrix as
// centre and |matrix| * half extent.
void transform_box_0092e250(const float* box, float* out, const float* matrix, Left61Host& host);

// 0092FD50, __fastcall(ECX = element): the two-vector element destructor
// (vectors at +0 and +10h; +14h storage freed first).
void two_vector_element_destroy_0092fd50(Raw element, Left61Host& host);

// 00930520 / 00930570, __thiscall(ECX = vector; count), RET 4, AL = bought:
// vector<14h-byte element>::_Buy.
bool vector20_buy(Raw vector, std::uint32_t count, std::uint32_t xlen_target,
                  std::uint32_t allocate_target, Left61Host& host);
bool vector20_buy_00930520(Raw vector, std::uint32_t count, Left61Host& host);
bool vector20_buy_00930570(Raw vector, std::uint32_t count, Left61Host& host);

// 00930F20 / 00931070, __thiscall(ECX = this; const vector* right), RET 4, EAX = this:
// vector<14h-byte element>::operator=.
struct Vector20Targets {
    std::uint32_t clear, copy, size, ucopy;
    bool (*buy)(Raw, std::uint32_t, Left61Host&);
};
Raw vector20_assign(Raw self, Raw right, const Vector20Targets& t, Left61Host& host);
Raw vector20_assign_00930f20(Raw self, Raw right, Left61Host& host);
Raw vector20_assign_00931070(Raw self, Raw right, Left61Host& host);

// 009311C0 / 00931200, __fastcall(ECX = memory, EDX = const vector* source), RET:
// _Construct (placement copy when memory is not null).
void construct_vector_009311c0(void* memory, const void* source, Left61Host& host);
void construct_vector_00931200(void* memory, const void* source, Left61Host& host);
// 009317D0 / 00931820, __thiscall(ECX = allocator (unused); memory, const vector*), RET 8:
// allocator::construct.
void allocator_construct_009317d0(void* memory, const void* source, Left61Host& host);
void allocator_construct_00931820(void* memory, const void* source, Left61Host& host);

// 009319D0, __thiscall(ECX = vector; count), RET 4: resize(count) as
// resize(count, value) with a value whose +0 and +10h are zero.
void debris_entry_vector_resize_009319d0(Raw vector, std::uint32_t count, Left61Host& host);

// 00931B70 / 00931C10, __fastcall(ECX = first, EDX = count; const vector* value,
// allocator, 2 tags), RET 10h: _Uninit_fill_n of 10h-byte vectors.
void uninit_fill_n_vectors(std::uint8_t* first, std::uint32_t count, const void* value,
                           void* allocator, std::uint32_t copy_target, std::uint32_t destroy_target,
                           Left61Host& host);
void uninit_fill_n_vectors_00931b70(std::uint8_t* first, std::uint32_t count, const void* value,
                                    void* allocator, Left61Host& host);
void uninit_fill_n_vectors_00931c10(std::uint8_t* first, std::uint32_t count, const void* value,
                                    void* allocator, Left61Host& host);

// 00931CB0 / 00931DE0, __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags),
// RET 10h, EAX = end of dest: _Uninit_move of 10h-byte vectors by swapping.
std::uint8_t* uninit_swap_move_vectors(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                       void* allocator, std::uint32_t copy_target,
                                       std::uint32_t destroy_target, Left61Host& host);
std::uint8_t* uninit_swap_move_vectors_00931cb0(std::uint8_t* first, std::uint8_t* last,
                                                std::uint8_t* dest, void* allocator, Left61Host& host);
std::uint8_t* uninit_swap_move_vectors_00931de0(std::uint8_t* first, std::uint8_t* last,
                                                std::uint8_t* dest, void* allocator, Left61Host& host);

// 009322F0 / 00932570, __thiscall(ECX = vector; const_iterator where (container,
// pointer), count, const vector* value), RET 10h: vector<vector<...>>::_Insert_n.
// The same body serves the named-row vector at 0093B690 (see below); only the
// helpers and the release of the local copy of the value differ.
struct NestedVectorTargets {
    std::uint32_t copy_construct, xlen, allocate, uninit_move, ufill, destroy_range, destroy, umove,
        fill, move_backward;
    void (*release_tmp)(std::uint32_t* tmp, Left61Host& host);
};
void insert_n_nested(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                     const NestedVectorTargets& t, Left61Host& host);
void insert_n_nested_009322f0(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host);
void insert_n_nested_00932570(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host);
// 0093B690, same ABI: vector<named row>::_Insert_n (the local value copy is released
// through the string pool, the helpers are the 0093Axxx/0093Bxxx row instantiations).
// 009488F0, same ABI: vector<spawn member record>::_Insert_n (helpers 00946A80, 009416B0,
// 009466A0, 00947900, 00946D70, 009479D0, 00948080, 00945CF0, 00946C00; the local value copy
// is made by 00660540 and released through the string pool).
void insert_n_record_009488f0(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host);
void insert_n_named_row_0093b690(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                                 Left61Host& host);

// --- segment 61: 10h-byte named rows {+0 dword, +4 native string, +0Ch float} --
// (the ShipSystems failure rows; 0093A8B0 is searched by BSP_ShipSystems_PickRandomFailure)

// 0093A8B0, __fastcall(ECX = first, EDX = last; const native string* name), RET 4:
// the first row whose name equals `name` ignoring case (_stricmp), else last.
std::uint8_t* named_row_find_0093a8b0(std::uint8_t* first, std::uint8_t* last, const std::uint32_t* name);
// 0093A9E0, __fastcall(ECX = out iterator, EDX = name; first {container, ptr},
// last {container, ptr}), RET 10h: out = {first.container, 0093A8B0(first.ptr, last.ptr)}.
std::uint32_t* named_row_find_iterator_0093a9e0(std::uint32_t* out, const std::uint32_t* name,
                                                std::uint32_t first_container, std::uint8_t* first,
                                                std::uint32_t last_container, std::uint8_t* last);
// 0093AC30, __fastcall(ECX = row): release the row's name storage (string at +4).
void named_row_release_name_0093ac30(Raw row, Left61Host& host);
// 0093AFF0, __thiscall(ECX = allocator (unused); row), RET 4: allocator::destroy, same body.
void named_row_destroy_0093aff0(Raw row, Left61Host& host);
// 0093AD30, __thiscall(ECX = row; const row* source), RET 4, EAX = row: copy constructor.
Raw named_row_copy_construct_0093ad30(Raw row, Raw source, Left61Host& host);
// 0093AE60, __fastcall(ECX = memory, EDX = const row* source): _Construct.
void named_row_construct_0093ae60(std::uint8_t* memory, Raw source, Left61Host& host);
// 0093AD90, __cdecl(first, last, const row* value): fill by assignment.
void named_row_fill_0093ad90(std::uint8_t* first, std::uint8_t* last, Raw value, Left61Host& host);
// 0093ADF0, __cdecl(first, last, dest), EAX = dest end: copy by assignment.
std::uint8_t* named_row_copy_0093adf0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                      Left61Host& host);
// 0093AEC0, __cdecl(first, last, dest_end), EAX = dest begin: copy_backward by assignment.
std::uint8_t* named_row_copy_backward_0093aec0(std::uint8_t* first, std::uint8_t* last,
                                               std::uint8_t* dest_end, Left61Host& host);
// 0093B070, __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h,
// EAX = dest end: _Uninit_copy.
std::uint8_t* named_row_uninit_copy_0093b070(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                             Left61Host& host);
// 0093B210, __fastcall(ECX = first, EDX = count; const row* value, allocator, 2 tags),
// RET 10h: _Uninit_fill_n.
void named_row_uninit_fill_n_0093b210(std::uint8_t* first, std::uint32_t count, Raw value,
                                      Left61Host& host);

// --- segment 61: unit controller, simple 3D scene, map overlay, sound overlay --

// 0092F2E0 BSP_UnitController_ApplyNetworkState, __thiscall(ECX = controller (unit+1018h);
// const dword state[8]), RET 4: copies the state into +40h..+5Ch, sets +60h, and tail-jumps
// to 0092E5B0 with its stack argument replaced by 1.
void unit_controller_apply_network_state_0092f2e0(Raw controller, const std::uint32_t* state,
                                                  Left61Host& host);

// 0093CB10 BSP_Simple3dScene_Construct, __fastcall(ECX = scene): vtable 00D19724,
// +4h..+10h and +24h zero.
void simple_3d_scene_construct_0093cb10(Raw scene);
// 0093CC20 BSP_Simple3dScene_Release, __fastcall(ECX = scene).
void simple_3d_scene_release_0093cc20(Raw scene, Left61Host& host);
// 0093D250 BSP_Simple3dScene_SubmitRender, __fastcall(ECX = scene).
void simple_3d_scene_submit_render_0093d250(Raw scene, Left61Host& host);

// Map overlay: +0 scale x, +4 scale y, +8 rgba[4], +18h screen origin x, +1Ch screen
// origin y, +20h world origin x, +24h world origin y.
// 0093E510 BSP_MapOverlay_DrawWorldLine, __thiscall(ECX = overlay; x0, y0, x1, y1), RET 10h.
void map_overlay_draw_world_line_0093e510(Raw overlay, float x0, float y0, float x1, float y1,
                                          Left61Host& host);
// 0093EAE0 BSP_MapOverlay_DrawArrowHead, __thiscall(ECX = overlay; const float point[2],
// const float direction[2], size), RET 0Ch.
void map_overlay_draw_arrow_head_0093eae0(Raw overlay, const float* point, const float* direction,
                                          float size, Left61Host& host);
// 0093EC80 BSP_MapOverlay_DrawArrowMidHead, same ABI with (from, to, size).
void map_overlay_draw_arrow_mid_head_0093ec80(Raw overlay, const float* from, const float* to, float size,
                                              Left61Host& host);
// 0093EDC0 BSP_MapOverlay_DrawArrow, same ABI with (from, to, size).
void map_overlay_draw_arrow_0093edc0(Raw overlay, const float* from, const float* to, float size,
                                     Left61Host& host);
// 0093EEB0 BSP_MapOverlay_DrawWorldCircle, __thiscall(ECX = overlay; const float centre[2],
// radius), RET 8.
void map_overlay_draw_world_circle_0093eeb0(Raw overlay, const float* centre, float radius,
                                            Left61Host& host);

// 0093F280 BSP_SoundOverlay_AxisStep, ECX (CL) = char round_up, stack float value, RET 4,
// ST0 = the axis value rounded to a magnitude-dependent step, clamped to [1, 1e6].
float sound_overlay_axis_step_0093f280(char round_up, float value, Left61Host& host);
// 0093F4B0 BSP_SoundOverlay_CompareByVfunc1C, __cdecl(context, object** a, object** b):
// the qsort_s comparator on vtable +1Ch.
int sound_overlay_compare_0093f4b0(void* context, void* const* a, void* const* b, Left61Host& host);
// 0093FA70, __thiscall(ECX = overlay, whose 0Ch-element array of {float x, float y,
// sound*} sits at +4 {+4 data, +8 count, +0Ch capacity}; float* out, float x, float y,
// void* sound), RET 10h, EAX = out: the label placement. y moves by 12 until no existing
// entry has the same (x, y); {x, y, sound} is appended and (x, y) returned. The fourth
// argument is the sound pointer 0093FF10 pushes (MOV EAX,[ESI]; PUSH EAX at 0094009A),
// stored as a dword (MOV EDX,[ESP+24h]; MOV [EAX+8],EDX at 0093FBDE).
float* sound_overlay_place_label_0093fa70(Raw overlay, float* out, float x, float y, void* sound,
                                          Left61Host& host);
// 0093FE90 BSP_SoundOverlay_ApplySampleSettings, __thiscall(ECX = overlay; 48h-byte options
// by value), RET 48h. `options` is that by-value argument (this function destroys it).
void sound_overlay_apply_sample_settings_0093fe90(Raw overlay, std::uint8_t* options, Left61Host& host);

// --- segment 61: spawn members (10h bytes: +0 vehicle class, +4 native string name,
// +0Ch property bag) ----------------------------------------------------------

// 009420A0 BSP_SpawnMember_SeedPropertyBag, __thiscall(ECX = member; party), RET 4.
void spawn_member_seed_property_bag_009420a0(Raw member, std::int32_t party, Left61Host& host);
// 00942130, __fastcall(ECX = player index), ST0 = the support-points cap per team member:
// float[00E0CFB4] * 0.5 / (active players of the same team), 0 when none.
float team_support_points_cap_00942130(std::int32_t player, Left61Host& host);
// 00943C00 BSP_SpawnList_LinkNode, (next, prev, const dword* record), RET 0Ch (ECX unused),
// EAX = a new 0Ch-byte list node.
void* spawn_list_buy_node_00943c00(void* next, void* prev, const std::uint32_t* record, Left61Host& host);
// 00944130, __thiscall(ECX = member; party, vehicle class, native string name by value
// {length, data}), RET 10h, EAX = member: the member constructor.
Raw spawn_member_construct_00944130(Raw member, std::int32_t party, std::uint32_t vehicle_class,
                                    std::uint32_t name_length, char* name_data, Left61Host& host);
// 00944210 BSP_SpawnMember_ReadLuaTable, __thiscall(ECX = member; Lua table), RET 4.
void spawn_member_read_lua_table_00944210(Raw member, void* table, Left61Host& host);
// 00944400, __cdecl(first, last, dest), EAX = dest end: memmove_s of a pointer range.
void* pointer_range_copy_00944400(void* first, void* last, void* dest);
// 009444C0, __cdecl(first, last, dest_end), EAX = dest begin: member copy_backward.
std::uint8_t* spawn_member_copy_backward_009444c0(std::uint8_t* first, std::uint8_t* last,
                                                  std::uint8_t* dest_end, Left61Host& host);
// 00944540, __fastcall(ECX = first, EDX = hole; top, member value by value {class, length,
// data, bag}), RET 14h: _Push_heap ordered by class+0B0h (a parent moves down while its
// key is greater than the value's).
// The by-value 10h-byte record argument of the heap helpers {+0 dword, +4 length,
// +8 data, +0Ch dword}; the callee releases `data` to the string pool.
struct RecordByValue {
    std::uint32_t head;
    std::uint32_t length;
    char* data;
    std::uint32_t tail;
};

// 00945820, __fastcall(ECX = manager), EAX = manager: the spawn manager constructor
// (10h bytes): +4 list head (00943B90), +8 size 0, +0Ch float bits D01502F9h
// (00CE4ADC), and the singleton 00F89B3C = manager.
Raw spawn_manager_construct_00945820(Raw manager, Left61Host& host);
// 00945C30 BSP_SpawnMember_Construct, __thiscall(ECX = member; party, Lua table), RET 8.
Raw spawn_member_construct_from_table_00945c30(Raw member, std::int32_t party, void* table,
                                               Left61Host& host);
// 00945CF0, __cdecl(first, last, const record* value): fill by assignment.
void record_fill_00945cf0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value,
                          Left61Host& host);
// 00945DA0, __fastcall(ECX = memory, EDX = const record*): _Construct through 00660540.
void record_construct_00945da0(void* memory, const void* source, Left61Host& host);
// 00945E00, __fastcall(ECX = a, EDX = b): swap through a local copy.
void record_swap_00945e00(std::uint8_t* a, std::uint8_t* b, Left61Host& host);
// 00945EE0, __fastcall(ECX = first, EDX = hole; bottom, record value by value, pred),
// RET 18h: _Adjust_heap ordered by class+0B0h.
void spawn_member_adjust_heap_00945ee0(std::uint8_t* first, std::int32_t hole, std::int32_t bottom,
                                       const RecordByValue& value, Left61Host& host);
// 009460B0, __fastcall(ECX = first, EDX = middle; last, 2 tags), RET 0Ch: _Rotate (cycle form).
// EAX is left over from the last pool call and is not modelled.
void record_rotate_009460b0(std::uint8_t* first, std::uint8_t* middle, std::uint8_t* last,
                            Left61Host& host);
// 009462B0, __fastcall(ECX = first, EDX = last; dest, record value by value, pred, tag),
// RET 1Ch: _Pop_heap.
void spawn_member_pop_heap_009462b0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                    const RecordByValue& value, Left61Host& host);
// 00946450, __thiscall(ECX = allocator (unused); record), RET 4: allocator::destroy.
void record_destroy_00946450(Raw record, Left61Host& host);
// 00946480, __fastcall(ECX = first, EDX = last; dest, allocator, 2 tags), RET 10h,
// EAX = dest end: _Uninit_copy.
std::uint8_t* record_uninit_copy_00946480(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                          Left61Host& host);
// 00946570, __fastcall(ECX = first, EDX = last; pred, 2 tags), RET 0Ch: _Make_heap.
void spawn_member_make_heap_00946570(std::uint8_t* first, std::uint8_t* last, Left61Host& host);
// 00946610, __fastcall(ECX = first, EDX = mid; last, pred), RET 8: _Med3.
void spawn_member_median3_00946610(std::uint8_t* first, std::uint8_t* mid, std::uint8_t* last,
                                   Left61Host& host);

// 009466A0, same ABI and body as 00946480 (another instantiation).
std::uint8_t* record_uninit_copy_009466a0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                          Left61Host& host);
// 009467B0, __fastcall(ECX = member vector {+4 first, +8 last}), ST0 = the sum of the
// members' "ResourceUsage" (float when the property type is 1, else the int converted).
float members_resource_usage_009467b0(Raw members, Left61Host& host);
// 00946870, __thiscall(ECX = member vector; player), RET 4, ST0: the same sum over the
// members whose "OwnerPlayer" (-1 when absent) equals `player`.
float members_resource_usage_for_player_00946870(Raw members, std::int32_t player, Left61Host& host);
// 009469F0, __stdcall(team), RET 4, ST0: 009467B0 summed over the spawn manager's queued
// requests (list at [00F89B3C]+4) whose +78h equals `team`.
float queued_resource_usage_for_team_009469f0(std::int32_t team, Left61Host& host);
// 00946C80, __fastcall(ECX = first, EDX = count; const record* value, allocator, 2 tags),
// RET 10h: _Uninit_fill_n.
void record_uninit_fill_n_00946c80(std::uint8_t* first, std::uint32_t count, const std::uint8_t* value,
                                   Left61Host& host);
// 00946DD0, __fastcall(ECX = first, EDX = mid; last, pred), RET 8: _Median (Tukey's
// ninther above 40 elements, else 00946610).
void spawn_member_median_00946dd0(std::uint8_t* first, std::uint8_t* mid, std::uint8_t* last,
                                  Left61Host& host);
// 00946EC0, __fastcall(ECX = first, EDX = last; pred), RET 4: _Pop_heap_0.
void spawn_member_pop_heap_00946ec0(std::uint8_t* first, std::uint8_t* last, Left61Host& host);
// 00946F00, __fastcall(ECX = player), ST0: the player's free support points:
// 00942130(player) minus +304h of every live unit counted to the player, minus 00946970.
float player_support_points_00946f00(std::int32_t player, Left61Host& host);
// 00946FC0 BSP_AiParty_AvailableResources, __fastcall(ECX = team), ST0: [00E0CFB4] * 0.5
// minus +304h of every live unit of the team, minus 009469F0(team).
float team_available_resources_00946fc0(std::int32_t team, Left61Host& host);

// 009473B0, __fastcall(ECX = std::pair<member*, member*>* out, EDX = first; last, pred),
// RET 8: MSVC9 _Unguarded_partition with pred(a, b) = key(a) > key(b), key = class+0B0h.
void spawn_member_unguarded_partition_009473b0(std::uint8_t** out, std::uint8_t* first, std::uint8_t* last,
                                               Left61Host& host);
// 00947680, __fastcall(ECX = first, EDX = last; pred), RET 4: insertion sort that inserts
// through 009460B0 (rotate).
void spawn_member_insertion_sort_00947680(std::uint8_t* first, std::uint8_t* last, Left61Host& host);
// 00947750, __fastcall(ECX = first, EDX = last; pred), RET 4: _Sort_heap.
void spawn_member_sort_heap_00947750(std::uint8_t* first, std::uint8_t* last, Left61Host& host);

// 009480B0, __fastcall(ECX = first, EDX = last; ideal, pred), RET 8: MSVC9 _Sort
// (introsort: partition while more than 32 elements and ideal > 0, heap sort when the
// ideal runs out, insertion sort below 33).
void spawn_member_sort_009480b0(std::uint8_t* first, std::uint8_t* last, std::int32_t ideal,
                                Left61Host& host);

// 009481A0 BSP_SpawnRequest_ReadExcludeRadius, __fastcall(ECX = float block[7]; LuaObject
// by value), RET 14h. `object` is that by-value argument (destroyed here).
void spawn_request_read_exclude_radius_009481a0(float* block, void* object, Left61Host& host);

// 00948BB0, __fastcall(ECX = spawn request): the request destructor.
void spawn_request_destroy_00948bb0(Raw request, Left61Host& host);
// 009496A0, __fastcall(ECX = spawn manager): the manager destructor.
void spawn_manager_destroy_009496a0(Raw manager, Left61Host& host);

// 0094C830, __thiscall(ECX = spawn manager; a0..a7), RET 20h: builds message 42h through
// 00947BC0(a0, a1, a3, a2, a4, a5, a6); the host (game+1FE4h 0 or 1) handles it locally
// with 0094B600(&message, a7), a client sends it to the first secondary peer.
void spawn_manager_send_spawn_request_0094c830(Raw manager, const std::uint32_t (&args)[8], Left61Host& host);
// 0094CA10, __thiscall(ECX = tree (passed on); node), RET 4: post-order delete of a
// red-black subtree {+4 left, +8 right} through each node's deleting destructor.
void ordered_map_delete_subtree_0094ca10(void* tree, void* node, Left61Host& host);
// 0094CA40, __fastcall(ECX = tree): iostdnet::TRedBlackTree<TOrderedMapElement<const char*,
// ZString>, 1> destructor (vtables 00D2505C, then ZMemoryPooledObject 00D24F84).
void ordered_map_destruct_0094ca40(Raw tree, Left61Host& host);

// Entity id table: +4 first id, +8 id count, +10h the first-entry pointer, +1Ch and +3Ch
// list sentinels {next, prev}, +4Ch entry array (10h bytes {next, prev, word id, tag}),
// +50h counter.
// 00951820 BSP_EntityIdTable_LoadUsed, __thiscall(ECX = table; archive), RET 4.
void entity_id_table_load_used_00951820(Raw table, void* archive, Left61Host& host);
// 00951940 BSP_EntityIdTable_SaveUsed, __thiscall(ECX = table; archive), RET 4.
void entity_id_table_save_used_00951940(Raw table, void* archive, Left61Host& host);

// 0094C900, __thiscall(ECX = spawn manager; a0..a7), RET 20h: wraps a1, a4 and a5 into
// one-element vectors (push_back 00947FD0, 00442190, 00442190) and calls 0094C830(a0,
// &v1, a2, a3, &v4, &v5, a6, a7); the three vectors are freed afterwards.
void spawn_manager_send_single_0094c900(Raw manager, const std::uint32_t (&args)[8], Left61Host& host);
// 0094EEF0, __fastcall(ECX = list {+0 entries (8 bytes, +4 text), +4 count}, EDX =
// vector<std::string>*): appends every entry's text ("" for null, 00F89B74).
void append_entry_texts_0094eef0(Raw list, void* strings, Left61Host& host);
// 009510F0, __thiscall(ECX = object; unused, value), RET 8, EAX = object: constructor.
Raw traffic_record_construct_009510f0(Raw object, std::uint32_t unused, std::uint32_t value, Left61Host& host);
// 00951450, __fastcall(ECX = vector): clear through 00951160, free the storage.
void traffic_record_vector_destroy_00951450(Raw vector, Left61Host& host);

void spawn_member_push_heap_00944540(std::uint8_t* first, std::int32_t hole, std::int32_t top,
                                     std::uint32_t value_class, std::uint32_t value_length,
                                     char* value_data, std::uint32_t value_bag, Left61Host& host);

// --- segment 61: effect entities ------------------------------------------------
// The single-effect entity keeps the definition handle at +1E4h and the live effect at
// +1E8h; the periodic one keeps an updater sub-object at +1E4h whose +34h is the period
// ("Period"), +38h the countdown ("Delay"), +3Ch the definition and +40h the live effect.

// 0094F830, __fastcall(ECX = entity): the single-effect entity destructor.
void effect_entity_destruct_0094f830(Raw entity, Left61Host& host);
// 0094F910, __fastcall(ECX = entity): activate, then (entity+0C0h type 1) take the
// "Effect" definition handle into +1E4h.
void effect_entity_activate_0094f910(Raw entity, Left61Host& host);
// 0094FB50, __thiscall(ECX = updater (entity+1E4h); float seconds), RET 4: countdown;
// at or below 0 it restarts and, with a definition, spawns the effect at the entity.
void periodic_effect_update_0094fb50(Raw updater, float seconds, Left61Host& host);
// 0094FC60, __fastcall(ECX = entity): activate, then (type 1) read Period (default 1.0),
// Delay (default 0) and the Effect handle into +218h, +21Ch, +220h.
void periodic_effect_entity_activate_0094fc60(Raw entity, Left61Host& host);

// --- segment 1 ------------------------------------------------------------------

// 0041E490, __thiscall(ECX = array {+0 data, +4 count, +8 capacity}; capacity), RET 4:
// reserve for an array of reference-counted pointers (copies add a reference, the old
// slots are released).
void counted_pointer_array_reserve_0041e490(Raw array, std::int32_t capacity, Left61Host& host);
// 0041F370, __thiscall(ECX = array; count), RET 4: resize (new slots null, trailing slots
// released).
void counted_pointer_array_resize_0041f370(Raw array, std::int32_t count, Left61Host& host);
// 00420660, __fastcall(ECX = owner): 0041F370(owner+30h, 0), then _free of its storage.
void counted_pointer_array_destroy_at30_00420660(Raw owner, Left61Host& host);
// 0041E780 BSP_NativeStringAt4_DestroyStorage, __fastcall(ECX = owner): string at +4.
void native_string_at4_destroy_0041e780(Raw owner, Left61Host& host);
// 004204A0, __thiscall(ECX = allocator (unused); owner), RET 4: same body on the argument.
void native_string_at4_destroy_004204a0(Raw owner, Left61Host& host);
// 0041E7A0, __fastcall(ECX = record): releases the counted pointers at +2Ch, +28h, +24h,
// then the string {+0 length, +4 data}.
void counted_record_destroy_0041e7a0(Raw record, Left61Host& host);
// 00424C40 BSP_GameSettings_GetSingleton, __cdecl(), EAX: the lazy singleton at 00F8753C.
void* game_settings_get_singleton_00424c40(Left61Host& host);
// 00427AC0 BSP_NativeStringVector_Size, __fastcall(ECX = vector), EAX: (last - first) >> 3.
std::int32_t native_string_vector_size_00427ac0(Raw vector);
// 00427C90 BSP_Vector3f_SubtractTo, __thiscall(ECX = a; float out[3], const float b[3]), RET 8.
void vector3_subtract_to_00427c90(const float* a, float* out, const float* b);
// 00427E30 BSP_Vector3f_LengthSquared, __fastcall(ECX = v), ST0 = (x*x + y*y) + z*z.
float vector3_length_squared_00427e30(const float* v);
// 00427EB0 BSP_EntityPose_GetWorldPositionRefreshed, __fastcall(ECX = entity), EAX =
// entity+0FCh after 00414DB0 when +0C8h is clear.
float* entity_world_position_refreshed_00427eb0(Raw entity, Left61Host& host);
// 00428090 BSP_Geometry_SegmentIntersectsSphere, __fastcall(ECX = centre, EDX = start;
// radius, end), RET 8, AL.
bool segment_intersects_sphere_00428090(const float* centre, const float* start, float radius,
                                        const float* end);
// 0042A7E0 BSP_Plane_IsOnGround, __fastcall(ECX = plane), EAX: +900h is 4 or 5.
std::uint32_t plane_is_on_ground_0042a7e0(Raw plane);

}  // namespace bsp::cc13_left61
