#include "bsp/d3d9_states.hpp"
#include <cstdlib>

namespace bsp {
void set_renderer_synchronization_00b33aa0(RendererSynchronization& state, bool enabled) {
    state.enabled = enabled;
    state.observed_enabled = enabled;
}

bool renderer_device_lifecycle_busy_00b20220(const TrackedCriticalSection& lock) {
    return lock.depth > 0;
}

bool D3D9StateCache::enter_00b33ad0() {
    ++synchronization_.nesting;
    if (!synchronization_.enabled || !lock_) return false;
    EnterCriticalSection(&lock_->native);
    ++lock_->depth;
    return true;
}

void D3D9StateCache::leave_00b33b00(bool /*entered*/) {
    // Native RET 4h consumes but ignores the saved enter result.
    --synchronization_.nesting;
    if (synchronization_.enabled && lock_) {
        --lock_->depth;
        LeaveCriticalSection(&lock_->native);
    }
    if (synchronization_.nesting == 0
        && synchronization_.observed_enabled != synchronization_.enabled)
        synchronization_.observed_enabled = synchronization_.enabled;
}

struct D3D9StateCache::Guard {
    D3D9StateCache& owner;
    bool entered{};
    explicit Guard(D3D9StateCache& cache) : owner(cache) {
        if (owner.synchronization_.enabled) entered = owner.enter_00b33ad0();
    }
    ~Guard() {
        if (owner.synchronization_.enabled) owner.leave_00b33b00(entered);
    }
};

void D3D9StateCache::set_render_state_00b24460(D3DRENDERSTATETYPE state, DWORD value) {
    const auto index = static_cast<UINT>(state);
    if (index >= render_.size()) std::abort(); // New interface bounds precondition.
    Guard guard(*this);
    auto& entry = render_[index];
    if (!entry.valid || entry.value != value) {
        entry = {true, value}; // Native caches even failed SetRenderState requests.
        device_.SetRenderState(state, value);
        ++render_calls_;
    }
}

void D3D9StateCache::set_sampler_state_00b24610(UINT sampler, D3DSAMPLERSTATETYPE state, DWORD value) {
    const auto index = static_cast<UINT>(state);
    if (sampler >= samplers_.size() || index >= samplers_[0].size()) std::abort();
    Guard guard(*this);
    auto& entry = samplers_[sampler][index];
    if (!entry.valid || entry.value != value) {
        entry = {true, value};
        device_.SetSamplerState(sampler < 16 ? sampler : sampler + 0xf1, state, value);
        ++sampler_calls_;
    }
}

void D3D9StateCache::bind_render_state_block_00b27a80(std::shared_ptr<RenderStateBlock> value) {
    if (render_block_ == value) return; // 00b27a88..8b precedes all work.
    // Retain the new block before releasing the previous block. Keep the input
    // alive while iterating, like native ESI; do not snapshot or deduplicate it.
    render_block_ = value;
    if (value) {
        for (const auto& item : value->states)
            set_render_state_00b24460(item.state, item.value);
    }
    ++render_block_calls_; // Includes null/empty replacements, modulo 2^32.
}

void D3D9StateCache::bind_sampler_state_block_00b27b90(std::shared_ptr<SamplerStateBlock> value) {
    if (sampler_block_ == value) return; // No outer optional guard in native.
    sampler_block_ = value;
    if (value) {
        for (const auto& item : value->states)
            set_sampler_state_00b24610(item.sampler, item.state, item.value);
    }
    ++sampler_block_calls_;
}

HRESULT D3D9StateCache::set_vertex_shader_constants_f_00b21820(UINT start_register,
    const float* data, UINT vector_count) {
    if (vector_count == 0) return S_FALSE; // TEST/JZ at 00b2183e precedes guard.
    Guard guard(*this);
    const HRESULT result = device_.SetVertexShaderConstantF(start_register, data, vector_count);
    ++vertex_constant_calls_;
    vertex_constant_bytes_ += static_cast<std::uint32_t>(vector_count) << 4;
    return result;
}

HRESULT D3D9StateCache::set_pixel_shader_constants_f_00b218c0(UINT start_register,
    const float* data, UINT vector_count) {
    if (vector_count == 0) return S_FALSE; // TEST/JZ at 00b218de precedes guard.
    Guard guard(*this);
    const HRESULT result = device_.SetPixelShaderConstantF(start_register, data, vector_count);
    ++pixel_constant_calls_;
    pixel_constant_bytes_ += static_cast<std::uint32_t>(vector_count) << 4;
    return result;
}

HRESULT D3D9StateCache::bind_vertex_shader_00b21d10(const LogicalVertexShader* value) {
    Guard guard(*this);
    // 00b21d64..6a compares through the logical objects, not a saved COM value.
    // Two non-null logical objects with null COM pointers also compare equal.
    if (vertex_shader_ && value && vertex_shader_->shader == value->shader) return S_FALSE;
    const auto* previous = vertex_shader_;
    vertex_shader_ = value;
    if (!previous && !value) return S_FALSE;
    const HRESULT result = device_.SetVertexShader(value ? value->shader : nullptr);
    ++vertex_shader_calls_;
    return result;
}

HRESULT D3D9StateCache::bind_pixel_shader_00b21c20(const LogicalPixelShader* value) {
    Guard guard(*this);
    // 00b21c74..7a skips without adopting an equivalent new logical object.
    if (pixel_shader_ && value && pixel_shader_->shader == value->shader) return S_FALSE;
    const auto* previous = pixel_shader_;
    pixel_shader_ = value;
    if (!previous && !value) return S_FALSE;
    const HRESULT result = device_.SetPixelShader(value ? value->shader : nullptr);
    ++pixel_shader_calls_;
    return result;
}

HRESULT D3D9StateCache::bind_texture_00b24710(UINT sampler,
    std::shared_ptr<LogicalTexture> value) {
    if (sampler >= textures_.size()) std::abort(); // New interface precondition.
    Guard guard(*this);
    auto& cached = textures_[sampler];
    if (cached == value) return S_FALSE;
    cached = std::move(value); // Retain new logical identity, release old.
    const HRESULT result = device_.SetTexture(sampler < 16 ? sampler : sampler + 0xf1,
        cached ? cached->texture : nullptr);
    ++texture_binding_calls_;
    return result;
}

void D3D9StateCache::invalidate() {
    Guard guard(*this);
    render_ = {};
    samplers_ = {};
    render_block_.reset();
    sampler_block_.reset();
    textures_ = {};
    stream_frequencies_ = {};
    streams_ = {};
    indices_.reset();
    base_vertex_ = 0;
    // New-interface reset discards borrowed projections; it does not issue
    // device unbinds. Caller coordinates reset/device state as for streams.
    vertex_shader_ = nullptr;
    pixel_shader_ = nullptr;
}

void D3D9StateCache::bind_vertex_stream_00b24840(UINT stream, std::shared_ptr<LogicalVertexStream> value) {
    if (stream >= streams_.size() || (value && (!value->physical || !value->declaration))) std::abort();
    Guard guard(*this);
    auto& cached = streams_[stream];
    if (cached.object == value) return;
    if (cached.object && value
        && cached.object->physical->buffer == value->physical->buffer
        && cached.stride == value->declaration->stride && cached.offset == value->offset) {
        cached.object = std::move(value); // Transfer logical ownership without API call.
        return;
    }
    cached.object = std::move(value);
    IDirect3DVertexBuffer9* buffer = nullptr;
    cached.stride = cached.offset = 0;
    if (cached.object) {
        buffer = cached.object->physical->buffer;
        cached.stride = cached.object->declaration->stride;
        cached.offset = cached.object->offset;
    }
    device_.SetStreamSource(stream, buffer, cached.offset, cached.stride);
    ++vertex_binding_calls_;
}

void D3D9StateCache::bind_index_stream_00b24b00(std::shared_ptr<LogicalIndexStream> value, INT base_vertex) {
    if (value && !value->physical) std::abort();
    Guard guard(*this);
    base_vertex_ = base_vertex; // Updated even for the same logical object.
    if (indices_ != value) {
        indices_ = std::move(value);
        device_.SetIndices(indices_ ? indices_->physical->buffer : nullptr);
        ++index_binding_calls_;
    }
}

HRESULT D3D9StateCache::draw_indexed_00b24010(const D3D9DrawState& state,
    D3DPRIMITIVETYPE type, UINT minimum_vertex, UINT vertex_count,
    UINT start_index, UINT primitive_count) {
    if (state.inhibit || state.device_lost) return S_FALSE;
    Guard guard(*this); // Native guard precedes the zero-count checks here.
    if (vertex_count == 0 || primitive_count == 0) return S_FALSE;
    const auto& stream = streams_[0].object;
    if (stream && vertex_count > stream->vertex_count && stream->tag == 0x40000001) return S_FALSE;
    return device_.DrawIndexedPrimitive(type, base_vertex_, minimum_vertex,
        vertex_count, start_index, primitive_count);
}

HRESULT D3D9StateCache::lock_vertex_stream_00b49980(LogicalVertexStream& stream,
    UINT count, UINT offset, bool read_only, void*& data) {
    Guard guard(*this);
    data = stream.mapped;
    if (!stream.physical) return S_FALSE;
    if (!stream.declaration) std::abort();
    const UINT stride = stream.declaration->stride;
    const bool dynamic = (stream.flags & 0xf000) == 0x1000;
    UINT bytes = stride * count;
    UINT extra;
    if (dynamic) {
        stream.vertex_count = count;
        extra = stride * offset;
        read_only = false;
    } else {
        if (bytes == 0) bytes = stream.vertex_count * stride;
        extra = (stream.base_vertex + offset) * stride;
    }
    BufferLockResult output{};
    const HRESULT result = vertex_buffer_lock_00b4ba00(*stream.physical, bytes, extra, read_only, output);
    if (dynamic) stream.offset = output.base_offset;
    stream.mapped = data = output.data;
    return result;
}
void D3D9StateCache::unlock_vertex_stream_00b49a80(LogicalVertexStream& stream) {
    Guard guard(*this);
    if (stream.physical) vertex_buffer_unlock_00b4b9d0(*stream.physical);
    stream.mapped = nullptr;
}
HRESULT D3D9StateCache::lock_index_stream_00b49b60(LogicalIndexStream& stream,
    UINT count, UINT offset, bool read_only, void*& data) {
    Guard guard(*this);
    data = nullptr;
    if (!stream.physical) return S_FALSE;
    const UINT size = stream.format == D3DFMT_INDEX16 ? 2u
        : stream.format == D3DFMT_INDEX32 ? 4u : 0u;
    UINT bytes = size * count;
    if (bytes == 0) bytes = size * stream.index_count;
    BufferLockResult output{};
    const HRESULT result = index_buffer_lock_00b4b850(*stream.physical, bytes,
        (stream.base_index + offset) * size, read_only, output);
    stream.offset = output.base_offset;
    data = output.data;
    return result;
}
void D3D9StateCache::unlock_index_stream_00b49c70(LogicalIndexStream& stream) {
    Guard guard(*this);
    if (stream.physical) index_buffer_unlock_00b4b820(*stream.physical);
}

namespace {
template<class Stream> void register_unique(std::vector<Stream*>& registry, Stream& stream) {
    for (const auto* existing : registry) if (existing == &stream) return;
    if (registry.size() == registry.capacity())
        registry.reserve(registry.capacity() ? registry.capacity() * 2 : 1);
    registry.push_back(&stream);
}
template<class Stream> void remove_swap_last_00b4b2e0(std::vector<Stream*>& registry, Stream& stream) {
    for (auto& existing : registry) {
        if (existing == &stream) {
            existing = registry.back();
            registry.pop_back();
            return;
        }
    }
}
}
void D3D9StateCache::register_logical_stream_00b4b1e0(VertexBufferBinding& buffer, LogicalVertexStream& stream) {
    Guard guard(*this);
    register_unique(buffer.logical_streams, stream);
}
void D3D9StateCache::register_logical_stream_00b4b1e0(IndexBufferBinding& buffer, LogicalIndexStream& stream) {
    Guard guard(*this);
    register_unique(buffer.logical_streams, stream);
}
void D3D9StateCache::unregister_vertex_stream_00b4b3f0(VertexBufferBinding& buffer, LogicalVertexStream& stream) {
    Guard guard(*this);
    remove_swap_last_00b4b2e0(buffer.logical_streams, stream);
}
void D3D9StateCache::unregister_index_stream_00b4b390(IndexBufferBinding& buffer, LogicalIndexStream& stream) {
    Guard guard(*this);
    remove_swap_last_00b4b2e0(buffer.logical_streams, stream);
}
void D3D9StateCache::rewind_vertex_buffer_00b232b0(VertexBufferBinding& buffer) {
    Guard guard(*this);
    // Native virtual +8h resolves to 00b48d40 for these logical streams.
    for (auto* stream : buffer.logical_streams) stream->offset = 0xffffffff;
    buffer.cursor = 0;
    buffer.dynamic_locks = 0;
}
void D3D9StateCache::rewind_index_buffer_00b231c0(IndexBufferBinding& buffer) {
    Guard guard(*this);
    // Native virtual +8h resolves to 00b48dd0. Lock depth is not reset.
    for (auto* stream : buffer.logical_streams) stream->offset = 0xffffffff;
    buffer.cursor = 0;
    buffer.dynamic_locks = 0;
}

void D3D9StateCache::set_stream_frequency_00b24a40(UINT stream, UINT frequency) {
    if (stream >= stream_frequencies_.size()) std::abort();
    Guard guard(*this);
    if (stream_frequencies_[stream] != frequency) {
        stream_frequencies_[stream] = frequency;
        device_.SetStreamSourceFreq(stream, frequency);
    }
}

HRESULT D3D9StateCache::draw_primitive_00b21b40(const D3D9DrawState& state,
    D3DPRIMITIVETYPE type, UINT start_vertex, UINT primitive_count) {
    if (state.inhibit || state.device_lost) return S_FALSE;
    // Native calls 00b1f740 with object at +1904h: LEA EAX,[ECX+10h]; RET.
    // Its unused return and absence of memory accesses produce no observable work.
    if (primitive_count == 0) return S_FALSE;
    Guard guard(*this);
    return device_.DrawPrimitive(type, start_vertex, primitive_count);
}

void D3D9StateCache::initialize_defaults_00b26170() {
    // Preserve the assembly's order, including all four color-write masks.
    const struct { D3DRENDERSTATETYPE state; DWORD value; } states[] = {
        {D3DRS_DITHERENABLE, 1}, {D3DRS_FOGENABLE, 0}, {D3DRS_FOGTABLEMODE, 0},
        {D3DRS_ZENABLE, 1}, {D3DRS_ZWRITEENABLE, 1}, {D3DRS_ZFUNC, 2},
        {D3DRS_STENCILENABLE, 0}, {D3DRS_TWOSIDEDSTENCILMODE, 0},
        {D3DRS_SCISSORTESTENABLE, 0}, {D3DRS_ALPHATESTENABLE, 0},
        {D3DRS_ALPHABLENDENABLE, 0}, {D3DRS_CULLMODE, 3}, {D3DRS_SPECULARENABLE, 0},
        {D3DRS_FILLMODE, 3}, {D3DRS_COLORWRITEENABLE, 15}, {D3DRS_COLORWRITEENABLE1, 15},
        {D3DRS_COLORWRITEENABLE2, 15}, {D3DRS_COLORWRITEENABLE3, 15}, {D3DRS_SRGBWRITEENABLE, 0}
    };
    for (const auto& state : states) set_render_state_00b24460(state.state, state.value);
    for (UINT sampler = 0; sampler < 20; ++sampler) {
        set_sampler_state_00b24610(sampler, D3DSAMP_MINFILTER, 2);
        set_sampler_state_00b24610(sampler, D3DSAMP_MAGFILTER, 2);
        set_sampler_state_00b24610(sampler, D3DSAMP_MIPFILTER, 1);
        set_sampler_state_00b24610(sampler, D3DSAMP_ADDRESSU, 1);
        set_sampler_state_00b24610(sampler, D3DSAMP_ADDRESSV, 1);
        set_sampler_state_00b24610(sampler, D3DSAMP_ADDRESSW, 1);
        set_sampler_state_00b24610(sampler, D3DSAMP_BORDERCOLOR, 0xffffffff);
    }
}
}
