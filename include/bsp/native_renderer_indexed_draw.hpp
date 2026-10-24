#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Full B1F740 (4 bytes): ECX viewport, EAX viewport+10, RET. Computes an
// address only; does not dereference the viewport or read its dimensions.
void* __fastcall native_viewport_size_address_00b1f740(const void*) noexcept;

// Full B48CD0 (4 bytes): ECX logical stream, EAX DWORD at+64, RET.
std::uint32_t __fastcall native_logical_vertex_count_00b48cd0(const void*) noexcept;

struct NativeRendererIndexedDrawContext {
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    // Borrow the original D61D6C profile; current slot20 is full B48CD0.
    // This operation only queries a borrowed stream, without lifetime work.
    const volatile std::uint32_t* actual_logical_vertex_profile_00d61d6c;
};

enum class NativeRendererIndexedDrawTerminal : std::uint8_t {
    skippedInhibited,
    skippedLost,
    skippedEmpty,
    skippedStreamCount,
    deviceCalled
};

struct NativeRendererIndexedDrawObservation {
    void* actual_renderer = nullptr;
    void* actual_device = nullptr;
    std::uint32_t primitive_type = 0;
    std::uint32_t minimum_vertex = 0;
    std::uint32_t vertex_count = 0;
    std::uint32_t start_index = 0;
    std::uint32_t primitive_count = 0;
    std::int32_t base_vertex = 0;
    std::uint32_t inhibit = 0;
    std::uint8_t lost = 0;
    const void* logical_stream = nullptr;
    std::uint32_t logical_count = 0;
    std::uint32_t logical_tag = 0;
    NativeRendererIndexedDrawTerminal terminal =
        NativeRendererIndexedDrawTerminal::skippedInhibited;
    std::int32_t returned_hresult = 0;
};

// Observations borrow all identities. Fields whose native read boundary was
// not reached stay zero: lost follows the inhibit gate, logical fields follow
// the empty-count gate, and device/base/HRESULT are valid only for deviceCalled.
// With an observer, a nonnull logical stream's tag is also read after its native
// count query. Callbacks run after native guard completion on normal returns.
using NativeRendererIndexedDrawObserver = void (*)(
    const NativeRendererIndexedDrawObservation&, void* userdata) noexcept;

// Thread-local, nestable diagnostic scope. A null callback adds no raw reads,
// logging, or device operations; the original draw result remains ignored.
class NativeRendererIndexedDrawObserverScope final {
public:
    explicit NativeRendererIndexedDrawObserverScope(
        NativeRendererIndexedDrawObserver, void* userdata = nullptr) noexcept;
    ~NativeRendererIndexedDrawObserverScope() noexcept;
    NativeRendererIndexedDrawObserverScope(
        const NativeRendererIndexedDrawObserverScope&) = delete;
    NativeRendererIndexedDrawObserverScope& operator=(
        const NativeRendererIndexedDrawObserverScope&) = delete;
    NativeRendererIndexedDrawObserverScope(
        NativeRendererIndexedDrawObserverScope&&) = delete;
    NativeRendererIndexedDrawObserverScope& operator=(
        NativeRendererIndexedDrawObserverScope&&) = delete;

private:
    NativeRendererIndexedDrawObserver previous_observer_;
    void* previous_userdata_;
};

// Complete B24010..B2415B. Original ECX renderer; stack primitive type,
// minimum vertex, vertex count, start index, primitive count; RET14h.
// Inhibit1D90/lost1D8A gates precede viewport1904 read and optional guard.
// Zero-count paths use the mode captured after guard entry. Nonzero paths
// query current stream0+20; excess count is rejected only for CURRENT
// stream0 tag54==40000001. Device/table/base vertex remain current reads.
// DrawIndexedPrimitive uses table148, ignores HRESULT and changes no counter.
void draw_native_renderer_indexed_00b24010(void* actual_renderer,
    std::uint32_t primitive_type, std::uint32_t minimum_vertex,
    std::uint32_t vertex_count, std::uint32_t start_index,
    std::uint32_t primitive_count, NativeRendererIndexedDrawContext&);

// New explicit-context interface, concrete logical profile, borrowed storage.
// Skipped native guard fields are uninitialized; mode changes requiring those
// fields are outside its valid domain. This does not implement stream binding,
// stream ownership, original caller ABI or gameplay.

} // namespace bsp
