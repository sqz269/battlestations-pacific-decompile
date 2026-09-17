#pragma once
#include "bsp/native_bloom_initializer.hpp"
#include "bsp/gui_camera_store_owner.hpp"

namespace bsp {
struct NativeDistortionTextureContext {
    NativeBloomTextureContext textures;
    void* const volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
};
struct NativeDistortionTextureAcquired {
    std::array<void*,3> raw{},returned{};
    std::array<bool,3> published{};
    std::array<NativeRenderTextureSurfaceOwnerAcquired,3> holders;
    NativeRenderTextureSurfaceOwnerArguments holder_arguments{};
    std::uint32_t width{},height{},current{};
    float saved_width;
    bool entered{},capabilities_passed{},completed{},raw_free_started{};
};
struct NativeDistortionInitializationContext {
    NativeDistortionTextureContext textures;
    NativePostEffect20ConstructionContext& post_effects;
    NativeMaterialParameterAccess& parameters;
    NativeGuiSceneEnvironment& scenes;
    const char* damp_name_00d61fcc;
    const char* sample_offset_name_00d61fbc;
    const char* sample_offsets_name_00d5e40c;
    const char* sample_weights_name_00d61fac;
    const char* bump_fade_name_00d61f9c;
    const char* bump_to_disp_name_00d61f80;
    const char* texel_offset_name_00d61f70;
    const char* bump_height_name_00d61f64;
    const char* refraction_name_00d61f50;
    const char* scene_name_00d61f40;
    const char* camera_name_00d61f30;
    const char* passthrough_name_00d5e448;
    const char* scene_color_offset_name_00d5e430;
};
struct NativeDistortionInitializationAcquired {
    void* receiver{};
    NativeDistortionTextureAcquired textures;
    std::array<void*,3> raw_posts{},returned_posts{};
    std::array<bool,3> posts_published{},post_raw_free_started{},completed_post_preserved{};
    void* raw_scene{}; NativeGuiSceneOwner* scene{};
    void* raw_frame{}; NativeFrameTargetOwnerStorage* frame{};
    void* raw_camera{}; NativeCameraOwner* camera_owner{}; NativeCameraReference* camera_reference{};
    void* raw_viewport{}; NativeViewportOwner* viewport{};
    NativeString* effect_name_header{}; NativeString* parameter_name_header{}; NativeString* last_name_header{};
    const void* last_return_header{};
    std::uint32_t name_mask{},name_returns_started{};
    int native_state{-1};
    bool scene_published{},frame_published{},camera_published{},camera_native_completed{},camera_retired{};
    bool scene_raw_free_started{},frame_raw_free_started{},camera_slot_return_started{},viewport_raw_free_started{};
    bool viewport_creator_release_started{},completed{},capability_failure{};
};

// Caller-owned stable host metadata. Prepare before native initialization;
// preserve it through every surviving post/camera/viewport and parameter borrow.
// No published-child rollback or replay of a begun free/return is provided.
class NativeDistortionInitializationBlock final {
public:
    enum class Phase {idle,preparing,prepared,executing,settled};
    NativeDistortionInitializationBlock() noexcept=default;
    ~NativeDistortionInitializationBlock() noexcept;
    NativeDistortionInitializationBlock(const NativeDistortionInitializationBlock&)=delete;
    NativeDistortionInitializationBlock& operator=(const NativeDistortionInitializationBlock&)=delete;
    void prepare(NativeDistortionInitializationContext&);
    void cancel_preparation() noexcept;
    Phase phase() const noexcept {return phase_;}
    const NativeDistortionInitializationAcquired& acquired() const noexcept {return acquired_;}
    NativePostEffect20ConstructionBlock& post(std::uint32_t index);
    // Borrow for the existing distortion lifetime context. Scene destruction
    // self-deletes its companion; this diagnostic publication then stays stale.
    NativeGuiSceneOwner* const volatile& scene_owner_3c() const noexcept {return scene_publication_;}
    // Caller has disposed every survivor/residual name, ended parameter borrows,
    // and reset all child post blocks. Scene publication is not dereferenced.
    void reset_after_host_quiescence() noexcept;
private:
    friend bool initialize_native_distortion_00b4f560(void*,std::size_t,std::uint32_t,std::uint32_t,NativeDistortionInitializationBlock&);
    void settle() noexcept;
    void build_name(NativeString&,const char*,std::uint32_t);
    void return_name(NativeString&);
    void create_post(std::uint32_t,std::uint32_t,const char*,std::uint32_t,int,int,bool=false);
    void parameter(std::uint32_t,std::uint32_t,const char*,std::uint32_t,std::uint32_t,int,bool=false);
    void unwind();
    static void retire_camera(void*,NativeCameraReference&) noexcept;
    NativeDistortionInitializationContext* context_{};
    Phase phase_{Phase::idle};
    NativeDistortionInitializationAcquired acquired_;
    NativePostEffect20ConstructionBlock posts_[3];
    NativeString effect_name_,parameter_name_,last_name_;
    NativeGuiSceneOwner* volatile scene_publication_{};
    std::optional<NativeCameraOwner> camera_owner_;
    std::optional<NativeCameraReference> camera_reference_;
    SceneAttachmentRuntime::BindingAdmission camera_scene_;
    GeneratedModelLifetimeRuntime::BindingAdmission camera_lifetime_;
    NativeViewportRegistry::Storage viewport_records_[2];
    NativeViewportRegistry::Admission viewport_admissions_[2];
};

// Full2664-byte B4F560, ECX existing26Ch owner, width/height stack, AL/RET8.
// Capability111 then fallback114; require112, query post-blend112 into260.
// Round dimensions down to8, create three holders and exact x87 half/inverse
// texels24..30, three post20s and nine borrowed material parameter sources.
// Construct scene3C/frame38/camera34, replace viewport and release its creator.
// Preserve28-state cleanup, individual name-mask ordering and early false
// return stores. No initialization of previously unwritten34/38/3C is added
// on capability failure. Full runtime, original ABI and native FH3 are unproved.
bool initialize_native_distortion_00b4f560(void*,std::size_t,std::uint32_t width,
    std::uint32_t height,NativeDistortionInitializationBlock&);

namespace detail {
// Actual production prefix B4F560..B4F7A3, ending before first post allocation.
// Caller owns holder raw cleanup via native_state0/1 when this throws.
bool initialize_native_distortion_texture_prefix(void*,std::uint32_t,std::uint32_t,
    NativeDistortionTextureContext&,NativeDistortionTextureAcquired&,int& native_state);
void distortion_reciprocal_width(std::uint32_t,const volatile float*,const volatile double*,bool,float*) noexcept;
void distortion_reciprocal_pair(void*,std::uint32_t,const float*,const volatile float*,const volatile double*,bool) noexcept;
}
} // namespace bsp
