# B107F0 normal resource tail (CC10)

The source covers only `[00B12C9A,00B13010)`: 886 bytes, 214 instructions,
38 direct caller calls. The last instruction is the five-byte CALL B4ECC0
at B1300B, inclusive last byte B1300F. `[B13010,B13029)` is the final
25-byte, six-instruction machine epilogue, recorded as **observed only**.
The original containing ABI is ECX=service, three borrowed DWORD argument
cells and eventual RET0C. This fragment reads no argument cells and executes
no native register/SEH/stack-return wrapper.

`continue_native_render_resource_init_00b12c9a_fragment` consumes the exact
post664 frontier once, checking every retained predecessor identity back to
the entry, the original argument-cell identity and the SAME continuation
context object. It admits EBX=service+4, EBP=FFFFFFFF, EDI/ESP14=raw664,
mask0 and EH-1. The previous header gains only a tail identity and two phases.
The new state retains two distinct genuine post20 construction blocks and
seven raw string headers. The wave block is prepared at the first stage;
the clear block is prepared only after the completed wave frame assignment.
No new companion, registry, count, nested-owner registration or ownership
credit is added. Preserve all blocks, contexts, literal views, predecessor
states and actual borrowed parameter storage through dependent work.

## Completed wave configuration at +654

`[B12C9A,B12F54)` is a complete 698-byte substage, 163 instructions, 31 calls.
BF681B at B12C9C requests exactly 20h. Its actual result becomes EDI/ESP14,
EH state95. On non-null allocation, construct D5E1E4 `Passtrough_Wave.mshd`,
OR the full DWORD temporary mask with immediate200h, set state96 and invoke
the actual B4E470 constructor at B12CE8 with count3 and null optional input.
Inherited EBX remains service+4 and EBP remains FFFFFFFF.

At B12CF1 test DWORD mask200, publish actual result/null to service+654 at
B12CF9, then disarm EH at B12CFF. Capture name data at B12D08 before clearing
bit200 at B12D0F. A non-null captured pointer causes current length+1 and
the actual current pool/manager/gate return; the raw header remains stale.
No texture assignment or extra default/parameter appears here.

Each parameter constructs its own raw name, preserves its distinct capture
order, loads current post material through B4CBA0, registers a borrowed record,
then captures name data before disarming and conditionally returning it.

| Literal | Source | Capture order after name | Words / vector count | EH | Registration |
| --- | --- | --- | --- | --- | --- |
| D5E430 cSceneColorSampleOffset | inherited EBX=service+4 | current654 D46 before inherited EBX push D4C; no fresh LEA | 2 / none | 98 | B12D67 -> B18B00 |
| D5E2F8 cDisplacementSampleOffset | service+194 | current654 DA9 before fresh LEA DAF | 2 / none | 99 | B12DD0 -> B18B00 |
| D5E260 cHParams | service+628 | fresh LEA E14 before current654 E23 | 4 / 1 | 100 | B12E3B -> B18AC0 |
| D5E254 cVParams | service+638 | current654 E7D before fresh LEA E85 | 4 / 1 | 101 | B12EA6 -> B18AC0 |
| D5E2E4 cAASampleOffsets | service+F4 | current654 EE8 before fresh LEA EF0 | 36 / 9 | 102 | B12F11 -> B18AC0 |

All addresses abbreviated in the order column have prefix B12. All records
use matrix0. B18AC0 is the already recovered complete wrapper: DWORD4*count,
matrix0 to B17E10, RET0C. Reuse original continuation SceneColor, retained
passthrough displacement/AA, and retained post65C H/V literal views. Only
the wave and clear effect literals are newly passed original views.

Capture current frame service+1D4 at B12F42 before current receiver+654 at
B12F49, then call existing full B4E2B0 at B12F4F with actual entry/current
profile/import identities. Its publication, incoming retain and old-owner
release ordering remain authoritative. B12F54 is a real completed substage
boundary: EBX=service4, EBP=-1, EDI/ESP14=raw654, mask0, EH-1.

## Separate clear effect and current focus work

`[B12F54,B13010)` is 188 bytes, 51 instructions and seven calls. BF681B at
B12F56 requests another 20h, distinct from wave. EDI/ESP14 become raw660 and
state103; EBX becomes400h at B12F71 **before** the allocation conditional.
The non-null path constructs D5E1D0 `cleartargets.mshd`, ORs DWORDmask with
EBX, sets state104 and invokes B4E470 at B12FA3 with count3/null.

