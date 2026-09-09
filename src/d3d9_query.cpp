#include "bsp/d3d9_query.hpp"

namespace bsp {
D3D9OcclusionQuery::~D3D9OcclusionQuery() {
    // New owner cleanup only: native destructor also unregisters via renderer.
    release_occlusion_query_00b5fe20(*this);
}

HRESULT restore_occlusion_query_00b5fe60(D3D9OcclusionQuery& owner, IDirect3DDevice9& device) {
    if (owner.query) return D3DERR_INVALIDCALL;
    static_assert(D3DQUERYTYPE_OCCLUSION == 9);
    IDirect3DQuery9* created = nullptr;
    const HRESULT result = device.CreateQuery(D3DQUERYTYPE_OCCLUSION, &created);
    if (FAILED(result) || !created) {
        if (created) created->Release();
        return FAILED(result) ? result : E_POINTER;
    }
    owner.query = created; // Adopt CreateQuery's reference, without extra AddRef.
    return result;
}

HRESULT create_occlusion_query_00b5fe90(D3D9OcclusionQuery& owner, IDirect3DDevice9& device) {
    if (owner.query) return D3DERR_INVALIDCALL;
    owner.field_08 = 1;
    owner.field_0c = 0;
    return restore_occlusion_query_00b5fe60(owner, device);
}

void release_occlusion_query_00b5fe20(D3D9OcclusionQuery& owner) {
    if (owner.query) {
        owner.query->Release();
        owner.query = nullptr;
    }
}

bool begin_occlusion_query_00b5fc30(D3D9OcclusionQuery& owner) {
    if (owner.field_08 == 0 || !owner.query) return true;
    static_assert(D3DISSUE_BEGIN == 2);
    return owner.query->Issue(D3DISSUE_BEGIN) == S_OK;
}

bool end_occlusion_query_00b5fc60(D3D9OcclusionQuery& owner) {
    if (!owner.query || owner.field_08 == 0) return true;
    static_assert(D3DISSUE_END == 1);
    const HRESULT result = owner.query->Issue(D3DISSUE_END);
    owner.field_08 = result != S_OK ? 1u : 0u;
    return result == S_OK;
}

bool poll_occlusion_query_00b5fca0(D3D9OcclusionQuery& owner) {
    if (!owner.query) return true;
    static_assert(D3DGETDATA_FLUSH == 1);
    const HRESULT result = owner.query->GetData(&owner.field_0c, 4, D3DGETDATA_FLUSH);
    if (result == S_OK) owner.field_08 = 2;
    return result == S_OK;
}

std::uint32_t occlusion_query_samples_00b5fce0(const D3D9OcclusionQuery& owner) noexcept {
    return owner.field_0c;
}

void D3D9QueryRegistry::append_00b27c20_fragment(D3D9OcclusionQuery& query) {
    queries_.push_back(&query); // No intrusive or COM retain at native append.
}

bool D3D9QueryRegistry::remove_00b25290(const D3D9OcclusionQuery* query) noexcept {
    for (std::size_t i = 0; i < queries_.size(); ++i) {
        if (queries_[i] != query) continue;
        if (i + 1 != queries_.size()) queries_[i] = queries_.back();
        queries_.pop_back();
        return true;
    }
    return false;
}

void D3D9QueryRegistry::release_00b262c0_fragment() {
    for (std::size_t i = 0; i < queries_.size(); ++i)
        release_occlusion_query_00b5fe20(*queries_[i]);
}

HRESULT D3D9QueryRegistry::restore_00b23b10_fragment(IDirect3DDevice9& device) {
    HRESULT first_failure = S_OK;
    for (std::size_t i = 0; i < queries_.size(); ++i) {
        const HRESULT result = restore_occlusion_query_00b5fe60(*queries_[i], device);
        if (FAILED(result) && SUCCEEDED(first_failure)) first_failure = result;
    }
    return first_failure;
}
}
