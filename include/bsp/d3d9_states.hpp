#pragma once
#include "bsp/d3d9_startup.hpp"
#include "bsp/random_threads.hpp"
#include "bsp/d3d9_buffers.hpp"
#include "bsp/vertex_declaration.hpp"
#include <array>
#include <memory>

namespace bsp {
// Semantic equivalents of globals 0108d6dc/dd/e0. The original counter is
// non-atomic. Configure locking before workers start; live mode changes unverified.
struct RendererSynchronization {
    bool enabled{};
    bool observed_enabled{};
    std::uint32_t nesting{};
};

// Native setter changes both mode bytes, preserving nesting. Call only at a
// coordinated mode transition; this does not stop workers or release held locks.
void set_renderer_synchronization_00b33aa0(RendererSynchronization&, bool enabled);

// Polls the separate device-lifecycle lock stored at native renderer+199Ch.
// This plain signed-depth observation does not acquire a lock or establish
// thread-safe access in this semantic interface.
bool renderer_device_lifecycle_busy_00b20220(const TrackedCriticalSection&);

struct D3D9DrawState {
    std::uint32_t inhibit{}; // Native renderer +1d90h, exact meaning unresolved.
    bool device_lost{};     // Native +1d8ah.
};

// Semantic projections of stream getters, not full native constructors/layouts.
struct LogicalVertexStream {
    std::shared_ptr<VertexBufferBinding> physical; // Native stream +58h.
    std::shared_ptr<VertexDeclaration> declaration; // +68h.
    UINT offset{};       // +5ch.
    UINT vertex_count{}; // +64h.
    DWORD tag{};         // +54h.
    DWORD flags{};       // +60h.
    UINT base_vertex{};  // +70h, used by non-dynamic locks.
    void* mapped{};      // +8h.
};
struct LogicalIndexStream {
    std::shared_ptr<IndexBufferBinding> physical; // Native stream +8h.
    UINT offset{};      // +ch, physical lock's returned byte offset.
    UINT index_count{}; // +14h.
    D3DFORMAT format{D3DFMT_INDEX16}; // +18h.
    UINT base_index{};  // +20h.
};

// Borrowed semantic projections of native logical shader +8h. Neither these
// structs nor the cache retain/release COM or logical objects. Cached objects
// must remain alive until replaced, explicitly unbound, or invalidate() runs.
struct LogicalVertexShader { IDirect3DVertexShader9* shader{}; };
struct LogicalPixelShader { IDirect3DPixelShader9* shader{}; };

// Retained logical identity with a borrowed COM texture. The COM object must
// outlive this projection and every cache reference to it. Native virtual+1Ch
// is projected to this pointer; native intrusive lifetime is not the C++ ABI.
struct LogicalTexture { IDirect3DBaseTexture9* texture{}; };

// New interface, not the original renderer's memory layout. Device, shared sync
// state and optional tracked lock must outlive this object. No COM ownership.
class D3D9StateCache {
public:
    D3D9StateCache(IDirect3DDevice9& device, RendererSynchronization& synchronization,
        TrackedCriticalSection* lock) : device_(device), synchronization_(synchronization), lock_(lock) {}
    void set_render_state_00b24460(D3DRENDERSTATETYPE state, DWORD value);
    void set_sampler_state_00b24610(UINT sampler, D3DSAMPLERSTATETYPE state, DWORD value);
    // Native thiscall RET Ch: start register, float data, float4 count. These
    // new APIs expose HRESULT; S_FALSE means zero count skipped guard and call.
    // Native ignores HRESULT and counts attempted uploads, including failures.
    HRESULT set_vertex_shader_constants_f_00b21820(UINT start_register,
        const float* data, UINT vector_count);
    HRESULT set_pixel_shader_constants_f_00b218c0(UINT start_register,
        const float* data, UINT vector_count);
    // Native thiscall RET4. Guard precedes comparison; non-null logical objects
    // with equal COM values preserve the old pointer. New API returns S_FALSE on a
    // skipped call, otherwise HRESULT (native ignores it and caches failures).
    HRESULT bind_vertex_shader_00b21d10(const LogicalVertexShader* value);
    HRESULT bind_pixel_shader_00b21c20(const LogicalPixelShader* value);
    // Thiscall RET8, logical sampler and retained texture. S_FALSE skips an
    // identical logical pointer; otherwise cache updates even on API failure.
    HRESULT bind_texture_00b24710(UINT sampler, std::shared_ptr<LogicalTexture>);
    std::uint32_t texture_binding_calls() const { return texture_binding_calls_; }
    void initialize_defaults_00b26170();
    void set_stream_frequency_00b24a40(UINT stream, UINT frequency);
    // S_FALSE means the native draw gate skipped the call; native ignores HRESULT.
    HRESULT draw_primitive_00b21b40(const D3D9DrawState&, D3DPRIMITIVETYPE,
        UINT start_vertex, UINT primitive_count);
    void bind_vertex_stream_00b24840(UINT stream, std::shared_ptr<LogicalVertexStream> value);
    void bind_index_stream_00b24b00(std::shared_ptr<LogicalIndexStream> value, INT base_vertex);
    HRESULT draw_indexed_00b24010(const D3D9DrawState&, D3DPRIMITIVETYPE,
        UINT minimum_vertex, UINT vertex_count, UINT start_index, UINT primitive_count);
    HRESULT lock_vertex_stream_00b49980(LogicalVertexStream&, UINT count, UINT offset,
        bool read_only, void*& data);
    void unlock_vertex_stream_00b49a80(LogicalVertexStream&);
    HRESULT lock_index_stream_00b49b60(LogicalIndexStream&, UINT count, UINT offset,
        bool read_only, void*& data);
    void unlock_index_stream_00b49c70(LogicalIndexStream&);
    // Explicit lifecycle for semantic stream projections. Registered pointers must
    // remain valid until unregister; operations use the optional renderer guard.
    void register_logical_stream_00b4b1e0(VertexBufferBinding&, LogicalVertexStream&);
    void register_logical_stream_00b4b1e0(IndexBufferBinding&, LogicalIndexStream&);
    void unregister_vertex_stream_00b4b3f0(VertexBufferBinding&, LogicalVertexStream&);
    void unregister_index_stream_00b4b390(IndexBufferBinding&, LogicalIndexStream&);
    void rewind_vertex_buffer_00b232b0(VertexBufferBinding&);
    void rewind_index_buffer_00b231c0(IndexBufferBinding&);
    std::uint32_t vertex_binding_calls() const { return vertex_binding_calls_; }
    std::uint32_t index_binding_calls() const { return index_binding_calls_; }
    // Needed after device reset; caller owns reset sequencing.
    void invalidate();
    std::uint32_t render_calls() const { return render_calls_; }
    std::uint32_t sampler_calls() const { return sampler_calls_; }
    std::uint32_t vertex_constant_calls() const { return vertex_constant_calls_; }
    std::uint32_t pixel_constant_calls() const { return pixel_constant_calls_; }
    std::uint32_t vertex_constant_bytes() const { return vertex_constant_bytes_; }
    std::uint32_t pixel_constant_bytes() const { return pixel_constant_bytes_; }
    std::uint32_t vertex_shader_calls() const { return vertex_shader_calls_; }
    std::uint32_t pixel_shader_calls() const { return pixel_shader_calls_; }
    const LogicalVertexShader* vertex_shader() const { return vertex_shader_; }
    const LogicalPixelShader* pixel_shader() const { return pixel_shader_; }
private:
    struct Entry { bool valid{}; DWORD value{}; };
    struct Guard;
    bool enter_00b33ad0();
    void leave_00b33b00(bool entered);
    IDirect3DDevice9& device_;
    RendererSynchronization& synchronization_;
    TrackedCriticalSection* lock_;
    // Native render-valid area +40h..113h; sampler states 0..13 in twenty banks.
    std::array<Entry, 212> render_{};
    std::array<std::array<Entry, 14>, 20> samplers_{};
    std::array<UINT, 4> stream_frequencies_{};
    struct StreamBinding {
        std::shared_ptr<LogicalVertexStream> object;
        UINT stride{};
        UINT offset{};
    };
    std::array<StreamBinding, 4> streams_{};
    std::shared_ptr<LogicalIndexStream> indices_;
    INT base_vertex_{};
    std::uint32_t vertex_binding_calls_{};
    std::uint32_t index_binding_calls_{};
    std::array<std::shared_ptr<LogicalTexture>, 20> textures_{};
    std::uint32_t texture_binding_calls_{};
    std::uint32_t render_calls_{};
    std::uint32_t sampler_calls_{};
    // Native +1bd8h/+1bdch calls and +1be0h/+1be4h bytes, modulo 2^32.
    std::uint32_t vertex_constant_calls_{};
    std::uint32_t pixel_constant_calls_{};
    std::uint32_t vertex_constant_bytes_{};
    std::uint32_t pixel_constant_bytes_{};
    const LogicalVertexShader* vertex_shader_{}; // Native renderer +1770h.
    const LogicalPixelShader* pixel_shader_{};   // Native renderer +176ch.
    std::uint32_t vertex_shader_calls_{};        // Native +1bc0h.
    std::uint32_t pixel_shader_calls_{};         // Native +1bbch.
};
}
