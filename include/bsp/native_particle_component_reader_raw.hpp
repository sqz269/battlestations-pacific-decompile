#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeLuaObjectStorage;
struct NativeParticleResourceCacheContext;
struct NativeParticleResourceAcquisitionRawContext;
class NativeParticleResourceAcquisitionRawAcquired;

struct NativeParticleComponentReaderRawContext {
    NativeParticleResourceCacheContext& cache;
    NativeParticleResourceAcquisitionRawContext& acquisition;
    const bool& actual_crt_sse2_conversion;
};

enum class NativeParticleComponentReaderRawPhase { fresh, running, complete, failed };
// One immovable invocation. The actual8h name and three actual14h Lua objects
// precede retained acquisition children. Destruction never replays native
// cleanup, releases appended resources, or rolls back the component. Retain a
// failed frame and its borrowed domains through every child's obligations.
class NativeParticleComponentReaderRawAcquired final {
public:
    explicit NativeParticleComponentReaderRawAcquired(std::int32_t incoming_parser_builder_kind);
    ~NativeParticleComponentReaderRawAcquired();
    NativeParticleComponentReaderRawAcquired(const NativeParticleComponentReaderRawAcquired&) = delete;
    NativeParticleComponentReaderRawAcquired& operator=(const NativeParticleComponentReaderRawAcquired&) = delete;
    NativeParticleComponentReaderRawPhase phase() const noexcept;
    std::uint32_t failure_site() const noexcept;
    std::int32_t native_state_at_failure() const noexcept;
    std::size_t acquisition_count() const noexcept;
    const NativeParticleResourceAcquisitionRawAcquired* acquisition_invocation(std::size_t index) const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void read_native_particle_component_00871d00(void*, NativeLuaObjectStorage&,
        NativeParticleComponentReaderRawContext&, NativeParticleComponentReaderRawAcquired&);
};

// Complete657B 00871D00. ECX actual34h component; stack Lua object, RET4.
// Base reader first. UnderWater is the BOOLEAN TYPE predicate: false and true
// both produce1. A Particle table contributes VALUES in native Lua iteration
// order; otherwise its string is used once. Acquire(cache+4,name,0,0,1), return
// temporary name, store resource+70, append at component+28. Never clear prior
// entries or release an acquired resource on subsequent failure. Current header
// rereads, signed capacity growth and DWORD wrap match the original body.
// New source ABI only; original FH3/SEH/CRT faults and gameplay are unproved.
void read_native_particle_component_00871d00(void* actual_component,
    NativeLuaObjectStorage& definition, NativeParticleComponentReaderRawContext&,
    NativeParticleComponentReaderRawAcquired&);
} // namespace bsp
