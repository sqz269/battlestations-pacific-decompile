#pragma once
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw renderer texture lookup requires MSVC Win32.
#endif

namespace bsp {
struct NativeTextureCacheContext;
struct NativeTextureCacheAcquired;
struct NativeStringRawPoolContext;

// All borrows are genuine existing domains, prepared BEFORE Native entry.
// strings must use the SAME actual pool/manager/gate cells as cache.strings;
// feature is the SAME authenticated mutable CRT cell, not a copied value.
struct NativeRendererTextureFindStorageContext {
    NativeTextureCacheContext& cache;
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t& feature_word_0109eea4;
};

// Ordinary actual DWORD backing: raw string helpers operate on these SAME
// objects. Reached local reads/writes use volatile views. Establish authentic
// preimages before entry: skipped guard entry writes NEITHER saved DWORD,
// while a reached enter writes only token's LOW BYTE. Do not blanket zero.
// Header identity may equal the reached input header; preserve that alias.
// preserve_home is written to DWORD1 only at the reached resize call.
struct NativeRendererTextureFindStorageFrame {
    std::uint32_t header[2];
    std::uint32_t guard_token;
    std::uint32_t guard_renderer;
    std::uint32_t preserve_home;
};

// One retained invocation, no owner/count/cleanup or permission. The existing
// cache record and ALL its resolver/loader/fallback/provider backing must be
// fresh/distinct for this reached call, including allow_load0 miss paths.
// No optional null/hot-hit subset, record pool, reset or retry is supplied.
struct NativeRendererTextureFindStorageAcquired final {
    enum class Phase { fresh, entered, complete, failed };
    explicit NativeRendererTextureFindStorageAcquired(NativeTextureCacheAcquired& cache)
        : cache_(cache) {}
    NativeRendererTextureFindStorageAcquired(const NativeRendererTextureFindStorageAcquired&) = delete;
    NativeRendererTextureFindStorageAcquired& operator=(const NativeRendererTextureFindStorageAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::int32_t native_eh_state{-1}; // diagnostic, never Native unwind authority
    std::uint32_t active_native_site{};
    void* renderer{};
    const volatile std::uint32_t* home_identity{}; // history only after reuse/return
    const void* name_header{};
    void* captured_buffer{};
    std::uint32_t post_load_word{};
    void* result{}; // identity only after any provider may end its lifetime
    bool guard_enter_returned{}, resize_returned{}, copy_returned{};
    bool cache_returned{}, buffer_return_entered{}, buffer_returned{};
    bool guard_leave_entered{}, guard_leave_returned{};
    std::exception_ptr failure; // snapshot diagnostic, NOT exception-origin identity
private:
    friend void* find_native_renderer_texture_storage_00b31ac0(void*,
        volatile std::uint32_t (&)[2], NativeRendererTextureFindStorageContext&,
        NativeRendererTextureFindStorageFrame&, NativeRendererTextureFindStorageAcquired&);
    NativeTextureCacheAcquired& cache_;
};

// Whole normal B31AC0..B31BC0 [257B/77ops]. Original ECX is the captured
// renderer, stacked homes are [name-header pointer, post-load word], RET8,
// EAX texture. Name HOME is first read AFTER optional guard; word HOME only
// AFTER lowercase. Both remain mutable actual homes, never entry snapshots.
// Guard's separately CURRENT F8D394 receiver is not the incoming receiver.
// Cache is incoming receiver+1A74 with acquire_new0/allow_load0.
//
// Caller retains live actual renderer/cache/string/guard/import/feature and
// mutable argument/header/local backing, including authentic skipped-guard
// preimages and every reached/transitive alias/lifetime. Metadata/context/
// acquisition are disjoint from payload; no guard proves that here. Physical
// Native private-stack aliases/ABI and fault/SEH/unmasked-FP are unproved.
//
// Normal cleanup returns captured post-resize DATA with CURRENT LEN+1, not
// current DATA. Current exit mode and full saved DWORD are read at their
// Native epochs. No semantic GuardScope, 41DD20 substitution, automatic
// leave/free, zero/default, mode repair, validation, rollback or retry.
// Any C++ escape records failed and retains reached effects/ALL backing;
// no Native-origin issuer, selected-scope/secondary policy or unwind runs.
void* find_native_renderer_texture_storage_00b31ac0(void* actual_renderer,
    volatile std::uint32_t (&actual_homes)[2], NativeRendererTextureFindStorageContext&,
    NativeRendererTextureFindStorageFrame&, NativeRendererTextureFindStorageAcquired&);
} // namespace bsp
