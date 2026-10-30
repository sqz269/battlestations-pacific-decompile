#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Font glyph selection requires MSVC Win32.
#endif

namespace bsp {
// Actual caller8B iterator output, not a Font/map/glyph owner. No initializer,
// implicit zeroing, destructor, retain or lookup policy. Embedded-character
// arms never read/write this scratch. Distinct scratch for every entered call;
// caller retains it and the whole genuine raw Font/head/node/payload graph.
struct NativeFontGlyphSelectionStorageScratch final {
    std::uint32_t iterator_words[2];
};

// WHOLE AD4480..AD44FD[126B/45ops], native ECX actual Font, stacked code-unit
// DWORD (only lowWORD is read), EAX borrowed glyph pointer, RET4. This new C++
// interface borrows that actual lowWORD home; no highWORD/Unicode/range policy.
// Initial key is read FIRST. SPACE/LF return actualFont+4C; CR returns+8C.
// Others use existing genuine raw AD4370 with SAME actual key home, then capture
// current head before the invalid-owner guard. End iterator returns actual+6C.
// Other nodes go through privately composed WHOLE AD3E70, then CURRENT pair+4.
// The actual returned pointer may be NULL and is returned unchanged: no fake
// glyph, fallback-on-null, copied payload, texture operation or new owner.
//
// AD3E70[36B/14ops] preserves current owner/head/node reads around each real SDK
// BF6713 call. The selected SDK invalid-parameter transport may return or throw;
// no noreturn/noexcept/termination/cleanup/retry/rollback is invented. Its CRT/
// handler/Watson differences remain the existing Source qualification.
//
// Caller supplies genuine initialized AC-byte Font and live tree/head/nodes/
// key-value pairs/embedded and pointed glyph payloads, valid key WORD object and
// aligned output scratch, serialized and disjoint from Source/private frames.
// Callback mutations remain valid through their reached reloads; captured Font
// and the selector's captured pre-handler head must remain usable. No Source
// FontData/FontDescriptor/typed owner reinterpret, cache/index shortcut, pool,
// allocation, reference count or FP operation is performed. Existing populated
// Font production/resource lifetimes and AB9FD0/AB98F0 caller binding are separate.
// Native register/private-stack/FH3/SEH/fault/runtime identity is not supplied.
void* select_native_font_glyph_storage_00ad4480(void* captured_actual_font,
    const volatile std::uint16_t& actual_code_unit_home,
    NativeFontGlyphSelectionStorageScratch& actual_scratch);
} // namespace bsp
