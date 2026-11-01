#include "bsp/native_mesh_external_producers.hpp"
#include <stdexcept>
namespace bsp {
NativeMeshExternalProducer::NativeMeshExternalProducer(NativeMeshCanonicalDomain& d,
    const void* owner) noexcept : domain_(d), owner_(owner) {}
void NativeMeshExternalProducer::prepare() {
    if (prepare_entered_ || prepared_ || frame_ || entered_)
        throw std::logic_error("External Mesh preparation is once-only before parent effects");
    prepare_entered_ = true; // one Source preparation attempt; no cleanup retry
    frame_.emplace(domain_, owner_);
    prepared_ = true;
}
void NativeMeshExternalProducer::check_prepared_metadata() const {
    if (!prepared_ || !frame_ || entered_)
        throw std::logic_error("Ordinary external Mesh binding must be cold-prepared before parent effects");
}
NativeMeshStorage* NativeMeshExternalProducer::create_explicit(NativeMeshEnvironment& environment) {
    if (!prepared_ || !frame_ || entered_)
        throw std::logic_error("External Mesh source requires its actual cold retained preparation");
    entered_ = true; // fail closed before any fallible source/provider operation
    try {
        NativeMeshCanonicalAccess::require_domain_identity(domain_, environment,
            environment.retained_owners);
        auto* const raw = NativeMeshCanonicalAccess::allocate_explicit(*frame_);
        if (!raw) { NativeMeshCanonicalAccess::cancel_without_native_payload(*frame_); return nullptr; }
        auto* const result = NativeMeshCanonicalAccess::construct(*frame_);
        birth_normal_ = true;
        (void)NativeMeshCanonicalAccess::publish(*frame_);
        NativeMeshCanonicalAccess::transfer_creator_to_caller(*frame_);
        return result;
    } catch (...) { retain_source_failure(); throw; }
}
NativeMeshStorage* NativeMeshExternalProducer::create_explicit(NativeMeshEnvironment& environment,
    NativeMeshConstants constants) {
    NativeMeshCanonicalAccess::require_domain_identity(domain_, environment,
        environment.retained_owners, constants); // address-role check, not value reads
    return create_explicit(environment);
}
NativeMeshStorage* NativeMeshExternalProducer::create_static(NativeMeshEnvironment& environment) {
    if (!prepared_ || !frame_ || entered_)
        throw std::logic_error("External Mesh source requires its actual cold retained preparation");
    entered_ = true;
    try {
        NativeMeshCanonicalAccess::require_domain_identity(domain_, environment,
            environment.retained_owners);
        auto* const raw = NativeMeshCanonicalAccess::allocate_static(*frame_);
        if (!raw) { NativeMeshCanonicalAccess::cancel_without_native_payload(*frame_); return nullptr; }
        auto* const result = NativeMeshCanonicalAccess::construct(*frame_);
        birth_normal_ = true;
        (void)NativeMeshCanonicalAccess::publish(*frame_);
        NativeMeshCanonicalAccess::transfer_creator_to_caller(*frame_);
        return result;
    } catch (...) { retain_source_failure(); throw; }
}
void NativeMeshExternalProducer::close_at_quiescence() {
    if (!frame_) return;
    if (birth_normal_) NativeMeshCanonicalAccess::close_producer_link_at_quiescence(*frame_);
    else if (frame_->record_) NativeMeshCanonicalAccess::cancel_without_native_payload(*frame_);
    frame_.reset(); // genuine exact link is already closed; never disposes Native
}
bool NativeMeshExternalProducer::requires_process_retention() const noexcept {
    return frame_ && frame_->requires_process_retention();
}
bool NativeMeshExternalProducer::source_retired() const noexcept {
    return frame_ && NativeMeshCanonicalAccess::source_retired(*frame_);
}
void NativeMeshExternalProducer::retain_source_failure() noexcept {
    if (frame_) NativeMeshCanonicalAccess::record_source_failure(*frame_);
    NativeMeshCanonicalAccess::retain_source_graph(domain_);
}
} // namespace bsp
