#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Exact raw NativeString destruction contracts for native14h subtitle and24h
// extratitle records. Each original is56B/24 starts: ECX first, EDX end,
// cached end in EDI, two ignored stacked pointer/tag slots, RET8. These new
// three-argument cdecl Source interfaces do not provide that register/stack ABI.
//
// Require actual live DWORD-aligned record backing/constructed8h string headers
// through the captured [first,end) range, in the SAME genuine raw string-pool
// domain. Retain the context, actual01090AA8/AA4/AA0 cells, record backing,
// providers and independent cleanup metadata through callbacks/failure
// disposition. Each captured buffer stays valid until its release; afterward
// its bits may dangle and grant no retry/dereference permission. Serialize use.
// Empty ranges touch no header/pool. Valid native ranges, including equality
// termination under DWORD arithmetic, are caller obligations; no guards/defaults.
//
// Each step composes real raw41DD20: data+4 first, null skips length/pool;
// otherwise current length+1 wraps, then419CC0/BD1510. End never reloads after
// callbacks. Only the first8-byte header is consumed, without clearing it.
// No title+18 deletion, range-prefix rollback/retry, vector tidy/backing free,
// allocator/heap ownership or consumer admission. On escape the outer stage
// owns subsequent cleanup/custody; partially consumed headers may dangle.
void destroy_native_gui_movie_subtitle_strings_00aaf0e0(void* actual_first,
    const void* actual_end, NativeStringRawPoolContext& actual_strings);
void destroy_native_gui_movie_extratitle_strings_00aaf120(void* actual_first,
    const void* actual_end, NativeStringRawPoolContext& actual_strings);

} // namespace bsp
