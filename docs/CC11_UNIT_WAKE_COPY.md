# Raw unit wake copy, 00815680

Packet `cc11_health_unit_wake_copy` reconstructs the complete stored body
`[00815680,00815848)`: 456 bytes and 135 instructions. The descriptive name
is a hypothesis. Source retains every instruction, including `MOV EDI,EDI`;
its complete Win32 COFF bytes equal the installed PE and live Ghidra bytes.

Body SHA-256:
`a262b08eda2774d3f406cd56fa1f9a44f6447ae96d10e3d7efec9d3bf6859ecd`.

## Actual inputs and writes

Native ECX is the destination ring, the source ring is the one stack argument,
and the entry executes `RET 4`. The C++ declaration's unused EDX argument
preserves this native stack spelling. Both pointers borrow actual storage.
No object view, constructor, critical-section service or support archive is
introduced. The whole body has zero calls, globals and relocations.

The integer head at source `+3C8` is stored to destination `+3C8` first. Four
iterations then perform sixty sequential float32 `FLD`/`FSTP` pairs each,
reading and writing all 240 words at `+8..+3C7` in ascending order. All six
words of each native `18h` sample participate. The older semantic
`UnitPoseHistorySlot` with five float members is not a raw layout provider.

Destination `+0..7` and `+3CC..3DB` remain untouched. These include the native
vptr, owned critical-section handle, flag and residual. The operation neither
retains nor destroys anything. The actual containing unit constructs this
`3DCh` inline ring at `+BD0` through `00815600`; that constructor and its
`00818100` destructor remain outside this packet.

There is no overlap guard or snapshot. Self-copy still executes floating-point
loads/stores, including SNaN quieting. Partial overlaps retain the initial head
store and each subsequent read-after-write effect. These are floating-point
operations: replacing them with `memcpy`, integer stores or a staged ring
changes observable behavior. Source preserves the ambient x87 control word,
stack and exception effects; one free x87 stack slot is required.

ESI is saved/restored. EBX, EDI and EBP remain unchanged. Incidental exits are
EAX = destination `+3CC`, EDX = source `+3D4`, and ECX = zero. No semantic
return value is established. Final arithmetic flags come from subtracting one
from the final loop counter; the retained original instructions also preserve
these flags.

## Caller and remaining integration

The live caller witness is `00815EAA` in `00815E20`, supplying destination
`new_unit+BD0` and source `old_unit+BD0`. Its one verified call row is structural
caller evidence; the reconstructed leaf itself contains zero calls.

This body does not finish the enclosing wake handoff. Whole `00815E20`, its
raw `00414DB0` pose dependency or an honest existing view binding, usable unit
and ancestor lifetimes, successor/type/speed bindings, detach, synchronization
and world/game integration remain external. No unsupported owner receives a
default pose or ring. Unmasked exceptions, memory faults and concurrent mutation
are outside the verified ordinary component domain.

## Focused verification

One new isolated fixture family passed on its first execution: **208 checks,
four cases, eight Original/Source calls, zero failures**. The count includes
fixture preconditions and artifact I/O as well as native comparisons. No older
fixture was run and no tracked tests were added.

The Original is all 456 unchanged PE bytes, with no patch or relocation. The
freshly compiled Source has the same 456 bytes before linking, in the linked
PE, and before and after execution. The Source translation unit and fixture
are freshly compiled from four pinned project/fixture inputs, with 175 actual
host headers and six searched system libraries recorded. No BSP archive is
consumed. Compilation uses MSVC Win32, `/W4 /WX /O2 /fp:strict`; the executable
contains an `asInvoker` manifest through `/MANIFEST:EMBED`.

Each pair uses the same actual pointers into one aligned 3072-byte guarded
allocation. Caller input bytes are restored in that allocation between runs.
The body receives the actual borrowed byte spans directly. Overlap cases test
raw access order; they do not claim that overlapping views are separately
constructed owning objects or fabricate critical-section handles.

| Case | CW | Sticky status before / after | Independent observation |
| --- | --- | --- | --- |
| Disjoint | `037F` | `20 / 23` | Finite, signed zero, subnormal, infinity and NaN results; previous precision flag retained |
| Self | `007F` | `00 / 03` | SNaN quiets even when both actual pointers are equal |
| Destination = source + 4 | `027F` | `20 / 21` | First SNaN quiets and propagates through all 240 output words |
| Destination = source - 4 | `0A7F` | `00 / 03` | Initial integer head store becomes the final sample's subnormal value |

All 3072 bytes compare exactly, and every byte outside destination samples/head
remains unchanged. Checks capture EAX/ECX/EDX, all four nonvolatile registers,
RET4 stack balance and EFLAGS. Each target receives the same full x87 seed with
two live 80-bit values, TOP 6, tag `0FFF`, and controlled sticky flags. Complete
CW/SW/tag/opcode/data-address fields and all 80 saved register payload bytes
match. The live values are preserved. The only difference in the 148-byte result
records is the code-address FPIP field: both end at entry `+1BB`, the final
`FSTP`. Fixture adapters save/restore their caller's ambient state; production
Source never changes the FP environment.

## Sealed evidence

The worker's ignored directory is
`local/cc11_unit_wake_copy_20261007_a`. It contains `build.cmd`, the frozen
`inputs_01`, `probe.exe`, `original_image.bin`, `probe.log`, `runtime`, complete
COFF and live-byte receipts, `recipe.json` and `artifact_manifest.json`.
The tracked report pins the actual source and all receipt hashes.

Independent replay must use `probe.exe original_image.bin NEW_OUTPUT_DIRECTORY`
with a newly created output directory and fresh log. Do not overwrite this
family's sealed runtime/log files or rerun earlier families. The synchronized
successor files, its 125 artifacts and all 27 audit files retain their starting
hashes. Primary integration metadata, CMake registration, Ghidra writes, full
build and existing CTests are the integrator's separate work.

## Primary integration

Whole456-byte/135-instruction raw copy Source equals PE/live/COFF/linked/executed Original, zero calls/globals/relocations. Actual borrowed pointers: integer head3C8 first,240 ascending x87 pairs8..3C7, untouched vptr/owned handle/flag/residual; no staged/memcpy/typed-ring/default/provider. One208-check/four-case/eight-call family preserves whole3072 guarded backing, RET4/registers/EFLAGS/full x87 state and80-byte payload; disjoint/self/forward4/backward4 alias behavior exact, only normalized FPIP entry+1BB differs. One additional free x87 slot required. Caller row815EAA is structural only; full ring/unit construction, wake/group/type/speed/world/class/game remain external.

Main Source `b92b4710f` passed the full MSVC Win32 build and all three existing CTests. The independent primary receipt uses 2 fresh TUs and is `local/cc11_wake_copy_current_primary/inputs_after.json`. Its source inputs remain unchanged through integration. Saved annotations/exports/snapshot evidence follows in the report.
