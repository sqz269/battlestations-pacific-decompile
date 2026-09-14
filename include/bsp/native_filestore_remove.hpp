#pragma once

namespace bsp {
class NativeStringStorage;
class NativeAdoptedSubstreamDispatch;
struct SingletonLifetimeCallbacks;

// Complete BE7130..BE7209[218]. Native ECX actual2Ch FileStore; stack actual
// eight-byte name header; RET4; no specified semantic result. Normalize a
// copied name and search resident tree+14. Capture returned iterator owner and
// current head before the returning CRT check, then read the current iterator
// node. A hit erases through complete BE6760 using the captured owner/current
// node and the same iterator as output. A miss only reaches RET-only4254B0.
// Erasure owns the resident stream release; this route adds no retain, pending
// removal, not-found exception, Boolean result, or callback notification.
// Always release the completed normalized copy, including on erase failure.
void remove_native_file_store_file_00be7130(void* actual_store,
    const void* actual_name, NativeStringStorage&,
    NativeAdoptedSubstreamDispatch&, const SingletonLifetimeCallbacks&);

// Actual native storage and complete existing string/tree/provider services
// are borrowed. Numeric stream profiles need a concrete source dispatcher such
// as NativeVfsRuntimeBindings. These explicit C++ arguments are not the native
// ABI/FH3 interface; noexcept string release excludes throwing-getter cleanup,
// and arbitrary native EH spill aliases/SEH/gameplay remain unproved.
} // namespace bsp
