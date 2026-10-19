#pragma once
// Segments 3 and 4 (0043E8F0-004486A0, 004486C0-00453C30), packet cc13-seg3:
// device-entity virtuals and the device class lifetime (segment 3), and the
// dialog command classes the panel sequencer reads and writes (segment 4).
// docs/CC13_SEG3_4.md has the evidence table. Names are hypotheses; offsets
// are the image's. Objects are raw layouts at byte offsets; every call the
// image makes is a named host method invoked at the same point.
#include <cstdarg>
#include <cstdint>
#include <string_view>

namespace bsp::seg3_4_cc13 {

// ===========================================================================
// Tagged values passed by value (8 bytes) to the property reader / writer
// virtuals: kind 0 string (payload const char* or NativeString*), 1 int,
// 2 float; payload a value or the address of the destination.
// ===========================================================================
struct TaggedValue {
    std::uint32_t kind{0};
    std::uintptr_t payload{0};
};
inline constexpr std::uint32_t kTagString = 0;
inline constexpr std::uint32_t kTagInt = 1;
inline constexpr std::uint32_t kTagFloat = 2;

// The property stream the dialog commands and the device entity save state
// use (reader slots +0Ch / +10h, writer slots +04h / +08h / +0Ch).
struct PropertyStreamHost {
    virtual ~PropertyStreamHost() = default;
    // Reader vtable +0Ch(key, destination, default): read with a default.
    virtual void read_with_default_0c(void* reader, TaggedValue key, TaggedValue destination,
                                      TaggedValue fallback) = 0;
    // Reader vtable +10h(key, destination): plain read.
    virtual void read_10(void* reader, TaggedValue key, TaggedValue destination) = 0;
    // Writer vtable +0Ch(key, value): write one key.
    virtual void write_0c(void* writer, TaggedValue key, TaggedValue value) = 0;
    // Writer vtable +04h(key): open a sub-table; vtable +08h(): close it.
    virtual void open_table_04(void* writer, TaggedValue key) = 0;
    virtual void close_table_08(void* writer) = 0;
    // The image address of a C string literal or of an object field.
    virtual std::uintptr_t address_of(const void* object) = 0;
    virtual std::uintptr_t literal(std::string_view text) = 0;
};

// ===========================================================================
// Dialog command classes (base vtable 00CE4940). Layout: +0h vtable, +4h panel
// index; msg / callback: NativeString +8h (length) / +Ch (text); setpanel: name
// +8h/+Ch, character +10h/+14h; pause: float time +8h. Type ids: msg 0,
// setpanel 1, hidepanel 2, pause 3, callback 4.
// ===========================================================================
inline constexpr std::uint32_t kDialogCommandBaseVtable = 0x00CE4940u;
inline constexpr std::string_view kKeyPanel = "panel";          // 00CE499C
inline constexpr std::string_view kKeyMessage = "message";      // 00CE49A4
inline constexpr std::string_view kKeyName = "name";            // 00CE49B8
inline constexpr std::string_view kKeyCharacter = "character";  // 00CE49AC
inline constexpr std::string_view kKeyTime = "time";            // 00CE37A8
inline constexpr std::string_view kKeyCallback = "callback";    // 00CE49C0
inline constexpr std::string_view kKeyType = "type";            // 00CE4AD0
inline constexpr std::string_view kEmptyCString = "";           // 00E17BFB (a zero byte)

struct DialogCommandHost : PropertyStreamHost {
    virtual void free_00bf65ac(void* block) = 0;
    // 00419CC0 then 00BD1510(text, length + 1, 1): return a string block to the pool.
    virtual void pool_return_block(void* text, std::uint32_t size) = 0;
    virtual std::int32_t command_type_04(void* command) = 0;   // command vtable +04h
    // The type name 0044C060 builds (0041DD40 + memcpy) and releases after the write.
    virtual std::uintptr_t temporary_string(std::string_view text) = 0;
    virtual void release_temporary_string(std::uintptr_t text) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 00425850 __thiscall(const NativeString*, const char*) RET 4 -> AL: case-insensitive equality.
    virtual bool native_string_equals_ci_00425850(const void* string, const char* text) = 0;
    virtual void command_read_08(void* command, void* reader) = 0;   // command vtable +08h
};
inline constexpr std::uint32_t kDialogMsgVtable = 0x00CE49CCu;
inline constexpr std::uint32_t kDialogSetPanelVtable = 0x00CE49DCu;
inline constexpr std::uint32_t kDialogHidePanelVtable = 0x00CE4950u;
inline constexpr std::uint32_t kDialogPauseVtable = 0x00CE4960u;
inline constexpr std::uint32_t kDialogCallbackVtable = 0x00CE49ECu;

// 00449A30 / 00449A50 __thiscall(command, flags) RET 4 -> EAX: the hidepanel
// (8-byte) and pause (0Ch-byte) scalar deletes: vtable = 00CE4940, _free when
// flags bit 0.
void* dialog_command_scalar_delete_00449a30(void* command, std::uint32_t flags, DialogCommandHost& host);
void* dialog_command_scalar_delete_00449a50(void* command, std::uint32_t flags, DialogCommandHost& host);
// 0044A3F0 __thiscall(command, reader) RET 4: reader +0Ch("panel", int ->
// +4h, default 1), then +4h = 0 (the value read is always overwritten).
void dialog_command_read_base_0044a3f0(void* command, void* reader, DialogCommandHost& host);
// 0044A440 __thiscall(command, reader) RET 4: the base read inline, then
// reader +10h("message", string -> +8h).
void dialog_command_msg_read_0044a440(void* command, void* reader, DialogCommandHost& host);
// 0044A4C0 __thiscall(command, reader) RET 4: the base read inline, then
// +10h("name" -> +8h) and +10h("character" -> +10h).
void dialog_command_setpanel_read_0044a4c0(void* command, void* reader, DialogCommandHost& host);
// 0044A570 __thiscall(command, reader) RET 4: +4h = 0, then +10h("time",
// float -> +8h).
void dialog_command_pause_read_0044a570(void* command, void* reader, DialogCommandHost& host);
// 0044A5B0 __thiscall(command, reader) RET 4: +4h = 0, then +10h("callback",
// string -> +8h).
void dialog_command_callback_read_0044a5b0(void* command, void* reader, DialogCommandHost& host);
// 0044B380 / 0044B3B0 / 0044B3E0 __fastcall -> EAX: the msg, setpanel and
// callback type ids 0, 1, 4.
std::int32_t dialog_command_msg_type_0044b380() noexcept;
std::int32_t dialog_command_setpanel_type_0044b3b0() noexcept;
std::int32_t dialog_command_callback_type_0044b3e0() noexcept;
// 0044B3F0 / 0044B4D0 __fastcall(command): the msg / callback destructors:
// the NativeString +8h's block (when +Ch is set, length +8h + 1) back to the
// pool, then vtable 00CE4940. 0044B450: setpanel, the character string
// (+10h/+14h) first, then the name (+8h/+Ch).
void dialog_command_msg_destruct_0044b3f0(void* command, DialogCommandHost& host);
void dialog_command_setpanel_destruct_0044b450(void* command, DialogCommandHost& host);
void dialog_command_callback_destruct_0044b4d0(void* command, DialogCommandHost& host);
// 0044BE50 __fastcall(ECX = reader) -> EAX: the dialog command factory. A
// local NativeString gets reader +0Ch("type", default "msg"); "msg" (non-null
// text and __stricmp) makes a 10h-byte msg (vtable 00CE49CC, +8h/+Ch = 0),
// then through 00425850: "setpanel" 18h bytes (00CE49DC, +8h..+14h = 0),
// "hidepanel" 8 bytes (00CE4950), "pause" 0Ch bytes (00CE4960), "callback"
// 10h bytes (00CE49EC, +8h/+Ch = 0); +4h is left for the read. The command's
// vtable +08h then reads the reader. An unknown type (or a failed operator
// new) leaves the command null and the image calls through it anyway; the
// host receives the null. The local string is returned to the pool.
void* dialog_command_create_from_archive_0044be50(void* reader, DialogCommandHost& host);
// 0044C060 __thiscall(command, writer) RET 4: writer +0Ch("panel", int +4h + 1),
// then the type name by vtable +04h (0 "msg", 1 "setpanel", 2 "hidepanel",
// 3 "pause", 4 "callback", other: an empty NativeString, written as 00E17BFB)
// as +0Ch("type", string); the temporary is released.
void dialog_command_write_base_0044c060(void* command, void* writer, DialogCommandHost& host);
// 0044C220 / 0044C340 __thiscall(command, writer) RET 4: the base write, then
// "message" / "callback" = the text +Ch (00E17BFB when null).
void dialog_command_msg_write_0044c220(void* command, void* writer, DialogCommandHost& host);
void dialog_command_callback_write_0044c340(void* command, void* writer, DialogCommandHost& host);
// 0044C270 __thiscall(command, writer) RET 4: the base write, "name" = +Ch,
// "character" = +14h (00E17BFB for null).
void dialog_command_setpanel_write_0044c270(void* command, void* writer, DialogCommandHost& host);
// 0044C2F0 __thiscall(command, writer) RET 4: the base write, "time" = float +8h.
void dialog_command_pause_write_0044c2f0(void* command, void* writer, DialogCommandHost& host);

// ===========================================================================
// Device entity virtuals and the device class lifetime.
// ===========================================================================
struct DeviceEntityHost : PropertyStreamHost {
    virtual void entity_die_70(void* entity, std::int32_t arg) = 0;              // entity vtable +70h
    virtual void unit_init_health_and_parts_0087bcc0(void* entity) = 0;          // __thiscall
    virtual void* decal_list_00e1aea0() = 0;                                     // [00E1AEA0]
    virtual std::uint32_t decal_list_register_owner_00740fe0(void* list) = 0;    // __thiscall -> EAX
    // 00878350 __thiscall(entity, message) RET 4 -> AL (reached by a tail jump).
    virtual bool entity_handle_message_fallback_00878350(void* entity, void* message) = 0;
    // 008776E0 __thiscall(entity, writer) RET 4: the tickable-entity state write.
    virtual void unit_write_state_008776e0(void* entity, void* writer) = 0;
    // 0087C640 / 0087C700 __thiscall(class): the damageable class base constructor / destructor.
    virtual void damageable_class_construct_base_0087c640(void* device_class) = 0;
    virtual void damageable_class_destruct_base_0087c700(void* device_class) = 0;
    // 00441780 cdecl -> the device class registry (+4h slot array, +8h count).
    virtual void* device_class_registry_00441780() = 0;
    // 00440180 __thiscall(slots = registry+4h, count) RET 4.
    virtual void device_class_registry_resize_00440180(void* slots, std::int32_t count) = 0;
    // [00CE2220] InterlockedDecrement(&object+4h) -> EAX.
    virtual std::int32_t interlocked_decrement_ce2220(void* counter) = 0;
    virtual void object_destroy_00(void* object) = 0;                            // object vtable +00h
};

inline constexpr std::uint32_t kDeviceClassVtable = 0x00CE4480u;
inline constexpr std::string_view kKeyDeviceClass = "deviceclass";  // 00CE4474
inline constexpr std::string_view kKeyDevice = "_device";          // 00CE446C
inline constexpr std::string_view kKeyPlatform = "platform";       // 00CE4460

// 0043F120 / 0043F7D0 __fastcall(entity), two identical vtable slots: when the
// health +370h is <= 0.0f (COMISS; NaN keeps it alive) vtable +70h(1).
void device_die_if_health_depleted_0043f120(void* entity, DeviceEntityHost& host);
void device_die_if_health_depleted_0043f7d0(void* entity, DeviceEntityHost& host);
// 0043F7B0 __fastcall(entity): 0087BCC0, then +168h = 00740FE0([00E1AEA0]).
void device_init_health_and_parts_0043f7b0(void* entity, DeviceEntityHost& host);
// 0043F860 __thiscall(entity, message) RET 4 -> AL: message types 4Bh / 4Ch
// (byte +10h) answer 1; anything else goes to 00878350.
bool device_handle_messages_4b4c_0043f860(void* entity, void* message, DeviceEntityHost& host);
// 0043FEA0 __thiscall(entity, writer) RET 4: 008776E0, "deviceclass" =
// int [[+354h]+6Ch], then sub-table "_device" with "platform" = int +38Ch.
void device_write_state_0043fea0(void* entity, void* writer, DeviceEntityHost& host);
// 00440770 __fastcall(class) -> EAX: 0087C640, vtable 00CE4480, +70h = 0.
void* device_class_construct_00440770(void* device_class, DeviceEntityHost& host);
// 00440A30 __fastcall(slot): a counted reference release: when [slot] is set,
// InterlockedDecrement(+4h) reaching 0 destroys it (vtable +0h), then [slot] = 0.
void ref_slot_release_00440a30(void* slot, DeviceEntityHost& host);
// 004420D0 __fastcall(class): vtable 00CE4480; the registry slot +6Ch is
// cleared (the registry grows to +6Ch + 1 first when +6Ch >= its count); the
// platform reference +70h is released as in 00440A30; then 0087C700.
void device_class_destruct_004420d0(void* device_class, DeviceEntityHost& host);

// ===========================================================================
// Panel sequence records.
// ===========================================================================
struct PanelRecordHost {
    virtual ~PanelRecordHost() = default;
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t keep) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // 0044DC40 / 0044EF30 __thiscall(dst member, const src member*) RET 4: the
    // two container copy-constructors 0044F380 delegates to.
    virtual void copy_member_0044dc40(void* dst, const void* src) = 0;
    virtual void copy_member_0044ef30(void* dst, const void* src) = 0;
};
inline constexpr std::uint32_t kRecord40Vtable = 0x00CE4B40u;

