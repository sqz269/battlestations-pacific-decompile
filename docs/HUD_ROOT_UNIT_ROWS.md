# HUD root selection and unit rows

Addresses: 00644C20, 00644CC0, 00644DB0, 00648C20, 00647080.

Packet `orch2_hud_root_unit_rows`. Source: `include/bsp/hud_root_rows.hpp`,
`src/hud_root_rows.cpp`; structured evidence: `reports/hud_root_unit_rows.json`.
Names in this packet are hypotheses, not recovered symbols.

The five functions are exported, read from their native listings, and reconstructed
as bounded C++ policy plus mandatory host interfaces. The MSVC Win32 Release build
passes. Existing `reconstructed_math` and `native_math_differential` checks pass;
they do not exercise these new HUD contracts. No new test was added. No native HUD
fixture, ABI compatibility, in-game execution, or visual correctness is claimed.

## Scope corrections to the earlier packet

00644DB0 handles selection input, not row widgets. Its inclusive byte span is
00644DB0..0064505B (684 bytes), rather than the older document's 1195-byte figure.
00648C20 is 3104 bytes, inclusive 00648C20..0064983F. The two native tuple fields
at root+C2h/C4h are a list discriminator and an index, not weapon information.

None of these five functions writes `Medal_Icon` or `Medal_Text`. Layout function
006463E0 binds them at root+5Ch and root+60h respectively. The three roots used by
00648C20 for icon state changes are `unit_command_Icon` (+84h),
`unit_payload_Icon` (+50h), and the health widths (+7Ch/+80h). Medal behavior
remains outside this packet. Root enter 006488D0's existing zero-scalar writes to
+5Ch/+60h are documented in `HUD_SCREEN_PAGES.md`; their dynamic writers were
not traced here.

The 00648C20 decompiler loses the payload state in its stack aliases and prints
a constant 3 at the tail. Assembly stores 3 at 006492DD, 0 for kind 1Ch at
00649516, or EAX from 00653200 at 00649561; 006497F2 loads that same stack slot
and 00649802 pushes EAX to widget +50h virtual +88h. The implementation preserves
all three branches.

## Native ABI and boundaries

| Entry | Last instruction | Bytes | Native contract |
| --- | --- | --- | --- |
| 00644C20 | 00644CB0, `RET 8` (3 bytes) | 147 | ECX=root; stack result pointer, unit; EAX=result pointer |
| 00644CC0 | 00644DAD, `RET 8` (3 bytes) | 240 | Same two arguments and return convention |
| 00644DB0 | 00645059, `RET 4` (3 bytes) | 684 | ECX=root; one unused four-byte stack argument |
| 00648C20 | 0064983F, `RET` (1 byte) | 3104 | ECX=root; no stack arguments; no meaningful result |
| 00647080 | 0064715B, `RET` (1 byte) | 220 | ECX=root; no stack arguments; no meaningful result |

All five already exist as Ghidra functions. `ghidra flow` reports respectively
56, 93, 180, 885 and 68 listed instructions, with zero gaps. No missing definition
or flow repair was requested. Each Ghidra command/export verifies project `bsp`
and program `/battlestationspacific.exe` through the tool's `Client.verify()`.
Ghidra was read-only throughout; proposed names exist only in the repository
ledger, pending integrator annotation. Existing library names were retained.

## Selection tuple and input sequence

The logical tuple is byte discriminator +0 and signed word index +2. Byte +1 is
uninterpreted padding: 00644C20 does not write it; 00644CC0 copies a stack value
whose padding originated in saved ECX. It is excluded from the value projection.
FFFFh denotes no match. Root+90h/+94h are the first/end pointers of the primary
list, and +A0h/+A4h the secondary list. Stable, valid list views are required; no
CRT vector checks or allocation internals are replaced. Native signed BX lookup
indices stop at 8000h even if a larger valid list is supplied.

00644C20 maps a member to the leader and searches the primary list. The member
test is 007788B0, which observes unit+284h and compares [that+14h] with the unit;
007788D0 returns [that+14h]. 00644CC0 delegates a leader (00778890) to this primary
lookup. Otherwise it searches only members in the secondary list. A found
secondary member sets the discriminator only when the secondary count exceeds
one; a singleton still returns index zero with the discriminator clear.

00644DB0 polls actions 8Dh, 8Ch, 8Eh, 8Fh in that order into +EBh,+EAh,+E8h,+E9h.
Base screen active AND panel [manager+54h]+5 clear AND controlled unit present
clears all four edges. Primary cycling runs for count >1 or a controlled unit
whose virtual +124h is false. The next edge wins over previous. If currently
secondary, 00644A60 resolves the selected unit before the primary lookup. The
index increments/decrements as a word; forward wraps on the unsigned comparison
of the sign-extended result, backward only when the result is FFFFh. The primary
path sets discriminator zero and pending byte +C0h to one.

Secondary cycling requires count >1 and pending clear. Next wins over previous;
entering from primary starts at 1 on next, count-1 on previous. Reaching secondary
index zero maps secondary[0] through the primary lookup instead of leaving it
selected as a secondary entry. Every taken secondary path sets pending. The
routine never clears an already-set pending byte, and does not use its delta
argument. The host's `selected_unit()` observes the same current selection state.

## Row update 00648C20

First call 00648290 rebuilds the lists. Prefer controlled unit 00E188D8, otherwise
00644A60's selection; active panel +74h can override this through its nonnull +20h
object's nonnull +9D4h unit. A kind 18h unit with no first member at +3D0h returns
immediately, before caches or payload state change. Otherwise panel +54h active
skips only the weapon-source block. Null unit skips all unit/name/health updates,
writes null to cached unit +6Ch, and still selects default payload state 3.

Weapon-source rules (00648CBD..006491C5):

