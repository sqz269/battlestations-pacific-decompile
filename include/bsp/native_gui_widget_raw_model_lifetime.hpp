#pragma once
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_model_storage_node_dispatch.hpp"

namespace bsp {
struct NativeGuiWidgetRawModelRoute {
    NativeModelStorageDispatchContext& record;
    NativeModelStorageDispatch& tree;
    NativeModelStorageDispatchAcquired& tree_acquired;
    NativeModelStorageNodeDispatch& node;
    NativeModelStorageNodeDispatchAcquired& node_acquired;
};
struct NativeGuiWidgetRawModelReleaseAcquired {
    void* captured{}; // diagnostic only after the genuine terminal
    bool entered{}, returned{};
    NativeNodeTreeRetirementAcquired unlink; // OUTER B6DFA0, never inner B6F310
};
struct NativeGuiWidgetRawModelLifetimeAcquired {
    enum class Phase { fresh, entered, complete, failed };
    bool bound{}, active{}, poisoned{};
    Phase scene{Phase::fresh}, base{Phase::fresh}, glyph{Phase::fresh};
    void* receiver{};
    std::uint32_t query_target{}, descriptor{};
    bool query_result{};
    NativeGuiWidgetRawModelReleaseAcquired scene188, scene4c, late4c;
};
class NativeGuiWidgetRawModelSlots;
struct NativeGuiWidgetRawModelLifetimeContext {
    NativeGuiTextDefaultWidgetLifetimeRecord& text;
    NativeGuiTextDefaultWidgetLifetime& authentication;
    NativeGuiWidgetLifetimeContext& widget;
    NativeGuiWidgetRawModelRoute& primary;
    NativeGuiWidgetRawModelRoute& internal188;
    NativeGuiWidgetRawModelSlots& slots;
};

// Concrete two-record Source transport. Pure bind precedes ANY pool effect;
// callers prove both e769/2049 providers were bound to these EXACT aliases.
// The first callable domain is genuine raw118's active base entry only,
// childless/unparented/no-timed with unattached Model first tree credits.
class NativeGuiWidgetRawModelSlots final {
public:
    explicit NativeGuiWidgetRawModelSlots(NativeGuiWidgetRawModelLifetimeAcquired&) noexcept;
    ~NativeGuiWidgetRawModelSlots();
    NativeGuiWidgetRawModelSlots(const NativeGuiWidgetRawModelSlots&) = delete;
    NativeGuiWidgetRawModelSlots& operator=(const NativeGuiWidgetRawModelSlots&) = delete;
    void bind(NativeGuiWidgetRawModelLifetimeContext&);
    void begin_base(NativeGuiWidgetRawModelLifetimeContext&, void*);
    void begin_scene(NativeGuiWidgetRawModelLifetimeContext&, void*, bool nested_base);
    void finish_scene(bool nested_base) noexcept;
    void finish_base() noexcept;
    void boundary(void*);
    void release_captured(void*, NativeGuiWidgetRawModelReleaseAcquired&);
    NativeGuiWidgetRawModelReleaseAcquired& scene_primary() noexcept { return acquired_.scene4c; }
    NativeGuiWidgetRawModelReleaseAcquired& late_primary() noexcept { return acquired_.late4c; }
    void query(std::uint32_t target, std::uint32_t descriptor, bool result) noexcept;
    [[noreturn]] void reject(const char*);
    [[noreturn]] void reject_glyph(NativeGuiWidgetRawModelLifetimeContext&, void*);
    void fail() noexcept;
private:
    void authenticate(void*);
    void require_domain(void*);
    NativeGuiWidgetRawModelLifetimeContext& context();
    NativeGuiWidgetRawModelLifetimeAcquired& acquired_;
    NativeGuiWidgetRawModelLifetimeContext* context_{};
};

// Successful-call Source schedules only. ALL raw escapes poison/retain and
// rethrow without base/container unwind; no exception-origin/FH3/SEH equivalence.
// Genuine Model calls still own their existing internal failure/terminal policy.
// Receiver/AA9390 ring and all remaining scheduled reads/stores stay live;
// callbacks may replace slots, never retire/reuse the Text under this call.
// Bind/records/provider/opaque backing survive all credits/flags0/failures.
// Retired unused188 is metadata only. No initial93/122/118/owner admission is
// widened, no primary producer/publication, parent/child/timed/generic scene or
// TRUE Text188 branch supplied. The existing DEFAULT owner cannot call this.
void release_native_gui_widget_scene_00aa8320(void*, NativeGuiWidgetRawModelLifetimeContext&);
void destroy_native_gui_widget_base_00aa9730(void*, NativeGuiWidgetRawModelLifetimeContext&);
void release_native_gui_text_glyph_00ab73b0(void*, NativeGuiWidgetRawModelLifetimeContext&);
} // namespace bsp
