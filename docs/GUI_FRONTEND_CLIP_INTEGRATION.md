# Frontend progress, ClipBox fields and widget lifetime audit

The integrated C++ now converts loading progress through the shared recovered
CRT ST0 entry and constructs type16 ClipBox companions over their existing
layout fields. The accompanying lifetime audit corrects the base deleting
destructor identity; it does not introduce a widget retention implementation.
Source validation is pinned to `f9da084da9fcd674a3ec90d11f103cf423d50b9b`.
The integration report records hashes and preserved worker evidence.

## Recovered behavior and corrections

At0057BEC0, the x87 comparison chooses the incoming float on equal or unordered
operands, preserving the old value only when it is strictly greater. The
incoming extended ST0 value survives the comparison. Multiplication by the
actual double at00CEF258 (128) precedes the float store at+30 and the call to
00BF7420; the resulting EAX updates signed progress units at+2C. These are
progress units, not clock ticks. See [the source audit](FRONTEND_PROGRESS_CONVERSION.md).

The default adapter now calls `native_crt_truncate_st0_00bf7420` beside the
existing00BF7456 adapter. The GUI group-radius path calls that same entry.
The actual mutable CRT dispatch word remains a required borrowed input, and
the library name in Ghidra is preserved. No duplicate conversion kernel or
invented native global was added.

ClipBox constructor00ACE0A0, copy00ACE0F0, update00ACE120 and reader00ACE650
are implemented by the type16 companion. Derived fields+EC..108 remain
uninitialized until the original paths write them. The reader's BorderWidth
default is0.1; ancestor clip parameters reference the same fields. Verified
x87 operand order distinguishes DC C9 from D8 C9. The integrated reader borrows
the same live CRT-mode reference already used by the type dispatcher. Copy
still requires the unreconstructed00AA9520 service, and AA8710 current+24
dispatch is a separate integration task. See [ClipBox evidence](GUI_CLIP_BOX.md).

The actual base scalar deleting wrapper is00AA9A90, which calls00AA9730 and
returns storage through00AA75F0/F8BC94.00A9E130 instead destroys the derived
D5BBF8 profile and has wrapper00A9E210. Manager/parent teardown directly calls
current+04(1) without consulting widget+04. Current host page/widget counts
and unique ownership cannot be replaced by an isolated shared_ptr token.
Canonical widget identity/count, derived deletion and material-parameter drain
remain coordinated work. See [the retention audit](GUI_WIDGET_RETENTION.md).

## Validation and saved analysis

- MSVC Win32 Release build and both existing CTests passed.
- Sixteen saved-native loading comparisons passed across both CRT dispatch
  modes, including float bits, signed units and full x87 status/TOP. The host
  side used the default shared converter; null-screen bypass was checked.
- Eight ClipBox native field comparisons passed, including exceptional float
  inputs; the reader default also passed.
- Three original group-bound writer comparisons and eight radius comparisons
  passed with the shared converter dependency.
- Sixteen selected Ghidra names/comments were read back, five pre-existing
  comment fields were preserved, and all sixteen exports were refreshed after
  locked annotation updates. Project/program were verified before each batch.

No functions required creation. Two false free-call continuations were decoded:
A9E130 now has no reported call gaps; AA9730's104-byte continuation is decoded
but its stored function body still ends atAA9950. The latter is explicitly an
incomplete body-metadata repair, not a fully repaired saved function.

The parallel lease incident was separately repaired by preserving756 complete
records and removing only the811-byte malformed suffix from this integrator's
failed claim, after exclusive preimage verification and backup. The subsequent
[transaction fix](LEASE_TRANSACTION_SAFETY.md) passed one isolated concurrent
Windows regression and four existing exporter checks in the integrator tree.
Older writer worktrees/processes remain unsafe until they consume that fix.

These results establish reconstructed behavior, compilation and bounded native
fixture agreement. They do not establish binary replacement compatibility,
complete GUI material lifetime, a working draw path or gameplay validation.

## Follow-up packets

1. Bind current+24 to supported widget companions during AA8710 layout finish.
2. Finish canonical native material/pool publication and raw material color.
3. Coordinate widget/page identity, current deletion dispatch and actual
   material-parameter lifetime before enabling the full GUI bridge.
4. Extend AA9730 stored function-body metadata through its verified return
   using a supported, locked Ghidra operation.
