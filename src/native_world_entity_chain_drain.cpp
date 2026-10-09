#include "bsp/native_world_entity_chain_drain.hpp"
#include "bsp/native_game_physics_lifetime.hpp"

#include <cstring>
#include <type_traits>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);
static_assert(std::is_standard_layout_v<NativeWorldChainHeader>);
static_assert(sizeof(NativeWorldChainHeader) == 0x0c);
static_assert(offsetof(NativeWorldChainHeader, first) == 0);
static_assert(offsetof(NativeWorldChainHeader, last) == 4);
static_assert(offsetof(NativeWorldChainHeader, count) == 8);

// Access the actual entity's pointer representations through bytes, without
// inventing an entity type or typed reference into its opaque storage.
void* read_link(const void* entity, std::size_t offset) noexcept {
    void* value;
    std::memcpy(&value, static_cast<const unsigned char*>(entity) + offset,
                sizeof(value));
    return value;
}

void write_link(void* entity, std::size_t offset, void* value) noexcept {
    std::memcpy(static_cast<unsigned char*>(entity) + offset, &value,
                sizeof(value));
}

} // namespace

void drain_native_world_entity_chain_009041a0(
    NativeWorldChainHeader* actual_header,
    NativeGamePhysicsLifetimeCalls& lifetime_calls) {
    // Volatile qualifies the existing header object, preserving these explicit
    // field accesses. It supplies neither synchronization nor a promise of
    // native machine-instruction ordering; concurrent mutation is unsupported.
    volatile NativeWorldChainHeader* const header = actual_header;
    if (header->count == 0) return;

    do {
        void* const entity = header->first;
        void* const previous = read_link(entity, 0x34);
        // 009041B5..C2 short-circuits in this order. Win32's signed DWORD
        // conversion retains JG's treatment of high-bit counts as negative.
        if (previous != nullptr || read_link(entity, 0x38) != nullptr ||
            static_cast<std::int32_t>(header->count) <= 1) {
            if (previous != nullptr) {
                write_link(previous, 0x38, read_link(entity, 0x38));
            } else {
                header->first = read_link(entity, 0x38);
            }

            // Reload next after the previous-link/head store, then reload
            // previous for the reciprocal-link/tail store (009041D5..E7).
            void* const next = read_link(entity, 0x38);
            if (next != nullptr) {
                write_link(next, 0x34, read_link(entity, 0x34));
            } else {
                header->last = read_link(entity, 0x34);
            }
            write_link(entity, 0x38, nullptr);
            write_link(entity, 0x34, nullptr);
            header->count = header->count - 1u;
        }

        // 009041F4..FA uses the actual current entity table's slot0, flags1.
        // Qualify the existing concrete dispatcher; do not substitute an
        // unqualified service override. The entity may cease to exist here.
        lifetime_calls.NativeGamePhysicsLifetimeCalls::physics_virtual_scalar(
            entity, 0, 1);
        // Never dereference entity after the call. Reload the retained header's
        // count, then its current head on the next iteration (009041FC..FF).
    } while (header->count != 0);
}

} // namespace bsp
