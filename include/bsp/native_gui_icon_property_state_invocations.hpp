#pragma once
#include <cstdint>
#include <exception>

namespace bsp {
struct NativePlatformFocusOwnersContext;
struct NativeGuiIconStorageContext;
struct NativeParticleTextureNamesRawContext;
struct NativeTextureCacheContext;
struct GuiTextureCallbacks;
struct GuiIconStateTextureConstants;
struct NativeGuiIconStateConstructionStorageFrame;
class NativeGuiIconReference;
namespace game { class GameNativeGuiTextState; }

// Required genuine SAME process/renderer/pool/feature/RO services. No service,
// owner, global, callback or value is manufactured by this metadata bundle.
// GameNativeGuiTextState supplies its existing widget_empty_00f8bc60 Source
// role, not another empty bank or an Original-VA/extent certificate.
struct NativeGuiIconPropertyStateServices {
    NativePlatformFocusOwnersContext& gui;
    NativeGuiIconStorageContext& icon;
    NativeParticleTextureNamesRawContext& atlas;
    NativeTextureCacheContext& cache;
    const GuiTextureCallbacks& dimensions;
    const GuiIconStateTextureConstants& constants;
    const volatile std::uint32_t& feature_word_0109eea4;
    const volatile float& actual_one_00d7a24c;
    const game::GameNativeGuiTextState& widget_state;
    const void* transparent_literal_00d5c058;
};

// Retains concrete per-reached-call Source invocations of real AB66C0. This
// owns host metadata only: no Native Icon/resource/reference/count/pool birth,
// class admission, Origin or disposal authority. The eventual genuine Page
// IconRecord supplies the completed canonical companion/SAME service graph.
// Its actual AA1410/AB5C60 or AB5D30 and Reference-bind history, current class,
// generation, transitive lifetime and remaining borrowers are caller proof;
// profiles/public flags cannot supply them. Keep companion/services/providers
// and this nonmoving owner live through all records and their independent debt.
class NativeGuiIconPropertyStateInvocations final {
public:
    class Invocation; // Concrete definition/callee storage lives in the CPP.
    enum class Phase { prepared, entered, normal_returned, failed };

    NativeGuiIconPropertyStateInvocations(NativeGuiIconReference&,
        const NativeGuiIconPropertyStateServices&);
    ~NativeGuiIconPropertyStateInvocations();
    NativeGuiIconPropertyStateInvocations(const NativeGuiIconPropertyStateInvocations&) = delete;
    NativeGuiIconPropertyStateInvocations& operator=(const NativeGuiIconPropertyStateInvocations&) = delete;
    NativeGuiIconPropertyStateInvocations(NativeGuiIconPropertyStateInvocations&&) = delete;
    NativeGuiIconPropertyStateInvocations& operator=(NativeGuiIconPropertyStateInvocations&&) = delete;

    // Actually constructs a distinct Frame, two fresh cache acquisitions,
    // observed-release record and contexts referencing those EXACT objects.
    // One stable host allocation; publish only after construction returns.
    // Fresh nested calls do not reuse/overwrite outer records. No fixed pool,
    // count bound, Native reads/calls or blanket Frame initialization occurs.
    // Source allocation/new-handler/construction may throw/reenter; previous
    // records stay retained. Failure is host metadata history, not Native EH.
    // Future3310 prepares after UVleave BEFORE37C3's current-A4 capture, then
    // stages its real four homes. No early property/value/target snapshot.
    Invocation& prepare();

    // Cold preparation access to actual no-init Frame/preimages. Supply the
    // genuine initialized bits required by inherited raw suppliers; no zero
    // defaults are supplied. Only this record's still-prepared Frame is mutable
    // here. This borrow and Phase are NOT proof of Native storage/ownership.
    NativeGuiIconStateConstructionStorageFrame& preparation_frame(Invocation&);

    // Concrete direct100V2 operation, NOT a current-target dispatcher. Caller
    // invokes this only for its actual captured known-class AB66C0 edge over
    // this SAME companion identity and four live homes [name,UV,pivot,size].
    // Genuine creation/custody proves that route; no numeric call, target
    // rescan/profile guard, OTHER substitution or fake CurrentA4 port exists.
    // Different fresh records can synchronously nest on this same owner.
    // Same-record reuse/replay is Source misuse, rejected before Native entry.
    // Any escaping C++ exception records failed history/barethrows; no caller
    // name cleanup, reader pop, Native unwind, retry or record erasure follows.
    // Source100's actual temp cleanup/lower policies are inherited unchanged.
    std::uint32_t invoke_00ab66c0(Invocation&,
        volatile std::uint32_t (&actual_argument_homes)[4]);

    // Const Source history only, never permission to read ended payload/home
    // phases or discharge provider debt. After normal66, parent input homes/
    // name/UV may be reused/returned; old child pointer metadata is historical.
    const Invocation* first() const;
    const Invocation* next(const Invocation&) const;
    Phase phase(const Invocation&) const;
    const std::exception_ptr& failure(const Invocation&) const;
    std::uint32_t returned_bits(const Invocation&) const;
    bool preparation_started() const;
    bool preparation_failed() const;
    const std::exception_ptr& first_preparation_failure() const;

    // No erase/reset/complete=>disposition API. Records survive normal and
    // failed calls until the OWNER lifetime ends. End that lifetime only after
    // genuine external obligations permit destruction of every actual frame/
    // acquisition/provider borrower. Existing failed CacheAcquired destructor
    // policy is inherited; retain the entire owner on unresolved failure/debt,
    // not an automatic local around Native work. No new terminal policy,
    // Native cleanup, allocator/retirement authority or extra body credit.
private:
    void require_record(const Invocation&) const;
    NativeGuiIconReference& companion_;
    const NativeGuiIconPropertyStateServices services_;
    void* const actual_icon_;
    Invocation* first_{};
    bool preparation_started_{}, preparation_failed_{};
    std::exception_ptr first_preparation_failure_;
};
} // namespace bsp
