#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 vector wrappers require MSVC Win32.
#endif

namespace bsp {

// Complete 00421A50..00421AD8. Original ECX raw vector, four stacked words:
// actual output iterator, input iterator owner, position, value. RET10h;
// EAX actual output iterator. Source EDX is an ignored binding placeholder.
// Preserve the early owner/first capture, raw SAR3 index, returning validation,
// physical argument reloads, and result-position store BEFORE result-owner.
// A returning foreign/null-owner validation continues in the actual receiver;
// no replacement iterator class or extra admission policy is introduced.
void* __fastcall insert_one_native_pod8_vector_checked_00421a50(
    void* owner, void* unused_edx, void* output_iterator,
    const void* iterator_owner, void* position, const void* value);

// Complete 004223B0..00422432. Original ECX raw vector, one stacked value
// pointer, RET4; no established semantic return. Source EDX is unused.
// The spare path uses the real null-skipping C7E0 kernel with its four actual
// stacked words, including the partial local tag. It publishes captured last+8.
// The growth path keeps its captured position across returning validation,
// reloads the physical value argument and invokes the complete checked entry.
void __fastcall append_native_pod8_vector_004223b0(
    void* owner, void* unused_edx, const void* value);

// Raw header proxy+0/first+4/last+8/end+Ch and eight-byte values must have
// backing for every reached access. Literal DWORD arithmetic, aliases and
// current argument/result/control slots retain the Native schedule. The full
// genuine growth/insertion and current SDK invalid-parameter providers are
// required; no std::vector, callback, early success or partial fast path is
// substituted. Current CRT, C++ frames, Native virtual/runtime ABI and gameplay
// remain separate evidence boundaries. See GOAL15G_POD8_VECTOR_APPEND.md.

} // namespace bsp
