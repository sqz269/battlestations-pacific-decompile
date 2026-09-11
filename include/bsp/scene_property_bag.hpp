#pragma once
// Scene property bag: the 114h-byte object that 008F5A00 fills from a `{ ... }`
// property body, the 38h-byte property record it stores, and the twelve type
// codes. See docs/SCENE_PROPERTY_BAG.md for the evidence behind every offset.
//
// Addresses: 008F5A00 008F2260 008F33F0 008F3370 008F38A0 008F41A0 008F41F0
//            008F54F0 008F0700 008F4F60
//
// Names below are hypotheses, not recovered symbols. The rules are pure
// projections of the native reads; nothing here is binary-compatible with the
// game's classes and no allocation layout is reproduced.
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// Type code stored at record+04h. 008F0700 switches on exactly these twelve
// values and 008F4F60 constructs one subclass per value; both agree that a
// record is 38h bytes whatever its type.
enum class ScenePropertyType : std::int32_t {
    Int = 0,           // "I"
    Float = 1,         // "F"
    String = 2,        // "S"
    Bool = 3,          // "B"
    Enum = 4,          // "E" and "LUA_S"; the two are told apart by decl+118h
    Reference = 5,     // "R" and the seven typed reference letters
    SubBag = 6,        // no letter: a nested `Key { ... }` block
    Vector3 = 7,       // "V3"
    ByteArray = 8,     // "BIN"
    FloatArray = 9,    // "FA"
    IntArray = 10,     // "IA"
    Vector3Array = 11, // "V3A"
};

// The reference sub-kind the parser puts in EDI before 008F0420/008F3AC0.
// Both dispatch chains (008F5FED..008F60A1 with a declaration, 008F616A..
// 008F6231 without) map the same eight letters to the same eight codes.
enum class SceneReferenceKind : std::int32_t {
    Any = 0,        // "R"
    Land = 1,       // "RLand"
    Fort = 2,       // "RFort"
    Ship = 3,       // "RShip"
    Plane = 4,      // "RPlane"
    PlaneOrShip = 5,        // "RPlnShp"
    LandPlaneOrShip = 6,    // "RLPlnShp"
    Path = 7,       // "RPath"
};

// How many value tokens the parser reads for a type once the letter is settled.
// Variable-length types report kSceneVariableTokenCount; their token count is
// the leading element count, and a `;` in the value position means "empty".
inline constexpr int kSceneVariableTokenCount = -1;

// Bag field offsets, from the constructor 008F41A0 and the inline copy of it at
// 008F66DB. The embedded hash map starts at +04h, so its bucket array (map+08h)
// is bag+0Ch and its 64 buckets end at bag+10Bh.
inline constexpr std::uint32_t kScenePropertyBagSize = 0x114;
inline constexpr std::uint32_t kScenePropertyBagMapOffset = 0x04;
inline constexpr std::uint32_t kScenePropertyBagCountOffset = 0x08;
inline constexpr std::uint32_t kScenePropertyBagBucketsOffset = 0x0C;
inline constexpr std::uint32_t kScenePropertyBagBucketCount = 0x40;
inline constexpr std::uint32_t kScenePropertyBagOrdinalOffset = 0x10C;
inline constexpr std::uint32_t kScenePropertyBagOwnerOffset = 0x110;

// Record field offsets, from the producers 008F38A0 (allocate + fill), 008F33F0
// (insert), 008F0700 (assign by type) and the parser's own stores.
inline constexpr std::uint32_t kScenePropertyRecordSize = 0x38;
inline constexpr std::uint32_t kScenePropertyRecordTypeOffset = 0x04;
inline constexpr std::uint32_t kScenePropertyRecordValueOffset = 0x0C;
inline constexpr std::uint32_t kScenePropertyRecordReferenceOffset = 0x18;
inline constexpr std::uint32_t kScenePropertyRecordArrayDataOffset = 0x20;
inline constexpr std::uint32_t kScenePropertyRecordArrayCountOffset = 0x24;
inline constexpr std::uint32_t kScenePropertyRecordDeclOffset = 0x28;
inline constexpr std::uint32_t kScenePropertyRecordOwnerOffset = 0x30;
inline constexpr std::uint32_t kScenePropertyRecordOrdinalOffset = 0x34;

