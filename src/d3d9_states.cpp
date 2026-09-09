#include "bsp/d3d9_states.hpp"
#include <cstdlib>

namespace bsp {
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

void D3D9StateCache::invalidate() {
    Guard guard(*this);
    render_ = {};
    samplers_ = {};
    stream_frequencies_ = {};
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
