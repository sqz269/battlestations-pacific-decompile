# B107F0 service+658 post configuration

This packet owns only `[00B120B9,00B124A1)`, 1000 bytes of the existing
`BSP_RenderResources_InitializeMembers_PartialEntry`. The final included
instruction is the five-byte B1249C call to B4E2B0; inclusive end B124A0.
B124A1 begins the excluded next20h post allocation. The containing ECX service,
three borrowed argument cells and eventual B13026 RET0C remain unchanged.
This fragment reads no argument cell and executes no native return.

All 1000 live Ghidra bytes match the configured original PE, SHA-256
`970db6215f0e4b79197790b263c44efd4bc72fdbcf18d472fe8a1bac9dd5b5de`.
The full listing contains 244 instructions and 48 direct calls. The six new
literal views occupy the verified D5E28C..D5E2E3 region; its live/PE SHA-256 is
`ed1a5b792b55a1e95b9a084ff44bf31d9382a235663b49ab63f9d024c243b6ea`.
Evidence is retained under `local/render_init_next_post2_*` and
`local/render_init_post658_*`. Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified through the read-only wrappers.
No Ghidra mutation or full B107F0 implementation claim is made.

## Actual construction and current texture

B120BB allocates the actual20h owner, then EDI/ESP14 become the raw result and
state62 is armed. On nonnull allocation B120E1 constructs the raw name header
from D5E2CC `Passtrough_OldFilm.mshd`, with the original spelling. Mask40 is set
only after name construction returns. State63 precedes B120FE's existing full
B4E470(count3,null). Its independent `NativePostEffect20ConstructionBlock`
already supplies the real canonical owner companion; no generic duplicate.

At B12107, EDI becomes FFFFFFFF **before** mask40 is tested and before B1210F
publishes the actual result/null to service+658. B12115 disarms **after** that
publication. A previous+658 value is not released. Conditional normal name
cleanup captures data before clearing mask40, then captures current length+1
and uses the actual raw-pool singleton/manager/gate. Raw headers remain stale
after return; diagnostics do not authorize replay.

B12142 captures current service+67C texture before B12149 reads current+658.
B12151 obtains that captured owner's material+14 through existing B4CBA0;
B12158 calls the complete existing unchecked B189F0 for slot3 and the captured
texture. No construction-time material/post snapshot is substituted. The
actual texture was produced through the existing renderer texture-cache path
in B14A10; this stage uses its current publication and canonical owner domain.

## Eight borrowed parameter records

Each row constructs a distinct raw name header, captures the exact source and
current+658 in the listed order, obtains material+14, registers through existing
B18B00/B18B20 or the exact B18AC0 expansion, then returns the raw name through
the current native string pool. No source values are copied into a fabricated
parameter record; the real B17E10 provider retains its established behavior.

| Literal/name | Borrowed source | DWORD count | State | Source/receiver order |
| --- | --- | --- | --- | --- |
| D5E430 cSceneColorSampleOffset | service+04 | 2 | 65 | current+658, source address |
| D5E2F8 cDisplacementSampleOffset | inherited EBX | 2 | 66 | current+658, inherited EBX use |
| D5E2E4 cAASampleOffsets | inherited EBP | 36 | 67 | inherited EBP use, current+658 |
| D5E2BC cLetterboxRatio | service+228 | 1 | 68 | current+658, EBX=source |
| D5E2AC cNoiseOffset | current service+34, then +08 | 2 | 69 | current+34, ADD08, current+658 |
| D5E2A0 cNoiseScale | service+648 | 2 | 70 | current+658, EBP=source |
| D5E294 cFlicker | current service+34, then +10 | 1 | 71 | current+34, ADD10, current+658 |
| D5E28C cShake | current service+34, then +14 | 1 | 72 | current+34, current+658, ADD14 |

All records use matrix0. B18AC0 receives vector count9 and forwards wrapping
DWORD `4*9` to B17E10. This is the same complete32-byte wrapper contract already
used by earlier packets; no new wrapper/library port. The supporting B18AD8
call is a separate evidence row, not another claimed implementation.

The first three names reuse the original continuation scene-color view and
the retained predecessor's actual displacement/AA views. Entry validates
EBX=service194 and EBP=serviceF4, but registration **uses the inherited bits**
at B121CE/B1222D. Recomputing those addresses is not the implementation.
Only B12296 subsequently replaces EBX with service228; only B1236B replaces
EBP with service648. The new state's EBX field carries the later frontier.
B122BC captures the letterbox name data before B122C3 resets EDI toFFFFFFFF.

The three +34-based rows each reload current service+34 separately. Callbacks
can replace that field between rows, so they may borrow different actual CCh
owners. Each captured backing owner must survive the full parameter borrow.
This fragment adds no retain, substitute storage or cleanup policy to make an
otherwise invalid backing lifetime safe. The original CCh producer, dust
initialization and outer texture-owner lifetime contracts remain authoritative.

## Final frame assignment and retained state

B1248F captures current service+1D4, then B12496 reads current+658. B1249C
composes the existing full B4E2B0 with these captures, preserving its incoming
publication/retain before captured outgoing release and current import/profile
loads. It does not use an earlier frame/post receipt after intervening callbacks.

The state retains the independent post20 block, nine raw name headers,
source/material/current-owner captures, actual allocation/result/publication
receipts and exact predecessor/context/entry/argument identities. The only
predecessor header change is one successor identity and two phases. Existing
canonical raw post construction remains authoritative; no registry, count,
companion or primitive is invented. All predecessor and provider lifetimes
remain required through dependent stages; explicit external quiescence governs
reset/destruction, including partially prepared blocks and failed acquisitions.

At B124A1: ESI=same service, EBX=service228, EBP=service648, EDI=FFFFFFFF,
ESP14=raw+658 allocation, mask0 and EH state-1. Other original/aligned/half
dimension cells and all three unread original argument cells remain retained.
Failure marks the whole chain failed and preserves publications, raw names,
borrowed captures and provider acquisition state. Existing provider failure
policies apply; this caller adds no rollback/free/retry or native FH3 cleanup.

Strict `scripts/build.ps1` passed MSVC Win32 Release with `/MD`, and all three
existing CTests passed. The report verifier checked 48 caller rows plus the
supporting B18AD8 wrapper call with zero failures. No new test or probe was
added. Static source/live-byte/build evidence is not composed
runtime, native ABI/FH3/SEH/private-stack alias, full initializer/teardown or
gameplay proof. Earlier packets retain their published limits.
