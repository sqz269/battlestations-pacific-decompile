# Actual GUI resource probe shader-cache bracket (CC12)

Packet `cc12_gui_effect_shader_cache_bracket`, Source base `9daa118b1`.
Owned application composition at cache entries `00B3A600`/`00B3B140`; the held
`0073BF80` caller and native cache/compiler bodies are read-only dependencies.

## Exact crash and classification

Root's immutable GUI run 04 used executable Source
`e20f1b31bac3338fe006f4a222fa2119edc75d1b`, SHA256
`c14be9b4b25afb29672d1a58556541d959f0e6f4ec0346de34c8b24209248bb6`,
PE timestamp `6ad005ce`, preferred/actual base `10000000`. Slot 5/core 12 ran
`2026-10-14T22:56:04.1589929Z..22:56:06.2370511Z`, ending in hardware access
violation `C0000005`/`-1073741819`. Launcher checkout metadata `9c5991f6`
postdates the immutable build Source and must not replace that identity.

The frozen CDB exception context reports `1038EB9D`, bytes `8b07`,
`mov eax,[edi]`, EDI zero, read address zero and top cache argument zero.
The matching map places this at
`find_native_shader_binary_cache_record_00b34890 +3Dh`,
`native_shader_compiler_lookups.obj`. Return addresses map through
`cached_shader`, actual material compiler/children/program/variant/cache
loading and GUI acquire/run. CDB warns that later frame unwinds may be wrong;
the fault instruction/register and top argument are direct dump observations.
Root's frozen map, dump, transcript and launch receipt are recorded by exact
hash in the report.

The real effect graph already checks that its compiler/cache context borrows
the canonical `GameNativeShaderProcess::cache_0108d6ec()` cell. Its constructor
and acquire admission ran before the crash. The reached cached-shader branch
uses the unchanged actual mode bytes. With source mode zero, Original
`00B348A3` and its Source counterpart dereference the cache owner. No null cache
is a successful miss/default. The compiler lookup and its guards are unchanged.

`ShaderGraph` already provides the real constructor/record loader and release
over this same process cell and actual VFS/string/stream/allocation domain.
The resource probe never scheduled its create/release methods. This is a
**missing probe cache lifecycle prerequisite**, not evidence of a private or
incorrectly bound cache cell. It is separate from the earlier numeric resolver
and Lua activation failures, whose receipts remain frozen.

## Original-supported schedule and bounded composition

Original `0073BF80` calls `00B3A600` at `0073BFA6`, before its Lua bootstrap and
material iteration. `0073C12F` invokes the current renderer `+48` per preload
value. `0073C1BE` calls `00B3B140` after that iteration and before reverse Lua
cleanup. This packet verifies bounded saved/live/Original spans; it does not
mutate, annotate, claim the ABI of or activate the whole held caller.

The opt-in resource probe now schedules the existing renderer cache creation
after real shadow construction and before its cold GuiFont acquisitions. It
requires an unused canonical bracket: no existing creation operation or cache
publication. Creation publishes the persistent operation before native entry.
The native constructor uses the same actual `shaderfx/shaders.bin` literal,
current manager, mode references, native stream route, string pool, record
allocator and global publication. No private owner, default cache, fake record,
mode bypass, extra registration or refcount is introduced.

Actual run 04 log line 63 reports `variants=0 source=0 hires=0 reload=0`; its
manifest has no shader-mode override. Those selected cells choose flags 2/read
for the native cache file. The existing variants-1 branch uses flags 5/write
and rewrites the count during release; it is not silently normalized to read.
The caller must preserve/pin any real writable target before choosing that
mode. Root owns that launch/backup decision; this worker opens no cache file
and changes no original installation. New observations capture the actual
shared source/variants bytes before creation without overriding either one.

The probe records actual mode bytes for observation only; the native helpers
still resample their actual shared cells. It rejects a real null allocation as
an explicit resource-probe boundary before admitting a binary consumer. The
primitive's null publication semantics are preserved; no owner is fabricated.

Only after both actual acquisitions return does the probe call the existing
release. This follows the established create/acquire/release order while the
effect caller credits remain owned in their stable probe slots. Original
release destroys the captured cache, frees it, then clears publication; no
caller effect credit is released as part of cache retirement. Existing reverse
effect/name retirement still occurs before shared native renderer drain.

This is a bounded **resource-probe bracket**, not the full preload script,
application `+08` vector, `FileNames` order or normal frontend activation.
Cursor ordering, absent/mismatched compiled records, actual stream/provider
support and later compiler/sampler branches remain genuine runtime boundaries.
A missing cache row still follows existing native behavior; this packet does
not soften that separate policy or seed private sampler bytes.

## Identity, retention and failure diagnostics

The renderer's existing graph now rejects creation over another publication
and release retargeting: current publication must equal its recorded created
owner. Its existing one-shot operation/completion/released guards remain.
There is no destructive replacement, automatic retry or adoption of another
bracket. Serialize mode/cache/renderer/VFS use and retain every borrowed parent.

Read-only metadata accessors expose the existing creation operation and release
status. They perform no native work and remain readable after entered failure
or normal release. Raw cache/allocation pointers become opaque diagnostic
identities after release; diagnostics never dereference them then.

The retained probe records creation/release entered/returned flags, captured
cache identity and mode observations. Failure output includes the real
creation operation's phase/function/native site, raw cache/allocation identity,
completed records, index and temporary-header ownership. The new creation and
release stages have explicit diagnostic names. The release helper has no
separate persistent substep frame: entered/returned flags and the renderer's
failed state do not claim a recovered release-failure subsite or safe replay.

An entered creation, acquisition or release failure does not run a finally
release or destroy the real partial graph. Existing renderer/probe failure
states and operation destructors require retained parents and the caller's
no-unwind process exit. Successful acquisition followed by release failure
still retains its exact effect credits and cache state. Global drain is not
an alternate cache cleanup path; the existing renderer closure requires the
cache bracket to be quiescent and publication clear.

## Verification and limits

Exact current Source/COFF/log/build artifacts and bounded Original evidence are
recorded in `reports/native_gui_effect_shader_cache_bracket_cc12.json`.
Strict Win32 compilation and the normal registered `scripts/build.ps1` passed;
the exact final header inputs, including the caller contract comment, were
recompiled/rebuilt. Local suites were `reconstructed_math` and `tool_tests`.
Native differential was not configured: `local/seed_reference.hpp` was absent;
the optional CC13 leaf reference header was also absent. These skipped suites
are not claimed as worker evidence; Root supplies its separate combined checks.
No CMake overlay, host change, new tests or live launch belongs to this packet.
Root's next immutable executable/resource run is the acceptance check.

The prior dump proves the null-cache failure. It does not prove this new cache
construction/record loading or GuiFont acquisition succeeds. Native Text draw,
visible frontend/world, full preload/GUI attachment and Original ABI/FH3 parity
remain unproven. No new x87/register/large-body recovery was required.
