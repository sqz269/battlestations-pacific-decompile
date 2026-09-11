// The ten mission Lua bindings the installed scripts reach most often at load time.
//
// Packet cc_lua_core, worktree agent/cc-lua-core. Ghidra was read-only for this packet.
// Every name here is a hypothesis, not a recovered symbol.
//
// docs/LUA_BINDING_TABLE.md ranked the 560 rows of 00E0B7B8 by how many of the 299 installed
// mission scripts call them during the chunk and the four entry points. `CreateScript` (299)
// and `FindEntity` (258) were already read by docs/LUA_BINDING_ENTITY.md. The next ten are
// the subject of this header, in rank order:
//
//   258 PrepareClass 008C8F70            196 Scoring_RealPlayTimeRunning 008B87F0
//   237 Music_Control_SetLevel 008C4D10  194 LoadMessageMap 008C61C0
//   206 GetDifficulty 008AE030           192 Scoring_SetFinalScoringFunctionName 008B8640
//   203 SETLOG 0088C620                  191 SetThink 00897FB0
//   202 SetParty 008A8930                170 EnableMessages 008CFE40
//
// `debugtrap` (160) is excluded: docs/LUA_BINDING_TABLE.md established that its count is the
// probe's own error-handler pushes, not script calls.
//
// Every one of the ten is `__fastcall(lua_State* in ECX)`, returns its result count in EAX,
// and shares the same shape: a one-time static initialisation of the `luakod` log category,
// then BSP_LuaStateOwner_ConstructBorrowed (00B66C00) / BSP_LuaObject_OpenCallFrame (00B679B0),
// argument reads through the indexed accessor 00B677E0, and a close through
// BSP_LuaObject_ResultCount (00B66400) + BSP_LuaStateOwner_Close (00B669A0). That prologue and
// epilogue are the machine's, not the binding's, and are docs/MISSION_LUA_MACHINE.md's; this
// header models only what each binding does between them.
//
// No struct, enum or k* constant declared here is declared by another header in include/bsp.
// Two game-object offsets this packet needed were already declared by
// include/bsp/mission_tree_screens.hpp and are reused rather than restated:
// `kGameNonCampaignFlagOffset` (1FE4h) and `kGameEffectiveDifficultyOffset` (6ACh).
#ifndef BSP_LUA_BINDING_CORE_HPP
#define BSP_LUA_BINDING_CORE_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

// ---------------------------------------------------------------------------
// Game-object fields these ten bindings touch, all relative to *(00E188A8)
// ---------------------------------------------------------------------------

// 008B88FA and 008B8753 both load `[[00E188A8] + 21A0h]` as the `this` of a scoring method.
// Two distinct fields of that object are written by two of the ten, below.
inline constexpr std::size_t kGameMissionScoringOffset = 0x21A0;

// 008C62FC loads `[[00E188A8] + 21DCh]` as the `this` of the message-map loader 00706760.
// include/bsp/world_construct.hpp's `kTailConstructions` already records that game+21DCh is a
// 70h-byte object built by 00707480 at 004DFA1A, which is the producer for this field.
inline constexpr std::size_t kGameMessageMapManagerOffset = 0x21DC;

// 008C63D0 and 008C4E66 both take `[00E188A8] + 1EF0h` (an interior pointer, not a load) as
// the `this` of a session sender: 0076A9F0 for the message map and 0076D280 for the music
// level. The music-level path reloads the game pointer at 008C4E56 rather than reusing the
// music base it loaded at 008C4E05, which is the only reason the two are distinguishable.
inline constexpr std::size_t kGameSessionSenderOffset = 0x1EF0;

// Fields of the scoring object at game+21A0h.
inline constexpr std::size_t kMissionScoringRealPlayTimeRunningOffset = 0x14A4; // 00905344
inline constexpr std::size_t kMissionScoringFinalFunctionNameOffset = 0x147C;   // 0090BFC0

// Fields of the native entity that two of the ten write.
inline constexpr std::size_t kEntityThinkScriptNameOffset = 0x1D8; // 0088A333, char* or null
inline constexpr std::size_t kEntityThinkStateByteOffset = 0x1DC;  // 0088A37F, always cleared
inline constexpr std::size_t kEntityPartyFieldOffset = 0x58;       // 008A8AD2, passed to vtbl+2Ch

// Fields of the message system singleton at *(00F8A0C4).
inline constexpr std::size_t kMessageSystemGlobalSuppressOffset = 0xD0;   // 008D0000
inline constexpr std::size_t kMessageSystemPerEntityMapOffset = 0xD4;     // 008CFFDA