// Letter override string on the declaration object at record+28h, read by
// 008F02B0 ("E") and 008F02E0 ("LUA_S") to split the two type-4 grammars.
inline constexpr std::uint32_t kSceneEnumDeclLetterOffset = 0x118;

// A decoded property value. The native record is a union over 0Ch..27h; this
// projection keeps every arm because the reconstruction has no type erasure.
struct ScenePropertyValue {
    ScenePropertyType type{ScenePropertyType::Int};
    std::int32_t int_value{0};       // Int, Enum (post-lookup), Bool as 0/1
    float float_value{0.0F};         // Float
    std::string text;                // String, and the unresolved Enum/Reference token
    float vector[3]{};               // Vector3
    SceneReferenceKind reference_kind{SceneReferenceKind::Any};
    std::vector<std::int32_t> int_array;   // IntArray
    std::vector<float> float_array;        // FloatArray, 3 per element for V3A
    std::vector<std::uint8_t> byte_array;  // ByteArray
};

// One entry of the bag, in the order 008F33F0 stamps at record+34h.
struct ScenePropertyEntry {
    std::string key;
    ScenePropertyValue value;
    std::int32_t ordinal{0};   // record+34h
    int sub_bag{-1};           // index into ScenePropertyBagModel::bags, type 6
};

// The bag as a value type. `bags` holds the tree: index 0 is the root, a type-6
// entry names its child by index. The native object is a 64-bucket hash map;
// key comparison is case-insensitive (0043B8B0 falls back to __stricmp), so
// lookups here are too.
struct ScenePropertyBagModel {
    struct Bag {
        std::vector<ScenePropertyEntry> entries;
        std::int32_t next_ordinal{0}; // bag+10Ch
    };
    std::vector<Bag> bags{Bag{}};
};

// --- Pure rules -------------------------------------------------------------

// The canonical letter for a type code. Type 4 reports "E"; "LUA_S" is the same
// code with a different declaration letter, and type 6 has no letter at all.
const char* scene_property_type_letter(ScenePropertyType type) noexcept;

// The letter for a reference sub-kind ("R", "RLand", ...).
const char* scene_reference_kind_letter(SceneReferenceKind kind) noexcept;

// Letter -> type code, in the order the parser tests them. Returns false for a
// letter the parser never matches. On a reference letter `kind` is filled in.
bool scene_property_type_for_letter(const std::string& letter,
                                    ScenePropertyType& type,
                                    SceneReferenceKind& kind) noexcept;

// Value tokens consumed after the letter, excluding the terminating `;`.
// Separator tokens count: "E" and "LUA_S" report 3 for `<decl> : <value>`.
int scene_property_value_token_count(ScenePropertyType type) noexcept;

// True when the parser reads a leading element count and may find `;` instead.
bool scene_property_type_is_counted(ScenePropertyType type) noexcept;

// The native element stride of a counted type, in bytes: 1 for BIN, 4 for FA
// and IA, 12 for V3A. Zero for every other type.
std::uint32_t scene_property_array_stride(ScenePropertyType type) noexcept;

// count * stride with the parser's saturating overflow guard (008F65E7:
// MUL EDX / SETO CL / NEG ECX / OR ECX,EAX yields FFFFFFFFh on overflow).
std::uint32_t scene_property_array_bytes(ScenePropertyType type,
                                         std::uint32_t count) noexcept;

// Decode the value tokens of one already-typed assignment. Returns false when
// the token list does not match the grammar for the type. `kind` is used only
// for Reference. Enum and Reference keep the final token in `text`; resolving
// it to an integer is a host call (0048E840 / 008F0420), not a rule.
bool scene_decode_property_value(ScenePropertyType type,
                                 SceneReferenceKind kind,
                                 const std::vector<std::string>& tokens,
                                 ScenePropertyValue& out) noexcept;

// Bag lookup, the 008F2260 rule: split the key at the first '.', find the head
// case-insensitively, and recurse into the child only when the head record is
// type 6. Returns nullptr for an absent property, which is how the native
// routine reports one (EAX = 0; see the doc's "Absent properties" section).
const ScenePropertyEntry* scene_property_bag_find(const ScenePropertyBagModel& model,
                                                  int bag_index,
                                                  const std::string& key) noexcept;

