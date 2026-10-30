#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole raw vector tidy contracts AAF720/AAF7A0, each61B/21 instruction starts.
// Native ECX is the actual16-byte header; plain RET, no stack inputs or proved
// semantic EAX result. These explicit two-argument cdecl Source APIs are not
// the native register/stack/FH3/SEH ABI.
//
// Require live DWORD-aligned actual {opaque,first,end,capacity} storage, SAME
// genuine RawPoolContext and constructed14h/24h record headers when reached.
// Every CURRENT first used for backing return must be the matching allocation
// from the real AACD30/AACD90 singleton_lifetime_allocate/free Source domain,
// including replacements during callbacks. No foreign/private/new[]/string/
// widget-pool backing is authenticated. Retain header, candidate backing,
// context/cells/providers and independent metadata through their proper native
// phases/failure disposition; metadata/header must not alias freed backing.
// Record/string buffers may end at their real releases; no later dereference.
//
// Capture CURRENT first+4 once. Initial null skips end/range/free, but all three
// pointer fields still clear. Otherwise capture end+8, destroy that range, then
// reread CURRENT first+4 for actual free, even when the replacement is null.
// Clear +4/+8/+C in that order only after normal return; opaque+0 is untouched.
// Callers serialize lifetime/reuse and retain all callback-reached ownership;
// no range/header repair, local catch/rollback/retry, extra-title deletion,
// scalar owner, whole Movie destructor or consumer admission is supplied.
void tidy_native_gui_movie_subtitle_vector_00aaf720(void* actual_header,
    NativeStringRawPoolContext& actual_strings);
void tidy_native_gui_movie_extratitle_vector_00aaf7a0(void* actual_header,
    NativeStringRawPoolContext& actual_strings);

} // namespace bsp
