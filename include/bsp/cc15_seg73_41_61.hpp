#pragma once

// cc15_seg73_41_61: the game-side MetricsSystem translation unit at
// 00750880-00758090 (segment 41; packet cc15-rB1, docs/CC15_SEG73_41_61.md).
// This is Eidos telemetry plumbing, not gameplay: a singleton (global
// 00E1AED4) built once by cSkeletonAppMidway::Init (0073E48A), that formats
// session ids, mission-start and hardware rows and hands them to the
// statically linked iostdnet/iometrics library (00A4D610-00A5DE34, bound,
// not reconstructed). The std::map / std::vector instantiations of the same
// TU are bound as STL (tag rows in config/tags/00750000.jsonl).
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the
// original ABI; each function's comment gives the native ABI. Native objects
// are raw byte blocks touched only at the offsets the image reads or writes,
// and every call the image makes goes through a named Cc15MetricsHost method
// at the same point with the same arguments. Calls into the trivial logger
// 004254B0 (a bare RET; its format strings are quoted in comments) are
// omitted. The checked-iterator hooks (_invalid_parameter_noinfo 00BF6713)
// on the vector walks test invariants these walks cannot break and are
// omitted; each site says so. SEH unwind states are not modelled.
// Nothing here is wired into the game. Build-tested only. Win32 only: native
// pointer slots are 32-bit.
//
// Not reconstructed here: 007556A0 (9.7 KB mission-end score report, the
// filler of the per-category row map); see the doc.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg41.hpp"  // SingletonLifetimeHost, singleton_register/unregister

namespace bsp {
namespace cc15_metrics {

using cc13_seg41::SingletonLifetimeHost;

// iostdnet::ZString, 0Ch bytes, owned by the bound library; only its address
// is ever used here.
struct ZStringSlot {
    alignas(4) std::uint8_t bytes[0xC];
};

// The game's 8-byte native string {length, data}; data is a pool block of
// length+1 bytes (00419CC0 / 00BD1510) or null.
struct NativeStringSlot {
    std::uint32_t length;
    std::uint32_t data;  // char*
};
static_assert(sizeof(NativeStringSlot) == 8, "native string is two dwords");

// Native constants.
inline constexpr std::uint32_t kMetricsSystemVtable_00d015c0 = 0x00D015C0u;
inline constexpr std::uint32_t kMetricsSingletonVtable_00d012c0 = 0x00D012C0u;
inline constexpr std::uint32_t kStaticPoolVtable_00d0127c = 0x00D0127Cu;
inline constexpr std::uint32_t kStringDataVtable_00d01290 = 0x00D01290u;
inline constexpr std::uint32_t kIntDataVtable_00d01298 = 0x00D01298u;
inline constexpr std::uint32_t kBoolDataVtable_00d012a0 = 0x00D012A0u;
inline constexpr std::uint32_t kPooledObjectVtable_00d24f84 = 0x00D24F84u;  // iostdnet::ZMemoryPooledObject
inline constexpr std::uint32_t kMetricsSessionVtable_00d01324 = 0x00D01324u;
inline constexpr std::uint32_t kSessionMessageBaseVtable_00ce4974 = 0x00CE4974u;
inline constexpr std::uint32_t kMetricsSessionType = 0xE9u;  // MT_METRICSSESSION
inline constexpr std::uint32_t kStaticPoolBytes = 0x100000u;
inline constexpr std::uint32_t kNoUpload = 10;  // +22Ch idle value

// 00D012E8, copied by 007524D0 into a fresh 96h-byte block at +10h (REP MOVSD, 0Fh dwords).
inline constexpr char kMetricsUrl_00d012e8[] = "http://bp-g4w-game.metrics.eidos.co.uk!0x534307E8.xlsp:1000";
static_assert(sizeof(kMetricsUrl_00d012e8) == 0x3C, "15 dwords");

// 00E0A544: the ten upload categories (index 9 is the empty string 00CE3A0C).
extern const char* const kScoreCategories_00e0a544[10];
// 00E0A570: the 19 unit-type tags (index 18 empty). 00E0A5C0: the 19x19
// attacker-target tags (row-major; empty where the image has 00CE3A0C).
extern const char* const kUnitTypeTags_00e0a570[19];
extern const char* const kUnitPairTags_00e0a5c0[19 * 19];

// The function-local static arena of 00752020 (native: guard 00F1AF20, object
// 00E1AEE0 = vtable + library allocator header, buffer 00E1AF20, 1 MB).
struct MetricsStaticArena {
    std::uint32_t* guard_00f1af20;
    std::uint8_t* object_00e1aee0;
    std::uint8_t* buffer_00e1af20;
};

struct Cc15MetricsHost {
    virtual ~Cc15MetricsHost() = default;