| Branch | Source |
| --- | --- |
| Default; surface selector inactive or mode 0/outside 0..5 | `ingame.selector_noweapon` |
| Surface mode 1 / 2 / 3 / 5 | `ingame.selector_aagun` / `ingame.selector_aaflak` / `ingame.selector_atrillery` / `ingame.selector_depthcharge` |
| Surface mode 4, not kind 6 | `ingame.selector_torpedo` |
| Surface mode 4, kind 6 | counted torpedo using 00815850(unit) |
| Kind 18h or 0Fh, panel +6Ch active and its +BDh set | first true probe: bomb 007B9320, torpedo 007B93F0, depth charge 007B94F0, rocket 007B9400; every probe takes 0 |
| Same kinds, gate false or all probes false | machinegun key if effective unit+C24h set; otherwise literal `.` |

For kind 18h the effective weapon unit is +3D0h. The first three counted probes
use 007C1DB0; rocket uses 007C1DE0. The exact counted source is
`.` + signed decimal count + `x |` + localization key. Native DAT_00CE3A70 is
bytes `2E 00`, not an empty string. 004263B0 copies that prefix and appends
004260B0's `%d` result; 004261A0 appends the suffix. The misspelling `atrillery`
is also literal native data. String allocation, formatting, localization and
cache storage stay in the host; the module supplies their source recipe.

When the selected unit changes or 00449AF0 reports a case-insensitive source
difference, kinds 45h/46h stop animation slot zero and apply scalar zero to
WeaponInfo (+3Ch). Others apply scalar one; unless the panel +6Ch gate above was
true, obtain animation slot zero (00AA8B00), call 00AC2F20(0,1), and write one to
animation+4h. The panel gate remains true even when all four probes fail and the
machinegun fallback is used. Source submission (00ABAED0, flag 1) and the cache
copy to root+70h happen on every nonsuppressed source update. Numeric scalar and
animation meanings are not further inferred here.

Then update cached unit +6Ch. Kind 18h's first member or kind 0Fh unit can force
command icon state 2 via 00927F30(1). Otherwise virtual +114h and 0071BE40 supply
the descriptor identity for the exact table in `hud_root_command_icon_state`:
0E08F10h..0E08F58h at stride 8 and 0E08F78h -> 3; 0E08F60h,68h,80h -> 1;
0E08F70h -> 2; 0E08FA0h -> 4; everything else -> 0.

Unit virtual +14h supplies the name for +34h through 00ABB000(source,-1,1).
FlagJP (+64h) versus FlagUS (+68h) uses the **controlled** unit's +54h even when
the displayed unit was overridden: nonzero shows JP; zero shows US. Hide the
other flag first. Payload state is zero for kind 1Ch, otherwise the result of
00653200 on displayed unit+538h, or first member+538h for kind 18h. This class
mapping remains a host dependency; it is not a medal update.

Health selects first member for kind 18h. Kind 45h writes 006D2560's result to
damage width (+7Ch). Otherwise kind 5 shows that widget and writes health from
00923BE0. A non-kind-6 then writes health to healthy width (+80h) and resets
damage width to zero, in that order. Other kinds leave the health widgets alone.

For kind 6, the four metrics are read at selected unit+A20h through 00939F70,
00939F80,00939FC0,00939FB0, each spilling ST0 to float. These fields come from
the selected unit, whereas health still comes from the effective health unit.
Let A/B/C/D be those four results in order. Let F1/F2 be separate calls to
00470440(3, selected unit). S1 is tuning+3C8h for selected+A44h==4, else 1;
S2 is tuning+3CCh for ==3, else 1. The width is:

`float(health - ((D*B)/float(F1*S1) + (C*A)/float(F2*S2)) / selected[36Ch])`.

The calculation precedes the zero-metric branch. Both A and B ordered-equal zero
cause a fresh health call and unconditional healthy-width write, without changing
the +88h cache. Otherwise update the healthy width only if it differs from the
cache, then always store the computed width. NaNs compare unequal and trigger
the write. No clamp or division guards exist. C++ preserves explicit float
spills and arithmetic order; MSVC's 64-bit `long double` does not reproduce all
80-bit x87 intermediates or floating exception timing. This is a documented
numerical reconstruction, not native bit parity.

Finally select the payload widget state with arguments (state,0,1). Panel +54h
byte +4 set then stops weapon animation zero and applies scalar zero again.

## Closed rows 00647080

Only root+1Ch equal to zero or one has a transition. From zero, interface 22h
tests first member's kinds 10h and 16h; either sets +1Ch=1 and requests interface
23h with first-member payload. Interface 23h requires controlled+379h clear and
controlled+1A8h equal to 9 or the local team [game+18ECh], then sets +1Ch=1 and
requests 22h with the same payload. From exactly one, clear +1Ch and request the
saved interface +CCh only if saved payload +E4h is nonnull. Other state values
are unchanged. These transitions preserve writes before request calls. The
native caller, not this function, supplies the valid controlled unit/member.

## Remaining work and validation limits

Mandatory hosts retain list reconstruction, selected-unit resolution, group
probes, unit/weapon/health internals, class icon mapping, text/localization,
animation and widget calls. No stand-in implementations were added. Existing
HUD state types were inspected; this module projects additional fields and
uses the existing convention of numeric widget/unit tokens. It does not change
the old `HudRootUpdateHost` integration boundary or alias its misleading
`weapon_info_field` name into this new tuple type.

The five seed functions need integrator-reviewed Ghidra names/comments. Medal
dynamic writers and gameplay identity names for health metrics remain open.
Validation used native listings, small raw-data reads, zero-gap checks, all eight
installed seed-byte comparisons, and the two existing build checks. The seed
byte and differential math checks are infrastructure checks, not HUD proof.
