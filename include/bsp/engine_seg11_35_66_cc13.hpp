#pragma once

// Segments 35 (00701B70-0070BBA0: messages, whoSaysThat, data tables, fish schools and flags),
// 66 (0098C630-00996060: hardware detection, the BSM_HWD registry key, language, the online system
// base and the downloadable-content manager) and 11 (004E7DD0-004F9D30: scene-database entities,
// 2D geometry helpers and the front-end screen sets): leaves that had a reviewed ledger name but no
// reconstruction record and no tag (cc13-sys3, docs/CC13_SYS_3.md).
//
// Conventions, as in the sibling cc13 files (src/engine_services_cc13.cpp, src/band_0067_cc13.cpp):
// - Native layouts are reproduced where the routine's behaviour is a layout operation (strings,
//   {data, count, capacity} arrays, MSVC 2005 tree nodes). Strings are bsp::NativeString with an
//   explicit NativeStringStorage (0041DD40 resize, the 00419CC0 / 00BD1510 release pair).
// - Image globals are never invented. Every global word a routine reads is passed in, and every
//   call to a routine owned elsewhere goes through a host interface whose virtuals name the image
//   address.
// - Intrusive references release as the image does: InterlockedDecrement on +4h and, at zero, the
//   object's vtable slot 0. Hosts implement that through release_reference.
// - Presentation draws from the shared generator 00BD2F10 (stream 0) go through a host callback, so
//   a host can give them their own stream (cc9 RNG rule).
// - Original C++ EH and SEH frames are not claimed. Nothing here is wired into a game host. Names
//   are hypotheses, not recovered symbols.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_string.hpp"
#include "bsp/scheduled_voice_types.hpp"
#include "bsp/voice_playback.hpp"

