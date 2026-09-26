#include "bsp/native_texture_source_clear.hpp"
#include "bsp/native_procedural_resource_lifetime.hpp"
#include "bsp/native_texture_source_storage.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)) + offset));
}
}
NativeTextureSourceClearOperation::~NativeTextureSourceClearOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeTextureSourceClearOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase != Phase::failed) std::terminate();
    phase = Phase::diagnostic_retired;
}
void clear_native_texture_source_children_00c303b0(void* owner,
    NativeRenderActualOwners& owners, NativeTextureSourceClearOperation& op) {
    if (op.phase != NativeTextureSourceClearOperation::Phase::fresh)
        throw std::logic_error("texture source clear diagnostic is one-shot");
    op.owner = owner;
    op.phase = NativeTextureSourceClearOperation::Phase::running;
    try {
        auto& payload = *std::launder(reinterpret_cast<NativeTextureSourcePayload*>(at(owner, 8)));
        volatile auto& actual = payload;
        std::uint32_t index = 0;
        while (index < static_cast<std::uint32_t>(actual.children_08.count_04)) {
            op.native_site = 0x00c303c0;
            void** const array = actual.children_08.data_00;
            void* const child = *static_cast<void* const volatile*>(at(array, index * 4u));
            void** const slot = static_cast<void**>(at(array, index * 4u));
            op.index = index;
            op.array = array;
            op.child = child;
            op.captured_slot = slot;
            op.child_release_started = false;
            op.child_release_returned = false;
            op.slot_clear_returned = false;
            if (child) {
                op.native_site = 0x00c303cd;
                op.child_release_started = true;
                release_native_render_actual_owner(owners, child);
                op.child_release_returned = true;
                op.native_site = 0x00c303e3;
                *static_cast<void* volatile*>(slot) = nullptr;
                op.slot_clear_returned = true;
            }
            index += 1u;
        }
        op.native_site = 0x00c303f3;
        op.resize_started = true;
        resize_native_procedural_pointer_array_00737390(payload.children_08, 0);
        op.resize_returned = true;
        op.native_site = 0x00c303fe;
        actual.initialized_14 = 0;
        op.initialized_clear_returned = true;
        op.phase = NativeTextureSourceClearOperation::Phase::complete;
    } catch (...) {
        op.phase = NativeTextureSourceClearOperation::Phase::failed;
        throw;
    }
}
} // namespace bsp
