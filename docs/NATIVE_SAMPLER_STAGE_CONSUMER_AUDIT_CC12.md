# Bounded native sampler stage-field audit

The audited canonical pass path interprets sampler-row+04 as a **byte** during
texture binding and transports the physical field as a **DWORD** during array
grow/copy. This does not establish whole-program consumer closure or justify
changing the existing complete-word acquisition guard. Unknown private upper
bytes remain unknown; no default, normalization or captured value is introduced.

## Identity, scope and evidence

Source base is Main `23fbb8a1d`. Target configuration and saved export metadata
identify `C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`; the live wrapper confirms that program. Original
installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The local read-only validator uses `bsp.py ghidra bytes/disasm/ensure` wrappers,
saved function exports and the original PE. All 996 bytes in 19 explicitly
listed spans match live Ghidra memory to the original PE. Thirty selected
instruction lines match saved and live listings; their original PE bytes are
independently disassembled with Capstone. Full receipts, scope endpoints and
hashes are in `reports/native_sampler_stage_consumer_audit_cc12.json` and
`local/cc12_sampler_stage_consumer_audit`. This verifies static bytes/listings,
not execution, ABI or every control-flow/alias path.

The field means `[*(pass+24h) + row_index*8 + 4]`. Pass+28h is the **header
count**, not this field. Render-state owner+08h rows also have an eight-byte
stride; their second DWORD is not automatically this texture-stage field.

## Audited physical operations

| Operation | Exact instruction(s) | Access to stage-row+04 |
| --- | --- | --- |
| B5F100 append | B5F10B; B5F13C/B5F142 | Writes the stage low byte in its stack local, reads that complete local DWORD, stores all four bytes into the new row. No upper-byte initializer is added. |
| B40C80 reserve/grow | B40CBF/B40CC3 | Reads and copies the complete DWORD for each existing row. It does not inspect stage meaning or initialize unused rows. |
| B40E00 resize | B40E29/B40E2D | Zero-writes both DWORDs in newly exposed rows. Shrink changes count without reading or clearing their bytes. |
| B41E80 pair copy | B41ED5/B41ED8 | Reads and copies the complete DWORD. B455C0 passes actual source/destination+24h to it at B455E2..B455E9. |
| B414F0 push-back template | B41521/B41524 | Reads and copies the complete second DWORD of its input pair. No current live xrefs or absolute-address byte-pattern matches were found; this is not proof against computed indirect entry. |
| B41530 clear | B41532 | Forwards resize(0); no direct row-byte interpretation. |
| B5F3F0 destruction | B5F3F5/B5F3FD | Resize(0), then free backing storage; no stage-value read. |
| B5F510 normal pair-cleanup tail | B5F666..B5F67C | Addresses the header, resizes zero and frees data; no stage-value read in this bounded tail. |
| B5F720 initialization prefix | B5F753..B5F75B | Clears the data/count/capacity header only. It does not initialize a caller's private stack word or already absent row data. |
| B43410 texture loop | B434B4/B434F3 | Two BYTE comparisons, disabled-pixel skip and vertex/pixel stage selection. B434BE reads the separate signed selector DWORD at row+00. |

The two full-width copies are transport, not evidence of a full-width semantic
stage predicate. Conversely, a byte predicate does not erase the original
full-width transport or prove its upper bits unobservable outside this scope.
Resize's explicit zero producer applies only to its newly exposed rows;
B5F100 does not obtain its stack word from that producer.

Live callers of B40C80 are B40E00, B414F0, B41E80 and B5F100. Live callers of
B40E00 are B41530, B41E80, B5F3F0 and B5F510. These are the queried direct-call
sets, not a complete alias/indirect-call census. B41E80's direct caller is
B455C0. Self-copy clears count before reading the live source end, rather than
being an identity no-op; distinct header aliases require their native allocation
and callback contracts to remain valid.

## Canonical profiles and escape boundaries

Original/live table words identify D61BE8 virtual+08h as B454D0. Its audited
15-byte wrapper forwards to B44750, whose established route reaches B43410.
D62A80 virtual+08h is the three-byte B5E5E0 `RET4` no-op. Both tables' +0Ch
entry is B5F6A0: its audited 64-byte body forwards the actual pass to B5F160,
then caches state owners at pass+18h/+1Ch/+20h. B5F160's existing typed/saved
representation uses those render-state owner rows; that is separate from a
new machine-level closure proof for every call it can reach.

B43410's texture loop passes selected resource pointers and slot numbers to
the current renderer's virtual+130h. It does not pass the stage row itself at
that call. Its later B4353A/B4353C sequence passes the **whole pass** as ECX to
B42350. B435B1 is a callback loaded through the entry/material chain. These are
remaining alias/callback boundaries; no new exhaustive register/x87 recovery of
the large constant builder or arbitrary callback targets is performed here.
The B455C0 copy prefix is audited; the rest of that large function is not
reclassified as a closed consumer set by this packet.

D5E13C remains excluded. Its queried live data xref is B10404; only its raw
table words are recorded. This packet neither assigns it a compatible pass
layout nor activates its constructor/virtual paths. Other profile substitutions,
row-pointer escapes, computed calls and external raw-storage inspection remain
outside this bounded proof.

Saved analysis has known retirement-tail limitations: B5F3F0's recorded body
can stop before its returning-free tail, and the B5F510 saved listing/body does
not establish every tail instruction. The bounded live-memory/PE checks and
existing raw disassembly cover the cited bytes without repairing saved control
flow or claiming complete current live instruction coverage for those tails.

## Gating result

The current strict complete-word guard stays in place. This evidence supplies
additional exact sites for diagnosing an actual GuiFont sampler failure and
separates byte interpretation from opaque DWORD transport. It is insufficient
to authorize a defined-stage replacement interface, fabricate upper bytes, or
claim an exhaustive consumer audit. Closing the whole-pass alias and dynamic
profile/callback boundaries would require a separately owned, qualified packet.

No reconstructed Source, core service, sampler guard, renderer/host, original
installation, saved Ghidra state or probe22 receipt is changed. No build or
runtime launch is required for these documentation-only static findings.