// The two entity virtuals SetParty dispatches through, by byte offset in the vtable.
// Neither callee body was read: a virtual has no single callee, and the concrete vtable was
// not resolved. The host below therefore names them by slot, never by a verb.
inline constexpr std::size_t kEntityVtableSlotPartyQuery = 0x5C;  // 008A8A7D, called with 2
inline constexpr std::size_t kEntityVtableSlotSetParty = 0x2C;    // 008A8AD7

// The literal argument SetParty passes to the +5Ch virtual at 008A8A80 (PUSH EBX, EBX = 2).
inline constexpr int kEntityPartyQuerySelector = 2;

// The session message SetParty builds when the +5Ch query answers non-zero: base id 7Fh
// (008A8A89 PUSH 7Fh into 0075B430), vtable pointer 00D032E8 (008A8AA6), dword 1 at the
// message base (008A8A99) and the new party at message+1Ch (008A8AAE). It is routed by
// 0077C2A0 with channel 7 and flag 0 (008A8AB3/008A8AB2).
inline constexpr int kSetPartySessionMessageId = 0x7F;
inline constexpr std::uint32_t kSetPartySessionMessageVtable = 0x00D032E8u;
inline constexpr std::size_t kSetPartySessionMessagePartyOffset = 0x1C;
inline constexpr int kSetPartySessionChannel = 7;

// The session message Music_Control_SetLevel builds on the non-campaign path: base id 2Ah
// (0076D28E), vtable 00D0319C, dword 1 at the base and the level at message+10h; sent by
// 00784790 with flag 0. Read from 0076D280, the callee, not from the call site.
inline constexpr int kMusicLevelSessionMessageId = 0x2A;
inline constexpr std::uint32_t kMusicLevelSessionMessageVtable = 0x00D0319Cu;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// GetDifficulty 008AE030, the whole body. 008AE113 compares game+1FE4h against zero; the
// non-zero branch pushes the literal 2 (008AE123) and the zero branch pushes the dword at
// game+6ACh (008AE12A). Both go to 00B664B0, which is lua_checkstack(1) + lua_pushnumber, so
// the script always sees a number even though the source is an int.
//
// mission_tree_screens.hpp names 1FE4h the non-campaign flag and 6ACh the effective
// difficulty, so in those terms: outside a campaign the binding reports a fixed 2, and in a
// campaign it reports whatever the launch path wrote.
int lua_binding_difficulty_value(int non_campaign_flag, int effective_difficulty) noexcept;

// The literal 2 of 008AE123, named so the rule above reads as the listing does.
inline constexpr int kNonCampaignReportedDifficulty = 2;

// PrepareClass 008C8F70 builds this path per argument and resolves it against the globals:
// "VehicleClass." (00CE9248, 13 bytes) + the decimal class id (004263B0 over 004260B0) +
// ".Race" (00D153E0, 5 bytes). Pure; no state.
std::string vehicle_class_race_path(int class_id);

inline constexpr const char* kVehicleClassPathPrefix = "VehicleClass."; // 00CE9248
inline constexpr const char* kVehicleClassRacePathSuffix = ".Race";     // 00D153E0

// 008C9282..008C9290: the party-required flag PrepareClass derives from the resolved `.Race`
// value is 1 when the race is 1 or 4 and 0 otherwise. 0095BA60 then ignores anything that is
// not 0 or 1, so this rule is the whole domain of that call.
int vehicle_class_party_from_race(int race) noexcept;

// EnableMessages 008CFE40 accepts three argument shapes, decided by 00B663F0 alone:
//   argc >= 2: slot 0 is the entity table (008CFF4E), slot 1 the flag (008CFF71 CMP EAX,2
//              then PUSH 1 at 008CFF7A)
//   argc == 1: no entity, slot 0 is the flag (008CFF95 CMP EAX,1 then PUSH EBP=0 at 008CFF9A)
//   argc == 0: no entity and the flag defaults to true (008CFF6A MOV BL,1, never overwritten)
// This returns the slot holding the flag, or -1 when there is none.
int enable_messages_flag_slot(int argument_count) noexcept;

// Both EnableMessages stores write the negation of the script's flag: 008CFFE9/008CFFEB is
// TEST BL,BL + SETZ DL, and 008CFFFC/008D0000 is the same over the global. The byte the engine
// keeps is a suppression flag, not an enable flag, which is why this rule exists separately
// from the routine.
bool message_suppression_byte(bool messages_enabled) noexcept;

