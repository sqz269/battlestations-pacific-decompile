#pragma once
// Fonts and GUI bring-up of BSP_Application_Initialize.
// Addresses: 0073bae0, 007371d0, 00ac3910, 0053bc00, 00be9620, 00be9760,
//            004c12b0, 00aa5d70, 00aa5e20, 0073c960, 004c14c0.
// Every name below is a hypothesis, not a recovered symbol. Evidence and the
// full call-by-call recovery are in docs/APP_INIT_FONTS_GUI.md. Nothing here is
// binary compatible with the original: native strings, the pooled allocator and
// the vtables are not reproduced.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bsp/font_registry.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 0073bae0 BSP_FontSystem_LoadDefinitions
// ---------------------------------------------------------------------------

// The two literals the native frame builds, 00cfefa8 and 00cff088. The trailing
// separator of the root is a backslash in the image even though the descriptor
// path uses a forward slash.
inline constexpr std::string_view kFontRootPrefix = "Fonts\\";
inline constexpr std::string_view kFontDescriptorPath = "Fonts/Fonts.lua";

// ---------------------------------------------------------------------------
// 00aa5e20 BSP_GuiManager_LoadResources
// ---------------------------------------------------------------------------

// Which native call built the entry. The manager is the ECX of every Group
// call; a Child's ECX is the group that precedes it, not the manager.
enum class GuiResourceKind : std::uint8_t {
    Texture, // renderer *(00f8d394) virtual +0x64
    Group,   // 00aa5840(&name, 1, 0), ECX = manager
    Child,   // 00aa7e00(&name, 1), ECX = the enclosing group
};

// One step of the fixed resource list. Offsets are into the 0x88-byte manager;
// kNotStored marks the `_Highlight` group, which the native code keeps only in
// EDI. second_offset is non-zero only for MousePtrFE_Icon, stored twice.
inline constexpr std::uint16_t kGuiResourceNotStored = 0xFFFF;
struct GuiManagerResource {
    std::string_view name;
    GuiResourceKind kind;
    std::uint16_t offset;
    std::uint16_t second_offset;
    bool sets_visibility; // a virtual +0x34 call follows
    bool visibility;      // its argument
    // MousePtrFE_Icon is the one entry whose +0x34 call the native code issues
    // after the next entry's, at 00aa60a3 rather than between the two
    // creations. The two objects are distinct, so the deferral is preserved
    // here for order fidelity rather than for any observed dependency.
    bool visibility_deferred;
};

// Order, names, lengths and destination offsets read from the disassembly of
// 00aa5e20; six of its blocks are reported unreachable by the decompiler, so the
// literals came from the listing rather than the pseudocode.
inline constexpr std::array<GuiManagerResource, 10> kGuiManagerResources{{
    {"data/interface/textures/whiteGui.tga", GuiResourceKind::Texture, 0x28,
        kGuiResourceNotStored, false, false, false},
    {"interface/textures/common/transparent.tga", GuiResourceKind::Texture, 0x2C,
        kGuiResourceNotStored, false, false, false},
    {"_Mouse", GuiResourceKind::Group, 0x4C, kGuiResourceNotStored, true, true, false},
    {"MousePtrFE_Icon", GuiResourceKind::Child, 0x54, 0x50, true, false, true},
    {"MousePtrGUI_Icon", GuiResourceKind::Child, 0x58, kGuiResourceNotStored, true, false, false},
    {"_Highlight", GuiResourceKind::Group, kGuiResourceNotStored, kGuiResourceNotStored,
        true, true, false},
    {"hl_FrameBox", GuiResourceKind::Child, 0x74, kGuiResourceNotStored, true, false, false},
    {"hlCircle_FrameBox", GuiResourceKind::Child, 0x78, kGuiResourceNotStored, true, false, false},
    {"safezone_43_FrameBox", GuiResourceKind::Child, 0x7C, kGuiResourceNotStored, true, false,
        false},
    {"safezone_169_FrameBox", GuiResourceKind::Child, 0x80, kGuiResourceNotStored, true, false,
        false},
}};

