#pragma once
// The GUI's Lua binding: the reader behind the descriptor visitor that
// 00AAA710 drives, and the script half of the screen constructor that creates
// the interpreter it reads from.
//
// Addresses: 00AC6600 (screen constructor, script half), 00B6A020 (state
//            creation and library mask), 00B669C0 (panic handler),
//            00B69D40 (run a script and its overrides), 00B66CA0 (load and
//            call one chunk), 00B67980 (globals), 004425C0 (visitor
//            constructor), 00BD8E20/00BD7A20/00BD68D0/00BD6830/00BD5EB0/
//            00BD5F50 (the six visitor virtuals), 00BD5790 (child lookup),
//            00BD63B0 (Lua value -> typed field), 00BD61C0 (default -> typed
//            field), 00441210/00442220/00BD7130 (the LuaObject stack).
//
// Every name below is a hypothesis, not a recovered symbol. The evidence is in
// docs/GUI_LUA_READER.md. Nothing here is binary compatible with the original:
// the native reader holds real Lua references and this one holds host handles.
// The interpreter is Lua 5.1.1 and is matched by name in the inventory, not
// rebuilt here; every call site that reaches it is a host method.
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/gui_layout_loader.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The interpreter the GUI creates
// ---------------------------------------------------------------------------

// The bit each entry of the library table at 00D62BB8 answers to, in table
// order. 00B6A020 always opens base, then for every other set bit runs the standard
// Lua 5.1.1 open sequence: lua_pushcclosure(L, opener, 0) (00A67B20),
// lua_pushstring(L, name) (00A67A50), lua_call(L, 1, 0) (00A68090).
enum class GuiLuaLibrary : std::uint32_t {
    Base = 0x01,     // name "" at 00CE3A0C, opener 00A67210
    Package = 0x02,  // opener 00C2FF90, the loadlib module
    Table = 0x04,    // name 00CF8344, opener 00A66010
    Io = 0x08,       // name 00CF8340, opener 00A652B0
    Os = 0x10,       // name 00CF833C, opener 00A64190
    String = 0x20,   // name 00CF8334, opener 00A63910
    Math = 0x40,     // name 00CF832C, opener 00A61C50
    Debug = 0x80,    // name 00CF8324, opener 00A61620
};

// 00AC6600 pushes 65h at 00AC671D. Base, table, string and math only: the GUI
// page scripts get no io, os, package or debug.
inline constexpr std::uint32_t kGuiLuaLibraryMask = 0x65;

// The libraries `mask` opens, in table order. Names are the literals above,
// with the base library reported as the empty string the table holds.
std::vector<std::string_view> gui_lua_libraries_00b6a020(std::uint32_t mask);

// 00B669C0, installed with lua_atpanic (00A67390) at 00B6A03B. It reads the
// message at the top of the stack with lua_gettop + lua_tolstring, drops it and
// returns 0. Nothing is logged and nothing is recovered, so returning lets
// Lua 5.1.1's luaD_throw fall through to exit(EXIT_FAILURE).
inline constexpr std::uint32_t kGuiLuaPanicHandler = 0x00B669C0u;

// 00B66CA0 runs the chunk with lua_call (00A68090), not lua_pcall, and ignores
// what luaL_loadbuffer returned. A page script that fails to compile or raises
// at run time therefore reaches the panic handler and takes the process with
// it; there is no per-page recovery anywhere on this path.
inline constexpr bool kGuiLuaScriptErrorsAreFatal = true;

// ---------------------------------------------------------------------------
// The 8-byte variant the visitor speaks
// ---------------------------------------------------------------------------

// Every argument the visitor's virtuals take is one of these pairs: a tag and
// one machine word. As a key the tag is a GuiLuaKeyKind, as a field it is a
// GuiLuaFieldType and the word is the destination address, as a default it is
// the value itself. 00BD61C0 reads the tag from the field pair only.
struct GuiLuaVariant {
    std::int32_t tag{0};
    union Value {
        const char* text;
        std::int32_t integer;
        float number;
        void* pointer;
    } value{};
};

