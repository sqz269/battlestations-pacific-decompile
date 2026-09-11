#pragma once
#include "bsp/camera_projection.hpp"

namespace bsp {
struct RenderNodeRootList;
struct CameraTransform;
class SceneAttachmentRuntime;

// One link view, never a second hierarchy. Diagnostic links address companion
// pointer storage. Bound links address the actual native DWORD and resolve its
// current raw key through the owner's existing SceneAttachmentRuntime binding.
class CameraTransformLink final {
public:
    explicit CameraTransformLink(CameraTransform*& diagnostic) noexcept;
    CameraTransformLink(CameraTransform& owner, std::uint32_t& actual_word) noexcept;
    CameraTransformLink(const CameraTransformLink&) = delete;
    CameraTransform* get() const;
    operator CameraTransform*() const { return get(); }
    CameraTransform* operator->() const { return get(); }
    CameraTransformLink& operator=(CameraTransform*);
    CameraTransformLink& operator=(const CameraTransformLink& other) { return *this = other.get(); }
private:
    void validate_target(CameraTransform*) const;
    friend struct CameraTransform;
    CameraTransform** diagnostic_{};
    CameraTransform* owner_{};
    std::uint32_t* actual_word_{};
};
// References to one actual node's fields. Constructing the view does not read or
// initialize them; the owner supplies storage and controls its lifetime.
struct CameraTransformBacking {
    std::uint32_t& parent;
    std::uint32_t& first_child;
    std::uint32_t& child_count;
    std::uint32_t& next_sibling;
    std::uint32_t& previous_sibling;
    RenderNodeRootList*& root_list;
    std::uint32_t& valid_flags;
    std::uint32_t& auxiliary_flags;
    void*& notification_context;
    CameraMatrix& view;
    CameraMatrix& local;
    CameraMatrix& world;
    std::uint32_t actual_node_key;
};
// New C++ interface, not the native object ABI. Parents and borrowed backing must
// remain alive and form an acyclic hierarchy. Default construction owns zeroed
// diagnostic storage; it is not the native constructor. A bound transform uses
// only its supplied fields, without copying them into the unused owned storage.
struct CameraTransform {
private:
    struct OwnedStorage {
        CameraTransform* parent{};
        CameraTransform* first_child{};
        std::uint32_t child_count{};
        CameraTransform* next_sibling{};
        CameraTransform* previous_sibling{};
        RenderNodeRootList* root_list{};
        std::uint32_t valid_flags{};
        std::uint32_t auxiliary_flags{};
        void* notification_context{};
        CameraMatrix view{}, local{}, world{};
    } owned_;
    std::uint32_t raw_node_key_{};
    SceneAttachmentRuntime* hierarchy_runtime_{};
    friend class SceneAttachmentRuntime;
    friend class CameraTransformLink;
public:
    CameraTransform() noexcept;
    CameraTransform(CameraTransformBacking, void (*actual_notify_changed)(void*)) noexcept;
    // Value construction owns diagnostic storage; assignment preserves target
    // backing/resolver ownership. All four link reads and target-domain checks
    // precede assignment stores. Invalid host links throw without partial
    // assignment; live backing remains required. Moving does not detach links.
    CameraTransform(const CameraTransform&);
    CameraTransform(CameraTransform&&);
    CameraTransform& operator=(const CameraTransform&);
    CameraTransform& operator=(CameraTransform&&);
    CameraTransformLink parent; // native+30, actual node address when bound
    CameraTransformLink first_child; // +34; caller supplies consistent sibling links
    std::uint32_t& child_count; // +38; maintained by recovered hierarchy operations
    CameraTransformLink next_sibling; // +3C
    CameraTransformLink previous_sibling; // +40
    RenderNodeRootList*& root_list; // +A4; borrowed root registration, distinct from parent
    std::uint32_t& valid_flags; // native+5C
    std::uint32_t& auxiliary_flags; // +138
    // Explicit adapter for attached+A0 virtual+3C. Null invoke means no attachment.
    void*& notification_context;
    void (*notify_changed)(void*){};
    CameraMatrix& view;
    CameraMatrix& local;
    CameraMatrix& world; // native+60,+B0,+F0
    // Stable identity of raw backing, zero for standalone diagnostic storage.
    // This accessor never reads backing bytes, including after native teardown.
    std::uint32_t raw_node_key() const noexcept { return raw_node_key_; }
};
struct CameraState {
private:
    struct OwnedStorage {
        CameraTransform transform;
        CameraProjection projection;
        CameraMatrix view_projection{};
        std::array<float, 3> direction{}, target{};
    } owned_;
public:
    // Default storage is for diagnostics. Binding uses the existing transform
    // and projection views and actual tail fields without reading or initializing
    // them. All supplied views/storage must outlive this state.
    CameraState() noexcept;
    CameraState(CameraTransform&, CameraProjection&, CameraMatrix& view_projection,
        std::array<float, 3>& direction, std::array<float, 3>& target) noexcept;
    // Value construction owns copies; assignment preserves target bindings.
    CameraState(const CameraState&);
    CameraState(CameraState&&);
    CameraState& operator=(const CameraState&);
    CameraState& operator=(CameraState&&);
    CameraTransform& transform;
    CameraProjection& projection;
    CameraMatrix& view_projection; // native+220; valid bit10 in projection flags
    std::array<float, 3>& direction;
    std::array<float, 3>& target; // native+1AC,+1A0; semantic names provisional
};
// Native thiscall, no stack arguments. Refresh always recomputes this world;
// only parent refresh is skipped when its world-valid bit2 is already set.
void refresh_camera_world_00b6db70(CameraTransform&);
const CameraMatrix& get_camera_view_00b6fcb0(CameraTransform&);
const CameraMatrix& get_camera_view_projection_00b70490(CameraState&);
void invalidate_camera_descendants_00b6da30(CameraTransform&);
void set_transform_local_matrix_00b6db10(CameraTransform&, const CameraMatrix&);
void refresh_camera_direction_00b70660(CameraState&);
void set_camera_local_matrix_00b71430(CameraState&, const CameraMatrix&);
void derive_camera_local_from_world_00b6e7e0(CameraTransform&);
void notify_camera_world_changed_00b6dbe0(CameraTransform&);
// Required callback projects native virtual+40; callers select the actual override.
// Flags=2 is assigned only after notification, derivation, descendants and callback.
void set_transform_world_matrix_00b6e870(CameraTransform&, const CameraMatrix&,
    void (&world_changed)(CameraTransform&));
// Same native body with explicit callback context. The callback resolves the
// current virtual+40 at invocation time, after attachment notification and
// hierarchy work; context is borrowed for this synchronous call only.
void set_transform_world_matrix_00b6e870(CameraTransform&, const CameraMatrix&,
    void* callback_context, void (&world_changed)(void*, CameraTransform&));
void set_camera_world_matrix_00b71460(CameraState&, const CameraMatrix&);
}
