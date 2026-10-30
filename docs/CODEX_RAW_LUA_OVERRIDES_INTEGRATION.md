# Raw Lua override integration

The raw BDEF90 binding uses the existing VFS application, raw string cells,
captured-entry existence lookup and 0Ch string-vector operations. It borrows the
actual mapped slash/dot/underscore spans and retained 0109CEF0 fallback role.
It creates no VFS provider, semantic string/container owner or startup operation.

Primary review of the complete saved body confirms the conditional design's
captured/current reads and ten-state cleanup graph. Normal footers capture data
before disarming an action and read current length afterward. Underscore normal
return uses its earlier data/length captures; unwind uses its current header.
Failed callee outputs are not armed or cleaned a second time by the caller.

The reverse-find correction is 0265ee8ad. Raw assignment/concat byte-field
corrections are 276857287; both exact changes pass independent source review
and strict Win32 object checks. Inverse projections preserve unrelated bodies
and cleanup behavior. The new override body is f73a9ecbd; its complete source
passes primary and independent review plus a strict Win32 object check.
Existing lower-provider
fencing, exception, CRT and zero-copy qualifications remain explicit; outer
fences do not establish transitive Native ABI/fault parity.

The new body is not yet registered or linked. Its bsp_game CMake entry is queued
behind a peer's held file lease; the prerequisite build above does not cover it.
Complete registration and the combined build before claiming link validation.
Raw16 growth, AA0020
parser composition, loaded locale and startup/runtime activation remain separate.
