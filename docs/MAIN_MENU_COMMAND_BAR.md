# Main-menu command bar and separate help line

Addresses: 0054B530 0054A0C0 005495A0 005495E0 00531030 0054AC10 0054AA70 00AB6C30 00ABB000 0058C010 005922F0

Packet `orch5_main_menu_command_bar`, read against the existing `bsp.gpr`,
`/battlestationspacific.exe`. All Ghidra work in this packet was read-only.
Descriptive names are hypotheses, not recovered symbols.

`0054B530` rebuilds five command slots; `0054A0C0` changes a separate pair of
help-text widgets. The mission-detail footer's fifteen dwords are five triples.
The grouping and sequence below come from the assembly: the current decompiler
incorrectly treats the first command as `unaff_retaddr`, misidentifies later
stack locals, and gives `0054A0C0` another spurious return-address input.

## Arguments and placement

`0054B530` takes ECX as the command-bar screen and fifteen stack dwords, with
`RET 3Ch` at `0054C045` (three-byte instruction, body ends `0054C047`). Its SEH
prologue plus saved registers subtracts CCh from entry ESP. Consequently
`[ESP+D0h]` is the first argument, not the return address.

| Triple | Entry ESP offsets | Body command/label/placement reads |
| --- | --- | --- |
| 0 | +04 / +08 / +0C | +D0 / +D4 / +D8 |
| 1 | +10 / +14 / +18 | +DC / +E0 / +E4 |
| 2 | +1C / +20 / +24 | +E8 / +EC / +F0 |
| 3 | +28 / +2C / +30 | +F4 / +F8 / +FC |
| 4 | +34 / +38 / +3C | +100 / +104 / +108 |

The command consumes only the low byte. Zero omits the entire triple, including
its label and placement. Labels are pointers to eight-byte native length/data
strings, copied into a temporary vector. Placement consumes the complete dword.
The byte, string and dword vectors compact together in source argument order.
`00450540`, `0054B2E0`, and `0054B470` are storage contracts, not additional menu
operations to invent.

After compaction, default slots come from `00CEDFF4 + count*14h`, then one
dword per compacted ordinal. The address before the first actual row overlaps
a preceding vtable; that does not make the row values function pointers.

| Active count | Default slot sequence |
| --- | --- |
| 1 | 0 |
| 2 | 1, 2 |
| 3 | 1, 0, 2 |
| 4 | 3, 1, 2, 4 |
| 5 | 3, 1, 0, 2, 4 |

At `0054B994..0054B9C1`, placement 0 overrides the table with slot 3, placement
2 with slot 4, and every other value retains the default. Overrides do not
remove entries from the count or advance the table differently. Duplicate slots
are possible and processed in order; labels can overwrite earlier cached labels.
The geometry of the slots is not recovered here, so 0/2 are not named left/right.

The mission-detail call `0058CC1F` is, in source argument order:

```
(A2, "globals.continue", 0)
(00, "",                 1)
(00, "",                 1)
(A3, "globals.back",     2)
(00, "",                 1)
```

Thus Continue occupies slot 3 and Back slot 4. The negative pushes -5Eh/-5Dh
are the same low bytes A2h/A3h. In contrast, `0059244C` supplies five
`(00, "", 1)` triples and takes the empty-plan path.

## Observable sequence

The screen's relevant field families are five pointers each: labels +34h,
glyph text +48h, alternate labels +84h, and alternate backgrounds +98h. The
five persistent label strings are +5Ch with eight-byte stride. The page root
is +28h; the separate help-text handles are +2Ch/+30h.

1. `0054B555` calls `005495E0`, which hides each +84h/+98h pair in that order.
2. `0054B55C` calls `005495A0`, hiding each +34h/+48h pair in that order.
3. `0054B5A7` finds `button_FrameBox` under +28h with required=1, then calls
   its virtual +34h with `00E198C4 != 0`. The word's broader meaning is open.
4. `0054B5F1..0054B613` repeats the +34h/+48h hides, then calls `00AB80C0`
   on each glyph text. These duplicate visibility calls are retained.
