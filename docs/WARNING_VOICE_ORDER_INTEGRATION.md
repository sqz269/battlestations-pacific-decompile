# Warning tables, voice playback and HUD order records

Addresses: 009870a0, 0097f570, 00979990, 00979730, 0096dbb0, 005bbc10,
005bbdc0, 005b71d0, 005babb0, 007027b0, 005b9760, 005b7790, 00815440,
00816a40, 0064b870, 00651800, 0067c4f0

This batch adds three bounded reconstructions from the documented warning and
unit-command follow-ups. Workers used disjoint leased worktrees; the joining
orchestrator reviewed and combined their results in agent/orch4-20260910.
The main orchestrator's renderer, settings and platform work remained separate.

The warning loaders use the existing Lua 5.1.1 host and native-string comparator.
They recover recursive alternative-message and escape-replacement tables,
including prefix accumulation and replacement on reload. The installed Lua
fixture contains 122 message IDs, 36 entity replacements and 5 section replacements.
The Init implementation is explicitly the table-loading fragment only.

The voice implementation models the one-slot polling sequence, cleanup and
callback reentry, speaker selection, line construction/queueing, positional
admission and fade startup. The old static readiness and scalar attenuation
helpers were unused and have been removed. The native unordered admission
branches and original clip payload are preserved; meaningful sound, subtitle,
camera and other game services remain required host interfaces.

The order constructor clamps two float parameters to [-2,+2], writes four
bound floats and a kind byte, and preserves seven untouched bytes. The issue
sequence publishes before its session-mode check, constructs the session
message and copies the complete 20h-byte record into it. Its previous compact
message interface has been removed. HUD quantization remains three bounded
fragments with explicit CRT call sites. Review corrected negative-zero steering
subtraction and the previously misattributed 00651800 HUD function boundary.

## Evidence and verification

The packet reports are reports/warning_message_table.json,
reports/voice_line_playback.json and reports/unit_order_record.json. The primary
independently checked the main ABI, branch, payload and ownership sites, and a
second review checked the constructor/issue sequence and signed-zero correction.
An isolated native reference matched all 32 record bytes in 121 constructor
comparisons, including clamp boundaries, signed zeros, infinities and quiet NaN.
Its 209 code bytes and two 4-byte constants matched disk and saved Ghidra bytes.
Only the two constant addresses were relocated; no game process was loaded.

The combined build, existing tests and saved-analysis results are recorded in
reports/warning_voice_order_integration.json. Ghidra mutations use the repository
write lock. The missing 00651800 function and five CRT fall-through gaps have
separate definition/repair records in reports/warning_voice_order_*.json.

## Limits and follow-ups

These are new C++ interfaces, not original binary object or exception ABIs.
No game, audible playback, HUD interaction or network roundtrip was validated.
The native constructor comparison does not cover the full issue path:
signaling-NaN arguments are quieted by its native x87 ingress spills, which the
typed issue API does not reproduce. General x87 precision and unmasked FP
exceptions remain outside the verified domain. The existing bounded order-queue
projection retains its documented eight-slot host capacity, not a native bound.

Useful next packets are concrete warning substitution/line-start services,
warning subscriptions/producers, and the receive side of unit session messages.
Their addresses must be checked and leased before dispatch; they are not part
of this completed bounded batch. See the worker documents for exact call sites.
