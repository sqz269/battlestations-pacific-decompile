# Native GUI, movie and sound focus dispatch

Addresses: `00aa33a0`, `00aa8e40`, `00aac8a0`, `00a9e110`, `00a4cb60`,
`00a7a3f0`, `00a7a480`, `00a7a4a0`.

`native_window_focus_dispatch` supplies eight raw operations needed by the
actual BED3B0 message handler. It consumes the existing GUI manager, raw widget
and decoder storage, actual sound-manager fields, the verified native table
mapper and the shipped `_BinkPause@8` import. Numeric native vtables resolve to
reviewed source functions; they are never treated as host code pointers.

AA33A0 traverses GUI manager vector +18/+1C and calls AA8E40 for each page.
It captures the cursor before its first validation. Each loop captures end
before checking live begin against end. A returning BF6713 validation callback
may repair the live header, but equality still compares the captured cursor
and end. Bounds are reloaded before dereference and after the recursive call.
Pointer advancement wraps by four. The CMP ESI,ESI validation path is dead.
The old `STL_inst_00aa33a0` inventory classification is superseded: iterator
checks are inlined into a game operation that pauses movie descendants.

AA8E40 traverses raw circular child list +68 with next at node+0 and child at
node+8. It checks each child's current virtual +5C. Type10 gets decoder pause
first, then every child is recursively visited. It reloads the node's child
and current sentinel after calls, retaining returning validation behavior;
it adds no snapshots, null skips or cycle suppression. Its CMP EDI,EDI branch
is also dead. All 19 Ghidra data references found for current type slot +5C
point to A9E110, a four-byte `MOV EAX,[ECX+60h]; RET` leaf. AA9390's argument
load and +60 store establish that field; AAFE20 passes type10 and installs
the movie profile D5C270. The full GUI producer remains separate work.

AAC8A0 tests widget+F0 and returns RET4 if null. Otherwise it reloads +F0 and
tail-dispatches the current decoder profile's +14 slot with the incoming DWORD
argument. Four concrete/shared decoder profiles resolve this slot to A4CB60.
The source accepts the reviewed numeric type/pause targets and explicitly
rejects unsupported targets. A4CB60 captures handle+4 before reading the low
pause byte. A null handle returns without reading the half literal or changing
delay. Byte zero captures CE3800=0.5 and writes decoder+14 before calling
BinkPause(captured handle,zero-extended byte). Its result is ignored. The
incoming DWORD's high bits are not forwarded to Bink.

A7A3F0 captures sound manager +8C array and +90 count, computes the wrapping
end once, and walks raw pointers. Entry descriptor+44/class+8 selects
`1u << (class & 31)`. A mask match stores only dirty byte+14. A7A480 changes
pause byte+50 from zero to one before dirtying mask0000FFFF. A7A4A0 changes any
nonzero byte to zero before the same call. Repeated pause/resume does nothing.
FFFF is a 16-bit class mask; it does not select every possible shifted class.
These operations consume an already-constructed sound manager; they do not
publish or fabricate F8BBD8.

## Evidence and validation

Report: `reports/native_window_focus_dispatch.json`. All 387 bytes of the eight
bodies match live Ghidra and the unchanged installed executable. Fifteen direct
call rows are checked, including the two dead validation sites. The GUI type
call, decoder tail dispatch and Bink import are recorded separately. Another
468 bytes retain 23 current vtable slots, the half literal, GUI producer slices
and existing decoder constructor bodies. Existing descriptive names/comments
are preserved, except the misleading AA33A0 inventory name. Its old tag record
is retained locally; the corrected tag and new reviewed comment supersede the
historical Ghidra inventory classification.

Strict Win32 build and both existing CTests pass. No permanent test was added.
Fresh fixture compile/link attempt1 and execution capture1 pass with 24 explicit
source units compiled and 23 selected, using the current production libraries.
The complete A7A3F0/A7A480/A7A4A0 bodies (110 bytes) execute unchanged in a
single RX buffer that preserves their relative call displacements. Five cases
compare all raw manager and entry bytes against source, covering low-five-bit
class shifts, FFFF selection, noncanonical pause-state bytes and repeated
transition skips. The original four-byte A9E110 leaf is also compared directly.

An explicit raw widget graph under the actual constructed GUI manager reaches
a nested movie and a null decoder. The decoder uses the existing complete38h
source constructor; the fixture opens installed `movies/fe_eidos.bik` through
shipped `binkw32.dll` after selecting track0, with the native4000h flags. The
real handle reports1280x720 and90 frames. Source traversal calls the real
pause import with a high-bits-set argument ending in01, then one ending in00;
the decoder delay stays7 and then becomes0.5. The handle is explicitly closed.
No video frame is decoded or displayed. A null captured handle leaves delay
unchanged. The physical Bink DLL and movie hashes are retained and unchanged.

Two returning CRT validation callbacks deliberately repair GUI vector bounds.
The source keeps its captured cursor/end and returns without visiting the
newly visible range; live bounds have both repairs and movie delay stays9.
The CRT handler and actual manager vector fields are restored afterward.
The existing platform/renderer/device/focused Reset/cache/installed DDS/effect
fixture and prior constructor-failure checks continue to pass in the same run.

## Boundaries and follow-up packets

Widget graph and sound-entry storage are explicit raw fixture entry preimages,
not full widget/sound producers. The existing decoder constructor covers its
complete-object flag1 path; further-derived virtual-base layouts are not claimed.
GUI recursion and the Bink-backed leaf execute source, not original GUI machine
code. Source contexts are new interfaces, not original register/FH3/SEH ABI.
Nested callback mutation, concurrency, invalid pointers, cyclic trees, throwing
validation and unsupported virtual targets are not runtime validated here.

The actual BED3B0 window handler still needs renderer refresh B24FB0 with
B22030/B21F70 and service restore B0D1E0 with B50010/B4ECC0. Full raw sound
production around A88770 remains required because activation unconditionally
reads F8BBD8. Full GUI page/widget production and resource construction also
remain. BEC3B0's inner BED3B0 callback is callee-cleans-five/RET14h; full BECEE0
is cdecl11. Full window activation, active frame/draw, shutdown and gameplay
validation remain outstanding.

Immutable closure: `local/checkpoints/f9e68cc1/native-window-focus-dispatch/validation.json`, SHA-256 `57fd9ea79bb21e9b59593170624bcfad98e638a5c4d7bb4370349ba189e53d68`. It retains 4958 artifacts, 82 physical Win32 modules and 308 selected production source providers. Captured docs/report precede this closure metadata addition.
