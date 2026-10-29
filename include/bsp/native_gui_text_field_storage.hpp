#pragma once
#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_font_resources.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text field storage requires MSVC Win32.
#endif

namespace bsp {
// Required pure metadata view of the SAME captured current raw Text profile.
// Return its genuine live table (at least 84h); never a copied/forged numeric
// table or callable host-vtable overlay. Resolution cannot reenter or mutate.
// Known AB7200/AB6B50 entries execute the actual field bodies below. Other
// reached targets require their genuine implementations through these calls;
// no default target, successful no-op or numeric-address host call is supplied.
class NativeGuiTextFieldCurrentBindings {
public:
    virtual ~NativeGuiTextFieldCurrentBindings() = default;
    virtual const volatile std::uint32_t* current_text_table(void* actual_text,
        std::uint32_t captured_profile) = 0;
    virtual void call_unknown_state_80(void* actual_text,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        std::int32_t state) = 0;
    virtual void call_unknown_color_50(void* actual_text,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        const void* original_rgba) = 0;
};
struct NativeGuiTextFieldStorageContext {
    // Already ready actual10h registry/list and SAME image Font allocations.
    // Font108 is borrowed; this collection outlives every Text font use.
    NativeFontResourceOwners& fonts;
    NativeGuiTextFieldCurrentBindings& current;
};
struct NativeGuiTextFieldStorageFrame {
    // Genuine invocation-local scratch only, never another field authority.
    // No preinitialization is needed: every reached read follows its store.
    // Keep a different retained frame for every nested/current callback.
    volatile std::uint32_t rgba[4];
    volatile float shadow_alpha;
};
struct NativeGuiTextFieldStorageAcquired {
    // Diagnostics only; no count/ownership and no automatic cleanup. One fresh
    // record before entry. An escaping call leaves failed and reached native
    // mutations/credits intact. Retain raw backing, ALL original creator/122/
    // provider graphs, this frame and record; never rollback, reset or retry.
    // A terminal/provider may end a payload lifetime before failure: identities
    // below are diagnostic only, never permission to rediscover dead fields.
    enum class Phase { fresh, entered, complete, failed };
    enum class Entry { none, font_name, flag77, state_color, color };
    Phase phase{Phase::fresh};
    Entry entry{Entry::none};
    void* destination{};
    std::uint32_t active_call_site{};
    bool equal_name{}, name_resized{}, name_copied{}, font_published{}, alpha_published{};
    bool flag_published{}, state_row_copied{}, state_brightened{}, base_color_returned{};
    bool shadow_alpha_published{};
    std::uint32_t state_profile{}, state_target{}, color_profile{}, color_target{};
    bool state_entered{}, state_returned{}, color_entered{}, color_returned{};
    void* selected_font{}; // borrowed actual Font, no retained credit
    void* shader{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long shader_result{};
    bool shader_decrement_entered{}, shader_decremented{};
    bool shader_zero_entered{}, shader_zero_returned{}, shader_slot_cleared{};
};

// These operations admit ONLY the genuine full raw1F4 completed93 bound by
// canonical122. The Step1 eight-byte identity/typed Text companion is excluded.
// No second field/tree/Model/count/registry, factory admission or owner is made.
// Preflight is Source metadata checking, not original Native ABI/SEH behavior.
//
// Caller retains the complete existing93/122 service graph and serializes its
// native lifetime through return/failure disposition. Actual reached models,
// materials, arrays, native headers and current tables must be live/readable.
// Frames/diagnostics and their transitive backing are separate from Text1F8,
// companion, inputs and every context/provider/import/constant/native resource.
// Known-region checks cannot prove all opaque/transitive extent disjointness.
// Required providers may alter fields as Original does; subsequent reads are
// current. No callback may retire this Text before its remaining native reads.

// Whole normal AB8C30[165B]. ECX Text, stack actual8B string header, RET4.
// Compare435C40 -> actual1C8 resize/copy -> borrowed ready AC3570 registry
// (original7371D0 getter remains external) -> publish actual Font108 BEFORE
// raw Font+1C/default live one to1D4 -> CURRENT shader1EC decrement/zero/clear.
// Native Font retain/release is NOT added. Cached header identity/length and
// post-callback reads are preserved; Source CRT memmove qualifies BF7680.
void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextReference&,
    const void* actual_name_header8, NativeGuiTextFieldStorageContext&,
    NativeGuiTextFieldStorageFrame&, NativeGuiTextFieldStorageAcquired&);

// Raw Font-name transport only. No unused legacy color/current callback facade.
struct NativeGuiTextRawFontNameStorageContext { NativeFontResourceOwners& fonts; };
class NativeGuiTextRawFontNameStorageFrame;
struct NativeGuiTextRawFontNameStorageAcquired;
class NativeGuiTextRawFontNameStoragePublication final {
public:
    enum class Origin { resized_normal, inherited_self };
    NativeGuiTextRawFontNameStoragePublication(const NativeGuiTextRawFontNameStoragePublication&) = delete;
    NativeGuiTextRawFontNameStoragePublication& operator=(const NativeGuiTextRawFontNameStoragePublication&) = delete;
    bool issued() const noexcept { return issued_; } // private producer state, never supplied back
    const void* text_identity() const noexcept { return text_; }
    const void* header_identity() const noexcept { return header_; }
    const void* data_identity() const noexcept { return data_; } // diagnostic after return
    std::uint32_t length() const noexcept { return length_; }
    const void* font_identity() const noexcept { return font_; } // borrowed; no Font credit
    std::uint32_t alpha_bits() const noexcept { return alpha_; }
    Origin origin() const noexcept { return origin_; }
    const NativeGuiTextRawModelReferenceContext* reference_context() const noexcept { return reference_context_; }
    const NativeGuiTextReferenceFrame* reference_frame() const noexcept { return reference_frame_; }
    const NativeGuiTextRawFontNameStorageContext* context() const noexcept { return context_; }
    const NativeGuiTextRawFontNameStorageFrame* frame() const noexcept { return frame_; }
    const NativeGuiTextRawFontNameStorageAcquired* acquisition() const noexcept { return acquisition_; }
    const NativeStringRawPoolContext* strings() const noexcept { return strings_; }
    const NativeFontResourceOwners* fonts() const noexcept { return fonts_; }
    const NativeGuiTextRawFontNameStoragePublication* previous() const noexcept { return previous_; }
private:
    friend struct NativeGuiTextRawFontNameStorageAcquired;
    friend void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextRawModelReference&,
        const void*, NativeGuiTextRawFontNameStorageContext&,
        NativeGuiTextRawFontNameStorageFrame&, NativeGuiTextRawFontNameStorageAcquired&);
    NativeGuiTextRawFontNameStoragePublication() noexcept = default;
    bool issued_{};
    Origin origin_{Origin::resized_normal};
    const void* text_{}; const void* header_{}; const void* data_{}; const void* font_{};
    std::uint32_t length_{}, alpha_{};
    const NativeGuiTextRawModelReferenceContext* reference_context_{};
    const NativeGuiTextReferenceFrame* reference_frame_{};
    const NativeGuiTextRawFontNameStorageContext* context_{};
    const NativeGuiTextRawFontNameStorageFrame* frame_{};
    const NativeGuiTextRawFontNameStorageAcquired* acquisition_{};
    const NativeStringRawPoolContext* strings_{};
    const NativeFontResourceOwners* fonts_{};
    const NativeGuiTextRawFontNameStoragePublication* previous_{};
};
struct NativeGuiTextRawFontNameStorageAcquired final {
    NativeGuiTextRawFontNameStorageAcquired() noexcept = default;
    NativeGuiTextRawFontNameStorageAcquired(const NativeGuiTextRawFontNameStorageAcquired&) = delete;
    NativeGuiTextRawFontNameStorageAcquired& operator=(const NativeGuiTextRawFontNameStorageAcquired&) = delete;
    NativeGuiTextFieldStorageAcquired diagnostics; // never sealing/reentry authority
    bool failed() const noexcept { return failed_; }
    const NativeGuiTextRawFontNameStoragePublication* publication() const noexcept;
    // Fresh storage identity for preinstalled later invocations/extent checks;
    // unissued storage is never prior-header or lifetime authority.
    const NativeGuiTextRawFontNameStoragePublication* publication_storage() const noexcept { return &publication_; }
private:
    friend class NativeGuiTextRawFontNameStorageFrame;
    friend void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextRawModelReference&,
        const void*, NativeGuiTextRawFontNameStorageContext&,
        NativeGuiTextRawFontNameStorageFrame&, NativeGuiTextRawFontNameStorageAcquired&);
    const NativeGuiTextRawFontNameStorageFrame* bound_frame_{};
    bool failed_{};
    NativeGuiTextRawFontNameStoragePublication publication_;
};
class NativeGuiTextRawFontNameStorageFrame final {
public:
    enum class Phase { fresh, entered, complete, failed };
    enum class HeaderEntry { unobserved, constructor_empty, prior_normal };
    // Addresses only, including an uninitialized caller8B input. Keep every
    // referent independently alive through failure/Text return/parent closure.
    NativeGuiTextRawFontNameStorageFrame(const NativeGuiTextRawFontNameStorageContext&,
        const void* actual_input_header8, const NativeGuiTextReferenceFrame&,
        NativeGuiTextShaderStorageFrame&, NativeGuiTextRawFontNameStorageAcquired&,
        const NativeGuiTextRawFontNameStoragePublication* prior_header = nullptr) noexcept;
    ~NativeGuiTextRawFontNameStorageFrame() noexcept;
    NativeGuiTextRawFontNameStorageFrame(const NativeGuiTextRawFontNameStorageFrame&) = delete;
    NativeGuiTextRawFontNameStorageFrame& operator=(const NativeGuiTextRawFontNameStorageFrame&) = delete;
    const NativeGuiTextRawFontNameStorageContext* prepared_context() const noexcept { return context_; }
    const void* prepared_input_header8() const noexcept { return input_; }
    const NativeGuiTextReferenceFrame* prepared_reference_frame() const noexcept { return reference_frame_; }
    const NativeGuiTextShaderStorageFrame* prepared_shader_frame() const noexcept { return &shader_; }
    const NativeGuiTextRawFontNameStorageAcquired* acquisition() const noexcept { return acquired_; }
    const NativeGuiTextRawFontNameStoragePublication* prior_header() const noexcept { return prior_; }
    const NativeGuiTextFieldStorageFrame& body_frame() const noexcept { return body_; }
    Phase phase() const noexcept { return phase_; }
    HeaderEntry entry_header() const noexcept { return entry_header_; }
    bool normal_equal_name() const noexcept { return phase_ == Phase::complete && equal_name_; }
    bool matches(const NativeGuiTextRawFontNameStorageContext&, const void*,
        const NativeGuiTextReferenceFrame&, const NativeGuiTextShaderStorageFrame&,
        const NativeGuiTextRawFontNameStorageAcquired&, Phase) const noexcept;
private:
    friend void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextRawModelReference&,
        const void*, NativeGuiTextRawFontNameStorageContext&,
        NativeGuiTextRawFontNameStorageFrame&, NativeGuiTextRawFontNameStorageAcquired&);
    const NativeGuiTextRawFontNameStorageContext* const context_;
    const void* const input_;
    const NativeGuiTextReferenceFrame* const reference_frame_;
    NativeGuiTextShaderStorageFrame& shader_; // actual mutable credited frame, never const_cast
    NativeGuiTextRawFontNameStorageAcquired* const acquired_;
    const NativeGuiTextRawFontNameStoragePublication* const prior_;
    NativeGuiTextFieldStorageFrame body_; // only actual reached native scratch
    Phase phase_{Phase::fresh};
    HeaderEntry entry_header_{HeaderEntry::unobserved};
    bool equal_name_{};
};
// Uncalled library dependency. Does not relax owner FRESH/108/1EC, raw122 or
// scalar admission. Input construction/return is the genuine retained caller's
// responsibility. Equal-name grants no new publication. Existing/self1C8 needs
// the exact private prior receipt. All failures/reentry poison, never cleanup.
void set_native_gui_text_font_name_storage_00ab8c30(NativeGuiTextRawModelReference&,
    const void* actual_name_header8, NativeGuiTextRawFontNameStorageContext&,
    NativeGuiTextRawFontNameStorageFrame&, NativeGuiTextRawFontNameStorageAcquired&);

