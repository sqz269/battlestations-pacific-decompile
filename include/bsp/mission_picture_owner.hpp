#pragma once

#include "bsp/gui_texture.hpp"
#include "bsp/native_render_context.hpp"
#include <array>
#include <functional>
#include <string_view>

namespace bsp {

// Borrow the actual atlas/renderer bindings and the SAME canonical native
// owner domain as those textures. Every nonnull result is an acquired native
// reference with its actual atomic at +04; it is never an IDirect3DTexture9*.
// Both dependencies outlive every picture and its copies. Missing identities
// or current profiles are errors in NativeRenderActualOwners, never no-ops.
struct MissionPictureTextureServices {
    const GuiTextureCallbacks& resolve;
    NativeRenderActualOwners& actual_owners;
    // Required 005C75A9 boundary: lazy GUI manager construction/registration
    // can have effects even though AA2660 does not read the resulting ECX.
    std::function<void*()> get_gui_manager_004c12b0;
    // 005C75B4 passes an uninitialized frame float2 to AA2660. Supply its
    // preimage for each nonempty picture; no zero/default size is invented.
    std::function<std::array<float, 2>()> size_preimage;
};

void validate_mission_picture_services(const MissionPictureTextureServices&);

// The picture subobject of the SAME semantic MissionRecordData. The default
// native state (005C89B0) is null +A0 and {0,0,1,1} +A4. Copy construction
// (005C81F0) retains; assignment (005C7B80) publishes, retains, releases old,
// then copies UV. Destruction (005C8AE0) releases before clearing +A0.
// No move-steal operation is invented: C++ rvalues also use the retain-copy.
// All reached native lifetime operations must return normally; failure during
// C++ destruction terminates. Other record members and SEH are not ABI ports.
class MissionPictureOwner final {
public:
    explicit MissionPictureOwner(NativeRenderActualOwners&) noexcept;
    MissionPictureOwner(const MissionPictureOwner&) noexcept;
    MissionPictureOwner& operator=(const MissionPictureOwner&);
    ~MissionPictureOwner();

    void* texture_0a0() const noexcept { return texture_; }
    const std::array<float, 4>& uv_0a4() const noexcept { return uv_; }

    // 005C75A2..005C763B: pass this exact UV storage to AA2660, acquire
    // new, release old, then publish. Empty names release and preserve UV.
    // A null resolver result is published as native null, not a failed
    // binding fallback. Resolver/provider exceptions are not swallowed.
    void read_005c6a70(std::string_view, const MissionPictureTextureServices&);

private:
    NativeRenderActualOwners* owners_;
    void* texture_ = nullptr;
    std::array<float, 4> uv_{0.0f, 0.0f, 1.0f, 1.0f};
    void release_then_clear();
};

} // namespace bsp
