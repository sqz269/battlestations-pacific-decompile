#pragma once
#include "bsp/d3d9_startup.hpp"
#include "bsp/vertex_declaration.hpp"
#include <array>
#include <memory>
#include <vector>

namespace bsp {
class D3D9VertexLayout {
public:
    D3D9VertexLayout() = default;
    D3D9VertexLayout(const D3D9VertexLayout&) = delete;
    D3D9VertexLayout& operator=(const D3D9VertexLayout&) = delete;
    ~D3D9VertexLayout() { if (native_) native_->Release(); }
    void append_stream_00b48a00(std::shared_ptr<VertexDeclaration> declaration);
    void recompute_stride_00b47d60();
    // Partial 00b60a10: conversion/API call/stride update; diagnostic singleton omitted.
    HRESULT create_if_missing_00b60a10(IDirect3DDevice9& device);
    IDirect3DVertexDeclaration9* native() const { return native_; }
    UINT stride() const { return stride_; }
    std::vector<D3DVERTEXELEMENT9> elements() const;
private:
    struct Stream {
        std::shared_ptr<VertexDeclaration> declaration;
        UINT frequency{1}; // Native stream record +4h.
        UINT extra{};      // Native stream record +8h.
    };
    std::array<Stream, 4> streams_{};
    UINT count_{};  // Native layout +38h.
    UINT stride_{}; // Native layout +3ch, sum across streams.
    IDirect3DVertexDeclaration9* native_{}; // Native +40h.
};
}