// Degrees to radians as the image spells it: 00CE3D28 is the double 3.141592653589793 and
// 00CE3D20 the double 180.0, multiplied then divided in that order. The spawn binding does it
// twice, at 00944980/00944992 and at 009449C3/009449D5, on the two angle arguments. Declared
// here because both this header and lua_binding_spawn.hpp need it; it is a rule, not a
// reconstruction of a routine.
double lua_binding_degrees_to_radians(double degrees) noexcept;

// ---------------------------------------------------------------------------
// The argument and result surfaces
// ---------------------------------------------------------------------------

// What the ten bindings read from their call frame. Every method is one native accessor; the
// index is the Lua argument index, which 00B67720 turns into slot `base + index` with base 1
// (include/bsp/mission_lua_bindings.hpp's `mission_binding_argument_slot`). The routines never
// index past what `count` reports.
struct LuaBindingArgumentReader {
    virtual ~LuaBindingArgumentReader() = default;
    virtual int count() = 0;                         // 00B663F0
    virtual int get_integer(int index) = 0;          // 00B66290, __ftol, truncates toward zero
    virtual double get_number(int index) = 0;        // 00B66270
    virtual bool get_boolean(int index) = 0;         // 00B66250
    virtual std::string get_string(int index) = 0;   // 00B662B0, borrowed, copied by the caller
    virtual bool is_string(int index) = 0;           // 00B660A0
    virtual bool is_nil(int index) = 0;              // 00B65FB0
    virtual bool is_entity_table(int index) = 0;     // 008889C0
    virtual void* entity_at(int index) = 0;          // 00888AA0, null when the table is not one
};

// What they push back. `push_number` is 00B664B0, which is lua_pushnumber over an int, so an
// integer result still arrives in Lua as a number.
struct LuaBindingResultWriter {
    virtual ~LuaBindingResultWriter() = default;
    virtual void push_number(int value) = 0;   // 00B664B0
    virtual void push_boolean(bool value) = 0; // 00B66450
    virtual void push_nil() = 0;               // 00B66430
};

// ---------------------------------------------------------------------------
// The host: one method per native call site
// ---------------------------------------------------------------------------

// There are no default implementations. A method whose callee body this packet did not read
// says so in its comment and is named by its address, per the callee-before-contract rule of
// docs/WORKER_VERIFICATION_CHECKLIST.md.
struct LuaBindingCoreHost {
    virtual ~LuaBindingCoreHost() = default;

    // --- game-object reads ---
    // 008AE113, 008C4E3F and 008C6340 all load the same dword.
    virtual int game_non_campaign_flag() = 0;
    // 008AE12A, only on the campaign branch of GetDifficulty.
    virtual int game_effective_difficulty() = 0;

    // --- PrepareClass 008C8F70 ---
    // 004254B0 at 008C9099 with the format "Prepare class %d" (00D153E8). The callee is the
    // variadic log sink; the binding passes exactly one int (008C9099 ADD ESP,8 after two
    // pushes, so format plus one argument).
    virtual void log_prepare_class(int class_id) = 0;
    // 00B68D70 at 008C91BA over the globals object 00B67980 returned for game+1A0Ch. Answers
    // whether the path resolved to an integer and, when it did, its value. The native keeps
    // the resolved LuaObject and asks 00B66A60 (is-integer) before 00B66290; this folds the
    // two into one call because the routine uses them together and nothing else.
    virtual bool resolve_global_integer(const std::string& dotted_path, int& value) = 0;
    // 0095BA60 at 008C9297, __fastcall(class id in ECX, party flag in EDX). Body read: it
    // returns immediately unless the flag is 0 or 1, maps the id through the vehicle-class
    // registry and sets a byte in the per-party required-class bitmap. docs/SCENE_UNIT_CREATORS.md.
    virtual void vehicle_class_mark_party_required(int class_id, int party) = 0;
    // 00964790 at 008C92BE, __fastcall(class id in ECX, read-race flag in DL=1). Body read:
    // the vehicle-class descriptor factory. docs/VEHICLE_CLASS_DESCRIPTORS.md.
    virtual void vehicle_class_get_or_create(int class_id, bool read_race) = 0;

