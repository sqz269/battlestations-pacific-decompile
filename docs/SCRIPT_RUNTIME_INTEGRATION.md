# VFS Lua and keyboard runtime composition

This batch connects the reconstructed script runner to mounted VFS providers,
the profile DLC commit to a real Lua owner, and keyboard settings application
to the existing action records and device resolver. These are new C++ interfaces,
not native object layouts or a runnable game claim.

## Script variants and cache evidence

`append_lua_script_overrides_00bdef90` reconstructs the full normal-flow body
`00bdef90..00bdf306`. Native ECX is the VFS manager; stack arguments are a
NativeString path pointer and output vector pointer; return is `RET 8`.
Pseudocode drops the output argument, established by assembly at `00bdf22d`.
The manager's ordered suffix vector is `+48/+4C`.

At `00bdefc9`, `004bcb80` searches rightmost `/` using literal `00ce7898`.
The `_strcspn` call then finds the first `.` at or after that separator, using
literal `00ce3a70`. Backslash is not a separator here. The stem, `_` literal
`00ce7890`, suffix and extension form each candidate. Virtual `+8` existence
at `00bdf227` gates append `004cdc20` at `00bdf235`. Output is not cleared;
duplicates and suffix order survive. The diagnostic calls are omitted.
The host rejects embedded NUL and lengths beyond signed 32-bit storage and
requires stable input/callbacks with a distinct output vector.

`VfsLuaScriptFiles` uses the existing `00bdf310` memory-opening and `00bdd440`
existence projections. Lua file reads use mode 2. A missing open is distinct
from an opened stream with incomplete backing; the latter throws rather than
trying a lower-priority provider. Exact read counts are checked by the host.
No asset-prefix/extension candidate search is inserted. The ordered suffix
list is explicit caller-owned state; its initialization is not inferred from
the settings downloaded-content list.
Read-only follow-up identified the append thunk `00bdef80` (`ADD ECX,48h;
JMP 004cdc20`) and callers `0073bf22` within `0073bc40` and `00995e06`
within `00995690`. They gate package/XLive content suffix registration;
the constructor `00be1dc0` initially clears `+48/+4C/+50`. Those registration
paths remain outside this composition.

`cached_fundamentals_00884770` composes the normal successful path of
`00884770` (no arguments, EAX singleton, `00884770..0088482c`) and `00b68340`
(ECX object, EAX object, RET, `00b68340..00b68458`). It reads
`Scripts\fundamentals.lua` once through VFS, retaining the bytes for later
owners. Native `00b683e1..00b6840a` stores size, allocates, reads and caches
without a nonzero-size check. A successful empty file is accepted, including
by `PcStorageLuaOwner`; missing/incomplete reads throw. Native global locking,
lifetime registry, allocator, unchecked failure and exception ABI are not
reimplemented by this application-owned, serialized cache.

## Concrete consumers

`VfsProfileDlcHost` executes the existing `007fae70` content commit through
`LuaScriptRuntime`, including base and ordered override files. It supplies
current PC/X360COMP/region globals and real DoFile callbacks to a Lua5.1.1
owner, and releases its borrowed reader before closing that owner. Platform
download management, localization and scene selection remain required host
calls. This class does not claim the native stack-owner layout.

`KeyboardActionRuntime` connects `006aa640` to recovered `006aa090`,
`00a92260` and `00a93750`, then existing `00a91e80` device rebinding. The
native binding `+08` word denotes a retained device pointer: zero maps to
null; nonzero requires an explicit native-word-to-live-device translator.
Registration at action byte `+00` is independent of enablement at `+01`.
Native flag padding is outside the value projection. Device groups must
satisfy the existing rebind class-index contract.

The axis predicate may insert absent input descriptions. Existing settings
apply snapshots descriptions before invoking it; traversal after insertion
remains unverified for incomplete settings. Normal registered descriptions,
ordered code comparison, alternate slots and device resolution are covered
by the worker evidence and combined validation.

Validation and annotation/save/readback details are recorded in
`reports/script_runtime_integration.json`. Worker evidence is in
`LUA_SCRIPT_RUNTIME.md`, `KEYBOARD_AXIS_PAIRS.md` and
`INPUT_BINDING_INSTALL.md`. Host fixtures and native relocated/adapted
differentials have separate scopes; neither is gameplay validation.