// Whole normal AB6A70[39B]: store LOWBYTE at77, then CURRENT80(3 or0).
// This is native vslot7C, not the Font property at byte108.
void set_native_gui_text_flag77_storage_00ab6a70(NativeGuiTextReference&,
    std::uint8_t flag, NativeGuiTextFieldStorageContext&,
    NativeGuiTextFieldStorageFrame&, NativeGuiTextFieldStorageAcquired&);

// Whole normal AB7200[132B]: current77 ? row14C : DWORD-wrap(state*10+11C),
// four native DWORD loads, exact x87 state1 additions using SAME live Original
// constant cells, then CURRENT50. No synthetic state bound/clamp is added;
// every reached row remains the caller's actual readable-storage obligation.
void apply_native_gui_text_state_color_storage_00ab7200(NativeGuiTextReference&,
    std::int32_t state, NativeGuiTextFieldStorageContext&,
    NativeGuiTextFieldStorageFrame&, NativeGuiTextFieldStorageAcquired&);

// Whole normal raw listing AB6B50..AB6BC3 (116B), current Text50 entry. Genuine
// raw AA6870 uses main Model4C; CURRENT shadow188 traverses real native Model
// tail/mesh/section and changes only material diffuse alpha. x87 product spills
// BEFORE B179F0, then reloads after it. No semantic GUI receiver/raw cast.
// Main Model4C is required even if its geometry is empty, just as AA6870 reads.
void set_native_gui_text_color_storage_00ab6b50(NativeGuiTextReference&,
    const void* original_rgba, NativeGuiTextFieldStorageContext&,
    NativeGuiTextFieldStorageFrame&, NativeGuiTextFieldStorageAcquired&);
} // namespace bsp