// ---------------------------------------------------------------------------
// Keys
// ---------------------------------------------------------------------------

// The selector 00BD5790 switches on. Anything else leaves the looked-up object
// default constructed, which reads back as nil.
enum class GuiLuaKeyKind : std::int32_t {
    Name = 0,        // 00BD57E2: lua_pushlstring + lua_gettable through 00B67800
    Index = 1,       // 00BD5802: lua_pushnumber + lua_gettable through 00B67720
    FloatIndex = 2,  // 00BD5824: CVTTSS2SI first, then the same integer path
};

GuiLuaVariant gui_lua_key_by_name(const char* name) noexcept;
GuiLuaVariant gui_lua_key_by_index(std::int32_t index) noexcept;
GuiLuaVariant gui_lua_key_by_float(float index) noexcept;

// ---------------------------------------------------------------------------
// Field types
// ---------------------------------------------------------------------------

// The switch of 00BD63B0. GuiValueTag in bsp/gui_layout_loader.hpp names the
// six the base widget descriptors use; this is the whole set, and the static
// asserts below keep the two from drifting.
enum class GuiLuaFieldType : std::int32_t {
    String = 0,       // 00BD63D4: lua_tolstring, then a native string assign
    Int = 1,          // 00BD642F: lua_tonumber narrowed by __ftol
    Float = 2,        // 00BD6452: lua_tonumber stored as one float
    Bool = 3,         // 00BD647A: lua_toboolean stored as one byte
    Handle = 4,       // 00BD649E: an integral number through 0109CED4, a table
                      //           through 0109CED8, anything else ignored
    Vec3 = 5,         // 00BD6509: t[1], t[2], t[3] as floats
    Vec2 = 6,         // 00BD65C7: t[1], t[2] as floats
    Matrix4 = 7,      // 00BD664A: t[i][j], i and j in 1..4, row major
    Vec4 = 8,         // 00BD66D8: t[1]..t[4] as floats
    Unhandled9 = 9,   // no case: 00BD66D2 falls to the 0Ah test and then out
    ParsedFloat = 10, // 00BD67F2: lua_tolstring then the CRT parse at 00BF8417
};

static_assert(static_cast<std::int32_t>(GuiLuaFieldType::String) ==
                  static_cast<std::int32_t>(GuiValueTag::String),
              "field type 0 is the tag the base descriptors call String");
static_assert(static_cast<std::int32_t>(GuiLuaFieldType::Float) ==
                  static_cast<std::int32_t>(GuiValueTag::Float),
              "field type 2 is the tag the base descriptors call Float");
static_assert(static_cast<std::int32_t>(GuiLuaFieldType::Bool) ==
                  static_cast<std::int32_t>(GuiValueTag::Bool),
              "field type 3 is the tag the base descriptors call Bool");
static_assert(static_cast<std::int32_t>(GuiLuaFieldType::Vec3) ==
                  static_cast<std::int32_t>(GuiValueTag::Vec3),
              "field type 5 is the tag the base descriptors call Vec3");
static_assert(static_cast<std::int32_t>(GuiLuaFieldType::Vec2) ==
                  static_cast<std::int32_t>(GuiValueTag::Vec2),
              "field type 6 is the tag the base descriptors call Vec2");
static_assert(static_cast<std::int32_t>(GuiLuaFieldType::Vec4) ==
                  static_cast<std::int32_t>(GuiValueTag::Color),
              "field type 8 is the tag the base descriptors call Color");

// Word count each type consumes from an array-like table. Zero for the scalar
// types, which read the value itself.
std::size_t gui_lua_field_arity(GuiLuaFieldType type) noexcept;

