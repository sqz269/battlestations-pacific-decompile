#pragma once
// The entity kind predicate at vtable slot 5Ch, read as compiled code: the 88
// class-test bodies in `.text`, the set of query literals each one answers true
// for, and the catalogue of literals the engine actually asks with.
//
// docs/UNIT_KIND_QUERY.md. Addresses: 0042B8F0, 004351E0, 00435360, 0047F190,
// 00480930, 004F1750, 006D1610, 006D1650, 006DFE50, 006E3D50, 006FDE40,
// 006FE530, 0074E400 (class-test bodies decoded byte for byte; all 88 are in
// kUnitKindClassBodies), 00937CFD, 0090316F, 0064332E, 004C3D38, 0099A26D,
// 0077F2E6, 007AC9DE, 0072C6E1 (call sites).
//
// This builds on include/bsp/entity_class_ids.hpp, which recovered the id ->
// class -> parent table (packet cc2_class_id_table). Nothing here redeclares
// that table. The difference is the evidence direction: entity_class_ids.hpp
// models the rule as "query is on the parent chain", derived from the tree;
// this header carries the accepted-literal set that each compiled body really
// compares against, decoded from the body itself, so the two can be checked
// against one another. They agree for all 87 classes the peer packet recovered.
//
// Names below are hypotheses about meaning, not recovered symbols. The literal
// meanings are read from the accepting class set plus the call sites listed in
// the doc; they are not strings in the image.
//
// This is not an ABI-compatible replacement. The native test is a __thiscall
// virtual taking one stack argument and returning the answer in AL (RET 4);
// everything here is a pure function over the recovered tables.
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// One compiled class-test body
// ---------------------------------------------------------------------------

// Each body is a straight run of `CMP EAX,imm` over the owning class's whole
// ancestor chain (the root compare emitted as `TEST EAX,EAX`), then one
// `CMP EAX,[ECX+0C4h]` against the object's own most-derived id, then
// `XOR EAX,EAX / RET 4` or `MOV EAX,1 / RET 4`. No body calls anything: the
// chain is unrolled at compile time, so the predicate needs no host method.
struct UnitKindClassBody {
    int class_id;                // owning class, the id its vtable belongs to
    std::uint32_t test_address;  // first byte of the body in .text
    std::uint32_t vtable;        // a vtable that installs it at +5Ch
    const char* class_name;      // recovered literal, or nullptr
    const int* accepts;          // literals the compare run answers true for
    std::size_t accept_count;
};

// All 88 bodies, ordered by class id. 87 classes have one body; class 4Eh has
// two identical ones (004351E0 for vtable 00CE3E60, 00435360 for 00CE3FD0).
extern const UnitKindClassBody kUnitKindClassBodies[];
extern const std::size_t kUnitKindClassBodyCount;

// ---------------------------------------------------------------------------
// The predicate
// ---------------------------------------------------------------------------

// The class test as a pure function over the decoded compare runs: true when
// `literal` is in the accepted set compiled into the body of `class_id`'s own
// class test. Equivalent to the run-time routine for any object whose class
// installs its own test, because the object's `+C4h` id is then already in the
// compare run. False when `class_id` has no body in the image.
bool unit_is_kind_of(int class_id, int literal) noexcept;

// The full native rule, including the run-time `CMP EAX,[ECX+0C4h]`:
// `owner_class_id` is the class whose vtable supplies the body (it may be a
// base, for an object of a class that does not override slot 5Ch), and
// `dynamic_class_id` is the value at the object's +C4h.
bool unit_kind_body_answers(int owner_class_id, int dynamic_class_id, int literal) noexcept;

// The body record for a class id, or nullptr when the image has none.
const UnitKindClassBody* unit_kind_body_for_class(int class_id) noexcept;

// How many of the 88 classes answer true for `literal`. 0 means no object in
// the shipped image can answer it through a compiled compare run.
std::size_t unit_kind_member_count(int literal) noexcept;

// ---------------------------------------------------------------------------
// The literals the engine asks with
// ---------------------------------------------------------------------------

// Every distinct literal passed to slot 5Ch at a call site, with how many sites
// pass it and what its accepting class set is. `meaning` is this packet's
// reading of the set, not a recovered string.
struct UnitKindLiteralRow {
    int literal;
    int call_sites;        // sites in .text whose last push before the call is this literal
    int member_classes;    // classes whose compiled body accepts it
    const char* meaning;
};

