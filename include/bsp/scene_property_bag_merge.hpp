#pragma once
// Scene property bag merge and per-type value copy.
// Addresses: 008F54F0 merge, 008F23E0 assign-existing, 008F4F60 record clone,
// 008F0700 record assign, 008F0340 reference payload copy, 008F03B0 array block
// copy, 008F03F0 array block release, 008F0640 record release, 008F0420 and
// 008F3AC0 reference setters, 008EF2B0 / 008EF2F0 / 008EF360 record
// constructors, 008EF7F0 element count. See docs/SCENE_PROPERTY_BAG_MERGE.md.
//
// Every type here is a hypothesis over the record layout established in
// bsp/scene_property_bag.hpp, not a recovered declaration. The record types,
// the twelve type codes, the eight reference kinds, the bag model, the
// 008F54F0 merge rule and the 008F41F0 clone rule already live in that header
// and are reused, never redeclared.
#include <cstdint>
#include <string>

#include "bsp/scene_property_bag.hpp"

namespace bsp {

// --- Record offsets this packet added to the layout -------------------------

// record+08h. Written by the reference setters 008F0420 (008F0451) and 008F3AC0
// (puVar1[2] = param_4) and by the type-5 constructor 008EF2B0; read only by the
// type-5 clone arm at 008F50xx, which passes it back to 008EF2B0. It carries the
// SceneReferenceKind. The prior packet listed it as a gap.
inline constexpr std::uint32_t kScenePropertyRecordReferenceKindOffset = 0x08;

// record+1Ch. The heap copy of the reference target name: 00438E40 duplicates it
// in 008F0420, 008F3AC0 and 008EF2B0, 008F0340 frees and re-duplicates it on an
// assign, and the type-5 arm of the record release 008F0640 frees it.
inline constexpr std::uint32_t kScenePropertyRecordReferenceNameOffset = 0x1C;

// record+2Ch. Every constructor read in this packet stores the byte 1 and no
// routine read here ever loads it. Provisional, no reader.
inline constexpr std::uint32_t kScenePropertyRecordFlagOffset = 0x2C;

// The single vtable pointer every record constructor stores at +00h: 008EF140
// (008EF148), 008EF2B0, 008EF2F0 (008EF2F8) and 008F38A0 all write this one
// address, so the twelve type codes are one class with a tag, not twelve
// subclasses. Its only slot is the deleting destructor 004E6730, which forwards
// to the type switch 008F0640.
inline constexpr std::uint32_t kScenePropertyRecordVtable = 0x00CE89D4;

// --- The reference payload --------------------------------------------------

// The three fields a type-5 record carries, in the order 008EF2B0 writes them.
// `tag` is the dword at +18h: the parser passes 0 at both setter call sites
// (008F60C1 and 008F6254), so no authored value has ever been observed in it,
// but 008F0340 copies it on an assign and the type-5 clone arm carries it.
struct SceneReferencePayload {
    SceneReferenceKind kind{SceneReferenceKind::Any}; // +08h
    std::int32_t tag{0};                              // +18h, parser writes 0
    std::string target;                               // +1Ch, owned heap copy
};

// --- Per-type copy classification -------------------------------------------

// What one arm of 008F4F60 (clone) or 008F0700 (assign) moves. The two differ,
// which is the point of the two spec accessors below: a clone reconstructs the
// record from scratch and carries the kind and the declaration, an assign writes
// into a record that already has its own type and keeps those fields.
struct ScenePropertyCopySpec {
    bool handled{false};          // false: the switch has no arm for this type
    bool inline_dword{false};     // +0Ch carried as a dword
    bool inline_byte{false};      // +0Ch carried as a single byte
    bool vector3{false};          // +0Ch, +10h and +14h carried as three floats
    bool owned_string{false};     // +0Ch duplicated on the heap
    bool reference_payload{false};// the SceneReferencePayload fields
    bool reference_kind{false};   // +08h carried (clone only)
    bool shared_decl{false};      // +28h carried but not owned
    bool owned_block{false};      // +20h / +24h duplicated on the heap
    bool nested_bag{false};       // +0Ch deep-cloned through 008F41F0
    bool carries_ordinal{false};  // +34h copied onto the destination
};

// The 008F4F60 arm for a type. All twelve codes are handled; every arm allocates
// 38h bytes, runs one constructor, and copies +34h.
ScenePropertyCopySpec scene_property_clone_spec(ScenePropertyType type) noexcept;

// The 008F0700 arm for a type, selected by the DESTINATION's code at +04h.
// SubBag reports handled == false: code 6 has no arm and falls through to the
// tail, so 008F54F0's own recursion is the only thing that merges a sub-bag.
ScenePropertyCopySpec scene_property_assign_spec(ScenePropertyType type) noexcept;

// --- Array block rules ------------------------------------------------------

// record+24h is a BYTE SIZE, not an element count: 008EF2F0 and 008F03B0 pass it
// straight to operator new and memcpy, and 008EF360 stores count * 4 into it.
// 008EF7F0 is the inverse, and the only routine that recovers a count: 4-byte
// elements for FloatArray and IntArray, 12 for Vector3Array, and 0 for every
// other type including ByteArray, whose count and byte size are the same number.
std::uint32_t scene_property_array_elements(ScenePropertyType type,
                                            std::uint32_t byte_size) noexcept;

// --- Value assignment, the 008F0700 rule ------------------------------------

// Copy `source`'s value into `dest` the way 008F0700 does: switch on **dest**'s
// type, not the source's, and move only the fields that type owns. Returns false
// when the destination type has no arm (SubBag) or when the source does not
// carry the field the destination's arm reads. Only the SubBag case matches a
// native no-op: the native arms copy raw storage and never compare the two type
// codes, so a mismatched pair reinterprets the source's dword there and is
// refused here.
bool scene_property_assign_value_from(ScenePropertyValue& dest,
                                      const ScenePropertyValue& source) noexcept;

// --- The 008F23E0 rule ------------------------------------------------------

// Walk `source`'s entries and assign each one into the key of the same name in
// `dest`; a key absent from `dest` is dropped. A key present in both as a type-6
// record recurses on the two sub-bags. There is no keep-existing flag: the
// source always wins for a key the destination already has. This is the sibling
// of scene_property_bag_merge (008F54F0), which differs by cloning absent keys
// in and by honouring keep_existing.
void scene_property_bag_assign_existing(ScenePropertyBagModel& model,
                                        int dest_bag,
                                        const ScenePropertyBagModel& source_model,
                                        int source_bag) noexcept;

// --- Native sequence boundary -----------------------------------------------

// One method per native call site of 008F54F0 and 008F23E0, in the order the two
// routines reach them. Handles are opaque: nothing here models the 114h bag or
// the 38h record, and there are no default implementations.
struct ScenePropertyMergeHost {
    virtual ~ScenePropertyMergeHost() = default;

