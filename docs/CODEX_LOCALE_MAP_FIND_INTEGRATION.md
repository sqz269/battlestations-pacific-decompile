# Raw locale map lookup integration

The actual A9EC70 bucket-chain lookup is now a Core Source entry, separate from
the existing semantic LocaleTables lookup. It is needed beneath the raw locale
resolver and eventual actual consumers. It creates no owner or loaded-manager
permission, and it changes no application caller or startup switch.

Primary and independent complete Source/65-line Native assembly review accepted
the captured hash inputs, signed-byte ASCII folding and DWORD wrapping, current
candidate/key operand order, empty-key hit without data reads, current CRT
comparison and post-mismatch next-pointer reload. Existing insertion, uppercase,
semantic lookup and resolver sources stay unchanged.

The strict x86 object passed with machine014C. One direct bsp_core registration
in root CMakeLists passed the standard Win32 build and both existing checks in
13.29s. No new test, entry/game execution, Native ABI/FH3/hardware or runtime proof
follows. The ledger preserves the prior semantic implementation and records zero
additional Native body credit. Exact source/assembly/object pins and limits are
in reports/native_locale_map_find.json.

Actual canonical consumer/lifetime authority, full raw resolver/loading and
raw16/startup remain gated. The nine raw Text switches remain off.
