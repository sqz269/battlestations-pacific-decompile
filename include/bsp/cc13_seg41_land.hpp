#pragma once
// Segment 41 land units, landing ships, plane-unit slots and the mothership
// (cc13-seg41). Names are hypotheses, not recovered symbols. Nothing here is
// wired into the game build: new C++ interfaces, not ABI-compatible,
// build-tested only. Evidence: docs/CC13_SEG41.md and the comments below.
//
// Entity objects are addressed by their image base (std::uint8_t*) and read or
// written at the image offsets; nothing invents a struct field for them. Calls
// out of a function go through named host hooks at the point of the call.
#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg41.hpp"
#include "bsp/cc15_land_convoy.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_session_message_base.hpp"
#include "bsp/net_entity_create_cc13.hpp"

namespace bsp::cc13_seg41 {

template <class T>
inline T& at(std::uint8_t* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}
template <class T>
inline const T& at(const std::uint8_t* base, std::uint32_t offset) {
    return *reinterpret_cast<const T*>(base + offset);
}
inline std::uint8_t* ptr(std::uint32_t value) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(value));
}
inline std::uint32_t dword_of(const void* p) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}

// The two-bank entity table the image inlines for a 16-bit net id: id <
// [00F89A10] reads bank 0 ([00F89A54], base id [00F89A0C]), else bank 1
// ([00F89AA8], base [00F89A60]); 10h-byte records with the entity at +0Ch.
struct EntityIdTable {
    const std::int32_t& split_00f89a10;
    const std::int32_t& bank0_base_00f89a0c;
    std::uint8_t* const& bank0_records_00f89a54;
    const std::int32_t& bank1_base_00f89a60;
    std::uint8_t* const& bank1_records_00f89aa8;
    std::uint8_t* entity(std::uint32_t id) const {
        if (static_cast<std::int32_t>(id) < split_00f89a10)
            return at<std::uint8_t*>(bank0_records_00f89a54, (id - bank0_base_00f89a0c) * 0x10u + 0x0Cu);
        return at<std::uint8_t*>(bank1_records_00f89aa8, (id - bank1_base_00f89a60) * 0x10u + 0x0Cu);
    }
};

// Shared numeric codec convention: (zero 0, signed 1, scale [00D7A248] =
// FLT_MAX, 32 bits), the raw 32-bit path.
inline constexpr std::uint32_t kFltMaxBits_00d7a248 = 0x7F7FFFFFu;

// ---------------------------------------------------------------------------
// LANDCONVOY_CREATE (type CFh, vtable 00CFF2BC, 134h bytes): the generic
// entity-create head (net_cc13::EntityCreateMessage, 110h in the image) plus
// the convoy fields.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kLandConvoyCreateVtable_00cff2bc = 0x00CFF2BCu;
struct LandConvoyCreateMessage {
    net_cc13::EntityCreateMessage base;  // image +0..+10Fh
    std::uint32_t vtable_00{0};
    std::uint16_t path_entity_id_110{0};  // id of the path object's entity (+344h), 13 bits
    std::int32_t rows_114{0};             // convoy +358h, 8 bits
    std::int32_t columns_118{0};          // convoy +354h, 8 bits
    float float_11c{0.0f};                // convoy +360h
    float float_120{0.0f};                // convoy +364h
    float float_124{0.0f};                // convoy +35Ch
    std::uint16_t* member_ids_128{nullptr};  // rows*columns ids, 12 bits each
    std::uint8_t reverse_12c{0};          // convoy +3A9h
    float speed_130{0.0f};                // convoy +368h
};

struct ConvoyMessageHost {
    virtual ~ConvoyMessageHost() = default;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void base_destruct_007668e0(LandConvoyCreateMessage& m) = 0;  // ECX = message
    // 007ACA30, ECX = convoy+344h path object -> the path's entity.
    virtual std::uint8_t* path_entity_007aca30(std::uint8_t* path) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    // 00779FC0, __thiscall(ECX entity, peer, message*): send to a peer.
    virtual void send_to_peer_00779fc0(std::uint8_t* entity, std::uint32_t peer, LandConvoyCreateMessage& m) = 0;
};

// 00741A40, __thiscall(ECX message, word, convoy) RET 8 -> message. The base
// is 00766FC0(CFh, word, convoy, 1), here build_entity_create_00766fc0.
LandConvoyCreateMessage& land_convoy_create_construct_00741a40(
    LandConvoyCreateMessage& m, std::uint16_t word, const std::uint8_t* convoy,
    const net_cc13::EntityCreateSource& source, net_cc13::EntityCreateHost& base_host, ConvoyMessageHost& host);
// 00741180, __fastcall(ECX message): vtable, free(+128h) (00BF65AC), JMP 007668E0.
void land_convoy_create_destruct_00741180(LandConvoyCreateMessage& m, ConvoyMessageHost& host);
// 00741420, RET 4, ECX ignored: t == CFh || t == 47h || t == 46h.
bool land_convoy_create_is_type_00741420(std::uint32_t type);
// 00741450, __thiscall(ECX message, cursor) RET 4.
void land_convoy_create_write_00741450(const LandConvoyCreateMessage& m, NativeBitCursor* cursor,
                                       const NativeBitNumericContext& numeric);
// 00741570, __thiscall(ECX message, stream) RET 4. The id array is allocated
// with operator new (00BF55BE) of rows*columns*2 bytes (0xFFFFFFFF on 32-bit
// overflow); the previous +128h is not freed.
void land_convoy_create_read_00741570(LandConvoyCreateMessage& m, NativeSessionReadStream& stream,
                                      const NativeBitNumericContext& numeric, ConvoyMessageHost& host);
// 00741D40, __thiscall(ECX convoy, peer) RET 4: build a create message on the
// stack (word 0), send it through 00779FC0, destroy it.
void land_convoy_send_create_00741d40(std::uint8_t* convoy, std::uint32_t peer,
                                      const net_cc13::EntityCreateSource& source,
                                      net_cc13::EntityCreateHost& base_host, ConvoyMessageHost& host);

// 00743B70, __fastcall(ECX message) -> new convoy (vtable slot +1Ch of
// 00CFF2BC): operator new 3CCh, memset 0, construct (004F2410, ECX memory,
// id word +1Ch) when the allocation succeeded; then path +344h = 007AC9D0 of
// the entity with id +110h (null stays null), copy the dimensions and floats,
// append every member entity to the std::vector at +394h (inline push_back,
// growth through 00743350), copy reverse and speed.
struct ConvoyCreateEntityHost {
    virtual ~ConvoyCreateEntityHost() = default;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual std::uint8_t* convoy_construct_004f2410(void* memory, std::uint16_t id) = 0;
    virtual std::uint8_t* path_interface_007ac9d0(std::uint8_t* entity) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    // 00743350, __thiscall(ECX vector, iterator* out, vector, where, const value*) RET 10h.
    virtual void vector_insert_00743350(std::uint8_t* vector, std::uint32_t* out, std::uint8_t* container,
                                        std::uint32_t where, const std::uint32_t* value) = 0;
};
std::uint8_t* land_convoy_create_entity_00743b70(const LandConvoyCreateMessage& m, const EntityIdTable& ids,
                                                 ConvoyCreateEntityHost& host);

// 007416F0, __thiscall(ECX convoy, lua, key) RET 8, vtable 00CEA570 +138h:
// 00779BB0 (the unit getter) first, then the keys. "InitialSize" is matched
// with _stricmp on key+4 (skipped for a null key text), the others with
// 00425850 (ECX key). Speed, Offset and DistanceTraveled push a BOOLEAN of
// (value != 0.0f) through 00B66450 - the image does that (UCOMISS against
// [00D7A218] = 0.0f, NaN pushes true); the C++ keeps it.
struct LuaPropertyHost {
    virtual ~LuaPropertyHost() = default;
    virtual void unit_get_property_00779bb0(std::uint8_t* self, void* lua, const std::int32_t* key) = 0;
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;
    virtual bool key_equals_00425850(const std::int32_t* key, const char* text) = 0;
    virtual void push_integer_00b664b0(void* lua, std::int32_t value) = 0;
    virtual void push_float_00b66480(void* lua, float value) = 0;
    virtual void push_boolean_00b66450(void* lua, bool value) = 0;
};
void land_convoy_get_property_007416f0(std::uint8_t* convoy, void* lua, const std::int32_t* key,
                                       const float& clock_00f876a4, LuaPropertyHost& host);

