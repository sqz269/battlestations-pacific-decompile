# Checked string-vector raw field qualification

The existing checked10h string-vector append450540, insert-one450180 and
insert-count44F410 implementations accept actual raw headers. Their shared
local DWORD accessors previously used typed volatile lvalues over those bytes.
Each accessor now uses a fenced four-byte copy. No raw DWORD object lifetime,
alignment overlay or semantic container cast is assumed.

Only the local word/put helpers and required includes change. Full implementation
bodies are inverse-exact after removing that substitution: read/write callsites,
DWORD wrapping, returning invalid-parameter continuations, growth, publication,
selected cleanup, normal temporary return and lower calls remain unchanged.
The same byte operations also accept the genuine local DWORD output arrays.
Compiler barriers preserve explicit operand epochs; they add no synchronization.

All three translation units pass strict MSVC Win32 object compilation. Final
independent review and canonical build evidence are recorded in the matching
integration report. No new test, game/probe or Original/vendor body credit is
introduced. This corrects existing Source access; it grants no table registration,
loaded-manager, raw16, startup, Original ABI/FH3/SEH or runtime authority.

The raw alias constructor and range-assignment corrections are reviewed
separately in CODEX_CHECKED_STRING_PROVIDER_FIELD_QUALIFICATION.md. Neither
packet supplies a new canonical caller or claims transitive runtime closure.

Primary and both independent reviewers accepted the exact corrected sources.
Three strict x86 objects are014C. The joined canonical Win32 build with all five
field corrections passed existing checks2/2 first at83c7 (9.95s), then after Main's
Frame/child-lookup publication at6f786fd (10.41s). Source pins stayed unchanged.
