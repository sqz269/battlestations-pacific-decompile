#include "bsp/native_session_message_append.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session message append reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
template<class T> volatile T& field(void* owner, Word offset) noexcept {
    return *reinterpret_cast<volatile T*>(
        reinterpret_cast<Word>(owner) + offset);
}
} // namespace

void append_native_session_message_0076e470(
    void* actual_session, NativeSessionMessageStorage* message) {
    auto& buffer = field<void*>(actual_session, 0x25c);
    auto& count = field<Word>(actual_session, 0x260);
    auto& capacity = field<Word>(actual_session, 0x264);
    const Word captured_capacity = capacity;
    if (count == captured_capacity) {
        const Word grown = captured_capacity * 2u + 2u;
        if (grown > capacity) {
            capacity = grown; // 0076E491: before allocation, even if it throws.
            const Word bytes = grown > 0x3fffffffu ? 0xffffffffu : grown * 4u;
            void* const replacement = singleton_lifetime_allocate({
                SingletonAllocationKind::pointer_slots, bytes, bytes});
            if (buffer != nullptr) {
                Word index = 0;
                if (count != 0) {
                    do {
                        const Word value = field<Word>(buffer, index * 4u);
                        field<Word>(replacement, index * 4u) = value;
                        const Word current_count = count;
                        ++index;
                        if (index >= current_count) break;
                    } while (true);
                }
                singleton_lifetime_free(buffer); // Fresh +25C, returns normally.
            }
            buffer = replacement; // 0076E4F8, after free and caller cleanup.
        }
    }
    const Word append_index = count;
    void* const append_buffer = buffer;
    field<NativeSessionMessageStorage*>(append_buffer, append_index * 4u) = message;
    count = count + 1u;
}
} // namespace bsp