// 00742A70, __fastcall(ECX out string {len, data}, EDX n) -> out: the Roman
// numeral of n for 1 <= n <= 3999 (table 00E0CC68 = thousands 00E0CC5C,
// hundreds 00E0CC38, tens 00E0CC14, units 00E0CBF0), else "N/A". Built in a
// temporary native string: each digit's text is assigned (0041E870), appended
// (0041DD40 to len+n, memcpy) and released; then copied into out (0041DD40 +
// memcpy when the length is non-zero; out is zeroed first) and the temporary
// released. ADOPTION: game_hosts_units.cpp:11918 and plane_squadron.cpp:62
// use decimal digits for this suffix.
struct NativeStringHost {
    virtual ~NativeStringHost() = default;
    virtual void assign_0041e870(std::int32_t* string, const char* text) = 0;
    virtual void resize_0041dd40(std::int32_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
};
extern const char* const* const kRomanDigitTable_00e0cc68[4];
std::int32_t* land_convoy_member_name_00742a70(std::int32_t* out, std::int32_t n, NativeStringHost& host);

// 007412F0, __fastcall(ECX slot, EDX value): when *slot is set, 00926D90(ECX
// *slot, value) and *slot = 0.
struct EntityKillHost {
    virtual ~EntityKillHost() = default;
    virtual void kill_00926d90(std::uint32_t entity, std::uint32_t value) = 0;
};
void entity_ref_kill_and_clear_007412f0(std::uint32_t* slot, std::uint32_t value, EntityKillHost& host);
// 00741310, __fastcall(ECX slot): free(*slot) (00BF65AC) and clear when set.
void free_and_null_00741310(void** slot, ConvoyMessageHost& host);

// ---------------------------------------------------------------------------
// LANDFORT_CREATE (type D0h, vtable 00CFF358, 118h bytes): the entity-create
// head plus an entity handle reference at +110h (vtable 00CFF354, present
// byte +4 = +114h, id word +6 = +116h).
// ---------------------------------------------------------------------------
struct EntityHandleRef {
    std::uint32_t vtable_00{0};
    std::uint8_t present_04{0};
    std::uint8_t pad_05{0};
    std::uint16_t id_06{0};
};
inline constexpr std::uint32_t kLandFortCreateVtable_00cff358 = 0x00CFF358u;
inline constexpr std::uint32_t kEntityHandleRefVtable_00cff354 = 0x00CFF354u;
struct LandFortCreateMessage {
    net_cc13::EntityCreateMessage base;
    std::uint32_t vtable_00{0};
    EntityHandleRef handle_110;
};
struct FortMessageHost {
    virtual ~FortMessageHost() = default;
    // 00809290, __thiscall(ECX handle, const handle-source* (fort+72Ch or 0)) RET 4.
    virtual void handle_assign_00809290(EntityHandleRef& handle, std::uint8_t* source) = 0;
    virtual void base_destruct_007668e0(LandFortCreateMessage& m) = 0;
    virtual void send_to_peer_00779fc0(std::uint8_t* entity, std::uint32_t peer, LandFortCreateMessage& m) = 0;
};
// 00743DB0, __thiscall(ECX message, word, fort) RET 8 -> message: base
// 00766FC0(D0h, word, fort, 1), vtables, then 00809290(fort+72Ch or 0).
LandFortCreateMessage& land_fort_create_construct_00743db0(
    LandFortCreateMessage& m, std::uint16_t word, std::uint8_t* fort, const net_cc13::EntityCreateSource& source,
    net_cc13::EntityCreateHost& base_host, FortMessageHost& host);
// 00743F90, __thiscall(ECX fort, peer) RET 4: message with word
// [[fort+538h]+70h], 00779FC0, inline destroy (vtables, 007668E0).
void land_fort_send_create_00743f90(std::uint8_t* fort, std::uint32_t peer, const net_cc13::EntityCreateSource& source,
                                    net_cc13::EntityCreateHost& base_host, FortMessageHost& host);
// 00744760, __thiscall(ECX handle, cursor) RET 4: present bit, then the id in
// 13 bits when present.
void entity_handle_write_00744760(const EntityHandleRef& handle, NativeBitCursor* cursor);
// 007447C0, RET 4, ECX ignored: t == D0h || t == 47h || t == 46h.
bool land_fort_create_is_type_007447c0(std::uint32_t type);
// 007447F0 / 00744830, __thiscall(ECX message, cursor|stream) RET 4: base
// codec (00765000 / 00767E30), then the handle (inlined 00744760 shape).
void land_fort_create_write_007447f0(const LandFortCreateMessage& m, NativeBitCursor* cursor,
                                     const NativeBitNumericContext& numeric);
void land_fort_create_read_00744830(LandFortCreateMessage& m, NativeSessionReadStream& stream,
                                    const NativeBitNumericContext& numeric);
// 007474E0, __fastcall(ECX message) -> unit (vtable slot +1Ch of 00CFF358):
// class = 00964790(ECX class id +1Eh, DL 1); unit = class->vtable[28h](id
// +1Ch); copy the message name into unit+154h (0041DD40 with flag 0, memcpy of
// len bytes when data is set); 00876EC0(ECX unit, 0).
struct FortCreateEntityHost {
    virtual ~FortCreateEntityHost() = default;
    virtual std::uint8_t* vehicle_class_00964790(std::uint16_t class_id, bool create) = 0;
    virtual std::uint8_t* class_create_unit_28(std::uint8_t* vehicle_class, std::uint16_t id) = 0;
    virtual void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void set_model_group_state_00876ec0(std::uint8_t* unit, std::uint32_t state) = 0;
};
std::uint8_t* land_fort_create_entity_007474e0(const LandFortCreateMessage& m, FortCreateEntityHost& host);

// LANDFORT_EFFECT (type D1h, vtable 00CFF37C, 2Ch): entity header (id word
// +18h, relay +1Ah), effect kind +1Ch (3 bits), value +20h (3 bits) or a float
// +24h, selector byte +28h (1: the float form).
struct LandFortEffectMessage {
    NativeSessionMessageStorage storage;
    std::uint16_t id_18;
    std::uint8_t relay_1a;
    std::uint8_t pad_1b;
    std::uint32_t kind_1c;
    std::uint32_t value_20;
    float point_24;
    std::uint8_t has_point_28;
    std::uint8_t pad_29[3];
};
#if defined(_M_IX86)
static_assert(sizeof(LandFortEffectMessage) == 0x2C);
#endif
struct SessionMessageHost {
    virtual ~SessionMessageHost() = default;
    // The profile the host uses for the native vtable at image_address.
    virtual const std::uint32_t* vtable(std::uint32_t image_address) = 0;
    virtual void write_header_0075b480(const void* message, NativeBitCursor* cursor) = 0;  // ECX message
    virtual void read_header_0075b4c0(void* message, NativeSessionReadStream& stream) = 0;
};
// 00744870 / 007448B0, __thiscall(ECX message, kind, value|point) RET 8:
// 0075B430(D1h), header zeros, delivery 1, vtable, the two arguments, the
// selector (0 / 1). 00744870 leaves +24h unset; 007448B0 leaves +20h unset.
LandFortEffectMessage& land_fort_effect_construct_00744870(LandFortEffectMessage& m, std::uint32_t kind,
                                                           std::uint32_t value, const NativeSessionMessageContext& ctx,
                                                           SessionMessageHost& host);
LandFortEffectMessage& land_fort_effect_construct_point_007448b0(LandFortEffectMessage& m, std::uint32_t kind,
                                                                 float point, const NativeSessionMessageContext& ctx,
                                                                 SessionMessageHost& host);
// 00744920 / 00744990, __thiscall(ECX message, cursor|stream) RET 4: header,
// selector bit, kind (3 bits), then the float (raw 32) or the value (3 bits).
void land_fort_effect_write_00744920(const LandFortEffectMessage& m, NativeBitCursor* cursor,
                                     const NativeBitNumericContext& numeric, SessionMessageHost& host);
void land_fort_effect_read_00744990(LandFortEffectMessage& m, NativeSessionReadStream& stream,
                                    const NativeBitNumericContext& numeric, SessionMessageHost& host);

// ---------------------------------------------------------------------------
// MLandFort (class 1Bh, 758h bytes, primary vtable 00CFF3F8).
// ---------------------------------------------------------------------------
struct LandFortHost : RefCountedHost {
    // 0095CC90, __thiscall(ECX unit, row, 0) RET 8: the unit game-object base.
    virtual void unit_base_construct_0095cc90(std::uint8_t* unit, std::uint32_t row, std::uint32_t zero) = 0;
    // 00809270 / 00809650, ECX = fort+72Ch: the owned-reference slot ctor / dtor.
    virtual void ref_slot_construct_00809270(std::uint8_t* slot) = 0;
    virtual void ref_slot_destruct_00809650(std::uint8_t* slot) = 0;
    // 00867B10, ECX = effect: stop.
    virtual void effect_stop_00867b10(std::uint8_t* effect) = 0;
    // 00C34F70, __thiscall(ECX [[00E188A8]+18h], body) RET 4: remove a static body.
    virtual void physics_remove_body_00c34f70(std::uint8_t* world, std::uint32_t body) = 0;
    virtual void unit_base_destruct_00959940(std::uint8_t* unit) = 0;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual void set_vehicle_class_009553d0(std::uint8_t* unit, std::uint32_t vehicle_class) = 0;
    virtual void attach_lua_self_0077e830(std::uint8_t* unit) = 0;
    // 008363E0, __thiscall(ECX memory, owner, 0) RET 8 -> weapon director.
    virtual std::uint32_t weapon_director_construct_008363e0(void* memory, std::uint8_t* owner, std::uint32_t zero) = 0;
    // 0095A880, __thiscall(ECX unit, 0, 1, 0) RET 0Ch.
    virtual void instantiate_class_devices_0095a880(std::uint8_t* unit, std::uint32_t a, std::uint32_t b,
                                                    std::uint32_t c) = 0;
};
// 00745940, __thiscall(ECX fort, row) RET 4 -> fort.
std::uint8_t* land_fort_construct_00745940(std::uint8_t* fort, std::uint32_t row, LandFortHost& host);
// 00745AA0, __fastcall(ECX fort): the destructor body.
void land_fort_destruct_00745aa0(std::uint8_t* fort, void* const& game_00e188a8, LandFortHost& host);
// 00747000, __thiscall(ECX vehicle class, row) RET 4 -> fort: operator new
// 758h (00BF55BE), memset 0 (before the null test), construct, 009553D0(ECX
// fort, class), +354h = class.
std::uint8_t* land_fort_allocate_00747000(std::uint32_t vehicle_class, std::uint32_t row, LandFortHost& host);
// 00743E90, __fastcall(ECX fort): 0077E830, weapon director (operator new
// 240h; 008363E0(fort+310h or 0, 0)) into +74Ch, then 0095A880(0, 1, 0) when
// [[fort+C0h]+4] == 1.
void land_fort_init_lua_and_weapons_00743e90(std::uint8_t* fort, LandFortHost& host);

// 00744A90, __thiscall(ECX fort, order message) RET 4: copy the message's
// command target (+24h word, +26h word, +28h dword, +2Ch..+38h floats) into a
// local record, read the command (0077A050, ECX message); command 00E08EF8:
// 00835860(ECX director +74Ch, 00521EA0(ECX &record), 1); commands 00E08F00 /
// 00E08F08: 00835860(director, 0, 1).
struct CommandTargetRecord {
    std::uint16_t word_00, word_02;
    std::uint32_t dword_04;
    float floats_08[4];
};
struct FortOrderHost {
    virtual ~FortOrderHost() = default;
    virtual std::uint32_t command_from_ordinal_0077a050(const std::uint8_t* message) = 0;
    virtual std::uint32_t resolve_target_00521ea0(const CommandTargetRecord& record) = 0;
    virtual void set_fire_target_00835860(std::uint32_t director, std::uint32_t target, std::uint32_t flag) = 0;
};
void land_fort_apply_fire_target_order_00744a90(std::uint8_t* fort, const std::uint8_t* message, FortOrderHost& host);

// Doubly linked node list {count +0, head +4, tail +8}; nodes {prev +0, next
// +4}, freed through 00BF65AC.
struct LinkedNodeList {
    std::int32_t count_00;
    std::uint32_t* head_04;
    std::uint32_t* tail_08;
};
struct FreeHost {
    virtual ~FreeHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
};
// 00744F70, __thiscall(ECX list, node) RET 4 -> the following node (or null).
std::uint32_t* linked_list_unlink_00744f70(LinkedNodeList& list, std::uint32_t* node, FreeHost& host);
// 00745200, __fastcall(ECX list): unlink and free the head until count is 0.
void linked_list_clear_00745200(LinkedNodeList& list, FreeHost& host);

// 00745130, __thiscall(ECX effect, const float anchor[3]) RET 4: copy the anchor
// to +100h..+108h (FLD/FSTP); with a parent +8Ch: refresh its world matrix
// (00B6DB70) unless parent+5Ch bit 1 is set, then [+110h]->vtable[34h] with
// 00413920(ECX effect+D0h, &local, parent+F0h); without: vtable[34h](effect+D0h).
struct EffectAnchorHost {
    virtual ~EffectAnchorHost() = default;
    virtual void refresh_world_matrix_00b6db70(std::uint8_t* node) = 0;
    virtual const float* matrix_multiply_00413920(const float* left, float* out, const float* right) = 0;
    virtual void set_transform_34(std::uint8_t* emitter, const float* matrix) = 0;  // [+110h]->vtable[34h]
};
void land_fort_set_effect_anchor_00745130(std::uint8_t* effect, const float* anchor, EffectAnchorHost& host);

// 007453C0, __thiscall(ECX reader, key_tag, key_value, dword* out) RET 0Ch
// (the first two stack dwords are an 8-byte {tag, value} key passed by value):
// reader->vtable[10h](key{key_tag, key_value}, value{4, &local}) RET 10h,
// then *out = local. Listing 007453C0-007453F2: the value record is built at
// [esp] with tag 4 and a pointer to the dword slot at [esp+8]; after the
// callee pops 10h, [esp+10h] is the third stack argument (out) and [esp] is
// the local. The local starts out holding a stack address (MOV [ESP+8],ESP);
// the callee is expected to overwrite it, and that initial value is not
// modelled.
struct SaveReaderHost {
    virtual ~SaveReaderHost() = default;
    // The reader's own vtable[10h](key by value, value by value), ECX = reader, RET 10h.
    virtual void read_10(void* reader, std::uint32_t key_tag, std::uint32_t key_value, std::uint32_t value_tag,
                         void* value) = 0;
};
void save_reader_read_observer_ref_007453c0(void* reader, std::uint32_t key_tag, std::uint32_t key_value,
                                            std::uint32_t* out, SaveReaderHost& host);

// 007455F0, __thiscall(ECX fort, archive) RET 4: 0095BB10 (the unit part),
// then "_landFort" { ["gazdi"] = master +738h (tag 4); ["section"] = index of
// the master's +348h vector element whose ADDRESS equals +73Ch (tag 1, -1
// when none) } (only with a master), then "gameUnit" { 008360C0(director) }.
struct ArchiveWriter {
    virtual ~ArchiveWriter() = default;
    virtual void begin_04(std::int32_t tag, const void* value) = 0;
    virtual void end_08() = 0;
    virtual void write_0c(std::int32_t key_tag, const void* key, std::int32_t value_tag, std::uint32_t value) = 0;
};
struct FortArchiveHost {
    virtual ~FortArchiveHost() = default;
    virtual void unit_serialize_0095bb10(std::uint8_t* fort, ArchiveWriter& archive) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void director_visit_008360c0(std::uint32_t director, ArchiveWriter& archive) = 0;
};
void land_fort_serialize_007455f0(std::uint8_t* fort, ArchiveWriter& archive, FortArchiveHost& host);

// 00745FA0, __thiscall(ECX fort, index, anchor) RET 8: effect definition
// class+154h[index] (checked against class+150h's {first +4, last +8}),
// passed by value (AddRef) to 008687C0(ECX &out, EDX parent +4A4h, definition,
// identity 4x4, 1, 0, 0); the out reference is released at once, effect+9 = 1,
// and for anchor != 0 the effect is anchored at [class+28h] + anchor*0Ch.
struct FortEffectHost : RefCountedHost {
    virtual long interlocked_increment(volatile long* counter) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    // 008687C0 / 008689C0, __fastcall(ECX out slot, EDX parent, definition by
    // value, matrix...) -> out slot; the callee owns the passed reference.
    virtual RefCounted** effect_create_matrix_008687c0(RefCounted** out, std::uint8_t* parent, RefCounted* definition,
                                                       const float* matrix, std::uint32_t a, std::uint32_t b,
                                                       std::uint32_t c) = 0;
    virtual RefCounted** effect_create_008689c0(RefCounted** out, std::uint8_t* parent, RefCounted* definition,
                                                const float* matrix) = 0;
    virtual void effect_stop_00867b10(std::uint8_t* effect) = 0;
    // 00484620(ECX slot, 0) reset / 004845D0(ECX slot, RefCounted** source) assign.
    virtual void ref_slot_reset_00484620(std::uint8_t** slot, std::uint32_t value) = 0;
    virtual void ref_slot_assign_004845d0(std::uint8_t** slot, RefCounted** source) = 0;
    // 00744C40, __thiscall(ECX class+18h, 0, 1, 0, 0) RET 10h -> record; the
    // effect matrix is record+18h.
    virtual std::uint8_t* class_effect_frame_00744c40(std::uint8_t* class_18, std::uint32_t a, std::uint32_t b,
                                                      std::uint32_t c, std::uint32_t d) = 0;
};
void land_fort_create_damage_effect_00745fa0(std::uint8_t* fort, std::uint32_t index, std::uint32_t anchor,
                                             FortEffectHost& host, EffectAnchorHost& anchor_host);
// 007460E0, __thiscall(ECX fort, kind, float duration) RET 8 (vtable +208h,
// the D1h handler's float form). kind 7: stop and reset the fire +740h, then
// unless the smoke +744h is already set, create class+160h (when set) into
// +744h and store +748h = duration. Other kinds: stop and reset the smoke,
// then unless the fire is set, create class+144h[kind] into +740h and store
// +748h. The matrix is 00744C40(class+18h, 0, 1, 0, 0) + 18h.
void land_fort_create_state_effect_007460e0(std::uint8_t* fort, std::uint32_t kind, float duration,
                                            FortEffectHost& host);

// 007469B0, __fastcall(ECX object) and 00747C10, __stdcall(object) RET 4:
// return the native string {len +8, data +0Ch} storage when data is set.
struct StringReleaseHost {
    virtual ~StringReleaseHost() = default;
    virtual void release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
};
void release_string_storage_007469b0(std::uint8_t* object, StringReleaseHost& host);
void release_string_storage_00747c10(std::uint8_t* object, StringReleaseHost& host);
// 00746A00, __fastcall(ECX set): release the ten references +24h down to +0.
void ref_member_set_release_00746a00(RefCounted** set, RefCountedHost& host);
// 00747870 / 00747CA0 / 007480F0, __fastcall(ECX array): 005471B0(ECX, 0),
// then free(data) (00BF6989).
struct RefArrayHost {
    virtual ~RefArrayHost() = default;
    virtual void resize_005471b0(DwordVector& v, std::int32_t n) = 0;
    virtual void free_00bf6989(void* block) = 0;
};
void ref_array_tidy_00747870(DwordVector& v, RefArrayHost& host);

// 00747560, __fastcall(ECX node, EDX const float* factor), recursive: when
// node->vtable[0Ch](00B74310()) accepts the node, every element of every
// geometry (00B74650 count, 00B74640(i), 00B72B40 count, 00B732C0(j)) gets
// 00B18B20(ECX element+20h material, &"cShadowFactor", factor) and
// 00B179F0(ECX material, 0)->+0Ch = *factor; then every child (+34h, next
// +3Ch) recurses.
struct ShadowFactorHost {
    virtual ~ShadowFactorHost() = default;
    virtual std::uint32_t generated_model_type_00b74310() = 0;
    virtual bool node_is_kind_0c(std::uint8_t* node, std::uint32_t type) = 0;
    virtual std::int32_t geometry_count_00b74650(std::uint8_t* node) = 0;
    virtual std::uint8_t* geometry_00b74640(std::uint8_t* node, std::int32_t index) = 0;
    virtual std::int32_t element_count_00b72b40(std::uint8_t* geometry) = 0;
    virtual std::uint8_t* element_00b732c0(std::uint8_t* geometry, std::int32_t index) = 0;
    virtual void native_string_resize_0041dd40(std::int32_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
    virtual void material_register_float_00b18b20(std::uint8_t* material, const std::int32_t* name, const float* value) = 0;
    virtual std::uint8_t* material_color_00b179f0(std::uint8_t* material, std::uint32_t index) = 0;
};
void register_shadow_factor_00747560(std::uint8_t* node, const float* factor, ShadowFactorHost& host);

// 00748B40, __thiscall(ECX entity, kind) RET 4: kind == 36h || kind == 0 ||
// kind == entity+C4h.
bool stationary_is_kind_of_00748b40(const std::uint8_t* entity, std::uint32_t kind);
// 00749030, RET 4, ECX ignored: kind == 1Bh || 5 || 4.
bool land_fort_class_is_kind_of_00749030(std::uint32_t kind);
// 00749150, RET 4: returns 0 (the dummy-target class creates no instance).
std::uint32_t vehicle_class_create_instance_null_00749150(std::uint32_t unused);

struct StationaryHost : RefCountedHost {
    virtual void physics_remove_body_00c34f70(std::uint8_t* world, std::uint32_t body) = 0;
    virtual void entity_destruct_base_00925780(std::uint8_t* entity) = 0;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual std::uint8_t* stationary_prop_construct_00748a40(void* memory) = 0;
    virtual void prop_set_class_0084f7f0(std::uint8_t* prop, std::uint32_t vehicle_class) = 0;
};
// 00748B70, __fastcall(ECX prop): vtables 00CFF678 / +10h 00CFF65C / +24h
// 00CFF654; remove the body +1A8h from [[00E188A8]+18h] when set; release
// +184h; 00925780.
void stationary_prop_destruct_00748b70(std::uint8_t* prop, void* const& game_00e188a8, StationaryHost& host);
// 00748C40, __fastcall(ECX vehicle class) -> prop: new 1ACh, memset, 00748A40
// when allocated, 0084F7F0(ECX prop, class) unconditionally.
std::uint8_t* land_fort_allocate_stationary_prop_00748c40(std::uint32_t vehicle_class, StationaryHost& host);

struct FortClassHost : RefCountedHost {
    virtual void vehicle_class_base_construct_00749050(std::uint8_t* cls) = 0;
    virtual void vehicle_class_base_destruct_0095f1c0(std::uint8_t* cls) = 0;
    // 004CC760, ECX first, EDX last, push vector, push owner (callee pops).
    virtual void destroy_range_004cc760(std::uint32_t first, std::uint32_t last, std::uint8_t* vector,
                                        std::uint8_t* owner) = 0;
    virtual void free_00bf65ac(void* block) = 0;
};
// 00749180, __fastcall(ECX class) -> class: 00749050, vtables 00CFF790 and
// +6Ch 00CFF78C, zero +13Ch, +144h..+14Ch, +154h..+160h.
std::uint8_t* land_fort_class_construct_00749180(std::uint8_t* cls, FortClassHost& host);
// 00748EC0, __fastcall(ECX class): vtables, release +160h, destroy the vectors
// +150h and +140h (004CC760 + free, three dwords zeroed), release +13Ch,
// 0095F1C0.
void land_fort_class_destruct_00748ec0(std::uint8_t* cls, FortClassHost& host);

// ---------------------------------------------------------------------------
// MLandingShip (class 0Ch, 122Ch bytes, primary vtable 00CFFA30) and its
// messages. Messages are addressed as raw image storage here because their
// bases (00813A20 SHIP_CREATE, 0075B430) are native.
// ---------------------------------------------------------------------------
struct LandingShipHost : RefCountedHost {
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00939F40, __thiscall(ECX memory, unit) RET 4 -> motion controller.
    virtual std::uint32_t motion_controller_construct_00939f40(void* memory, std::uint8_t* unit) = 0;
    virtual void pad_release_006ac550(std::uint8_t* pad) = 0;         // ECX = ship+1200h
    virtual void ship_on_destroyed_00824b60(std::uint8_t* ship) = 0;  // JMP, ECX = ship
    // 00813A20, __thiscall(ECX message, word, ship, type) RET 0Ch; 00813B10
    // (ECX message, cursor) RET 4; 0080E380 (ECX message) destructor.
    virtual void ship_create_construct_00813a20(std::uint8_t* message, std::uint32_t word, std::uint8_t* ship,
                                                std::uint32_t type) = 0;
    virtual void ship_create_write_00813b10(const std::uint8_t* message, NativeBitCursor* cursor) = 0;
    virtual void ship_create_destruct_0080e380(std::uint8_t* message) = 0;
    virtual void send_to_peer_00779fc0(std::uint8_t* entity, std::uint32_t peer, std::uint8_t* message) = 0;
    virtual void ship_base_construct_0081ed40(std::uint8_t* ship, std::uint32_t row) = 0;
    virtual void ship_base_destruct_0081f3a0(std::uint8_t* ship) = 0;  // JMP
    virtual void ship_detach_base_006dffc0(std::uint8_t* ship) = 0;
    virtual void list_erase_004837d0(std::uint8_t* list, std::uint8_t* node) = 0;  // ECX list, node RET 4
    virtual void set_vehicle_class_009553d0(std::uint8_t* unit, std::uint32_t vehicle_class) = 0;
    // 0074BA20, __thiscall(ECX vector, n, 4Ch record by value) RET 50h.
    virtual void record4c_resize_0074ba20(std::uint8_t* vector, std::uint32_t n, const std::uint8_t (&fill)[0x4C]) = 0;
    virtual int strncmp_00bf75b3(const char* a, const char* b, std::size_t n) = 0;
};
// 00749790, __fastcall(ECX unit): +1018h = new 390h (00BF681B) constructed by
// 00939F40(unit), or 0 when the allocation fails.
void create_motion_controller_00749790(std::uint8_t* unit, LandingShipHost& host);
// 007497F0, __fastcall(ECX ship): 006AC550(ECX +1200h) when set, then JMP 00824B60.
void landing_ship_on_destroyed_007497f0(std::uint8_t* ship, LandingShipHost& host);
// 00749AA0, __fastcall(ECX message) -> message: 0075B430(A6h), id/relay 0,
// delivery 1, vtable 00CFF8D4 (LANDINGSHIP_OPEN).
std::uint8_t* landing_ship_open_construct_00749aa0(std::uint8_t* message, const NativeSessionMessageContext& ctx,
                                                   SessionMessageHost& host);
// 00749AD0, RET 4: t == A6h || t == 46h.
bool landing_ship_open_is_type_00749ad0(std::uint32_t type);
// 00749BF0, __thiscall(ECX message, ship, a, b, c) RET 10h: 00813A20(word
// [[ship+538h]+70h], ship, A4h), vtable 00CFF8E8, +12Ch = id(a) or 0, +12Eh =
// id(b) or 0, +130h = c.
std::uint8_t* landing_ship_create_construct_00749bf0(std::uint8_t* message, std::uint8_t* ship, std::uint8_t* a,
                                                     std::uint8_t* b, std::uint32_t c, LandingShipHost& host);
// 00749C70, RET 4: t == 8Ah || t == 47h || t == 46h (the SHIP_CREATE family).
bool ship_create_is_type_00749c70(std::uint32_t type);
// 00749CD0, __thiscall(ECX message, cursor) RET 4: 00813B10, then +12Ch (13
// bits), +12Eh (12 bits), +130h signed in 4 bits.
void landing_ship_create_write_00749cd0(const std::uint8_t* message, NativeBitCursor* cursor, LandingShipHost& host);
// 00749D90, __thiscall(ECX message, pad, building) RET 8: 0075B430(A5h),
// header zero, delivery 1, vtable 00CFF908, +1Ch = id(pad) or 0, +1Eh =
// id(building) or 0.
std::uint8_t* land_at_pad_construct_00749d90(std::uint8_t* message, std::uint8_t* pad, std::uint8_t* building,
                                             const NativeSessionMessageContext& ctx, SessionMessageHost& host);
// 0074A5A0, __thiscall(ECX ship, peer) RET 4: LANDINGSHIP_CREATE(ship, +1200h,
// +1204h, +1208h) on the stack, 00779FC0, 0080E380.
void landing_ship_replicate_0074a5a0(std::uint8_t* ship, std::uint32_t peer, LandingShipHost& host);
// 0074A8D0, __fastcall(ECX unit) -> bool: true when the class byte +809h is
// set; otherwise over the twelve lists +394h + i*0Ch {count, head +4} with i
// in {2, 3, 4, 6} and a non-zero first dword, count every node (next +4,
// entity +8) and those whose [[[entity+354h]+74h]+34h]+1Ch name (default
// 00E1AEC2) starts with "Rocket"; true when total > 7 and 2*rockets > total.
bool unit_is_rocket_armed_0074a8d0(const std::uint8_t* unit, const char* empty_00e1aec2, LandingShipHost& host);
// 0074BB00, __thiscall(ECX ship, row) RET 4 -> ship.
std::uint8_t* landing_ship_construct_0074bb00(std::uint8_t* ship, std::uint32_t row, LandingShipHost& host);
// 0074BD20, __fastcall(ECX ship): 006DFFC0, then erase the ship's node from
// the list [ship+30h]+A8h (head +4, node next +4, value +8) when present.
void landing_ship_detach_0074bd20(std::uint8_t* ship, LandingShipHost& host);

struct MotherShipDetachHost {
    virtual ~MotherShipDetachHost() = default;
    // 006DFFC0: ECX=ship, no stack arguments, RET0. Performs the early
    // detach followed by the world's +54h and +60h list removals.
    virtual void unit_instance_detach_mid_lists_006dffc0(std::uint8_t* ship) = 0;
    // 004837D0: ECX=actual list, one actual node, RET4. Unlink/free the node;
    // the borrowed ship payload is untouched. The unused EAX result is successor.
    virtual void list_erase_004837d0(std::uint8_t* list, std::uint8_t* node) = 0;
};
// Complete normal 00758FE0..00759015, __thiscall ECX=ship, RET0, no delete
// flag. After the mid-list detach, re-read ship+30h and scan world+84h's head
// at+88h, following node+4h, for the first node whose+8h equals ship. Erase
// only that node. No world-null guard, replacement list, or game host wiring.
void mothership_detach_from_world_lists_00758fe0(std::uint8_t* ship, MotherShipDetachHost& host);
// 0074BD60, __fastcall(ECX ship): free +1198h, zero +1198h..+11A0h, JMP 0081F3A0.
void landing_ship_destruct_0074bd60(std::uint8_t* ship, LandingShipHost& host);
// 0074BE00, __thiscall(ECX class, row) RET 4 -> ship: new 122Ch (00BF55BE),
// memset, construct, 009553D0(class), +354h = class.
std::uint8_t* landing_ship_create_0074be00(std::uint32_t vehicle_class, std::uint32_t row, LandingShipHost& host);
// 0074BE90, __thiscall(ECX vector, n) RET 4: 0074BA20(n, a 4Ch record copied
// from the caller's UNINITIALISED stack); the C++ passes an uninitialised
// local byte array the same way.
void record4c_resize_tail_0074be90(std::uint8_t* vector, std::uint32_t n, LandingShipHost& host);

// ---------------------------------------------------------------------------
// MLandVehicle (class 19h, 740h bytes, primary vtable 00CFFDE0).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGroundVehicleCreateVtable_00cffd18 = 0x00CFFD18u;
struct GroundVehicleCreateMessage {  // type CDh: the entity-create head only
    net_cc13::EntityCreateMessage base;
    std::uint32_t vtable_00{0};
};
// The by-value {tag, word} pair the archive and Lua-reader virtuals take.
struct VariantPair {
    std::int32_t tag;
    std::uint32_t value;
};
struct LandVehicleHost : RefCountedHost {
    virtual long interlocked_increment(volatile long* counter) = 0;
    virtual void attach_lua_self_0077e830(std::uint8_t* unit) = 0;
    virtual void instantiate_class_devices_0095a880(std::uint8_t* unit, std::uint32_t a, std::uint32_t b,
                                                    std::uint32_t c) = 0;
    virtual void convoy_member_died_007422a0(std::uint8_t* convoy, std::uint8_t* vehicle) = 0;  // ECX convoy
    virtual void unit_on_death_00951fb0(std::uint8_t* vehicle) = 0;                             // JMP
    virtual void base_destruct_007668e0(GroundVehicleCreateMessage& m) = 0;
    virtual void send_to_peer_00779fc0(std::uint8_t* entity, std::uint32_t peer, void* message) = 0;
    virtual void release_peer_00779f80(std::uint8_t* entity, const std::uint32_t* peer_ref) = 0;
    // 00927850, __thiscall(ECX vehicle, value) RET 4 -> byte.
    virtual std::uint8_t party_00927850(std::uint8_t* vehicle, std::uint32_t value) = 0;
    // 0077B280, __cdecl, 15 arguments (ADD ESP,3Ch).
    virtual void build_sync_0077b280(std::uint8_t* vehicle, const std::uint32_t* peer_ref, std::uint8_t party,
                                     float a, std::uint32_t b, float c, std::uint32_t d, float e, std::uint32_t f,
                                     float g, std::uint32_t h, float i, std::uint32_t j, float k,
                                     std::uint32_t l) = 0;
    virtual void matrix_set_004134f0(std::uint8_t* matrix, const float* source) = 0;          // ECX matrix
    virtual void motion_resync_008746a0(std::uint8_t* motion, std::uint32_t ticks) = 0;       // ECX unit+310h
    virtual void motion_step_slot04(std::uint8_t* motion, float seconds) = 0;                 // [+310h]->vtable[4]
    virtual void points_reserve_0074d190(std::uint8_t* vector, std::int32_t n) = 0;          // ECX vector
    virtual void unit_serialize_0095bb10(std::uint8_t* unit, ArchiveWriter& archive) = 0;
    virtual std::uint8_t* vehicle_class_00964790(std::uint16_t class_id, bool create) = 0;
    virtual std::uint8_t* class_create_unit_28(std::uint8_t* vehicle_class, std::uint16_t id) = 0;
    virtual void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void class_bind_base_0095f500(std::uint8_t* cls) = 0;
    // 00408720, __thiscall(ECX std::string, const char*, length) RET 8.
    virtual void std_string_assign_00408720(std::uint8_t* string, const char* text, std::uint32_t length) = 0;
    // 00718870, __thiscall(ECX model, const std::string*, index) RET 8 -> node.
    virtual std::uint8_t* model_find_node_00718870(std::uint8_t* model, const std::uint8_t* name, std::uint32_t index) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void refresh_pose_00414db0(std::uint8_t* unit) = 0;
    // 008685E0, __fastcall(ECX out slot, EDX scene, definition by value, const
    // float* position, 0, 0) -> out slot.
    virtual RefCounted** effect_create_at_008685e0(RefCounted** out, std::uint32_t scene, RefCounted* definition,
                                                   const float* position, std::uint32_t a, std::uint32_t b) = 0;
    virtual void sound_play_0047bb70(std::uint32_t sounds, std::uint32_t sound, std::uint8_t* unit) = 0;  // ECX sounds
    virtual void unit_destroyed_00959450(std::uint8_t* unit) = 0;
    virtual void effect_stop_00867b10(std::uint8_t* effect) = 0;
    virtual void effect_vector_resize_004c9550(std::uint8_t* vector, std::int32_t n) = 0;  // ECX vector
    virtual void kill_00926d90(std::uint8_t* unit, std::uint32_t value) = 0;
    virtual void unit_base_construct_0095cc90(std::uint8_t* unit, std::uint32_t row, std::uint32_t zero) = 0;
    virtual void unit_base_destruct_00959940(std::uint8_t* unit) = 0;
    virtual void ai_query_008ddf00(std::uint32_t ai, std::uint8_t* unit) = 0;  // ECX ai
    virtual void unit_detach_base_006d3620(std::uint8_t* unit) = 0;
    virtual void list_erase_004837d0(std::uint8_t* list, std::uint8_t* node) = 0;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual void set_vehicle_class_009553d0(std::uint8_t* unit, std::uint32_t vehicle_class) = 0;
    // Saved-state loading: 0095E5B0, 009238A0 (ECX unit, out 14h record),
    // 004425C0 (ECX reader, 14h record by value), 00BD8E20 enter, the reader's
    // vtable+10h read, 00BD6830 read, 00BD7A20 leave, 00441A20 destroy,
    // unit vtable+148h(1FFh, 8).
    virtual void unit_load_common_0095e5b0(std::uint8_t* unit) = 0;
    virtual void saved_lua_data_009238a0(std::uint8_t* unit, std::uint8_t (&record)[0x14]) = 0;
    virtual void lua_reader_construct_004425c0(std::uint8_t* reader, const std::uint8_t (&record)[0x14]) = 0;
    virtual void lua_reader_enter_00bd8e20(std::uint8_t* reader, VariantPair key) = 0;
    virtual void lua_reader_read_10(std::uint8_t* reader, VariantPair key, VariantPair field) = 0;
    virtual void lua_reader_read_00bd6830(std::uint8_t* reader, VariantPair key, VariantPair field) = 0;
    virtual void lua_reader_leave_00bd7a20(std::uint8_t* reader) = 0;
    virtual void lua_reader_destroy_00441a20(std::uint8_t* reader) = 0;
    virtual void unit_vslot_148(std::uint8_t* unit, std::uint32_t a, std::uint32_t b) = 0;
};
// 0074CCF0, __fastcall(ECX vehicle): 0077E830, then 0095A880(0, 1, 0).
void land_vehicle_init_lua_and_devices_0074ccf0(std::uint8_t* vehicle, LandVehicleHost& host);
// 0074CD10, __fastcall(ECX vehicle): 007422A0(ECX convoy +738h, vehicle) when
// in a convoy, then JMP 00951FB0. (land_and_structures.hpp describes the
// convoy side.)
void land_vehicle_on_death_0074cd10(std::uint8_t* vehicle, LandVehicleHost& host);
// 0074CDB0, __thiscall(ECX vehicle, peer) RET 4: stack CDh message (00766FC0
// with word [[v+538h]+70h], v, 1; vtable 00CFFD18), 00779FC0, vtable, 007668E0.
void land_vehicle_send_create_0074cdb0(std::uint8_t* vehicle, std::uint32_t peer,
                                       const net_cc13::EntityCreateSource& source,
                                       net_cc13::EntityCreateHost& base_host, LandVehicleHost& host);
// 0074D030, __thiscall(ECX vehicle, const dword* peer_ref) RET 4 (vtable
// +180h): party = 00927850(peer ? [peer+20h] : 8); 0077B280(vehicle, peer_ref,
// party, 0.0f, 1, 200.0f [00CE386C], 3, 500.0f [00CE397C], 5, 1000.0f
// [00CE3804], 12, 2000.0f [00CFFD60], 20, FLT_MAX [00D7A248], 100).
void land_vehicle_replicate_sync_0074d030(std::uint8_t* vehicle, const std::uint32_t* peer_ref, LandVehicleHost& host);
// 0074D100, __thiscall(ECX vehicle, const dword* peer_ref, unused) RET 8:
// stack CEh message (0075B430, header 0, delivery 0, vtable 00CFFD4C),
// 00779FC0(vehicle, *peer_ref, &msg), vtable reset to 00CE4974, 00779F80(peer_ref).
void land_vehicle_send_sync_0074d100(std::uint8_t* vehicle, const std::uint32_t* peer_ref, std::uint32_t unused,
                                     const NativeSessionMessageContext& ctx, SessionMessageHost& messages,
                                     LandVehicleHost& host);
// 0074D230, __thiscall(ECX vehicle, const message*) RET 4: 004134F0(ECX
// vehicle+674h, identity), 008746A0(ECX vehicle+310h, [00F876B0] -
// [message+0Ch]), then [+310h]->vtable[4]([00F876AC] as float).
void land_vehicle_apply_sync_0074d230(std::uint8_t* vehicle, const std::uint8_t* message, const std::uint32_t& tick_00f876b0,
                                      const float& step_00f876ac, LandVehicleHost& host);
// 0074D2F0 / 0074D450 (identical), __thiscall(ECX vector of 0Ch points, const
// float[3]) RET 4: growth through 0074D190, FLD/FSTP copy, ++size.
void points_push_back_0074d2f0(std::uint8_t* vector, const float* point, LandVehicleHost& host);
// 0074D3B0, __thiscall(ECX vehicle, archive) RET 4: 0095BB10, begin
// "_landVehicle", "convoyID" {4, +738h}, "convoyPosition" {1, +73Ch}, end.
void land_vehicle_serialize_0074d3b0(std::uint8_t* vehicle, ArchiveWriter& archive, LandVehicleHost& host);
// 0074D710, __fastcall(ECX message) -> unit (vtable +1Ch of 00CFFD18): the
// 007474E0 body without the final 00876EC0.
std::uint8_t* ground_vehicle_create_entity_0074d710(const GroundVehicleCreateMessage& m, LandVehicleHost& host);
// 0074DA10, __fastcall(ECX class): 0095F500, then for i = 1, 2, ...: node =
// 00718870(ECX [class+50h], std::string "kemeny" (00CFFD90, 00408720 with
// length 6), i); the temporary std::string is destroyed (free 00BF65AC when its
// capacity is >= 10h); stop at a null node; else append the first vertex of
// node+48h (12-byte elements, 00BF6713 when empty) to the points vector
// class+140h (0074D190 growth).
void land_vehicle_class_bind_model_0074da10(std::uint8_t* cls, LandVehicleHost& host);
// 0074DB70, __fastcall(ECX vehicle) (vtable death hook): when +70h == 1:
// 00414DB0 unless +C8h, explosion effect class+138h at +FCh (008685E0 with
// scene [[00E188A8]+19ECh]), +9 = 1, sound class+13Ch through 0047BB70(ECX
// [[00E188A8]+21D0h]) when set; then 00959450; stop and release every effect in
// the vector +72Ch (count +730h); 004C9550(ECX +72Ch, 0); 00926D90(+70h).
void land_vehicle_on_destroyed_0074db70(std::uint8_t* vehicle, void* const& game_00e188a8, LandVehicleHost& host);
// 0074DCC0, __thiscall(ECX vehicle, row) RET 4 -> vehicle.
std::uint8_t* land_vehicle_construct_0074dcc0(std::uint8_t* vehicle, std::uint32_t row, LandVehicleHost& host);
// 0074DD90, __thiscall(ECX unit, kind) RET 4: kind in {19h, 5, 4, 2, 1, 0} or kind == unit+C4h.
bool land_vehicle_is_kind_of_0074dd90(const std::uint8_t* unit, std::uint32_t kind);
// 0074DDD0, __fastcall(ECX vehicle): 008DDF00(ECX [game+21A4h + [game+18ECh]*4], vehicle).
void land_vehicle_vslot_b4_0074ddd0(std::uint8_t* vehicle, void* const& game_00e188a8, LandVehicleHost& host);
// 0074DE50, __fastcall(ECX vehicle): 006D3620, then erase the vehicle's node
// from [v+30h]+54h and from [v+30h]+144h (head +4, next +4, value +8).
void land_vehicle_detach_0074de50(std::uint8_t* vehicle, LandVehicleHost& host);
// 0074DEB0, __fastcall(ECX vehicle): 004C9550(ECX +72Ch, 0), free(+72Ch)
// (00BF6989), 00959940.
void land_vehicle_destruct_0074deb0(std::uint8_t* vehicle, LandVehicleHost& host);
// 0074DF10, __thiscall(ECX class, row) RET 4 -> vehicle: new 740h, memset,
// 0074DCC0, 009553D0(class), +354h = class.
std::uint8_t* land_vehicle_allocate_0074df10(std::uint32_t vehicle_class, std::uint32_t row, LandVehicleHost& host);
// 0074DFC0, __fastcall(ECX vehicle): 0095E5B0; when [+C0h]+4 == 3 read
// "_landVehicle" { convoyID (tag 4) -> +738h, convoyPosition (tag 1) -> +73Ch }
// from the saved Lua data; then vtable+148h(1FFh, 8).
void land_vehicle_load_saved_state_0074dfc0(std::uint8_t* vehicle, LandVehicleHost& host);

// ---------------------------------------------------------------------------
// Plane units (recon plane 14h, large recon plane 16h) and plane slots.
// ---------------------------------------------------------------------------
struct PlaneUnitHost {
    virtual ~PlaneUnitHost() = default;
    virtual void plane_base_construct_007cfd20(std::uint8_t* plane, std::uint32_t row, std::uint32_t zero) = 0;
    // vtable+38h, x87 ST0 return compared unrounded (a double carries it here).
    virtual double speed_ratio_38(std::uint8_t* plane) = 0;
    virtual void set_forward_speed_007d9e80(std::uint8_t* controller, float speed) = 0;  // ECX plane+AB0h
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual void set_vehicle_class_009553d0(std::uint8_t* unit, std::uint32_t vehicle_class) = 0;
    virtual void register_base_00928560(std::uint8_t* unit) = 0;
    virtual void list_insert_00484540(std::uint8_t* list, std::uint8_t* unit) = 0;  // ECX list, unit RET 4
    virtual void unit_detach_base_006d3620(std::uint8_t* unit) = 0;
    virtual void list_erase_004837d0(std::uint8_t* list, std::uint8_t* node) = 0;
    virtual void free_00bf6989(void* block) = 0;
};
// 0074E0F0, __thiscall(ECX plane, row) RET 4 -> plane: 007CFD20(row, 0),
// eight vtables (00D00070 .. +72Ch 00CFFFE8), +C4h = 14h, +AACh = 0Bh.
std::uint8_t* recon_plane_construct_0074e0f0(std::uint8_t* plane, std::uint32_t row, PlaneUnitHost& host);
// 0074E2D0, __thiscall(ECX plane, row) RET 4 -> plane: 0074E0F0, then the large
// recon plane vtables (00D00308 .. +72Ch 00D00280), +C4h = 16h, +AACh = 0Bh.
std::uint8_t* large_recon_plane_construct_0074e2d0(std::uint8_t* plane, std::uint32_t row, PlaneUnitHost& host);
// 0074E540, __thiscall(ECX class, row) RET 4 -> plane: new E94h, memset,
// 0074E2D0, 009553D0(class), +354h = class.
std::uint8_t* large_recon_plane_create_0074e540(std::uint32_t vehicle_class, std::uint32_t row, PlaneUnitHost& host);
// 0074E160, __fastcall(ECX unit) -> [unit+808h].
std::uint32_t plane_unit_vslot1c0_0074e160(const std::uint8_t* unit);
// 0074E1A0, __fastcall(ECX plane) -> vtable+38h() > 3.0f ([00CE3854]):
// FLD const, FXCH, FCOMIP ST0,ST1 (bytes DF F1), JBE -> 0; NaN gives 0.
bool plane_unit_vslot10c_0074e1a0(std::uint8_t* plane, PlaneUnitHost& host);
// 0074E1E0, __thiscall(ECX plane, float speed) RET 4: 007D9E80(ECX plane+AB0h, speed).
void plane_set_forward_speed_0074e1e0(std::uint8_t* plane, float speed, PlaneUnitHost& host);
// 0074E210, __fastcall(ECX control sub-object) -> [+1D4h] == 7.
bool plane_control_mode_is_free_flight_0074e210(const std::uint8_t* control);
// 0074E440, __thiscall(ECX plane, unused) RET 4: false for states +900h in {7, 6, 4, 5, 2, 3}, else true.
bool plane_unit_state_admits_ai_0074e440(const std::uint8_t* plane, std::uint32_t unused);
// 0074E5D0, __fastcall(ECX plane): 00928560, then 00484540(ECX [p+30h] + 30h /
// 48h / 54h / CCh, plane).
void plane_unit_register_in_world_lists_0074e5d0(std::uint8_t* plane, PlaneUnitHost& host);
// 0074E610, __fastcall(ECX plane): 006D3620, then erase from [p+30h]+54h and +CCh.
void plane_unit_detach_0074e610(std::uint8_t* plane, PlaneUnitHost& host);
// 0074E6C0 / 0074E750, __fastcall(ECX plane): 0074E610, then erase from
// [p+30h]+108h (recon) / +120h (large recon).
void recon_plane_detach_0074e6c0(std::uint8_t* plane, PlaneUnitHost& host);
void large_recon_plane_detach_0074e750(std::uint8_t* plane, PlaneUnitHost& host);
// 0074E7F0, __fastcall(ECX sub-object at unit+10D4h): vtable 00D00514, free
// (00BF6989) +1Ch, +18h, +20h when set (not cleared).
void unit_subobject_10d4_destruct_0074e7f0(std::uint8_t* object, PlaneUnitHost& host);

// SHIP_SINK (type 92h, vtable 00D00518).
// 0074EAB0, __thiscall(ECX message, count, const float* values) RET 8:
// 0075B430(92h), header 0, delivery 1, +1Ch = count, values copied (FLD/FSTP)
// to +20h onward for count != 0 (unsigned).
std::uint8_t* ship_sink_construct_0074eab0(std::uint8_t* message, std::uint32_t count, const float* values,
                                           const NativeSessionMessageContext& ctx, SessionMessageHost& host);
// 0074EB00, RET 4: t == 92h || 46h.
bool ship_sink_is_type_0074eb00(std::uint32_t type);

// ---------------------------------------------------------------------------
// Mission Lua triggers (base vtable 00D00554). Layout as the constructor
// 007506B0 writes it: +4/+8/+0Ch dwords, +10h byte, +11h byte, +14h, +18h,
// name {len +1Ch, data +20h, flag byte +24h}, callback {len +28h, data +2Ch},
// +30h.
// ---------------------------------------------------------------------------
struct TriggerHost {
    virtual ~TriggerHost() = default;
    virtual void trigger_attach_04(std::uint8_t* trigger, std::uint8_t* list) = 0;  // trigger->vtable[4](list)
    virtual void variant_construct_00886920(std::uint8_t* variant) = 0;
    virtual bool trigger_is_active_08(std::uint8_t* trigger, std::uint32_t flag) = 0;  // vtable+8(1)
    virtual bool string_equals_00435c40(const std::uint8_t* string, const std::int32_t* other) = 0;  // ECX string
    virtual void trigger_fire_10(std::uint8_t* trigger) = 0;                           // vtable+10h()
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
    virtual void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void trigger_base_destruct_00875b30(std::uint8_t* trigger) = 0;
    virtual std::uint32_t resolve_target_00521ea0(const std::uint8_t* record) = 0;   // ECX record
    virtual void observer_destruct_00695870(std::uint8_t* observer) = 0;              // ECX this+30h or 0
    virtual bool entity_is_kind_5c(std::uint8_t* entity, std::uint32_t kind) = 0;
    virtual void owner_register_00876020(std::uint8_t* owner, std::uint8_t* trigger) = 0;  // ECX owner
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual std::uint8_t* line_of_sight_construct_007500e0(void* memory, std::uint8_t* target, float range) = 0;
    virtual std::uint8_t* proximity_construct_0074fee0(void* memory, const std::uint8_t (&target)[0x18], float range) = 0;
    // 0074FDD0's Lua call: variants of 14h bytes.
    virtual void variant_from_entity_00884a90(std::uint8_t* variant, std::uint32_t lua_handle) = 0;
    virtual void args_push_back_006edf00(std::uint8_t* args, const std::uint8_t* variant) = 0;
    virtual void variant_destruct_00884af0(std::uint8_t* variant) = 0;
    virtual void variant_default_008849b0(std::uint8_t* variant) = 0;
    virtual void variant_set_string_00886a10(std::uint8_t* variant, const std::uint8_t* string) = 0;
    // 00887E50, __thiscall(ECX [[00E188A8]+1A08h], 0, const name*, args*, 0, -1).
    virtual void lua_call_00887e50(std::uint32_t lua, std::uint32_t a, const std::uint8_t* name, std::uint8_t* args,
                                   std::uint32_t b, std::int32_t c) = 0;
    virtual void args_resize_006b88e0(std::uint8_t* args, std::uint32_t n) = 0;
    virtual void free_00bf6989(void* block) = 0;
};
// 0074FAB0, __fastcall(ECX list, EDX trigger): trigger->vtable[4](list).
void trigger_list_add_0074fab0(std::uint8_t* list, std::uint8_t* trigger, TriggerHost& host);
// 0074FB30, __fastcall(ECX variant): 00886920, then +4 = 6.
void mission_lua_variant_init_tag6_0074fb30(std::uint8_t* variant, TriggerHost& host);
// 0074FBB0, __fastcall(ECX list, EDX name) -> bool: walk +1Ch / next +0Ch;
// a trigger answering vtable+8(1) with flag +24h set and name equal (00435C40
// on +1Ch) is fired (vtable+10h); returns whether any fired.
bool trigger_list_invoke_by_name_0074fbb0(std::uint8_t* list, const std::int32_t* name, TriggerHost& host);
// 0074FC10, __fastcall(ECX string {len, data}): return the storage when set.
void native_string_release_0074fc10(std::int32_t* string, TriggerHost& host);
// 0074FC40, __fastcall(ECX trigger): vtable 00D00554, release callback (+28h)
// then name (+1Ch) storage, 00875B30.
void lua_trigger_destruct_0074fc40(std::uint8_t* trigger, TriggerHost& host);
// 0074FCD0, RET 4: argument == 1.
bool stub_arg_is_one_0074fcd0(std::uint32_t value);
// 0074FD40, __thiscall(ECX out {len, data, byte +8}, const source*) RET 4 -> out.
std::uint8_t* flagged_string_copy_construct_0074fd40(std::uint8_t* out, const std::uint8_t* source, TriggerHost& host);
// 0074FDD0, __fastcall(ECX trigger): call the Lua callback (+28h) with
// (owner's Lua handle [+4]+28h, name string when flag +24h else a tag-6
// variant), then fire vtable+10h; the argument vector is cleared and freed.
void lua_trigger_fire_callback_0074fdd0(std::uint8_t* trigger, void* const& game_00e188a8, TriggerHost& host);
// 0074FFE0, __thiscall(ECX observer = trigger+30h, entity) RET 4: when entity ==
// 00521EA0(ECX observer+14h) the trigger (observer-30h) fires (vtable+10h).
void proximity_trigger_on_target_gone_0074ffe0(std::uint8_t* observer, std::uint32_t entity, TriggerHost& host);
// 00750190, __thiscall(ECX observer, entity) RET 4: when entity == observer+14h, fire.
void line_of_sight_trigger_on_target_gone_00750190(std::uint8_t* observer, std::uint32_t entity, TriggerHost& host);
// 00750090 / 00750430, __fastcall(ECX trigger): 00695870(ECX trigger+30h, or 0
// for a null trigger), then 0074FC40.
void observer_trigger_destruct_00750090(std::uint8_t* trigger, TriggerHost& host);
// 007501F0, __fastcall(ECX entity) -> entity when entity->vtable[5Ch](18h), else 0.
std::uint8_t* entity_as_kind18_007501f0(std::uint8_t* entity, TriggerHost& host);
// 007504C0, __thiscall(ECX trigger, owner) RET 4: 00876020(ECX owner, trigger),
// trigger+30h = owner+28h.
void squad_ordnance_trigger_attach_007504c0(std::uint8_t* trigger, std::uint8_t* owner, TriggerHost& host);
// 00750550, __thiscall(ECX target, float range) RET 4: new 50h, 007500E0(target, range) or 0.
std::uint8_t* line_of_sight_trigger_create_00750550(std::uint8_t* target, float range, TriggerHost& host);
// 007505C0, __thiscall(ECX target record, float range) RET 4: new 60h,
// 0074FEE0(record by value (18h bytes: two words, a dword, four floats), range) or 0.
std::uint8_t* proximity_trigger_create_007505c0(const std::uint8_t* target, float range, TriggerHost& host);
// 00750660, __cdecl() -> trigger: new 34h with the inline base construction
// (vtable 00D005C0) or 0.
std::uint8_t* squad_ordnance_trigger_create_00750660(TriggerHost& host);
// 007506B0, __thiscall(ECX trigger, const name*, const callback*) RET 8 -> trigger.
std::uint8_t* lua_trigger_construct_007506b0(std::uint8_t* trigger, const std::uint8_t* name,
                                             const std::uint8_t* callback, TriggerHost& host);
// 00750790, __thiscall(ECX trigger, const name*, const callback*) RET 8:
// assign +1Ch (and its flag byte +24h) and +28h.
void lua_trigger_set_callback_name_00750790(std::uint8_t* trigger, const std::uint8_t* name,
                                            const std::uint8_t* callback, TriggerHost& host);

// ---------------------------------------------------------------------------
// Telemetry / METRICSSESSION.
// ---------------------------------------------------------------------------
struct TelemetryHost {
    virtual ~TelemetryHost() = default;
    virtual void stream_write_00a4d610(std::uint32_t value, std::uint8_t* stream_04) = 0;  // __cdecl
    // 00752900, ECX first, EDX last, push vector, push owner (callee pops).
    virtual void destroy_range_00752900(std::uint32_t first, std::uint32_t last, std::uint8_t* vector,
                                        std::uint8_t* owner) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00429F20 (cursor, header) reads an owned native string.
    virtual void read_owned_string_00429f20(NativeBitCursor* cursor, std::uint8_t* header) = 0;
};
// 00751850, __thiscall(ECX stream, value) RET 4: 00A4D610(value, stream+4).
void telemetry_stream_write_00751850(std::uint8_t* stream, std::uint32_t value, TelemetryHost& host);
// 00752BF0, RET 4: t == E9h (SETZ).
bool metrics_session_is_type_00752bf0(std::uint32_t type);
// 00752C30, __thiscall(ECX message, stream) RET 4: type byte +10h (8 bits,
// 00428C70), then 00429F20(stream+4, message+18h).
void metrics_session_read_00752c30(std::uint8_t* message, NativeSessionReadStream& stream, TelemetryHost& host);
// 00753530 / 007547A0 (__fastcall ECX object) and 00754A40 (__stdcall object,
// RET 4): the vector at object+10h {first +14h, last +18h, end +1Ch}: when first
// is set 00752900(first, last, vector, object) and free(first); zero the three.
void telemetry_list_tidy_00753530(std::uint8_t* object, TelemetryHost& host);

// ---------------------------------------------------------------------------
// MMothership (class 09h, 12C8h bytes, primary vtable 00D01630; air-ops block
// at +1188h, vtable 00D015C4).
// ---------------------------------------------------------------------------
struct MothershipHost {
    virtual ~MothershipHost() = default;
    virtual double owner_heading_50(std::uint8_t* node) = 0;  // [[holder+4]+7Ch] vtable+50h, x87
    virtual double wrapped_angle_delta_00438b10(float a, float b) = 0;  // stdcall(a, b), x87
    virtual void block_destruct_006ca410(std::uint8_t* block) = 0;
    virtual void ship_base_destruct_0081f3a0(std::uint8_t* ship) = 0;
    virtual void ship_init_pass_c_0081f980(std::uint8_t* ship) = 0;
    virtual void ship_update_008255b0(std::uint8_t* ship, float dt) = 0;
    virtual void block_update_006ba670(std::uint8_t* block, float dt) = 0;
    virtual void ship_update_motion_00825f20(std::uint8_t* ship, float dt) = 0;
    virtual void deck_update_006cdc70(std::uint8_t* deck, float dt) = 0;  // ECX ship+E78h
    virtual bool block_may_follow_006ba6a0(std::uint8_t* block, std::uint32_t a, std::uint32_t b) = 0;
    virtual bool ship_may_follow_008162b0(std::uint8_t* ship, std::uint32_t a, std::uint32_t b) = 0;
    virtual int compare_00438e10(const char* a, const char* b) = 0;  // ECX a, EDX b
    virtual bool ship_supports_command_00812b00(std::uint8_t* ship, const char* command) = 0;
    virtual void ship_get_property_00815870(std::uint8_t* ship, std::uint32_t lua, std::uint32_t key) = 0;
    virtual void block_get_property_006c6630(std::uint8_t* block, std::uint32_t lua, std::uint32_t key) = 0;
    virtual void ship_reset_condition_0080e410(std::uint8_t* ship) = 0;
    virtual void enter_section(void* section) = 0;  // [00CE2218] EnterCriticalSection
    virtual void leave_section(void* section) = 0;  // [00CE2210] LeaveCriticalSection
    virtual void ship_base_construct_0081ed40(std::uint8_t* ship, std::uint32_t row) = 0;
    virtual void block_construct_006cac00(std::uint8_t* block, std::uint8_t* ship) = 0;
    virtual double controller_speed_0092d730(std::uint8_t* controller) = 0;  // ECX [ship+1018h], x87
    virtual void node_set_transform_38(std::uint8_t* node, const float* matrix) = 0;  // [ship+1284h] vtable+38h
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t bytes) = 0;
    virtual void set_vehicle_class_009553d0(std::uint8_t* unit, std::uint32_t vehicle_class) = 0;
    virtual void ship_serialize_00818340(std::uint8_t* ship, ArchiveWriter& archive) = 0;
    virtual void block_serialize_006c19d0(std::uint8_t* block, ArchiveWriter& archive) = 0;
    virtual std::uint8_t* find_node_0071ad50(std::uint8_t* scene, const char* name) = 0;  // ECX scene
    virtual const float* node_world_matrix_00b6db60(std::uint8_t* node) = 0;
    virtual void matrix_set_004134f0(std::uint8_t* matrix, const float* source) = 0;
    virtual void ship_update_00811ab0(std::uint8_t* ship, float dt) = 0;
    virtual void holder_refresh_006bee40(std::uint8_t* holder, float dt) = 0;
    virtual bool message_is_type_0c(const std::uint8_t* message, std::uint32_t type) = 0;
    virtual void ship_vslot_220(std::uint8_t* ship, const std::uint8_t* message) = 0;
    virtual void ship_vslot_224(std::uint8_t* ship, const std::uint8_t* message) = 0;
    virtual void elevator_move_50(std::uint8_t* elevator, std::uint8_t flag, std::uint8_t* entity, float seconds) = 0;
    virtual bool block_handle_message_006cd6c0(std::uint8_t* block, const std::uint8_t* message) = 0;
    virtual bool ship_handle_message_00821e80(std::uint8_t* ship, const std::uint8_t* message) = 0;
};
// 00758090, __thiscall(ECX holder, float dt) RET 4: +88h = heading (rounded
// float); acc +94h += dt (rounded); when acc > 0.2 ([00CE3D10] double of
// 0.2f): r = 00438B10(heading, +90h); acc = 0; +8Ch = (r*0.5)/acc_old +
// 0.5*+8Ch ([00D7A280] = 0.5; DC C9 / DE FA / DE C1 read from the bytes);
// +90h = +88h.
void air_ops_holder_step_00758090(std::uint8_t* holder, float dt, MothershipHost& host);
// 00758150, __fastcall(ECX ship): vtables, block vtable 00D015C4, 006CA410(block), 0081F3A0.
void mothership_destruct_00758150(std::uint8_t* ship, MothershipHost& host);
// 00758210, __fastcall(ECX ship): 0081F980, then +11A8h = 1 (dword).
void mothership_init_pass_c_00758210(std::uint8_t* ship, MothershipHost& host);
// 00758240, __thiscall(ECX ship, float dt) RET 4: 008255B0(dt), 006BA670(ECX block, dt).
void mothership_update_00758240(std::uint8_t* ship, float dt, MothershipHost& host);
// 00758270, __thiscall(ECX ship, float dt) RET 4: 00825F20(dt), 006CDC70(ECX
// ship+E78h, dt), 00758090(ECX [ship+EF8h], dt) when set.
void mothership_update_motion_00758270(std::uint8_t* ship, float dt, MothershipHost& host);
// 007582B0, __thiscall(ECX ship, a, b) RET 8 -> bool: block 006BA6A0(a, b) or ship 008162B0(a, b).
bool mothership_may_follow_007582b0(std::uint8_t* ship, std::uint32_t a, std::uint32_t b, MothershipHost& host);
// 007582F0, __thiscall(ECX ship, const char* command) RET 4 -> bool:
// "attackmove" (00CECCA8) -> false; "launchairstrike" (00CF85E0) -> true; else
// 00812B00 (00438E10 returns 0 on equality).
bool mothership_supports_command_007582f0(std::uint8_t* ship, const char* command, MothershipHost& host);
// 00758340, __thiscall(ECX ship, lua, key) RET 8: 00815870, then 006C6630(ECX block).
void mothership_get_property_00758340(std::uint8_t* ship, std::uint32_t lua, std::uint32_t key, MothershipHost& host);
// 00758370, __fastcall(ECX ship): 0080E410, then +11A5h = +11A4h = 0 (bytes).
void mothership_reset_condition_00758370(std::uint8_t* ship, MothershipHost& host);
// 007584B0, __fastcall(ECX block) -> bool: under the section +8 (Enter,
// ++depth +18h; --depth, Leave; skipped when null) the 20-byte record vector
// +A8h/+ACh is non-empty.
bool air_ops_block_has_queue_records_007584b0(std::uint8_t* block, MothershipHost& host);
// 00758550, __thiscall(ECX ship, row) RET 4 -> ship.
std::uint8_t* mothership_construct_00758550(std::uint8_t* ship, std::uint32_t row, MothershipHost& host);
// 00758630, __fastcall(ECX ship) -> bool: |+984h| > 0.05 ([00D7A270]) and
// |0092D730()| > 1.0 and (007584B0(block) or +11C0h != 0). |x| is x > 0 ?
// x : -0.0 - x ([00D7A208] = -0.0f).
bool mothership_vslot228_00758630(std::uint8_t* ship, MothershipHost& host);
// 007586D0, __thiscall(ECX ship, float dt) RET 4: copy the 4x4 at +1288h,
// subtract dt * row 1 from row 3 (DE CA, D8 C9 from the bytes; products
// rounded to float), [+1284h] vtable+38h(&copy).
void mothership_step_elevator_frame_007586d0(std::uint8_t* ship, float dt, MothershipHost& host);
// MOTHERSHIP_ELEVATOR_MOVE (87h, vtable 00D01870).
// 00758B90, __thiscall(ECX message, entity, byte flag) RET 8.
std::uint8_t* elevator_move_construct_00758b90(std::uint8_t* message, std::uint8_t* entity, std::uint8_t flag,
                                               const NativeSessionMessageContext& ctx, SessionMessageHost& host);
// 00758BF0, RET 4: t == 87h || 49h || 46h.
bool elevator_move_is_type_00758bf0(std::uint32_t type);
// 00758C20 / 00758C70: header, bools +1Ch, +22h, +23h, then the id +20h in 12
// bits when +23h (the reader zeroes +20h otherwise).
void elevator_move_write_00758c20(const std::uint8_t* message, NativeBitCursor* cursor, SessionMessageHost& host);
void elevator_move_read_00758c70(std::uint8_t* message, NativeSessionReadStream& stream, SessionMessageHost& host);
// 00758CF0, __fastcall(ECX message) -> the entity of id +20h when +23h, else 0.
std::uint8_t* elevator_move_entity_00758cf0(const std::uint8_t* message, const EntityIdTable& ids);
// 00758D30, __thiscall(ECX class, row) RET 4 -> ship: new 12C8h, memset,
// 00758550, 009553D0(class), +354h = class.
std::uint8_t* mothership_create_00758d30(std::uint32_t vehicle_class, std::uint32_t row, MothershipHost& host);
// 00758DC0, __thiscall(ECX ship, archive) RET 4: +11A8h = 1, 00818340,
// begin "_airBase", 006C19D0(block), end, begin "_motherShip", end.
void mothership_serialize_00758dc0(std::uint8_t* ship, ArchiveWriter& archive, MothershipHost& host);
// 00758E40, __fastcall(ECX ship): +1284h = 0071AD50(ECX [ship+360h] ?
// [[ship+360h]+160h] : 0, "elevator_2"), then 004134F0(ECX +1288h, 00B6DB60(node)).
void mothership_bind_elevator_node_00758e40(std::uint8_t* ship, MothershipHost& host);
// 00758E80, __thiscall(ECX ship, float dt) RET 4: 00811AB0(dt), then
// 006BEE40(ECX [ship+EF8h], dt) when set.
void mothership_controller_refresh_00758e80(std::uint8_t* ship, float dt, MothershipHost& host);
// 00758EB0, __thiscall(ECX ship, message) RET 4 -> bool: 72h / 74h ->
// vtable+220h, 73h / 75h -> vtable+224h (tested 72h, 73h, 74h, 75h in that
// order); 87h -> [ship+11C4h] vtable+50h(+22h, 00758CF0(msg), float([00F876B0]
// - [msg+0Ch])); else the block 006CD6C0 then the ship 00821E80.
bool mothership_handle_message_00758eb0(std::uint8_t* ship, const std::uint8_t* message, const EntityIdTable& ids,
                                        const std::int32_t& tick_00f876b0, MothershipHost& host);

// ---- LandConvoy member vector kill (packet cc13-convoy) ---------------------
// 00742210 BSP_EntityGroup_KillMembersVector, __thiscall(ECX = convoy, char
// hard) RET 4, no x87. The Lua Kill binding calls it for a LandConvoy
// (008AC740 IsKindOf 1Ah; 008AC748 CMP ESI,EBP / SETNE with EBP = 0 from
// 008AC5E3, and the cause 1 or 2, so hard is always true and the members get
// cause 2; 008AC74E), then JMPs past 008AC756's 00926D90, so the convoy itself
// is not killed.
// 1. 00742213/00742219: ESI = first; the first <= last assert cannot fail.
// 2. Each pass: EBP = last re-read (00742231); the first <= last (00742234)
//    and same-container (0074223E, CMP EDI,EDI) asserts cannot fail on a
//    count. Exit when ESI == last (00742247).
// 3. 0074224B checked deref; 00742255 a null slot is skipped; 0074225A checked
//    deref; 00742264 ECX = slot (00742266 TEST is the same word, redundant);
//    cause = hard ? 2 : 1 (0074226A..00742271); 00742275 00926D90(member,
//    cause); 0074227A MOV dword [slot], 0.
// 4. 00742280 checked deref on both arms; ESI += 4.
// Member +738h is NOT cleared, so each member's own on-killed (0074CD10, when
// 009273A0's loop reaches it on the kill list) still calls 007422A0, which no
// longer finds the slot:
// no decrement, and the convoy dies only if +3BCh was already zero.
// This is the one 00742210 body; it uses cc15's member-slot seam
// (include/bsp/cc15_land_convoy.hpp). src/unit_damage.cpp
// group_kill_members_vector_00742210 forwards to it through an adapter over
// UnitDamageHost's vector hooks; no game host implements UnitDamageHost.
struct LandConvoyKillVectorHost : cc15_land_convoy::LandConvoyMemberSlots {
    // 00926D90 BSP_MissionEntity_Kill, __thiscall(ECX = entity, cause) RET 4.
    virtual void mission_entity_kill_00926d90(std::uint8_t* entity, std::int32_t cause) = 0;
    // MOV dword [slot], 0 (0074227A).
    virtual void null_member_slot(std::uint8_t* convoy, std::size_t i) = 0;
};
void land_convoy_kill_member_vector_00742210(std::uint8_t* convoy, bool hard, LandConvoyKillVectorHost& host);

}  // namespace bsp::cc13_seg41

