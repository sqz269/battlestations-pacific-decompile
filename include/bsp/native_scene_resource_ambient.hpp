#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_texture_surface_getter.hpp"

namespace bsp {

struct NativeSceneResourceAmbientContext {
    // Same canonical actual-count registry used by every reachable ambient.
    // Resolve only after the captured old owner's imported decrement returns0.
    // Its existing companion must dispatch CURRENT virtual0 genuinely; no
    // admission, copied count, default callback or unknown-profile fallback.
    NativeRenderActualOwnerRegistry& owners;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
};
struct NativeSceneResourceAmbientFrame {
    // The same native stacked scene-argument DWORD is reused by remove/append.
    // Caller owns its preimage/address. Set immediately before each reached
    // wrapper; reserve callbacks may mutate it before append's CURRENT read.
    volatile std::uint32_t scene_argument;
};

// B7BD50[16B], ECX actual98h ambient, stacked raw scene identity, AL result,
// RET4. No scene dereference/retain/release. Empty array does not read the key.
// The valid, nonoverflowing12B array has nonnegative count<=capacity, accessible
// disjoint backing, and no asynchronous mutation. The existing B7B620 first-
// match/swap-last leaf is reused; upper EAX/private-frame identity is separate.
bool remove_native_ambient_scene_00b7bd50(void* actual_ambient,
    const volatile std::uint32_t& actual_scene_argument);

// B7BF90[58B], same ambient/raw scene argument, RET4. Equality count==capacity
// reaches existing genuine B7B390 reserve in the same CRT domain. Double the
// capacity modulo32, signed<=1 becomes1. After reserve reread count then begin;
// only a nonzero computed destination reads the CURRENT scene argument/stores
// it. Increment the CURRENT count modulo32 afterward, including null destination.
// Growth requires valid backing/count/capacity and a successful disjoint CRT
// allocation; allocator callbacks may alter current fields/argument while
// retaining that contract. No key is ever used as a SceneResource object.
void append_native_ambient_scene_00b7bf90(void* actual_ambient,
    const volatile std::uint32_t& actual_scene_argument);

// Complete89B B825D0; native ECX actual3Ch resource, stacked ambient pointer,
// RET4. Only resource+10 is accessed. Remove its initial currentambient backlink
// BEFORE capturing current requested cell and current old+10. Changed pointer:
// publish, current increment(captured requested+4), current decrement(captured
// old+4), genuine CURRENT0 dispatch only atzero. Finally reread resource+10
// and append there. Same-pointer still removes/appends, with no ref operations.
// Imports/canonical companions may mutate resource fields, import cells and
// caller argument cells. The requested cell is distinct from frame.scene_argument.
// Frame/context addresses remain stable and disjoint
// from native owner/array storage; no native caller-stack ABI is asserted.
// Allocation failure during append leaves publication/previous count effects
// intact. No rollback, admission or extra reference credit is introduced.
void set_native_scene_resource_ambient_00b825d0(void* actual_resource,
    const volatile std::uint32_t& requested_ambient_argument,
    NativeSceneResourceAmbientFrame&, NativeSceneResourceAmbientContext&);

} // namespace bsp
