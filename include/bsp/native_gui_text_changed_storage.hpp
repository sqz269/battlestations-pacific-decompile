#pragma once
#include "bsp/native_gui_text_failure_origin.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual changed Text storage requires MSVC Win32 and x87 inline assembly.
#endif

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
class ActualNativeStringPoolStorage;
struct NativeStringRawPoolContext;
struct NativeGuiTextContentStorageScratch;
class NativeGuiTextGlyphBuffersStorageFrame;
class NativeGuiTextSingleLineStorageFrame;
struct NativeGuiTextGlyphChildrenContext;
struct NativeGuiTextGlyphChildrenAcquired;
struct NativeMeshSectionLayoutStorageContext;
class NativeGuiTextRawModelReference;
struct NativeGuiTextMainSectionStorageContext;
class NativeGuiTextMainSectionStorageFrame;
struct NativeGuiTextMainSectionStorageAcquired;
struct NativeGuiTextShaderStorageContext;
struct NativeGuiTextShaderStorageFrame;

// PREPREPARED borrowed record selections, no new backing/owner or operation.
// Selection inspects metadata identity ONLY; no Native read/callback/allocation.
// The graph and every selected invocation survive nested entry and failure.
struct NativeGuiTextChangedSectionsInvocation {
    NativeGuiTextRawModelReference& reference;
    NativeGuiTextMainSectionStorageContext& context;
    NativeGuiTextMainSectionStorageFrame& frame;
    NativeGuiTextMainSectionStorageAcquired& acquired;
};
struct NativeGuiTextChangedShaderInvocation {
    NativeGuiTextRawModelReference& reference;
    NativeGuiTextShaderStorageContext& context;
    NativeGuiTextShaderStorageFrame& frame;
};
struct NativeGuiTextChangedGlyphChildrenInvocation {
    NativeGuiTextGlyphChildrenContext& context;
    NativeGuiTextGlyphChildrenAcquired& acquired;
};
struct NativeGuiTextChangedLayoutInvocation {
    const NativeMeshSectionLayoutStorageContext& context;
    void* const actual_key_home;
    void** const acquired_before_insertion;
};

// Authenticated original-VA borrowed literals/constants; no copied values or
// new global bank. Prepare before entry and retain the SAME readonly owner.
class NativeGuiTextChangedStorageData final {
public:
    const char* parameter_name(std::size_t index) const;
    const double* height_divisor_00cef1b8() const;
    const volatile std::uint32_t& depth_00ce3800() const;
    const volatile std::uint32_t& depth_00ce69d0() const;
private:
    NativeGuiTextChangedStorageData(const game::GameNativeReadOnlyData&,
        const std::array<const void*, 7>&);
    friend NativeGuiTextChangedStorageData borrow_native_gui_text_changed_data(
        const game::GameNativeReadOnlyData&);
    const game::GameNativeReadOnlyData& owner_;
    const std::array<const void*, 7> views_;
};
NativeGuiTextChangedStorageData borrow_native_gui_text_changed_data(
    const game::GameNativeReadOnlyData&);

