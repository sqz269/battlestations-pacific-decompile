#include "bsp/native_vfs_fileblock_scope.hpp"

#include "bsp/native_fileblock_gate_list.hpp"
#include "bsp/native_pak_registry_block_callbacks.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS FileBlock scope requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
void* at(const void* value, U offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(value) + offset);
}
void* volatile& pointer(const void* value, U offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(value, offset));
}
volatile U& word(const void* value, U offset = 0) noexcept {
    return *static_cast<volatile U*>(at(value, offset));
}
volatile std::uint8_t& octet(const void* value, U offset = 0) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(value, offset));
}
void diagnostic_004254b0(const char*, const void*) noexcept {} // Verified one-byte RET.
} // namespace

void NativePakRegistryFileBlockObserverDispatch::invoke_enter(std::uintptr_t entry,
    void* observer, const void* name, NativePakRegistryBlockInvocation& invocation) {
    if (entry != 0x00bb5770)
        throw std::invalid_argument("Unimplemented current VFS FileBlock entry observer");
    enter_native_pak_registry_block_00bb5770(observer, name, context_, invocation);
}
void NativePakRegistryFileBlockObserverDispatch::invoke_leave(std::uintptr_t entry,
    void* observer, const void* name, NativePakRegistryBlockInvocation& invocation) {
    if (entry != 0x00bb5910)
        throw std::invalid_argument("Unimplemented current VFS FileBlock exit observer");
    leave_native_pak_registry_block_00bb5910(observer, name, context_, invocation);
}

struct NativeVfsFileBlockScopeAcquired::Impl {
    NativeVfsFileBlockScopePhase phase = NativeVfsFileBlockScopePhase::fresh;
    U active = 0, failure = 0;
    void* manager = nullptr;
    const void* input = nullptr;
    void* list = nullptr;
    void* head = nullptr;
    void* previous = nullptr;
    void* allocated = nullptr;
    void* restored_node = nullptr;
    void* erased_node = nullptr;
    void* observer = nullptr;
    void* observer_table = nullptr;
    U observer_entry = 0;
    const void* observer_name = nullptr;
    U erase_output[2]{};
    const void* diagnostic_data = nullptr;
    NativePakRegistryBlockInvocation nested;

    void begin(void* owner, const void* name) {
        if (phase != NativeVfsFileBlockScopePhase::fresh)
            throw std::logic_error("Native VFS FileBlock scope invocation cannot replay");
        manager = owner; input = name; list = at(owner, 0x7c);
    }
    void diagnose(const char* format, const char* empty) {
        diagnostic_data = pointer(input, 4);
        if (!diagnostic_data) diagnostic_data = empty;
        diagnostic_004254b0(format, diagnostic_data);
    }
    void finish() noexcept { active = 0; phase = NativeVfsFileBlockScopePhase::complete; }
    void failed() noexcept { failure = active; phase = NativeVfsFileBlockScopePhase::failed; }
};

NativeVfsFileBlockScopeAcquired::NativeVfsFileBlockScopeAcquired() : impl_(std::make_unique<Impl>()) {}
NativeVfsFileBlockScopeAcquired::~NativeVfsFileBlockScopeAcquired() {
    if (impl_->phase != NativeVfsFileBlockScopePhase::fresh &&
        impl_->phase != NativeVfsFileBlockScopePhase::complete) std::terminate();
}
NativeVfsFileBlockScopePhase NativeVfsFileBlockScopeAcquired::phase() const noexcept { return impl_->phase; }
U NativeVfsFileBlockScopeAcquired::active_call_site() const noexcept { return impl_->active; }
U NativeVfsFileBlockScopeAcquired::failure_site() const noexcept { return impl_->failure; }
const void* NativeVfsFileBlockScopeAcquired::allocated_gate_node() const noexcept { return impl_->allocated; }
const void* NativeVfsFileBlockScopeAcquired::actual_erase_output() const noexcept { return impl_->erase_output; }
const NativePakRegistryBlockInvocation& NativeVfsFileBlockScopeAcquired::observer_invocation() const noexcept { return impl_->nested; }