// Insert, the 008F33F0 rule: stamp the owner and, when the record has no
// ordinal yet, take the bag's counter and post-increment it.
ScenePropertyEntry& scene_property_bag_insert(ScenePropertyBagModel& model,
                                              int bag_index,
                                              const std::string& key,
                                              ScenePropertyValue value) noexcept;

// Merge, the 008F54F0 rule: walk `source`'s entries; a key absent from `dest`
// is cloned in with its source ordinal; a key present as a type-6 record
// recurses; a key present otherwise is overwritten only when keep_existing is
// false. 008F5AB3 merges a group with keep_existing = false; 0046DB66 merges
// with keep_existing = true.
void scene_property_bag_merge(ScenePropertyBagModel& model,
                              int dest_bag,
                              const ScenePropertyBagModel& source_model,
                              int source_bag,
                              bool keep_existing) noexcept;

// Deep copy, the 008F41F0 rule: a fresh bag with owner 0, every record cloned
// through 008F4F60 (which carries +34h across) and re-inserted under its key.
int scene_property_bag_clone(ScenePropertyBagModel& model,
                             const ScenePropertyBagModel& source_model,
                             int source_bag) noexcept;

// --- Parse sequence ---------------------------------------------------------

// One native call site per method, in the order 008F5A00 reaches them. There
// are no default implementations: nothing here stands in for game behaviour.
struct ScenePropertyBagHost {
    virtual ~ScenePropertyBagHost() = default;

    // 008D8A70 peek: the next token's text without consuming it. An empty
    // string is the tokenizer's end marker (the [ESI+4] tests at 008F60AA).
    virtual std::string peek_token() = 0;
    // 008D8960 consume: drop the peeked token.
    virtual void consume_token() = 0;
    // 008D9980 read: consume and return the next token.
    virtual std::string read_token() = 0;
    // 008D9930 expect: consume only on a match, and report an error otherwise.
    // A property authored without its `;` costs one error and consumes nothing.
    virtual bool expect_token(const std::string& text) = 0;
    // 008D9AD0 read int, 008D9B40 read float; both take an out-of-band ok flag.
    virtual std::int32_t read_int(bool& ok) = 0;
    virtual float read_float(bool& ok) = 0;
    // 008D8F40: does the next token parse as "%f"? Guards the untyped I and F
    // reads at 008F5B92 and 008F5C2E.
    virtual bool next_token_is_number() = 0;

    // 00469B60 on the registry passed as argument 3: group name -> bag index,
    // or -1. Called once per token of the leading `( name name ... )` list.
    virtual int lookup_group_bag(const std::string& name) = 0;
    // 0048E960 on the global enum registry at 00E1867C: enum name -> index for
    // an `E` assignment whose key has no declaration (008F5F94).
    virtual int lookup_enum_declaration(const std::string& name) = 0;
    // 0048E840 on the declaration at record+28h: value name -> integer, for an
    // `E` or `LUA_S` assignment whose key does have one (008F33A6).
    virtual std::int32_t resolve_enum_value(int declaration, const std::string& name) = 0;
    // 008EF6D0: one `BIN` token -> one byte.
    virtual std::uint8_t decode_byte_token(const std::string& token) = 0;
};

// What 008F5A00 did, for a caller that wants the outcome rather than the tree.
struct ScenePropertyParseResult {
    int bag_index{0};
    int properties_read{0};
    int groups_merged{0};
    int sub_blocks{0};
    int errors{0};       // failed expect_token calls
    bool ok{false};
};

// The 008F5A00 sequence over the host. `group_registry` is argument 3: the
// leading `( ... )` list is parsed only when it is non-null (008F5A4E), and
// `allow_untyped_numbers` is argument 2, which forces the I and F readers to
// read even when the next token does not look like a number.
//
// Coverage: complete for the assignment body and the group list. The `traffic`
// and `SceneBrowserGroups` bodies that docs/SCENE_FILE_READER.md lists as
// undecoded are the caller's business, not this routine's.
ScenePropertyParseResult scene_property_bag_parse(ScenePropertyBagHost& host,
                                                  ScenePropertyBagModel& model,
                                                  int bag_index,
                                                  bool group_registry,
                                                  bool allow_untyped_numbers);

}  // namespace bsp
