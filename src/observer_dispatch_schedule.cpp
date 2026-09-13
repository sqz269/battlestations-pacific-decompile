#include "bsp/observer_dispatch_schedule.hpp"

#include "bsp/native_singleton_vector_insert_count.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"

#include <cstdlib>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word address(const volatile void* p) noexcept {
    return reinterpret_cast<Word>(p);
}
Word read(const volatile void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p) + offset) = value;
}
Word shifted_distance(Word last, Word first) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2);
}
Word size_with_begin(const void* owner, Word begin) noexcept {
    return begin ? shifted_distance(read(owner, 8), begin) : 0;
}
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}

// Only the dispatcher's resize-to-saved-size consumption of00695D00.
// This is not a standalone STL resize/erase implementation. The normal
// native-valid vector domain keeps erase.last at the captured current end,
// so erase-to-end copies no elements and retains capacity. Returning invalid
// handlers that mutate this storage are outside this adapter's guarantee.
void restore_dispatch_size(NativeObserverDispatchStorage* owner, Word count) {
    const Word begin = read(owner, 4);
    const Word size = size_with_begin(owner, begin);
    if (size < count) {
        const Word current_size = size_with_begin(owner, begin);
        const Word captured_end = read(owner, 8);
        if (begin > captured_end) current_crt_invalid_parameter();
        void* fill = nullptr;
        insert_count_native_singleton_slots_00bd0700(owner, nullptr, owner,
            reinterpret_cast<void*>(captured_end), count - current_size, &fill);
    } else if (begin) {
        const Word captured_end = read(owner, 8);
        if (count < shifted_distance(captured_end, begin)) {
            if (begin > captured_end) current_crt_invalid_parameter();
            const Word current_begin = read(owner, 4);
            if (current_begin > read(owner, 8)) current_crt_invalid_parameter();
            const Word position = current_begin + count * 4;
            if (position > read(owner, 8) || position < read(owner, 4))
                current_crt_invalid_parameter();
            if (position != captured_end) {
                // Native erase rereads end here; valid storage still equals
                // captured_end. Its discarded output iterator is unobserved.
                (void)read(owner, 8);
                write(owner, 8, position);
            }
        }
    }
}

class CapturedObserverSection final {
public:
    explicit CapturedObserverSection(TrackedCriticalSection* section)
        : section_(section) {
        if (section_) {
            EnterCriticalSection(&section_->native);
            write(section_, 0x18, read(section_, 0x18) + 1);
        }
    }
    ~CapturedObserverSection() {
        if (section_) {
            write(section_, 0x18, read(section_, 0x18) - 1);
            LeaveCriticalSection(&section_->native);
        }
    }
    CapturedObserverSection(const CapturedObserverSection&) = delete;
    CapturedObserverSection& operator=(const CapturedObserverSection&) = delete;
private:
    TrackedCriticalSection* section_;
};
} // namespace

void dispatch_observer_edges_00695f90(NativeObserverLifetime& lifetime,
    NativeObserverDispatchStorage* volatile& global_00e198e4,
    NativeObserverOwnerStorage& first, NativeObserverDispatchCallback callback,
    void* context) {
    CapturedObserverSection guard(lifetime.lock_owner_00694280()->section_04);
    auto* current = global_00e198e4;
    const Word previous_size = size_with_begin(current, read(current, 4));
    Word cursor = read(&first, 4);
    Word endpoint_end = cursor + read(&first, 8) * 4;
    while (cursor != endpoint_end) {
        const Word begin = read(current, 4);
        Word edge = read(reinterpret_cast<void*>(cursor));
        auto* const captured_owner = current;
        const Word size = size_with_begin(current, begin);
        if (begin && size < shifted_distance(read(current, 0x0c), begin)) {
            auto* const end_field = reinterpret_cast<void*>(address(current) + 8);
            const Word end = read(end_field);
            write(reinterpret_cast<void*>(end), 0, edge);
            write(end_field, 0, end + 4);
        } else {
            const Word end = read(current, 8);
            if (begin > end) current_crt_invalid_parameter();
            Word output_iterator[2];
            // 00695BC0 and BD08D0 are verified equivalent template instances.
            insert_one_native_singleton_slots_checked_00bd08d0(captured_owner,
                nullptr, output_iterator, captured_owner,
                reinterpret_cast<void*>(end), &edge);
        }
        // Retain the old cursor; only the endpoint end and publication reload.
        const Word count = read(&first, 8);
        endpoint_end = read(&first, 4) + count * 4;
        current = global_00e198e4;
        cursor += 4;
    }
    const Word stop = size_with_begin(current, read(current, 4));
    for (Word index = previous_size; index != stop; ++index) {
        const Word begin = read(current, 4);
        // Native EDI retains this field address even if validation republishes
        // the vector. ECX is reloaded on validation's returning error path.
        const auto* const captured_begin_field =
            reinterpret_cast<void*>(address(current) + 4);
        if (!begin || index >= shifted_distance(read(current, 8), begin)) {
            current_crt_invalid_parameter();
            current = global_00e198e4;
        }
        const Word slot = read(captured_begin_field) + index * 4;
        const Word edge = read(reinterpret_cast<void*>(slot));
        if (edge) {
            auto* const owner = reinterpret_cast<NativeObserverOwnerStorage*>(
                read(reinterpret_cast<void*>(edge), 8));
            callback(context, first, *owner);
            current = global_00e198e4;
        }
    }
    restore_dispatch_size(current, previous_size);
}
} // namespace bsp
