# Main-menu Listbox activation

Addresses: 00598B60, 00585810, 00585B40, 005E6F70.

`main_menu_listbox_current04_00598b60` reconstructs the full normal caller
sequence using the existing command listener's screen, canonical widget owner,
Listbox, profile/checkpoint services, native strings and prompt. The new binding
adds references to the same selection globals and tactical-library binding;
it owns no replacement menu state. Unrecovered session setup and page-builder
providers remain mandatory. This is a new C++ interface, not an original binary
ABI replacement or proof of gameplay behavior. Ghidra was read-only throughout.

| Routine | Native ABI/body | Coverage |
| --- | --- | --- |
| 00598B60 | ECX=screen+8, selected-row/Listbox stack, RET8; 598B60..599331 | complete normal caller; 608 instructions, zero flow gaps |
| 00585810 | CL=check multiplayer privilege, RET, AL; 585810..585B3D | complete normal caller with required 4BB600 predicate |
| 00585B40 | incoming registers unread, RET, AL; 585B40..585C47 | complete normal caller with required session current14 |
| 005E6F70 | incoming ECX unread, RET, AL; 5E6F70..5E6FAA | complete normal caller with required session current1A0 |

## Listener ABI and page dispatch

The constructor installs CEFC48 at screen+8; CEFC4C contains598B60. At598B8A
EDI captures ECX;598B8E and598FBF subtract8 before screen methods. Neither
stack argument is read anywhere in the complete body. Every selection query
uses `[EDI+1B0]`, the screen's current1B8. The public interface retains both
callback operands without using them as substitutes for the current screen
slot. A9C990 returns D8 data bits, including FFFFFFFF for no selection, rather
than a list ordinal. Unexpected page/selection values fall through unchanged.

| Page | Selection publication and dispatch |
| --- | --- |
| 1 | Same command service588A80 |
| 2 | D8 to E194C8; ids4,0,1,2,3 call597870 with pages8,4,5,6,7; id5 runs5E6F70 |
| 3 | D8 to E194CC; id1 calls5E74B0; id0 requires585B40 then585810(true), then5E76D0 |
| 4..7 | D8 to E194DC, then E08878, followed by checkpoint sequence |
| 8 | D8 to E194DC, then E08878, then existing5922F0 |
| 11 | D8 to E194D0, then extras dispatch below |
| 12 | D8 to E194D4 only |

For extras ids3,1,4,2, push interface0B with null payload through the existing
4CC460 body, then freshly resolve the current manager and screen+70 and store
mode94=3,2,0,1 respectively, selector98=99. Id1 first clears the current screen's
mission9C, writes sideA0=2 and flagA4=0, before the request. This ordering permits
the payload callbacks to replace the manager or tactical screen. It also keeps
the field stores when4CC460 rejects a locked request, as native does. Id5
requires585B40 then585810(false) before interface0A; id6 requests8; id0 invokes
the existing5886C0 tactical-library service. No extra selection fields are
changed in those arms.

ESI=2 at598C30 is retained through the extras arm, proving the mode and side
stores at598E23/45. EBX is zeroed at598E1A or598E75 for their respective arms.
On the campaign arm EBX=0 at598F9E, ESI=screen at598FBF, and EBP captures each
profile owner. ESI becomes the composed message only on terminal prompt arms.
The campaign store `[EDI+55C]` is screen564 (`screen.us_campaign`), not565.
The no-checkpoint store `[EDI+54]` is the same screen5C reference.

## Checkpoints and actual native strings

The campaign page is reloaded after the selection query and publications.
Pages5/7 set screen564. For each of7F8D60,7FC490,7FC370 the current game+650
profile is captured before calling the existing selected-mission provider.
No checkpoint clears screen5C then invokes58C010. Otherwise7FC490 constructs
the existing14h temporary (three words and NativeString at0C). If7FC370
succeeds, the temporary's class word+8 and then current screen564 supply
584750's two operands. A false7FC370 skips the unlock call.

| Condition | Native message construction | Callback |
| --- | --- | --- |
| available and unlocked | `globals.checkpoint_available` | 592AD0 |
| available and locked | `globals.checkpoint_unit_locked\u007c.\n\u007c.` + current checkpoint text + `\u007c.\n\u007cglobals.continue_without_checkpoint` | 58F540 |
| unavailable | `globals.checkpoint_unit_notavailable\u007c.\n\u007c.` + current checkpoint text + same suffix | 58F540 |

