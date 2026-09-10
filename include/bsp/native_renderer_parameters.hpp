#pragma once
#include "bsp/native_viewport_owner.hpp"

namespace bsp {

// Exact native renderer+1A14..1A27 region, separate from its presentation
// block at +1A28. Only width/height meanings are used here. This is not the
// complete 1D94h renderer object. No implicit initialization of its padding.
struct NativeRendererParametersOwner final {
    std::uint8_t flag_00;
    std::uint8_t preserved_01[3];
    DWORD value_04;
    std::uint8_t flag_08;
    std::uint8_t preserved_09[3];
    DWORD width_0c;
    DWORD height_10;
};

// Five stores at [00B32512,00B32534), within constructor 00B32410. Native
// ESI is the full renderer and EBX=0. This new ABI accepts its actual region;
// it writes 0,3,0,0,0 at +00,+04,+08,+0C,+10 and preserves all other bytes.
void initialize_native_renderer_parameters_00b32410_fragment(
    NativeRendererParametersOwner&) noexcept;

// The state borrows its CURRENT dispatch; replacement is visible on the next
// getter call. Implementations must return actual region references or throw
// an explicit unsupported-binding error. No dimension/device-size fallback.
class NativeRendererParameterDispatch {
public:
    virtual ~NativeRendererParameterDispatch() = default;
    virtual NativeViewportRendererParameters parameters_00b1ff60(
        D3D9StateCache& captured_renderer) = 0;
};

// Concrete D5F0A8 virtual+30 -> 00B1FF60: native ECX=renderer, EAX=renderer
// +1A14, RET. This new interface binds that exact region to one state identity;
// it returns result+0C/+10 references without reading/copying their values.
// Both borrowed objects must outlive this binding and all getter calls.
class NativeD3D9RendererParameterDispatch final : public NativeRendererParameterDispatch {
public:
    NativeD3D9RendererParameterDispatch(D3D9StateCache& renderer,
        NativeRendererParametersOwner& owner) noexcept : renderer_(renderer), owner_(owner) {}
    NativeViewportRendererParameters parameters_00b1ff60(D3D9StateCache&) override;
private:
    D3D9StateCache& renderer_;
    NativeRendererParametersOwner& owner_;
};

// Resolve only the exact captured state's CURRENT dispatch on each call.
// Renderer publication reloads belong to the native viewport constructor.
class D3D9ViewportRendererAccess final : public NativeViewportRendererAccess {
public:
    NativeViewportRendererParameters parameters_00b1ff60(D3D9StateCache&) override;
};

} // namespace bsp
