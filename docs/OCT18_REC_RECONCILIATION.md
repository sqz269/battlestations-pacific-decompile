# October 18 reconciliation lane

Packet: the 96 rows in coordinator `local/codex_tail_R.txt`. Owner: agent/oct18-rec.
No game hosts or original installation are modified; this is reconstruction accounting
and bounded source work. Descriptive names remain hypotheses. Build evidence does not
establish original ABI compatibility or game validation.

## Progress board correction

Baseline at origin/main 21b82decf: 203 remaining (all segments), 19,994 game functions
done, 20,197 live game targets. The history parser remembered the first fragment at an
address and discarded every later function record. This hid whole bodies already
recorded in the ledger, especially naked MSVC definitions the source-name regex misses.
`first_ledger_records` now promotes a fragment when the first whole-function record
appears, using that function record's commit date and author. No duplicate implementation
or duplicate function record is required. A focused synthetic history checks promotion,
its timestamp, a fragment that stays partial, and an already-complete function.

## Compiler and library exclusions

The following original scalar-deleting destructors are compiler-generated wrappers:
00403610, 00403C60, 004041F0, 004042A0, 00405FC0, 00406210, 00406240, 00406270,
00406290, 004062F0. Exports show an ECX object, stack deletion-flags byte, conditional
`_free` under flag bit 0, and the returned object. The SAP variants perform member
cleanup or call the existing destructor first. This exclusion applies to those
generated deleting entries, not to the in-house Dyn physics implementation.

007512B0 and 007512F0 are telemetry library scalar-deleting entries: RTTI identifies
`iostdnet::ZMemoryPooledObject` and `iometrics::ZMetric::IStoredData`; both restore the
telemetry base vtable and conditionally call TELEMETRY_iostdnet_00A51E70. They are
outside the game-code contract even though RTTI tags obscured that classification.
Evidence: per-address `exports/bsp/functions/<address>/decompiled.c` and assembly;
existing inventory comments and direct callee identities. No Ghidra annotations changed.
