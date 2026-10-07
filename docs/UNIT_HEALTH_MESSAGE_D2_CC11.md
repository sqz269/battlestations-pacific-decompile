# D2 unit-health message and borrowed setter constructor

The complete ordinary `00876D30` constructor now has typed 32-byte storage,
an executable five-slot source profile, and an opt-in constructor adapter for
the reconstructed `00877B90` setter. The adapter remains abstract: actual table
and session access, health notification/provider, and complete routing are
still required. It binds the setter and the base constructor to the same
mutable game-pointer cell. No concrete runtime host is installed by this packet.

## Evidence and coverage

Primary handoff `cc0d428cc` defined the missing functions in three independently
reviewed gaps and refreshed their exports. The worker changed no Ghidra data.
The definition receipt is `reports/unit_health_message_d2_definition_cc11.json`.
Original profile `00D032A0` contains the following native entries:

| Slot | Original entry | Reconstructed behavior |
| --- | --- | --- |
| 0 | `0075FE50` | Stamp root profile, release on flags bit 0, return identity |
| 1 | `0075FD10` | Write type 8, sender 12, relay 1, payload 8 bits |
| 2 | `0075FD60` | Read those fields through the wrapper's cursor at +4 |
| 3 | `0075FCE0` | Accept query D2 or 46; no receiver-field comparison |
| 4 | `004499C0` | Existing always-true operation |

Reviewed end-exclusive ranges are `0075FCE0..0075FCFD`,
`0075FD10..0075FD52`, `0075FD60..0075FDA2`, `0075FE50..0075FE6F`,
and `00876D30..00876D60`. Names describe observed behavior; they are not
recovered symbols. The D2-or-46 predicate does not establish a separate type-46
construction or receive-factory path.

The exported writer/reader pseudocode omits their stack argument. Assembly
shows ECX=message, one stack cursor or stream-wrapper argument, and RET4.
The scalar pseudocode's post-free `extraout_EAX` is misleading: `0075FE69`
explicitly restores EAX from the saved receiver after the release call.
The constructor also receives ECX=message, one stack DWORD, and returns the
receiver with RET4. Profile bridges use ignored-EDX fastcall source entries
to support this ECX/stack placement inside the Win32 reconstruction. The
public functions and release-service interface are new C++ interfaces.

## Storage, constructor and live owner selection

`NativeUnitHealthMessage` is exactly 20h bytes. It contains the existing 18h
base, sender WORD at 18h, relay BYTE at 1Ah, retained byte 1Bh, and full DWORD
payload at 1Ch. Bytes 11h..13h and 1Bh remain untouched by the constructor.

`00876D38` calls the existing complete `0075B430` implementation with type D2.
That implementation writes the base header, then reads the actual mutable
game-pointer cell, the signed owner index at game+18ECh, and, for index 0..7,
the borrowed owner pointer at game+18CCh+index*4. Other indices store null as
the original branch specifies. No owner is synthesized or acquired.
The selected game backing must remain readable through those accesses.

The derived constructor then writes sender WORD=0, relay BYTE=0, the complete
incoming DWORD to 1Ch, delivery field 4=1, and the supplied translated D2
profile, in native order. Delivery field 4 is not a reference count.
Payload values such as 12345678h and FFFFFFFFh are retained without truncation.

## Codec and release contracts

The writer loads each actual field immediately before its corresponding
existing cursor service. The DWORD writer is called with width 8: only the
low payload byte is serialized. The reader uses `NativeSessionReadStream`'s
embedded cursor at +4 and calls unsigned BYTE, WORD, Boolean and unsigned
DWORD readers with widths 8, 12, 1 and 8. The last read zero-extends into
the whole DWORD. Unwritten message bytes and stream-wrapper fields stay intact.

