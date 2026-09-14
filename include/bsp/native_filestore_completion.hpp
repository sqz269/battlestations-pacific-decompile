#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;
class NativeAdoptedSubstreamDispatch;
struct SingletonLifetimeCallbacks;
struct NativeFileStoreFactoryContext;

// BE7942 calls the callback captured from pending node+14 before its erasure.
// The two ORIGINAL name headers remain borrowed; the callee removes eight bytes.
// Numeric identities must resolve to recovered source, never game code pointers.
class NativeFileStoreCompletionDispatch {
public:
    virtual ~NativeFileStoreCompletionDispatch() = default;
    virtual void invoke_00be7942(std::uint32_t captured_callback,
        const void* first_name, const void* second_name) = 0;
};
struct NativeFileStoreCompletionContext {
    NativeStringStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeAdoptedSubstreamDispatch& streams;
    NativeFileStoreCompletionDispatch& completion;
};

// Actual 0Ch resident pairs contain length/data/retained-stream. Constructors
// clear the header before the identity check; source fields are reread after
// resize. BE6090 reads through the supplied stream-pointer cell; BE6250 copies
// current pair+8. Both publish a nonnull stream before incrementing stream+4.
// ECX destination; name,stream-cell stack/RET8 or pair stack/RET4; EAX output.
void* construct_native_file_store_resident_pair_00be6090(void* output,
    const void* name, const void* stream_pointer_cell, NativeStringStorage&);
void* copy_native_file_store_resident_pair_00be6250(void* output,
    const void* pair, NativeStringStorage&);
// Full BE5CF0[127], ECX pair, RET. Same ordered stream/string destruction as
// BE5D70: release captured stream, clear CURRENT +8 after successful dispatch,
// then release current string. Throwing dispatch releases the current string.
void destroy_native_file_store_resident_pair_00be5cf0(void* pair,
    NativeStringStorage&, NativeAdoptedSubstreamDispatch&);
// Full BE7340[276], ECX tree; output,pair stack, RET8, EAX output. Unique
// case-insensitive insertion; output owner/node/inserted, bytes9..11 untouched.
void* insert_native_file_store_resident_pair_00be7340(void* tree, void* output,
    const void* pair, NativeStringStorage&, const SingletonLifetimeCallbacks&);

// Full BE7760[321], ECX store, name/stream stack, RET8. Normalize a copy,
// skip existing key without retaining stream; otherwise retain through both
// temporary pairs and the resident insertion. The fresh path requires a real
// nonnull intrusive stream. The original diagnostic target4254B0 is RET-only.
void add_native_file_store_file_00be7760(void* store, const void* name,
    void* stream, NativeFileStoreCompletionContext&);
// Full BE78B0[207], ECX store, first/second/stream stack, RET0C. Normalize
// first, find pending callback, erase pending iterator, AddFile, then call the
// captured callback with both original headers. No callback-null guard exists.
void complete_native_file_store_file_00be78b0(void* store,
    const void* first_name, const void* second_name, void* stream,
    NativeFileStoreCompletionContext&);
// Full BE7B20[31], stream/first/second stack, RET0C. Reacquire CURRENT factory
// through4FC150 and its CURRENT cache+8; never remember the submitting store.
void dispatch_native_file_store_completion_00be7b20(void* stream,
    const void* first_name, const void* second_name,
    NativeFileStoreFactoryContext&, NativeFileStoreCompletionContext&);

// Explicit C++ source services, not original ABI/FH3 replacements. Borrowed
// NativeStringStorage has returning/noexcept release; hardware faults, mutable
// native EH spill aliases and original CRT exception identity remain outside
// this source domain. No implicit request, queue cancellation or loader exists.
} // namespace bsp