    // ---- bound iostdnet / iometrics library (00A4D610-00A5DE34) ----
    virtual void* pool_alloc_00a51e50(std::uint32_t size) = 0;       // cdecl
    virtual void pool_free_00a51e70(void* block) = 0;                 // cdecl
    virtual void* zstring_from_cstr_00a52560(void* out, const char* text) = 0;  // thiscall(ECX out) RET 4 -> out
    virtual void* zstring_from_int_00a52820(void* out, std::int32_t value) = 0;  // thiscall(ECX out) RET 4 -> out
    virtual void zstring_copy_00a529d0(void* out, const void* src) = 0;          // thiscall(ECX out) RET 4
    virtual void zstring_destroy_00a53210(void* str) = 0;                        // thiscall(ECX str)
    virtual void stream_write_zstring_00a4d610(std::uint32_t stream, const void* str) = 0;  // cdecl(stream, str)
    virtual void stream_write_u32_00a4d670(std::uint32_t stream, std::uint32_t value) = 0;  // cdecl(stream, value)
    virtual void static_pool_construct_00a54010(void* pool, void* buffer, std::uint32_t size) = 0;  // thiscall RET 8
    virtual void static_pool_destroy_00a54040(void* pool) = 0;                   // thiscall
    virtual void set_allocator_00a51ee0(void* pool) = 0;                         // cdecl
    virtual void library_startup_00a51d40() = 0;                                 // cdecl, result unused
    virtual void* collector_construct_00a4f7c0(void* block) = 0;                 // thiscall(ECX block) -> block
    virtual void* url_construct_00a57210(void* out, const char* url) = 0;        // cdecl -> out
    virtual void url_destroy_00a56400(void* url) = 0;                            // thiscall
    virtual void* transmitter_construct_00a50210(void* block, void* url) = 0;    // thiscall RET 4 -> block (ZHttpTransmitter)
    virtual void set_transmitter_00a4f040(void* transmitter_10) = 0;             // cdecl
    virtual void collector_enable_00a4efe0(void* collector, std::uint32_t value) = 0;  // thiscall RET 4
    virtual bool collector_submit_00a4f4d0(void* collector, void* metric) = 0;  // thiscall RET 4 -> AL
    virtual void report_error_00a52320(std::uint32_t code) = 0;                  // cdecl
    virtual void collector_shutdown_00a4f300(void* collector) = 0;               // thiscall
    virtual void library_shutdown_00a52390() = 0;
    virtual void library_shutdown_00a51a30() = 0;
    virtual void library_shutdown_00a52170() = 0;
    // Virtual slots of library objects.
    virtual bool collector_ready_vslot4(void* collector) = 0;                    // [[c]+4](ECX c)
    virtual void* collector_create_metric_vslotC(void* collector, const char* name) = 0;  // [[c]+0Ch](ECX c, name)
    virtual void metric_add_vslot4(void* metric_08, const void* key, std::uint32_t* data_slot) = 0;  // [[m+8]+4](ECX m+8, key, &data)
    virtual void transmitter_stop_vslot8(void* transmitter_10) = 0;              // [[t+10h]+8](ECX t+10h)
    virtual void delete_vslot0(void* object) = 0;                                // [[o]](ECX o, 1)

