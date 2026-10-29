# Dyn SAP endpoint append and axis update

This packet reconstructs complete bodies `00C40900..00C40A2A` and
`00C4C700..00C4CBA4`. Names are descriptive hypotheses. The new C++ interfaces
operate on actual raw Win32 objects and the caller's genuine `DynLiveCrt`.
They supply no class, object, global, default heap, vtable or returning stub.

The live batches used the verifying `bsp.py ghidra` client against project
`bsp`, `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 LE
32-bit, image base `00400000`. Both bodies were freshly exported and their
complete assembly reviewed. Live byte spans are 299 and 1189 bytes; the
hashes and exact call sites are recorded in `reports/cc16_dyn_sap_update.json`.

## Original and public contracts

`00C40900` receives storage in ECX, endpoint and axis on the stack, and uses
`RET 8`. It preserves EBX/EBP/ESI/EDI. Each axis vector is at
storage+`4+axis*0Ch`, with data/size/capacity dwords and eight-byte entries.
It captures the old size before either allocation, appends minimum and
maximum tags with that same old size as their initial key, and stores the
truncated old size / old size+1 at endpoint+`20h+axis*2` /
`26h+axis*2`. Observed EAX is the full 32-bit old size+1. The public function
returns those observed bits without asserting that native callers use them.

The two capacity checks are equality-only. Capacity is published as
`capacity*2+2` before actual `operator_new_00bf55be`, and its byte count wraps
at 32 bits. Copy loops reread size and old data; a zero computed destination
suppresses only that entry's writes. Nonzero old data is passed to actual
`free_00bf6989` before replacement publication. No allocation-failure guard,
rollback or exception translation is added.

`00C4C700` receives axis in ESI and storage/endpoint on the stack, preserves
EBX/EBP/EDI and unchanged ESI, and uses `RET 8`. Its public C++ interface
exposes those inputs explicitly and claims no native EAX result contract.
The source requires MSVC x86 because its four inline-assembly instructions
preserve the exact two x87 float copies.

## Bound copies and ordering

At `00C4C70B`, FLD reads endpoint+`4+axis*4` before the minimum index and data
word reads; FSTP at `00C4C724` writes the selected key slot. The maximum
index is loaded before FLD at `00C4C741`; data is then reloaded before FSTP
at `00C4C74B`. These are actual x87 conversions through extended precision,
including ambient control word, exception/status and NaN quieting behavior.
They are not replaced by integer copies or SSE float assignments. Nothing
sets a new control word or clears status. The resulting stored bits become
`((0u-(bits>>31))|80000000h)^bits`, preserving signed zero and NaN bit order
after that native x87 conversion.

Minimum reordering completes before a fresh maximum-index/entry lookup.
Forward size comparisons are signed JGE/JL; adjacent-key comparisons are
unsigned. Equal keys do not move. Neighbor entries copy key then tag, and
the copied tag is reread through the current data pointer. Its low bit
selects a wrapping 16-bit minimum or maximum index update. A moved entry
publishes its own index, then tag, reloads the data pointer, and writes key.

An opposite endpoint crossing checks byte flags at +1Ch: both nonzero
suppresses the pair operation. Other-axis selectors use the native masked
shift formulas `(1u<<(axis&31))&3` and `(1u<<first_selector)&3`. Four ordered
unsigned ushort comparisons test overlap, with the native short circuits.
Minimum moving forward / maximum moving backward calls the real recovered
`cc16_dyn_pair_retire::retire_endpoint_pair_00c4c5d0`; the opposite directions
call `cc16_dyn_storage::add_endpoint_pair_00c40a30`. The same actual storage,
moving endpoint, other endpoint and CRT are passed intact. Pair absence,
invalid indices and invalid objects remain unguarded native preconditions.

## Validation and integration boundary

MSVC Win32 object compilation passed with C++17, `/permissive- /EHsc
/fp:strict /O2 /W4 /WX`. Generated assembly confirms both FLD/FSTP pairs in
their required order, with no intervening helper calls and no control-word
write. All eight original direct call rows passed `verify_report_calls.py`.
No tests, probe executables, native/game launches or Ghidra mutations were
added. Vendor CRT bodies are dependencies, outside reconstruction credit.

The packet does not edit shared CMake, ledgers, annotations or activation.
The integrator must register the source, run the common Win32 build and
existing relevant checks, annotate/save/refresh Ghidra evidence, and wire
genuine callers/CRT custody before activation. Fixtures, original register
ABI compatibility, native differential behavior and game validation remain
unproven. Defined incoming bodies include `00C4CBF0` (both routines),
`00C54DA0` (append), and `00C4CBB0`/`00C5A850` (update); this packet claims
no reconstruction or wiring of those parents.

## Primary integration

The primary reviewed the complete source/assembly bodies and registered the
module in `bsp_core`. The combined Win32 Release build over accepted
`c458c4d24` with source candidates `57f644d5`/`a5b6836e` and the dormant
accuracy correction passed both existing suites (10.69 seconds). Native
call rows passed; final-object inspection confirms the actual x87 copy epochs.
Receipt: `local/cc16_sap_accuracy_off_build.log`. Metadata-only placeholders
now name compiled/unwired bodies. Locked annotations were saved and exports
refreshed; previous values are in
`local/ghidra-annotations-20261029T052526Z.json`. Genuine game/root, caller
and CRT custody, original register/fault ABI and runtime gates remain open.
