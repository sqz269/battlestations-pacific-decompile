# Canonical Listbox row state, selection and layout

The companion in `gui_listbox_runtime.hpp/.cpp` uses the existing
`GuiWidgetOwner` and its actual `GuiLayoutWidget`. Its separate FC row list
contains borrowed pointers to those owners, as the original Listbox does.
It does not copy a menu list, widget tree, selection index or sound queue.

Evidence was read from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Descriptive names are hypotheses. The new C++
interface is not a raw180h object, native checked-STL ABI, or game replacement.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| A9DF40 constructor | ECX storage, EAX this, RET | Partial: FC list/selection/cache/listener and declared layout fields only; remaining derived fields, base call, pool/SEH ABI excluded |
| A9AC40 listener setter | ECX owner, pointer stack, RET4 | Complete normal body |
| A9AC90 three layout flags | ECX owner, three raw low bytes stack, RETC | Complete normal body |
| 425E50 selected row | ECX owner, EAX row/null, RET | Complete valid iterator domain; existing STL name retained |
| A9C990 selected data | ECX owner, EAX row+D8/FFFFFFFF, RET | Complete valid iterator domain |
| A9C920 selected ordinal | ECX owner, EAX index/-1, RET | Complete valid iterator domain |
| A9BE00 row at ordinal | ECX owner, ordinal stack, EAX row/null, RET4 | Complete valid iterator domain; existing STL name retained |
| A9C7C0 select ordinal | ECX owner, ordinal stack, RET4 | Complete normal body |
| A9C740 select row | ECX owner, row pointer stack, RET4 | Complete normal body |
| A9C220 current80 | ECX owner, force low byte stack, RET4 | Complete normal body with required actual listener/current sound binding |
| 941250 GUI sound requests | CL first flag, DL priority flag, RET | Complete body over same `SoundRequestQueue` singleton getter |
| A9D750 insert row | ECX owner, row/iterator/after stack, RETC | Partial: null-position A9D750..A9D7AB plus A9D813..A9D863; A9D7AC..A9D812 non-null insertion excluded |
| A9BE60 remove row | ECX owner, row stack, RET4 | Complete normal body; C++ list transport replaces native node allocator |
| A9C0A0 current7C | ECX owner, RET, x87 | Vertical and centering complete; horizontal114 reads supported for actual Text rows |
| A9C050 current78 | ECX owner, RET | Complete normal body; actual current60 remains a type integration requirement |
| AA7D00 local XY | ECX widget, x/y stack, RET8 | Complete canonical owner adaptation: recompose then bounds |
| AA78F0 local Y | ECX widget, y stack, RET4 | Complete canonical owner adaptation: recompose only |

A9DF7F..A9DF8E establish the FC list's empty sentinel/count and the checked
selection iterator. A9DF9C clears cached row110; A9DFA2 clears listener114;
A9DFA8 sets highlight118 to -1; A9DFB2..A9DFC4 clear11C..11F;
A9E016 stores positive zero to line distance13C; A9E030 clears paging148.
The companion initializes only fields with those producers. A9AC90 writes the
raw11C..11E argument bytes in order; it does not normalize Boolean values.
Native invalid-iterator diagnostics are outside the private, valid C++ list
invariant. The Listbox and every borrowed row must remain alive across calls.

A9C990 loads the selected row's live D8 at A9C9E5. A9C920 counts the selected
list node's ordinal. They are not interchangeable: the main-menu command at
599BEE obtains D8 before arrow handling, but its page12 path at599B36 compares
the ordinal to a clicked widget's captured D8. The source preserves both values.

A9C7C0 resets selected to end before searching by ordinal. An existing row is
selected only if its actual hidden77 byte is zero; current80(false) runs even
for a hidden, negative or out-of-range index. A9C740 instead preserves the old
selection when a non-null row is hidden or absent, and does nothing for null.
A9C050 calls actual base78, layout7C, selects the first row without checking77
when nonempty, then calls80(false). This loaded-state difference is intentional.

A9C220's false-force fast path compares current selected row with cached110
and checks paging148. Otherwise it calls the current borrowed114 listener's
slot08 with `(selected row or null, same Listbox)`. The listener may change
selection. A9C2C1..A9C2F5 read current count and selection again before calling
the live F8BC0C binding with CL=1,DL=0; A9C2FD reloads selected row for cached110.
No pre-callback row snapshot is substituted for these later reads.

The sole setter A9AC80 receives941250 from4DD6FB..4DD700. The helper941250
gives DL precedence and calls4C1B90 only on an active flag: DL requests slot2,
otherwise CL requests slot1. It uses the existing request bytes18..1D in
`SoundRequestQueue`; no playback is claimed. F8BC0C remains a live required
binding, since an uninitialized or replaced callback cannot be assumed to be
941250. Main-menu listener114 points at screen+8:59032D installs CEFC48, whose
slot08 is5966F0. That listener body remains a required external implementation.

A9D750's pseudocode loses a stack argument. Assembly establishes row at stack20,
position at24, after byte at28, and RETC atA9D861. In the implemented null-position
path it calls row current34(true), the real AA83A0/AAA5A0 ownership transfer,
then appends that same row to FC. The first row establishes selection and80;
7C runs after every insertion. A9BE60 calls row34(false), removes all matching
FC identities without detaching/deleting the real widget, resets selection
and cached110, calls80(false), then7C. Duplicate FC pointers retain one actual
GUI allocation, matching the native row-list/tree distinction.

Layout7C observes current11F and row fields each iteration. Vertical increments
use x87 `(height + line_distance) + offset` without an intermediate float
rounding. Horizontal layout reads actual Text measured_width114, divides by
live CEC380, spills to float atA9C128, then adds live D7A2F8 and offset. The
centering pass negates total, multiplies by live D7A280, and then uses heights
even following horizontal layout. Installed constant bits are408E000000000000,
3F947AE140000000,3FE0000000000000 respectively. Outgoing offset arguments also
retain the original x87 float copy. Unsupported raw derived114 aliases throw
at the native read phase. Current row nodes must remain valid across callbacks.

The strict MSVC Win32 translation-unit check passes with `/W4 /WX /fp:strict`.
The call report checks27 numeric call rows with zero failures;3 symbolic rows
remain explicit dynamic contracts. The canonical-owner fixture passed using
the parent's actual type11 owner/type/color TUs with this source and the prior
parent library. It covers real row attachment/removal, duplicate FC identity
with one GUI allocation, vertical layout/centering, live D8 versus ordinal,
hidden selection versus78, listener reentry/final cache reload and the actual
sound-request queue flags. Input hashes and the combined-build limitation are
recorded in the report. No new
permanent test, native-byte differential, full Text rendering, game or full
Listbox property/frame/scalar validation is claimed.

`reports/gui_listbox_runtime.json` contains exact sites, body ranges, corrected
contracts and validation boundaries. Parent integration owns A9C220/941250
function definitions/names and the dedicated Listbox type adapter. Remaining
work includes A9E400 properties, A9D030 frame and navigation, paged-list
producers, non-null row insertion, linked highlight/automatic row-control
tails, and the actual native derived destructor/copy/pool paths.

## Correction from docs/ORCH5_MENU_SELECTION_ROW_CONTROL_BATCH.md

The later selection/row-control batch implements menu current08 and actual vehicle unlock, canonical listener/layout bindings, Listbox row predicate/state/current60, null-position D8 append and existing native table64/70 profiles. BA90 retains an explicit visited-prefix callback domain. Full Listbox frame/properties/teardown, non-null insertion and complete menu/resource providers remain open. Earlier validation remains pinned to its original commit; see the new batch report for current scope and integrated validation.
