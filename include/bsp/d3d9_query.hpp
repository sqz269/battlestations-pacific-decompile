#pragma once
#include "bsp/d3d9_startup.hpp"
#include <cstdint>
#include <vector>

namespace bsp {
// Concrete query wrapper projection. One owned COM reference; no native vtable
// or intrusive count. Query-use meaning of +08h/+0Ch remains unaudited.
struct D3D9OcclusionQuery {
    std::uint32_t field_08{1};
    std::uint32_t field_0c{};
    IDirect3DQuery9* query{}; // Native +10h.
    D3D9OcclusionQuery() = default;
    ~D3D9OcclusionQuery();
    D3D9OcclusionQuery(const D3D9OcclusionQuery&) = delete;
    D3D9OcclusionQuery& operator=(const D3D9OcclusionQuery&) = delete;
};

// Native ECX wrapper, RET; supplied device replaces renderer-singleton lookup.
// Empty COM owner required. Initializes +08h/+0Ch before CreateQuery(type9).
HRESULT create_occlusion_query_00b5fe90(D3D9OcclusionQuery&, IDirect3DDevice9&);
// Native ECX wrapper, RET. One Release then null; other fields untouched.
void release_occlusion_query_00b5fe20(D3D9OcclusionQuery&);
// Native ECX wrapper, RET, no stack device argument. New device dependency is
// explicit. Require empty owner rather than native unchecked pointer overwrite.
// Reports HRESULT and cleans up failure outputs; no state-field reinitialization.
HRESULT restore_occlusion_query_00b5fe60(D3D9OcclusionQuery&, IDirect3DDevice9&);

// Borrowed renderer+19A0h reset-list projection. Unregister before destruction.
// Stable membership, storage, and owner lifetimes required during traversal.
// Parent renderer provides optional guard for factory append/unregistration;
// these fragments have no internal guard, renderer readiness or lost gates.
class D3D9QueryRegistry {
public:
    void append_00b27c20_fragment(D3D9OcclusionQuery&);
    bool remove_00b25290(const D3D9OcclusionQuery*) noexcept;
    std::size_t size() const noexcept { return queries_.size(); }
    const D3D9OcclusionQuery* at(std::size_t index) const { return queries_.at(index); }
    void release_00b262c0_fragment();
    // Attempts every concrete restore callback, returns first failed HRESULT.
    // Native ignores results. Device must stay stable across the batch.
    HRESULT restore_00b23b10_fragment(IDirect3DDevice9&);
private:
    std::vector<D3D9OcclusionQuery*> queries_;
};
}
