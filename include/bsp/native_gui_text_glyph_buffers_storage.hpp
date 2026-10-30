#pragma once
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text glyph buffers require MSVC Win32.
#endif

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Borrows the EXISTING renderer/declaration/stream domain and its SAME registry.
// The declaration string adapter must use strings' exact AA8/AA4/AA0 cells.
// No renderer, Mesh, stream, declaration cache or ownership reference is made.
struct NativeGuiTextGlyphBuffersStorageContext final {
    GuiTextNativeRendererServices& graphics;
    NativeRenderActualOwnerRegistry& owners;
    NativeStringRawPoolContext& strings;
    game::GameNativeReadOnlyData& data;
    // SAME GameNativeRendererApplication::publication_00f8d394() cell, not
    // a copied pointer. Domain admission checks its address against streams.
    void* const volatile& renderer_00f8d394;
    // Genuine existing CE2220 import target CELL. Capture only at AB84B9,
    // after vertex publication, and use that one value for all three returns.
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};

enum class NativeGuiTextGlyphBuffersStoragePhase : std::uint8_t {
    fresh, entered, complete, failed
};
struct NativeGuiTextGlyphBuffersStorageRelease final {
    void* identity{}; // address-only after a terminal; never rediscover storage
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiTextGlyphBuffersStorageAcquired final {
    using Phase = NativeGuiTextGlyphBuffersStoragePhase;
    enum class Boundary : std::uint8_t {
        none, admission, name_resize, name_copy, declaration, name_return,
        vertex_factory, vertex_publication, decrement_capture, vertex_return,
        declaration_return, index_factory, index_publication, index_return
    };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t native_site{}, escaped_site{};
    void* renderer38{};
    void* renderer5c{};
    void* renderer60{};
    std::uint32_t captured_capacity{};
    bool capacity_read{};
    void* vertex_mesh{};
    void* index_mesh{};
    bool vertex_published{}, index_published{};
    bool name_cleanup_armed{}, name_return_entered{}, name_returned{};
    bool exceptional_name_return_entered{}, exceptional_name_returned{};
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    NativeStreamCloneAcquired index;
    NativeGuiTextGlyphBuffersStorageRelease vertex_release;
    NativeGuiTextGlyphBuffersStorageRelease declaration_release;
    NativeGuiTextGlyphBuffersStorageRelease index_release;
    std::exception_ptr name_cleanup_trigger;
    std::exception_ptr failure;
    NativeGuiTextGlyphBuffersStorageAcquired() = default;
    NativeGuiTextGlyphBuffersStorageAcquired(const NativeGuiTextGlyphBuffersStorageAcquired&) = delete;
    NativeGuiTextGlyphBuffersStorageAcquired& operator=(const NativeGuiTextGlyphBuffersStorageAcquired&) = delete;
};

// Prepare a stable frame BEFORE entry; constructor reads no argument/native
// cell and does not initialize the name. arguments[0/1] are the actual mutable
// DWORD capacity/Mesh homes, not copied values. The original captures capacity
// late ONCE, but reloads the Mesh home separately for both publications.
// Caller keeps frame/acquired/context/arguments disjoint and nonmoving; each
// entered invocation needs its own frame. Raw Mesh/stream backing and every
// transitive provider must survive callbacks. No native synchronization/retain
// is supplied by this host record or by the registry's metadata lock.
class NativeGuiTextGlyphBuffersStorageFrame final {
public:
    using Phase = NativeGuiTextGlyphBuffersStoragePhase;
    NativeGuiTextGlyphBuffersStorageFrame(NativeGuiTextGlyphBuffersStorageContext&,
        volatile std::uint32_t (&arguments)[2],
        NativeGuiTextGlyphBuffersStorageAcquired&) noexcept;
    ~NativeGuiTextGlyphBuffersStorageFrame() noexcept = default;
    NativeGuiTextGlyphBuffersStorageFrame(const NativeGuiTextGlyphBuffersStorageFrame&) = delete;
    NativeGuiTextGlyphBuffersStorageFrame& operator=(const NativeGuiTextGlyphBuffersStorageFrame&) = delete;
    NativeGuiTextGlyphBuffersStorageFrame(NativeGuiTextGlyphBuffersStorageFrame&&) = delete;
    NativeGuiTextGlyphBuffersStorageFrame& operator=(NativeGuiTextGlyphBuffersStorageFrame&&) = delete;
    Phase phase() const noexcept { return phase_; }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::entered || phase_ == Phase::failed;
    }
private:
    friend void create_native_gui_text_glyph_buffers_storage_00ab8400(
        NativeGuiTextGlyphBuffersStorageFrame&);
    NativeGuiTextGlyphBuffersStorageContext& context_;
    volatile std::uint32_t (&arguments_)[2];
    NativeGuiTextGlyphBuffersStorageAcquired& acquired_;
    // Actual caller temporary. Only AB8400's reached stores initialize it.
    alignas(4) volatile std::uint32_t name_[2];
    Phase phase_{Phase::fresh};
};

// WHOLE normal AB8400..AB852D (302B/110 instructions), ECX unused, two stacked
// DWORDs (capacity, actual Mesh), RET8. New C++ ABI only. Real raw name/pool,
// current renderer38/5C/60, existing factories and B73BB0/B73B70 are composed.
// Resolve each CURRENT Mesh home in the SAME registry to NativeMeshReference
// and use its exact storage() reference: never cast raw bytes to a typed Mesh.
// No added capacity bound, count reuse, snapshot, fake stream or fallback.
// Products wrap at32 bits. Vertex and declaration returns precede index factory.
//
// Name EH covers only current38, like native state0; resize/copy and explicit
// return are outside it. Other exceptions preserve partial creator/publication
// effects. Frame failure is sticky and not resumable. Default destruction does
// no work: caller MUST retain unresolved graph/records and obey the explicit
// requires_process_retention() contract; no cleanup/retry/termination is added.
// Existing factory null/unsupported-profile and
// canonical terminal noexcept policies remain, not Native FH3/SEH/fault/private
// stack or calling-convention parity. Semantic/equal-only Text APIs are unchanged;
// this supplier alone does not close changed Text/layout/frontend admission.
void create_native_gui_text_glyph_buffers_storage_00ab8400(
    NativeGuiTextGlyphBuffersStorageFrame&);
} // namespace bsp
