# GOAL16L: three complete physical x87 getters

Status: **STRICT_PREPARED_NORMAL_BUILD_PENDING**. This preparation has not run
the normal CMake build. It makes no registered-object, Core archive, final
reconstruction-ledger, drop-in ABI, caller-adoption or game-validation claim.

| Original entry | Complete Native and strict body | Read offset |
| --- | --- | --- |
| 00BC3010 | `D9 41 2C C3` — `FLD m32real [ECX+2Ch]; RET` | 2Ch |
| 00BC7AA0 | `D9 41 34 C3` — `FLD m32real [ECX+34h]; RET` | 34h |
| 00BC7AB0 | `D9 41 1C C3` — `FLD m32real [ECX+1Ch]; RET` | 1Ch |

Fresh whole typed single ranges, live bytes/listings and the installed I386 PE
agree on all three four-byte bodies: 12 bytes, six instructions. The saved
Ghidra project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Current names `TRIV_body_00bc3010`,
`BSP_CloudPuff_GetSortKey` and `BSP_CloudPuff_GetSize` remain unchanged.
Existing `undefined name(void)` prototypes do not establish Original ABI.

The new C++17 interfaces take a raw backing address in ECX and expose ST(0)
through a float return declaration. This is a descriptive callable Source
projection, not recovery of Original float-versus-double spelling, class,
receiver extent or calling-convention declaration. Native comparator callers
consume the result from ST(0), including a subsequent double store; that
consumer also does not settle the Original formal return spelling.

Each naked MSVC Win32 body performs the literal complete `FLD` and `RET`.
It adds no bit normalization, helper, environment reset, stack-empty check,
extra load or `FSTP`. The first instruction reads one four-byte real from the
wrapping effective address ECX plus the stated offset. The inherited x87
control/status/tags/TOP/stack, SNaN and denormal processing, pending exceptions,
masked/unmasked exceptions and stack-fault behavior remain the actual `FLD`
instruction behavior. Exceptional cases are not reduced to normal-value
semantics or assumed always to push. GPRs and EFLAGS are preserved, with plain
`RET` consuming the return slot on normal completion (ESP +4).

The first-instruction memory and FP fault schedule is retained statically.
Runtime exceptional and inaccessible-memory behavior was not probed. Absolute
fault EIP and saved FIP follow relocated code placement: byte identity does
not establish equality of absolute saved FP-environment pointers, placement,
full ABI or game behavior. Intel's [instruction reference](https://cdrdv2-public.intel.com/868137/325462-089-sdm-vol-1-2abcd-3abcd-4.pdf)
defines the retained operation; Microsoft's [fastcall documentation](https://learn.microsoft.com/en-us/cpp/cpp/fastcall?view=msvc-160)
documents the new declaration's register argument mapping.

The full existing Source/header search found caller inline approximations and
an abstract interface, not these complete standalone getters. In particular,
`water_weather_cc13.cpp` uses `w2_x87_reload_bits` for the BC3010 caller; that
helper quiets signaling-NaN bits in software and cannot stand for the complete
physical load/return, inherited x87 state or exception prefixes. Those prior
files and all earlier review artifacts remain unchanged.

Strict compilation used `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy` and passed
in 1.6912838 seconds. Independent full COFF inspection found exactly three
four-byte executable sections, six decoded instructions, zero relocations in
all sections, zero undefined externals and zero extra helpers. Strict object
SHA-256: `b8e46da80db1f8158097082d7241678b06691553df783571ff01b4ce4d6ced22`.
Source and evidence hashes, complete bodies and capture details are recorded in
`reports/goal16l_tagged_float_bc.json` and
`local/goal16l_tagged_float_bc/strict_proof.json`.

The review worktree has no build directory or CMake cache. A warm-worktree
successor must run `scripts/build.ps1` and the existing checks, prove the full
actual registered object and unique Core member in both linker directories,
and only then close exact three-address ledgers and final metadata. No new
tests, probe executable, Native execution or Ghidra writes were performed.
The frozen post-commit receipt records the inclusive preparation clock from
2026-10-16 07:43:27 UTC; subsequent build and independent review clocks must be
added by their owners. No speedup claim is made.
