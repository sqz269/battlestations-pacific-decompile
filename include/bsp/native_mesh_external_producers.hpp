#pragma once
#include "bsp/native_mesh_canonical.hpp"
#include <optional>
namespace bsp {
// Concrete nonmoving retained caller storage, not a permit for an incoming raw
// address. Prepare at a stable owning-record address BEFORE parent Native
// effects. The actual owner keeps this object/Domain/contexts through failure,
// zero, callbacks and all later C++ fences; destruction is metadata only.
class NativeMeshExternalProducer final {
public:
    NativeMeshExternalProducer(NativeMeshCanonicalDomain&,
        const void* actual_stable_owning_record) noexcept;
    ~NativeMeshExternalProducer() noexcept = default;
    NativeMeshExternalProducer(const NativeMeshExternalProducer&) = delete;
    NativeMeshExternalProducer& operator=(const NativeMeshExternalProducer&) = delete;
    NativeMeshExternalProducer(NativeMeshExternalProducer&&) = delete;
    NativeMeshExternalProducer& operator=(NativeMeshExternalProducer&&) = delete;
    void prepare();
    void check_prepared_metadata() const;
    NativeMeshStorage* create_explicit(NativeMeshEnvironment&);
    NativeMeshStorage* create_explicit(NativeMeshEnvironment&, NativeMeshConstants);
    NativeMeshStorage* create_static(NativeMeshEnvironment&);
    void close_at_quiescence();
    bool requires_process_retention() const noexcept;
    bool source_retired() const noexcept; // diagnostic, never close/action authority
    void retain_source_failure() noexcept;
private:
    NativeMeshCanonicalDomain& domain_;
    const void* const owner_;
    std::optional<NativeMeshCanonicalProducerFrame> frame_;
    bool prepare_entered_{}, prepared_{}, entered_{}, birth_normal_{};
};

// AF74A0 has TWO independent creator sites, at +1B4 and +1C8. No global next
// index or count cap maps them. A failed second preparation leaves the first
// in this ALREADY-constructed caller-owned object for explicit cold disposal.
class NativeParticleMeshProducerFrames final {
public:
    NativeParticleMeshProducerFrames(NativeMeshCanonicalDomain& d,
        const void* actual_stable_particle_owning_record) noexcept
        : first_(d, actual_stable_particle_owning_record),
          second_(d, actual_stable_particle_owning_record) {}
    NativeParticleMeshProducerFrames(const NativeParticleMeshProducerFrames&) = delete;
    NativeParticleMeshProducerFrames& operator=(const NativeParticleMeshProducerFrames&) = delete;
    void prepare() { first_.prepare(); second_.prepare(); }
    void check_prepared_metadata() const { first_.check_prepared_metadata(); second_.check_prepared_metadata(); }
    NativeMeshStorage* create_first(NativeMeshEnvironment& e, NativeMeshConstants c) { return first_.create_explicit(e, c); }
    NativeMeshStorage* create_second(NativeMeshEnvironment& e, NativeMeshConstants c) { return second_.create_explicit(e, c); }
    void close_at_quiescence() { first_.close_at_quiescence(); second_.close_at_quiescence(); }
    void retain_source_failure() noexcept { first_.retain_source_failure(); second_.retain_source_failure(); }
    bool requires_process_retention() const noexcept {
        return first_.requires_process_retention() || second_.requires_process_retention();
    }
private:
    NativeMeshExternalProducer first_, second_;
};
} // namespace bsp
