#include "bsp/native_game_current_virtual_calls.hpp"

#include <cstddef>
#include <cstring>
#include <type_traits>

namespace bsp {
namespace {

using TerminalMethod = void (__thiscall*)(void*);
using PeerRecordMethod = void (__thiscall*)(void*, void*);

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);
static_assert(sizeof(TerminalMethod) == 4 && sizeof(PeerRecordMethod) == 4);
static_assert(!std::is_abstract_v<NativeGameCurrentVirtualCalls>);

template<class Method>
Method current_method(const void* receiver, std::size_t byte_slot) {
    // Copy actual pointer representations through bytes; do not manufacture
    // an object reference, translate a profile ID or retain a cached table.
    void* table;
    std::memcpy(&table, receiver, sizeof(table));
    Method method;
    std::memcpy(&method, static_cast<const unsigned char*>(table) + byte_slot,
                sizeof(method));
    return method;
}

} // namespace

void NativeGameCurrentVirtualCalls::virtual_scalar(
    void* captured, std::uint32_t byte_slot, std::uint32_t flags) {
    NativeGamePhysicsLifetimeCalls::physics_virtual_scalar(
        captured, byte_slot, flags);
}

void NativeGameCurrentVirtualCalls::virtual_terminal(void* captured) {
    const auto method = current_method<TerminalMethod>(captured, 0);
    method(captured);
    // The target may retire captured. No receiver access follows the call.
}

void NativeGameCurrentVirtualCalls::virtual_04(void* receiver, void* record) {
    const auto method = current_method<PeerRecordMethod>(receiver, 4);
    method(receiver, record);
    // The target controls receiver/record lifetimes; neither is read afterward.
}

} // namespace bsp
