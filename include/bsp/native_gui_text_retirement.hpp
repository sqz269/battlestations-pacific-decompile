#pragma once
#include "bsp/native_gui_text_storage_construct.hpp"
#include "bsp/native_gui_text_glyph_children.hpp"
#include "bsp/gui_text_factory.hpp"
#include <array>

namespace bsp {
struct NativeGuiTextRetirementContext {
    // SAME existing85/93 services, current import and cache profile views.
    NativeGuiTextSectionStorageContext& resources;
    NativeGuiWidgetLifetimeContext& base;
    NativeGuiTextGlyphChildrenContext& glyphs;
    NativeGuiTextPool& actual_pool_00f8bdf0;
};
struct NativeGuiTextRetirementShader {
    void* identity{}; // diagnostic after terminal, never another owner
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    std::uint32_t captured_profile{}, captured_target{};
    bool decrement_entered{}, decremented{}, zero_entered{}, zero_returned{}, slot_cleared{};
};
struct NativeGuiTextRetirementString {
    void* captured_data{}; // may dangle after the consumed pool return
    std::uint32_t captured_bytes{};
    bool data_captured{}, length_captured{}, getter_entered{}, getter_returned{};
    bool return_entered{}, returned{};
};
struct NativeGuiTextRetirementAcquired {
    // Public trivially destructible diagnostics: no ownership, implicit cleanup
    // or uneditable capability. Never reset/assign/replay a reached invocation.
    enum class Boundary { none, provider_call, host_metadata };
    bool started{}, complete{}, exception_cleanup_started{};
    bool shadow_slot_cleared{}, vector_entered{}, vector_returned{};
    bool base_entered{}, base_returned{};
    volatile std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{}, disarmed_cleanup_actions{};
    std::uint32_t normal_actions_entered{}, normal_actions_returned{}, unwind_actions_consumed{};
    // SAME state-bit numbering0..6. Consumed/disarmed is recorded BEFORE the
    // unwind helper; returned is recorded only AFTER that helper returns.
    // A returned null-data cleanup need not have called a pool return.
    std::uint32_t unwind_actions_returned{};
    Boundary boundary{Boundary::none};
    void* destination{};
    NativeGuiTextRetirementShader shader;
    void* captured_shadow{};
    NativeNodeTreeRetirementAcquired shadow;
    NativeGuiTextGlyphChildrenAcquired glyphs;
    // Native reverse member order:1C8,1C0,1A4,F4,EC. Header/data aliases only.
    std::array<NativeGuiTextRetirementString, 5> strings;
};
struct NativeGuiTextScalarRetirementAcquired {
    bool started{}, complete{}, destructor_entered{}, destructor_returned{};
    bool flags_captured{}, pool_return_entered{}, pool_returned{};
    std::uint8_t captured_flags_byte{};
    NativeGuiTextRetirementAcquired destruction;
};

// Whole AB8250[384B/113starts], Native ECX actual Text/no args/RET. The new
// Source interface is not Native FH3/SEH/private stack/register ABI. SAME1F4
// payload/actual+04 is used; no semantic GUI owner cast, extra count or owner.
//
// Shader zero supports the established SAME canonical material-effect base
// D5E534 and derivedD61A00 profiles, with their CURRENT numeric slot0 BD30E0
// and real existing actual-count terminal. Unknown current profiles/targets
// are required provider boundaries, never successful defaults. CE2220 is
// loaded at the reached decrement site, not snapshotted on raw entry.
// Profile pointers are the SAME existing live canonical aliases in resources'
// effect-cache/creator graph, never copies or query-only numeric arrays.
// Shadow/current113/base dispatch require their genuine providers. Nonempty
// raw Text recursive AB8EE0->AB8250 binding remains external, no activation.
//
// Known-region preflight precedes entry. Caller additionally holds the WHOLE
// independent85/93 creator/context/registry/profile/backing graph and distinct
// prepared recursive frames BEFORE callbacks/unwind; it is disjoint from
// writable diagnostics and stays live through failure disposition. Borrowed
// service/context identities remain stable; pure metadata lookups cannot
// reenter or mutate state. Genuine reached calls retain their actual raw
// mutations and may replace raw Text fields as Original does. Pool/slab/trailer
// provenance is the SAME actual creator's caller binding, not inferred here.
//
// DATA is captured before state consumption, then CURRENT length only when
// nonnull; no header clear. Real5D6CF0 runs while state3, no normalstate2.
// Exception projection consumes each reverse action before its real call;
// cleanup escape terminates. No shader/shadow rollback, retry or pool return.
// A provider_call marker does NOT establish Native exception origin. A base
// or terminal can end payload lifetime before a throwing provider returns:
// retain independent diagnostics/creators; never rediscover dead raw fields.
void destroy_native_gui_text_storage_00ab8250(void* actual_text,
    NativeGuiTextRetirementContext&, NativeGuiTextRetirementAcquired&);

// Whole AB8EE0[32B/11starts], Native ECX Text/stack flags, EAX SAME identity,
// RET4. Actual caller flags DWORD backing is stable/disjoint through the call;
// only its ADDRESS is checked before entry; only its CURRENT LOW BYTE is read
// AFTER AB8250 returns, never a DWORD load or entry snapshot.
// Bit0 then returns SAME slot through SAME actual F8BDF0/AB75A0. No count test,
// decrement, rollback, premature unbind or flag snapshot. Flags0 still destroys.
// Failure does not read flags/return slot. Public diagnostics cannot be reset
// or replayed; aliases do not keep any payload alive or own the failed graph.
void* delete_native_gui_text_storage_00ab8ee0(void* actual_text,
    const volatile std::uint32_t& actual_flags_word,
    NativeGuiTextRetirementContext&, NativeGuiTextScalarRetirementAcquired&);

struct NativeGuiTextPrimaryPrefixLifetimeContext;
struct NativeGuiTextShaderStorageFrame;
struct NativeGuiTextRawModelRetirementContext {
    NativeGuiTextRawModelSectionStorageContext& resources;
    NativeGuiTextRawModelRecord& model; // SAME independent85 record, not Text slot storage
    NativeGuiWidgetLifetimeContext& base;
    NativeGuiTextGlyphChildrenContext& glyphs;
    NativeGuiTextPool& actual_pool_00f8bdf0;
    // Fixed typed optional graph, never enum/receipt-shaped ownership authority.
    const NativeGuiTextPrimaryPrefixLifetimeContext* const primary_prefix{};
    // Fixed pre-pool reservation: SAME raw122 Frame's scalar.destruction.shader.
    // Presence grants no admission/credit. Existing callers leave this null;
    // current FRESH/108/1EC/header/default/prefix guards remain unchanged.
    NativeGuiTextShaderStorageFrame* const font_shader_frame{};
};
// DEFAULT constructor/bind admission stays strict. These overloads additionally
// admit the optional private-receipt-qualified primary prefix lifetime graph;
// presence alone authorizes nothing. Base/container helpers stay within their
// explicitly admitted branches and CURRENT initialized type lineages.
// Raw188 uses the caller's genuine Model-only raw tree/481 terminal domain.
// Keep its independent model record after Text pool return until actual full
// Model cleanup/pool return and quiescence; retired metadata alone proves neither.
void destroy_native_gui_text_storage_00ab8250(void* actual_text,
    NativeGuiTextRawModelRetirementContext&, NativeGuiTextRetirementAcquired&);
void* delete_native_gui_text_storage_00ab8ee0(void* actual_text,
    const volatile std::uint32_t& actual_flags_word,
    NativeGuiTextRawModelRetirementContext&, NativeGuiTextScalarRetirementAcquired&);
} // namespace bsp
