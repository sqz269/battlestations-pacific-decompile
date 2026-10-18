# Raw gunnery target constructor 00864880

Owner agent/oct18-rec; coordinator follow-up after the 96-row reconciliation.
Whole ordinary source: `src/oct18_gunnery_target.cpp`, public contract in
`include/bsp/oct18_gunnery_target.hpp`. No host wiring or gameplay runs.
Names remain descriptive hypotheses. The existing pure ranging projection and
running `game_hosts_gunnery.cpp::engagement_00864880` are retained unchanged.

## Original evidence and ABI

Verified project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Original [00864880,00864B9E): 798 bytes, 219 instructions, SHA-256
`17b7af4483ce4aaba82acfff51338870acd6a9183912b73a9a8a8b693f0132ec`.
Original entry is thiscall: ECX 8Ch-byte record, stack owner and target pointers,
EAX record result, RET8. New source takes an explicit borrowed dependency domain.
Address/source greps preceded all writing; no dependency implementation was copied
or replaced. Matrix copying, CRT sqrt and random sampling reuse existing code.

The observer base clears only +4/+8/+C dwords and byte +10, then the record stores
vtable 00D0D2FC, owner at +18, owner's +390h class index at +14, target at +1C.
00694A60 is called with ECX target and EDX record, not owner and target as two
ordinary arguments. Clock +84 is copied to +80 through x87. The maximum-error
word and target pointer are captured before the conditional pose refreshes;
the row table and class index are reloaded after length computation.

The PE bytes at 00864979 are **DC C8**, FMUL ST0,ST0. The Ghidra listing is not
the authority for DC-form x87 instructions. The distance kernel keeps all three
float difference spills, the original squared-sum order, float sum spill,
double cutoff comparison including unordered JBE, ST0 CRT sqrt binding, and
the post-sqrt float spills. `native_crt_sqrt_st0_00bf7030` borrows actual CRT state;
there is no substitute local sqrt implementation.

## Exact own RNG calls

Both calls use `native_particle_random_range_00bd2f10` with the **existing**
`NativeParticleUnitRandomAccess` / `RandomThreads` domain and ECX=1
(`RandomStream::secondary`). No new seed, generator, stream owner or per-record
counter is introduced. The two calls occur in this order on every ordinary path:

| Original call | Bounds, rounded as in the original | Commit |
|---|---|---|
| 00864A12 | lower=f32(cap * row[+14h]), upper=cap; cap selects captured maximum or f32(row[+10h] * distance) using the original x87 comparison | ST0 immediately FSTP record+70h |
| 00864A31 | lower=+0.0f, upper=current float [00CE3D9C] (normally bits 40C90FDB) | ST0 immediately FSTP record+6Ch |

A zero-width radius range STILL consumes its draw, before the bearing draw.
Registration/pose failure before the first call consumes no own draw; failures
after both calls retain both consumed draws. Dependency callbacks may themselves
sample randomness; those are their contracts, not extra constructor calls.
The existing host's labelled RNG/draw policy is untouched, so cc9's pairs are
not switched to this constructor by this change.

## Raw aliasing, velocity and cleanup

The source retains the aliased 24h scratch span corresponding to B+10h..B+33h:
axis and first velocity output share B+1Ch, products use B+10h/14h/18h, second
velocity output starts B+28h. Vector rotation receives the SAME record+74h as
source and destination. The current config's +88h is the captured think-time;
the decompiler's `unaff_retaddr` is not an argument or a usable reconstruction.

Target world matrix +CCh is copied to record+20h with the existing exact x87
matrix copier. Target virtual +34h is invoked twice with separate scratch outputs.
Its returned pointer is consumed without a vector snapshot. First velocity seeds
record+50h..58h by subtraction; the second seeds +60h..68h by addition after the
conditional current-target pose refresh. Final +64h is explicitly zeroed.
**Record +88h is never initialized by this constructor.** Padding and every other
unwritten word are retained, rather than copying the pure projection's defaults.

Original handler 00C94C48 selects FuncInfo 00DC6A0C, maxState=1, unwind table
00DC6A04={-1,00C94C40}. The funclet reloads the captured record and calls
00695870 (callback-owner destruction). A noexcept source guard is armed before
registration and disarmed only on success; a throwing cleanup terminates.
This reconstructs ordinary C++ cleanup through explicit callbacks, not native
FS/SEH registration, access-violation dispatch, or binary ABI compatibility.

## Validation

Seed verification passes before the native differential fixture. The Win32 build
and all three enabled CTest groups pass. A manifested local native probe relocates
the original 798 bytes' 11 direct calls and six global operands to the SAME fixture
dependencies used by the new driver; all original control branches remain.
It compares the complete 8Ch-byte record and dependency call order for a finite
radius and a zero-width radius. Both match, including preserved +88h and two
velocity calls. The actual canonical RNG's next value matches a separately advanced
state after exactly two draws, and the original radius/bearing bounds are logged:

- finite case: radius 3E80B6FE..3FA0E4BD, bearing 00000000..40C90FDB;
- zero-radius case: radius 00000000..00000000, then the same bearing bounds.

Separate injected registration/velocity failures verify observer cleanup. Fixture
and byte evidence are in `local/rec_gunnery_probe.log`, `local/rec_recovery_seeds.log`,
`local/rec_gunnery_raw.asm` and the local reference/probe source. These are focused
fixture results, not gameplay proof or a claim about untested arbitrary FP/SEH states.
