#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore resident insertion requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Complete BE4D20: ECX actual iterator {owner,node}, RET or invalid-parameter
// tail. A returning initial validation resumes with the CURRENT node; decrement
// end to rightmost, otherwise visit maximum(left) or climb current parent links.
void decrement_native_file_store_resident_iterator_00be4d20(void* actual_iterator,
    const SingletonLifetimeCallbacks&);

// Complete BE6170: ECX output0Ch pair, stack source, EAX output, RET4. The
// complete body matches BE6250 except relocated calls/jumps and FH3 metadata.
// Reuse that explicit source operation: clear header before identity test,
// copy current string fields, clear stream before rereading source and retain.
void* copy_native_file_store_resident_pair_00be6170(void* output,
    const void* pair, NativeStringStorage&);

// Complete BE6590 plus catch-all BE660E..BE6622 inclusive: five stack words
// left,parent,right,pair,color, EAX node, RET14. Allocate actual1Ch, publish
// links, copy pair at+C, then color+18 and nil+19=0; +1A/+1B untouched. Pair
// failure calls RET-only placement cleanup, frees raw node, then rethrows.
void* allocate_native_file_store_resident_node_00be6590(void* left, void* parent,
    void* right, const void* pair, std::uint8_t color, NativeStringStorage&);

// Complete BE6CF0: ECX tree, stack output,left-byte,parent,pair; EAX output;
// RET10. Actual tree head/count+4/+8, nodes1Ch, black+18,nil+19,pair+C and
// retained stream+14. Unsigned count>=15555554h throws the existing owning
// NativeHardwareLayoutTreeLengthError with native D69260 legacy payload.
// After successful allocation increment CURRENT count, link, rebalance, set
// current root black; publish output NODE before OWNER. No lookup, comparator,
// duplicate check or std::map state is introduced by this insertion primitive.
void* link_native_file_store_resident_node_00be6cf0(void* tree, void* output,
    std::uint8_t insert_left, void* parent, const void* pair, NativeStringStorage&);

// New explicit C++ service interfaces, not original binary/FH3/SEH ABI.
// The resident pair-copy definition is supplied by native_filestore_completion.
// Existing string release is noexcept; native exception personality, hardware
// faults, concurrent mutation and gameplay remain outside verified evidence.
} // namespace bsp
