#pragma once
// GUI page loading: how a page name such as "FE_initial" becomes a widget tree.
// Addresses: 00aa5840 (page factory), 00ac6600 (screen construction, read only),
//            00ac5600 (script path), 00aa2490 (key suffix -> type id),
//            00aa6560 (type id -> constructor), 00aaa710 (property reader and
//            child builder), 00aa7e00 (named child search), 00aa52a0 (register),
//            00aa3140 (find a registered page).
// Every name below is a hypothesis, not a recovered symbol. The evidence is in
// docs/GUI_LAYOUT_LOADER.md. Nothing here is binary compatible with the
// original: the native widget is a vtable object with an intrusive std::list of
// children and a scene node, none of which are reproduced.
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bsp/gui_widget.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The widget type table
// ---------------------------------------------------------------------------

// The switch of 00AA6560, cases 1..12h. The names are the literals 00AA2490
// compares against, so only the value 1 is unnamed: it is the page root that
// 00AA5840 builds directly through 00AC6600, and no key suffix produces it.
enum class GuiWidgetType : std::int32_t {
    None = 0,       // 00AA2490 fell through: the key is a property, not a child
    Screen = 1,     // 00AA3BD0, the page root; reached only from 00AA5840
    Group = 2,      // 00AA12F0
    Text = 3,       // 00AA1380
    Line = 4,       // 00AA1530
    Progbar = 5,    // 00AA14A0
    Icon = 6,       // 00AA1410
    Scrollbar = 7,  // 00AA15C0
    Grid = 8,       // 00AA1650
    Model = 9,      // 00AA16E0
    Movie = 10,     // 00AA1770
    Listbox = 11,   // the singleton factory at 00F8BC10, not a fixed size
    AnimIcon = 12,  // 00AA2340
    Curve = 13,     // 00AA1800
    Sound = 14,     // 00AA1890
    Button = 15,    // 00AA4440
    ClipBox = 16,   // 00AA1920
    Section = 17,   // 00AA19B0
    FrameBox = 18,  // 00AA1A40
};

// One row of the reconstructed type table. `instance_size` is the immediate the
// constructor stub loads into ECX before its allocator call, read from the
// bytes at stub+19h; it is 0 for Listbox, which routes through the factory
// singleton at 00F8BC10 instead of allocating a fixed block.
struct GuiWidgetClass {
    GuiWidgetType type;
    std::string_view suffix;         // the literal 00AA2490 compares
    std::uint32_t instance_size;     // bytes, 0 when not a fixed allocation
    std::uint32_t constructor_stub;  // the 00AA6560 case target
};

// The seventeen rows in the order 00AA2490 tests them, which is not the order
// of the type ids. Screen (1) is absent: no key produces it.
const GuiWidgetClass* gui_widget_class_table(std::size_t& count) noexcept;

// Null for GuiWidgetType::None and for Screen, which the table does not carry.
const GuiWidgetClass* gui_widget_class(GuiWidgetType type) noexcept;

// 00AA2490, __fastcall(ECX = the key string), RET. Takes the substring after
// the last '_' (00CE7890), or the whole key when there is none, and compares it
// case-insensitively against the seventeen literals. Returns None when nothing
// matches, which is how a property key is told from a child key.
GuiWidgetType gui_widget_type_for_key_00aa2490(std::string_view key) noexcept;

// ---------------------------------------------------------------------------
// Where a page name resolves
// ---------------------------------------------------------------------------

// 00AA5840 builds "<name>.mmod" (00D0E67C) and asks the VFS singleton at
// 0109CEEC whether it exists. The name is used unchanged, with no directory.
std::string gui_page_model_path_00aa5840(std::string_view page_name);

// 00AC5600 builds "interface/" (00D5CB18) + name + ".lua" (00CFD2C8).
std::string gui_page_script_path_00ac5600(std::string_view script_name);

// The two script names 00AC6600 loads, in order: the shared library first, then
// the page itself. The global table it then reads is kGuiScreenTableName.
inline constexpr std::string_view kGuiCommonScriptName = "_Common";   // 00D5CB64
inline constexpr std::string_view kGuiScreenTableName = "GuiScreen";  // 00D5CB20