namespace bsp::cc13_seg41 {
struct FrameClothHost {
    virtual ~FrameClothHost() = default;
    virtual void flag_cloth_process_frame_008e2560(float dt) = 0;
};
// 00737870: frame-hook slot +0Ch; ECX ignored, float dt on stack, RET 4.
// The FLD/FSTP argument copy precedes the cdecl 008E2560 call.
void frame_hook_process_cloth_00737870(float dt, FrameClothHost& host);
// Oct18b-nB additions, same raw Win32 object contracts as the family above.
// Names remain hypotheses; explicit hosts/context make these new source APIs.
// Original ECX=convoy, one ignored stack word, XOR AL,AL; RET 4.
std::uint8_t land_convoy_slot164_false_00741110(void*, std::uint32_t) noexcept;
struct LandingShipReadHost {
    virtual ~LandingShipReadHost() = default;
    virtual void ship_create_read_00813bd0(std::uint8_t*, NativeSessionReadStream&) = 0;
};
// Each serializer is thiscall(ECX=message, cursor/stream), RET 4. The read
// wrapper passes stream+4 to the bit readers, after the complete base read.
void landing_ship_create_read_00749d20(std::uint8_t*, NativeSessionReadStream&, LandingShipReadHost&);
void land_at_pad_write_00749e10(const std::uint8_t*, NativeBitCursor*);
void land_at_pad_read_00749e40(std::uint8_t*, NativeSessionReadStream&);
void land_vehicle_create_read_0074ccb0(net_cc13::EntityCreateMessage&, NativeSessionReadStream&, const NativeBitNumericContext&);
struct LandVehicleMessageHost {
    virtual ~LandVehicleMessageHost() = default;
    virtual std::uint8_t unit_handle_message_0095abe0(void*, void*) = 0;
};
std::uint8_t land_vehicle_handle_message_0074cd90(void*, void*, LandVehicleMessageHost&);
void land_vehicle_message_write_0074cef0(const void*, NativeBitCursor*);
void land_vehicle_message_read_0074cf00(void*, NativeSessionReadStream&);
void land_vehicle_message_write_0074d000(const void*, NativeBitCursor*);
void land_vehicle_message_read_0074d010(void*, NativeSessionReadStream&);
// ECX=vehicle, RET; selected AI manager from game+21A4h+[game+18ECh]*4.
void land_vehicle_register_ai_0074ddf0(std::uint8_t*, void* const& game_00e188a8, LandVehicleHost&);
// ECX=plane, RET, null maps to null; returns the interior observer +72Ch.
std::uint8_t* plane_observer_0074e170(std::uint8_t*) noexcept;
// ECX=plane, stack destination, RET 4, EAX=destination. Exactly sixteen
// forward DWORD copies (REP MOVSD), including the original overlap order.
void* plane_copy_matrix_e00_0074e270(const std::uint8_t*, void*);
void* plane_copy_matrix_e40_0074e290(const std::uint8_t*, void*);
// ECX=observer subobject; SUB ECX,72Ch, tailcalls 007B8E60 / 007BBB70.
float plane_observer_speed_0074e3c0(const std::uint8_t*);
float* plane_observer_vector_0074e3f0(const std::uint8_t*, float*);
// SHIP_SINK, ECX=message, RET 4. Six-bit count; no image capacity clamp.
// Borrow the live scale at 00D7A248; original FLD/FSTP steps are preserved.
void ship_sink_write_0074eb20(const std::uint8_t*, NativeBitCursor*, const volatile float& scale_00d7a248, const NativeBitNumericContext&);
void ship_sink_read_0074eb80(std::uint8_t*, NativeSessionReadStream&, const volatile float& scale_00d7a248, const NativeBitNumericContext&);
// FLD [00CE89CCh] (immutable binary32 25.0), RET, ST0 result.
float mothership_air_ops_speed_00758200() noexcept;
// ECX=air-ops subobject, RET; [ECX-C50h]+830h, with no null test.
std::uint8_t* mothership_air_ops_class_00758230(const std::uint8_t*);
struct MothershipFailureSourceHost : SessionMessageHost {
    virtual int stricmp_00bf7fbf(const char*, const char*) = 0;
    virtual bool native_string_equals_00425850(const std::uint8_t*, const char*) = 0;
    virtual std::uint8_t ship_get_failure_008135c0(std::uint8_t*, const std::uint8_t*, std::uint32_t, std::uint8_t*) = 0;
    virtual std::uint8_t ship_failure_expired_00813660(std::uint8_t*, const std::uint8_t*, std::uint32_t) = 0;
    virtual void route_message_0077c2a0(std::uint8_t*, std::uint8_t*, std::uint32_t, std::uint32_t) = 0;
};
// ECX=ship; (native name, frame, out) RET 0Ch, AL result; runway/hangar
// bytes +11A4h/+11A5h, else the complete base query with original arguments.
std::uint8_t mothership_get_failure_007589d0(std::uint8_t*, const std::uint8_t*, std::uint32_t, std::uint8_t*, MothershipFailureSourceHost&);
// ECX=ship; (native name, frame) RET 8, AL result. Builds stack message
// 73h/75h and routes (7,0), or delegates 00813660. Scope cleanup is compiler
// generated; no allocator/host side effects are introduced for that cleanup.
std::uint8_t mothership_failure_expired_00758a50(std::uint8_t*, const std::uint8_t*, std::uint32_t, const NativeSessionMessageContext&, std::uint32_t clear_value_00cf8b3c, MothershipFailureSourceHost&);
}
