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

Canonical registration now uses the existing deferred Game pattern in the
unleased root CMakeLists.txt, at 51102e823. Independent review confirms exactly
one registration; the peer's held startup file is unchanged and the queue request
is canceled. The standard Win32 build and both existing checks pass (11.46 seconds)
without a local overlay. The body remains uncalled by a loading caller.

Earlier local CMake overlay evidence separately retained both public entries
in bsp_game (constructor 103009A0, raw body 10300B00, plus unwind metadata).
Both existing checks pass (2/2, 11.21 seconds). The exact linked PE is retained
under local/raw_lua_overrides_local_link_cc16_20261030/; no game/body ran.

The local overlay initially ran before target creation; moving its registration
after the deferred target creation corrected that configuration error. The
overlay cache setting is removed. Neither local retention nor the later canonical
build establishes a loading caller, runtime behavior or Original ABI parity.
Raw16 growth, AA0020
parser composition, loaded locale and startup/runtime activation remain separate.
