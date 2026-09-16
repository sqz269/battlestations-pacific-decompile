# Native system constant gather R44

## Result and ownership

This packet adopts the corrected raw `00B46A70..00B47638` gather and nine raw
shadow, texture and foliage accessors into the current source tree. It is a new
explicit source interface, not a binary replacement or gameplay validation.
The historical source is `a6b2884f2`, including primary captured-profile correction
`1bbc1662`; the pre-correction `80027137` worker source is not used.

The current timer interval `00BEE070` in `native_frame_clock_actual.cpp` and light
shadow-map owner `00B7AAB0` in `native_material_constant_metadata.cpp` are reused.
Their historical duplicate declarations and bodies are omitted. The only other
source substitutions are the current provider includes and the current B7AAB0
symbol. Exact equality after these substitutions is recorded in the report and
frozen adaptation diff. Existing projected shadow/time wrappers cannot replace
these raw field reads or their load order.

## Contract

Original gather inputs are ECX optional scene and EDX camera; it returns with
plain RET. The source adds a borrowed 68-byte context and persistent frame as
stack arguments, with RET8. The initialized caller prefix preimage is exactly
1,232 bytes and is copied bitwise before native stores. Unwritten words retain
that preimage. No synthetic default bank or copied native owner is introduced.

The context borrows the actual service, renderer, timer, sampler, manager,
foliage, parameter, register-count and numeric-constant cells. Captured owner
profiles are validated against the corresponding current borrowed table slot:

| Profile and slot | Concrete provider |
| --- | --- |
| D68D50 + 1C | BEE070 timer interval |
| D5B5D8 + 08 | A8FCF0 shadow depth texture |
| D61948 + 3C | B3CE50 reported width |
| D61948 + 40 | B3CE60 reported height |

Each bridge receives the original one-time profile capture; it does not reload
owner[0]. The selected table slot remains a live read. Unsupported profiles
raise the explicit source boundary instead of calling numeric game addresses.
The full original x87/SSE schedule, unsigned-width bias, intermediate stores,
raw global reloads, and separate VS/PS renderer/count loads are retained.
The VS HRESULT does not suppress the subsequent PS upload.

The sampler adapter now delegates the established particle-clock singleton
provider. Gather requires only its getter and persistent operation phase; it
owns no adapter-internal snapshots. On failure the child frame remains alive
with its existing diagnostic obligations. Gather adds no rollback, cleanup or
retry. Original private stack aliases, CRT/FH3/SEH and hardware-fault ABI remain
outside this source contract.

## Verification and reproducibility

The machine-readable receipt is
`reports/native_system_constant_gather_main_r44.json`. Native capture verifies
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, base 00400000
before each live read. The 22 relevant live spans match the installed PE across
3,280 bytes. The 47 direct call/tail-jump rows are independently checked against
exact live instruction sites. All Ghidra work in this packet is read-only;
annotation and ledger integration belong to the primary agent.

The restored local HAL fixture is taken from the rehashed primary `1bbc1662`
checkpoint (validation SHA256
`78ee9018977843340db954b871be19c91a27e56c175f83a216053571a02b2f66`).
Its existing two cases use real Direct3D9 HAL devices in a private hidden window,
observe actual VS/PS calls, preserve the initialized preimage, exercise raw
fog/lighting/shadow, unsigned width and x87 reciprocal, and confirm the current
renderer/register count reload after an ignored VS failure. Original COM slots
and vptrs are restored before device destruction. The original four-byte width
and height leaves are the only original code executed differentially.

Module provenance is gathered in the 32-bit fixture process with
`GetMappedFileNameW`, opening the physical mapped path and resolving it through
`GetFinalPathNameByHandleW`. Freezing does not infer WOW64 module identity from a
Toolhelp System32 spelling. Current source, compiler dependencies, configured
projects, libraries, objects, fixture, logs, tools and actual loaded module files
are frozen with individual hashes; the final report records the archive identity.

## Limits

Full original B46A70 was not executed. Supplied native-layout fixture storage is
not proof of game-created owner compatibility. The uncached axes route, cold
sampler creation, unsupported-profile failure, invalid-parameter path and native
unwind/hardware-fault behavior remain unexecuted. No full renderer, gameplay or
visual validation is claimed. The user's game and camera were not touched.
There are no new permanent tests or fabricated mock providers.
