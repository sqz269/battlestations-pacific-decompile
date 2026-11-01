#pragma once
#include "bsp/native_gui_icon_texture_apply_storage.hpp"
#include "bsp/native_gui_icon_delayed_request_storage.hpp"
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp {
struct NativeRendererTextureFindStorageContext;
class NativeGuiIconDelayedLoads;
struct NativeGuiIconVisibilityStorageContext {
    NativeRendererTextureFindStorageContext& find;
    NativeGuiIconTextureApplyStorageContext& apply;
    NativeGuiIconDelayedRequestStorageContext& request;
    NativeGuiIconDelayedLoads& delayed;
    // Required initialized PRIVATE Source ABI inputs for each raw Find frame.
    // Copied CURRENT before entry, never default zeros/Original stack identity.
    const volatile std::uint32_t (&find_private_preimages)[2]; // token, renderer
    const void* transparent_literal_00d5c058;
};

class NativeGuiIconVisibilityStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        void* actual_icon{};
        const volatile std::uint32_t* visible_home{};
        std::uint32_t active_native_site{}, escaped_native_site{};
        std::exception_ptr failure;
    };
    NativeGuiIconVisibilityStorageFrame(); // No Native home seeds.
    ~NativeGuiIconVisibilityStorageFrame(); // Metadata only after caller-proven last-use.
    NativeGuiIconVisibilityStorageFrame(const NativeGuiIconVisibilityStorageFrame&) = delete;
    NativeGuiIconVisibilityStorageFrame& operator=(const NativeGuiIconVisibilityStorageFrame&) = delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void native_gui_icon_visibility_storage_00ab6120(void*, const volatile std::uint32_t&,
        NativeGuiIconVisibilityStorageFrame&, NativeGuiIconVisibilityStorageContext&);
    struct Storage;
    std::unique_ptr<Storage> storage_;
    Observation observation_;
};

// Whole NORMAL [00AB6120,00AB63A0), inclusive end639F,640B/199starts.
// Original ECX Icon138/stack visibility DWORD lowBYTE/RET4 becomes this ordinary
// Source ABI. The actual ordinary caller HOME is borrowed through a volatile
// view: read its current byte separately at6194/6332, no entry bool snapshot.
// Genuine complete default Icon/state/texture/Model/renderer class, generation,
// physical C++ storage, canonical companions and provider/callback custody are
// caller proof. Numeric profiles/slots, count/phase/return never establish it.
// Use distinct retained records for reached find/apply/get/request/placeholder/
// raw80, with same actual contexts/cells. No semantic adapter, numeric codeword
// invocation, no-op80, accepted verdict, owner retain or new native bank.
// Unknown Source escape retains remaining name/texture/guard/child debt and
// backing/providers; no private FH3/SEH cleanup, rollback/retry or new terminate
// policy. Source ABI/FLAGS/fault/NoThrow/Origin/whole-object/runtime remain held.
void native_gui_icon_visibility_storage_00ab6120(void* actual_icon138,
    const volatile std::uint32_t& actual_visible_home,
    NativeGuiIconVisibilityStorageFrame&, NativeGuiIconVisibilityStorageContext&);
} // namespace bsp