extern const UnitKindLiteralRow kUnitKindLiterals[];
extern const std::size_t kUnitKindLiteralCount;

// nullptr when no call site in the image passes this literal.
const UnitKindLiteralRow* unit_kind_literal_row(int literal) noexcept;

// ---------------------------------------------------------------------------
// The literals with a settled meaning
// ---------------------------------------------------------------------------

// A subtree root: the query selects the class and everything under it.
inline constexpr int kUnitKindQueryAnyEntity = 0x00;        // all 88 bodies
inline constexpr int kUnitKindQueryCommandable = 0x02;      // 52 classes: units, devices, ordnance, squadrons
inline constexpr int kUnitKindQueryUnitOrDevice = 0x04;     // 36 classes: units plus the weapon-device family
inline constexpr int kUnitKindQueryUnit = 0x05;             // 25 classes: every ship, plane, land unit and structure
inline constexpr int kUnitKindQueryShip = 0x06;             // 9 classes: the ship family
inline constexpr int kUnitKindQueryDestroyer = 0x07;
inline constexpr int kUnitKindQuerySubmarine = 0x08;
inline constexpr int kUnitKindQueryCarrier = 0x09;          // MMothership
inline constexpr int kUnitKindQueryLandingShip = 0x0C;
inline constexpr int kUnitKindQueryTorpedoBoat = 0x0E;
inline constexpr int kUnitKindQueryPlane = 0x0F;            // 9 classes: the plane family
inline constexpr int kUnitKindQueryBomberPlane = 0x10;
inline constexpr int kUnitKindQueryReconPlane = 0x14;       // 3 classes
inline constexpr int kUnitKindQueryKamikazePlane = 0x17;
inline constexpr int kUnitKindQuerySquadron = 0x18;         // PlaneSquadronGen
inline constexpr int kUnitKindQueryLandStructure = 0x1B;    // MLandFort and MCommandBuilding
inline constexpr int kUnitKindQueryCommandBuilding = 0x1C;
inline constexpr int kUnitKindQueryDeviceBase = 0x1E;       // 10 classes: the weapon-device family root
inline constexpr int kUnitKindQueryGun = 0x20;              // 9 classes: the gun/platform family
inline constexpr int kUnitKindQueryTurningGun = 0x22;       // 4 classes: RT, ST, depth-charge launcher
inline constexpr int kUnitKindQuerySternGun = 0x24;         // MSTGun and MDepthChargeLauncher
inline constexpr int kUnitKindQueryBombPlatform = 0x25;     // 2 classes
inline constexpr int kUnitKindQueryCatapult = 0x28;
inline constexpr int kUnitKindQueryBullet = 0x29;           // MBullet and MFlakBullet
inline constexpr int kUnitKindQueryOrdnance = 0x2A;         // 10 classes: the MBomb family
inline constexpr int kUnitKindQueryTorpedo = 0x2B;
inline constexpr int kUnitKindQueryNavPoint = 0x41;
inline constexpr int kUnitKindQueryLandscape = 0x44;
inline constexpr int kUnitKindQueryAirfield = 0x45;
inline constexpr int kUnitKindQueryShipyard = 0x46;
inline constexpr int kUnitKindQueryPath = 0x47;

// Asked at three call sites and accepted by nothing: no class test in the image
// carries 1Fh and no constructor immediate stamps it at +C4h, so every one of
// those tests is false in the shipped build. See the doc's Corrections section.
inline constexpr int kUnitKindQueryUnreachable1F = 0x1F;

// Convenience readings of the settled family literals.
inline bool unit_kind_is_unit(int class_id) noexcept { return unit_is_kind_of(class_id, kUnitKindQueryUnit); }
inline bool unit_kind_is_ship(int class_id) noexcept { return unit_is_kind_of(class_id, kUnitKindQueryShip); }
inline bool unit_kind_is_plane(int class_id) noexcept { return unit_is_kind_of(class_id, kUnitKindQueryPlane); }
inline bool unit_kind_is_gun(int class_id) noexcept { return unit_is_kind_of(class_id, kUnitKindQueryGun); }
inline bool unit_kind_is_ordnance(int class_id) noexcept { return unit_is_kind_of(class_id, kUnitKindQueryOrdnance); }

}  // namespace bsp