At B12FAC capture the DWORD mask test result. Native POP EDI at B12FB0 comes
before publishing actual result/null to service+660 at B12FB1; EH disarm
follows at B12FB7. Native POP EBX at B12FBE comes before the cleanup branch
and data capture at B12FC1. These POP sites are recorded as observations;
the source has no original saved caller values. After the observations,
stored EDI/EBX bits are explicitly LAST PRE-RESTORE diagnostics, not current
native register values, and neither is consumed again by this fragment.

The clear raw name has original baseline length/data spills26C/270. After
the two native POPs its physical accesses use ESP264/268. This source uses
the retained raw header; it does not claim physical private-stack aliasing.
For a non-null captured data pointer, read current length+1 and call current
pool419CC0 at B12FDA followed by BD1510 at B12FE1. **Native never clears
mask400 here.** The successful construction path leaves mask400 and a stale
returned raw header at EH-1; the allocation-null path leaves mask0. There
are no extra clear-effect parameter, texture or frame assignments.

The focus sequence reads current byte service+1C4 at B12FE6. Only if nonzero,
capture current+20 at B12FEF and, if non-null, call existing
`mark_native_render_batch_dirty_00b50010` at B12FF6. Re-read current byte1C4
at B12FFB after that call (also when +20 was null). Only when this second
sample is nonzero, capture current+30 at B13004 and, if non-null, invoke
`invalidate_native_render_root_chain_00b4ecc0` at B1300B. Do not substitute
an earlier captured service owner, cached gate, host focus hook or new helper.

Both existing providers are in `platform_renderer_activation.cpp`. B50010
writes actual batch byte24C=1. B4ECC0 captures actual owner+3C before setting
byte250=1, follows actual root+0C, calls the existing fixed-null-root B6D890
projection, reloads current owner+3C after each return, and finally writes
byte251=1. Its raw root/node traversal uses the existing B72220 unlink
provider. This requires genuine current owner/root/intrusive-node storage
and surviving lifetimes; a non-null field alone does not establish those
preconditions. No missing native callee or new lifetime bridge was found.

## Terminal boundary and explicit limits

The tail and immediate predecessor mark normal work complete at B13010.
Older predecessor/entry phases remain partial `awaiting_later_continuation`;
the source makes no whole-initializer completion promise. ESI/service,
EBP=-1, the three original argument cells and original/aligned/half dimension
spills remain the inherited identities. ESP14 records raw660 at its original
baseline location. Mask is400 when the clear name was constructed, otherwise
zero; EH is-1. Saved EDI/EBX values are unavailable as explained above.

The excluded native epilogue loads saved SEH link from ESP26C at B13010,
POPs ESI/EBP at B13017/B13018, restores FS0 at B13019, adds270h to ESP at
B13020 and executes three-byte RET0C at B13026, ending exclusively B13029.
Reaching this observed RET does not prove native ABI/FH3/SEH equivalence,
composed runtime, original private-stack aliases, full initializer/service
lifetime or game behavior. Earlier static/build-only fragments, bloom extent
and distortion cleanup-preimage admission limitations remain unchanged.

Failures retain actual publication, names/mask, register observations,
latest captures and both blocks' genuine provider acquisitions. The consumed
chain fails without caller repair, rollback, free or retry. Existing external
quiescence requirements for resetting retained blocks remain authoritative.

## Evidence and validation

Read-only wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 911 live bytes through the observed epilogue
match the installed PE, SHA256
`64eeb3de2847faa961e6f7383244f869e9a89dcb291d6e079ca3b46fb4fb0aa7`.
The implemented normal 886 bytes have SHA256
`608b766b63a4fcd68841632c19c2ad51b789f78f9898fb31d563c532296eee2f`.
Both new literal regions and the full 8/50-byte focus helper bodies also match
the PE. All 40 report rows passed verification: 38 caller rows plus two existing
wrapper/helper support rows. `scripts/build.ps1` passed MSVC Win32 Release
`/MD /W4 /WX` and all three existing CTests (`reconstructed_math`,
`native_math_differential`, `tool_tests`). Ignored evidence
is `local/render_init_post654_tail_*`, `local/render_init_next_post5_*` and
`local/render_init_tail_*`. All prior evidence remains unchanged. These are
static/build checks, with no new test/probe or composed-runtime claim.
