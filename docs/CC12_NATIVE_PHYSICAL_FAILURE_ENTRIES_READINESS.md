# Independent physical failure entries: readiness

The two raw entry contracts are ready for primary consideration as a bounded Source implementation. **The genuine borrowed manager needed to execute the notifier is still unavailable in the retained evidence.** No Source implementation is authorized by this audit, and raw read `00BF5030` remains unready.

| Boundary | Finding |
| --- | --- |
| Exact raw callback `00530620` | Ready contract: the genuine whole Original body is one byte, `C3`; no owner is needed |
| Raw notifier `00BD9E30` | Ready implementation contract under the genuine borrowed-manager/callable-target preconditions below |
| Notifier execution with current retained inputs | Unready: no genuine actual-manager/+90 callable-domain preimage is available |
| Connected raw-read failure or whole owner graph | Unready; independent entry readiness does not supply publication or field18 provenance |

This packet changes only this document and its JSON report. It performs no C++/CMake/ledger/packet-registry/Ghidra edits, build, link, native call, or old-fixture replay. Baseline: `8ce20387edb6eecf9c346d99d37b1ec06fabf35d`.

## Complete notifier ABI

The complete Original `00BD9E30-00BD9E3A` is **11 bytes / 3 instructions**, SHA-256 `58246cba1d89a1a17a59328d7a44e07142d11b9540850343ee031982143d014c`:

```text
8B 81 90 00 00 00    MOV EAX,[ECX+90h]
FF D0                CALL EAX
C2 04 00             RET4
```

ECX is a borrowed actual manager; the only data read is its DWORD at +90. The entry does not read global `0109CEEC` or manager+18. Incoming EDX reaches the callback unchanged. EAX is set to the actual loaded target before CALL. A one-DWORD stack input is discarded by RET4.

Let S be ESP on notifier entry. At callback entry, ESP is S-4: [S-4] is the return address inside the notifier, [S] is the original notifier caller return address, and [S+4] is the discarded DWORD. The notifier pushes no callback arguments. The target must return without popping callback arguments; a balanced plain RET/RET0 returns to the notifier, whose RET4 leaves the caller at S+8.

For the genuine C3 callback, EAX returns as the loaded target address, ECX remains the manager, EDX remains its incoming value, and other non-stack general registers and arithmetic flags remain unchanged. That EAX value is observable machine state, not a recovered semantic return value. Other callback effects or callback exception/unwind behavior are outside this bounded admission.

BF5030 separately loads actual `0109CEEC`, reads manager+18 into EDX, pushes the same field18 value, and calls this entry. Equality between EDX and the stacked word belongs to that caller path. A raw notifier must not add a +18 fetch, forward the stacked word as a callback argument, or impose their equality as an independent-entry rule.

## The callback really is RET

The complete Original `00530620` body is **one byte / one instruction**, `C3`, SHA-256 `ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e`. This is a genuine Original leaf installed by startup, not a proposed default or synthetic successful callback. It consumes only the return address; it needs no owner/global/data pointer, reads no manager fields, and leaves non-stack registers and arithmetic flags unchanged.

Current Source `ignore_native_vfs_mount_failure_00530620` emits **three bytes**, `C2 00 00` (`RET0`). Its normal stack/register/flag behavior is equivalent, but it is not the exact one-byte body. A separate explicit raw entry can represent C3 exactly; empty C++ syntax alone does not prove emitted bytes. Complete production COFF and linked/loaded bytes would need verification after primary Source authorization.

## Genuine borrowed input and code domains

For a notifier invocation, the caller must lend a genuine, already-produced initialized actual manager whose lifetime spans the call. It must have a readable +90 DWORD containing an executable target in that process. Readable 94h storage alone is not ownership evidence; a zeroed byte array or mock owner with a convenient callback word is excluded.

The bounded callback is the genuine Original C3 entry in its actual loaded image, or a separately qualified exact Source counterpart after genuine callable-address publication. Numeric `00530620` is callable only if that actual code is present at that address. A Source function's address cannot be silently substituted into a fake owner, nor can an Original identity be cast to a host pointer without the producer/domain proof. EDX and the discarded DWORD are real caller inputs; connected BF5030 additionally requires their actual field18 provenance.

The notifier borrows an owner directly and does not publish it. It does not need a global slot merely to define its own entry contract. However, no genuine borrowed runtime manager or +90 preimage is retained here. These preconditions make the independent implementation precise; they do not establish that the current production Source route or a native fixture satisfies them.

## Actual producer and current Source boundary

The exact **32-byte / 4-instruction startup fragment** `0073D63C-0073D65B` reloads actual `0109CEEC`, stores manager+90=`00530620`, reloads the publication, and stores +8C=`00735B30`. Fresh Ghidra xrefs identify the callback's DATA reference at `0073D642`. This fragment is not a whole Application_Initialize claim.

Current complete Source provider evidence is:

| Provider | Bytes / instructions | Interface |
| --- | --- | --- |
| Notify BD9E30 | 24 / 11 | cdecl manager/discarded/dispatch; injected virtual slot+0C |
| Empty callback00530620 | 3 / 1 | complete RET0 |
| Callback installer | 32 / 9 | literal Original identities through supplied publication reference |
| Game runtime failure dispatcher | 15 / 5 | forwards to identity binding |
| Runtime open-failure binding | 27 / 9 | checks identity00530620 and calls named Source callback |
| Typed physical read | 103 / 42 | extra context and direct fastcall function-pointer call |

The Source installer's +90 immediate has no COFF relocation to a Source callback symbol. Its runtime identity dispatch and the typed physical helper's direct-pointer route remain distinct. Neither the typed adapter nor that direct helper supplies a native ECX/EDX/RET4 notifier or a demonstrated callable-domain owner. The static zero-filled `0109CEEC` image also supplies no runtime owner.

## Evidence and smallest next work

Manifest SHA-256: `124e5fafa76bcaf3ac6f0491b7f04e125092d7924a8d40d4a1a1e0187086c149`. The unique family is `local/cc12_native_physical_failure_entries_readiness_20261008a/audit01`. It contains **89 physically retained rows**, including the whole installed PE, complete11/1 bodies and32-byte producer fragment with prior live evidence, five complete current Source providers and headers, five complete COFF objects, the whole existing core archive, selected tool/backend files, and the accepted parent audit with its wording correction. Each complete object matches one archive member exactly. Source provider hashes still match the prior accepted audit. Every retained row passed pre/copy/post equality.

The existing archive is the prior normal-build result, not a new build of merged main. Retained compiler backends were not executed. Historical unused header/library candidate hash dictionaries were transient; candidate counts are neither retained identity pins nor physical copies. This audit counts only selected rows with actual frozen copies.

After primary authorization, the bounded Source candidate is two separate raw entries: whole11-byte notifier and exact1-byte callback. The callback can be considered independently of an owner. A notifier execution still needs a genuine actual manager and callable +90 target; do not run it on fabricated storage. The smallest missing provider/witness is at the existing `0073D63C` callback-publication boundary: establish the actual already-produced manager, its lifetime and +90 preimage, and the real executable target. If unavailable, identify that exact owner producer and stop before a larger VFS expansion.

Raw BF5030 Source, a synthetic `0109CEEC`, altered production callback domains, and connected failure-path execution are outside this packet. Machine-readable details: [cc12_native_physical_failure_entries_readiness.json](../reports/cc12_native_physical_failure_entries_readiness.json).