    // 00480690 with ECX = the 0Ch iterator block whose +00h is source_bag+4,
    // the embedded map. 008F5520 / 008F241A.
    virtual void* iterator_open(void* source_bag) = 0;
    // The `[iterator+4] != 0` loop test at 008F5546 / 008F242D.
    virtual bool iterator_valid(void* iterator) = 0;
    // 00484D20 with ECX = the iterator: the current entry's key text.
    // 008F5559 / 008F2446.
    virtual std::string iterator_key(void* iterator) = 0;
    // The current entry's record, `*(*(iterator+4) + 8)`. Read inline at
    // 008F5629, 008F55EF and 008F24E5.
    virtual void* iterator_record(void* iterator) = 0;
    // 0047E480 with ECX = the iterator: step to the next entry.
    // 008F5651 / 008F250E.
    virtual void iterator_advance(void* iterator) = 0;
    // free(iterator) at 008F5665 / 008F2523.
    virtual void iterator_close(void* iterator) = 0;

    // 0043B8B0 with ECX = the destination bag: the case-insensitive lookup.
    // Returns the found map entry's record, or null. 008F5588 / 008F249B.
    virtual void* find_in_dest(void* dest_bag, const std::string& key) = 0;

    // The type code at record+04h, compared against 6 at 008F55D0 / 008F24D8.
    virtual ScenePropertyType record_type(void* record) = 0;
    // The sub-bag pointer at record+0Ch, read on both sides at 008F55E4 /
    // 008F24EB before the recursion.
    virtual void* record_sub_bag(void* record) = 0;
    // The ordinal at record+34h, read from the source at 008F5626.
    virtual std::int32_t record_ordinal(void* record) = 0;
    // The ordinal store at clone+34h, 008F562E. The merge does this itself
    // rather than relying on the copy 008F4F60 already made.
    virtual void set_record_ordinal(void* record, std::int32_t ordinal) = 0;

    // 008F4F60 with ECX = the source record: a fresh 38h record. 008F561D.
    virtual void* clone_record(void* source_record) = 0;
    // 008F33F0 with ECX = the destination bag: insert under the key, stamp the
    // owner at +30h and the ordinal at +34h when it is still 0. 008F5636.
    virtual void insert_record(void* dest_bag, const std::string& key, void* record) = 0;
    // 008F0700 with ECX = the destination record. 008F560D / 008F2503.
    virtual void assign_value(void* dest_record, void* source_record) = 0;
};

// 008F54F0, __thiscall(dest /*ECX*/, source, char keep_existing), RET 8.
// Per source entry: absent from dest means clone, copy the source ordinal and
// insert; present as a type-6 record means recurse on the two sub-bags with the
// same flag; present otherwise means assign, but only when keep_existing is 0.
void scene_property_bag_merge_native(ScenePropertyMergeHost& host,
                                     void* dest_bag,
                                     void* source_bag,
                                     bool keep_existing);

// 008F23E0, __thiscall(dest /*ECX*/, source), RET 4. The same walk with no
// clone-and-insert branch and no flag.
void scene_property_bag_assign_existing_native(ScenePropertyMergeHost& host,
                                               void* dest_bag,
                                               void* source_bag);

} // namespace bsp