    // ---- game, CRT and platform ----
    virtual MetricsStaticArena static_arena() = 0;   // guard 00F1AF20, object 00E1AEE0, buffer 00E1AF20
    virtual void get_system_time(std::uint16_t out[8]) = 0;  // [00CE226C] GetSystemTime
    virtual std::int32_t get_user_geo_id_nation() = 0;       // [00CE2250] GetUserGeoID(GEOCLASS_NATION = 10h)
    virtual void* game_00e188a8() = 0;                       // the game object
    virtual const char* empty_string_00e1aed8() = 0;         // the native-string null fallback
    virtual const char* version_string_00e0a540() = 0;       // [00E0A540]
    virtual void* manager_00e198ac() = 0;                    // [00E198AC]
    virtual void* xenon_00f8abe8() = 0;                      // [00F8ABE8]
    virtual std::uint32_t random_threads_next_u32_00bd2fc0(std::uint32_t stream) = 0;  // ECX stream
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void atexit_00bf6ff5(std::uint32_t function) = 0;  // registers 00CDD6E0
    virtual void sprintf_s_00bf7a6a(char* out, const char* text) = 0;          // sprintf(out, "%s", text)
    virtual void sprintf_llx_00bf7a6a(char* out, std::uint64_t value) = 0;     // sprintf(out, "%llx", value)
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    virtual SingletonLifetimeHost& singletons() = 0;
    // XenonSystemManager, ECX = [00F8ABE8].
    virtual bool xenon_has_selected_user_00a3e510(void* xenon) = 0;
    virtual std::int32_t xenon_sign_in_state_00a3ead0(void* xenon) = 0;
    virtual bool xenon_is_live_enabled_00a3e520(void* xenon) = 0;
    virtual std::uint64_t xenon_selected_xuid_00a3eb00(void* xenon) = 0;  // EDX:EAX
    virtual std::int32_t game_effective_mode_004bca50(void* game) = 0;     // ECX game
    // Native strings.
    virtual void native_string_resize_0041dd40(NativeStringSlot* s, std::uint32_t length, std::uint32_t keep) = 0;  // RET 8
    virtual void native_string_construct_cstr_0041e870(NativeStringSlot* s, const char* text) = 0;  // RET 4
    virtual void native_string_assign_cstr_0041e350(NativeStringSlot* s, const char* text) = 0;     // RET 4
    virtual void native_string_assign_00425f40(NativeStringSlot* s, const NativeStringSlot* src) = 0;  // RET 4
    virtual NativeStringSlot* native_string_from_int_004260b0(NativeStringSlot* out, std::int32_t value) = 0;  // RET 4
    virtual NativeStringSlot* native_string_concat_004261a0(const NativeStringSlot* left, NativeStringSlot* out,
                                                            const NativeStringSlot* right) = 0;  // RET 8
    virtual NativeStringSlot* native_string_concat_int_004263b0(const NativeStringSlot* left, NativeStringSlot* out,
                                                                std::int32_t value) = 0;  // RET 8
    virtual NativeStringSlot* native_string_substring_00469840(const NativeStringSlot* src, NativeStringSlot* out,
                                                               std::uint32_t start, std::uint32_t count) = 0;  // RET 0Ch
    // 00419CC0 then 00BD1510(ECX pool, block, size, 1): returns a pool block.
    virtual void pool_return_block_00bd1510(std::uint32_t block, std::uint32_t size) = 0;
    // Session messages and the bit cursor.
    virtual void session_message_construct_0075b430(void* message, std::uint32_t type) = 0;  // thiscall RET 4
    virtual void session_send_0076a9f0(void* game_1ef0, void* message, std::uint32_t flag) = 0;  // thiscall RET 8
    virtual void bit_cursor_write_bits_00428ff0(void* cursor, std::uint32_t value, std::uint32_t bits) = 0;  // RET 8
    virtual void bit_cursor_write_owned_string_00429ac0(void* cursor, const NativeStringSlot* s) = 0;     // RET 4
    // Lua objects (14h-byte handles).
    virtual void* lua_globals_00b67980(void* lua_1a0c, void* out) = 0;                  // thiscall RET 4 -> out
    virtual void* lua_get_field_00b67800(const void* table, void* out, const char* key) = 0;  // thiscall RET 8 -> out
    virtual void* lua_get_index_00b67720(const void* table, void* out, std::int32_t key) = 0;  // thiscall RET 8 -> out
    virtual const char* lua_to_string_00b662b0(const void* value) = 0;                  // thiscall
    virtual void lua_destroy_00b67700(void* value) = 0;                                 // thiscall
    // The bound map<int, vector<pair<ZString,ZString>>> at +4: operator[] (00755520,
    // thiscall(ECX map, const int*) RET 4) returns the vector {+4 first, +8 last}.
    virtual void* category_rows_00755520(void* map_04, const std::int32_t* key) = 0;
    // The bound map ctor/dtor pieces: 00751FB0 _Buynode (ECX map) returns the head
    // node; 00755250 (thiscall(ECX map, iterator* out, {map, first}, {map, last})
    // RET 14h) erases a node range.
    virtual void* map_buy_head_node_00751fb0(void* map_04) = 0;
    virtual void map_erase_range_00755250(void* map_04, std::uint32_t first, std::uint32_t last) = 0;
};

// 00750880, __fastcall(ECX v) -> AL: one 6-bit value as a session-id character.
// 0-9 '0'-'9'; 10-36 'A'-'['; 37-52 'B'-'Q' (the third test overlaps the
// second, so as in the image 36 maps to '[' and 'a'-'z' never appear); 53-63
// '+' '-' '[' ']' '{' '}' '_' '!' '.' ',' '('; 64 ')'; anything else '@'.
std::uint8_t metrics_encode_char_00750880(std::uint32_t v);

// 00751BD0, __fastcall(ECX metrics), RET: +1CEh = ' ', then encode(year-2008),
// month, day, hour, minute, second, ms%64, (ms>64 ? encode((ms-64)%64) : '0'),
// then 14 characters at +1D7h from the local player name (game +6A0h when its
// length is nonzero, else +68Ch; data null -> 00E1AED8): pair i (i = 1, 3, ...)
// encodes (name[i] + name[i-1] - 40h) % 64 while i < length, else
// 00BD2FC0(0) & 3Fh; +1E5h = 0. Logs "Generated Session id %s".
void metrics_generate_session_id_00751bd0(std::uint8_t* metrics, Cc15MetricsHost& host);

// The static 1 MB arena class (vtable 00D0127C, slots 1-4 are library
// allocator virtuals 00A54070/00A540A0/00A540D0/00A54100; buffer at +40h).
// 00751080, __thiscall(ECX pool), RET -> pool (unreferenced copy of what
// 00752020 inlines).
std::uint8_t* metrics_static_pool_construct_00751080(std::uint8_t* pool, Cc15MetricsHost& host);
// 007510A0 (no Ghidra function), __thiscall(ECX pool): vtable, JMP 00A54040.
void metrics_static_pool_destroy_007510a0(std::uint8_t* pool, Cc15MetricsHost& host);
// 007510B0, __thiscall(ECX pool, flags) RET 4: vtable slot 0.
std::uint8_t* metrics_static_pool_deleting_dtor_007510b0(std::uint8_t* pool, std::uint32_t flags, Cc15MetricsHost& host);

// iometrics stored-data records (library-pooled, base ZMemoryPooledObject):
// string 10h bytes {vtable 00D01290, ZString +4}; int 8 bytes {00D01298, u32 +4};
// bool 8 bytes {00D012A0, u8 +4}. Slot 0 deleting dtor, slot 1 write(stream).
// 00751800, __thiscall(ECX data, const ZString*) RET 4 -> data.
std::uint8_t* metric_string_data_construct_00751800(std::uint8_t* data, const void* src, Cc15MetricsHost& host);
// 00751870, __thiscall(ECX data), RET (unreferenced; 00751960 inlines it).
void metric_string_data_destroy_00751870(std::uint8_t* data, Cc15MetricsHost& host);
// 00751960, __thiscall(ECX data, flags) RET 4; frees through 00A51E70.
std::uint8_t* metric_string_data_deleting_dtor_00751960(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host);
// 007518C0 / 00751910 (no Ghidra functions), __thiscall(ECX data, const T*) RET 4 -> data.
std::uint8_t* metric_int_data_construct_007518c0(std::uint8_t* data, const std::uint32_t* src);
std::uint8_t* metric_bool_data_construct_00751910(std::uint8_t* data, const std::uint8_t* src);
// 007518E0 / 00751930 (no Ghidra functions), __thiscall(ECX data, stream) RET 4:
// 00A4D670(stream, value) (the bool zero-extended).
void metric_int_data_write_007518e0(const std::uint8_t* data, std::uint32_t stream, Cc15MetricsHost& host);
void metric_bool_data_write_00751930(const std::uint8_t* data, std::uint32_t stream, Cc15MetricsHost& host);
// 00751900 / 00751950 (no Ghidra functions), __thiscall(ECX data): base vtable.
void metric_int_data_destroy_00751900(std::uint8_t* data);
void metric_bool_data_destroy_00751950(std::uint8_t* data);
// 007519D0 / 007519F0 (no Ghidra functions), __thiscall(ECX data, flags) RET 4.
std::uint8_t* metric_int_data_deleting_dtor_007519d0(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host);
std::uint8_t* metric_bool_data_deleting_dtor_007519f0(std::uint8_t* data, std::uint32_t flags, Cc15MetricsHost& host);

// 00751D70 / 00752410: the singleton holder register / unregister (vtable
// 00D012C0, global 00E1AED4), same body as the cc13_seg41 copies.
std::uint32_t* metrics_singleton_register_00751d70(std::uint32_t* self, void*& global_00e1aed4, SingletonLifetimeHost& host);
void metrics_singleton_unregister_00752410(std::uint32_t* self, void*& global_00e1aed4, SingletonLifetimeHost& host);

// 00752020, cdecl, RET -> AL = 1: the function-local static arena (guard bit 0
// of 00F1AF20; object 00E1AEE0, buffer 00E1AF20, 1 MB; atexit 00CDD6E0),
// then 00A51EE0(&arena) and 00A51D40().
bool metrics_library_startup_00752020(const MetricsStaticArena& arena, Cc15MetricsHost& host);

// 007520A0 / 00752120 / 00752170, __thiscall(ECX metric, const ZString* key,
// const T* value) RET 8: pool-allocate a string (10h) / int (8) / bool (8)
// record (null when the pool fails) and pass &record to [[metric+8]+4](key, &record).
void metric_add_string_007520a0(void* metric, const void* key, const void* value, Cc15MetricsHost& host);
void metric_add_int_00752120(void* metric, const void* key, const std::uint32_t* value, Cc15MetricsHost& host);
void metric_add_bool_00752170(void* metric, const void* key, const std::uint8_t* value, Cc15MetricsHost& host);

// 007524D0, __fastcall(ECX metrics), RET: +20h..+32h = the 19 unit-type tags
// are non-empty, +33h = 0; +34h + 14h*row + col (19x19) = the pair tags are
// non-empty; zero +47h + 14h*i and +1B0h + i for i = 0..19. When +238h is
// clear and 00752020 succeeds: URL block, collector (64h pooled bytes) at
// +230h, transmitter (9Ch pooled bytes, from a ZUrl temporary) at +234h,
// 00A4F040(transmitter + 10h or 0), +238h = 1 (logs "MetricsSystem ...
// Initialised ..."). Always: +14h = GetUserGeoID(nation) unless -1 (then 0),
// +1CEh = "NULL", +22Ch = 10.
void metrics_system_init_007524d0(std::uint8_t* metrics, Cc15MetricsHost& host);

// METRICSSESSION (E9h) session message: vtable 00D01324 {00752C60 deleting
// dtor, 00752C00 write, 00752C30 read (cc13_seg41), 00752BF0 IsType, 004499C0};
// byte +10h, owned native string +18h.
// 00752B90, __thiscall(ECX message), RET.
void metrics_session_message_destroy_00752b90(std::uint8_t* message, Cc15MetricsHost& host);
// 00752C00, __thiscall(ECX message, cursor) RET 4: 8 bits of +10h, then +18h.
void metrics_session_message_write_00752c00(const std::uint8_t* message, void* cursor, Cc15MetricsHost& host);
// 00752D50, __thiscall(ECX message, native string by value) RET 8: base
// construct (E9h), +4 = 1, vtable, copy the id into +18h, release the argument.
std::uint8_t* metrics_session_message_construct_00752d50(std::uint8_t* message, NativeStringSlot id, Cc15MetricsHost& host);

// 00752E00, __fastcall(ECX metrics), RET: +18h = "Y-M-D H:M:S" from
// GetSystemTime through the native string helpers (no zero padding).
void metrics_build_timestamp_00752e00(std::uint8_t* metrics, Cc15MetricsHost& host);

// 00753350, __thiscall(ECX metrics, native string by value) RET 8: ids longer
// than 18h characters are cut to their first 17h; copied to +1CEh; released.
void metrics_set_session_id_00753350(std::uint8_t* metrics, NativeStringSlot id, Cc15MetricsHost& host);

// 00753570, __thiscall(ECX metrics, const vector* rows) RET 4: when enabled
// and the selected Live user is signed in (state 2 or 1) on a Live-enabled
// account: "Hardwares" metric with one string row per {key, value} native
// string pair (10h-byte elements), "Location" int (+14h), "TimeStamp".
void metrics_send_hardware_00753570(std::uint8_t* metrics, const std::uint8_t* rows, Cc15MetricsHost& host);

// 00753810, __thiscall(ECX metrics, int event, char checkpoint) RET 8: the
// "MissionStart" metric (event 1 single, 5 training ground, 3 skirmish,
// 9 multiplayer). See the .cpp for the row list.
void metrics_mission_start_00753810(std::uint8_t* metrics, std::int32_t event, std::uint8_t checkpoint,
                                    Cc15MetricsHost& host);

// 00757CE0, __thiscall(ECX metrics, float raw_delta unused) RET 4: per-frame
// upload of one category (+22Ch) when the collector is ready.
void metrics_upload_tick_00757ce0(std::uint8_t* metrics, Cc15MetricsHost& host);

// 00757EA0, __fastcall(ECX metrics), RET: the destructor body (00758070 is
// its scalar deleting wrapper).
void metrics_system_destroy_00757ea0(std::uint8_t* metrics, void*& global_00e1aed4, Cc15MetricsHost& host);

// 00757FE0, __fastcall(ECX metrics) -> metrics: the 23Ch-byte constructor.
std::uint8_t* metrics_system_construct_00757fe0(std::uint8_t* metrics, void*& global_00e1aed4, Cc15MetricsHost& host);

}  // namespace cc15_metrics
}  // namespace bsp