// 0044C620 __thiscall(dst, const src*) RET 4 -> EAX = dst: the sequence slot
// copy-constructor: byte +0h, then the NativeStrings +4h and +Ch, each zeroed
// first and, unless it is the source object, resized to the source length and
// filled (memcpy of the destination length) when the source is not empty.
void* panel_sequence_slot_copy_construct_0044c620(void* dst, const void* src, PanelRecordHost& host);
// 0044C7C0 __thiscall(dst, const src*) RET 4 -> EAX = dst: the same as an
// assignment (no zeroing first).
void* panel_sequence_slot_assign_0044c7c0(void* dst, const void* src, PanelRecordHost& host);
// 0044C8D0 __fastcall(ECX = place, EDX = src): 0044C620 into place when place
// is not null (an allocator construct).
void panel_sequence_slot_construct_at_0044c8d0(void* place, const void* src, PanelRecordHost& host);
// 0044F9D0 __fastcall(ECX = record) -> EAX: vtable 00CE4B40, +4h/+8h/+Ch =
// 0.0f, byte +10h = 0, +18h..+24h and +2Ch..+34h = 0, byte +38h = 0, +3Ch = 0.
void* record40_construct_0044f9d0(void* record) noexcept;
// 0044F380 __thiscall(dst, const src*) RET 4 -> EAX = dst: vtable 00CE4B40,
// floats +4h..+Ch (FLD/FSTP), byte +10h, 0044DC40 on +14h, dword +24h,
// 0044EF30 on +28h, byte +38h, dword +3Ch.
void* record40_copy_construct_0044f380(void* dst, const void* src, PanelRecordHost& host);
// 00450450 __thiscall(dst, const src*) RET 4 -> EAX = dst: the NativeString
// key +0h (zeroed, then copied unless self) and the 0044F380 record at +8h.
void* panel_sequence_pair_copy_construct_00450450(void* dst, const void* src, PanelRecordHost& host);