namespace bsp::engine_seg11_35_66_cc13 {

// ---------------------------------------------------------------------------------------------------
// Shared native shapes.

// MSVC 2005 red-black tree node links: left +0h, parent +4h, right +8h, value from +0Ch. The colour
// and nil bytes sit after the value and their offsets depend on the value size.
struct NativeTreeLinks {
    NativeTreeLinks* left;
    NativeTreeLinks* parent;
    NativeTreeLinks* right;
};

// Tree header: allocator word +0h, head node +4h, size +8h.
struct NativeTreeHeader {
    std::uint32_t allocator;
    NativeTreeLinks* head;
    std::uint32_t size;
};

// Vector header as these instantiations lay it out: allocator word +0h, first +4h, last +8h,
// end-of-storage +0Ch.
struct NativeVectorHeader {
    std::uint32_t allocator;
    std::uint8_t* first;
    std::uint8_t* last;
    std::uint8_t* end;
};

// {data, count} prefix of the engine's own arrays (count at +4h, capacity at +8h when present).
struct NativeIntArray {
    const std::int32_t* data;
    std::int32_t count;
};

// The six-slot property visitor (abstract base vtable 00CE374C; the Lua reader 00CE44FC and the
// archive writers implement it). Every argument is an eight-byte {tag, word} pair (GuiLuaVariant).
// The writer side uses the same slots: +0Ch writes a key/value pair.
class PropertyVisitor {
public:
    virtual ~PropertyVisitor() = default;
    virtual void enter_04(const GuiLuaVariant& key) = 0;                       // vtable +4h, RET 8
    virtual void leave_08() = 0;                                               // vtable +8h, RET
    virtual bool read_or_default_0c(const GuiLuaVariant& key, const GuiLuaVariant& field,
                                    const GuiLuaVariant& fallback) = 0;        // vtable +0Ch, RET 18h
    virtual bool read_10(const GuiLuaVariant& key, const GuiLuaVariant& field) = 0;  // +10h, RET 10h
    virtual bool has_key_14(const GuiLuaVariant& key) = 0;                     // vtable +14h, RET 8
};

// The archive writer as 00703020 and 00703380 drive it: slot +0Ch with two pairs, key and value.
class PropertyWriter {
public:
    virtual ~PropertyWriter() = default;
    virtual void write_0c(const GuiLuaVariant& key, const GuiLuaVariant& value) = 0;
};

GuiLuaVariant variant_name(const char* text) noexcept;           // tag 0, a key by name / a string
GuiLuaVariant variant_index(std::int32_t index) noexcept;        // tag 1, a key by index / an integer
GuiLuaVariant variant_field(std::int32_t type, void* address) noexcept;  // field type and address

// ===================================================================================================
// Segment 35: the message manager (vtable 00CFD3A4, 70h bytes, game +21DCh).
// ===================================================================================================

// 13h category bytes plus the three group flags the loaders fill (manager +20h/+33h/+46h and
// +59h/+5Ch/+5Fh).
struct MessageCategoryFilter {
    std::array<std::uint8_t, 0x13> category{};
};
struct MessageGroupFlags {
    std::uint8_t group_0_6_13{};   // categories 0..6 and 0Dh
    std::uint8_t group_7_12{};     // categories 7..0Ch
    std::uint8_t group_14_17{};    // categories 0Eh..11h
};

// 007021C0, __stdcall(filter*, flags*, const int array*, bool enabled), RET 10h. Zeroes both
// outputs, then (only when enabled) sets filter[id] for every listed id and the matching group flag.
// Ids outside 0..12h still store filter[id], as the image does; the caller's arrays are config data.
void init_message_filter_007021c0(MessageCategoryFilter& filter, MessageGroupFlags& flags,
                                  const NativeIntArray& ids, bool enabled) noexcept;

// The 70h-byte manager in its native layout.
struct MessageManagerLayout {
    std::uint32_t vtable_00;            // 00CFD3A4
    NativeTreeHeader records_04;        // message records keyed by name (CI), node 58h, nil +55h
    std::uint32_t allocator_10;
    void* names_first_14;               // vector of NativeString (actor names; freed by the dtor)
    void* names_last_18;
    void* names_end_1c;
    MessageCategoryFilter filter_20;
    MessageCategoryFilter filter_33;
    MessageCategoryFilter filter_46;
    MessageGroupFlags flags_59;
    MessageGroupFlags flags_5c;
    MessageGroupFlags flags_5f;
    std::uint8_t pad_62[2];
    NativeTreeHeader loaded_maps_64;    // string set of "name#side" keys, node 18h, nil +15h
};
static_assert(offsetof(MessageManagerLayout, filter_20) == 0x20);
static_assert(offsetof(MessageManagerLayout, flags_59) == 0x59);
static_assert(offsetof(MessageManagerLayout, loaded_maps_64) == 0x64);
static_assert(sizeof(MessageManagerLayout) == 0x70);

// Routines the message manager calls that are reconstructed or owned elsewhere.
class MessageManagerCalls {
public:
    virtual ~MessageManagerCalls() = default;
    virtual NativeStringStorage& strings() = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // Tree heads: 00702480 (record map, nil byte +55h) and 004C26B0 (string set, nil byte +15h).
    virtual NativeTreeLinks* buy_record_head_00702480() = 0;
    virtual NativeTreeLinks* buy_string_set_head_004c26b0() = 0;
    // Destructor helpers: 004D1A50 erases the string set, 00705D30 the record map, 00432050 destroys
    // a NativeString range. 00BF65AC frees.
    virtual void erase_string_set_004d1a50(NativeTreeHeader& set) = 0;
    virtual void erase_record_map_00705d30(NativeTreeHeader& map) = 0;
    virtual void destroy_string_range_00432050(void* first, void* last) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // The record map: 00705C50 operator[] (creates a default record), 00702300 lower_bound,
    // 007030D0 the reverse lookup that returns the node's key string for a record.
    virtual VoiceClipRecord& lookup_record_00705c50(const NativeString& name) = 0;
    virtual const NativeString* lower_bound_key_00702300(const NativeString& name) = 0;  // null at end
    virtual const NativeString& record_key_007030d0(const VoiceClipRecord* record) = 0;
    // 00705E00: resolve a record by name (second word unused), as the archive reader does.
    virtual const VoiceClipRecord* resolve_record_00705e00(const NativeString& name) = 0;
    // String set of loaded maps (+64h): 004BE7xx walk, find, erase, insert.
    virtual std::vector<std::string> loaded_map_keys() = 0;          // in tree order
    virtual bool loaded_map_contains(const std::string& key) = 0;    // 0044xxxx find != end
    virtual void loaded_map_erase_004cf8a0(const std::string& key) = 0;
    virtual void loaded_map_insert(const std::string& key) = 0;
    // Lua state owner and reader (00B66BD0 construct, 00B6A020 open(1), 00B69D40 run, 00B67980
    // globals, 004425C0 reader, 00B669A0 close). The host returns the visitor over the globals.
    virtual PropertyVisitor* open_script_reader(const std::string& path) = 0;
    virtual void close_script_reader(PropertyVisitor* reader) = 0;
    // 00BD5F50 key enumeration (up to FAh keys; string keys carry their text).
    virtual std::vector<GuiLuaVariant> enumerate_keys_00bd5f50(PropertyVisitor& reader) = 0;
    // 0057C1A0, ECX = 4: loading progress for phase 4.
    virtual void report_loading_progress_0057c1a0(float fraction) = 0;
    // References: InterlockedIncrement / InterlockedDecrement at +4h, slot 0 at zero.
    virtual void retain_reference(void* object) = 0;
    virtual void release_reference(void* object) = 0;
    // The file store [0109CEEC]: vtable +8h and +0Ch existence probes.
    virtual bool file_store_exists_vslot8(const std::string& path) = 0;
    virtual bool file_store_exists_vslotc(const std::string& path) = 0;
    // 00A83FD0, ECX = [00F8BBE8]: the sound bank for a path, returned with one reference.
    virtual void* load_sound_bank_00a83fd0(const std::string& path) = 0;
    // 008D57A0 on 00F88980: the current language name.
    virtual std::string language_name_008d57a0() = 0;
    // 00E08FCC: the four voice directory templates, indexed by slot (null ends nothing; a null
    // entry skips the slot).
    virtual const char* voice_directory_template_00e08fcc(std::size_t slot) = 0;
    // 00706760 helpers: the profile-hints singleton's +8h, the language list and selection, the
    // DL_Content_0000062 ownership test (008D57A0 / 00BC... per caller), and the 1Ch FileBlock
    // (00BE0A30 construct with flag 1, deleted through vtable +4h(1)).
    virtual std::int32_t profile_hints_mode_plus8() = 0;
    virtual std::vector<std::string> collect_language_names() = 0;
    virtual void select_language_by_name(const std::string& name) = 0;
    virtual bool profile_owns_content(const std::string& content_id) = 0;
    virtual void* construct_file_block_00be0a30(const std::string& name) = 0;  // null if new failed
    virtual void delete_file_block(void* block) = 0;
    // 00BF8417 as 007072C0 uses it on the side text.
    virtual std::int32_t atol_00bf8417(const char* text) = 0;
    // 00704190 (src/message_record_resolver.cpp): resize the record's timed keys, filling with a key
    // whose two strings are zeroed (its timing words and flag are the caller's uninitialized stack).
    virtual void resize_timed_keys_00704190(VoiceTimedKeys& keys, std::uint32_t count) = 0;
};

// 007026B0 (__fastcall ECX = tree, RET) and 00702A30 (same body, RET 8, returns the tree): buy the
// head node, set its nil byte (+55h), self-link it and zero the size.
void construct_record_tree_007026b0(NativeTreeHeader& tree, MessageManagerCalls& calls);
NativeTreeHeader& construct_record_tree_00702a30(NativeTreeHeader& tree, MessageManagerCalls& calls);

// 00702A80, __stdcall(out NativeString*, const NativeString* name), RET 8, returns out:
// "Scripts/datatables/messagemaps/" + name + ".lua" (00CFD2C8).
std::string message_map_path_00702a80(const std::string& name);

// 00702BD0, __fastcall(ECX = record), RET. Appends " (Unloaded message!)" to the text, sets the
// sound id (+8h) to -1 and releases the four resource references (+20h..+2Ch), zeroing each slot.
void mark_record_unloaded_00702bd0(VoiceClipRecord& record, MessageManagerCalls& calls);

// 00703020, __thiscall(ECX = manager, writer*), RET 4. Writes every loaded map key (the +64h set,
// in order) as {1, index} -> {0, text}. Null text writes "" (00E19AC3). The ledger name
// BSP_MessageMap_CountEntries undersells it: it is the save side of 007072C0.
void write_loaded_message_maps_00703020(MessageManagerCalls& calls, PropertyWriter& writer);

// 00703230, __thiscall(ECX = key, const key*), RET 4, returns this: the two strings by the inline
// self-guarded copy, then +10h, +14h and the flag byte +18h.
VoiceTimedKey& assign_timed_key_00703230(VoiceTimedKey& to, const VoiceTimedKey& from,
                                         NativeStringStorage& strings);

// 00703170 copy construct and 007029B0 destroy as the STL helpers below call them (existing
// reconstructions in src/message_record_resolver.cpp).
class TimedKeyOps {
public:
    virtual ~TimedKeyOps() = default;
    virtual void copy_construct_00703170(VoiceTimedKey* destination, const VoiceTimedKey& source) = 0;
    virtual void destroy_007029b0(VoiceTimedKey& key) = 0;
    virtual void assign_00703230(VoiceTimedKey& to, const VoiceTimedKey& from) = 0;
};

// 007032F0 (__fastcall ECX = slot, EDX = source, RET) and 00703530 (__cdecl, RET 8 form): placement
// copy construct when the slot is not null.
void construct_timed_key_007032f0(VoiceTimedKey* slot, const VoiceTimedKey& source, TimedKeyOps& ops);

// 007034A0, __cdecl(first, last, dest): forward assignment over [first, last); returns the
// destination end. 00703620 is the backward form (dest is the END of the destination range) and
// returns the destination begin.
VoiceTimedKey* copy_timed_keys_007034a0(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                        VoiceTimedKey* dest, TimedKeyOps& ops);
VoiceTimedKey* copy_backward_timed_keys_00703620(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                                 VoiceTimedKey* dest_end, TimedKeyOps& ops);

// 00703590 / 00703680, __fastcall(ECX = first, EDX = last, dest, three unused words), RET 10h:
// copy construct into raw storage; on an exception destroy what was built and rethrow (00BF6885).
VoiceTimedKey* uninitialized_copy_timed_keys_00703590(const VoiceTimedKey* first, const VoiceTimedKey* last,
                                                      VoiceTimedKey* dest, TimedKeyOps& ops);

// 00703860, __fastcall(ECX = first, EDX = count, value*, unused...), RET 10h; 00703AA0 is the
// thiscall wrapper (ECX unused allocator, first/count/value, RET 0Ch) that returns first + count.
void uninitialized_fill_timed_keys_00703860(VoiceTimedKey* first, std::uint32_t count,
                                            const VoiceTimedKey& value, TimedKeyOps& ops);
VoiceTimedKey* uninitialized_fill_timed_keys_00703aa0(VoiceTimedKey* first, std::uint32_t count,
                                                      const VoiceTimedKey& value, TimedKeyOps& ops);

// 00703A70, __cdecl(first, last), RET 8 form: destroy each key.
void destroy_timed_keys_00703a70(VoiceTimedKey* first, VoiceTimedKey* last, TimedKeyOps& ops);

// Allocation hooks for the vector instantiation: 00701CA0 allocate (count of 1Ch keys), 00BF65AC
// free, 00703710 the "vector<T> too long" throw (does not return).
class TimedKeyStorage {
public:
    virtual ~TimedKeyStorage() = default;
    virtual VoiceTimedKey* allocate_00701ca0(std::uint32_t count) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    [[noreturn]] virtual void throw_length_error_00703710() = 0;
};

// 00703920, __thiscall(ECX = vector, n), RET 4: zero the three pointers; n == 0 returns false;
// n > 09249249h throws; otherwise allocate n keys and return true.
bool buy_timed_keys_00703920(NativeVectorHeader& vector, std::uint32_t n, TimedKeyStorage& storage);

// 00703CE0, __thiscall(ECX = vector, position owner, position node, count, fill*), RET 10h:
// vector<key>::_Insert_n. The fill value is copied into a local first (it may alias the vector).
// The middle-insertion helpers are passed in: 00703BE0 (uninitialized copy into raw tail storage,
// returns the end), 007039A0 (copy_backward), 007037B0 (fill assign).
class TimedKeyInsertHelpers {
public:
    virtual ~TimedKeyInsertHelpers() = default;
    virtual VoiceTimedKey* uninitialized_move_00703be0(VoiceTimedKey* first, VoiceTimedKey* last,
                                                       VoiceTimedKey* dest) = 0;
    virtual void copy_backward_007039a0(VoiceTimedKey* first, VoiceTimedKey* last,
                                        VoiceTimedKey* dest_end) = 0;
    virtual void fill_007037b0(VoiceTimedKey* first, VoiceTimedKey* last, const VoiceTimedKey& value) = 0;
};
void insert_fill_timed_keys_00703ce0(NativeVectorHeader& vector, VoiceTimedKey* position,
                                     std::uint32_t count, const VoiceTimedKey& fill, TimedKeyOps& ops,
                                     TimedKeyStorage& storage, TimedKeyInsertHelpers& helpers);

// The 58h-byte record map node value: key NativeString +0Ch, record +14h (value offsets from the
// node; the pair itself starts at node +0Ch).
struct MessageRecordPair {
    NativeString key;
    VoiceClipRecord record;
};

// Record routines owned by src/message_record_resolver.cpp (00704060 destroy, 007042B0 copy).
class MessageRecordOps {
public:
    virtual ~MessageRecordOps() = default;
    virtual void destroy_record_00704060(VoiceClipRecord& record) = 0;
    virtual void copy_construct_record_007042b0(VoiceClipRecord& to, const VoiceClipRecord& from) = 0;
};

// 00704120, __fastcall(ECX = pair), RET: destroy the record, then release the key string.
void destroy_record_pair_00704120(MessageRecordPair& pair, MessageRecordOps& ops, NativeStringStorage& strings);

// 007043E0, __thiscall(ECX = fresh pair, const pair*), RET 4, and 007044A0, __thiscall(ECX = fresh
// pair, const key*, const record*), RET 8: zero the key, self-guarded key copy, then copy construct
// the record.
MessageRecordPair& copy_construct_record_pair_007043e0(MessageRecordPair& to, const MessageRecordPair& from,
                                                       MessageRecordOps& ops, NativeStringStorage& strings);
MessageRecordPair& copy_construct_record_pair_007044a0(MessageRecordPair& to, const NativeString& key,
                                                       const VoiceClipRecord& record, MessageRecordOps& ops,
                                                       NativeStringStorage& strings);

// The record-map node (native 58h: links, the pair at +0Ch, colour +54h, nil +55h). The pair is a
// semantic VoiceClipRecord here, so offsets past the links are not native.
struct MessageRecordNode {
    MessageRecordNode* left;
    MessageRecordNode* parent;
    MessageRecordNode* right;
    MessageRecordPair value;
    std::uint8_t color;
    std::uint8_t is_nil;
};

// 007052F0, __cdecl(left, parent, right, const pair*, color), RET 14h form: operator new(58h) (null
// skips construction), links, pair copy (007043E0), colour, nil = 0. The allocator returns raw
// storage of sizeof(MessageRecordNode) or null.
class RecordNodeAllocator {
public:
    virtual ~RecordNodeAllocator() = default;
    virtual void* new_node_00bf681b() = 0;
    virtual void delete_node_00bf65ac(void* storage) = 0;
};
MessageRecordNode* buy_record_node_007052f0(MessageRecordNode* left, MessageRecordNode* parent,
                                            MessageRecordNode* right, const MessageRecordPair& value,
                                            std::uint8_t color, RecordNodeAllocator& allocator,
                                            MessageRecordOps& ops, NativeStringStorage& strings);

// 00705980, __thiscall(ECX = tree, node), RET 4: post-order destruction of a subtree (right
// recursion, left iteration): destroy the record, release the key, free the node.
void destroy_record_subtree_00705980(MessageRecordNode* node, RecordNodeAllocator& allocator,
                                     MessageRecordOps& ops, NativeStringStorage& strings);

// Tree internals of the record map (owned elsewhere): 007053A0 _Insert(add_left, where, value),
// 00701BC0 decrement, 00701FC0 increment, 00443D00 case-insensitive less on two NativeStrings.
class RecordTreeInternals {
public:
    virtual ~RecordTreeInternals() = default;
    virtual MessageRecordNode* insert_at_007053a0(bool add_left, MessageRecordNode* where,
                                                  const MessageRecordPair& value) = 0;
    virtual MessageRecordNode* decrement_00701bc0(MessageRecordNode* node) = 0;
    virtual MessageRecordNode* increment_00701fc0(MessageRecordNode* node) = 0;
    virtual bool less_ci_00443d00(const NativeString& left, const NativeString& right) = 0;
    virtual const NativeString& key_of(const MessageRecordNode* node) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
};

// 007055A0, __thiscall(ECX = tree, out {node, inserted}*, const pair*), RET 8: _Insert for unique
// keys. The descent compares with the inline null-aware __stricmp; the final duplicate test uses
// 00443D00.
struct RecordInsertResult {
    MessageRecordNode* node;
    bool inserted;
};
RecordInsertResult insert_unique_record_007055a0(NativeTreeHeader& tree, const MessageRecordPair& value,
                                                 RecordTreeInternals& internals);

// 00705A20, __thiscall(ECX = tree, out iterator*, hint owner, hint node, const pair*), RET 10h:
// insert with hint. The hint owner must be this tree (else 00BF6713, which may return).
MessageRecordNode* insert_record_with_hint_00705a20(NativeTreeHeader& tree, const void* hint_owner,
                                                    MessageRecordNode* hint, const MessageRecordPair& value,
                                                    RecordTreeInternals& internals);

// 00703380, vtable 00CF0DD0 slot 0, __thiscall(ECX = reference, writer*), RET 4: writes
// "msg" -> the record's key (via 007030D0 on [[00E188A8]+21DCh]) and "whoSaysThat" -> +8h.
void write_message_ref_00703380(const VoiceClip& reference, PropertyWriter& writer, MessageManagerCalls& calls);

// 00703400, __fastcall(ECX = text, EDX = prefix), RET: text.Substring(0, prefix.length) (00469840)
// equals prefix case-insensitively (00435C40: length, then __stricmp).
bool has_prefix_insensitive_00703400(const std::string& text, const std::string& prefix);

// 00706060, __fastcall(ECX = out reference, EDX = reader), RET, returns out: reads "msg" as a string
// and "whoSaysThat" as an integer (no defaults: a missing key leaves the zeroed name and the
// uninitialized speaker word, here the caller's), resolves the record through 00705E00 and stores
// vtable 00CF0DD0, record and speaker.
VoiceClip& read_message_ref_00706060(VoiceClip& out, PropertyVisitor& reader, MessageManagerCalls& calls,
                                     std::int32_t speaker_scratch = 0);

// 00704590, __thiscall(ECX = record, reader*, const uint8 flags[4]*), RET 8. Reads one message
// record from the table on top: "text"; the "subtitles" array of timed keys; "voice"; and, for a
// non-empty voice, "actor" and the per-slot sound banks. See the .cpp for the order.
void read_message_record_00704590(VoiceClipRecord& record, PropertyVisitor& reader,
                                  const std::array<std::uint8_t, 4>& flags, MessageManagerCalls& calls);

// 00706160, __thiscall(ECX = manager, bool load, const NativeString* path, const uint8 flags[4]*,
// int side), RET 10h. Opens the script, walks "Actors" (each value read as a string and discarded),
// then "Messages" (and [side] when side != -1). Loading reads each record (00704590); unloading
// marks every record that exists (00702BD0).
void load_actors_and_messages_00706160(MessageManagerCalls& calls, bool load, const std::string& path,
                                       const std::array<std::uint8_t, 4>& flags, std::int32_t side);

// 007065E0 inputs: the local player's side record kind (game +18CCh[game +18ECh] +28h), the options
// block (game +5FCh): its +1064h enable byte and the three id arrays +1068h, +1074h, +1080h.
struct MessageLoadOptions {
    std::int32_t local_side_kind;
    bool filters_enabled;
    NativeIntArray ids_1068;
    NativeIntArray ids_1074;
    NativeIntArray ids_1080;
};
// 007065E0, __thiscall(ECX = manager, int side), RET 4. Builds the three filters (the first two
// swap their id arrays when the side kind is not 0), sets flags[1], [2] or [3] by side kind (0 ->
// 1, 1 -> 2, other -> 3), then loads Scripts/datatables/Messages.lua.
std::array<std::uint8_t, 4> load_messages_007065e0(MessageManagerLayout& manager, const MessageLoadOptions& options,
                                                    std::int32_t side, MessageManagerCalls& calls);

// 00706760, __thiscall(ECX = manager, const NativeString* name, int side), RET 8. Loads a message
// map once per "name#side" key. See the .cpp for the profile-hints mode 2 language sweep and the
// voice FileBlock ("<language>_<key>" or "englishauthentic_<key>").
void load_message_map_00706760(MessageManagerCalls& calls, const std::string& name, std::int32_t side);

// 00706FC0, __thiscall(ECX = manager, const NativeString* name, int side), RET 8: when "name#side"
// is loaded, erase it and run the unload walk over the map's script.
void unload_message_map_00706fc0(MessageManagerCalls& calls, const std::string& name, std::int32_t side);

// 007072C0, __thiscall(ECX = manager, reader*), RET 4: the load side of 00703020. For index 0, 1,
// ... while {1, index} exists: read the key text, split it at the LAST '#', load_message_map(name,
// atol(side)).
void load_message_map_list_007072c0(MessageManagerCalls& calls, PropertyVisitor& reader);

// 00707480, __fastcall(ECX = manager), RET: vtable 00CFD3A4, the record map head, the zeroed name
// vector, the string set head. The filters and flags are left as they are.
MessageManagerLayout& construct_message_manager_00707480(MessageManagerLayout& manager, MessageManagerCalls& calls);

// 007071F0, __fastcall(ECX = manager), RET: vtable 00CFD3A4, erase and free the string set, destroy
// and free the name vector, erase and free the record map.
void destruct_message_manager_007071f0(MessageManagerLayout& manager, MessageManagerCalls& calls);

// ===================================================================================================
// Segment 35: fish-school and flag model pools (ambient presentation).
//
// Both are instances of one pool template (Mit::allocator_list_elem base, vtable 00CFD410 for fish,
// 00CFD480 for flags; static instances 00E19ACC and 00E19B14). Layout: +0h vtable, +4h/+8h the
// allocator-list links (prev, next; list head 00E188B4), +0Ch CRITICAL_SECTION (18h), +24h lock
// depth, +28h block array {data, count +2Ch, capacity +30h}, +34h lowest block with a free slot or
// -1. A block holds the slots, then a uint16 free stack and a uint16 free count; every slot keeps
// its block index in its last dword. src/band_0067_cc13.cpp has a bookkeeping-only model of the
// allocate/free pair; this is the native-layout form.
// ===================================================================================================

struct PoolGeometry {
    std::uint32_t block_bytes;       // 320h fish, 3444h flag
    std::uint32_t slot_bytes;        // 18Ch fish, 1A0h flag
    std::uint16_t slots_per_block;   // 2 fish, 20h flag
    std::uint32_t free_stack_offset; // 318h fish, 3400h flag
    std::uint32_t free_count_offset; // 31Ch fish, 3440h flag
    std::uint32_t slot_index_offset; // 188h fish, 19Ch flag
    std::uint32_t vtable;            // 00CFD410 fish, 00CFD480 flag
};
inline constexpr PoolGeometry kFishSchoolPool{0x320, 0x18C, 2, 0x318, 0x31C, 0x188, 0x00cfd410};
inline constexpr PoolGeometry kFlagPool{0x3444, 0x1A0, 0x20, 0x3400, 0x3440, 0x19C, 0x00cfd480};

// {data, count, capacity} of dwords as the pool's block array uses it (growth 2n+2).
struct NativeDwordVector {
    std::uint32_t* data;
    std::uint32_t count;
    std::uint32_t capacity;
};

struct ModelPoolLayout {
    std::uint32_t vtable_00;
    ModelPoolLayout* list_prev_04;
    ModelPoolLayout* list_next_08;
    std::uint8_t section_0c[0x18];
    std::int32_t lock_depth_24;
    NativeDwordVector blocks_28;     // block pointers stored as dwords (Win32)
    std::uint32_t first_free_34;
};
static_assert(sizeof(void*) != 4 || sizeof(ModelPoolLayout) == 0x38, "native pool size on Win32");

// operator new (00BF681B), free (00BF9DC8 / 00BF65AC) and the Win32 critical-section calls.
class PoolMemory {
public:
    virtual ~PoolMemory() = default;
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;  // may return null
    virtual void free_00bf9dc8(void* block) = 0;
    virtual void enter(void* section) = 0;
    virtual void leave(void* section) = 0;
    virtual void initialize(void* section) = 0;
    virtual void remove(void* section) = 0;  // DeleteCriticalSection
};

// 007076E0 / 00709920, __thiscall(ECX = vector, n), RET 4: capacity = n, reallocate, copy, free.
void reserve_exact_dword_vector_007076e0(NativeDwordVector& v, std::uint32_t n, PoolMemory& memory);
// 00707730 / 00709970, __fastcall(ECX = vector), RET: capacity = 2 * capacity + 2, reallocate.
void grow_dword_vector_00707730(NativeDwordVector& v, PoolMemory& memory);
// 00707A10 / 0070AD20, __thiscall(ECX = vector, const dword*), RET 4: grow when full, append.
void push_back_dword_vector_00707a10(NativeDwordVector& v, std::uint32_t value, PoolMemory& memory);
// 00707A90 / 0070ADA0, __thiscall(ECX = vector, n), RET 4: reserve when n > capacity.
void reserve_dword_vector_00707a90(NativeDwordVector& v, std::uint32_t n, PoolMemory& memory);
// 007076B0 / 007098F0, __fastcall(ECX = owner), RET: free the block at +0h when set (unwind only).
void free_owned_block_007076b0(void** owner, PoolMemory& memory);

// 007075D0 / 00709810, __thiscall(ECX = block, pool index), RET 4: free stack {N-1, ..., 0}, count
// N, every slot's index dword = the pool index.
void init_pool_block_007075d0(std::uint8_t* block, std::uint32_t pool_index, const PoolGeometry& geometry);

// 007086C0 / 0070AE10, __fastcall(ECX = pool), RET: link at the head of the allocator list (list
// head passed in: 00E188B4), vtable, InitializeCriticalSection, zeroed block array, first-free -1,
// then reserve 20h block slots.
ModelPoolLayout& construct_model_pool_007086c0(ModelPoolLayout& pool, ModelPoolLayout*& allocator_list_head,
                                               const PoolGeometry& geometry, PoolMemory& memory);

// 007087F0 / 0070AF40, __fastcall(ECX = pool), RET: free every block and the array, leave the
// section once per remaining lock depth, delete it, unlink from the allocator list.
void destruct_model_pool_007087f0(ModelPoolLayout& pool, ModelPoolLayout*& allocator_list_head,
                                  const PoolGeometry& geometry, PoolMemory& memory);

// 00708880 / 0070AFD0 (vtable slot 0), __fastcall(ECX = pool), RET: free every completely free
// block, moving the last block into the hole and renumbering its slots; then recompute first-free.
// No lock is taken.
void release_empty_pool_blocks_00708880(ModelPoolLayout& pool, const PoolGeometry& geometry, PoolMemory& memory);

// 00708920 / 0070B070, __fastcall(ECX = pool), RET: under the section, add a block when none has a
// free slot (a failed operator new stores a null block, as the image does), pop a slot and advance
// first-free past full blocks.
std::uint8_t* allocate_pool_slot_00708920(ModelPoolLayout& pool, const PoolGeometry& geometry, PoolMemory& memory);

// 007079A0 / 0070ACB0, __thiscall(ECX = pool, slot), RET 4: under the section, push the slot index
// ((slot - block) / slot_bytes) and lower first-free (unsigned compare).
void free_pool_slot_007079a0(ModelPoolLayout& pool, std::uint8_t* slot, const PoolGeometry& geometry,
                             PoolMemory& memory);
// 00707AF0 / 0070AE00, __fastcall(ECX = slot), RET: the same on the static pool (00E19ACC fish,
// 00E19B14 flag), which the caller passes.
void free_static_pool_slot_00707af0(ModelPoolLayout& static_pool, std::uint8_t* slot, const PoolGeometry& geometry,
                                    PoolMemory& memory);

// 0070B1B0 / 0070B1E0, __fastcall(ECX = cell), RET, returns the cell: release the intrusive
// reference and clear the cell.
class ReferenceReleaser {
public:
    virtual ~ReferenceReleaser() = default;
    virtual void release_reference(void* object) = 0;
};
void** release_reference_cell_0070b1b0(void** cell, ReferenceReleaser& releaser);

// 00708A90 (string at +54h of its owner), 0070B2A0 (+8h): release one native string (unwind only).
void release_native_string_00708a90(NativeString& s, NativeStringStorage& strings);

// The fish-school parameter block (5Ch: 15h dwords and a NativeString at +54h). Field names follow
// how 007077B0 and 00709310 read the copy at model +4h; they are descriptive.
struct FishSchoolParams {
    std::int32_t count_00;          // 9C4h
    float radius_x_04;              // 25
    float radius_y_08;              // 25
    float radius_z_0c;              // 60
    float box_half_x_10;            // 50
    float box_half_y_14;            // 10
    float box_half_z_18;            // 50
    float word_1c;                  // 0
    float word_20;                  // -20
    float word_24;                  // 0
    float word_28;                  // 2
    float word_2c;                  // 2
    float word_30;                  // 1
    float word_34;                  // 1
    float word_38;                  // 4
    float word_3c;                  // 7
    float word_40;                  // 2
    float speed_min_44;             // 1
    float speed_max_48;             // 2
    float respawn_min_4c;           // 0.3
    float respawn_max_50;           // 0.75
    NativeString texture_54;        // ""
};

// 00708B00, __fastcall(ECX = params), RET, returns params: the defaults above (00CE89CC 25,
// 00CEB4B0 60, 00CEB4D4 50, 00CE38B8 10, 00CF180C -20, 00CE3958 2, 00D7A24C 1, 00CE3D34 4, 00CF5BF8
// 7, 00CE69C8 0.3, 00CEE07C 0.75) and texture "" through 0041E870 (a constructor: the previous
// string is not released).
FishSchoolParams& init_fish_school_params_00708b00(FishSchoolParams& params, NativeStringStorage& strings);

// 00708C60, __thiscall(ECX = params, const params*), RET 4: the dwords, then the self-guarded string.
FishSchoolParams& assign_fish_school_params_00708c60(FishSchoolParams& to, const FishSchoolParams& from,
                                                     NativeStringStorage& strings);

// The fish-school descriptor (7Ch, vtable 00CFD414): params at +4h, two arrays +60h/+64h, the model
// pointer +78h.
struct FishSchoolDescLayout {
    std::uint32_t vtable_00;
    FishSchoolParams params_04;
    void* positions_60;  // count * 0Ch
    void* fish_64;       // count * 28h
    std::uint8_t state_68[0x10];
    void* model_78;
};

// 00708BE0, __fastcall(ECX = desc), RET: vtable 00CFD414, default params, +60h/+64h/+78h = 0.
FishSchoolDescLayout& construct_fish_school_desc_00708be0(FishSchoolDescLayout& desc, NativeStringStorage& strings);
// 00708AB0, __fastcall(ECX = desc), RET: vtable 00CFD414; when +64h is set free +64h AND +60h (the
// +60h array alone is not freed); release the texture string.
void destruct_fish_school_desc_00708ab0(FishSchoolDescLayout& desc, PoolMemory& memory, NativeStringStorage& strings);

// Presentation draws. The image calls 00BD2F10 with ECX = 0, the SHARED gameplay stream 0; a host
// must give these their own stream (or keep the caller off) so same-binary gameplay pairs do not
// shift (cc9 RNG rule).
class PresentationRandom {
public:
    virtual ~PresentationRandom() = default;
    virtual float uniform_00bd2f10(float lo, float hi) = 0;
};
// 0042B260 (BSP_Geometry_NormalizeVectorWithFloor), __thiscall(ECX = vec3), as a host call.
class VectorNormalizer {
public:
    virtual ~VectorNormalizer() = default;
    virtual void normalize_0042b260(float* v) = 0;
};

// 007077B0, __fastcall(ECX = school), RET (tail JMP 0042B260): respawn a run of fish.
//   n = ftol(U([+50h], [+54h]) * (count - 1)); start = ftol(U(1, count - n));
//   rotate positions[start .. start+n) left by one (memmove of n-1 entries, 0Ch stride at +60h);
//   +68h = U([+48h], [+4Ch]);
//   fish 0 (+64h) inside the half-extent box (+14h/+18h/+1Ch, both bounds inclusive): direction
//   (+6Ch) = (U(-1, 1), 0, U(-1, 1)); outside: direction = -fish0 (SSE -0.0 minus); normalize.
// The x87 products run at the main thread's 24-bit precision; float arithmetic models them.
void respawn_fish_007077b0(FishSchoolDescLayout& school, PresentationRandom& random, VectorNormalizer& normalizer);

// 00709310, __thiscall(ECX = school, const params*), RET 4: copy the params (00708C60); allocate
// count * 28h fish (+64h) and count * 0Ch positions (+60h) with the overflow-saturating size;
// direction (+6Ch) = (0, 0, 1); per fish i, t = float(i / count * pi):
//   x = (rx * sin t) * U(-0.5, 0.5), y = (ry * sin t) * U(-0.5, 0.5), z = ((cos t - 1) * rz) * 0.5,
//   velocity +0Ch = (0, 0, 1), its copy at +18h, the per-fish value +24h = params +38h;
//   fish 0's +24h is then (params +3Ch + params +38h) * 0.5 (rewritten every iteration and once more).
// Then a model slot from the static fish pool, named "FishSchool" (00B75030, vtable 00CFD3A8, +184h
// = 0) into +78h (null when the pool returned null), BuildGeometry (00708D20) and the model's vtable
// +30h(school + 20h). Draws go through PresentationRandom (the image: stream 0).
class FishSchoolInitCalls {
public:
    virtual ~FishSchoolInitCalls() = default;
    virtual void* new_saturating_00bf55be(std::uint32_t count, std::uint32_t element_size) = 0;
    virtual void* allocate_fish_model() = 0;  // 00708920 on 00E19ACC + 00B75030("FishSchool") + vtable
    virtual void build_geometry_00708d20() = 0;
    virtual void model_vslot30(void* model, std::uint8_t* school_plus_20) = 0;
};
void init_fish_school_00709310(FishSchoolDescLayout& school, const FishSchoolParams& params, PresentationRandom& random,
                               FishSchoolInitCalls& calls, NativeStringStorage& strings);

// 00709620, __fastcall(ECX = kind, EDX = const float[3]), RET: default params, the three floats into
// params +1Ch..+24h, texture "Fish_0<kind>.tga" for kinds 1..5 (0041E350; other kinds keep ""), a
// new 7Ch school (vtable 00CFD414, default params, +60h/+64h/+78h = 0), Init with the params, then
// release the params texture. The school pointer is not kept by this routine. A failed operator new
// reaches Init with a null school in the image; this stops.
class FishSchoolCreateCalls : public FishSchoolInitCalls {
public:
    virtual FishSchoolDescLayout* new_school_00bf681b() = 0;  // raw 7Ch storage or null
};
FishSchoolDescLayout* create_fish_school_00709620(std::int32_t kind, const float position[3], PresentationRandom& random,
                                                  FishSchoolCreateCalls& calls, NativeStringStorage& strings);

// The flag cloth (the object 0070AA70 steps; its own routines take it in ECX). Float vectors are
// three floats; the arrays hold `count` (+4h) points in rows (+40h) of `columns` (+3Ch).
struct FlagClothLayout {
    std::uint32_t word_00;
    std::int32_t count_04;
    float* positions_08;
    float* positions_tmp_0c;
    float* velocities_10;
    float* velocities_tmp_14;
    float* forces_18;
    float* normals_1c;
    float wind_20[3];
    float angle_2c;
    std::uint8_t pad_30[0x0C];
    std::int32_t columns_3c;
    std::int32_t rows_40;
    std::uint8_t pad_44[0x08];
    float stiffness_4c;
    float damping_50;
    float velocity_drag_54;
    float gravity_58;
    float rest_horizontal_5c;
    float rest_vertical_60;
    float uv_u0_64;
    float uv_v0_68;
    float uv_u1_6c;
    float uv_v1_70;
    float time_74;
    void* vertex_stream_78;
};
static_assert(offsetof(FlagClothLayout, wind_20) == 0x20 || sizeof(void*) != 4);
static_assert(offsetof(FlagClothLayout, columns_3c) == 0x3C || sizeof(void*) != 4);
static_assert(offsetof(FlagClothLayout, time_74) == 0x74 || sizeof(void*) != 4);

// 00709EF0, __thiscall(ECX = cloth, positions, velocities), RET 8. Per point: force = (wind . n /
// n . n) n with n the point normal (+1Ch), force.y += gravity (+58h), force += drag (+54h) *
// velocity. Then the structural springs, horizontal (rest +5Ch) then vertical (rest +60h): for the
// pair (a, b), d = pb - pa, len = sqrt(|d|^2) or 0 when |d|^2 <= 00CE3820 (the division by 0 is the
// image's), u = d / len, fs = (len - rest) * stiffness (+4Ch), fd = ((vb - va) . u) * damping
// (+50h), F = fd u + u fs; force[a] += F, force[b] -= F.
void apply_flag_wind_00709ef0(FlagClothLayout& cloth, const float* positions, const float* velocities);
// 0070A450, __thiscall(ECX = cloth, outA, outB, inA, inB, vel, float dt), RET 18h. Per point i:
// outA = inA + vel * dt; w = (float(i % columns) / float(columns - 1)) * dt; outB = inB + w * force.
void integrate_flag_cloth_0070a450(const FlagClothLayout& cloth, float* out_a, float* out_b, const float* in_a,
                                   const float* in_b, const float* vel, float dt);
// 0070A5A0, __fastcall(ECX = cloth), RET: per row, columns 1..: d = normalize(p[c] - p[c-1])
// (0042B260), p[c] = p[c-1] + rest_h * d (stored twice by the image).
void relax_flag_grid_0070a5a0(FlagClothLayout& cloth, VectorNormalizer& normalizer);
// 0070AA70, __thiscall(ECX = cloth, float dt), RET 4: +74h and the global 00E19B10 = game +648h
// (passed in); dt = min(dt, 0.5); steps = 1 - ftol(dt / -0.025); sub = dt / steps; wind = normalize(
// U(-0.01, 0.01), sin(+2Ch), -0.0 - cos(+2Ch)) * 100 (the draw: PresentationRandom, image stream 0);
// then per step a midpoint pass: normals (00709CF0), wind on (+8h, +10h), integrate into the
// temporaries with sub * 0.5; normals, wind on (+0Ch, +14h), integrate in place with the temporary
// velocities and sub.
class FlagClothCalls : public VectorNormalizer {
public:
    virtual void calculate_normals_00709cf0(FlagClothLayout& cloth) = 0;
};
void update_flag_wave_0070aa70(FlagClothLayout& cloth, float dt, float game_time_648, float& published_e19b10,
                               PresentationRandom& random, FlagClothCalls& calls);

// The vertex stream object the cloth writes (+78h): data +8h, stride +0Ch, element offsets position
// +10h, normal +1Ch, uv +28h, colour +34h (a negative colour offset means four float channels at
// +38h/+3Ch/+40h/+44h), vtable +10h lock(count, 0, 0) and +14h unlock.
struct VertexStreamView {
    std::uint8_t* data_08;
    std::int32_t stride_0c;
    std::int32_t position_10;
    std::int32_t normal_1c;
    std::int32_t uv_28;
    std::int32_t colour_34;
    std::int32_t channel_38, channel_3c, channel_40, channel_44;
};
class VertexStreamCalls {
public:
    virtual ~VertexStreamCalls() = default;
    virtual void render_lock() = 0;    // [00F8C274]+30h tracked critical section, when present
    virtual void render_unlock() = 0;
    virtual VertexStreamView lock_vslot10(void* stream, std::int32_t count) = 0;
    virtual void unlock_vslot14(void* stream) = 0;
};
// 0070A710, __fastcall(ECX = cloth), RET: lock 2 * count vertices under the render lock; vertex i is
// the front face (position, normal * -0.5), vertex count + i the back face (normal * 0.5); both get
// uv = (col / (columns - 1)) * (u1 - u0) + u0, (row / (rows - 1)) * (v1 - v0) + v0 and white; unlock
// under the render lock.
void write_flag_vertices_0070a710(FlagClothLayout& cloth, VertexStreamCalls& calls);

// 0070B9D0, __thiscall(ECX = cloth, mesh, const NativeString* texture), RET 8: bind the vertex (+78h)
// and index (+7Ch) streams to the mesh (00B73BB0 slot 0, 00B73B70); effect "flag.mshd" through
// [00F8D394] vtable +48h; material 110h (00B18780 + 00B18900) over it; the effect reference dropped;
// a non-empty texture loads through [00F8D394] vtable +64h(name, 0) into material slot 0 (00B189F0);
// a 60h draw section (00B85EE0 + 00B857F0) with that material (00B864C0), primitive 4, start 0, base 0,
// vertex count 2 * count, index count +80h; 00B73C10, 00B865A0(section, mesh), 00B73C60(mesh,
// section); the section reference dropped. Null allocations reach the image's null paths; this
// passes the nulls on.
class FlagMaterialCalls {
public:
    virtual ~FlagMaterialCalls() = default;
    virtual void set_vertex_stream_00b73bb0(void* mesh, std::int32_t slot, void* stream) = 0;
    virtual void set_index_stream_00b73b70(void* mesh, void* stream) = 0;
    virtual void* load_effect_vslot48(const std::string& name) = 0;          // returns a reference
    virtual void* new_material_00b18780_00b18900(void* effect) = 0;          // null when new failed
    virtual void* load_texture_vslot64(const NativeString& name) = 0;        // returns a reference
    virtual void set_material_texture_00b189f0(void* material, std::int32_t slot, void* texture) = 0;
    virtual std::uint32_t* new_draw_section_00b85ee0_00b857f0() = 0;         // null when new failed
    virtual void set_section_material_00b864c0(void* section, void* material) = 0;
    virtual void clear_section_owners_00b73c10(void* mesh) = 0;
    virtual void rebuild_vertex_layout_00b865a0(void* section, void* mesh) = 0;
    virtual void append_section_00b73c60(void* mesh, void* section) = 0;
    virtual void release_reference(void* object) = 0;
};
void build_flag_material_0070b9d0(FlagClothLayout& cloth, std::uint32_t index_count_80, void* index_stream_7c,
                                  void* mesh, const NativeString& texture, FlagMaterialCalls& calls);

// 0070AC00, flag model vtable 00CFD488 slot +20h, __thiscall(ECX = model, context, float lod, float
// alpha, mask), RET 10h. With a parent (+30h): copy its mask (+48h) to +48h and to every child
// (first +34h, next +3Ch) through 007099C0. A lod above 0.5 draws nothing; otherwise the base render
// 00B748E0 runs with the original lod and alpha * (0.5 - clamp(lod, 0.4, 0.5)) / 0.1.
class FlagRenderCalls {
public:
    virtual ~FlagRenderCalls() = default;
    virtual void set_hierarchy_mask_007099c0(void* child, std::uint32_t mask) = 0;
    virtual void render_generated_model_00b748e0(void* model, void* context, float lod, float alpha,
                                                 std::uint32_t mask) = 0;
};
void render_flag_model_0070ac00(std::uint8_t* model, void* context, float lod, float alpha, std::uint32_t mask,
                                FlagRenderCalls& calls);

// 00708D20, __thiscall(ECX = model, school), RET 4: model +184h = school; a BCh mesh (00B73B60 +
// 00B73D70); 00B75170(model, 0, mesh, -1, -1); the mesh bounds sphere (0, 0, 0, sqrt(hy^2 + hx^2 +
// hz^2)) from the school half extents (00B72650); the vertex declaration "simplecolor.mvfm"
// ([00F8D394] +38h); count*4 vertices (+5Ch(n, 1, decl)) and count*6 16-bit indices (+60h(n, 1,
// 65h)) bound to the mesh; quads (b, b+1, b+2, b, b+2, b+3); per quad uv (0,1) (1,1) (1,0) (0,0) and
// white; the "fish.mshd" effect, a 110h material with the school texture (school +58h) in slot 0;
// a 60h draw section (primitive 4, vertex count count*4, primitive count count*2) with the bounds
// (00B855B0), 00B865A0, 00B73C60; references dropped; 00B6D890(model, [game+19ECh]).
class FishGeometryCalls : public FlagMaterialCalls, public VertexStreamCalls {
public:
    virtual void* new_mesh_00b73b60_00b73d70() = 0;
    virtual void set_model_geometry_00b75170(std::uint8_t* model, void* mesh, float a, float b) = 0;
    virtual void set_mesh_bounds_00b72650(void* mesh, const float sphere[4]) = 0;
    virtual void* vertex_declaration_vslot38(const std::string& name) = 0;
    virtual void* create_vertex_stream_vslot5c(std::int32_t count, std::int32_t flags, void* declaration) = 0;
    virtual void* create_index_stream_vslot60(std::int32_t count, std::int32_t flags, std::int32_t format) = 0;
    virtual std::uint16_t* lock_indices_vslot0c(void* stream) = 0;
    virtual void unlock_indices_vslot10(void* stream) = 0;
    virtual VertexStreamView lock_vertices_vslot10(void* stream) = 0;
    virtual void set_section_bounds_00b855b0(void* section, const float sphere[4]) = 0;
    virtual void propagate_root_registration_00b6d890(std::uint8_t* model, void* root_19ec) = 0;
};
void build_fish_geometry_00708d20(std::uint8_t* model, FishSchoolDescLayout& school, void* root_19ec,
                                  FishGeometryCalls& calls);

// The fish-school model (pool slot, vtable 00CFD3A8; owned object at +184h).
class GeneratedModelCalls {
public:
    virtual ~GeneratedModelCalls() = default;
    virtual void destroy_model_base(void* model) = 0;  // BSP_Model_Destroy
    virtual void delete_owned_object(void* object) = 0;  // the object's slot 0 with 1
};
// 00707570, __fastcall(ECX = model), RET: vtable 00CFD3A8, delete +184h through its slot 0 (1),
// then the model base destructor.
void destruct_fish_school_model_00707570(std::uint8_t* model, GeneratedModelCalls& calls);
// 00708A60, vtable 00CFD3A8 slot 1, __thiscall(ECX = model, flags), RET 4: destruct, then (bit 0)
// return the slot to the static fish pool.
std::uint8_t* scalar_delete_fish_school_model_00708a60(std::uint8_t* model, std::uint32_t flags,
                                                       GeneratedModelCalls& calls, ModelPoolLayout& static_pool,
                                                       PoolMemory& memory);
// 0070B2C0, __fastcall(ECX = flag model), RET: vtable 00CFD488, release the texture name (+18Ch),
// then the model base destructor.
void destruct_flag_model_0070b2c0(std::uint8_t* model, NativeStringStorage& strings, GeneratedModelCalls& calls);

// ===================================================================================================
// Segment 66: hardware detection (the object 009927D0 builds), the BSM_HWD registry key, language.
// ===================================================================================================

// One 12-byte row of the GPU table at 00E0CFF4: device id, name, score. The table ends at the first
// row whose name is null. The image's table is data; the caller passes it.
struct GpuTableRow {
    std::uint32_t device_id;
    const char* name;
    std::uint32_t score;
};

// The fields of the detection object these routines touch (Win32 offsets).
struct HardwareInfoFields {
    std::int32_t gpu_index_28;       // -1 when the adapter's device id is not in the table
    std::uint32_t memory_mb_2c;
    std::uint32_t cpu_mhz_30;
    char cpu_name_34[0xA00];         // +34h..+A33h (sprintf "%s"; the image does not bound it)
};

// 0098C820, __fastcall(ECX = info), RET: name of row [info+28h], or row 0's name when the index is
// -1. Any other negative index reads before the table, as the image does (here: row 0).
const char* gpu_model_name_0098c820(const HardwareInfoFields& info, const GpuTableRow* table);
// 0098C870, __fastcall(ECX = info), RET: device id of row max(index, 0) (the AND mask clamps).
std::uint32_t gpu_device_id_0098c870(const HardwareInfoFields& info, const GpuTableRow* table);
// 0098C890, __fastcall(ECX = device id), RET: name of the first row with that id; row 0's name when
// the scan reaches the terminator (and row 0's when row 0 itself is the terminator).
const char* gpu_device_name_0098c890(std::uint32_t device_id, const GpuTableRow* table);

// 0098C940, __cdecl, RET: QueryPerformanceFrequency, RDTSC, spin on QueryPerformanceCounter for one
// frequency period, RDTSC again; returns the 64-bit TSC delta >> 20 truncated to 32 bits.
std::uint32_t measure_cpu_clock_0098c940();

// 0098D360, the DirectSoundEnumerateA callback (__stdcall guid, description, module, context; RET
// 10h): on the first device only, probe DirectSoundCreate(NULL); on success copy the description
// into the name buffer (00F8A0E8); always bump the counter (00F8A0E0) and return TRUE. A failed
// create returns FALSE without counting. Both words are passed in.
class DirectSoundProbe {
public:
    virtual ~DirectSoundProbe() = default;
    virtual bool create_default_device() = 0;  // DirectSoundCreate(NULL) >= 0, released at once
};
bool direct_sound_enum_first_0098d360(const char* description, std::uint32_t& device_counter,
                                      char* name_buffer, DirectSoundProbe& probe);

// 0098D400, __fastcall(ECX = LCID), RET: the message-table row: 40Ch -> 1, 407h -> 2, 410h -> 3,
// 415h -> 4, 419h -> 5, else 0.
std::int32_t language_row_for_lcid_0098d400(std::uint32_t lcid) noexcept;

// The BSM_HWD key (HKLM\SOFTWARE\Eidos\BSM_HWD). These call the Win32 registry directly.
// 0098D470, __fastcall(ECX = value name, EDX = type, data, size), RET 8: RegCreateKeyExA (0x20006),
// RegSetValueExA; true when the set succeeded. The key is closed only after a successful create.
bool write_hardware_value_0098d470(const char* value_name, std::uint32_t type, const void* data,
                                   std::uint32_t size);
// 0098D4E0, __fastcall(ECX = value name, EDX = expected type, buffer, size*), RET 8: RegOpenKeyExA
// (0x20019), RegQueryValueExA; true only when the query succeeded AND the type matched.
bool read_hardware_value_0098d4e0(const char* value_name, std::uint32_t expected_type, void* buffer,
                                  std::uint32_t* size);
// 0098D570, __fastcall(ECX = name, EDX = value), RET, AL = 0098D470's answer: four bytes typed
// REG_QWORD (0098D57C MOV EDX,0Bh; 0098D571 PUSH 4), so the REG_QWORD read 0098D5C0 accepts it.
bool write_hardware_dword_0098d570(const char* value_name, std::uint32_t value);
// 0098D590, __fastcall(ECX = name, EDX = text), RET: REG_SZ, strlen + 1 bytes.
bool write_hardware_string_0098d590(const char* value_name, const char* text);
// 0098D5C0, __fastcall(ECX = name, EDX = out), RET, AL = 0098D4E0's answer (EAX is untouched after
// 0098D5DD): an eight-byte REG_QWORD read; the low dword is stored EVEN WHEN THE READ FAILED (then
// it is the caller's uninitialized stack, passed here as scratch_low). 0073C3B0 sums the four
// answers (MOVZX at 0073C415 / 0073C41D / 0073C430 / 0073C448) and needs 4 (0073C44D CMP ESI,4).
bool read_hardware_qword_0098d5c0(const char* value_name, std::uint32_t& out, std::uint32_t scratch_low);
// 0098D5F0, __fastcall(ECX = name, EDX = buffer, size*), RET 4, AL = 0098D4E0's answer: REG_SZ read.
bool read_hardware_string_0098d5f0(const char* value_name, char* buffer, std::uint32_t* size);

// 0098F430, __thiscall(ECX = info, path), RET 4: fopen(path, "wb"); null returns false. The options
// text (0098E910 builds it into info+C3Ch) is built TWICE: once for strlen, once for fwrite(text,
// length, 1). fclose; true.
class OptionsTextBuilder {
public:
    virtual ~OptionsTextBuilder() = default;
    virtual const char* build_default_options_text_0098e910() = 0;
};
bool write_default_options_file_0098f430(const char* path, OptionsTextBuilder& builder);

// 00992180, __fastcall(ECX = info), RET, AL: HKLM\HARDWARE\DESCRIPTION\System\CentralProcessor\0:
// "~MHz" into an eight-byte buffer, then "ProcessorNameString" (size query, zeroed vector<char>,
// read, sprintf "%s" into +34h whatever the result). Success stores the MHz in +30h; an "AMD
// Athlon(tm) 64" name multiplies it by 1.5 (x87, unsigned fix-up, chop rounding). A zero name size
// returns false WITHOUT closing the key, as the image does.
bool query_processor_info_00992180(HardwareInfoFields& info);

// 00991EF0, __fastcall(ECX = info), RET: the Direct3D 9 survey (Direct3DCreate9(20h)).
//   adapter 0 identifier: +B34h = sprintf("%d.%d.%d.%d", HIWORD(High), LOWORD(High), HIWORD(Low),
//   LOWORD(Low)) of the driver version, +A34h = sprintf("%s", Description); +28h = the LAST GPU table
//   row whose id equals DeviceId, else -1;
//   every X8R8G8B8 (16h) mode of at least 640x480 that the list does not already hold
//   (0098E0E0) is appended as {width, height} (00991C20);
//   every multisample type 2..15 that CheckDeviceMultiSampleType(0, HAL, A8R8G8B8 (15h), fullscreen)
//   accepts is appended to the vector at +10h;
//   +24h = (PixelShaderVersion & FFFFh >= 200h) + 1, forced to 1 for the sixteen device ids
//   301h..344h listed in the .cpp (the GeForce FX family ids); then Release.
struct D3DAdapterSurvey {
    std::uint32_t driver_version_low;
    std::uint32_t driver_version_high;
    std::string description;
    std::uint32_t device_id;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> modes_x8r8g8b8;  // enumeration order
    std::vector<bool> multisample_ok;   // index = type, 2..15 used
    std::uint32_t pixel_shader_version;
};
class DisplayModeList {
public:
    virtual ~DisplayModeList() = default;
    virtual bool contains_0098e0e0(std::uint32_t width, std::uint32_t height) = 0;
    virtual void append_00991c20(std::uint32_t width, std::uint32_t height) = 0;
    virtual void append_multisample_type(std::int32_t type) = 0;  // vector at +10h
};
struct DisplayDriverFields {
    std::int32_t* shader_model_24;
    std::int32_t* gpu_index_28;
    char* description_a34;   // 100h
    char* driver_b34;        // 100h
};
void query_display_driver_00991ef0(const D3DAdapterSurvey& survey, const GpuTableRow* table, DisplayDriverFields fields,
                                   DisplayModeList& lists);

// 00992640, __fastcall(ECX = info), RET: OS description (0098C9C0), GlobalMemoryStatus (MB at +2Ch),
// processor info or the measured clock, the display driver query (00991EF0), then the sound probe:
// counter and name cleared, DirectSoundEnumerateA(0098D360, info).
class HardwareCollectCalls {
public:
    virtual ~HardwareCollectCalls() = default;
    virtual void describe_os_version_0098c9c0() = 0;
    virtual std::uint32_t total_physical_memory() = 0;  // GlobalMemoryStatus dwTotalPhys
    virtual bool query_processor_info_00992180() = 0;
    virtual std::uint32_t measure_cpu_clock_0098c940() = 0;
    virtual void query_display_driver_00991ef0() = 0;
    virtual void reset_sound_probe(std::uint32_t& counter, char& first_name_byte) = 0;
    virtual void enumerate_sound_devices() = 0;  // DirectSoundEnumerateA(0098D360, info)
};
void collect_hardware_info_00992640(HardwareInfoFields& info, HardwareCollectCalls& calls);

// 009927D0, __thiscall(ECX = info, owner), RET 4: zero the two vector headers (+4h..+0Ch,
// +14h..+1Ch), store the owner at +C38h, then collect.
struct HardwareInfoVectors {
    std::uint32_t words[8];  // +0h..+1Fh as the constructor sees them
};
void construct_hardware_info_009927d0(HardwareInfoVectors& vectors, void*& owner_c38, void* owner,
                                      HardwareInfoFields& info, HardwareCollectCalls& calls);

// 00992830, __cdecl, RET: the LCID from options.txt. Reads whitespace-separated tokens; after the
// exact token "Language" the next token's first letter (tolower) selects d 407h, e 409h, f 40Ch,
// i 410h, p 415h, r 419h (others keep 409h); a missing file or key keeps 409h.
std::uint32_t read_language_from_options_00992830(const char* path = "options.txt");

// ===================================================================================================
// Segment 66: the online system base and the XLive stats records.
// ===================================================================================================

// 00992A50, __fastcall, RET: when the published system (00F8A300) is null, operator new(2218h) and
// 00A47xxx construct; both 00F8A2FC and 00F8A300 get the result (null when new failed); returns
// true only when it created one.
class OnlineSystemFactory {
public:
    virtual ~OnlineSystemFactory() = default;
    virtual void* new_and_construct_xlive_system() = 0;  // null when operator new failed
    virtual void delete_system_vslot1a4(void* system) = 0;  // vtable +1A4h(1)
};
bool create_online_system_00992a50(void*& published_f8a300, void*& published_f8a2fc, OnlineSystemFactory& factory);
// 00992AC0, __cdecl, RET: delete through vtable +1A4h(1) and clear both words; true when there was one.
bool destroy_online_system_00992ac0(void*& published_f8a300, void*& published_f8a2fc, OnlineSystemFactory& factory);

// The 50h stat record: two key dwords, a name, +10h, then 0Eh dwords from +18h.
struct XLiveStatRecord {
    std::uint32_t key_00;
    std::uint32_t key_04;
    NativeString name_08;
    std::uint32_t word_10;
    std::uint32_t word_14;
    std::uint32_t values_18[0x0E];
};
// 00992F60, __thiscall(ECX = record, const record*), RET 4: keys, self-guarded name, +10h, 0Eh dwords.
XLiveStatRecord& assign_stat_record_00992f60(XLiveStatRecord& to, const XLiveStatRecord& from, NativeStringStorage& strings);
// 009934D0: the copy constructor form (name zeroed first).
XLiveStatRecord& copy_construct_stat_record_009934d0(XLiveStatRecord& to, const XLiveStatRecord& from,
                                                     NativeStringStorage& strings);
// 00993140, __thiscall(ECX = record, key0, key1, NativeString name BY VALUE, word10), RET 14h form:
// keys, name copy, +10h, zero +18h, +20h..+2Ch, byte +30h, +34h..+48h (+14h and +1Ch untouched),
// then release the by-value name.
XLiveStatRecord& construct_stat_field_00993140(XLiveStatRecord& record, std::uint32_t key0, std::uint32_t key1,
                                               NativeString name_by_value, std::uint32_t word10,
                                               NativeStringStorage& strings);
// 00992F40 / 00993780: release the name of a record (unwind helpers).
void release_stat_record_name_00992f40(XLiveStatRecord& record, NativeStringStorage& strings);

// The stats block base (vtable 00D1D5B0) as 00994080 builds it.
struct XLiveStatsBlockLayout {
    std::uint32_t vtable_00;
    std::uint32_t word_04;
    std::uint32_t word_08;
    std::uint8_t name_0c[0x18];   // std::string: buffer +0Ch, size +1Ch, capacity +20h
    std::uint32_t word_24;
    std::uint32_t word_28;
    std::uint32_t word_2c;
    std::uint32_t word_30;
    std::uint32_t word_34;
    std::uint8_t byte_38;
    std::uint8_t pad_39[3];
    std::uint32_t word_3c;
    std::uint8_t byte_40;
    std::uint8_t byte_41;
    std::uint8_t byte_42;
    std::uint8_t byte_43;
    std::uint32_t word_44;
    std::uint32_t word_48;
    std::uint8_t byte_4c;
    std::uint8_t byte_4d;
    std::uint8_t pad_4e[2];
    std::uint32_t sessions_50;    // allocator word of the session vector
    void* sessions_first_54;
    void* sessions_last_58;
    void* sessions_end_5c;
    std::uint32_t network_mode_60;
    std::uint32_t stats_64;       // allocator word of the stat-record vector
    void* stats_first_68;
    void* stats_last_6c;
    void* stats_end_70;
    std::uint8_t stats_ready_74;
};
// 00994080, __fastcall(ECX = block), RET: the base constructor (vtable 00D1D5B0, empty std::string,
// flags, empty vectors, network mode +60h = 2).
void construct_xlive_stats_block_00994080(XLiveStatsBlockLayout& block);
// 00994100 / 00994110 / 00994120 (vtable +198h/+19Ch/+1A0h): mode == 0, mode == 1, set mode.
// (00994100 and 00994110 are also in src/network_online_cc13.cpp.)
void set_network_mode_00994120(XLiveStatsBlockLayout& block, std::uint32_t mode) noexcept;
// 009940F0, base vtable +78h: RET.
void xlive_base_nop_009940f0() noexcept;

// 00994130, base vtable +18h, __thiscall(ECX = system, OnlinePlayerInfo by value (40h)), RET 40h:
// assign the argument into this+8h (006883B0), then free the argument's std::string buffer when its
// capacity is 10h or more.
class OnlinePlayerInfoOps {
public:
    virtual ~OnlinePlayerInfoOps() = default;
    virtual void assign_006883b0(void* destination, const void* source) = 0;
    virtual void destroy_by_value_string(void* source) = 0;
};
void set_player_info_00994130(std::uint8_t* system, void* info_by_value, OnlinePlayerInfoOps& ops);

// 00994180, __fastcall(ECX = block), RET: base vtable, destroy and free the stat-record vector
// (00993990) and the session vector (session range destroy), reset the std::string.
class StatsBlockTeardown {
public:
    virtual ~StatsBlockTeardown() = default;
    virtual void destroy_stat_records_00993990(void* first, void* last) = 0;
    virtual void destroy_sessions(void* first, void* last) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void reset_std_string(void* string_at_0c) = 0;  // free when capacity > 0Fh, then empty
};
void destruct_xlive_stats_block_00994180(XLiveStatsBlockLayout& block, StatsBlockTeardown& teardown);

// The game-side inputs of 00993220 / 00994310: the eight player slots game+18CCh..+18E8h and the
// per-side score blocks [game+21A0h]+70h + slot*284h. Read-only.
struct StatsPlayerSlot {
    bool present;                 // slot pointer not null
    bool flag_09;                 // player byte +9 (set: skipped)
    std::uint32_t key_80;
    std::uint32_t key_84;
    std::string name_58;          // the char array at +58h
    std::uint32_t word_28;
};
class StatsGameView {
public:
    virtual ~StatsGameView() = default;
    virtual StatsPlayerSlot player_slot(std::size_t slot) = 0;  // slot 0..7
    virtual std::int32_t effective_game_mode() = 0;           // BSP_Game_GetEffectiveGameMode
    // Score block of the slot: the keys of its set at +4h (in tree order), +118h and +174h.
    virtual std::vector<std::string> side_badge_keys(std::size_t slot) = 0;
    virtual std::uint32_t side_word_118(std::size_t slot) = 0;
    virtual std::uint32_t side_word_174(std::size_t slot) = 0;
    // 0090C000 with ECX = [game+21A0h] and the key by value.
    virtual bool badge_earned_0090c000(const std::string& key) = 0;
};
// 00993220, __fastcall(ECX = stats block), RET: for every stat record and every player slot that is
// present, not flagged and has the record's two keys and (null-aware, case-insensitive) name: count
// the earned badges of that slot's score block and store values +48h = side+174h, +44h = side+118h,
// +2Ch = the effective game mode, +40h = the badge count. Then +74h = 1.
void fill_stats_from_game_00993220(std::vector<XLiveStatRecord>& records, std::uint8_t& ready_74,
                                   StatsGameView& game);
// 00994310, __fastcall(ECX = stats block), RET: erase every record (00993B70), then for every present
// and unflagged player slot push construct(key80, key84, name58, word28) (00993140 / 00994240);
// +74h = 0.
void reset_stats_values_00994310(std::vector<XLiveStatRecord>& records, std::uint8_t& ready_74, StatsGameView& game,
                                 NativeStringStorage& strings);

// ===================================================================================================
// Segment 66: the downloadable-content manager (base vtable 00D1D758, list 00D1D75C, manager
// 00D1D7D8).
// ===================================================================================================

// The 18h content package record: name +0h, path +8h, four flag bytes +10h..+13h, index +14h.
struct ContentPackageRecord {
    NativeString name_00;
    NativeString path_08;
    std::uint8_t flags_10[4];
    std::int32_t index_14;
};
// 009946D0, __fastcall(ECX = record), RET: release path, then name.
void destruct_content_record_009946d0(ContentPackageRecord& record, NativeStringStorage& strings);
// 00994850, __thiscall(ECX = record, const record*), RET 4: zeroed self-guarded copies, flags, index.
ContentPackageRecord& copy_construct_content_record_00994850(ContentPackageRecord& to, const ContentPackageRecord& from,
                                                             NativeStringStorage& strings);
// 00994770, __cdecl(out NativeString*, NativeString base BY VALUE), returns out: base + "\" (00CE7894),
// then the by-value base is released.
std::string content_path_compose_00994770(const std::string& base);

// {data, count, capacity} of 18h records (signed counts).
struct ContentPackageArray {
    ContentPackageRecord* data;
    std::int32_t count;
    std::int32_t capacity;
};
class ContentArrayMemory {
public:
    virtual ~ContentArrayMemory() = default;
    virtual ContentPackageRecord* allocate_records_00bf681b(std::int32_t count) = 0;
    virtual void free_00bf65ac(void* block) = 0;
};
// 00994920, __thiscall(ECX = array, n), RET 4: n < 1 becomes 1; when n > capacity allocate n, copy
// construct every record, destroy the old ones, free, capacity = n.
void reserve_content_array_00994920(ContentPackageArray& a, std::int32_t n, ContentArrayMemory& memory,
                                    NativeStringStorage& strings);
// 00994A00, __thiscall(ECX = array, const record*), RET 4: when full reserve max(2 * capacity, 1)
// (2 * capacity < 2 gives 1), then copy construct at the end.
void push_back_content_array_00994a00(ContentPackageArray& a, const ContentPackageRecord& record,
                                      ContentArrayMemory& memory, NativeStringStorage& strings);
// 00994A80, __thiscall(ECX = array, n), RET 4: reserve when n > capacity; default records (zero
// strings and flags, index -1) up to n; destroy down to n; count = n.
void resize_content_array_00994a80(ContentPackageArray& a, std::int32_t n, ContentArrayMemory& memory,
                                   NativeStringStorage& strings);
// 00994DA0, __fastcall(ECX = array), RET: resize(0) then free the data (unwind helper).
void tidy_content_array_00994da0(ContentPackageArray& a, ContentArrayMemory& memory, NativeStringStorage& strings);
// 00994DC0, __thiscall(ECX = array, const array*), RET 4: resize(0), reserve(source count), push
// each record; returns this.
ContentPackageArray& assign_content_array_00994dc0(ContentPackageArray& a, const ContentPackageArray& source,
                                                   ContentArrayMemory& memory, NativeStringStorage& strings);

// The singleton lifetime manager as 00994540 / 009945E0 use it: an optional tracked critical
// section at manager +10h (recursion counter at +18h) and register / unregister.
class SingletonLifetimeCalls {
public:
    virtual ~SingletonLifetimeCalls() = default;
    virtual void lock_manager() = 0;     // EnterCriticalSection + counter, when the section exists
    virtual void unlock_manager() = 0;   // counter - 1 + LeaveCriticalSection, when it exists
    virtual void register_singleton(void* object) = 0;
    virtual void unregister_singleton(void* object) = 0;
};
// 00994540, __fastcall(ECX = manager), RET: vtable 00D1D758; under the lifetime lock publish the
// instance (00F8A304) and register it.
void construct_download_manager_base_00994540(std::uint32_t& vtable_00, void* self, void*& published_f8a304,
                                              SingletonLifetimeCalls& lifetime);
// 009945E0, __fastcall(ECX = manager), RET: vtable 00D1D758; under the lock unregister the published
// instance and clear it; then the singleton-base vtable 00CE3818.
void destruct_download_manager_base_009945e0(std::uint32_t& vtable_00, void*& published_f8a304,
                                             SingletonLifetimeCalls& lifetime);

// The content list (vtable 00D1D75C): base, then two record arrays at +4h and +10h.
struct ContentPackageListLayout {
    std::uint32_t vtable_00;
    ContentPackageArray active_04;
    ContentPackageArray pending_10;
};
// 00994F10, __fastcall(ECX = list), RET: base constructor, vtable 00D1D75C, zeroed arrays.
void construct_content_list_00994f10(ContentPackageListLayout& list, void*& published_f8a304,
                                     SingletonLifetimeCalls& lifetime);
// 00994E10, __fastcall(ECX = list), RET: vtable 00D1D75C, resize(0) and free +10h then +4h, base dtor.
void destruct_content_list_00994e10(ContentPackageListLayout& list, void*& published_f8a304,
                                    SingletonLifetimeCalls& lifetime, ContentArrayMemory& memory,
                                    NativeStringStorage& strings);

// 00994450, manager vtable slot 7, __fastcall(ECX = manager), RET: clear the enumeration words
// (+838h count, +83Ch cursor, +840h), then XLiveContentCreateEnumerator(40h, {1, 2, 0...}) and one
// XEnumerate of up to 800h bytes into +38h; ERROR_NO_MORE_FILES as an HRESULT (80070012h) counts
// as zero; a cursor at or past the count resets to 0; close the handle.
class XContentApi {
public:
    virtual ~XContentApi() = default;
    // Returns the HRESULT-normalised result: positive Win32 codes become 8007xxxxh.
    virtual std::int32_t create_enumerator(std::uint32_t max_items, void*& handle) = 0;
    virtual std::int32_t enumerate(void* handle, void* buffer, std::uint32_t bytes, std::uint32_t& items) = 0;
    virtual void close_handle(void* handle) = 0;
};
struct DownloadEnumerationState {
    std::uint8_t* buffer_38;     // 800h bytes of 20h XCONTENT_DATA rows
    std::uint32_t count_838;
    std::uint32_t cursor_83c;
    std::uint32_t word_840;
};
void reset_content_enumeration_00994450(DownloadEnumerationState& state, XContentApi& api);

// The manager (vtable 00D1D7D8) fields the remaining routines use: the content list base, the two
// parallel NativeString arrays of mounted directories (+20h data, +24h count) and package names
// (+2Ch data), and the enumeration state.
struct NativeStringArray {
    NativeString* data;
    std::int32_t count;
    std::int32_t capacity;
};
struct DownloadManagerFields {
    ContentPackageListLayout* list;   // +0h..+1Bh
    NativeStringArray* directories_20;
    NativeStringArray* packages_2c;
    DownloadEnumerationState* enumeration;
};
class DownloadManagerCalls : public XContentApi {
public:
    virtual NativeStringStorage& strings() = 0;
    // 00A3E510 with ECX = [00F8ABE8]: a Xenon user is selected.
    virtual bool has_selected_user_00a3e510() = 0;
    // XLiveContentGetDisplayName(0, data, buffer, size*): the HRESULT. A null buffer queries the size.
    virtual std::int32_t content_display_name(const std::uint8_t* content_data, wchar_t* buffer,
                                              std::uint32_t& size) = 0;
    // 00436630: a NativeString from the low bytes of a wide string.
    virtual std::string narrow_low_bytes_00436630(const wchar_t* text) = 0;
    // 0043E9A0, __thiscall(ECX = string, other*, 0): the match at offset 0.
    virtual bool matches_at_offset_0043e9a0(const NativeString& string, const NativeString& other) = 0;
    // 004CDC20 with ECX = 00F88A24: append a new content name to that global string vector.
    virtual void append_new_content_name_004cdc20(const NativeString& name) = 0;
    // 004254B0 logging; 00BE0750 / 00BDD020 on the file store [0109CEEC].
    virtual void log_004254b0(const char* format, const char* a, const char* b) = 0;
    virtual void unmount_00be0750(const std::string& path, const std::string& prefix) = 0;
    virtual void remove_content_package_00bdd020(const NativeString& path) = 0;
    // 00427110 resize(0) of a NativeString array, then 00BF6989 free of its data.
    virtual void destroy_string_array(NativeStringArray& array) = 0;
    virtual void destruct_content_list_00994e10() = 0;
    // The std::list<NativeString> append 009954E0 performs (node copy + count + link at the end).
    virtual void list_append(void* list, const NativeString& value) = 0;
};

// 00994B10, manager vtable slot 6, __fastcall(ECX = manager), RET, AL: without a selected user
// returns its (false) result. Otherwise re-enumerates (the 00994450 body inline) and, for each
// XCONTENT_DATA row (20h stride from +38h) whose display name fits 100h wide characters, appends
// the name to the new-content vector (004CDC20) when no active record matches it; returns true when
// at least one name was appended.
bool enumerate_new_content_00994b10(DownloadManagerFields& manager, DownloadManagerCalls& calls,
                                    ContentArrayMemory& memory);

// 00994F40, manager vtable slot 9, __fastcall(ECX = manager), RET: with a selected user, pending =
// active (00994DC0), active.resize(0); each row whose display name resolves is pushed to active as
// the matching pending record (by name) or as the fresh record (index -1); pending.resize(0).
void refresh_content_licenses_00994f40(DownloadManagerFields& manager, DownloadManagerCalls& calls,
                                       ContentArrayMemory& memory);

// 009954E0, manager vtable slot 10, __thiscall(ECX = manager, list*), RET 4: for every directory
// entry i with a non-empty name whose active record i has flag byte +11h set, append that record's
// path (+8h) to the list. (The ledger name BSP_DownloadManager_UnmountPackage is kept; the body only
// collects.)
void collect_mounted_content_paths_009954e0(DownloadManagerFields& manager, void* list, DownloadManagerCalls& calls);

// 00995150, __fastcall(ECX = manager), RET: vtable 00D1D7D8; for each mounted directory log
// "Unmount active content (dir: %s)(mpkg: %s)" and unmount dir + "\" and (when set) the package
// with the prefix "."; for each active record with flag +11h log "Remove active content package
// (%s)" (00F8A308 for a null path) and remove its path; destroy both string arrays; list destructor.
void destruct_download_manager_00995150(std::uint32_t& vtable_00, DownloadManagerFields& manager,
                                        DownloadManagerCalls& calls);

// ===================================================================================================
// Segment 11: scene-database presentation entities (camera paths, nav points, movie cameras, the
// simple and periodic effects) and small helpers. Gameplay entities (LandingPoint, SpawnPoint,
// LandConvoy, Stationary, Wreck, WaterMine, the unit creators) are left to cc9; see the doc.
// ===================================================================================================

// Routines the entity code calls. Entity offsets are the native ones (Win32): vtables at +0h, +10h,
// +24h and +170h, the class id at +C4h, the world at +30h, the name string at +154h.
class SceneEntityCalls {
public:
    virtual ~SceneEntityCalls() = default;
    virtual NativeStringStorage& strings() = 0;
    // 00BF55BE operator new + 00BF79F0 memset 0 of the same size; null when new failed.
    virtual std::uint8_t* new_zeroed(std::size_t bytes) = 0;
    // 006AF3D0 with ECX = [[00E188A8]+21D8h]: the 16-bit network id for a name (the by-value
    // NativeString the caller builds with 0041E870 is consumed).
    virtual std::uint16_t network_id_from_name_006af3d0(const char* name) = 0;
    // 00928630, __thiscall(entity, 0, id): BSP_GameEntity_Construct.
    virtual void construct_game_entity_00928630(std::uint8_t* entity, std::uint32_t id) = 0;
    // 00875890, __thiscall(entity + 1E4h, entity, 0): the tick registration sub-object.
    virtual void construct_tick_registration_00875890(std::uint8_t* tick, std::uint8_t* entity) = 0;
    // 004E5850, __thiscall(entity, id): BSP_CameraPath_Construct, returns the entity.
    virtual std::uint8_t* construct_camera_path_004e5850(std::uint8_t* entity, std::uint32_t id) = 0;
    // Entity vtable +98h(parent, [[00E188A8]+19CCh], frame*): place in the world.
    virtual void place_vslot98(std::uint8_t* entity, void* parent, void* frame) = 0;
    // 004C1130 (frame job pool) +4h, vtable +10h: true while jobs defer hierarchy writes.
    virtual bool frame_jobs_deferring() = 0;
    // 004F03C0, __fastcall(ECX = properties, EDX = parent, frame): defer the hierarchy write.
    virtual void defer_hierarchy_004f03c0(void* properties, void* parent, void* frame) = 0;
    // 00928560 / 00928570: base register / unregister; 00484540 push_back and the erase of
    // [world+390h] (the camera-path list, first node at +394h).
    virtual void register_base_00928560(std::uint8_t* entity) = 0;
    virtual void unregister_base_00928570(std::uint8_t* entity) = 0;
    virtual void camera_path_list_push_back_00484540(std::uint8_t* world, std::uint8_t* entity) = 0;
    virtual void camera_path_list_erase(std::uint8_t* world, void* node) = 0;
    // References (+4h counts, slot 0 at zero).
    virtual void release_reference(void* object) = 0;
    // 00875490 (tick element destructor, ECX = entity + 1E4h or null) and 009287B0 (entity).
    virtual void destroy_tick_element_00875490(std::uint8_t* tick) = 0;
    virtual void destroy_game_entity_009287b0(std::uint8_t* entity) = 0;
};

// The name tail every creator shares (004E9A82..): resize entity+154h to strlen(name) WITHOUT
// preserve (0041DD40 with 0), then copy exactly the length bytes when the buffer is not null.
void store_entity_name(std::uint8_t* entity, const char* name, NativeStringStorage& strings);

// 004E99B0 NavPoint (class 41h, vtables 00CE8550/8534/852C/8528), 004E9AE0 MovieCamPos (42h,
// 00CE86D8/86C0/86B8/86B4) and 004E9C10 MovieCamLookat (43h, 00CE8860/8848/8840/883C):
// __fastcall(ECX unused, EDX = name, parent, frame*, two unread words), RET 10h. new(1E4h) zeroed;
// network id from the name; GameEntity construct; vtables and class id; place (vtable +98h); name.
// A failed operator new leaves a null entity that the image then dereferences; this stops instead.
struct FixedEntitySpec {
    std::uint32_t size;
    std::uint32_t class_id;
    std::uint32_t vtable_00, vtable_10, vtable_24, vtable_170;
};
inline constexpr FixedEntitySpec kNavPointSpec{0x1E4, 0x41, 0x00ce8550, 0x00ce8534, 0x00ce852c, 0x00ce8528};
inline constexpr FixedEntitySpec kMovieCamPosSpec{0x1E4, 0x42, 0x00ce86d8, 0x00ce86c0, 0x00ce86b8, 0x00ce86b4};
inline constexpr FixedEntitySpec kMovieCamLookatSpec{0x1E4, 0x43, 0x00ce8860, 0x00ce8848, 0x00ce8840, 0x00ce883c};
std::uint8_t* create_named_scene_entity_004e99b0(const FixedEntitySpec& spec, const char* name, void* parent,
                                                 void* frame, SceneEntityCalls& calls);

// 004EA760, CameraPath (class 4Ah, 210h): as above with 004E5850 as the constructor; the 40h-byte
// frame is copied to a local and the copy's address is passed to vtable +98h.
std::uint8_t* create_camera_path_004ea760(const char* name, void* parent, const void* frame,
                                          SceneEntityCalls& calls);

// 004E7EF0, __thiscall(ECX = entity, id), RET 4: SimpleEffect (class 5Bh): GameEntity construct,
// vtables 00CE8BD0/8BB4/8BAC/8BA8, +1E4h/+1E8h = 0.
std::uint8_t* construct_simple_effect_004e7ef0(std::uint8_t* entity, std::uint32_t id, SceneEntityCalls& calls);
// 004E7F90, __thiscall(ECX = entity, id), RET 4: PeriodicEffect (class 5Ch): GameEntity construct,
// tick registration at +1E4h, vtables 00CE8D68/8D4C/8D44/8D40, +1E4h vtable 00CE8D28, +220h/+224h 0.
std::uint8_t* construct_periodic_effect_004e7f90(std::uint8_t* entity, std::uint32_t id, SceneEntityCalls& calls);
// 004E8060, __fastcall(ECX = entity), RET: release +224h then +220h (zeroing both), the tick element
// destructor (entity + 1E4h), the GameEntity destructor.
void destruct_periodic_effect_004e8060(std::uint8_t* entity, SceneEntityCalls& calls);
// 004F0DB0 (SimpleEffect, 1ECh) and 004F0EB0 (PeriodicEffect, 228h): __fastcall(ECX unused, EDX =
// name, parent, frame*, properties), RET 10h. new zeroed; construct (id 0); place through vtable
// +98h unless the frame jobs defer, in which case 004F03C0(properties, parent, frame); name.
std::uint8_t* create_simple_effect_004f0db0(const char* name, void* parent, void* frame, void* properties,
                                            SceneEntityCalls& calls);
std::uint8_t* create_periodic_effect_004f0eb0(const char* name, void* parent, void* frame, void* properties,
                                              SceneEntityCalls& calls);

// 004E7E90 / 004E7EB0, CameraPath vtable +130h / +134h, __fastcall(ECX = entity), RET: base register
// then push into [world+390h]; base unregister then erase the first node whose +8h is the entity
// (walking +4h links from [world+394h]).
void register_camera_path_004e7e90(std::uint8_t* entity, SceneEntityCalls& calls);
void unregister_camera_path_004e7eb0(std::uint8_t* entity, SceneEntityCalls& calls);

// 004E7DD0, __fastcall(ECX = array), RET: vtable 00CE8BA4, free +0Ch (00BF6989) when set and zero
// it, zero +8h and +4h. 004E7E20, __fastcall(ECX = cell), RET: free *cell (unconditionally) and zero.
class FreeCalls {
public:
    virtual ~FreeCalls() = default;
    virtual void free_00bf6989(void* block) = 0;
};
void destruct_simple_array_004e7dd0(std::uint8_t* array, FreeCalls& calls);
void free_owned_buffer_004e7e20(void** cell, FreeCalls& calls);

// 004E95F0 / 004F16E0 / 004F2500: MOV EAX, imm32; RET: the class-name literals "LandingPoint",
// "SpawnPoint" and "LandConvoy" (vtable slots, no static callers).
const char* landing_point_class_name_004e95f0() noexcept;
const char* spawn_point_class_name_004f16e0() noexcept;
const char* land_convoy_class_name_004f2500() noexcept;

// 004F2590 and its duplicate COMDAT 004F25B0, __thiscall(ECX = unit), RET, AL: 008DDF90 with ECX =
// [game + 21A4h + [game + 18ECh] * 4] (the local player's SzurkeNyil set) and the unit. A query.
class LocalObjectiveSet {
public:
    virtual ~LocalObjectiveSet() = default;
    virtual bool contains_unit_008ddf90(const void* unit) = 0;
};
bool unit_is_local_player_objective_004f2590(const void* unit, LocalObjectiveSet& local_player_set);

// The scene class map at scene database +34h: +4h count, +8h 40h bucket heads. Node 10h (from the
// 14h-byte node pool 00E18514): key NativeString +0h, value +8h, next +0Ch.
struct SceneClassMapNode {
    NativeString key;
    void* value;
    SceneClassMapNode* next;
};
struct SceneClassMapLayout {
    std::uint32_t word_00;
    std::uint32_t count_04;
    SceneClassMapNode* buckets_08[0x40];
};
class SceneClassMapCalls {
public:
    virtual ~SceneClassMapCalls() = default;
    virtual NativeStringStorage& strings() = 0;
    // 00468FB0, __thiscall(map, key*, out bucket*): the node for the key (case-insensitive), or null;
    // the bucket index is written either way.
    virtual SceneClassMapNode* find_node_00468fb0(const NativeString& key, std::uint32_t& bucket) = 0;
    // 004E7C00 on the pool 00E18514: raw node storage, or null.
    virtual void* allocate_node_004e7c00() = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void* new_descriptor_00bf55be() = 0;  // operator new(0Ch), may return null
};
// 004EE1A0, __thiscall(ECX = map, const NativeString* key, void* value), RET 8, returns the count: a
// hit frees the old value (when set) and stores the new one; a miss links a new node at the head
// of its bucket and bumps the count. A null pool node is dereferenced by the image; this stops.
std::uint32_t scene_class_map_insert_004ee1a0(SceneClassMapLayout& map, const NativeString& key, void* value,
                                              SceneClassMapCalls& calls);
// 004EE250, __thiscall(ECX = map, const char* name, int class id, create, register), RET 10h:
// operator new(0Ch) descriptor {id, create, register} (null when new failed), the name as a
// NativeString (0041E870), insert, release the name.
struct SceneClassDescriptor {
    std::int32_t class_id;
    std::uint32_t create;
    std::uint32_t register_pass;
};
void scene_class_map_register_004ee250(SceneClassMapLayout& map, const char* name, std::int32_t class_id,
                                       std::uint32_t create, std::uint32_t register_pass, SceneClassMapCalls& calls);

// 004F3030, __thiscall(ECX = segment {x0, y0, x1, y1}, out[2], float t), RET 8: each product is
// stored to a float first: out = (x0 (1 - t) + x1 t, y1 t + (1 - t) y0).
void segment_lerp_004f3030(const float segment[4], float out[2], float t) noexcept;
// 004F40C0, __stdcall(x1, y1, x2, y2, x3, y3), RET 18h, EAX: (y3 - y1)(x2 - x1) - (y2 - y1)(x3 - x1) > 0,
// with the four differences stored to floats first.
bool is_left_turn_004f40c0(float x1, float y1, float x2, float y2, float x3, float y3) noexcept;

// 004F1740: MOV EAX, 6; RET (the base sensor category). 004F17F0: RET 8 (a two-argument no-op).
std::int32_t sensor_category_unclassified_004f1740() noexcept;
void stub_ret8_004f17f0(std::uint32_t, std::uint32_t) noexcept;

// ===================================================================================================
// Segment 11: front-end screens.
// ===================================================================================================

// The screen object as the base virtuals see it: vtable +0h; +4h wanted byte, +5h active byte.
class FrontEndScreenObject {
public:
    virtual ~FrontEndScreenObject() = default;
    virtual void update_20(float seconds) = 0;  // vtable +20h
};
// 004F71F0, __thiscall(ECX = screen, float), RET 4: tail call of the screen's vtable +20h.
void update_front_end_screen_004f71f0(FrontEndScreenObject& screen, float seconds);
// 004F75A0 (+18h enter, RET), 004F75B0 (+1Ch exit, RET), 004F75C0 (+20h update, RET 4), 004F75D0
// (+24h collect children, RET 4): the empty base virtuals of vtable 00CEAE54.
void base_screen_enter_004f75a0() noexcept;
void base_screen_exit_004f75b0() noexcept;
void base_screen_update_004f75c0(float) noexcept;
void base_screen_collect_children_004f75d0(void*) noexcept;
// 004F8940: RET, the movie-player profile slot +18h.
void movie_player_vslot18_noop_004f8940() noexcept;

// 004F7230, __fastcall(ECX = vector header), RET, AL: no storage, or (last - first) >> 2 == 0.
bool pointer_vector_is_empty_004f7230(const NativeVectorHeader& vector) noexcept;

// 004F8390, __fastcall(ECX = level, EDX = saved set), RET (tail JMP 004F7620): byte 00E18CDC = 1,
// assign the saved set into the level vector at 00E18CE8 + level * 10h (004F81D0), then recompute
// the requests. The byte is passed in.
class ScreenSetCalls {
public:
    virtual ~ScreenSetCalls() = default;
    virtual void assign_level_vector_004f81d0(std::int32_t level, const void* saved_set) = 0;
    virtual void recompute_requests_004f7620() = 0;
};
void restore_level_set_004f8390(std::int32_t level, const void* saved_set, std::uint8_t& recompute_byte_e18cdc,
                                ScreenSetCalls& calls);

// 004F8B60, __thiscall(ECX = player, const NativeString* name), RET 4: name equals +FCh
// case-insensitively (00435C40) AND the movie is playing (BSP_GuiMovie_IsPlaying).
class MoviePlayerCalls {
public:
    virtual ~MoviePlayerCalls() = default;
    virtual bool equals_insensitive_00435c40(const void* player_name_fc, const std::string& name) = 0;
    virtual bool movie_is_playing() = 0;
};
bool movie_player_is_playing_named_004f8b60(const void* player_name_fc, const std::string& name,
                                            MoviePlayerCalls& calls);

// 004F8FA0, __fastcall(ECX = screen), RET: zero the 28h order dwords +138h..+1D4h.
void reset_tactical_library_order_004f8fa0(std::uint8_t* screen) noexcept;
// 004F90A0, __fastcall(ECX = flag), RET: +0h = +4h = 0, +8h = the event handle (BSP_Event_Create).
struct SignalledFlag {
    std::uint32_t word_00;
    std::uint32_t word_04;
    void* event_08;
};
SignalledFlag& construct_signalled_flag_004f90a0(SignalledFlag& flag, void* (*create_event)());
// 004F90D0, __fastcall(ECX = out0, EDX = out1, out2), RET 4: the edge-or-repeat state of front-end
// actions 46h, 47h and 4Ah.
void read_navigation_actions_004f90d0(std::uint8_t& out0, std::uint8_t& out1, std::uint8_t& out2,
                                      bool (*action_edge_or_repeat)(std::int32_t));
// 004F91A0, listener slot 6 of 00CEB97C, __thiscall(ECX = screen, widget*, bool focused), RET 8.
class GuiWidget {
public:
    virtual ~GuiWidget() = default;
    virtual std::int32_t kind_5c() = 0;                         // vtable +5Ch
    virtual void set_alpha_4c(float alpha) = 0;                 // vtable +4Ch
    virtual std::int16_t state_94() = 0;                        // vtable +94h
    virtual void set_highlight_84(bool on) = 0;                 // vtable +84h
    virtual void set_focus_88(bool on, std::int32_t, float) = 0;  // vtable +88h
};
void tactical_library_on_focus_004f91a0(GuiWidget* const* rows_a_170, GuiWidget* const* rows_b_210,
                                        GuiWidget* arrow_50c, GuiWidget* arrow_510, GuiWidget& widget,
                                        bool focused);
// 004F93A0, __fastcall(ECX = singleton), RET: clear the published word 00E18D4C, singleton-base
// vtable 00CE3818.
void destruct_tactical_library_singleton_004f93a0(std::uint32_t& vtable_00, void*& published_e18d4c) noexcept;
// 004F9980 / 004F9A00, __fastcall(ECX = key, EDX = out int*, LuaObject table BY VALUE (14h)), RET
// 14h: table[key] as an integer (00B67800 then BSP_LuaObject_GetInteger); both temporaries destroyed.
class LuaFieldReader {
public:
    virtual ~LuaFieldReader() = default;
    virtual std::int32_t integer_field(const char* key) = 0;
};
void read_lua_integer_field_004f9980(const char* key, std::int32_t& out, LuaFieldReader& table);
// 004F9A80, __cdecl(char* dst, fmt, ...): _vsprintf_s(dst, 80h, fmt, args).
int format_string128_004f9a80(char* dst, const char* format, ...);
// 004F9AB0, __fastcall(ECX = const char*), RET: 'a'..'z' -> 'A'..'Z', else the (signed) byte.
std::int32_t to_upper_ascii_004f9ab0(const char* c) noexcept;

// 0098C9C0, __fastcall(ECX = info), AL: the OS description (the MSDN "DisplaySystemVersion"
// sample with Vista strings). +C34h and +234h[0] are cleared; GetVersionExA with an
// OSVERSIONINFOEX (9Ch), retried as OSVERSIONINFO (94h); false when both fail. +C34h = 1 for
// major >= 5. Platform 0 "Microsoft Win32s"; platform 1 (major 4) 95 (+"OSR2 " when
// szCSDVersion[1] is 'C' or 'B'), 98 (+"SE " for 'A'), Me; platform 2 the NT names, then either
// the product-type / suite branch (EX) or the ProductOptions\ProductType registry branch, the
// service pack "%s (Build %d)", and for NT 4 "Service Pack 6" the Q246009 hotfix probe. Every
// text is appended (inline strcat) to the string at +234h. The registry branch's failures return
// false; every other path returns true.
struct OsVersionInfoEx {
    std::uint32_t major_04{0};
    std::uint32_t minor_08{0};
    std::uint32_t build_0c{0};
    std::uint32_t platform_10{0};
    std::string csd_version_14{};
    std::uint16_t suite_mask_98{0};
    std::uint8_t product_type_9a{0};
};
class OsVersionCalls {
public:
    virtual ~OsVersionCalls() = default;
    // GetVersionExA with the size word (9Ch, then 94h); false when the call fails.
    virtual bool get_version_ex(std::uint32_t size, OsVersionInfoEx& info) = 0;
    // RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE): true on ERROR_SUCCESS.
    virtual bool reg_open_hklm(const char* path) = 0;
    // RegQueryValueExA on the open key into an 80-byte buffer; false on failure; cbData returned.
    virtual bool reg_query_string(const char* name, std::string& value, std::uint32_t& length) = 0;
    virtual void reg_close() = 0;   // RegCloseKey on the key slot [ESP+10h]
};
bool describe_os_version_0098c9c0(std::string& description_234, bool& nt5_or_later_c34, OsVersionCalls& calls);

// 00995690, __fastcall(ECX = download manager), RET; vtable 00D1D7D8 slot 8: rescans the installed
// content packages. The two string vectors +20h (folders) and +2Ch (package names) are emptied
// (00427110); for each of the +8h packages (XCONTENT_DATA at +38h + i*20h, record +4h + i*18h):
// XLiveContentVerifyInstalledPackage sets record +12h (non-zero HRESULT), and a record with +10h
// set and +11h clear has its folder (XLiveContentGetPath, low bytes, + "\content") mounted
// (00BE1890 with "." -10, 1, -1), listed (vtable +14h), and the listing walked from the last entry:
// each name is logged "%d    %s"; the first whose ".mpkg" index equals size - 5 gives record +8h =
// name.substr(size - 12, 7) and is mounted itself (00BE1890(name, ".", -10, 0, -1)), ending the
// walk. With a package name 00BDEF80(&record+8h); otherwise "Damaged content %s", +12h = 1, the
// unmount 00BE0750(compose(compose(path)), ".") and 00BDD020(&record+8h). Then +11h = +10h && !+12h,
// and either (folder, name) or ("", "") is pushed onto +20h/+2Ch. Finally byte +1Ch = 0.
// A string-based view of ContentPackageRecord (+0h name, +8h path) for the scan.
struct ContentScanRecord {           // 18h
    std::string title_00{};          // +4h is its buffer, logged
    std::string package_08{};
    bool listed_10{false};
    bool mounted_11{false};
    bool damaged_12{false};
};
class ContentScanCalls {
public:
    virtual ~ContentScanCalls() = default;
    virtual std::uint32_t verify_installed_package(std::size_t index) = 0;   // HRESULT, 0 = verified
    // Both XLiveContentGetPath calls (size probe, then the 100h-wchar buffer when the probe returns
    // 8007007Ah with a size up to 100h) and 00436630's low-byte conversion of that buffer. When
    // the probe does not take that path the image converts an unset stack buffer.
    virtual std::string content_path_low_bytes(std::size_t index) = 0;
    virtual std::string compose_path_00994770(const std::string& path) = 0;
    virtual void* vfs_mount_00be1890(const std::string& path, const std::string& name, std::int32_t a,
                                     std::uint32_t b, std::int32_t c) = 0;
    virtual std::vector<std::string> directory_list_14(void* directory, const std::string& pattern,
                                                       const std::string& extension, std::uint32_t flag) = 0;
    virtual void log_004254b0(const std::string& line) = 0;
    virtual void vfs_unmount_00be0750(const std::string& path, const std::string& name) = 0;
    virtual void vfs_package_found_00bdef80(const std::string& package) = 0;
    virtual void vfs_package_missing_00bdd020(const std::string& package) = 0;
};
void download_manager_scan_content_00995690(std::vector<ContentScanRecord>& records,
                                            std::vector<std::string>& folders_20,
                                            std::vector<std::string>& names_2c, bool& busy_1c,
                                            ContentScanCalls& calls);

// 00707B00, __thiscall(ECX = school, float dt), RET 4 (no reference found; a vtable slot of the
// fish model is the likely caller). Main thread, 24-bit x87 precision: float arithmetic.
//   dt == 0 (UCOMISS, ordered equal) returns at once. No game, no camera [game+19FCh], a camera
//   above the water (+124h > 0) or a squared camera distance to the school's first fish (through
//   the model +78h matrix +F0h, 00414D10) above 90000 (00CFD404) sets the model's visibility to
//   0.0 (00B6DA70). Otherwise visibility 1.0; the timer +68h -= dt and 007077B0 respawns at <= 0;
//   k = min(1, +44h * dt) (00415510); fish 0 steers: dir += k * (+6Ch - dir), normalize, pos +=
//   (+24h * dt) * dir (0042AEB0); basis A = (0, 1, 0) x dir, B = dir x A (004F9B30), both
//   normalized. Each other fish i steers toward fish0.pos + o.z dir + o.x A + o.y B (o = positions
//   +60h [i]): u = (target - pos) / (|target - pos| + 0.005) (00419440), dir += k (u - dir),
//   normalize; speed += k (min(+3Ch + (+40h - +3Ch) * L / 5, +40h) - speed); pos += speed dt dir.
class FishUpdateCalls {
public:
    virtual ~FishUpdateCalls() = default;
    virtual void* camera_node_19fc() = 0;               // null when [00E188A8] or the camera is null
    virtual void node_refresh_world_if_stale(void* node) = 0;   // byte +5Ch bit 1 clear -> 00B6DB70
    virtual std::array<float, 3> node_translation_120(void* node) = 0;
    virtual std::array<float, 3> transform_point_00414d10(void* model, const float* point) = 0;  // model +F0h
    virtual void set_visibility_factor_00b6da70(void* model, float factor, std::uint32_t flag) = 0;
    virtual float min_float_00415510(float a, float b) = 0;
    virtual float length_00419440(const float* v) = 0;
    virtual std::array<float, 3> cross_004f9b30(const float* a, const float* b) = 0;
};
void update_fish_school_00707b00(FishSchoolDescLayout& school, float dt, FishUpdateCalls& calls,
                                 PresentationRandom& random, VectorNormalizer& normalizer);

}  // namespace bsp::engine_seg11_35_66_cc13
