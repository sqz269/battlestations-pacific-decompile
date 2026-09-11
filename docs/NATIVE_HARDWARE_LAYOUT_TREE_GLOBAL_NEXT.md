# Native hardware-layout tree global lifetime

The remaining canonical `108D530` tree path has five complete bodies totaling
437 bytes. Ten live Ghidra/installed-PE spans also verify the actual zero-filled
header, CRT registration pointer, and existing service boundaries. This is
read-only discovery; no source reconstruction or Ghidra mutation is claimed.

| Entry | End exclusive | Native contract | Packet |
| --- | --- | --- | --- |
| B25DC0 | B25DF7 | ECX unused, no stack args, EAX allocation, RET | lifetime3 |
| B230B0 | B230E5 | ECX tree, stack node, RET4 | lifetime3 |
| B2F3A0 | B2F469 | ECX tree, output and two iterator pairs, RET14h | lifetime3 |
| CD7960 | CD79A0 | no arguments, EAX atexit result, RET | static2 |
| CE0C50 | CE0C90 | no arguments, EAX zero on normal return, RET | static2 |

`B25DC0` allocates 28h bytes through actual `BF681B`. It conditionally clears
the link words at +0/+4/+8 after testing each computed address, writes black
color1 at +24 and nonsentinel0 at +25, and returns the allocation in EAX.
The key/value and final padding stay untouched. Its incoming ECX is unused.
The established shared allocator cannot return null successfully; preserve the
original checks without claiming that its later mandatory writes make a null
allocation valid. There is no EH frame or invented initializer callback.

`B230B0` recursively destroys the current right subtree, then captures the
current left link, frees the old node through actual `BF65AC`, and iterates
that captured left child. Sentinel nodes are neither traversed nor freed.
Values receive no Release call. Ghidra omits the returning-free continuation
`B230D4..B230DE`, which includes the next sentinel test and loop. The complete
function already ends at B230E4; its instruction flow still needs repair.

`B2F3A0` captures current minimum before first-owner validation, then reads
first node after the handler. If it equals that captured minimum, capture
current head before last-owner validation and compare last node afterwards.
The full-range branch destroys the current root through B230B0, rereads head,
sets root=self, captures head again, writes count0, writes minimum=self using
that captured head, then rereads head to set maximum=self. Capture current
minimum before publishing output OWNER then NODE.

The other branch validates first-owner against last-owner before every equality
check. When nodes differ, pass the actual local first iterator to B20DC0 before
erasing its captured old owner/node through B2EF00, then reload the advanced
first iterator. Normal output publishes final first OWNER then NODE. Returning
invalid-parameter handlers and output aliases require assembly-order review;
do not turn these iterators into snapshots of the tree's head or key storage.
Both iterator operations are already complete native-storage reconstructions.

The CRT word at `CE3504` points at the missing function `CD7960`, within the
previously verified C++ initializer interval `[CE2734, CE36E4)`. The initializer
allocates a sentinel through B25DC0, publishes current head+4, sets its sentinel
byte1, then rereads current global head separately for parent=self, left=self,
and right=self. It zeroes count+8 before calling real atexit with CE0C50.
Header word0 is untouched. Registration failure does not undo initialization.
The initializer's caller and CRT service boundaries are recorded separately in
`NATIVE_HARDWARE_LAYOUT_GLOBAL_STARTUP_NEXT.md`.

`CE0C50` captures current head and its minimum, invokes B2F3A0 over that complete
range, then reloads current head and frees it. Its hidden return continuation
`CE0C7C..CE0C8F` clears current head and count, preserves header word0, and
returns zero. The current Ghidra function ends at CE0C7B and has a misleading
static-initializer name. Restore the full body and preserve old comments before
renaming it as a shutdown thunk. Destruction releases tree storage only; it
does not destroy hardware-layout values.

The ready lifetime3 packet owns B25DC0/B230B0/B2F3A0 and four
`native_hardware_layout_tree_lifetime` source/header/document/audit files.
Static2 owns CD7960/CE0C50 and four corresponding
`native_hardware_layout_tree_static` files, depending on lifetime3. It requires
one explicit canonical actual header and persistent invalid-parameter domain
for real host atexit registration. It must not substitute a different pool,
private tree, placeholder shutdown, or replacement CRT callback collector.
The detailed report records exact spans and the required analysis repairs.
