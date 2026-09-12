# Native dynamics world storage construction

Address: `00C41AD0..00C420DB`. Packet `orch6_dyn_world_runtime_f`, 2026-09-12.
Implementation: `src/dyn_world_runtime.cpp`; evidence: `reports/dyn_world_runtime.json`.
`Dyn_World_Construct` remains a descriptive hypothesis, not a recovered symbol.

The complete normal fresh-construction sequence now builds the `48Ch` world,
its pools, collision scene, solver task arrays and shared motion. It consumes the
existing initialized engine, dispatch objects, allocator and callable task tables.
Their runtime ownership and execution remain separate dependencies.

The original takes two stack arguments, world then descriptor, returns the world
in EAX, and executes `RET 8` at `00C420D9`. This corrects the earlier `__cdecl`
annotation to `__stdcall`. The reconstruction exposes a new C++ interface and
does not claim compatibility with the original binary or its SEH/OOM behavior.

## Descriptor and owned storage

The fifteen descriptor copies preserve the instruction distinction: eight use
actual Win32 x87 `FLD`/`FSTP` spills, seven copy raw dwords. Gravity at descriptor
`+04/+08/+0C` uses raw words, preserving signaling-NaN bits that an x87 spill would
quiet. The report lists every source and destination offset. World `+30` and
`+458`, unused task payload fields and pool record padding remain untouched.

| World offset | Construction |
| --- | --- |
| `4C`, `170` | Existing static/dynamic body pool constructors `00409170` |
| `294` | Existing motion pool constructor `00409450` |
| `438/43C/440/44C/450/454` | Zero words; `448` is the world self link |
| `444` | Newly allocated `E8h` scene, fully constructed by `00C38070` |
| `45C/460/464` | LCP task data/count/capacity, `18h` per record |
| `468/46C/470` | LCP task pointer vector |
| `474/478/47C` | LCP2 task data/count/capacity, `18h` per record |
| `480/484/488` | LCP2 task pointer vector |

Scene construction precedes all four task vectors. The native function reads
global engine `0109E9FC`, then engine `+10`, then manager `+4` for the first task
count, and repeats the engine/count read before constructing the second pair.
Each task receives its world at `+8`, and the pointer vectors point at their
corresponding records. The shared motion allocation and `C0h` zeroing happen
last, at `00C41FA4..00C420C5`; the existing partial body-pool setup now reuses that
same helper without claiming to be the full constructor.

Each task table at `D7A080`, `D7A088` and `D7A090` has one callable slot. The
adjacent words at `D7A084/08C/094` are RTTI complete-object-locator pointers for
the following classes. No task method is synthesized by this constructor.

## Verification and limits

Win32 Release and the two existing CTests passed. The original world function's
1,548 bytes matched live Ghidra and the installed executable before relocation.
The ignored native fixture executes the original full constructor, scene, pools
and shared-motion tail, and compares them with the reconstructed implementation.

Four paired cases use worker counts `0/1/4/64` and x87 control words
`027F/0C7F/0A7F/077F`. They compare all world bytes and 118 owned buffers after
pointer normalization, plus x87 exception flags and stack TOP. Descriptor values
include finite numbers, signed zero, subnormals, infinities and NaNs. These are
four paired cases, not all count/FPU/value combinations. Both records hash to
`95a22368cff0536cacc8f568611d43e4022320d3f2110c1d4d171abbffa008ce`.

The engine and thread manager in this fixture are explicitly borrowed input
projections. Original engine startup and task execution do not run. Only the
OS-dependent critical-section DebugInfo pointer is excluded; all other storage
bytes remain compared. Cleanup uses fixture disposal, not native destruction.
Fresh construction does not exercise old-task copying or late pool growth.
A second count smaller than the first is rejected by the new C++ boundary;
the native link loop would otherwise overrun. Changing counts were not tested.

An independent read-only review confirmed the constructor sequence and caught
an overly broad fixture relocation loop that included adjacent RTTI pointers.
The loop now relocates only the three callable slots; the corrected fixture
passed again with identical records. The report preserves exact source and
artifact hashes. No tracked tests were added.

## Follow-up packets

- Connect actual engine/global dispatch owners, including `00C55F50`, `00C55EA0`
  and task-manager construction `00C37740`, before using this world in the game.
- Recover task execution and native world/scene destruction with their real
  scheduler and allocator lifetimes. Constructor byte parity does not establish
  collision stepping or gameplay parity.