// Byte +0x84, cleared after the last widget.
inline constexpr std::uint16_t kGuiManagerReadyFlagOffset = 0x84;

// ---------------------------------------------------------------------------
// 0073c960, the 0x4040-byte locale-table manager
// ---------------------------------------------------------------------------

// This is the object published in DAT_00f8bc4c, that is the localisation
// manager of docs/APP_INIT_LOCALE.md, not a GUI structure. The 0x4000-byte
// region is its hash bucket array: 0x1000 heads of one 4-byte node pointer.
// bsp::LocaleStringMap in include/bsp/locale_tables.hpp already models the map
// itself; this struct only fixes the byte layout the constructor establishes.
inline constexpr std::size_t kLocaleBucketCount = 0x1000;
inline constexpr std::size_t kLocaleBucketBytes = 0x4000;

// The 16-byte vector object this codebase uses. 00450540 reads first/last/end
// at +4/+8/+0xc of it, and nothing in the constructor chain writes +0, so the
// leading word is left as the allocator found it.
struct NativeVectorImage {
    std::uint32_t allocator; // +0x00, not written by 0073c960
    std::uint32_t first;     // +0x04
    std::uint32_t last;      // +0x08
    std::uint32_t end;       // +0x0c
};
static_assert(sizeof(NativeVectorImage) == 0x10);

// Native {size, data} string pair.
struct NativeStringImage {
    std::uint32_t size;
    std::uint32_t data;
};
static_assert(sizeof(NativeStringImage) == 8);

struct LocaleTableManagerImage {
    std::uint32_t vtable;              // +0x0000, 00cff140 after 0073c960
    NativeVectorImage table_names;     // +0x0004, registered table names
    std::uint32_t buckets[kLocaleBucketCount]; // +0x0014
    std::uint32_t entry_count;         // +0x4014
    NativeStringImage language_name;   // +0x4018
    NativeVectorImage lanx_a;          // +0x4020
    NativeVectorImage lanx_b;          // +0x4030
};
static_assert(sizeof(LocaleTableManagerImage) == 0x4040);
static_assert(offsetof(LocaleTableManagerImage, table_names) == 0x0004);
static_assert(offsetof(LocaleTableManagerImage, buckets) == 0x0014);
static_assert(offsetof(LocaleTableManagerImage, entry_count) == 0x4014);
static_assert(offsetof(LocaleTableManagerImage, language_name) == 0x4018);
static_assert(offsetof(LocaleTableManagerImage, lanx_a) == 0x4020);
static_assert(offsetof(LocaleTableManagerImage, lanx_b) == 0x4030);

// Vtable constants of the two-step construction. 00736540 stores the singleton
// base vtable and publishes the object in DAT_00f8bc4c under the lifetime
// manager's lock; 0073c960 then overwrites the slot with the derived vtable.
inline constexpr std::uint32_t kLocaleManagerBaseVtable = 0x00CFEA5Cu;    // via 00736540
inline constexpr std::uint32_t kLocaleManagerDerivedVtable = 0x00CFF140u; // 0073c960

// 0073c960: __fastcall, ECX = the object, returns it, RET. Writes exactly the
// fields the native constructor writes and leaves the three vector allocator
// words alone, so a caller can see which bytes the native code inherits from
// the 00bf681b(0x4040) block.
void construct_locale_table_manager_0073c960(LocaleTableManagerImage& image) noexcept;

// ---------------------------------------------------------------------------
// 004c14c0, the 4-byte diagnostic singleton
// ---------------------------------------------------------------------------