void enter_native_vfs_fileblock_00be0980(void* manager, const void* name, std::uint8_t gate,
    NativeVfsFileBlockScopeContext& context, NativeVfsFileBlockScopeAcquired& acquired) {
    auto& f = *acquired.impl_;
    f.begin(manager, name);
    try {
        f.phase = NativeVfsFileBlockScopePhase::allocating_gate;
        f.head = pointer(manager, 0x80);
        f.previous = pointer(f.head, 4);
        f.active = 0x00be099a;
        f.allocated = allocate_native_fileblock_gate_node_007f8390(f.head, f.previous,
            static_cast<const std::uint8_t*>(at(manager, 0x79)));
        f.phase = NativeVfsFileBlockScopePhase::growing_gate_count;
        f.active = 0x00be09a5;
        grow_native_fileblock_gate_count_007fa3a0(f.list, 1);
        f.phase = NativeVfsFileBlockScopePhase::linking_gate;
        pointer(f.head, 4) = f.allocated;
        auto* const previous = pointer(f.allocated, 4);
        pointer(previous) = f.allocated;
        octet(manager, 0x79) = static_cast<std::uint8_t>(octet(manager, 0x79) & gate);
        if (octet(manager, 0x78) != 0) {
            f.phase = NativeVfsFileBlockScopePhase::notifying;
            f.observer = pointer(manager, 0x88);
            f.observer_table = pointer(f.observer);
            f.observer_entry = word(f.observer_table, 8);
            f.observer_name = name;
            f.active = 0x00be09d2;
            context.observers.invoke_enter(f.observer_entry, f.observer, f.observer_name, f.nested);
        }
        word(manager, 0x14) = word(manager, 0x14) + 1u;
        f.phase = NativeVfsFileBlockScopePhase::writing_name;
        auto* const current_name = at(manager, 0x0c);
        if (current_name != name) {
            const auto length = word(name);
            f.active = 0x00be09e6;
            resize_native_string_header_0041dd40(current_name, context.strings, length, true);
            if (word(name) != 0) {
                const auto count = word(current_name);
                auto* const source = pointer(name, 4);
                auto* const destination = pointer(current_name, 4);
                f.active = 0x00be09fb;
                if (count != 0) std::memmove(destination, source, count);
            }
        }
        if (octet(manager, 0x79) != 0) {
            f.phase = NativeVfsFileBlockScopePhase::diagnosing;
            f.active = 0x00be0a1a;
            f.diagnose("+FileBlock %s", context.actual_empty_0109cef0);
        }
        f.finish();
    } catch (...) { f.failed(); throw; }
}

void leave_native_vfs_fileblock_00bdc9b0(void* manager, const void* name,
    NativeVfsFileBlockScopeContext& context, NativeVfsFileBlockScopeAcquired& acquired) {
    auto& f = *acquired.impl_;
    f.begin(manager, name);
    try {
        if (octet(manager, 0x78) != 0) {
            f.phase = NativeVfsFileBlockScopePhase::notifying;
            const bool empty = word(name) == 0; // Test before loading the actual observer.
            f.observer = pointer(manager, 0x88);
            f.observer_table = pointer(f.observer);
            f.observer_entry = word(f.observer_table, 0x0c);
            f.observer_name = empty ? at(manager, 0x0c) : name;
            f.active = empty ? 0x00bdc9de : 0x00bdc9d3;
            context.observers.invoke_leave(f.observer_entry, f.observer, f.observer_name, f.nested);
        }
        f.phase = NativeVfsFileBlockScopePhase::writing_name;
        auto* const current_name = at(manager, 0x0c);
        f.active = 0x00bdc9e9;
        resize_native_string_header_0041dd40(current_name, context.strings, 0, false);
        auto* const destination = pointer(current_name, 4);
        if (destination) {
            const auto count = word(current_name);
            f.active = 0x00bdc9fe;
            if (count != 0) std::memmove(destination, context.actual_clear_bytes_00ce3a0c, count);
        }
        word(manager, 0x14) = word(manager, 0x14) - 1u;
        if (octet(manager, 0x79) != 0) {
            f.phase = NativeVfsFileBlockScopePhase::diagnosing;
            f.active = 0x00bdca22;
            f.diagnose("-FileBlock %s", context.actual_empty_0109cef0);
        }
        f.phase = NativeVfsFileBlockScopePhase::restoring_gate;
        f.head = pointer(f.list, 4);
        f.restored_node = pointer(f.head, 4);
        if (f.restored_node == f.head) {
            f.active = 0x00bdca3a;
            context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
        }
        if (f.restored_node == pointer(f.list, 4)) {
            f.active = 0x00bdca44;
            context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
        }
        octet(manager, 0x79) = octet(f.restored_node, 8);
        f.phase = NativeVfsFileBlockScopePhase::erasing_gate;
        f.head = pointer(f.list, 4);
        f.erased_node = pointer(f.head, 4);
        if (f.erased_node == f.head) {
            f.active = 0x00bdca59;
            context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
        }
        f.active = 0x00bdca67;
        erase_native_fileblock_gate_iterator_00bdaf40(f.list, f.erase_output,
            f.list, f.erased_node, context.invalid_parameters);
        f.finish();
    } catch (...) { f.failed(); throw; }
}
} // namespace bsp
