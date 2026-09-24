# B107F0 service+65C post configuration

This packet reconstructs only `[00B124A1,00B1297A)`, 1241 bytes of the existing
`BSP_RenderResources_InitializeMembers_PartialEntry`. The last included
instruction is the five-byte B12975 call to B4E2B0, inclusive end B12979.
B1297A begins the excluded next20h post allocation. The containing ECX service,
three borrowed DWORD argument cells and eventual B13026 RET0C remain unchanged.
This fragment neither reads an argument cell nor executes a native return.

All 1241 live Ghidra bytes match the configured original PE, SHA-256
`a5cfc50e4ce95f106afc02cb79f2e2283fcfd699a5b931a1b99b0bc4fa724571`.
The complete listing has 293 instructions and 58 direct calls. The new literal
views occupy verified bytes D5E254..D5E28B. Full read-only evidence is preserved
under `local/render_init_next_post3_*` and `local/render_init_post65c_*`.
The wrappers verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. No Ghidra mutation or new complete-function claim.

## Construction, publication and texture

B124A3 allocates actual20h storage. EDI and retained ESP14 receive the raw
result, then state73 is armed. On nonnull allocation B124CC constructs the
actual raw name header from D5E26C `Passtrough_OldFilm_Wave.mshd`; mask80 is
set only after that call returns. State74 precedes B124EF's existing complete
B4E470(count3,null). This stage retains its own genuine
`NativePostEffect20ConstructionBlock`, including the real canonical
completed-owner companion. It adds no generic duplicate.

At B124F8 EDI becomes FFFFFFFF **before** mask80 is tested and before B12500
publishes the actual constructor result/null to service+65C. B12506 disarms
**after** publication. Native code releases no previous+65C value. Conditional
normal cleanup captures name data before clearing mask80, then captures
length+1 and uses the actual current raw-pool singleton/manager/gate. Headers
remain stale after return; diagnostic flags do not authorize replay.

B1253C captures current service+67C before B12542 reads current+65C. B1254B
borrows the captured post's material+14 using existing B4CBA0. B12552 invokes
the real unchecked B189F0 for slot3 and that captured texture, sharing the
original actual-owner domain. No construction-time material, post or texture
snapshot substitutes for those current reads.

## Ten exact borrowed records

Every row owns a separate raw name header and has its own current source/
receiver order. The source uses existing B18B00/B18B20 and the exact B18AC0
expansion into real B17E10; no fabricated material record or copied parameter
value is introduced. All records have matrix0.

| Literal/name | Borrowed source | DWORDs | State | Capture order |
| --- | --- | --- | --- | --- |
| D5E430 cSceneColorSampleOffset | service+04 | 2 | 76 | source address before current+65C |
| D5E2F8 cDisplacementSampleOffset | fresh service+194 LEA | 2 | 77 | current+65C before source address |
| D5E2E4 cAASampleOffsets | fresh service+F4 LEA | 36 | 78 | current+65C before source address |
| D5E260 cHParams | service+628..637 | 4 | 79 | source address before current+65C |
| D5E254 cVParams | service+638..647 | 4 | 80 | current+65C before source address |
| D5E2BC cLetterboxRatio | actual inherited EBX | 1 | 81 | current+65C before EBX use |
| D5E2AC cNoiseOffset | current service+34, then +08 | 2 | 82 | capture+34, ADD08, current+65C |
| D5E2A0 cNoiseScale | actual inherited EBP | 2 | 83 | current+65C before EBP use |
| D5E294 cFlicker | current service+34, then +10 | 1 | 84 | capture+34, ADD10, current+65C |
| D5E28C cShake | current service+34, then +14 | 1 | 85 | capture+34, ADD14, current+65C |

AA uses B18AC0 count9; horizontal and vertical use count1. The established
complete32-byte wrapper forwards wrapping DWORD `4*count`, matrix0, to B17E10.
The supporting B18AD8 call is evidence for this existing expansion, not a new
wrapper implementation. The original raw name returns retain captured-data,
disarm, current-length+1, pool getter and pool return ordering.

The displacement and AA addresses are fresh LEAs in this stage; they do not
reuse the inherited registers. Letterbox actually uses inherited EBX=service228
at B1277E and NoiseScale uses inherited EBP=service648 at B1284A. Entry checks
those meanings, but implementation uses the retained bits rather than a
recomputation. Neither register changes anywhere in this packet.

The scene-color source order, opposite H/V orders and Shake's offset-before-
receiver order are intentional. They differ from other post stages. Each
noise/flicker/shake row separately reads current service+34; callbacks can
replace that field between records. Every captured actual CCh backing owner
must survive its parameter borrow. The fragment creates no retain, replacement
storage or new outer lifetime guarantee to repair an invalid backing lifetime.

The effect, horizontal and vertical names are three original new views. Other
literal views remain borrowed from the same original continuation and retained
passthrough/+658 name contexts. Their objects and storage remain alive through
dependent stages. No new global or string-pool domain is invented.

## Frame, frontier and lifetime limits

B12968 captures current service+1D4 **before** B1296E reads current+65C.
B12975 composes the existing complete B4E2B0 with both captures, preserving its
incoming publication/retain before captured outgoing release, current imports
and actual D5E600 frame lifetime. It does not reuse an earlier frame receipt.

The state retains its independent post20 block, eleven raw name headers,
source/material/current-owner captures, allocation/result/publication receipts,
and the exact predecessor/context/entry/argument identities. The predecessor
header adds only one successor identity and two phases. No additional registry,
companion, count, primitive or default is introduced. All retained blocks and
provider domains require their existing external-quiescence contracts before
reset/destruction, including failed or partly prepared acquisitions.

At B1297A: ESI=same service, EBX=the same inherited service228, EBP=the same
inherited service648, EDI=FFFFFFFF, ESP14=raw+65C, mask0 and EH state-1.
Other original/aligned/half dimension spills are unchanged and the same three
argument cells remain borrowed and unread. Failure marks the entire chain
failed and preserves publications, name/mask state, actual source captures and
provider diagnostics. No caller rollback/free/retry or native FH3 cleanup.

Strict `scripts/build.ps1` passed MSVC Win32 Release with `/MD`, and all three
existing CTests passed. The report verifier checked 58 caller rows plus the
supporting B18AD8 wrapper call with zero failures. No new test or probe was
added. Static source/live-byte/build evidence does not establish
composed runtime, native ABI/FH3/SEH/private-stack alias, full initialization/
teardown or gameplay. Earlier packets retain their published boundaries.