// A lazily created object whose only member is its vtable pointer 00ce752c, a
// one-slot table holding the scalar deleting destructor 004bbca0. Every one of
// its twenty call sites discards the result, so construction plus lifetime
// registration is the whole observable behaviour; the reporting method the call
// sites were presumably written against is not in the shipped image.
struct DiagnosticSinkSingleton {
    bool constructed{false};
    bool registered_with_lifetime_manager{false};
    std::uint32_t vtable{0};
    std::size_t construction_count{0}; // must never exceed 1
};
inline constexpr std::uint32_t kDiagnosticSinkVtable = 0x00CE752Cu;
inline constexpr std::size_t kDiagnosticSinkSizeBytes = 4;

// 004c14c0: __cdecl, no arguments, RET, returns DAT_0109cf14. The native double
// check is under the lifetime manager's optional critical section; the lock is
// not modelled, only the once-only construction it protects.
void* diagnostic_sink_get_or_create_004c14c0(DiagnosticSinkSingleton& state) noexcept;

// ---------------------------------------------------------------------------
// The startup sequence itself
// ---------------------------------------------------------------------------

// Integration boundary for the subsystems 0073bae0 reaches. One method per
// native call site, in call order. There are no default implementations:
// nothing here stands in for unrecovered game behaviour.
struct GuiStartupHost {
    virtual ~GuiStartupHost() = default;

    // 008d4890, ECX = the settings object 00f88980. Empty for the shipped
    // English build, whose language descriptor has no fontpath key.
    virtual std::string language_font_path() = 0;

    // 007371d0, the font registry singleton over DAT_00f8bf44. Takes no
    // arguments despite what the decompiler shows at the 0073bae0 call site.
    virtual FontRegistry& font_registry() = 0;

    // 00ac3910, __thiscall with ECX = the registry and three stack arguments,
    // RET 0xc. Already reconstructed in src/font_registry.cpp; the host decides
    // whether to route this to load_font_registry_lua or to a real Lua host.
    virtual bool load_font_descriptors(FontRegistry& registry, std::string_view root,
        std::string_view descriptor, std::string_view language_font_path) = 0;

    // 0053bc00 then 00be9620. The adjusted pointer is discarded by the native
    // code, so this only has to force the 0x204-byte singleton into existence.
    virtual void preload_fallback_glyph_table() = 0;

    // 004c12b0, the 0x88-byte GUI manager singleton over DAT_00f8bc5c,
    // constructed by 00aa5d70. Returns an opaque handle: the manager's layout
    // is only partly recovered.
    virtual void* gui_manager() = 0;

    // The steps of 00aa5e20, in list order. `parent` is null for a Texture or a
    // Group and is the most recent Group's result for a Child. The return value
    // is stored at the entry's offsets; a Texture entry that the VFS rejects
    // must return null and nothing is stored.
    virtual void* create_gui_resource(void* manager, void* parent,
        const GuiManagerResource& entry) = 0;

    // The two stores of a resource entry, and the virtual +0x34 call.
    virtual void store_gui_resource(void* manager, std::uint16_t offset, void* value) = 0;
    virtual void set_gui_resource_visibility(void* resource, bool visible) = 0;

    // Byte +0x84 = 0, the last write of 00aa5e20.
    virtual void clear_gui_manager_ready_flag(void* manager) = 0;
};

// What run_gui_startup observed, so a caller can assert the order without the
// host having to record it.
struct GuiStartupResult {
    std::string language_font_path;
    bool font_descriptors_loaded{false};
    void* gui_manager{nullptr};
    std::size_t resources_created{0}; // entries whose factory returned non-null
    std::size_t stores{0};            // including the second MousePtrFE_Icon store
};

// 0073bae0: __cdecl void f(void), RET, no return value. The native body also
// wraps the three frame strings in an SEH scope (handler 00c867f8) and frees
// them through the sized storage pool; neither the unwind path nor the pooled
// allocator is modelled. The GUI manager is created here, before the
// `After InitGui` checkpoint at 0073e146, not after it.
GuiStartupResult run_gui_startup(GuiStartupHost& host);
}