5. Compact the supplied triples. No active commands means cleanup and return:
   no enter calls, no help-text writes, and no persistent label-cache clearing.
6. If screen byte +5 is zero, set +4/+5 to 1, call `004F83B0`, then screen
   virtual +18h. Snapshot the display-mode byte at `00F88A30` afterwards.
7. For each active command, choose its slot and show the glyph widget. Construct
   its glyph byte string, then clear its existing source through `00ABBE50`
   with empty C string and flag=1 before the display-mode branch.
8. With mode nonzero, configure the glyph auxiliary through `00AB6C30`
   `(1, 0, 0.05f, [0,0,0,1])`, set its source through `00ABAED0` with flag=1,
   show the +34h label, call `00ABB000(label,-1.0f,1)`, and cache that label.
9. With mode zero, first hide the glyph again. Case-insensitive labels
   `globals.navigate`, `globals.scroll_menu`, and `globals.change` skip the
   remainder of this entry. Every other label still gets the same auxiliary
   configuration and glyph-source call, despite the glyph being hidden, then
   uses +84h for the label and cache update. Finally show +98h, get its size,
   read the +84h text's +114h width, and call background virtual +58h.

The glyph string is literal bytes, not a lookup of an input action. `00531030`
constructs a one-byte pooled string. `0054AC10` copies and appends a byte through
`0054AA70`. Command 9 is replaced with `B4 2F B6`; command 1 with `B7 2F B8`;
all others remain their original byte. The discarded initial one-byte temporary
and intermediate allocations are omitted from the semantic projection.

Background width is `float(double(float(double(text_width)/960.0)) + padding)`;
height remains the prior height. The explicit float store at `0054BF45` precedes
the second x87 operation. Double `00CEC380` is 960.0 and double `00CE4D68` is
0.03999999910593033, with bytes `00 00 00 40 E1 7A A4 3F`. No clamping appears.

## Help line

`0054A0C0..0054A12A` is ECX=this, one native-string pointer stack argument,
`RET 4` at `0054A128`. It uses the same +4/+5 entry gate, then sets +2Ch and
+30h sources through `00ABAED0(source,1)`, in that order. Only after both calls
does it capture +20h, show +2Ch for zero and +30h for nonzero, then restore the
captured byte after both virtual calls. It preserves noncanonical nonzero values;
reentrant writes to +20h during those visibility calls are overwritten.

The native virtual calls pass dwords with only the low byte deliberately
prepared. The reconstruction exposes booleans; it does not reproduce incidental
upper argument bytes. Neither routine reconstructs the screen owner or GUI ABI.

## Integration corrections and limits

- `docs/MAIN_MENU_MISSION_DETAIL.md` and its header can replace the unresolved
  grouping with the five triples above. Its Continue/Back labels remain correct.
- `MissionBriefingPlayHost::clear_help_line` at `0059244C` is incorrectly named:
  the call clears the command bar, retaining the separate help texts and cached
  labels. Rename the host method and call-site row to `clear_command_bar` when
  those files are available to the integrator.
- Four functions are reconstructed in `src/main_menu_command_bar.cpp`: the
  rebuild, help setter, and two family resets. Standard strings and arrays
  replace temporary native pooled strings and STL vectors. Native SEH, failure
  paths, original-layout aliasing and register/stack upper bytes are not modeled.
- `00AB6C30` is a required external auxiliary-state setter; `00ABB000` retains
  the repository's existing localization/ellipsis contract. No scene or font
  implementation was duplicated. Display mode is named for observed behavior;
  interpreting it as a specific input-device type needs separate evidence.
- Source is intentionally unregistered in this worker commit. The primary owns
  source registration and the standard Win32 build. No build, fixture, native
  differential, runtime or visual equivalence is claimed by this packet.

## Follow-up packets

- Recover the command-bar owner/binder and +20h variant transitions; establish
  widget names and the geometric meaning of slots and the two global gates.
- Bind the semantic host to reconstructed GUI objects after their contracts are
  ready, preserving repeated visibility calls and cache lifetime.

## no_ghidra_function

None. All routines named here already have Ghidra functions. The repaired
`0054B530` export contained 845 instructions and no listing gaps.