The `\u007c` notation in this table denotes a literal pipe. Bytes were checked
at CEFE6C/CEFE98/CEFEBC/CEFEE4; no localization result is fabricated. The
43C130 prefix-string constructor,4261A0 concatenation and prefix destructor
are sequenced explicitly before the outer4261A0. The existing NativeString
headers/storage are used throughout, including the owned empty countdown
argument.531B00 has RET2C: ten logical arguments include one by-value8h header.
All checkpoint prompts use slot5, YesNo1, unused0, empty title, timeout+0,
result0 and dismissible1. Combined, prefixed, suffix and title strings are
destroyed in that order. A fresh425D10 then530C20 follows on **every** arm,
including the available/unlocked arm. The checkpoint text is destroyed last.
Unlike keyboard command5993A0, this listener never invokes530670.

## Connection, account and session helpers

585B40 resolves the actual session from F8A2FC and calls its current14 slot.
False raises `FE.multi_cablelost_live`, then returns false.585810 captures CL,
reads the current sign-in owner's11A via the existing live-enabled getter,
then reloads the owner for119. When live-enabled is true, the selected slot's
state is read from that same captured owner. No selected user or state other
than2 raises `FE_pc.xsm_requireslive`. If CL is false, the valid LIVE account
succeeds immediately; if true, the required4BB600 predicate must pass or
`FE_pc.xsm_requiresmultiprivilege` is raised. When11A is false,119 distinguishes
`FE_pc.xsm_requiresmembership` from `FE_xbox.xsm_requiresprofile`.

All these information prompts use the existing531B00 at slot2, Accept2,
callback0, unused0, empty title/countdown, timeout+0, result0, dismissible1.
Message then title are destroyed; there is no navigation-focus call. The
existing OnlineSignInState must be the actual current sign-in state; an
out-of-range selected index is outside the native domain and throws instead
of using the older getter's synthetic signed-out fallback. Neither helper
supplies a successful default for an unimplemented session/account service.

All five585810 call sites were read:565B1C,588B85,598F05,5F7D3D use CL=0;
598D7E uses CL=1. All five585B40 sites take no arguments. The only5E6F70
caller is598D10; it loads `[E198B4]+40`, but5E6F70 never reads incoming ECX.
5E6F70 calls current session1A0(true), then sets fresh game218C=1 and resolves
the actual existing GameStateRequestQueue separately for enqueues6 and7.
4D7920 is the read wrapper over4D3ED0, so the existing deque implementation
is reused directly. AL=1 is returned after both enqueues.

## Required dependencies and evidence limits

| Provider | Callee evidence and coverage |
| --- | --- |
| Existing command services | Same actual Listbox, profile/checkpoint/selected mission, unlock, prompt,588A80,58C010,5922F0,5886C0; their individual provider gaps remain as documented in MAIN_MENU_COMMAND_LISTENER.md |
| 597870(page) | Body read through initial page store and existing page evidence; ECX screen, RET4; the full 597870..59879D builder remains unreconstructed here |
| 5E74B0 | Full 5E74B0..5E76C5 body/listing read; network current14 gate, current1A0(true), current4D=0, currentD4(1), captured-vtableE0 with4BC830, current80 player-list head, profile48/4C/58/59 and display-name stores, state6/7; implementation remains required |
| 5E76D0 | Full 5E76D0..5E78F2 body/listing read; selected LIVE gate, current1A0(false),D8(1),38(),80(), first player id/name/flag, A3EB30, profile writes and state6/7; implementation remains required |
| Session current14/current1A0 | Calling ABI read at585B63/5E6F80; native target identities and complete session implementation unresolved. Required slot-addressed methods, no semantic guess |
| 4BB600 | Full body/listing read: same captured F8ABE8 for119/state==2, fresh F8ABE8 -> A3EB20 byte118+index; required predicate on current owners |
| Sign-in/game/tactical storage accessors | Pure association resolvers to current actual existing owners; they must preserve owner lifetimes across callbacks and never manufacture state |

The two session setup bodies are not replaced with no-ops or a name-only
success shell. Reconstructing them requires the producer/layout and complete
contracts for the native session virtuals and player list, outside this bounded
packet. No CRT/STL/Lua/render/VFS implementation is introduced. Native SEH,
debug traps, allocation-failure unwinding and original object/vtable ABI are
not claimed. No Ghidra functions were renamed, created, mutated or saved.

Validation: `scripts/build.ps1` passed for Release Win32 with MSVC19.51/v145,
`/W4 /WX /fp:strict`. The existing `reconstructed_math` test passed (1/1); it
does not exercise this activation listener. No permanent tests were added.
The report carries all118 CALL instructions in the four reconstructed bodies:
`verify_report_calls.py` checked116 direct call rows with zero failures and
explicitly skipped two unresolved session virtuals. The ignored worker CMake
cache reuses the existing parent dependency sources. This packet is not wired
into the executable's menu host, so it makes no frame/runtime claim.
