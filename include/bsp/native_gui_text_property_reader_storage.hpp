#pragma once
#include "bsp/native_gui_widget_property_children.hpp"
#include "bsp/native_gui_text_field_storage.hpp"
#include "bsp/native_gui_text_changed_storage.hpp"
#include "bsp/native_gui_text_shadow_storage.hpp"
#include "bsp/native_lua_reader_fields.hpp"
#include "bsp/object_handle_resolvers.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
namespace game { class GameNativeReadOnlyData; class GameNativeGuiTextState; }
class NativeCrtStricmpCallees;

// Cold authentic RO address loans; no copied strings or sampled constant values.
class NativeGuiTextPropertyReaderLiterals final {
public:
    explicit NativeGuiTextPropertyReaderLiterals(const game::GameNativeReadOnlyData&);
    const char* key(std::uint32_t index) const;
    const volatile std::uint32_t* table(std::uint32_t captured_profile) const;
private:
    const game::GameNativeReadOnlyData& data_;
    const char* const keys_[26];
};

// Two genuine arrays. Default calls use key18/field10/fallback08; required
// calls use key10/field08. The last array also backs one-key/preset/ABAED0 and
// footer pushes. No fabricated reference to a subarray is used.
struct NativeGuiTextPropertyReaderCallHomes final {
    std::uint32_t key_pair_18[2];
    std::uint32_t actual_homes_10[4];
};
static_assert(offsetof(NativeGuiTextPropertyReaderCallHomes, actual_homes_10) == 8);
static_assert(sizeof(NativeGuiTextPropertyReaderCallHomes) == 24);

// Construct this ordinary, stable backing FIRST. It seeds no Native word.
// Prepare the genuine raw Font frame against this exact font_34 header before
// Native effects/Text birth, together with its pre-pool shader/reference graph.
struct NativeGuiTextPropertyReaderHomes final {
    NativeGuiTextPropertyReaderHomes() {}
    NativeGuiTextPropertyReaderHomes(const NativeGuiTextPropertyReaderHomes&) = delete;
    NativeGuiTextPropertyReaderHomes& operator=(const NativeGuiTextPropertyReaderHomes&) = delete;
    NativeGuiTextPropertyReaderHomes(NativeGuiTextPropertyReaderHomes&&) = delete;
    NativeGuiTextPropertyReaderHomes& operator=(NativeGuiTextPropertyReaderHomes&&) = delete;
    NativeGuiTextPropertyReaderCallHomes calls;
    std::uint32_t metadata_10, align_14[2], vertical_1c[2], default_text_24[2];
    std::uint32_t shadow_pos_2c[2], font_34[2], state_44;
};

class NativeGuiTextPropertyReaderStorageFrame;
// Genuine initialized/unbound Lua preimages, separate from Native homes. The
// caller supplies opaque/padding bytes under the existing field/query APIs;
// neither the reader nor a value-initialized Frame invents those preimages.
struct NativeGuiTextPropertyReaderLuaScratch final {
    NativeLuaReaderFieldScratch field;
    NativeLuaObjectStorage query, enter;
    NativeLuaFieldResolverFrame resolver;
};
// Required genuine implementations for OTHER captured targets only. Each
// operation honors the passed capture/actual pair backing and retains its own
// real frames/graph through unknown escape. No numeric codeword invocation,
// current-slot recapture, semantic reader or successful default is supplied.
class NativeGuiTextPropertyReaderOtherCalls {
public:
    virtual ~NativeGuiTextPropertyReaderOtherCalls() = default;
    virtual void field_10(void* visitor, std::uint32_t profile, std::uint32_t target,
        std::uint32_t tag, std::uint32_t bits, void* actual_field8,
        NativeGuiTextPropertyReaderStorageFrame&) = 0;
    virtual void default_0c(void* visitor, std::uint32_t profile, std::uint32_t target,
        std::uint32_t tag, std::uint32_t bits, void* actual_field8,
        const void* actual_fallback8, NativeGuiTextPropertyReaderStorageFrame&) = 0;
    virtual std::uint32_t query_14(void* visitor, std::uint32_t profile, std::uint32_t target,
        std::uint32_t tag, std::uint32_t bits, NativeGuiTextPropertyReaderStorageFrame&) = 0;
    virtual void enter_04(void* visitor, std::uint32_t profile, std::uint32_t target,
        std::uint32_t tag, std::uint32_t bits, NativeGuiTextPropertyReaderStorageFrame&) = 0;
    virtual void leave_08(void* visitor, std::uint32_t profile, std::uint32_t target,
        NativeGuiTextPropertyReaderStorageFrame&) = 0;
};

class NativeGuiTextPropertyReaderStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        std::uint32_t active_site{}, escaped_site{}, captured_profile{}, captured_target{};
        void* actual_text{}; void* captured_visitor{};
        bool base_entered{}, base_returned{}, font_entered{}, font_returned{};
        bool source_entered{}, source_returned{};
        std::exception_ptr failure;
    };
    // Address-only loans. Font frame/acquisition must be distinct fresh records
    // already bound to homes.font_34 and their SAME reserved shader/reference.
    // base is the already initialized genuine AAA710 preimage, not zero scratch.
    NativeGuiTextPropertyReaderStorageFrame(NativeGuiTextPropertyReaderHomes& homes,
        NativeGuiWidgetPropertyScratch& base, NativeGuiTextPropertyReaderLuaScratch& lua,
        NativeGuiTextRawFontNameStorageFrame& font,
        NativeGuiTextRawFontNameStorageAcquired& acquisition,
        NativeGuiTextChangedSourceStorageScratch& changed)
        : homes_(homes), base_(base), lua_(lua), font_(font),
          font_acquisition_(acquisition), changed_(changed) {}
    NativeGuiTextPropertyReaderStorageFrame(const NativeGuiTextPropertyReaderStorageFrame&) = delete;
    NativeGuiTextPropertyReaderStorageFrame& operator=(const NativeGuiTextPropertyReaderStorageFrame&) = delete;
    NativeGuiTextPropertyReaderStorageFrame(NativeGuiTextPropertyReaderStorageFrame&&) = delete;
    NativeGuiTextPropertyReaderStorageFrame& operator=(NativeGuiTextPropertyReaderStorageFrame&&) = delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void read_native_gui_text_properties_storage_00abb630(void*,
        volatile std::uint32_t&, NativeGuiTextPropertyReaderStorageFrame&,
        struct NativeGuiTextPropertyReaderStorageContext&);
    NativeGuiTextPropertyReaderHomes& homes_;
    NativeGuiWidgetPropertyScratch& base_;
    NativeGuiTextPropertyReaderLuaScratch& lua_;
    NativeGuiTextRawFontNameStorageFrame& font_;
    NativeGuiTextRawFontNameStorageAcquired& font_acquisition_;
    NativeGuiWidgetPropertyChildrenAcquired children_;
    NativeGuiTextChangedSourceStorageScratch& changed_;
    NativeGuiTextShadowStorageFrame shadow_;
    Observation observation_;
};

struct NativeGuiTextPropertyReaderStorageContext {
    NativeLuaReaderStorage& reader;
    NativeLuaFieldValueBindings& fields;
    NativeLuaFieldResolverAdapter& resolvers;
    NativeGuiWidgetPropertyInteractionContext& base_interaction;
    NativeGuiWidgetPropertyChildrenRawContext& base_children;
    NativeGuiTextRawModelReference& text_reference;
    NativeGuiTextRawFontNameStorageContext& font_names;
    NativeGuiTextChangedStorageContext& changed;
    const NativeGuiTextShadowStorageContext& shadow;
    game::GameNativeGuiTextState& globals;
    const NativeGuiTextPropertyReaderLiterals& literals;
    const volatile std::int32_t& locale_changed_0109de1c;
    NativeCrtStricmpCallees& stricmp_callees;
    NativeGuiTextPropertyReaderOtherCalls& other;
};

// Whole NORMAL [00ABB630,00ABBE45),2069B/586 starts. Native ECX actual1F4
// Text + visitor DWORD/RET4 becomes this ordinary Source API. Capture the
// visitor once before whole AAA710. DefaultShadow writes the ORIGINAL incoming
// ordinary mutable HOME, borrowed via volatile view; do not restore/snapshot
// it or replace it with an integer. It is separate from Frame/homes/controls.
//
// Real raw Font/reference/shader/publication and changed-source contexts must
// already be retained/fresh before effects. The helpers keep their existing
// Source/failure domains; no semantic owner cast, pending-content success,
// extra color50, Native cleanup/rollback/pop/retry or Origin issuer is added.
// Unknown Source escape retains all five headers, reached records, reader
// depth and the complete transitive graph OUTSIDE unwind with the caller.
//
// Caller proves genuine births, physical C++ storage/alias domains, SAME live
// RO tables/strings/CRT V3/resolver/color/Font/model services and last use.
// Direct BF7FBF uses supplied actual services; 425850 keeps its documented
// actual-header/host CRT current-locale domain, not an injected CRT theorem.
// Other current targets need real typed providers. Page Font/scalar/queued
// borrower cuts/all nine gates remain external. No Original ABI/FLAGS/private
// homes/FH3/SEH/fault/NoThrow/Origin/unmasked FP/whole-object/runtime proof.
void read_native_gui_text_properties_storage_00abb630(void* actual_text1f4,
    volatile std::uint32_t& actual_visitor_home, NativeGuiTextPropertyReaderStorageFrame&,
    NativeGuiTextPropertyReaderStorageContext&);
} // namespace bsp
