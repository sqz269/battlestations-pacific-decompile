# Raw locale library integration

Three ordinary C++ entries now supply raw storage transport that the older
semantic LocaleTables interfaces do not provide. They are registered in
bsp_core and remain unbound to startup.

* A9EBA0 scans the actual manager's WORD sidecar vectors, retains first-match
  behavior, reloads current pointers after the potentially returning invalid-
  parameter provider, and reaches current Source towupper only on a miss.
* A9EC30 operates on the actual raw8 header. It captures the destination before
  mapping, stores there after the call, then compares current length. Zero
  length reaches neither the data pointer nor the manager/provider.
* AA0D30 routes wrapping manager+4 directly to the existing raw string-vector
  append helper using the same retained raw string-pool context.

Primary reviewed the complete CPP/HPP/doc sets against saved assembly and the
existing real callees. Worker commits are 79019d950 for the two uppercase
entries and 4ae5e1069 for registration. Both isolated strict MSVC Win32 object
checks passed. The combined current-main Win32 Release build passed both
existing checks in 14.19 seconds. No tests or probe executables were added,
and none of these new bodies was executed as part of qualification.

The Source input to A9EBA0 is the zero-extended WORD supplied by A9EC30;
arbitrary high bits of an incoming Original stack DWORD are outside it.
BF6713 binds the existing current-SDK invalid-parameter entry and may return;
it is not an invented range exception. Current towupper uses current CRT,
locale and PTD policy. Original vendor behavior, register/tail-stack ABI,
FH3/hardware-fault behavior and production compatibility remain unproved.
The existing semantic APIs are retained separately; their vector::at behavior
does not describe the raw invalid-parameter path.

The naming ledger and saved Ghidra comments now correct A9EC30's stale cdecl
description: ECX is the locale manager, the header is stacked, and return is
RET4. Its descriptive name is BSP_Localization_UppercaseWideStringInPlace.
Prior annotations were recorded, existing comments preserved, the project
saved, and the three entries plus two affected callers re-exported.

No canonical F8BC4C owner, actual loaded manager, native lifetime dispatch,
full map/parser/sidecar loading or startup consumer is supplied by this batch.
The separate constructor/base packet is dormant work, not activation. Raw
WORD-vector growth lacks a compatible existing provider; differently sized
string/DWORD helpers cannot substitute. Vendor bodies receive no new credit.
All nine raw Text host choices remain FALSE. Frozen ac60 remains unchanged
and on its Font48/Viper19 Source-admission hold; no runtime, visual, whole
caller, full13/USN21/Page/P4 or production proof transfers.

Exact validation and Source pins: reports/codex_raw_locale_binding_integration.json.