    // --- Music_Control_SetLevel 008C4D10 ---
    // 00A788A0 at 008C4E1D, __thiscall(*(00F8BBCC), level). Body read: returns at once when
    // this+208h already holds the level, stores it, indexes a table of 18h-byte rows at
    // this+10h by `level + this+210h * 9`, logs "MUSIC NOT FOUND %d %d" when the row is empty,
    // and otherwise compares the playing track name before switching.
    virtual void music_director_set_level(int level) = 0;
    // 0076D280 at 008C4E74, __thiscall over *(00E188A8)+1EF0h (the game pointer is reloaded at
    // 008C4E56; it is not the music base). Body read: builds session message 2Ah with vtable
    // 00D0319C carrying the level and hands it to 00784790.
    virtual void session_send_music_level(int level) = 0;

    // --- SetParty 008A8930 ---
    // The entity virtual at vtable+5Ch, called at 008A8A83 with the literal 2 and answering a
    // byte. Not read: it is a virtual and the concrete vtable was not resolved. Named by slot.
    virtual bool entity_vcall_5c(void* entity, int selector) = 0;
    // The entity virtual at vtable+2Ch, called at 008A8AE3 with (party, entity+58h, &zero).
    // Not read, for the same reason. The third argument is a local initialised to zero at
    // 008A8ADA and is an out parameter the binding discards.
    virtual void entity_vcall_2c(void* entity, int party, std::uint32_t entity_field_58) = 0;
    // 0077C2A0 at 008A8AC4, __thiscall(entity, message, 7, 0). Not read by this packet; the
    // ledger name BSP_Session_RouteMessage predates it and is carried through unchanged.
    virtual void session_route_party_message(void* entity, int party) = 0;

    // --- Scoring_RealPlayTimeRunning 008B87F0 ---
    // 00905340 at 008B8901, __thiscall(scoring, bool). Body read in full, three instructions:
    // MOV AL,[ESP+4]; MOV [ECX+14A4h],AL; RET 4.
    virtual void scoring_set_real_play_time_running(bool running) = 0;

    // --- Scoring_SetFinalScoringFunctionName 008B8640 ---
    // 0090BFC0 at 008B8766, __thiscall(scoring, NativeString*). Body read: a NativeString
    // assignment into the field at scoring+147Ch, self-assignment guarded.
    virtual void scoring_set_final_scoring_function_name(const std::string& name) = 0;

    // --- LoadMessageMap 008C61C0 ---
    // 00706760 at 008C6308, __thiscall(game+21DCh, NativeString*, int). Body not read in full.
    // What it reaches was read from its callee list and its two literals: it resolves the
    // language through 008D4870/008D56C0 ("englishauthentic"), checks downloadable-content
    // ownership through 007F8890 ("DL_Content_0000062") and builds a file block through
    // 00BE0A30. Contract: partial.
    virtual void message_map_load(const std::string& name, int index) = 0;
    // 0088B6D0 at 008C63C3, then 0076A9F0 at 008C63DE and 00765590 at 008C63EE, the three
    // steps of the non-campaign branch in order. None of the three callee bodies was read;
    // they are named by address. Contract: unread.
    virtual void call_0088b6d0_0076a9f0_00765590(const std::string& name, int index) = 0;

    // --- SetThink 00897FB0 ---
    // 0088A330 at 008980E8, __thiscall(entity, const char*). Body read, including the thirteen
    // bytes at 0088A360 that Ghidra leaves undisassembled: see the Corrections section of
    // docs/LUA_BINDING_CORE.md. Frees any previous name, duplicates the new one through
    // 00438E40, and always clears entity+1DCh.
    virtual void entity_set_think_script_name(void* entity, const std::string& name) = 0;

    // --- EnableMessages 008CFE40 ---
    // 0077EDF0 at 008CFFE4, __thiscall(*(00F8A0C4)+D4h, &entity). Body read: a red-black tree
    // find-or-insert keyed on the entity pointer as a uint, returning the mapped slot. The
    // binding writes one byte through the returned pointer at 008CFFEE.
    virtual void set_entity_message_suppression(void* entity, bool suppressed) = 0;
    // The direct store at 008D0000 into *(00F8A0C4)+D0h. No callee: this is the global form.
    virtual void set_global_message_suppression(bool suppressed) = 0;
    // 0096BF70 at 008D000A, __fastcall(*(00F8A0C4)). Body read far enough to establish that it
    // takes the critical section at this+24h and walks the list at this+E4h; what it does to
    // each element was not read. Contract: partial.
    virtual void message_system_drain_queue() = 0;
};

// ---------------------------------------------------------------------------
// The ten routines
// ---------------------------------------------------------------------------