These are the existing unchecked native cursor implementations, including
their carry-byte writes and unaligned lookahead reads. The D2 layer adds no
bounds checks, error conversion, allocation or fabricated stream constructor.
The supported source domain supplies live aligned message/wrapper storage,
valid cursor backing and all extra bytes required by reached cursor accesses.
Invalid backing, faults and original exception/return artifacts are not proved.

`NativeUnitHealthMessageProfile` contains five executable source entries and
a required borrowed `NativeUnitHealthMessageRelease` pointer beyond those
entries. This metadata extends the source table; it is not original game
layout. The profile and service must outlive every message using them.

The scalar captures the service while the D2 profile is still installed,
stamps the existing translated root profile, invokes the service only for
flags bit 0, and returns the original identity. Flags 2 alone do not release.
The service must release the exact allocation in its actual owning domain;
there is no default `std::free`, heap selection, arena or inferred allocator.
The service observes the root stamp before release. Returning the original
pointer after release does not make that allocation live again. After any
scalar destruction, no further D2 operation is admitted until reconstruction;
in particular the root profile is not a way to recover the discarded service.

## Setter adapter and lifetimes

`NativeUnitHealthMessageConstructorCalls` stores a borrowed globals view and
profile. Its `bind(fields)` method supplies the setter with that same game
pointer reference. Constructing the binding does not read a session or owner.
The final constructor override default-initializes the POD message in the
caller's exact 20h byte frame with placement new, preserving preexisting bytes,
then invokes the complete constructor and returns its constructed identity.

The adapter does not cache the game-pointer value: provider changes to the
cell after the setter's post-callback mode gate are visible to `0075B430`.
Use `bind` to preserve this shared-cell contract; manually assembling a
different setter binding is outside the adapter's supported use.

The frame is borrowed stack storage. Its identity remains valid only through
the complete router call and must not receive deleting scalar flags. Any
retention, cloning, transport allocation or later message destruction remains
the required router's responsibility. The adapter supplies no such policy and
no fallback implementation of the other pure virtual calls.

## Focused verification and limits

`local/cc11_health_message_d2_check.cmd` compiled the actual source and existing
base/cursor/setter dependencies using MSVC Win32, `/O2 /Gy /W4 /WX /fp:strict`,
and `/link /MANIFEST:EMBED`. The ignored probe passed 71 checks with zero
failures. Its embedded manifest requests `asInvoker`.

The probe covers poisoned retained bytes, owner indices 0/7/-1/8 and a swapped
game cell, full-DWORD construction, an independently calculated unaligned
29-bit wire value, reader write widths, all five executable profile slots,
D2-or-46 matching, scalar flags 0/1/2/3, and release after root stamping.
The fixture's two allocations use explicit Windows Heap allocation/release;
that verifies service dispatch without claiming the original CRT allocator.
The borrowed constructor path checks identical frame identity and shared cell.
One complete setter path changes game A to B in the notification and B to C
in the provider; construction captures C's selected owner even though C's
mode and the receiver's released flag changed after the earlier gate.
No borrowed frame is freed or retained by the fixture router.

Generated assembly confirms the native-width constructor stores, ordered codec
service calls, reader +4 adjustment, release-service capture before root stamp,
and receiver return after release. The adapter's generated code directly
constructs in its supplied frame and calls the existing base constructor.
Exact native direct-call rows are checked separately against live Ghidra.

This is reconstructed, locally build-tested and fixture-tested source behavior.
Full-repository build/integration belongs to the primary. Original allocator
identity, whole-profile binary ABI, native fault/private-EH equivalence,
receive-factory registration, complete network routing and in-game behavior
remain unvalidated. No new tracked test or concrete runtime binding was added.

Primary integration: d314dc6e834a24e974b1a6f8d5aa3ca241aff171; actual main sources independently recompiled for the manifested focused probe, PASS. MSVC Win32 Release and all three existing CTests passed. Executable SHA256 b0b07345ff367f9e0e255a70d44fab8eed4d89f531281d55e463d2f633978cbc. Original ABI, runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_identity_d2_integrated_build.log.
