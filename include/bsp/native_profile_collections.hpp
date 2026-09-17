#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Source CRT allocation/free bindings and the actual raw score-record destructor.
// No synthetic record projection or successful destruction fallback.
struct NativeProfileCollectionCalls {
    virtual ~NativeProfileCollectionCalls()=default;
    virtual void* allocate_00bf681b(std::uint32_t bytes);
    virtual void* allocate_00bf55be(std::uint32_t bytes);
    virtual void free_00bf65ac(void* allocation);
    virtual void call_00593570(void* actual_score_record,NativeStringStorage&);
};

// Native library storage contracts: allocate1Ch/2Ch/2A0h, initialize links
// +0/+4/+8 and color/nil +18/+19, +28/+29, +29C/+29D respectively. No payload
// or padding initialization. Original leaves have no consumed inputs/EAX/RET;
// source interfaces inject the allocation domain, not original binary ABI.
void* allocate_native_profile_counter_node_005826b0(NativeProfileCollectionCalls&);
void* allocate_native_profile_transient_node_007f8540(NativeProfileCollectionCalls&);
void* allocate_native_mission_score_node_00907ca0(NativeProfileCollectionCalls&);

// 58B520: ECX tree, stack node/RET4. Capture key data before left, then length
// before release, free current, follow the captured left; right recursion first.
void erase_native_profile_counter_subtree_0058b520(void* tree,void* node,
    NativeStringStorage&,NativeProfileCollectionCalls&);
// 7F89F0: ECX pair, RET. Release string +14/+18 before string +0/+4. Each
// pointer/length is reloaded when reached; retain all header/opaque bytes.
void destroy_native_profile_transient_pair_007f89f0(void* pair,NativeStringStorage&);
// 7FA880: ECX tree, stack node/RET4. Capture left BEFORE destroying pair+0C,
// then free and follow that captured left. Node nil byte+29.
void erase_native_profile_transient_subtree_007fa880(void* tree,void* node,
    NativeStringStorage&,NativeProfileCollectionCalls&);
// 7FD510: same traversal over2A0h nodes, nil+29D. Captured left survives score
// record+14 destruction; then reread/release key+0C and free node. The payload
// destructor now has a concrete raw-storage default, including nested maps.
void erase_native_mission_score_subtree_007fd510(void* tree,void* node,
    NativeStringStorage&,NativeProfileCollectionCalls&);

// Only the full-range paths reached by7FD780's current begin/end pairs.
// Reset current root/count/left/right after cleanup, then write output owner/node.
// These do not expose or replace the library's general checked partial erase.
void* clear_native_profile_counter_full_range_0058d860(void* tree,void* output,
    NativeStringStorage&,NativeProfileCollectionCalls&);
void* clear_native_mission_score_full_range_007fd5f0(void* tree,void* output,
    NativeStringStorage&,NativeProfileCollectionCalls&);

// Normal storage/call schedules only. Original FH3 cleanup of key/payload,
// native CRT/pool exception identity and concurrent mutation are unproved.
} // namespace bsp