// Each returns the value the native leaves in EAX: 00B66400's `lua_gettop - frame.base`, which
// for these ten is the number of values the body pushed, because none of them pops.

// 0088C620. The complete body: open the frame, take the result count, close. It reads no
// argument, touches no game state and pushes nothing, so 203 scripts call a routine that does
// nothing observable beyond the one-time `luakod` category init at 00F87938/00F8793C. Verified
// against the listing, not only the pseudocode: 0088C620..0088C74B is 25 instructions with no
// branch into game code.
int lua_binding_setlog() noexcept;

// 008AE030. Pushes one number and returns 1.
int lua_binding_get_difficulty(LuaBindingResultWriter& results, LuaBindingCoreHost& host);

// 008C8F70. Loops over every argument; pushes nothing and returns 0.
int lua_binding_prepare_class(LuaBindingArgumentReader& args, LuaBindingCoreHost& host);

// 008C4D10. Reads argument 0 as an integer twice, once per branch; returns 0. The session
// branch runs only when the non-campaign flag is exactly 1 (008C4E3E CMP EAX,EBP with EBP
// zero, then 008C4E42 CMP EAX,1); any other value takes the music-director path alone.
int lua_binding_music_control_set_level(LuaBindingArgumentReader& args, LuaBindingCoreHost& host);

// 008A8930. Reads the entity and the party, takes one of two paths, pushes the party back as a
// number and returns 1. The entity pointer is used without a null check at 008A8A7B, so a
// script that passes a non-entity table reaches a null dereference in the native; the
// reconstruction returns 0 without touching the host instead of reproducing the fault, and
// says so here rather than hiding it.
int lua_binding_set_party(LuaBindingArgumentReader& args,
                          LuaBindingResultWriter& results,
                          LuaBindingCoreHost& host);

// 008B87F0. Sets the flag and pushes it back; returns 1. The value pushed is the AL the setter
// left, which is the same byte the script passed (00905340 writes AL and returns without
// touching EAX further).
int lua_binding_scoring_real_play_time_running(LuaBindingArgumentReader& args,
                                               LuaBindingResultWriter& results,
                                               LuaBindingCoreHost& host);

// 008B8640. Copies argument 0 into the scoring object; returns 0.
int lua_binding_scoring_set_final_scoring_function_name(LuaBindingArgumentReader& args,
                                                        LuaBindingCoreHost& host);

// 008C61C0. Loads the map, then repeats the read and takes the session path when the
// non-campaign flag is exactly 1 (008C635B CMP dword [EDX+1FE4h],1, not a non-zero test);
// returns 0.
int lua_binding_load_message_map(LuaBindingArgumentReader& args, LuaBindingCoreHost& host);

// 00897FB0. Entity from argument 0, name from argument 1; returns 0. Same missing null check
// as SetParty, handled the same way.
int lua_binding_set_think(LuaBindingArgumentReader& args, LuaBindingCoreHost& host);

// 008CFE40. Three argument shapes; returns 0.
int lua_binding_enable_messages(LuaBindingArgumentReader& args, LuaBindingCoreHost& host);

// ---------------------------------------------------------------------------
// The three doubly-named handlers
// ---------------------------------------------------------------------------

// docs/LUA_BINDING_TABLE.md left open whether the three handlers that carry two names each
// differ by argument. They cannot: a lua_CFunction is handed only the state, all three are
// registered by 006B8610 with `nup = 0` so there is no upvalue to read, and none of the three
// calls lua_getinfo or any upvalue accessor. The pair is one entry point under two globals.
// docs/LUA_BINDING_ALIASES.md carries the evidence per handler.
struct MissionLuaBindingAlias {
    std::uint32_t handler;
    const char* first_name;
    const char* second_name;
};

inline constexpr MissionLuaBindingAlias kMissionLuaBindingAliases[] = {
    {0x00896A90u, "AddAirBaseStock", "AddAirBasePlanes"},
    {0x008B0C10u, "MissionNarrative", "MissionNarrativeEnqueue"},
    {0x0088E560u, "SetMotionBlurParams", "SetMBP"},
};

inline constexpr std::size_t kMissionLuaBindingAliasCount = 3;

// True when the two names of an aliased row are indistinguishable inside the handler. It is
// constant for all three rows; the predicate exists so that a caller states the claim it
// depends on rather than assuming it.
bool mission_binding_alias_is_indistinguishable(std::uint32_t handler) noexcept;

} // namespace bsp

#endif // BSP_LUA_BINDING_CORE_HPP