// ===========================================================================
// 00443A30 __thiscall(clock, int* hours, int* minutes, int* seconds, int* ms)
// RET 10h (caller BSP_LuaBinding_GetDayTime): the float hour +4h split by
// truncation (CVTTSS2SI): hours = (int)t; then, each only when its pointer is
// set, m = (float)((t - hours) * 60.0), minutes = (int)m; s = (float)((m -
// minutes) * 60.0), seconds = (int)s; ms = _ftol((float)((s - seconds) *
// 1000.0)). The two 60.0 products use the DC-form FMUL (ST1 *= ST0; the
// listing prints it as FMUL ST1); the image keeps the 80-bit intermediates,
// the C++ uses double (MINOR).
void time_of_day_split_clock_00443a30(const void* clock, std::int32_t* hours, std::int32_t* minutes,
                                      std::int32_t* seconds, std::int32_t* milliseconds) noexcept;

// ===========================================================================
// Small owners.
// ===========================================================================
struct SmallOwnerHost {
    virtual ~SmallOwnerHost() = default;
    // 00BF7289 cdecl(buffer, count, format, va_list) (the CRT _vsprintf_s).
    virtual int vsprintf_s_00bf7289(char* buffer, std::uint32_t count, const char* format, va_list args) = 0;
    virtual void object_scalar_delete_00(void* object, std::uint32_t flags) = 0;   // object vtable +00h
    // 00444B10 __thiscall(map, out, first {map, node}, last {map, node}) RET 14h: erase a range.
    virtual void float_map_erase_00444b10(void* map, void* out, void* first_map, void* first_node, void* last_map,
                                          void* last_node) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void pool_return_block(void* text, std::uint32_t size) = 0;   // 00419CC0 + 00BD1510
};
inline constexpr std::uint32_t kWeatherConfigVtable = 0x00CE4934u;