// ---------------------------------------------------------------------------
// The value model the visitor passes
// ---------------------------------------------------------------------------

// The first dword of every 8-byte variant 00AAA710 pushes. The name of a
// property is always tag 0; the field and the default carry the field's tag.
// The gaps (1, 4, 7) are not used by the base widget and were not recovered.
enum class GuiValueTag : std::int32_t {
    String = 0,  // char*: the property name, and WideScreenAlign's value
    Float = 2,   // one float:  Rotate, BlendFactor
    Bool = 3,    // one byte:   Visible, MouseHit, MouseBlock
    Vec3 = 5,    // three floats: Pos
    Vec2 = 6,    // two floats:   Pivot, Size, Scale
    Color = 8,   // four floats:  Color, LowColor, HighColor
};

struct GuiTable;

// A value of an evaluated page table. Lua numbers are doubles, so the widths
// below are the script's, not the field's; the binder narrows on assignment.
class GuiValue {
public:
    enum class Kind { Nil, Boolean, Number, String, Table };

    GuiValue() = default;
    explicit GuiValue(bool value) : kind_(Kind::Boolean), boolean_(value) {}
    explicit GuiValue(double value) : kind_(Kind::Number), number_(value) {}
    explicit GuiValue(std::string value)
        : kind_(Kind::String), string_(std::move(value)) {}
    explicit GuiValue(std::shared_ptr<const GuiTable> value)
        : kind_(Kind::Table), table_(std::move(value)) {}

    Kind kind() const noexcept { return kind_; }
    bool is_table() const noexcept { return kind_ == Kind::Table; }
    bool boolean() const noexcept { return boolean_; }
    double number() const noexcept { return number_; }
    const std::string& string() const noexcept { return string_; }
    const GuiTable* table() const noexcept { return table_.get(); }

private:
    Kind kind_{Kind::Nil};
    bool boolean_{false};
    double number_{0.0};
    std::string string_;
    std::shared_ptr<const GuiTable> table_;
};

// An evaluated Lua table. `named` keeps insertion order because the native
// visitor's key enumeration (vtable +18h) hands 00AAA710 an ordered array and
// the widget list it builds is walked in that order afterwards; a real Lua
// table has no such order, so the order here is the script's, which is a
// deliberate simplification. `array` holds the integer-keyed entries, which
// 00AAA710 skips: only entries whose tag is 0 become children.
struct GuiTable {
    std::vector<std::pair<std::string, GuiValue>> named;
    std::vector<GuiValue> array;

    const GuiValue* find(std::string_view key) const noexcept;
};

// ---------------------------------------------------------------------------
// The static page-table parser
// ---------------------------------------------------------------------------

// The shipped pages are Lua source, not a private format: 00AC6600 runs them
// through the interpreter and reads the global table afterwards. This parser
// covers only the assignment-and-table-constructor subset, which is what 69 of
// the 96 shipped pages use; the rest call helpers defined in interface/
// _common.lua and need a real evaluator. It is not a Lua implementation and
// makes no attempt to be one.
//
// Accepted:
//   chunk  := stmt*
//   stmt   := Name ('[' key ']')* '=' value
//   value  := table | string | number | 'true' | 'false' | 'nil'
//   table  := '{' (field ((',' | ';') field)* (',' | ';')?)? '}'
//   field  := '[' key ']' '=' value | Name '=' value | value
// Only statements whose root name is kGuiScreenTableName reach `out`; a bare
// `GuiScreen = { ... }` replaces the table and a subscripted assignment writes
// one path into it, which is how the shipped pages mix the two forms.
bool parse_gui_page_table(
    std::string_view text, GuiTable& out, std::string& error);

// ---------------------------------------------------------------------------
// The widget tree
// ---------------------------------------------------------------------------

// A widget as this packet models it. The native object is one of eighteen
// classes; here the class is a tag and the recovered base fields live in
// `transform`, the projection from bsp/gui_widget.hpp. `source` keeps the
// sub-table so a caller can read the per-class properties this packet did not
// recover (States, Font, Align, ...) without reparsing.
struct GuiLayoutWidget {
    std::string key;  // the Lua key, which is also the scene node's name
    GuiWidgetType type{GuiWidgetType::None};
    GuiWidgetTransform transform{};
    const GuiTable* source{nullptr};
    GuiLayoutWidget* parent{nullptr};  // widget +70h
    std::vector<std::unique_ptr<GuiLayoutWidget>> children{};  // widget +68h
    std::uint32_t node_id{0};  // widget +4Ch, what the host handed back

