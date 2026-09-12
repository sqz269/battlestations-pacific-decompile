# GUI timed-entry concrete profile operations

This packet supplies the 14h entry storage, base/alpha construction, derived
destructor phase, countdown dispatch and concrete alpha update for the existing
canonical widget owners. Allocation, the SAME widget +88/+8C/+90 header and
flags-dependent matching free belong to `GuiTimedEntryOwner`. No foreign raw
entry is adopted; numeric profile identities are never callable C++ vtables.
Names are hypotheses, not recovered symbols. Ghidra remained read-only in
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; wrappers verified the
project/program for each live batch. Shared exports were refreshed.

| Address / native ABI | Inclusive extent | C++ coverage |
| --- | --- | --- |
| AD3970: ECX entry, widget stack, EAX entry, RET4 | AD3970..AD3989 | complete constructor; leaves +C/+10 unwritten |
| AD3990: ECX entry, RET | AD3990..AD3996 | complete base-profile reset |
| AC2ED0: ECX entry, widget stack, EAX entry, RET4 | AC2ED0..AC2EFA | complete alpha constructor; live D7A24C read once |
| AC2FE0: ECX entry, flags stack, EAX entry, RET4 | AC2FE0..AC2FFD | partial: destructor phase; BF65AC/free phase belongs to allocation owner |
| AD3A40: ECX entry, flags stack, EAX entry, RET4 | AD3A40..AD3A5E | partial: destructor phase; BF65AC/free phase belongs to allocation owner |
| AD39A0: ECX entry, delta then unused widget stack, AL result, RET8 | AD39A0..AD39E9 | complete countdown/dispatch for established current8 bindings; unknown profiles throw |
| AC2F40: ECX entry, delta stack, AL changed, RET4 | AC2F40..AC2FD9 | complete arithmetic and canonical current54/current4C composition for established widget profiles |
| ABE7B0: ECX entry, delta stack, AL changed, RET4 | ABE7B0..ABE886 | partial: current5C gate and separate numeric fragment; type17 owner/read/emission ABE7C9..ABE877 remain unbound |

`GuiTimedEntryStorage` is exactly 14h on Win32: DWORD profile, float countdown
at+4, borrowed `GuiLayoutWidget*` at+8, target at+C and rate at+10. Its pointer
is a new C++ canonical identity, not an original widget ABI address. Every
widget dispatch resolves that token through the SAME `GuiWidgetOwnerRuntime`.
There is no second widget, mirrored entry payload or private color state.

AD3970 initializes only profile D5D204, zero countdown and borrowed widget.
AC2ED0 calls it, loads D7A24C once through MOVSS, writes D5CA74 then the same
loaded bits to+C/+10. Base destruction writes only D5D204. AC2FE0 calls
AD3990; AD3A40 writes it inline. Neither releases the widget or other resources.
Both scalar deleting bodies free iff flags bit0; C++ `destroy_gui_timed_entry_profile`
implements only the reset and rejects unsupported current profiles. The owner
transports flags0/flags1 and its matching allocation provenance. AC2FF5's omitted
saved-listing ADD ESP4 was verified from live bytes `83 c4 04`.

AD39A0 first subtracts delta using x87, spills to float32, reloads and publishes
the countdown, then tests zero against it. Exactly zero and unordered countdowns
return true without virtual8. A negative countdown produces the virtual8 delta
with SSE `D7A208-countdown`. The native second stack argument is unread; the sole
direct caller AA88E6 pushes original widget EBP (proven by AA87B9). RET8 confirms
both stack words. Countdown becomes +0 only after successful virtual8 return,
including a false result. A missing owner exception leaves the negative value.

AC2F40 calls the borrowed widget's current54 with a local float4 buffer, consumes
the returned pointer's alpha, and snapshots target through x87 float32 spills.
It preserves the equality/parity test, unspilled rate*delta product, candidate
float32 spill and ordered clamp. A change calls current4C after RELOADING the
entry's borrowed widget token. Established Text/current4C reaches the existing
`GuiTextRuntimeImplementation::set_alpha4c_00ab6ad0`; current54 uses parent-owned
AA68F0 material-diffuse lookup, including the no-geometry/zero-section fallback.
Reading only `GuiLayoutWidget::color` is not its general current54 contract.

ABE7B0 uses current5C and returns false unless it equals 11h (17, Section).
The Section branch snapshots widget+F4, +F8, +100 and target+C. It approaches
the first value, then calls **ABE6E0**, with the candidate, captured +F8/+100,
and literal FLD1. ABE6E0 compares/stores +F4/+F8/+100/+104 and calls current7C
when changed. AA68F0 is the color getter, not this four-value setter. No
canonical Section owner with those fields/current7C exists in this packet.
Unknown type-query profiles throw; even an implementation proving current5C=17
reaches an explicit Section boundary. The numeric helper is usable separately,
but does not complete the Section widget update or geometry emission.

The alpha and Section floating-point bodies differ on unordered direction.
With current0, targetNaN, rate1 and delta1, alpha yields +1 while Section yields
-1; both leave the finite candidate because unordered compares skip clamping.
An unordered current yields NaN. Equal values return false before rate loading;
a zero step with unequal values still returns true and calls the setter. These
are native branch results, not an imposed mathematical interpolation policy.
The assembly helpers retain the caller's x87/MXCSR state and do not normalize
NaNs, signed zero, negative rates or floating-point control modes.

Live xrefs enumerate AD3970 from AC2ED8, AD3990 from AC2FE3, AC2ED0 from AD3B1A,
AD39A0 from AA88E6, and profile-table data references for AC2F40/AC2FE0/AD3A40.
ABE7B0 lacked a reported xref; D5D218's live DWORD proves its current8 slot.
D5CA74's three words are AC2FE0, AC2F00, AC2F40; D5D210's are AD3A40, AD3A20,
ABE7B0. The current4 configuration entries AC2F00/AD3A20 remain undefined in
Ghidra and are not claimed as reconstructed callable slots here.

Verification: full translation unit compiled with MSVC x86 `/W4 /WX /fp:strict`
against the parent's actual owner-header integration. An ignored local probe
compiled exact extracted constructor/destructor/numeric source fragments and
passed borrowed identity/default preservation plus seven Section numeric cases
and the alpha unordered-direction case. It did not link replacement owner stubs
or claim native differential execution. The first whole-object probe could not
link without the real canonical owner library, so no result is claimed for it.
`./scripts/build.ps1` configured successfully, then failed twice with MSBuild
access-denied errors creating/writing Lua/zlib `.tlog` paths. Explicit directory
creation succeeded; retry failed writing MSBuild temporary tracking files.
Logs remain under `local/gui_timed_entry_types_build*.log`; the parent must run
the registered translation units in its normal integration build. There is no
game runtime, native differential, original ABI or Section rendering claim.

## 2026-09-12 timed and clip ownership integration

The parent links these routines with the canonical allocation/widget owners.
The combined Win32 build passes both existing tests. Separate linked-library
fixtures cover actual alpha/countdown integration and the seven Section numeric
cases, including NaN direction. This does not bind the active Section widget or
execute original native bytes. See reports/orch5_timed_clip_batch.json.
