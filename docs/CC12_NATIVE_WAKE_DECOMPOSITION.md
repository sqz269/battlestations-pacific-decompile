# Native unit wake decomposition, 00811180

`decompose_native_unit_wake_00811180(entity, point, across, along)` adds an
ordinary C++ interface to the complete native wake leaf. Its private Win32
assembly kernel preserves the original 1,683 bytes / 523 instructions after
four real address relocations. It directly calls the existing native
`normalize_camera_basis_0042b260` with ECX pointing to its scratch vector, and
the genuine current CRT `_CIsqrt`. The old `ShipAiWakeTrail` semantic port is
unchanged. This leaf does not establish a game entity producer or connect the
parent `0070ED30` group controller.

## Storage and numerical contract

- The caller supplies actual addressed storage containing all 40 records at
  `entity + 0xBD8`, stride `0x18`, and a signed head at `entity + 0xF98` in
  `0..39`. The leaf reads XYZ at record `+0/+4/+8` and stored segment length at
  `+0x10`. It neither projects these into a host ring nor consults a written
  count, validity flag, callback, or interpolator. The caller owns storage,
  lifetime, initialization, and synchronization.
- `point` addresses three readable binary32 words. Each output addresses a
  writable binary32 word; outputs may alias each other or writable input
  storage. Across is written first, followed by along using a raw MOVSS.
  Shared output pointers therefore end with along. No null or range repair
  is added.
- Search starts with FLT_MAX and ordinal `-1`, scans the native unrolled
  40-sample order, and preserves strict-minimum ties and unordered branches.
  Interior neighbor choice, native head/point rereads, all float spills,
  inclusive stored-length accumulation, and negative along results remain.
  The full neighborhood schedule is recorded in
  [the accepted readiness audit](CC12_WAKE_DECOMPOSITION_SOURCE_READINESS.md).
- Across uses the full three-dimensional residual from the selected adjacent
  line. Its sign preserves the native mixed x87 FCOMIP / SSE COMISS sequence,
  including its unordered and DAZ behavior. Projection and residual products
  spill exactly where the original spills. The residual cutoff can produce
  negative zero. There is no horizontal-only residual, along clamp, or
  idealized normalization substitution.
- All eight x87 stack slots must be available at entry. The kernel preserves
  the ambient x87 control word and MXCSR settings and retains the original
  arithmetic/status behavior; it does not reset the FP environment. Runtime
  cases below cover masked exceptions with 64-bit and 53-bit x87 precision,
  round-to-nearest, and MXCSR `0x1F80`. Other environments are supported by
  instruction preservation, not claimed as separately executed fixtures.

The only new constant storage is immutable FLT_MAX (`D7A248`,
`ffff7f7f`) and the binary64 residual cutoff (`CE3820`,
`bbbdd7d9df7cdb3d`). The existing normalizer owns its cutoff and floor
(`CE3C70`, `f168e388b5f8e43e`). These are original byte payloads, not
invented mutable globals.

## Provider checks before execution

The original installed PE was rehashed and both complete leaf bodies were
checked against its bytes. The new kernel's whole current COFF body matches
the original after exactly two DIR32 data operands and two REL32 calls; the
existing normalizer's 138 bytes / 47 instructions match after exactly three
relocations. The ordinary C++ wrapper is 24 bytes and is a distinct interface,
not a claim of public original-ABI compatibility.

The focused executable links the fresh production `bsp_core.lib`. Before its
first run, the complete objects, exact physical archive members, whole
archive, executable, map, inputs, and build log were retained. All three
complete linked providers were checked against relocated COFF bytes and each
had one physical whole-body occurrence. Map rows and aliases are recorded.
Every data relocation resolves to the expected read-only payload; direct
calls resolve to the real normalizer and `_CIsqrt` import thunk. The latter
is `FF 25` through the IAT for
`api-ms-win-crt-math-l1-1-0.dll!_CIsqrt`.

The Original arm executes independent complete Original wake and Original
normalizer copies, with only their seven address operands relocated. Both
arms use that genuine current CRT backend. The full relocated 4,096-byte map
was saved before the first native call and independently revalidated. The
loader's ASLR delta is recorded. Original CRT dispatch globals, diagnostics,
and exception policy are not reconstructed or claimed equivalent.

## Validation and scope

The first fresh execution passed all five Original/Source pairs over separate,
initialized caller-owned native-layout storage:

| Case | Verified result |
| --- | --- |
| Point ahead of head sample | Along `-2`, not clamped |
| Point `(2,3,0)` beside the Z-axis line | Across binary32 `-sqrt(13)` (`c066c15a`) |
| Both outputs use one pointer | Along's zero is the final stored word |
| Tiny negative-side residual | Across negative zero (`80000000`) |
| Head 37 with an interior pair and bulk length loop | Along `9.5`, under 53-bit x87 precision |

Each pair compared the full 4,024-byte input/output memory image and retained
before/after bytes, guards, and FP/register snapshots. ESP, EBX/ESI/EDI/EBP,
x87 control word and empty tags, and MXCSR settings were preserved; final
x87 status and MXCSR matched between arms. Only addressed output bytes
changed. There was one native run, with no failing pair. An offline checker
initially used the wrong output-field offset; its failure log is retained,
and correcting `0xFB0` to the actual `0xFAC` validated the same saved run.

`scripts/build.ps1` completed the MSVC Win32 Release build and all three
existing tests (`reconstructed_math`, `native_math_differential`, `tool_tests`).
The existing unrelated `spawn_request_id_matches` duplicate-symbol warning
remains. No permanent test target was added.

This is source, complete provider-byte, build/test, and bounded memory-domain
execution evidence. It does not prove a game-owned entity's construction,
concurrent lifetime, the full parent call chain, original CRT policy, a
drop-in public ABI, startup, or gameplay. Machine-readable details and hashes
are in [the packet report](../reports/cc12_native_wake_decomposition.json);
the retained evidence and reproducible probe are in
`local/cc12_native_wake_evidence` and its sibling ZIP.