// REQUIRED genuine whole callees, with preprepared retained actual contexts and
// fresh operation records for EACH reached/nested invocation. These are source
// transports, not implementations/defaults. Known existing AB8530/AB80C0/
// AB8CE0/AB8400/AB9FD0/B865A0 bodies must be composed at their exact contracts;
// wrapped ABA270 and recursive child updates remain genuine provider obligations.
// No semantic GuiText/Font/Model cast, substitute string/child or mode cap.
struct NativeGuiTextChangedStorageBindings {
    virtual ~NativeGuiTextChangedStorageBindings() = default;
    virtual void localise_00a9fad0(void* current_locale, void* actual_output8,
        const void* captured_narrow_header8) = 0;
    virtual const NativeGuiTextChangedSectionsInvocation& sections_invocation(
        void* captured_text, NativeGuiTextContentStorageScratch&) = 0;
    virtual const NativeGuiTextChangedGlyphChildrenInvocation& glyph_children_invocation(
        void* captured_text, NativeGuiTextContentStorageScratch&) = 0;
    virtual void* geometry_00b74640(void* captured_model, std::uint32_t index,
        NativeGuiTextContentStorageScratch&) = 0;
    virtual void* section_00b732c0(void* captured_mesh, std::uint32_t index) = 0;
    virtual NativeGuiTextGlyphBuffersStorageFrame& glyph_buffers_frame(void* captured_text,
        volatile std::uint32_t (&actual_capacity_mesh_homes)[2], NativeGuiTextContentStorageScratch&) = 0;
    virtual const NativeGuiTextChangedShaderInvocation& shader_invocation(
        void* captured_text, NativeGuiTextContentStorageScratch&) = 0;
    virtual void shader_00b19210(void* captured_material, void* captured_shader) = 0;
    virtual void parameter2_00b18b00(void* captured_material, const void* actual_name8,
        const void* actual_value2) = 0;
    virtual void parameter4_00b18aa0(void* captured_material, const void* actual_name8,
        const void* actual_value4) = 0;
    virtual void parameter1_00b18b20(void* captured_material, const void* actual_name8,
        const void* actual_value1) = 0;
    virtual void clip_00aa9f10(void* captured_text, void* captured_material) = 0;
    virtual const NativeGuiTextChangedLayoutInvocation& layout_invocation(void* captured_section,
        volatile std::uint32_t& actual_mesh_home, NativeGuiTextContentStorageScratch&) = 0;
    virtual NativeGuiTextSingleLineStorageFrame& single_line_frame(void* captured_text,
        volatile std::uint32_t (&actual_utf16_section_homes)[2], NativeGuiTextContentStorageScratch&) = 0;
    virtual void wrapped_00aba270(void* captured_text,
        volatile std::uint32_t (&actual_utf16_section_homes)[2], NativeGuiTextContentStorageScratch&) = 0;
    virtual const volatile std::uint32_t* current_table(void* captured_receiver,
        std::uint32_t captured_profile) = 0; // pure SAME live table, no reentry
    virtual void text_current50(void* captured_text, std::uint32_t captured_profile,
        std::uint32_t captured_target, void* actual_text_plus50) = 0;
    virtual void vertex_00b73bb0(void* captured_mesh, std::uint32_t index, void* captured_stream) = 0;
    virtual void index_00b73b70(void* captured_mesh, void* captured_stream) = 0;
    virtual void texture_00b189f0(void* captured_material, std::uint32_t index, void* captured_texture) = 0;
    virtual void* diffuse_00b179f0(void* captured_material, std::uint32_t index) = 0;
    virtual void model_current2c(void* captured_model, std::uint32_t captured_profile,
        std::uint32_t captured_target, void* actual_float3) = 0;
    virtual void parent_00b6e680(void* captured_node, void* captured_parent) = 0;
    virtual void root_00b6d890(void* captured_node, void* captured_root) = 0;
};

struct NativeGuiTextChangedStorageContext {
    NativeGuiTextChangedStorageBindings& calls;
    ActualNativeStringPoolStorage& wide_strings;
    NativeStringRawPoolContext& raw_strings;
    const NativeGuiTextChangedStorageData& data;
    void* const volatile& actual_locale_00f8bc4c;
    const char* const actual_empty_narrow_00f8be4c;
    const void* const actual_zero_parameter_00f8be54;
    const volatile std::uint32_t& actual_feature_word_0109eea4;
    const NativeGuiTextFailureOriginRuntime& failure_runtime; // SAME qualified own Source catch domain
};