// A field pair: the type and where the value lands. `dest` is a std::string*
// for String, a std::int32_t* for Int and Handle, a float* for Float and
// ParsedFloat and for the first element of Vec2/Vec3/Vec4/Matrix4, and a bool*
// for Bool. The native equivalents are a NativeString, an int, a float and a
// byte at the widget offsets docs/GUI_LAYOUT_LOADER.md lists.
GuiLuaVariant gui_lua_field(GuiLuaFieldType type, void* dest) noexcept;

// The two indirect calls 00BD63B0 makes for GuiLuaFieldType::Handle. The global
// at 0109CED4 takes the integral number the script gave; the one at 0109CED8
// takes the object itself, and is reached only when the object is a table
// (00B661B0 tests lua_type against 5). Both are installed by 00BD4FC0 and
// neither target was followed, so a caller that has no resolver gets a false
// back and the destination is left alone. That host boundary does not imply
// that the original indirect-call targets may be null.
// A value that is neither an integral number nor a table writes nothing.
struct GuiLuaHost;
struct GuiLuaRef;
struct GuiLuaHandleResolver {
    virtual ~GuiLuaHandleResolver() = default;
    virtual std::int32_t resolve_by_number(std::int32_t id) = 0;  // 0109CED4
    // Evaluated-value projection of 0109CED8. The parser's concrete table is
    // preserved; the live reader uses the distinct method below.
    virtual std::int32_t resolve_by_table(const GuiTable* table) = 0;
    // 00BD64E0..00BD64E9 passes the actual looked-up LuaObject to 0109CED8.
    // This token belongs to the supplied host and is borrowed for this call;
    // do not release it or retain it past the caller's normal reader leave.
    // Pass by value so host operations cannot invalidate the token reference.
    virtual std::int32_t resolve_by_live_table(GuiLuaHost& host, GuiLuaRef object) = 0;
};

// ---------------------------------------------------------------------------
// The conversions, as pure functions over an evaluated value
// ---------------------------------------------------------------------------

// 00BD63B0, __fastcall(ECX = the looked-up object, EDX = the field pair), RET.
// Writes one Lua value into one typed field. False means nothing was written:
// the type has no case (9, and anything above 0Ah), or Handle had no resolver,
// or the value was the wrong shape for an aggregate. The native routine has no
// such report; it simply leaves the destination as it found it.
bool gui_lua_store_value_00bd63b0(const GuiValue& value, const GuiLuaVariant& field,
    GuiLuaHandleResolver* resolver, const bool& crt_sse2_conversion) noexcept;

// 00BD61C0, __thiscall(ECX = the field pair, the default pair), RET 4. Copies
// the default's word into the destination using the *field's* tag, so a default
// whose own tag disagrees is still written as the field's type. Aggregates read
// the default's word as the address of the first element; Matrix4 tail calls the
// 40h-byte copy at 004134F0. ParsedFloat is the one asymmetry in the pair: the
// value path parses a string, the default path takes a plain float, and Handle
// takes a raw id with no resolver. Type 9 writes nothing here either.
bool gui_lua_store_default_00bd61c0(const GuiLuaVariant& field,
                                    const GuiLuaVariant& fallback) noexcept;

// ---------------------------------------------------------------------------
// The key array the enumerator fills
// ---------------------------------------------------------------------------

// 00BD5F50 writes entries at out + index * 8 and keeps the count at out + 7D0h,
// so the buffer holds 250 pairs followed by the count. 00AAA710 stages one on
// its frame and clears the 7D0h bytes to FFFFFFFFh before the call. Nothing
// bounds checks the index: a table with more than 250 keys walks past the end.
inline constexpr std::size_t kGuiLuaKeyArrayCapacity = 250;

struct GuiLuaKeyArray {
    GuiLuaVariant keys[kGuiLuaKeyArrayCapacity]{};
    std::int32_t count{0};
};

