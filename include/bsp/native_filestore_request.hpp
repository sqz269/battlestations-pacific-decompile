#pragma once

#include "bsp/native_vfs_date_leaf_providers.hpp"

#include <memory>

namespace bsp {
class NativeStringStorage;

// Current-manager operations remain explicit. Name resolution may replace the
// actual eight-byte local header. Submission must copy names before returning
// if their storage is needed later. Callback words are native identities, not
// host-callable pointers; completion BE7B20 reacquires the CURRENT factory/store.
class NativeFileStoreRequestDispatch {
public:
    virtual ~NativeFileStoreRequestDispatch() = default;
    virtual bool resolve_existing_name_00bdf4c0(void* manager, void* mutable_name) = 0;
    virtual bool open_file_overlapped_00bdda10(void* manager,
        const void* first_name, const void* second_name,
        std::uint32_t completion_address, std::uint32_t flags) = 0;
    // BD9E30 calls manager+90 with ECX still manager and no stack arguments.
    // Its own discarded stack DWORD is NOT forwarded to this callback.
    virtual void invoke_manager_failure_callback(std::uintptr_t target, void* manager) = 0;
};

struct NativeFileStoreRequestContext {
    NativeStringStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    void* volatile& actual_published_0109ceec;
    NativeFileStoreRequestDispatch& dispatch;
};

enum class NativeFileStoreRequestPhase {
    fresh, normalizing, resolving, resident, pending, inserting, submitting,
    rollback, notifying, complete, failed
};
// Publish this frame BEFORE invoking RequestFile. Interrupted nested resolver/
// provider calls may retain pointers into it. Failed frames and their borrowed
// context, store and original-name storage must remain alive; no replay or
// guessed cleanup is provided. Destruction of a failed frame terminates.
class NativeFileStoreRequestAcquired final {
public:
    NativeFileStoreRequestAcquired();
    ~NativeFileStoreRequestAcquired();
    NativeFileStoreRequestAcquired(const NativeFileStoreRequestAcquired&) = delete;
    NativeFileStoreRequestAcquired& operator=(const NativeFileStoreRequestAcquired&) = delete;
    NativeFileStoreRequestPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t failure_site() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool request_native_file_store_00be7cd0(void*, const void*,
        std::uint32_t, NativeFileStoreRequestContext&, NativeFileStoreRequestAcquired&);
};

// Actual pending-tree search bodies. Native ECX tree/name stack/RET4 for
// bounds; ECX tree/output,name stack/RET8/EAX output for find. BF6713 can return.
void* lower_bound_native_file_store_pending_name_00be5530(void* tree, const void* name);
void* upper_bound_native_file_store_pending_name_00be5630(void* tree, const void* name);
void* find_native_file_store_pending_name_00be5f00(void* tree, void* output,
    const void* name, const SingletonLifetimeCallbacks&);
// Native ECX iterator, RET or invalid-parameter tail; end decrements to max.
void decrement_native_file_store_pending_iterator_00be4db0(void* iterator,
    const SingletonLifetimeCallbacks&);
// Native six stack DWORDs, cdecl RET; final DWORD ignored. Count wraps in place.
void distance_native_file_store_pending_iterators_00be5750(void* first_owner,
    void* first_node, void* last_owner, void* last_node, volatile std::uint32_t* count,
    std::uint32_t ignored, const SingletonLifetimeCallbacks&);

// Complete pending pair/node construction. Actual pair is length/data/callback
// at +0/+4/+8. Constructors clear first, copy current post-resize data, then
// copy callback scalar. No callback retain/release or null guard.
void* construct_native_file_store_pending_pair_00be6120(void* output,
    const void* name, const volatile std::uint32_t* callback, NativeStringStorage&);
void* copy_native_file_store_pending_pair_00be62e0(void* output,
    const void* pair, NativeStringStorage&);
void* construct_native_file_store_pending_node_00be63e0(void* node, void* left,
    void* parent, void* right, const void* pair, std::uint8_t color, NativeStringStorage&);
void* allocate_native_file_store_pending_node_00be6630(void* left, void* parent,
    void* right, const void* pair, std::uint8_t color, NativeStringStorage&);
// Native ECX tree; output,left,parent,pair stack/RET10; EAX output.
// Count >=15555554h throws the existing owning native length-error transport.
void* link_native_file_store_pending_node_00be6ee0(void* tree, void* output,
    std::uint8_t insert_left, void* parent, const void* pair, NativeStringStorage&);
// Native ECX tree; output,pair stack/RET8; EAX output. Output is owner/node/
// inserted byte, with bytes9..11 untouched. Duplicate callback is not replaced.
void* insert_native_file_store_pending_pair_00be7460(void* tree, void* output,
    const void* pair, NativeStringStorage&, const SingletonLifetimeCallbacks&);
// Native ECX pending TREE (store+20); name stack/RET4; EAX removed count.
std::uint32_t erase_native_file_store_pending_name_00be79c0(void* tree,
    const void* name, NativeStringStorage&, const SingletonLifetimeCallbacks&);

// Complete BD9E30..BD9E3A: native ECX manager, discarded DWORD stack, RET4.
void notify_native_vfs_request_failure_00bd9e30(void* manager,
    std::uint32_t discarded, NativeFileStoreRequestDispatch&);
// Complete BE7CD0..BE7F6F. Native ECX actual store, original name header and
// callback DWORD stack, AL boolean, RET8. Resolve before duplicate checks;
// resident/pending duplicates return true without storing/invoking callback.
// Insert pending before submit(resolved,original,BE7B20,2). False submission
// erases matching pending range then calls current manager+90; exceptions from
// submission retain the pending record. User callback lifetime is external.
bool request_native_file_store_00be7cd0(void* actual_store,
    const void* original_name, std::uint32_t callback_address,
    NativeFileStoreRequestContext&, NativeFileStoreRequestAcquired&);

// New explicit-service source APIs, not native ABI replacements. Actual tree,
// string and manager storage is borrowed. Original FH3/SEH and arbitrary stack
// aliasing, concurrent mutation, callback-code execution and gameplay unproved.
} // namespace bsp
