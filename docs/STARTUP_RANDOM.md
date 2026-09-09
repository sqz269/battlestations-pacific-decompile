# Startup and per-thread random subsystem

## Confirmed startup path

`entry` → CRT startup → `BSP_WinMain` (`008f81f0`) → random-thread initialization →
current-thread registration → application construction (`00737970`) → initialization
(`0073d410`) → platform virtual loop dispatch (`00bea800`) → shutdown (`00737f30`) →
application destruction (`007379a0`) → thread unregister and random-thread shutdown.

WinMain's CRT call site supplies four stack arguments. Its assembly returns with `RET 10h`.
The Ghidra prototype is now an int-returning `__stdcall` function with instance, previous
instance, command-line and show-command parameters. Their values are unused in the visible body.
The application object has a vtable, an initialized flag at +4, a dynamic array at +8/+0c/+10,
a pointer at +14, and flags at +18/+19/+1a. It is not yet ported.

`00bea800` loads singleton `0109cf04` into ECX and tail-dispatches vtable offset +24h.
The actual implementation of that slot is the next startup dependency to resolve.
Initialization already exposes resource mounts, window creation, FMOD/Bink and Lua datatables;
none of those systems are reconstructed by the current probe.

## Random state layout and algorithm

| Offset | Field |
| --- | --- |
| `0` | uint32 next-word index |
| `4` | 624 uint32 state words |
| `09c4` | guard-allowed byte |
| `09c5` | guard-enabled byte |
| `09c6` | two padding bytes; total stride `09c8` |

`00bf0cf0` sets the first word to `seed | 1`, then multiplies each preceding word by
69069 modulo 2^32; index finishes at 624. Do not substitute a standard-library random
engine without proving its initialization and stream match. `00bf0d20` performs the
in-place 624-word twist with offset 397, splitting the loops at 227 and 623, and resets index.
`00ba2c20` refills when needed and tempers the next word using the observed bit operations.

The native guard deliberately faults through a null write if enabled but not allowed. The
C++ port uses `abort()` for that invalid state; the fault type is not identical. Guard semantics
outside this condition remain unnamed. Float-range and Gaussian helpers have been exported
but are not ported because their x87 rounding and rejection paths need separate work.

`00bd2e00` constructs a default state with guard bytes 1,0 and seed `1105h` (4357).
Static initialization bytes at `00cd8a30` pass the constructor, destructor `00b94b20`, count 20,
stride `09c8h`, and destination `01090af0` to the CRT vector constructor helper. The corresponding
vector destructor is `00ce1020`. Startup bytes at `00cd8a10` separately seed fallback `00e14748`
with `1105h`; fallback guard bytes at `00e1510c` are 1,0 on disk.

## Registry and lock behavior

Globals: lock pointer `01090ac0`, ten thread IDs from `01090ac4`, high-water count `01090aec`,
twenty random states from `01090af0`. Registered thread slot i and stream s select state
`2*i+s`; unregistered threads share the fallback. The port exposes stream selectors 0 and 1.

Registration and unregistration use a 28-byte tracked lock: 24-byte x86 CRITICAL_SECTION plus
a signed depth counter at +18h. Registration uses the first hole and does not deduplicate IDs.
A full table silently does nothing. Unregister clears the first matching ID and trims trailing
holes only when removing the last active slot. Reused slots retain their random state.

Lookup is unlocked in the original. The port preserves that algorithm, so callers must coordinate
registration against simultaneous lookup; general concurrent safety is not claimed. Shutdown
must happen on the owning/quiescent thread after worker use ends. The lock destructor drains its
recorded recursive entries, deletes the OS critical section, frees storage, and clears the
owner's pointer. The pointer-clear bytes at `0041ccb6` were hidden by the bad noreturn flag;
the port includes that store after checking the raw bytes and restoring disassembly.

The C++ wrapper owns the former globals. Its constructor models one-time CRT construction plus
registry startup; its destructor models registry shutdown. It does not expose the original's unsafe
repeated-initialize behavior, fixed addresses, or allocator. Layout assertions protect RandomState
and the tracked lock, but the functions are not ABI-compatible drop-in replacements.

## Ghidra changes

`config/ghidra_names.json` records 24 descriptive names and evidence. `tools/ghidra_annotate.py`
previews them; `--apply` renames functions, preserves existing comments, appends an evidence block,
records prior annotations under ignored `local/`, and saves the program. Names describe recovered
behavior and are not claimed to be original developer symbols.

The CRT `_free` implementation `00bf9dc8` incorrectly had `noreturn`; it demonstrably returns
for null input and normal heap-free paths. Cleared that flag, also checked its thunks `00bf65ac`
and `00bf6989`, and added a correction comment. Some caller bodies remain truncated where
analysis had previously stopped; recovering those fallthrough bodies is still pending.
WinMain's four-argument stdcall prototype was corrected separately and the project saved.

## Validation and remaining work

- Eight complete native function byte ranges match the current disk executable.
- One new differential case in the existing native harness compares seed state, 1,500 integer
  outputs (crossing refill boundaries), and the entire final RandomState against original code.
  It uses even seed `1104h` to exercise the seed's forced low bit; zero mismatches.
- Original next-word code has one relative CALL, relocated only to the copied verified refill.
  It executes in the test process, with neither game globals nor game startup loaded.
- Existing 455 math comparisons still pass; CTest 2/2 and existing exporter tests 4/4 pass.
- The startup probe verifies distinct registered streams and fallback restoration, prints the
  first five default-seed words, and exits through lock cleanup. It is a single-thread smoke run;
  table exhaustion, duplicate registration, worker interleavings and invalid guards are untested.

The ledger maps 18 original routines to compiled C++, with separate evidence levels. This is
partial source reconstruction, not a game executable or gameplay-equivalence result.