#if defined(_WIN32) && !defined(_WIN64)
static_assert(sizeof(GuiLuaVariant) == 8, "the native pair is two dwords");
static_assert(offsetof(GuiLuaKeyArray, count) == 0x7D0,
              "00BD5F50 keeps the count at base + 7D0h");
#endif

// What 00AAA710 stages before the call: every tag FFFFFFFFh, count zero.
void gui_lua_clear_key_array_00aaa710(GuiLuaKeyArray& array) noexcept;

// ---------------------------------------------------------------------------
// The host: one method per native Lua call site
// ---------------------------------------------------------------------------

// A handle to one entry of the reader's object stack. The native object is the
// 14h-byte LuaObject of the wrapper layer, whose first word is the state and
// whose third is the stack index; 00B67980 hands back one holding FFFFD8EEh,
// which is LUA_GLOBALSINDEX. Nothing here reproduces that layout.
struct GuiLuaRef {
    std::uint32_t id{0};
    bool valid() const noexcept { return id != 0; }
};

// Lua 5.1.1 type codes, as lua_type (00A675A0) returns them. Only the ones the
// reader tests are named.
enum class GuiLuaType : std::int32_t {
    None = -1,
    Nil = 0,
    Boolean = 1,
    Number = 3,
    String = 4,
    Table = 5,
};

// Every native call that leaves the reader for the interpreter. The wrapper
// functions in the comments are the game's thin accessors; the lua_* name after
// each is the library entry point they reach, which is what a rebuild links.
struct GuiLuaHost {
    virtual ~GuiLuaHost() = default;

    // 00B67980: the globals pseudo-index. No library call; the object is built
    // in place with the state and LUA_GLOBALSINDEX.
    virtual GuiLuaRef globals() = 0;

    // 00B67800 -> lua_gettop (00A673D0), lua_pushlstring (00A67A10),
    // lua_gettable (00A67C20).
    virtual GuiLuaRef get_by_name(const GuiLuaRef& table, const char* key) = 0;

    // 00B67720 -> lua_gettop, lua_pushnumber (00A679D0), lua_gettable.
    virtual GuiLuaRef get_by_index(const GuiLuaRef& table, std::int32_t key) = 0;

    // 00B67080 with restart true (lua_pushnil (00A679C0) then lua_next
    // (00A683A0)), 00B67190 otherwise (lua_pushvalue (00A67570) then lua_next).
    // False when lua_next reported the end, which leaves key and value unset.
    virtual bool next(const GuiLuaRef& table, GuiLuaRef& key, GuiLuaRef& value,
                      bool restart) = 0;

    // 00A675A0 lua_type, behind 00B65FB0, 00B66050, 00B660A0 and 00B661B0.
    virtual GuiLuaType type_of(const GuiLuaRef& object) = 0;

    virtual bool to_boolean(const GuiLuaRef& object) = 0;   // 00A677E0
    virtual double to_number(const GuiLuaRef& object) = 0;  // 00A67770
    virtual const char* to_string(const GuiLuaRef& object) = 0;  // 00A67810
    virtual void* to_userdata(const GuiLuaRef& object) = 0;  // 00B662D0 ->00A67910

    // 00B67700, the LuaObject destructor: it unrefs through 00B66DE0.
    virtual void release(const GuiLuaRef& object) = 0;
};

// The tests 00BD5F50 and 00BD63B0 make, in the order they make them.
bool gui_lua_is_nil_00b65fb0(GuiLuaHost& host, const GuiLuaRef& object);
bool gui_lua_is_string_00b660a0(GuiLuaHost& host, const GuiLuaRef& object);
bool gui_lua_is_number_00b66050(GuiLuaHost& host, const GuiLuaRef& object);
// 00B66A60: lua_type == number, then x87 float32 spill, CVTTSS2SI and an
// ordered comparison of the float with that signed int32. A double rounded
// to an integral float qualifies; NaN and values outside int32 do not.
// Win32 preserves the native x87/SSE instruction and caller FP environment.
bool gui_lua_is_integer_number_00b66a60(double number) noexcept;
bool gui_lua_is_integer_00b66a60(GuiLuaHost& host, const GuiLuaRef& object);

