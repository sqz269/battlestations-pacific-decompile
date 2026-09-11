# Main-menu command owner and help variant

Addresses: 00549620 0054A8D0 0054C050 00549FC0 0054B500 0054A990 0054A960 0054A970 00549570 00549580

Packet `orch5_command_owner`, read-only analysis of `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Descriptive names are hypotheses, not
recovered symbols. The wrappers verified the target before live/export batches.
No Ghidra function, prototype, name, comment or saved project was changed.

## Ownership and coverage

`Game_OnInit` allocates ACh bytes at 004E3E0B, calls 0054A8D0 at 004E3E27,
publishes EAX to 00E1930C at 004E3E30, then invokes virtual+10 at 004E3E40.
The constructor installs main vtable 00CEDFDC, callback owner vtable 00CEDFC4
at+08 and interface vtable 00CEDFA4 at+1C. This identifies the binder as
0054C050, which is not either adjacent event-forwarding routine.

| Routine | Native role and ABI | Coverage |
| --- | --- | --- |
| 0054A8D0 | Constructor, ECX/EAX owner, RET at0054A953 | complete analysis only |
| 0054C050 | Main virtual+10 binder, ECX owner, RET at0054D3BD | complete normal-path semantic reconstruction |
| 00549FC0 | Main virtual+18 enter, ECX owner, RET at0054A0BE | complete normal-path semantic reconstruction |
| 00549570 | Main virtual+1C exit, RET | complete evidenced no-op |
| 00549580 | Main virtual+20 update, RET4 | complete evidenced no-op |
| 0054B500 | Main virtual+24 collector, ECX owner, stack vector, RET4 at0054B520 | complete two-append projection |
| 0054A960 | Main virtual+00 id, MOV EAX,58h; RET | complete |
| 0054A970 | Main virtual+04, XOR AL,AL; RET | complete low-byte return; meaning unnamed |
| 00549620 | Help variant, ECX owner, stack dword, RET4 at00549648 | complete low-byte semantic behavior |
| 0054A990 | Destructor body, ECX owner, RET at0054AA30 | complete normal-path semantic reconstruction |

The constructor's existing `CG_array_ctor_helper_0054a8d0` label describes one
callee rather than the whole routine: it chains 004F7180, initializes callback
owner fields and constructs five eight-byte cached strings at+5C. It does
**not** initialize+20 or either page/widget pointer family. The new C++ owner
defaults provide usable storage; no C++ constructor is claimed for this address.
Correct CRT helper and deleting-destructor names remain unchanged.

Only `MainMenuCommandBarState` stores+04/+05/+20 and cached labels. There is
no second `FrontEndScreen` flag container to synchronize. The owner retains
`GuiLayoutPage*` and borrowed `GuiLayoutWidget*`; `widget_at(offset)`
bridges the previous command host's offset interface. Callback references denote
the native interface at owner+1C, not a reinterpret-cast of the semantic owner.

## Binder and exact widget schema

0054C077 registers screen id58h. 0054C0C7 loads `FE_helpline` into+24;
0054C145 loads `_Buttonhelp` into+28. Both calls are source-order
`(name, screen_flag=1, retain_existing=0)`. Assembly PUSH0/PUSH1/PUSHname
agrees with 00AA5840's body: its third source argument gates the existing-page
reference increment. The second goes to the new screen constructor.

After hiding the two page roots in order, 0054C1D1 finds `button_FrameBox`
under+28 and 0054C1DE hides it. The direct-child lookup00AA7E00 consumes its
second stack argument1 but never reads it; it does not recurse.

| Call site | Owner field | Parent field | Widget |
| --- | --- | --- | --- |
| 0054C24E | +2C | +24 | content_helpline_Text |
| 0054C2C4 | +30 | +24 | content_helpline2_Text |
| 0054C33A | +34 | +28 | CenterText_Text |
| 0054C3B0 | +48 | +28 | CenterIcon_Text |
| 0054C426 | +38 | +28 | Left1Text_Text |
| 0054C49C | +4C | +28 | Left1Icon_Text |
| 0054C512 | +3C | +28 | Right1Text_Text |
| 0054C588 | +50 | +28 | Right1Icon_Text |
| 0054C5FE | +40 | +28 | Left2Text_Text |
| 0054C674 | +54 | +28 | Left2Icon_Text |
| 0054C6EA | +44 | +28 | Right2Text_Text |
| 0054C760 | +58 | +28 | Right2Icon_Text |
| 0054C7DC | +84 | +28 | CenterText_pc_Text |
| 0054C851 | +88 | +28 | Left1Text_pc_Text |
| 0054C8CA | +8C | +28 | Right1Text_pc_Text |
| 0054C943 | +90 | +28 | Left2Text_pc_Text |
| 0054C9BC | +94 | +28 | Right2Text_pc_Text |
| 0054CA35 | +98 | +28 | Center_pc_FrameBox |
| 0054CAAE | +9C | +28 | Left1_pc_FrameBox |
| 0054CB27 | +A0 | +28 | Right1_pc_FrameBox |
| 0054CBA0 | +A4 | +28 | Left2_pc_FrameBox |
| 0054CC19 | +A8 | +28 | Right2_pc_FrameBox |

These writes establish slot0 Center, slot1 Left1, slot2 Right1, slot3 Left2 and
slot4 Right2. They refine the placement names in MAIN_MENU_COMMAND_BAR without
inventing absolute geometry or a screen-space unit.

0054CC4F..0054CCA5 walks five pairs: first the+34 label, then the+84 PC label.
Each gets Multiline(+FC)=0 and FontScale(+1D8)=0.75 before calling00ABBE50
with a single ASCII space and localize1. These fields reuse the producer's
`GuiTextWidget` schema. ESI is defined at0054C7D6 as owner+84 and has no
intervening writes before this loop; [ESI-50h] is consequently owner+34.
EDI remains the screen throughout. The full filtered ESI/EDI/EBX/EBP listing
was reviewed, including the later changes for string temporaries.

0054CCA7..0054CE57 concatenates nine one-byte strings into
`A2 A3 A5 A7 B4 B6 B7 B8 B1`, then 004C5E60 widens each unsigned byte.
The result is a UTF-16 code list, not UTF-8 decoding. At0054D129, EBP is
owner+1C and ESI is owner+48. For each of five glyphs:

1. 0054D13D calls00531130 with code list, owner+1C and glyph itself. The body
   copies the list to+1A4 and stores the two pointers at+1AC/+1B0; RET0Ch at
   00531188, 005311A2 and 005311BC establishes all three stack arguments.
2. 0054D148 calls00AA6BC0 on the glyph with null callback and flag0.
3. 0054D153 applies owner+1C/flag0 to the corresponding+98 framebox.
4. 0054D15E applies owner+1C/flag0 to the corresponding+84 PC text.

00AA6BC0's body stores the pointer at widget+DC and byte at+79, RET8 at
00AA6BD1. Their dispatch behavior is a required callback host contract.

0054D2B9 invokes the existing bar rebuild with five `(0,empty,1)` triples.
The callee's RET3Ch at0054C045 consumes all fifteen dwords. The new binder
calls the previous implementation directly, preserving its duplicate hides,
glyph-child clears and empty-plan early return. It then writes help variant0
at0054D37F; it does not call the help setter or clear cached labels here.

## Help variant and enter behavior

00549620 shows+2C when the input low byte is zero, then shows+30 when nonzero,
then stores the original low byte to+20. Noncanonical bytes survive the store.
A callback changing+20 during either visibility call is overwritten. There is
no entry gate or text write in this routine.

All seven direct sites were inspected and their containing functions verified:

| Call site | Containing function | Input low byte |
| --- | --- | --- |
| 004DAFD9 | 004DA780 Game teardown | 0; complete EBX listing, XOR at004DA7C2 |
| 00598B29 | 005987F0 Main menu enter | 0 |
| 005E5950 | 005E51D0 | 0; complete EBX listing, XOR at005E51F6 |
| 005F2535 | 005F24D0 Options settings | 1 |
| 005F3BB6 | 005F3B50 Options content | 1 |
| 005F5535 | 005F54F0 Options main list | game00E188A8+61F |
| 005F7039 | 005F7020 Options exit | 0, only when game+61F is zero |

The PUSH EDI immediately preceding PUSH EDX at005F5535 saves a register:
RET4 confirms one argument, and CL loaded from game+61F is copied into the
low byte of that dword at005F5525. Its upper bytes are incidental.

The main screen's enter virtual00549FC0 independently clears+2C then+30 via
00ABBE50 with localize0. For each+34 label it reads game+61F, chooses
black/white RGB with alpha1, applies text virtual+50, reloads game+61F,
configures00AB6C30 with the **raw** enable byte, position0, offset0.05 and
black shadow alpha(enable!=0), then applies the cached label with localize1.
The existing `GuiTextWidget` schema establishes+15C..+174 as shadow fields.
The implementation accepts a live byte reference and reloads after callbacks.
It never equates game+61F to global00F88A30 or modifies+20 here.

## Page collection and destruction

0054B500 appends+24 then+28 to the supplied vector; it does not clear it or
filter null entries. Calls at0054B50E/0054B519 target the checked vector push
004D6790. Thus the existing visibility commit004F83B0 can apply screen+05 to
both pages.

0054A990 obtains the manager and removes/destroys+24 at0054A9D9, then obtains
it again and removes/destroys+28 at0054A9E9. 00AA31F0's body removes the
manager-vector entry and invokes page virtual+20 then deleting virtual+04(1);
this is direct destruction, not reference-count decrement. Cached strings are
destroyed in reverse order by the correct CRT vector destructor at0054AA00.
Callback owner+08 is destroyed at0054AA0C, then004F71A0 removes every registry
slot referring to the screen at0054AA1B. The enclosing allocation is released
by the separate deleting wrapper and is outside this function.

The report contains every external host call's `address`, `native` and
containing `function`, including loop sites. Virtual targets remain marked
indirect. Host verbs were chosen only after inspecting their callee bodies.

## Verification and boundaries

This packet exports/reconstructs the above normal paths and records raw leaf
bodies. It introduces no test cases. The primary owns normal CMake registration
and the combined MSVC Win32 build; no baseline build was repeated here.
The parent's strengthened call verifier checked 55 direct call rows with zero
failures; seven virtual-call rows were retained as explicitly indirect.

The executable in this worktree does not bind the new owner host, so this packet
makes no frame-order, gameplay or visual claim. GUI text companions, scene-node
shadow operations, page management and callback forwarding require real hosts.
No empty implementation was supplied for those boundaries. A missing required
page/widget produces a new diagnostic exception, outside native failure-path
coverage. Native pooled strings, vector layout, SEH and callback/vtable ABI are
not reproduced. Neighboring STL helpers and event forwarding are not ported.

## no_ghidra_function

| Start | Final instruction | Length | Inclusive final byte |
| --- | --- | --- | --- |
| 0054A960 | 0054A965 RET | 1 | 0054A965 |
| 0054A970 | 0054A972 RET | 1 | 0054A972 |
| 00549570 | 00549570 RET | 1 | 00549570 |
| 00549580 | 00549580 RET4 | 3 | 00549582 |

Ghidra proto returned no body for these four addresses. Raw disk bytes and
00CEDFDC's entries establish the independent leaves; nearby candidate names
must not be used as containing-function attribution.
