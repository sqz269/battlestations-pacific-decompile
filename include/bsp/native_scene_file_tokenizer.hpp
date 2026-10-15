#pragma once
#include "bsp/native_scene_tokenizer.hpp"

namespace bsp {
class NativeVfsRuntimeBindings;

// Concrete Source services for actual VFS owners. Numeric stream tables remain
// original data identities; no fake stream/table or callable fallback is made.
// Allocation/free inherit the accepted current-CRT tokenizer heap services.
class NativeSceneFileTokenizerVfsCalls final : public NativeSceneTokenizerCalls {
public:
    explicit NativeSceneFileTokenizerVfsCalls(NativeVfsRuntimeBindings&) noexcept;
    void* open(void* actual_manager, const NativeString& actual_name,
        std::uint32_t full_flags);
    std::int64_t stream_size_slot30(void*, NativeRetainedMemoryOwnerContext&) override;
    void stream_read_slot24(void*, void*, std::uint32_t, std::uint32_t*,
        NativeRetainedMemoryOwnerContext&) override;
    void stream_zero_reference_slot0(void*, NativeRetainedMemoryOwnerContext&) override;
private:
    NativeVfsRuntimeBindings& bindings_;
};

struct NativeSceneFileTokenizerContext {
    NativeStringRawPoolContext& strings;
    NativeRetainedMemoryOwnerContext& memory;
    NativeSceneFileTokenizerVfsCalls& calls;
    void* const volatile& actual_vfs_publication_0109ceec;
    const char* const volatile& default_delimiters_00e0c944;
    const volatile std::uint32_t& actual_crt_feature_word_0109eea4;
};

// Whole ordinary008D9CF0. Original: ECX actual838h owner; entry stack+4/+8
// ONE owning8h NativeString by value, +0C extras; EAX owner, RET0C.
// New Source interface: consumed_filename is a genuine owning NativeString
// object supplying that parameter's ownership, NOT a borrowed name. This call
// releases its CURRENT buffer without clearing its header. Do not release it
// again; NativeString's Source destructor is trivial. Member820/824 owns a
// separate copy. The actual self-header equality branch is retained.
// Open CURRENT published VFS with mode32; acquire stream828 without AddRef.
// Null open leaves82C/830/834 untouched (818 is zero); later safe scan/destruct
// needs valid caller preimages/policy. No empty-buffer repair is manufactured.
// Source C++ exceptions attempt only the proven member/incoming string cleanup;
// no stream/text/delimiter rollback. Raw cleanup can itself fail. Keep actual
// owner/storage/contexts alive to resolve all remaining ownership explicitly.
// Supported actual VFS profiles are those of NativeVfsRuntimeBindings. Use the
// SAME concrete calls/raw strings/memory/defaults for subsequent tokenizer
// operations and destruction. Successful stable/disjoint/coherent current-CRT
// storage, finite valid strings/nonoverflowing extents and native CPU/DF domain.
// No Original ABI/FS/EH/privateCRT/class/runtime/game or app-startup claim.
void* construct_native_scene_file_tokenizer_008d9cf0(void* actual_838h_owner,
    NativeString& consumed_filename, const char* extra_delimiters,
    NativeSceneFileTokenizerContext&);
} // namespace bsp