// 00BD63B0 as the reader actually reaches it: the same rules against a live
// object, with the aggregate cases indexing it through 00B67720 instead of a
// materialised table. The GuiValue form above is the same switch over an
// evaluated value, which is what the static page parser produces.
bool gui_lua_store_ref_00bd63b0(GuiLuaHost& host, const GuiLuaRef& object,
                                const GuiLuaVariant& field,
                                GuiLuaHandleResolver* resolver,
                                const bool& crt_sse2_conversion);

// ---------------------------------------------------------------------------
// The reader
// ---------------------------------------------------------------------------

// The visitor 004425C0 builds: a vtable at 00CE44FC over a std::vector of
// LuaObject at +4h, seeded with one element. Its abstract base's vtable is at
// 00CE374C, whose slots +4h..+18h are pure, so any reader of the same six
// operations can stand in; the Lua one is the only implementation the GUI uses.
//
// The stack is the path from the globals table to the table being read, so the
// depth is the nesting depth of the page script's tables plus one.
class GuiLuaReader {
public:
    // Required alias of the live0109EEA4 decision. It must outlive the reader;
    // conversion reads it after the Lua callback and the float32 spill/reload.
    GuiLuaReader(GuiLuaHost& host, const GuiLuaRef& root,
        const bool& crt_sse2_conversion);
    ~GuiLuaReader();

    GuiLuaReader(const GuiLuaReader&) = delete;
    GuiLuaReader& operator=(const GuiLuaReader&) = delete;

    // 00BD8E20, vtable +4h, RET 8. Looks the key up in the table on top and
    // pushes the result, whatever it is: a missing key pushes nil and the walk
    // continues, so a caller that never checks reads a whole subtree of nils.
    void enter_00bd8e20(const GuiLuaVariant& key);

    // 00BD7A20, vtable +8h, RET. An adjustor thunk that adds 4 to `this` and
    // tail calls the vector's pop_back at 00BD7130. Popping the seed element is
    // possible and leaves the reader unusable; the native code never does it.
    void leave_00bd7a20();

    // 00BD5EB0, vtable +14h, RET 8. Returns whether the key is present, as
    // "the looked-up object is not nil".
    bool has_key_00bd5eb0(const GuiLuaVariant& key);

    // 00BD6830, vtable +10h, RET 10h. Looks the key up and converts it into the
    // field with no presence test: an absent key reaches 00BD63B0 as nil, and
    // what that leaves behind is the type's business.
    bool read_00bd6830(const GuiLuaVariant& key, const GuiLuaVariant& field,
                       GuiLuaHandleResolver* resolver = nullptr);

    // 00BD68D0, vtable +0Ch, RET 18h. The same, except that a nil takes the
    // default through 00BD61C0 instead of the value through 00BD63B0. This is
    // the slot the property descriptors of 00AAA710 use.
    bool read_or_default_00bd68d0(const GuiLuaVariant& key, const GuiLuaVariant& field,
                                  const GuiLuaVariant& fallback,
                                  GuiLuaHandleResolver* resolver = nullptr);

    // 00BD5F50, vtable +18h, RET 4. Walks the table on top with lua_next and
    // records every key: strings as tag 0 with the char*, integral numbers as
    // tag 1, other numbers as tag 2 with the float. A key of any other type is
    // recorded nowhere and does not advance the count, but the iteration goes
    // on. Returns the count, which is also left at array.count.
    std::size_t enumerate_keys_00bd5f50(GuiLuaKeyArray& array);

    // The 14h-byte elements between the seed and the top.
    std::size_t depth() const noexcept { return stack_.size(); }

private:
    GuiLuaHost* host_;
    const bool& crt_sse2_conversion_;
    std::vector<GuiLuaRef> stack_;
};