    // The descriptor fields GuiWidgetTransform does not carry. Kept here rather
    // than added to that projection, which belongs to another packet.
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};        // +50h, default 00F8BCE0
    float low_color[4]{0.0f, 0.0f, 0.0f, 1.0f};    // +A4h, default 00F8BCCC
    float high_color[4]{1.0f, 1.0f, 1.0f, 1.0f};   // +B4h, default 00F8BCE0
    float blend_factor{0.0f};                      // +C4h
    bool visible{false};                           // +E4h, default false
};

// A loaded page. `priority` is widget +FCh, the "Priority" key, and the only
// field the registration order depends on.
struct GuiLayoutPage {
    std::string name;
    std::int32_t priority{0};
    // Page +4h, the count 00AA5840 raises through 00CE221C when a caller asks
    // for an already-loaded page with the flag set. Nothing here releases it.
    std::int32_t reference_count{1};
    bool model_backed{false};  // the "<name>.mmod" branch was taken
    bool script_evaluated{false};
    std::unique_ptr<GuiLayoutWidget> root{};

    // The evaluated table, held so every widget's `source` stays valid. The
    // native screen keeps no such reference: it copies what it needs and lets
    // the Lua state collect the table.
    std::shared_ptr<const GuiTable> script_table{};
};

// 00AA7E00, __thiscall(parent, const NativeString* name), RET 8. Walks the
// parent's direct children only, skips any child whose scene node pointer
// (+4Ch) is null, and compares the node's name with __stricmp. The second stack
// argument the callers pass (always 1) is consumed by the RET and never read,
// so there is no recursion in this build despite what the call sites suggest.
GuiLayoutWidget* find_child_by_name_00aa7e00(
    GuiLayoutWidget& parent, std::string_view name) noexcept;

// The recursive form the screens actually want. It is NOT what 00AA7E00 does;
// it is provided because several screens reach nested widgets by chaining
// lookups, and having both makes the difference explicit at the call site.
GuiLayoutWidget* find_descendant_by_name(
    GuiLayoutWidget& parent, std::string_view name) noexcept;

// ---------------------------------------------------------------------------
// The property binding
// ---------------------------------------------------------------------------

// One row of the descriptor list 00AAA710 walks, in call order. `offset` is the
// widget field's byte offset; `name_literal` is the string address pushed with
// tag 0. Exposed as data so the doc's table and the binder cannot drift apart.
struct GuiPropertyDescriptor {
    std::string_view name;
    GuiValueTag tag;
    std::uint32_t offset;        // widget field offset
    std::uint32_t name_literal;  // the .rdata address of `name`
};

const GuiPropertyDescriptor* gui_base_property_descriptors(
    std::size_t& count) noexcept;

// 00AAA710's first half, __thiscall(widget, visitor), RET 4: each descriptor is
// read out of `table` into the widget, defaulting to the value the native frame
// stages when the key is absent. Then the wide-screen X fixup runs against the
// authored X cached at +8h, and "Visible" is read only when the widget is not
// the page root (vtable +5Ch != 1). "MouseHit" is read only when "MouseBlock"
// came back false; a true MouseBlock forces MouseHit to true without a lookup.
void bind_widget_properties_00aaa710(
    const GuiTable& table, GuiLayoutWidget& widget, bool is_page_root,
    bool widescreen_enabled) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site that leaves this packet. There are no default
// implementations for the recovered calls: nothing here stands in for
// unrecovered behaviour.
struct GuiLayoutHost {
    virtual ~GuiLayoutHost() = default;

    // 00AA58C3: the VFS singleton at 0109CEEC, 00BDF4C0, with "<name>.mmod".
    virtual bool vfs_name_exists(const std::string& path) = 0;

    // 00AA58CC..00AA58F5: 004C1400 for the resource manager, 00B80D70 to fetch
    // the model, then the resource's vtable +8h called with (0, 1.0f). The
    // result is stored at manager +30h and its +0Ch becomes the page's model.
    // Returns false when 00B80D70 hands back null, which leaves the page's
    // model null but does not stop the load.
    virtual bool instantiate_page_model(const std::string& path) = 0;

