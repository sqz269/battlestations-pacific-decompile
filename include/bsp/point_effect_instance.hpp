#pragma once

#include "bsp/native_node_destruction.hpp"
#include "bsp/render_command_queue.hpp"

namespace bsp {

struct PointEffectReferenceArray {
    RenderCommandReference** begin;
    std::int32_t count;
    std::int32_t capacity;
};

// Actual 114h storage on MSVC Win32. No member initializers: bytes not written
// by the recovered stages retain their allocation preimage. Pointer fields use
// the canonical companions borrowing actual owner fields, so this is NOT a
// native binary replacement. Integer vtable identities are evidence, never calls.
struct PointEffectInstanceStorage {
    std::uint32_t original_vtable_identity_00;
    std::atomic<std::int32_t> references_04;
    std::uint8_t option_08;
    std::uint8_t field_09;
    std::uint8_t field_0a;
    std::byte untouched_0b;
    PointEffectReferenceArray entries_0c;
    PointEffectReferenceArray auxiliary_18;
    std::uint32_t tail_word_24;
    std::uint32_t field_28;
    std::uint32_t field_2c;
    std::array<float, 6> fields_30;
    float field_48;
    float field_4c;
    std::array<float, 3> fields_50;
    std::array<float, 6> fields_5c;
    std::array<std::byte, 12> untouched_74;
    float field_80;
    RenderCommandReference* template_84;
    RenderCommandReference* owner_88;
    CameraTransform* parent_8c;
    CameraMatrix cached_world_90;
    CameraMatrix relative_d0;
    NativeNodeBinding* node_110;
};

// Owns ONLY the incoming by-value template reference: no retain at entry.
// Keep this scope alive across the entire eventual constructor. On exception,
// complete the native member unwind BEFORE destroying it. The fragments below
// do not implement that unwind or release the independently retained +84 member.
class PointEffectTemplateArgument final {
public:
    explicit PointEffectTemplateArgument(RenderCommandReference*) noexcept;
    ~PointEffectTemplateArgument();
    PointEffectTemplateArgument(const PointEffectTemplateArgument&) = delete;
    PointEffectTemplateArgument& operator=(const PointEffectTemplateArgument&) = delete;
    RenderCommandReference* get() const noexcept { return reference_; }
private:
    RenderCommandReference* reference_;
};

struct PointEffectInstanceCounters {
    std::uint32_t& actual_00f87604;
    std::uint32_t& actual_00f87600;
};

// ONLY 008680D9..00868192. Requires fresh aligned actual114h storage; starts its
// C++ lifetime, initializes +04 to1, retains template+84, increments F87604 THEN
// F87600 modulo2^32. No allocation, node creation, or constructor completion.
PointEffectInstanceStorage& initialize_point_effect_instance_008680d9(
    void* actual_storage, const PointEffectTemplateArgument&,
    std::uint8_t option_byte, std::uint32_t tail_word, PointEffectInstanceCounters);

class PointEffectInstanceLinks {
public:
    virtual ~PointEffectInstanceLinks() = default;
    // Pure current load of [E188A8]+19EC; nullable root is valid. No registration,
    // owner construction, or cached global snapshot belongs in this operation.
    virtual RenderNodeRootList* root_e188a8_19ec() noexcept = 0;
    // Pure projection of this EXACT parent to its existing +04 count and actual
    // virtual+00 terminal action. It must never make a second diagnostic count.
    virtual RenderCommandReference& parent_reference(CameraTransform&) noexcept = 0;
};

// ONLY 00868193..008681BD and its state6 raw-allocation unwind. The name is a
// borrowed reference to the original captured template's actual +1C header;
// do not copy its value before allocation. Uses the caller's constructed
// 0108FF58 pool and supplied string storage (ActualNativeStringPoolStorage for
// the actual native path). Returns constructed physical storage, not a companion.
// On construction failure, B6F5A0 unwinds members, then this stage returns the
// raw slot through B6E670 and rethrows. The outer states5..0 remain required.
// A null allocation skips name construction, matching the native branch; the
// following native retain still requires a nonnull result. No +110 publication,
// retain, registration, template release, or successful-node cleanup occurs here.
NativeNodeStorage* construct_point_effect_node_00868193(void* actual_pool_0108ff58,
    const NativeString& actual_captured_template_name_1c, NativeStringStorage& strings);

// ONLY 008681BE..0086824C. Caller supplies the actual node AFTER successful
// B6ED70 allocation and B6F5A0 construction, with stable canonical scene bindings.
// Stores node+110 and retains its actual +04. If third stack word OR parent is
// nonzero, executes canonical B6D890 BEFORE releasing/replacing parent+8C.
// The old parent terminal action can reenter; +8C is cleared AFTER that action.
// This retains an effect parent; it does not attach a CameraTransform hierarchy.
// Exceptions propagate with the partial native state; outer unwind is required.
void register_point_effect_node_and_parent_008681be(PointEffectInstanceStorage&,
    NativeNodeBinding& constructed_node, CameraTransform* parent,
    std::uint32_t third_stack_word, NativeNodeDestructionRuntime&,
    PointEffectInstanceLinks&);

// ONLY 0086826E..008682D4, AFTER the real53D9C0/72AA80 matrix setter. Captures
// current+110, conditionally refreshes world, snapshots its16 DWORDs with REP-MOVS
// semantics, then uses canonical x87 copy into+90. Sets +4C to native0.01 bits,
// +48 and +5C..70 to+0. Stops BEFORE vector resize8672A0.
void cache_point_effect_initial_world_0086826e(PointEffectInstanceStorage&);

} // namespace bsp