// ---------------------------------------------------------------------------
// Running the page scripts
// ---------------------------------------------------------------------------

// The file and interpreter calls 00B66CA0 and 00B69D40 make. The VFS singleton
// is the one at 0109CEEC that bsp/gui_layout_loader.hpp already names.
struct GuiLuaScriptHost {
    virtual ~GuiLuaScriptHost() = default;

    // 00B66CAA..00B66D16: the singleton's vtable +4h opens the path with mode
    // 2, +18h reports whether it opened, +30h its size and +24h reads it. False
    // for any of those, which makes 00B66CA0 return having done nothing.
    virtual bool read_file(const std::string& path, std::vector<char>& out) = 0;

    // 00B69D8B, 00BDEF90 on the same singleton: the extra files that override
    // or extend this path. 00B69D40 runs each of them after the base file, in
    // the order the search returned.
    virtual std::vector<std::string> override_paths(const std::string& path) = 0;

    // 00A6A160 luaL_loadbuffer(L, data, size, chunk_name). The chunk name is
    // the path's own text, or the empty string at 0108FF2C when it has none.
    // 00B66CA0 does not look at what this returned.
    virtual bool load_buffer(const char* data, std::size_t size,
                             const char* chunk_name) = 0;

    // 00A68090 lua_call(L, 0, LUA_MULTRET). Unprotected, by construction: no
    // lua_pcall appears anywhere on the page-loading path.
    virtual void call_unprotected(int argument_count, int result_count) = 0;
};

// What one chunk did. `fatal` is the case the native build cannot survive:
// load_buffer failed, so the value on the stack is the error message, and the
// call that follows runs anyway and raises through the panic handler.
struct GuiLuaChunkOutcome {
    std::string path;
    bool file_read{false};
    bool compiled{false};
    bool called{false};
    bool fatal{false};
};

// 00B66CA0, __thiscall(state, const NativeString *path, char obfuscated),
// RET 8. `obfuscated` is the second argument; the GUI passes zero, which skips
// the byte transform at 00B66D1F entirely. If set, bytes through the first 01h
// become spaces and subsequent bytes rotate by four bits (no marker: all spaces).
GuiLuaChunkOutcome run_gui_lua_chunk_00b66ca0(GuiLuaScriptHost& host,
                                              const std::string& path,
                                              bool obfuscated);

// 00B69D40, __thiscall(state, const NativeString *path, char obfuscated),
// RET 8. The base path first, then every override the search returns.
std::vector<GuiLuaChunkOutcome> run_gui_lua_file_00b69d40(GuiLuaScriptHost& host,
                                                          const std::string& path,
                                                          bool obfuscated = false);

// The script half of 00AC6600, in call order: interface/_Common.lua, then
// interface/<page>.lua, both through 00B69D40 with the obfuscation flag clear.
// The paths come from gui_page_script_path_00ac5600.
struct GuiScreenScriptRun {
    std::vector<GuiLuaChunkOutcome> chunks;
    bool fatal{false};
};
GuiScreenScriptRun run_gui_screen_scripts_00ac6600(GuiLuaScriptHost& host,
                                                   const std::string& page_name);

// The whole of 00AC6600's script half once the state exists: run the two files,
// take the globals, enter "GuiScreen", hand the reader to the widget walk, and
// leave. `visit` is the screen's vtable +18h, which docs/GUI_LAYOUT_LOADER.md
// covers; it is a callback here because it belongs to that packet.
//
// The native constructor destroys the reader and closes the interpreter before
// it returns, so nothing of the page's Lua state survives the call: the widget
// tree is the only record of the script.
bool load_gui_screen_table_00ac6600(GuiLuaScriptHost& scripts, GuiLuaHost& lua,
                                    const std::string& page_name,
                                    void (*visit)(GuiLuaReader&, void*),
                                    void* context, const bool& crt_sse2_conversion);

}  // namespace bsp
