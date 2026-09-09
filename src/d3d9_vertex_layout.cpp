#include "bsp/d3d9_vertex_layout.hpp"
#include <cstdlib>

namespace bsp {
void D3D9VertexLayout::append_stream_00b48a00(std::shared_ptr<VertexDeclaration> declaration) {
    if (count_ >= streams_.size()) std::abort();
    streams_[count_++] = {std::move(declaration), 1, 0};
}
void D3D9VertexLayout::recompute_stride_00b47d60() {
    stride_ = 0;
    for (UINT stream = 0; stream < count_; ++stream) {
        if (!streams_[stream].declaration) std::abort();
        stride_ += streams_[stream].declaration->stride;
    }
}
std::vector<D3DVERTEXELEMENT9> D3D9VertexLayout::elements() const {
    std::array<UINT, 14> occurrences{}; // Native counters persist across streams.
    std::vector<D3DVERTEXELEMENT9> result;
    for (UINT stream = 0; stream < count_; ++stream) {
        if (!streams_[stream].declaration) std::abort();
        for (const auto& element : streams_[stream].declaration->elements()) {
            if (element.usage >= occurrences.size()) std::abort();
            result.push_back({static_cast<WORD>(stream), static_cast<WORD>(element.offset),
                static_cast<BYTE>(element.type), static_cast<BYTE>(element.method),
                static_cast<BYTE>(element.usage), static_cast<BYTE>(occurrences[element.usage]++)});
        }
    }
    const D3DVERTEXELEMENT9 end = D3DDECL_END();
    result.push_back(end);
    return result;
}
HRESULT D3D9VertexLayout::create_if_missing_00b60a10(IDirect3DDevice9& device) {
    if (native_) return S_FALSE;
    const auto packed = elements();
    const HRESULT result = device.CreateVertexDeclaration(packed.data(), &native_);
    recompute_stride_00b47d60(); // Native updates this after the API call regardless of HRESULT.
    return result;
}
}
