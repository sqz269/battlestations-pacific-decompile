#include "bsp/cc16_dyn_endpoint_allocate.hpp"

#include "bsp/cc16_dyn_endpoint_init.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw-storage reconstruction targets MSVC Win32.
#endif

namespace bsp::cc16_dyn_endpoint_allocate {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4);

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U32& field(const void* owner, U32 offset) noexcept {
    return word(address(owner) + offset);
}

// Three native vector-growth paths share this exact sequence. The caller has
// already captured capacity and compared the live count for equality. Offsets
// describe actual manager words, not an invented vector/native class.
void grow_pointer_words(void* owner, U32 data_offset, U32 count_offset,
                        U32 capacity_offset, U32 captured_capacity,
                        const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 capacity = captured_capacity * 2u + 2u;
    field(owner, capacity_offset) = capacity;
    const U32 allocated = address(crt.operator_new_00bf55be(capacity * 4u));
    U32 index = 0;
    if (field(owner, count_offset) != 0) {
        U32 output = allocated;
        do {
            if (output != 0) {
                const U32 current_data = field(owner, data_offset);
                const U32 value = word(current_data + index * 4u);
                word(output) = value;
            }
            index += 1u;
            output += 4u;
        } while (index < field(owner, count_offset));
    }
    const U32 previous = field(owner, data_offset);
    if (previous != 0) {
        crt.free_00bf6989(pointer(previous));
    }
    field(owner, data_offset) = allocated;
}

// Count precedes data in the native final append; count is read again by the
// increment. Keep the zero COMPUTED destination test, not a base/null guard.
void append_pointer_word(void* owner, U32 data_offset, U32 count_offset,
                         U32 captured_value) {
    const U32 index = field(owner, count_offset);
    const U32 data = field(owner, data_offset);
    const U32 output = data + index * 4u;
    if (output != 0) {
        word(output) = captured_value;
    }
    field(owner, count_offset) = field(owner, count_offset) + 1u;
}

U32 take_endpoint(void* manager, U32 free_offset, U32 pages_data_offset,
                  U32 pages_count_offset, U32 pages_capacity_offset,
                  U32 live_count_offset, U32 sentinel_offset, U32 tail_offset,
                  const cc15_dyn_live::DynLiveCrt& crt) {
    if (field(manager, free_offset) == 0) {
        // Native C55AA0 / C55B9A: both paths allocate 1000 actual 44h slots.
        // A null result is not repaired: the following native-style stores run.
        const U32 pool = address(crt.malloc_00bf9f1a(68000));
        U32 link = pool + 0x40u;
        U32 remaining = 999;
        do {
            word(link) = link + 4u;
            link += 0x44u;
            remaining -= 1u;
        } while (remaining != 0);
        word(pool + 0x1099cu) = remaining;

        const U32 capacity = field(manager, pages_capacity_offset);
        const bool full = field(manager, pages_count_offset) == capacity;
        // The native comparison is BEFORE this free-head publication, while
        // any subsequent new/free callback is AFTER it.
        field(manager, free_offset) = pool;
        if (full) {
            grow_pointer_words(manager, pages_data_offset, pages_count_offset,
                               pages_capacity_offset, capacity, crt);
        }
        append_pointer_word(manager, pages_data_offset, pages_count_offset, pool);
    }

    const U32 endpoint = field(manager, free_offset);
    const U32 next = word(endpoint + 0x40u);
    field(manager, live_count_offset) = field(manager, live_count_offset) + 1u;
    field(manager, free_offset) = next;
    word(endpoint + 0x40u) = address(manager) + sentinel_offset;
    const U32 previous_tail = field(manager, tail_offset);
    word(endpoint + 0x3cu) = previous_tail;
    const U32 reloaded_tail = field(manager, tail_offset);
    word(reloaded_tail + 0x40u) = endpoint;
    field(manager, tail_offset) = endpoint;
    return endpoint;
}
}  // namespace

void* __cdecl allocate_endpoint_00c55a60(
    void* manager, U32 token, const void* bounds, std::uint8_t flag,
    const cc15_dyn_live::DynLiveCrt& crt) {
    U32 endpoint;
    if (flag != 0) {
        // C55A8E..C55B86: free head104, pagesF8/FC/100, live190,
        // sentinel14C and intrusive tail188.
        endpoint = take_endpoint(manager, 0x104u, 0xf8u, 0xfcu, 0x100u,
                                 0x190u, 0x14cu, 0x188u, crt);
    } else {
        // C55B8B..C55C55: free head68, pages5C/60/64, liveF4,
        // sentinelB0 and intrusive tailEC. This is a separate real pool path.
        endpoint = take_endpoint(manager, 0x68u, 0x5cu, 0x60u, 0x64u,
                                 0xf4u, 0xb0u, 0xecu, crt);
    }

    // Native state0 C55C67..C55C84 surrounds this call. Its actual outer
    // unwind action CC83CA calls RET-only00401130: no pool refund/destructor.
    // The real initializer provides its own established vector cleanup/rethrow.
    if (endpoint != 0) {
        (void)cc16_dyn_endpoint_init::initialize_endpoint_00c4ce60(
            token, flag, pointer(endpoint), bounds, crt);
    }

    // C55C8C..C55D0A: independent pending vector, with fresh native fields.
    const U32 capacity = field(manager, 0x19cu);
    const bool full = field(manager, 0x198u) == capacity;
    if (full) {
        grow_pointer_words(manager, 0x194u, 0x198u, 0x19cu, capacity, crt);
    }
    append_pointer_word(manager, 0x194u, 0x198u, endpoint);
    return pointer(endpoint);
}

}  // namespace bsp::cc16_dyn_endpoint_allocate