    // 00AC6600's script half: run interface/_Common.lua, then
    // interface/<page>.lua, then read the global table "GuiScreen". Null means
    // the script did not produce one, which the native build treats as an empty
    // page rather than a failure.
    virtual std::shared_ptr<const GuiTable> evaluate_page_script(
        const std::string& page_name) = 0;

    // 00AAAD8E..00AAADB1: 00B74EB0 allocates 184h bytes and 00B75030 builds the
    // widget's scene node from the key. The native code then clears the low two
    // bits of node +138h. Returning 0 means the allocation failed, which makes
    // 00AAA710 skip the child entirely.
    virtual std::uint32_t create_widget_node(const std::string& key) = 0;

    // 00AAAE24: 00B6E680, child node +4Ch under parent node +4Ch.
    virtual void set_node_parent(std::uint32_t child, std::uint32_t parent) = 0;

    // 00AA8750, through the wide-screen fixup 00AAA710 performs inline.
    virtual bool widescreen_enabled() = 0;

    // The child's own vtable +74h and +78h, called either side of the descend.
    // They have no recovered behaviour, so they default to nothing.
    virtual void on_widget_constructed(GuiLayoutWidget&) {}
    virtual void on_widget_loaded(GuiLayoutWidget&) {}
};

// 00AAA710's second half: the visitor's key enumeration (vtable +18h) followed
// by the child loop. Every string key whose suffix names a type becomes a child
// of `widget`: the type's constructor runs (00AA6560), a scene node is created
// and parented, the child is appended to the parent's list and its parent
// pointer set, then the visitor descends (vtable +4h), the child's own property
// enumerator runs (its vtable +18h, which reaches this function again) and the
// visitor ascends (vtable +8h). Integer-keyed entries are skipped.
void build_widget_children_00aaa710(
    const GuiTable& table, GuiLayoutWidget& widget, GuiLayoutHost& host);

// ---------------------------------------------------------------------------
// The manager's page list
// ---------------------------------------------------------------------------

// The std::vector<GuiScreen*> at GUI manager +14h: begin +18h, end +1Ch,
// capacity +20h. It is the only page list found, and the only order it carries
// is the Priority order the registration keeps.
class GuiPageRegistry {
public:
    // 00AA3140, __thiscall(manager, const NativeString* name), RET 4. Linear
    // scan calling each page's vtable +7Ch for its name and comparing with
    // __stricmp. Null when nothing matches.
    GuiLayoutPage* find_00aa3140(std::string_view name) noexcept;

    // 00AA52A0, __thiscall(manager, GuiScreen* page), RET 4. Inserts before the
    // first page whose +FCh is strictly greater than the new page's, so the
    // vector stays sorted by Priority ascending and equal priorities keep
    // insertion order. Returns a borrowed pointer to the stored page.
    GuiLayoutPage* register_00aa52a0(std::unique_ptr<GuiLayoutPage> page);

    const std::vector<std::unique_ptr<GuiLayoutPage>>& pages() const noexcept {
        return pages_;
    }

private:
    std::vector<std::unique_ptr<GuiLayoutPage>> pages_{};
};

// 00AA5840 as a whole, __thiscall(manager, const NativeString* name, int
// unknown, char add_reference), RET 0Ch.
//
// The second stack argument reaches 00AC6600 and is stored at screen +120h; the
// screens pass 1 and the loading screen passes 0, and what it selects was not
// recovered, so it is carried through rather than interpreted. The third is the
// add-a-reference flag, read only on the already-loaded path, where the native
// build does InterlockedIncrement on page +4h through 00CE221C.
//
// The order is: look the name up in the registry; if it is there, optionally
// add a reference and return it. Otherwise ask the VFS for "<name>.mmod" and
// instantiate it when it exists; build the page; evaluate its script; bind the
// root's properties and build its children; register it.
GuiLayoutPage* load_gui_page_00aa5840(
    GuiPageRegistry& registry, GuiLayoutHost& host, const std::string& name,
    std::int32_t screen_flag, bool add_reference);

}  // namespace bsp
