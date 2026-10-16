#include "bsp/native_scene_command_list_clear.hpp"

#include "bsp/native_scene_command_record_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw storage reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word p) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p));
}
void* add(const void* p, Word offset) noexcept { return pointer(bits(p) + offset); }
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void write(void* p, Word value) noexcept { *static_cast<volatile Word*>(p) = value; }
} // namespace

void clear_native_scene_command_list_0046a9f0(void* actual_header12,
    void* actual_record_destructor_saved_this4, NativeStringRawPoolContext& strings) {
    void* const header = actual_header12;
    while (read(header) != 0) {
        void* const node = pointer(read(add(header, 4)));
        void* const record = pointer(read(add(node, 8)));
        if (record != nullptr) {
            destroy_native_scene_command_record_00469010(record,
                actual_record_destructor_saved_this4, strings);
            singleton_lifetime_free(record);
            write(add(node, 8), 0);
        }

        void* const previous = pointer(read(node));
        if (previous != nullptr) {
            const Word current_next = read(add(node, 4));
            write(add(previous, 4), current_next);
        } else {
            const Word current_next = read(add(node, 4));
            write(add(header, 4), current_next);
        }

        if (read(add(node, 4)) != 0) {
            void* const current_next = pointer(read(add(node, 4)));
            const Word current_previous = read(node);
            write(current_next, current_previous);
        } else {
            const Word current_previous = read(node);
            write(add(header, 8), current_previous);
        }

        write(header, read(header) - 1u);
        singleton_lifetime_free(node);
        // The loop rereads CURRENT count, then CURRENT head after the free.
    }
}
} // namespace bsp
