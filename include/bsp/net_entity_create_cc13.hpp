#pragma once
// The generic net-entity create message (cc13-net3). Every replicated entity's
// create message (DFh MT_MNETTABLESYNCENTITY_CREATE and the other 47h-class
// messages; 11 builders call 00766FC0) starts from this snapshot:
//   00766FC0 base constructor  __thiscall(msg, type, word, entity, with_name) RET 10h
//   00765000 writer            __thiscall(msg, cursor) RET 4 (the cursor goes to
//                              the bit writers as ECX unchanged)
//   00767E30 reader            __thiscall(msg, stream) RET 4 (cursor at stream+4)
// Base vtable 00D03720; size 110h. The receive side is 0077EC20
// (BSP_Replication_ApplyPendingEntityCreates, game_hosts_fixed_step.cpp),
// which creates the entity through msg->vtable[1Ch] when +1Ch is unknown.
// Field names are hypotheses from what the constructor copies; nothing here
// spawns or changes a unit (codec and snapshot only).
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_session_message_base.hpp"

namespace bsp::net_cc13 {

inline constexpr std::uint32_t kEntityCreateBaseVtable = 0x00D03720u;
inline constexpr std::uint32_t kEntityCreateMessageSize = 0x110u;
// Every numeric float in this codec is written as (zero 0, signed 1, scale
// [00D7A248] = FLT_MAX, 32 bits): the unscaled raw-32 path.
inline constexpr std::uint32_t kFltMaxBits = 0x7F7FFFFFu;

struct EntityCreateMessage {
    std::uint8_t type_10{0};          // 0075B430 type byte; wire u8 8
    std::uint16_t net_id_18{0};       // 0 here; 00779FC0 stores entity+174h; wire 12
    std::uint8_t flag_1a{0};          // 0; wire bool
    std::uint16_t net_id_1c{0};       // entity+174h; wire 12
    std::uint16_t word_1e{0};         // constructor argument; written as signed 10, read unsigned
    std::uint16_t parent_id_20{0};    // [entity+3Ch]+174h, 0 without parent; wire 13
    float position_24[3]{};           // entity+A4h..+ACh
    float angles_30[3]{};             // 0042D2E0 of the matrix at entity+74h
    std::uint8_t byte_3c{0};          // entity+5Ch; wire bool
    std::uint32_t word_40{0};         // entity+28Ch; wire u32 16
    std::uint8_t has_pairs_44{0};     // 0 in the base; wire bool
    std::vector<std::array<float, 2>> pairs_4c;  // std::vector at +48h (begin +4Ch)
    std::uint32_t word_58{0};         // 0 in the base; wire u32 2
    std::int32_t word_5c{1};          // 1 in the base; wire (value+1) in 4 bits
    std::uint8_t uniform_60{0};       // all nine words_64 equal; wire bool
    std::uint32_t words_64[9]{};      // entity+188h..+1A8h; wire u32 7 each (one when uniform)
    std::uint8_t has_words_88{0};     // entity is not kind 2Ah; wire bool
    std::int32_t words_8c[9]{};       // entity+1ACh..+1CCh; wire signed 7 each
    std::uint32_t word_b0{0};         // entity+54h; wire u32 4
    std::uint8_t has_b8_b4{0};        // entity is kind 5; wire bool
    std::uint32_t word_b8{0};         // entity->vtable[12Ch](); wire u32 3
    std::uint8_t has_name_bc{0};      // the constructor's with_name byte; wire bool
    std::string name_c0;              // 00438E40 copy of entity->vtable[10h]() (char*, null when absent)
    bool name_present_c0{false};
    std::uint8_t byte_c4{0};          // entity+58h; wire u8 4
    std::uint8_t in_group_c5{0};      // kind 2 with entity+284h set; wire bool
    std::uint8_t is_leader_c6{0};     // [group+14h] == entity; wire bool
    std::uint16_t leader_id_c8{0};    // [group+14h]+174h; wire 12
    std::uint8_t group_kind_ca{0};    // group+4FCh; wire u8 8
    std::uint8_t group_byte_cb{0};    // group+500h; wire u8 8
    float member_vector_cc[3]{};      // kind 18h: 0070D980(group, &out, entity)
    // kind 6: record +10h..+1Ch / +20h..+2Ch of 0070D080(group, entity). The
    // writer sends these eight floats for every kind other than 18h, but the
    // constructor fills them only for kind 6 and never zeroes them, so other
    // non-leader kinds send whatever the message storage held.
    float member_a_d8[4]{};
    float member_b_ec[4]{};
    std::uint8_t non_unit_scale_100{0};  // |scale - 1.0| > 1e-4 (x87); wire bool
    float scale_104{0.0f};            // |matrix row 0| (0 when its square <= 1e-10)
    float float_108{0.0f};            // entity+304h
    std::uint32_t word_10c{0};        // entity+180h; wire u32 4 (DFh overwrites it)
};

// The entity fields 00766FC0 reads. The vcalls go through the host.
struct EntityCreateSource {
    std::uint16_t net_id_174{0};
    bool has_parent_3c{false};
    std::uint16_t parent_net_id{0};   // [entity+3Ch]+174h
    float position_a4[3]{};
    float matrix_row0_74[3]{};
    std::uint8_t byte_5c{0};
    std::uint32_t word_28c{0};
    std::uint32_t words_188[9]{};
    std::int32_t words_1ac[9]{};
    std::uint32_t word_54{0};
    std::uint8_t byte_58{0};
    float float_304{0.0f};
    std::uint32_t word_180{0};
};

struct EntityCreateGroupMember {
    float vector[3]{};   // 0070D980 result
    float record_10[4]{};  // 0070D080 result +10h..+1Ch
    float record_20[4]{};  // +20h..+2Ch
};

struct EntityCreateHost {
    virtual ~EntityCreateHost() = default;
    virtual bool is_kind_5c(std::uint32_t kind) = 0;          // entity->vtable[5Ch](kind)
    virtual std::uint32_t vcall_12c() = 0;                    // entity->vtable[12Ch]()
    virtual const char* name_10() = 0;                        // entity->vtable[10h]()
    // 0042D2E0 (ECX = entity+74h, EDX = &out[0], stack &out[1], &out[2]).
    virtual void basis_angles_0042d2e0(float (&out)[3]) = 0;
    // entity+284h and its fields; group_present false when the pointer is null.
    virtual bool group_present_284() = 0;
    virtual std::uint8_t group_kind_4fc() = 0;
    virtual std::uint8_t group_byte_500() = 0;
    virtual std::uint16_t group_leader_net_id() = 0;          // [group+14h]+174h
    virtual bool group_leader_is_self() = 0;                  // [group+14h] == entity
    virtual EntityCreateGroupMember group_member(std::uint8_t kind) = 0;
};

// 00766FC0. with_name selects +BCh/+C0h.
EntityCreateMessage build_entity_create_00766fc0(std::uint8_t type, std::uint16_t word_1e,
                                                 const EntityCreateSource& entity,
                                                 bool with_name, EntityCreateHost& host);

// 00765000 / 00767E30 on the reconstructed bit-cursor primitives.
void write_entity_create_00765000(const EntityCreateMessage& msg, NativeBitCursor* cursor,
                                  const NativeBitNumericContext& numeric);
void read_entity_create_00767e30(EntityCreateMessage& msg, NativeSessionReadStream& stream,
                                 const NativeBitNumericContext& numeric);

}  // namespace bsp::net_cc13
