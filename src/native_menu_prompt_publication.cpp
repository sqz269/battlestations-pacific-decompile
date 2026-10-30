#include "bsp/native_menu_prompt_publication.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
volatile std::uint32_t& tracked_counter(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(reinterpret_cast<std::byte*>(section) + 0x18);
}
} // namespace

__declspec(noinline) void* get_native_menu_prompt_screen_00425d10(
    NativeMenuPromptConstructionContext& context, NativeMenuPromptPublicationOperation& operation) {
    if (operation.phase != NativeMenuPromptPublicationOperation::Phase::fresh)
        throw std::logic_error("MenuPrompt getter requires a fresh retained operation");
    void* const initial = context.actual_prompt_publication_00e18e6c;
    operation.phase = NativeMenuPromptPublicationOperation::Phase::running;
    if (initial) {
        operation.phase = NativeMenuPromptPublicationOperation::Phase::complete;
        return initial;
    }

    try {
        operation.native_site = 0x00425d36;
        void* const first_manager = get_native_singleton_manager_00415350(
            context.strings.actual_manager_publication_01090aa0);
        auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
            static_cast<std::byte*>(first_manager) + 0x10);
        operation.guard_words[0] = 0x00ce37fc;
        operation.guard_words[1] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(captured_section));
        if (captured_section) {
            operation.native_site = 0x00425d4f;
            EnterCriticalSection(captured_section);
            auto& depth = tracked_counter(captured_section);
            depth = depth + 1u;
        }

        operation.unwind_state = 0;
        if (!context.actual_prompt_publication_00e18e6c) {
            operation.native_site = 0x00425d6f;
            void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x290, 0x290});
            operation.captured_allocation = allocation;
            operation.unwind_state = 1;
            void* constructed;
            try {
                operation.native_site = 0x00425d86;
                constructed = allocation ? construct_native_menu_prompt_screen_00533120(
                    allocation, context, operation.construction) : nullptr;
            } catch (...) {
                operation.failure_native_site = operation.native_site;
                operation.failure_unwind_state = operation.unwind_state;
                operation.unwind_state = 0;
                operation.native_site = 0x00c5e7dc;
                singleton_lifetime_free(allocation);
                throw;
            }
            operation.unwind_state = 0;
            context.actual_prompt_publication_00e18e6c = constructed;
            void* const current = context.actual_prompt_publication_00e18e6c;
            void* const adjusted = current ? static_cast<void*>(static_cast<std::byte*>(current) + 0x0c) : nullptr;
            operation.native_site = 0x00425daa;
            void* const second_manager = get_native_singleton_manager_00415350(
                context.strings.actual_manager_publication_01090aa0);
            operation.native_site = 0x00425db1;
            register_native_singleton_object_00bd0c30(second_manager, nullptr, adjusted);
        }
        if (captured_section) {
            auto& depth = tracked_counter(captured_section);
            depth = depth - 1u;
            operation.native_site = 0x00425dbf;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        operation.phase = NativeMenuPromptPublicationOperation::Phase::failed;
        if (operation.failure_native_site == 0) {
            operation.failure_native_site = operation.native_site;
            operation.failure_unwind_state = operation.unwind_state;
        }
        if (operation.unwind_state >= 0) {
            operation.unwind_state = -1;
            operation.native_site = 0x00c5e7d3;
            destroy_native_singleton_guard_00411ee0(operation.guard_words);
        }
        throw;
    }
    operation.phase = NativeMenuPromptPublicationOperation::Phase::complete;
    return context.actual_prompt_publication_00e18e6c;
}
} // namespace bsp
