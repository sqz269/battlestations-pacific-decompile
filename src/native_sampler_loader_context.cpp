#include "bsp/native_sampler_loader_context.hpp"
#include "bsp/native_particle_clock_singleton.hpp"
#include "bsp/native_singleton_publication.hpp"

#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Op = NativeSamplerLoaderOperation;
static_assert(std::is_same_v<NativeSamplerLoaderSingletonStorage,
    NativeParticleClockStorage>);

void begin(Op& operation, std::uint32_t function,
    void* volatile& manager, void* volatile& owner) {
    if (operation.phase != Op::Phase::fresh)
        throw std::logic_error("sampler loader operation is one-shot");
    operation.phase = Op::Phase::running;
    operation.function = function;
    operation.native_site = function;
    operation.manager_publication = &manager;
    operation.owner_publication = &owner;
}
} // namespace

NativeSamplerLoaderOperation::~NativeSamplerLoaderOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeSamplerLoaderOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) phase = Phase::diagnostic_retired;
}
NativeSamplerLoaderSingletonStorage* get_native_sampler_loader_singleton_004de4b0(
    void* volatile& manager, void* volatile& publication, Op& operation) {
    begin(operation, 0x004de4b0, manager, publication);
    try {
        auto* const result = get_native_particle_clock_singleton_004de4b0(
            manager, publication);
        operation.result = result;
        operation.phase = Op::Phase::complete;
        return result;
    } catch (...) {
        operation.phase = Op::Phase::failed;
        throw;
    }
}
void* create_native_sampler_factory_resource_00b1b810(const void* name, const void*,
    void* volatile& manager, void* volatile& registry,
    const NativeResourceRegistryLookupContext& context, Op& operation) {
    begin(operation, 0x00b1b810, manager, registry);
    operation.source_name = name;
    operation.lookup_context = &context;
    try {
        operation.native_site = 0x00b1b810;
        operation.registry = get_native_resource_registry_00b1b730(manager, registry);
        operation.native_site = 0x00b1b81c;
        operation.result = create_native_registered_resource_00b19e90(
            operation.registry, name, context);
        operation.phase = Op::Phase::complete;
        return operation.result;
    } catch (...) {
        operation.phase = Op::Phase::failed;
        throw;
    }
}
} // namespace bsp
