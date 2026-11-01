#pragma once
#include <cstdint>
#include <exception>
namespace bsp {
namespace game { class GameNativeReadOnlyData; }
struct NativeStringRawPoolContext;
struct NativeTextureAtlasItemConstructFrame;
struct NativeTextureAtlasItemDestroyFrame;
// Genuine same pool/live feature and immutable PE dot literal. Cold metadata
// only. Retain this exact provider graph through partial item/atlas retirement.
class NativeTextureAtlasItemStorageContext final {
public:
    NativeTextureAtlasItemStorageContext(const game::GameNativeReadOnlyData&,
        NativeStringRawPoolContext&, const volatile std::uint32_t& actual_feature_0109eea4);
private:
    friend void* construct_native_texture_atlas_item_00aee4d0(void*,
        volatile std::uint32_t (&)[3], NativeTextureAtlasItemConstructFrame&,
        NativeTextureAtlasItemStorageContext&);
    friend void destroy_native_texture_atlas_item_00aee220(void*,
        NativeTextureAtlasItemDestroyFrame&, NativeTextureAtlasItemStorageContext&);
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t& feature_;
    const void* const dot_00ce3a70_;
};
enum class NativeTextureAtlasItemStoragePhase { fresh, entered, complete, failed };
struct NativeTextureAtlasItemConstructFrame final {
    NativeTextureAtlasItemConstructFrame() {} // Native homes/preimages unseeded.
    NativeTextureAtlasItemConstructFrame(const NativeTextureAtlasItemConstructFrame&) = delete;
    NativeTextureAtlasItemConstructFrame& operator=(const NativeTextureAtlasItemConstructFrame&) = delete;
    NativeTextureAtlasItemConstructFrame(NativeTextureAtlasItemConstructFrame&&) = delete;
    NativeTextureAtlasItemConstructFrame& operator=(NativeTextureAtlasItemConstructFrame&&) = delete;
    std::uint32_t dot_header[2], preserve_home;
    // Source history below; no Native ownership/cleanup permission.
    NativeTextureAtlasItemStoragePhase phase{NativeTextureAtlasItemStoragePhase::fresh};
    void* actual_item{};
    void* captured_dot_data{};
    std::uint32_t active_site{}, dot_index{};
    std::int32_t native_state{-1};
    std::exception_ptr failure;
};
struct NativeTextureAtlasItemDestroyFrame final {
    NativeTextureAtlasItemStoragePhase phase{NativeTextureAtlasItemStoragePhase::fresh};
    void* actual_item{};
    std::uint32_t active_site{};
    std::int32_t native_state{-1};
    bool name_returned{}, descriptor_returned{};
    std::exception_ptr failure;
    NativeTextureAtlasItemDestroyFrame() = default;
    NativeTextureAtlasItemDestroyFrame(const NativeTextureAtlasItemDestroyFrame&) = delete;
    NativeTextureAtlasItemDestroyFrame& operator=(const NativeTextureAtlasItemDestroyFrame&) = delete;
};
// Whole ordinary NORMAL AEE4D0 payload, original ECX actual30h, three mutable
// DWORD HOMEs [name8, borrowed actual texture, descriptor8]/RET0C, EAX item.
// Zero descriptor0/4 then nameC/10 before capturing descriptor HOME2. Name HOME0
// is captured only after descriptor copying. Texture HOME1 is reloaded only at
// the final +08 store, after every resize/copy/dot-return/fill provider effect.
// Descriptor/name payload/header aliases observe the reached native reads;
// homes remain real initialized ordinary DWORDs, borrowed via volatile views.
// Genuine exact41DD40/BF7680/BF79F0 use the SAME live feature, including reached
// copycount0. Dot DATA is captured before reverse-find; normal return uses that
// captured block with CURRENT dot LEN+1. Other reached source operands stay
// current. Dot excludes index0 through genuine467CF0; UV/packed bytes untouched.
//
// Caller proves real actual30h backing, raw8 headers/bytes, actual pool/feature/
// allocator/CPU/DF domains and borrowed texture lifetime. This creates no
// allocation, canonical texture companion/refcount, actual atlas/Page custody,
// typed semantic item or successful C++ destructor policy. Frame/HOMEs/private
// controls are separate/disjoint from item/source/provider-write extents; item
// and source aliases remain only where genuine lower helper contracts admit.
// Fresh retained Frame; any CPP escape records failed state/barethrows without
// new outer unwind/rollback/free/replay. Retain partial item/dot/pool/provider
// debt externally. NORMAL-only; Original FH3/SEH/Origin/ABI/runtime held.
void* construct_native_texture_atlas_item_00aee4d0(void* actual_item30,
    volatile std::uint32_t (&actual_argument_homes)[3],
    NativeTextureAtlasItemConstructFrame&, NativeTextureAtlasItemStorageContext&);
// Whole ordinary NORMAL AEE220: CURRENT name DATA+10 then LEN+C return, then
// CURRENT descriptor DATA+4/LEN+0 return. Genuine raw41DD20 performs the same
// pool capture; do not clear stale headers, decrement texture or free item.
// Destruction frame and full remaining graph survive any escape; no retry,
// extra cleanup/Origin/terminal/NoThrow authority follows from source history.
void destroy_native_texture_atlas_item_00aee220(void* actual_item30,
    NativeTextureAtlasItemDestroyFrame&, NativeTextureAtlasItemStorageContext&);
} // namespace bsp
