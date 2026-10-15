# Actual opaque vector range erase and clear

Addresses: `004687E0`, `00468D70`. Packet `goal_oct15_dword_vector_clear_range`.

| Native library name retained | Coverage | Original ABI | Ordinary Source |
| --- | --- | --- | --- |
| `STL_inst_004687e0` | Complete90B/37ops, `004687E0..00468839` | ECX control,unused EDX; five stack DWORDs; `RET14h`; EAX output | `erase_native_scene_dword_vector_range_004687e0` |
| `STL_inst_00468d70` | Complete57B/26ops, `00468D70..00468DA8` | ECX control,unused EDX; no native stack inputs; plain RET; unchanged child EAX | `clear_native_scene_dword_vector_00468d70` |

The whole owned coverage is147 bytes/63 instructions, without alignment padding.
The descriptive erase/clear names are behavior hypotheses, not recovered symbols.
No nominal element, pointer ownership, destructor or container admission is inferred
from the library tags. Current Source uses opaque16-byte control storage and the
observed4-byte span arithmetic. Both addresses were unleased and had no indexed or
address-matching reconstructed Source before this packet.

## Complete provider equality and actual dependency

All90 bytes of the native erase match `004685A0` except its two relative
displacements. The only differing byte offsets are20,21,64,65; the complete operands
are offsets20..23 and64..67. Both versions have native targets `00BF6713` and
`00BF67A7`. The full listings, installed PE bytes, accepted ordinary Source
`native_scene_dword_erase_range.cpp` and its complete raw move provider
`native_scene_dword_move.cpp` were reviewed. The new alias delegates the actual
`erase_native_scene_dword_range_004685a0` with the same live parameter backing.
This is Source reuse; neither `004685A0` nor `00467D90` is a native transfer edge.
Reference bytes and reused provider bodies are not extra owned coverage.

Erase's five actual argument cells are output,first owner,first node,last owner,
last node at offsets0,4,8,C,10. Capture first owner before the diagnostic. Zero
owner skips the last-owner read; otherwise compare its current value. Do not
require either owner to equal control. After a returning diagnostic, read current
first node then current last node. Equal nodes skip control+8 entirely.

Unequal nodes read current control+8, subtract last modulo32 and compute exact
SAR2 bits. Capture first+wrapped byte count before any SDK call. Only signed
positive shifted count calls actual current SDK `memmove_s`, with equal count
and destination capacity; ignore status. Publish the captured result to control+8
even for zero/negative counts and a returning SDK diagnostic. Then read the current
output cell, store captured first node at output+4 before captured pre-diagnostic
owner at output+0, and return that output pointer. These accesses preserve valid
parameter/control/payload/output aliases, including copied argument cells and an
end publication that changes the output pointer cell.

## Complete clear wrapper

Capture control+8 end before reading control+4 begin. Compare unsigned begin>end;
the actual diagnostic may return. Then capture current control+4 begin, read
current control+8 and compare unsigned again. Keep both the first captured end
and second captured begin across the second diagnostic.

Publish the native outgoing argument cells in push order: end at+10,control at+C,
begin at+8,control at+4,private-output pointer at+0. Compose the complete erase
provider with current published cells. The Source API supplies distinct actual
20-byte outgoing and8-byte private-output backing; binding metadata stays stable
and disjoint from supplied storage. No extra native input argument cells exist.
Private output is not initialized or copied after the child. The full tail only
pops EDI/ESI/EBX, adds8 to ESP and returns, so Source returns the child's actual
pointer bits. No semantic ownership, pointee or lifetime meaning is assigned to
that raw result; it need not equal the originally supplied private-output pointer
when actual aliases change the child's current output cell. No caller-based live
return-type repair is made for this unreferenced wrapper.

## Actual native transfers

| Actual listing | Containing function | Contract |
| --- | --- | --- |
| `004687F3 CALL 00BF6713` | `004687E0` | actual returning invalid-parameter provider; zero/mismatched owner |
| `0046881F CALL 00BF67A7` | `004687E0` | four stack words, `ADD ESP,10h`; signed-positive count only |
| `00468D80 CALL 00BF6713` | `00468D70` | first unsigned check with captured old end |
| `00468D8D CALL 00BF6713` | `00468D70` | second unsigned check with captured begin/current end |
| `00468D9D CALL 004687E0` | `00468D70` | five stack words in described push order; child `RET14h` |

Fresh live xrefs find only the child call in this table for erase and no references
to the clear entry. This is unknown production reachability, not evidence that it
is dead or unused through indirect dispatch. Typed metadata captures both complete
bodies and all five real native CALL rows; no cleanup tail JMP is present.

## Validation and limits

The report carries all147 owned original bytes, live/installed-PE equality,
full90-byte canonical comparison, source hashes and typed transfers. Guarded live
CLI batches verify `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86/imagebase00400000. Locked annotation records
prior values, keeps correct library names/comments, saves, refreshes exports and
reads back the evidence. No provider or unowned caller is annotated.

Final validation: the primary combined normal `scripts/build.ps1` MSVC Release
Win32 build passed at `55d382440cd0360de19adff1c5a86cd6e13c8f34`; all three
existing checks (`reconstructed_math`, `native_math_differential`, `tool_tests`)
passed in7.77 seconds. Native math seeds do not execute these new bodies. The
primary explicitly superseded a duplicate worker build; no local worker build
was started or claimed. The exact primary log and archive hashes, single
project/archive object, x86 object/publics and Source/header hashes were
independently checked. All six owned/canonical Source/header hashes remain
unchanged in both trees. The report cites the primary receipt and exact log.
Primary independent complete63-op assembly/Source/header review passed; all
five typed native CALL rows and five documentation claims passed without
failures or unchecked claims. Primary registered this Source once during its
queue turn and released the lease; this worker left CMake untouched. No new
tests were added.

The selected current SDK CRT policy is real, including returning
or throwing diagnostics; original static CRT errno/encoded handler/debugger/
Watson behavior is not claimed. Reached storage must remain valid for that actual
provider. Native argument-home/implicit private-frame aliases and asynchronous
mutation are outside these ordinary Source interfaces. No original COFF,
register/SEH/fault-address/binary ABI, native-entry differential, application
adoption or game validation follows from static evidence or the build.
