#include "bsp/native_scene_command_queue.hpp"

#include "bsp/native_scene_command_list_push_back.hpp"
#include "bsp/native_scene_command_record_construct.hpp"
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
void* add(const void* p, Word n) noexcept { return pointer(bits(p) + n); }
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void write(void* p, Word n) noexcept { *static_cast<volatile Word*>(p) = n; }

class AllocationCleanup {
public:
    explicit AllocationCleanup(const void* cell) : cell_(cell) {}
    ~AllocationCleanup() noexcept(false) {
        if (armed_) unwind_native_scene_command_queue_00c61b10(cell_);
    }
    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }
private:
    const void* const cell_;
    bool armed_ = false;
};
} // namespace

void unwind_native_scene_command_queue_00c61b10(
    const void* actual_saved_allocation4) {
    singleton_lifetime_free(pointer(read(actual_saved_allocation4)));
}

void queue_native_scene_command_00469610(void* actual_database,
    const NativeSceneCommandQueueFrame& frame,
    NativeStringRawPoolContext& strings) {
    void* const database = actual_database;
    const void* const parameters = frame.actual_parameters12;
    void* const saved = frame.actual_saved_allocation4;
    void* const outgoing = frame.actual_constructor_parameters12;
    void* const constructor_saved = frame.actual_constructor_saved_record4;
    void* const list_parameter = frame.actual_list_parameter4;
    AllocationCleanup cleanup(saved);
    write(saved, bits(database)); // Native PUSH ECX before allocator call.
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x14, 0x14});
    write(saved, bits(allocation));
    cleanup.arm();
    void* result = nullptr;
    if (allocation != nullptr) {
        const Word target = read(add(parameters, 8));
        const Word command = read(add(parameters, 4));
        write(add(outgoing, 8), target);
        const Word owner = read(parameters);
        write(add(outgoing, 4), command);
        write(outgoing, owner);
        const NativeSceneCommandRecordConstructFrame child{outgoing, constructor_saved};
        result = construct_native_scene_command_record_004690d0(
            allocation, child, strings);
    }
    write(list_parameter, bits(result));
    void* const list = add(database, 0x14c);
    cleanup.disarm();
    push_back_native_scene_command_list_00468350(list, list_parameter);
}
} // namespace bsp