// Ordinary actual DWORD backing, no blanket initialization. The SAME words4/5
// are the narrow parameter header and later scale X/Y; word6 becomes scale Z.
// words0/1 are saved main mesh/material homes, words2/3 the copied wide header.
// Native accesses use volatile views; genuine string helpers access these SAME
// ordinary objects legally. Extra callee homes model reached pushes, not owners.
struct NativeGuiTextContentStorageScratch final {
    std::uint32_t words[7]; // entryESP-28h through -10h
    std::uint32_t buffer_homes[2];
    std::uint32_t builder_homes[2];
    std::uint32_t layout_mesh_home;
    NativeGuiTextFailureOrigin failure_origin;
    NativeGuiTextFailureHeaderArm copied_wide_arm;
    NativeGuiTextFailureOrigin parameter_origins[9]; // nine static Native sites, no input cap
    NativeGuiTextFailureHeaderArm parameter_arms[9];
    NativeGuiTextContentStorageScratch() = default;
    NativeGuiTextContentStorageScratch(const NativeGuiTextContentStorageScratch&) = delete;
    NativeGuiTextContentStorageScratch& operator=(const NativeGuiTextContentStorageScratch&) = delete;
};
struct NativeGuiTextChangedSourceStorageScratch final {
    std::uint32_t wide_headers[4]; // distinct localized[-1C] and widened[-14]
    std::uint32_t inner_utf16_home;
    NativeGuiTextContentStorageScratch inner;
    NativeGuiTextFailureOrigin failure_origin;
    NativeGuiTextFailureHeaderArm localized_arm, widened_arm;
    NativeGuiTextChangedSourceStorageScratch() = default;
    NativeGuiTextChangedSourceStorageScratch(const NativeGuiTextChangedSourceStorageScratch&) = delete;
    NativeGuiTextChangedSourceStorageScratch& operator=(const NativeGuiTextChangedSourceStorageScratch&) = delete;
};

// Compile-time enforcement of the unchanged actual DWORD/home prefixes.
static_assert(offsetof(NativeGuiTextContentStorageScratch, words) == 0);
static_assert(offsetof(NativeGuiTextContentStorageScratch, buffer_homes) == 28);
static_assert(offsetof(NativeGuiTextContentStorageScratch, builder_homes) == 36);
static_assert(offsetof(NativeGuiTextContentStorageScratch, layout_mesh_home) == 44);
static_assert(offsetof(NativeGuiTextContentStorageScratch, failure_origin) == 48);
static_assert(offsetof(NativeGuiTextChangedSourceStorageScratch, wide_headers) == 0);
static_assert(offsetof(NativeGuiTextChangedSourceStorageScratch, inner_utf16_home) == 16);
static_assert(offsetof(NativeGuiTextChangedSourceStorageScratch, inner) == 20);

// WHOLE ABA8D0..ABAEC6 [1527B/451ops/8x87], native ECX captured actual Text,
// one input DWORD home, RET4. FIRST read of input is AFTER wholeAB8530. That
// SAME home later receives shader AL in byte0, full SIGNED font height, then
// FLOAT scale. Do not replace it by const input/bool/unsigned height/snapshot.
void update_native_gui_text_content_storage_00aba8d0(void* captured_actual_text,
    volatile std::uint32_t& actual_utf16_dword_home,
    NativeGuiTextChangedStorageContext&, NativeGuiTextContentStorageScratch&);

// WHOLE ABAED0..ABAFF8 [297B/94ops], native ECX Text, narrow-header/flagDWORD
// stack homes, RET8. Capture input header at entry; read ONLY flag BYTE0, late
// AFTER changed F4 resize/copy. Two actual8B wide temporaries; normal current50
// occurs AFTER normal return. Unchanged input reads no flag/calls no current50.
void submit_native_gui_text_changed_source_storage_00abaed0(void* captured_actual_text,
    const void* captured_narrow_header8, const volatile std::uint32_t& actual_flag_dword_home,
    NativeGuiTextChangedStorageContext&, NativeGuiTextChangedSourceStorageScratch&);

// FIRST negative coupled slice only. No Native issuer exists: host and unknown
// failures retain the actual headers/graph; full Native EH behavior is UNADMITTED.
// Every Scratch is fresh/persistent, never reused after entry, including callbacks
// that swallow an escape. Reader may run ONLY in its qualified own Source catch.
// Full actual receiver/provider/header/cell/record lifetime is caller proof.
// Scratch/context cannot be replaced/reused during entered failure or recursion.
// Distinct retained operations are required at every reached recursive child;
// no depth/count/mode gate, synthetic owner, retry, rollback or FP reset is added.
// Existing wide copy/assign/widen take the genuine ActualNativeStringPoolStorage
// returning-getter domain (its pre-existing noexcept release limitation remains).
// Raw normal/unwind returns use the throwing raw-pool overloads. No new cleanup
// termination policy; Original secondary FH3/SEH/fault/private aliases and binary
// ABI remain unproved. Shared source fragments/current application are untouched;
// whole frontend activation stays gated and all additional body/runtime credit0.
} // namespace bsp
