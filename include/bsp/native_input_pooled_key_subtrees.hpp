#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual input pooled-key subtrees require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00558310..0055837C. Native ECX tree, EDX unconsumed, stacked
// node, RET4; no semantic result. Source retains those words and adds context.
// Actual28h node: left/parent/right at0/4/8,8B pooled key at+C, mapped raw
// vector opaque+14/data+18/end+1C/cap+20, color24/nil25. Tree is forwarded
// through right recursion but not dereferenced. Nil nodes are not freed.
// After right return: capture mapped backing FIRST, capture left SECOND,
// conditionally free captured backing, always clear18/1C/20, then current key
// data and conditional length+1/getter/return. Free captured current node,
// read captured left's nil byte, then continue left. No head/count reset.
void __fastcall destroy_native_input_word_vector_subtree_00558310(void* tree,
    std::uint32_t unused_edx, void* node, NativeStringRawPoolContext& strings);

// Complete00558E60..00558ECC, same physical interface and28h/nil25 layout.
// Descriptor and word mapped payloads both only free the raw backing here;
// neither iterates elements. Complete109B/46instruction bodies agree after
// normalizing self targets/four external rel32 operands:93 other bytes match,
// including the self-call bytes. Reuse the complete actual implementation.
void __fastcall destroy_native_input_descriptor_vector_subtree_00558e60(
    void* tree, std::uint32_t unused_edx, void* node,
    NativeStringRawPoolContext& strings);

// Both are whole subtree entries, no local EH frame/catch or extra guard.
// Raw41DD20 key composition preserves exact field reads, resolves the current
// real pool before every nonnull return and propagates getter failures. No
// noexcept storage bridge, termination adapter, callbacks, host container or
// allocator substitute. Failures stop later cleanup with earlier effects
// retained. Opaque mapped+14 and key fields are not cleared by these bodies.
// Valid reached actual tree/payload/key ownership and current pool/CRT services
// apply. Extra context/by-value inputs and normalized unused recursive EDX0
// do not bind native stack/register/privateframe/FH3/SEH/fault/CRT/exception
// identity or gameplay. Existing AV/full-range projections/fixtures remain
// separate; record55B690 is not promoted by these two leaves.
} // namespace bsp
