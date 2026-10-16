#pragma once

namespace bsp {
// Whole AD42C0 normal body: BF681B(18h), conditional DWORD zero stores at
// +00/+04/+08, then BYTE +14=1 and +15=0. All other bytes remain untouched.
// Original takes no arguments, returns the raw allocation in EAX, plain RET.
//
// New Win32 Source ABI over the existing owning singleton_lifetime_allocate
// domain: returns genuine nonnull 18h storage or throws an ordinary Source
// exception. Retire this allocation with the SAME singleton_lifetime_free
// domain when its actual tree lifetime permits; this leaf does not own it.
// Raw field accesses assume that SDK's aligned, writable storage domain.
// Native null/invalid-address paths, private heap/new-handler/exception ABI,
// and fault timing are not reproduced. No Font object, count, registry,
// publication, initializer completion, or whole Text rebuild is established.
void* allocate_native_font_tree_head_00ad42c0();
} // namespace bsp

#include <cstdint>

namespace bsp::game { class GameNativeStringProcess; }

namespace bsp {
// Required borrowed cells for AD55C0's four incoming stack words. Cell identity
// is stable; values remain live across allocation/name-provider calls. Only the
// low BYTE of uppercase_word is read, at its late Native operand position.
struct NativeFontConstructorArguments final {
    const void* const volatile& name_header;
    const volatile std::uint32_t& scale_bits;
    const volatile std::uint32_t& alpha_bits;
    const volatile std::uint32_t& uppercase_word;
};

// Whole AD55C0 normal body and its sole tree-cleanup action, with a new Source
// ABI. Original ECX=Font, stack=(name, scale bits, alpha bits, uppercase word),
// EAX=same Font, RET 10h. No float conversion or arithmetic is performed.
//
// Before entry the caller independently retains aligned, writable AC-byte raw
// Font storage, the SAME SDK CRT/string process, all four actual cells, and
// every reachable header/backing/tree/node through callbacks and failure.
// Argument/view/process/private-frame metadata must be mutually disjoint from
// the raw Font/tree/string regions. No callback may retire those regions or
// change their authority; permitted current-field/value changes stay valid.
// The existing Source CRT/raw-pool helpers' storage and alias domains apply.
// No metadata preflight reads an argument value or a raw name header.
//
// The name pointer is captured only after current head self-links and count=0.
// Ordinary Source exceptions after the identity comparison consume the sole
// cleanup arm before FULL 86FDE0 on the Font BASE; cleanup escape terminates.
// There is no name tidy, outer-Font free, rollback, retry, or empty-tree shortcut.
// Font +00, +16..17 and +49..AB retain their preimage. This is not a semantic
// FontDescriptor cast, ownership facade, loader/registry/retirement completion,
// Native FH3/private-frame/fault/SEH/catch-ABI replacement, or runtime proof.
void* construct_native_font_storage_00ad55c0(void* actual_font,
    const NativeFontConstructorArguments& arguments,
    game::GameNativeStringProcess& same_strings);
} // namespace bsp
