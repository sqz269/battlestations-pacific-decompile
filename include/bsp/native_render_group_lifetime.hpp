#pragma once
#include "bsp/native_render_group_storage.hpp"

namespace bsp {
class NativeModelReference;
class SizedStoragePool;

// Pure canonical association: one stable companion for each actual model slot
// stored in group+1C/+20. No logical release, reference adjustment or collection
// mutation belongs in lookup. It may reject an unbound identity. The concrete
// reference must borrow the SAME raw model prefix and actual +04 atomic.
class NativeRenderGroupModels {
public:
    virtual ~NativeRenderGroupModels() = default;
    virtual NativeModelReference& resolve_actual_model(void* actual_slot) = 0;
};

// Original ECX=actual4Ch group; RET. Release/clear actual binding, then reload
// and logically release models1C/20 through their concrete native companions;
// clear each field AFTER its call. Destroy source arrays30 then24, then return
// current pooled name without clearing its fields. Does not free the group.
// The model companions perform real shared hierarchy/lifetime/current-profile
// operations and may return their actual188h slots before this call resumes.
// Unwind cleans remaining array/name storage only; it does not release unvisited
// bindings/models. All backing/domain associations must outlive their last use.
void destroy_native_render_group_00b1d760(NativeRenderGroupStorage&,
    NativeRenderActualOwners& actual_binding_owners, NativeRenderGroupModels&,
    SizedStoragePool& actual_string_pool);

// Original ECX=actualgroup; stack flags; EAX=original pointer; RET4. Destroy
// first, then ordinary-free iff flags&1. A throwing destructor prevents free.
// The returned value may already designate deallocated storage.
NativeRenderGroupStorage* delete_native_render_group_00b1d8e0(
    NativeRenderGroupStorage*, NativeRenderActualOwners&, NativeRenderGroupModels&,
    SizedStoragePool&, std::uint32_t flags);

} // namespace bsp