// 0043EE60 cdecl(buffer, format, ...): _vsprintf_s(buffer, 2Ah, format, args).
void string_format_buffer_2a_0043ee60(SmallOwnerHost& host, char* buffer, const char* format, ...);
// 00449310 __fastcall(slot): when [slot] is set, its vtable +00h(1), then [slot] = 0.
void owned_ptr_delete_reset_00449310(void* slot, SmallOwnerHost& host);
// 00445940 __fastcall(config): vtable 00CE4934; the float maps +68h, +5Ch and
// +38h are emptied (00444B10 over [begin, head)), their head nodes freed and
// +4h/+8h zeroed; then the NativeString +2Ch/+30h block goes back to the pool.
void weather_config_destruct_00445940(void* config, SmallOwnerHost& host);

}  // namespace bsp::seg3_4_cc13

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::seg3_4_cc13 {
struct DeviceHitRelayHost {
    virtual ~DeviceHitRelayHost() = default;
    virtual bool is_kind_5c(void* object, std::uint32_t kind) = 0;
    virtual void apply_hit_ec(void* object, void* hit) = 0;
    virtual void unit_apply_hit_008777d0(void* device, void* hit) = 0;
};
// ECX device, stack hit, RET 4. Names are inferred from the 0043F7xx family.
void device_relay_hit_0043f7f0(void* device, void* hit, DeviceHitRelayHost& host);
// ECX unused, stack kind, RET 4, EAX = 0/1; vtable 00CE4480 slot 6.
bool device_class_is_kind_004407b0(std::uint32_t kind) noexcept;
// RET, EAX = 28h; MCatapult class table at 00CE46C0.
std::uint32_t catapult_class_kind_00443040() noexcept;
struct PooledStringAt0cHost {
    virtual ~PooledStringAt0cHost() = default;
    virtual void* pool_00419cc0() = 0;
    virtual void return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                       std::uint32_t unit) = 0;
};
// ECX owner, RET; pooled string is length +Ch, text +10h.
void pooled_string_at_0c_release_00444000(void* owner, PooledStringAt0cHost& host);
struct RefSlotAssignHost : DeviceEntityHost {
    virtual void interlocked_increment_ce221c(void* counter) = 0;
};
// 00440000: ECX destination slot, EDX source slot, RET, EAX destination.
// Engine-owned counted-reference helper; no stock-library provenance established.
void* ref_slot_assign_00440000(void* destination, const void* source, RefSlotAssignHost& host);
}
