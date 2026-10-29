#pragma once
#include "bsp/native_gui_text_reference.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
namespace game { class GameNativeGuiTextProcess; }

// Prepared caller-owned DEFAULT graph. All fields borrow existing domains;
// this creates no Text process, pool, Model, NodeBinding or SceneResource owner.
// publication and its constructor/retirement graph ALREADY have stable storage
// before entry. In particular publication.construction is exactly construction,
// and publication.retirement.model is construction.sections.model.
struct NativeGuiTextDefaultFactoryStorageContext final {
    game::GameNativeGuiTextProcess& process;
    NativeGuiTextRawModelReferenceContext& publication;
    NativeGuiTextRawModelDefaultStorageAcquired& construction;
    NativeGuiTextReferenceFrame& reference_frame;
    void* companion_storage;
    std::size_t companion_storage_bytes;
    NativeGuiTextRawModelReferenceDisposal disposal;
};

struct NativeGuiTextDefaultFactoryStorageAcquired final {
    enum class Phase { fresh, admission, allocating, empty, constructing, complete, failed };
    enum class Boundary { none, admission, allocation, constructor_call, result_admission };
    NativeGuiTextDefaultFactoryStorageAcquired() = default;
    NativeGuiTextDefaultFactoryStorageAcquired(const NativeGuiTextDefaultFactoryStorageAcquired&) = delete;
    NativeGuiTextDefaultFactoryStorageAcquired& operator=(const NativeGuiTextDefaultFactoryStorageAcquired&) = delete;
    NativeGuiTextDefaultFactoryStorageAcquired(NativeGuiTextDefaultFactoryStorageAcquired&&) = delete;
    NativeGuiTextDefaultFactoryStorageAcquired& operator=(NativeGuiTextDefaultFactoryStorageAcquired&&) = delete;
    // No destructor performs native cleanup, pool return, disposal or retry.
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none};
    bool started{}, complete{}, allocation_entered{}, allocation_returned{};
    bool constructor_entered{}, constructor_returned{};
    NativeGuiTextDefaultFactoryStorageContext* prepared_context{};
    NativeGuiTextRawModelDefaultStorageAcquired* construction_record{};
    NativeGuiTextPool* actual_pool{};
    void* raw_slot{};
    void* constructor_result{};
    std::uint32_t active_native_site{};
    std::exception_ptr failure;
};

// Qualified AA1380 DEFAULT path ONLY: native ECX source=0, AB79E0's SAME
// F8BDF0 allocation, then ECX=nonnull raw1F4 -> AB9650, EAX SAME Text or null.
// This explicit C++ API has no copy/source argument and is NOT native register,
// private-stack, FH3/SEH, full AA1380 body or binary-detour ABI support.
//
// The caller installs and retains this whole nonmoving attempt/graph before
// entry. Known-region validation precedes its first mutation; the attempt and
// current boundary are then published BEFORE any allocation callback. Null
// allocation completes Phase::empty normally, without ctor/binding/throw.
// Any entered admission/allocation/ctor/result failure is retained and rethrown;
// no AB76F0, AB8250, raw122 binding, credit change or inferred rollback follows.
// raw_slot/result are diagnostic only after failure or subsequent retirement.
//
// Genuine initialized canonical Text process/types/pool, raw Model pool/node/
// world/scene/type/import/point-light/recursive-frame providers, mandatory188
// mesh/shadow/GuiDefault material, and ready real renderer/cache/Lua/sampler/VFS
// are ENTRY requirements. The private Model dispatcher is already bound to
// exactly construction.sections.model (e769 qualified provider); no table-only,
// A1 camera-only or fabricated callback closes these requirements. Activate the
// genuine renderer Lua service binding around synchronous effect-cache entry.
// All providers/frames/backings are live, disjoint where written, serialized,
// and externally authenticated, including opaque/derived extents and pool/slab
// provenance. Public diagnostic flags alone prove none of that custody.
//
// On nonnull normal return the caller keeps the SAME initial Text credit and
// separately calls bind_native_gui_text_raw_model_reference with precisely
// c.companion_storage/c.publication/c.reference_frame/c.disposal. No extra retain
// is needed. The caller records that separate one-shot binding entry/failure;
// factory complete never means published. Bind failure is host admission,
// never native failed-ctor cleanup.
// Even successful Text disposal must preserve this independent producer/188
// graph for surviving Model credits, flags0 physical-slot custody or failure;
// retired metadata is not cleanup proof. No glyph/cursor/content/copy/Page path.
void* make_native_gui_text_default_factory_storage_00aa1380(
    NativeGuiTextDefaultFactoryStorageContext&,
    NativeGuiTextDefaultFactoryStorageAcquired&);
} // namespace bsp
