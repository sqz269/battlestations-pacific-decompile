# AZ integrated source and ledger audit

The read-only audit found all **30 actual-storage bodies / 3,792 bytes** present
and consistently registered. No source, native range, name or body-count
correction is required. Two stale validation descriptions in the selected
ledger records were corrected by the primary integrator and independently
checked with a selected-record reread. No audit finding remains unresolved.

The inspected primary checkout was
`J:/PROG/battlestations-pacific-decompile-orch2-20260910`, HEAD
`529025f3fd6bd28f55c7ac69d5a5c4826b9b62d0`, at 2026-09-13 06:05:52 UTC.
Integration metadata was still being finalized. The companion
[report](../reports/native_az_ledger_audit.json) pins the inspected source,
headers, component reports and registration file, plus retained local query
evidence. It does not substitute for the primary's later exact-commit promotion
gate.

| Component | New actual bodies | Native bytes |
| --- | ---: | ---: |
| Entry and raw constructor | 2 | 749 |
| Open and lookup | 3 | 488 |
| Enumeration and defaults | 7 | 769 |
| Raw inflater methods | 10 | 870 |
| VFS device route | 6 | 898 |
| Runtime position leaves | 2 | 18 |
| Total | 30 | 3,792 |

For every counted body, the actual-source function ledger record, source
definition and public declaration exist; coverage is complete. Each of the
seven source modules appears exactly once in `cmake/startup.cmake`. All 30
reported ranges equal current Ghidra `proto` ranges, all names agree with the
name ledger, and every reported native SHA-256 agrees with the installed PE.
The executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The CLI verifies `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`
before its read-only queries.

BDD0A0, BD90D0, BDBC00 and BEF750 extend existing body bindings and are excluded
from the new-body total. NativeMpakRuntime composition methods, historical
projection records and the three repaired compiler unwind funclets are also
excluded. BBC320 now has the complete range through BBC3D9; primary flow/EH
analysis is preserved rather than counted as additional source.

Live D641F8 bytes identify BB5BB0 at +8 as open and BB4B20 at +10h as contains.
BB79E0 is the false default and BB7A00 clears 14h bytes. The raw profile is
D64400; its buffer headers hold end at +4 and current cursor at +Ch. Those
values agree with the corrected docs and actual source. BB4A60 has no remaining
selected tag record; the integration report preserves its removed
`stl_probable` record and bookmark-removal result. BB4140 remains
`STL_inst_00bb4140`; BB4F40 and 5EFBA0 remain explicit original library/search
contracts without newly reconstructed library bodies.

The two inspection-time metadata findings concerned completed validation:

- **AZ-L01:** all ten raw-method reconstruction rows still said an independent
  fixture was pending, and their name evidence repeated that status. The current
  raw report and independent validation report already record the completed
  1,305-check fixture. The correction should reference that result while
  preserving shared stock-zlib/support and native FH3/stack-spill/game limits.
- **AZ-L02:** BB5080's reconstruction evidence said compressed conversion
  remained unvalidated without restricting that statement to the initial
  entry packet. The later fixture compares three compressed entry caller
  pairs with 1,468 checks, using shared actual constructor, raw methods and
  conversion support. That is bounded caller composition evidence. The sealed
  initial 218-check fixture remains unchanged.

The follow-up reread verifies all eleven actual reconstruction records now use
`actual_storage_win32_native_source_fixture_passed`. The ten raw name records
explicitly supersede their earlier pending wording with the bounded 1,305-check
result. BB5080 now identifies the initial 218 checks and later 1,468 checks,
three compressed caller pairs and 450,369 decoded bytes, with shared-support
limits preserved. Source, header and registration hashes are unchanged. The
report retains the original inspection snapshot and separately pins the
resolution evidence; no body audit or fixture was repeated.

The audit adds no native bodies, source changes or tests. It does not rerun the
1,305/1,468-check fixtures, assert original ABI/FH3 compatibility, or establish
installed archive/gameplay behavior. Primary integration status correctly
leaves its final replay promotion pending while that separate work proceeds.
