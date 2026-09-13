#include "bsp/dyn_world_factory.hpp"

#include <cstdint>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(DynEngineStorage) == 0x14);
static_assert(sizeof(DynWorldStorage) == 0x48c);
namespace {
template<class T> T& at(void* storage, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(storage) + offset);
}
} // namespace

DynWorldStorage* dyn_engine_create_world_00c420e0(DynEngineStorage& engine,
    const DynWorldDescriptor& descriptor, const DynWorldRuntimeContext& context) {
    const auto& memory = context.scene->memory;
    auto* world = static_cast<DynWorldStorage*>(memory.allocate(memory.context, 0x48c));
    if (world) world = dyn_world_storage_construct_00c41ad0(*world, descriptor, context);

    auto& data = at<DynWorldStorage**>(&engine, 0);
    auto& count = at<std::uint32_t>(&engine, 4);
    auto& capacity = at<std::uint32_t>(&engine, 8);
    if (count == capacity) {
        const std::uint32_t requested = capacity * 2u + 2u;
        const std::uint32_t bytes = requested * 4u;
        capacity = requested; //00C4213F, before the allocation may fail.
        auto* replacement = static_cast<DynWorldStorage**>(memory.allocate(memory.context, bytes));
        std::uint32_t destination = reinterpret_cast<std::uint32_t>(replacement);
        for (std::uint32_t i = 0; i < count; ++i, destination += 4u) {
            if (destination) *reinterpret_cast<DynWorldStorage**>(destination) = data[i];
        }
        if (data) memory.release(memory.context, data);
        data = replacement; //00C4217A, after _free's returning continuation.
    }
    const std::uint32_t destination = reinterpret_cast<std::uint32_t>(data) + count * 4u;
    if (destination) *reinterpret_cast<DynWorldStorage**>(destination) = world;
    ++count;
    return world;
}
} // namespace bsp
