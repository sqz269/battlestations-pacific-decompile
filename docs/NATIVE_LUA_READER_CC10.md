# Actual raw Lua reader (cc10)

This module supplies a 20-byte physical reader for the successful raw-object
path at 004425C0, 00441A20, 00441A70, 00BD8E20, 00BD7A20 and 00BD7130.
It uses the existing actual `NativeLuaObjectStorage`, `NativeLuaStateStorage`,
B66FA0 copy registration, BD5790 child lookup and B67700 release. The older
`GuiLuaReader` uses registry references and remains a separate projection.
This module is an explicit C++ interface, not a patched binary ABI or installed
game path.

The reader has profile DWORD at +0, a proxy DWORD at +4, and three vector
pointers at +8/+C/+10. Original 4425C0 writes CE44FC and zeroes only the
three pointers: +4 is unwritten. The source constructor also leaves +4 alone.
It consumes a caller-owned 14-byte by-value root only after copying it into
the vector; both native and source register the new element's actual address
and release the root. Original 441A20 restores CE44FC, tidies the vector,
then restores CE374C. Original 441A70 follows the same sequence and frees
only when stacked flags bit0 is set; its RET4 corrects the older no-argument
ledger description. Source bit0 deletion requires matching global `operator
new` ownership. The numeric profiles are stored, never called as host vtables.

`NativeLuaReaderValue` is a 14-byte RAII object. Its copy constructor invokes
actual B66FA0, and its destructor invokes actual B67700. A user-declared copy
constructor and destructor cause current `std::vector` growth to copy retained
values into their new addresses. The selected MSVC 14.51.36231 `/MD` Release
library with `_ITERATOR_DEBUG_LEVEL=0` has a 12-byte vector and growth capacity
`max(new size, old capacity + old capacity/2)`. It destroys old elements
forward; `pop_back` calls the last destructor before decrementing the end.
The source guards empty pop because original BD7130 makes it a no-op, while
empty standard-vector pop is invalid. `current_raw()` borrows the current
14-byte object without copying or registering another owner reference; it
requires nonempty storage and expires on vector mutation or destruction.

Original BD8E20 takes the actual last vector object as BD5790 parent, writes
a lookup result into caller-owned stack scratch, copies that result into the
vector and destroys scratch. The lookup initializes only owner/kind/index/
tracked. Its opaque DWORD and three padding bytes retain their prior bytes;
the source API therefore requires caller-owned initialized scratch and does
not fabricate zeroes. The existing raw lookup may perform unprotected Lua
work. The supported path has a valid nonempty parent, Lua stack state, fewer
than 50 tracked slots and five references per slot. Other key kinds retain
BD5790 behavior. Source invalid-empty access throws `out_of_range`; the
native CRT invalid-parameter path is outside the reconstructed domain.
BD7A20 is the reader+4 adjustor tail jump into BD7130. A successful pop
destroys the last 14-byte element before updating the vector end.

## Growth and tracked-owner evidence

The original 00441B70 full-capacity growth first copies the incoming object
to a temporary, chooses geometric capacity, allocates, copies the old prefix,
inserts the temporary, copies any suffix, destroys old elements forward,
frees their buffer, publishes new pointers, then destroys the temporary.
The current vendor vector constructs the inserted element first, then copies
old elements, and destroys the old range forward. Thus intermediate reference
counts and copy addresses differ. The source does not claim an identical
instruction trace or callback/exception behavior.

One ignored Win32 `/MD /MANIFEST:EMBED` probe runs the original 731-byte
00441B70 body and its original 00440230, 00440CF0, 00440D30, 00440600 and
0043F1C0 helpers from saved Ghidra bytes. Allocation/free and the already
reconstructed B66FA0/B67700 are rebound; this is not a test of their original
CRT/library bodies or FH3 unwind. The source half runs the real release vector
and raw providers. Both halves use actual Lua 5.1 stack values and actual
`NativeLuaStateStorage` slot registrations, including an existing tracked
parent. At construction, 1-to-2 growth, pop, reader teardown, and final
external-owner cleanup, Lua stack height, active slot counts and reference
identities, object indices and high-water agree. Growth keeps parent and child
refs at counts 2/2 and stack height 2. Pop leaves 2/1; reader teardown leaves
1/1; final cleanup leaves 0/0 and Lua stack height 0. The native bytes hash is
`ae10412a365700f278bfc67f6ac941aa6e00ab01bf126e5497c7518c9ad4bc20`.

Strict MSVC Win32 build and both existing CTests pass. The focused probe passes.
The implementation excludes native register calling convention, exact FH3/
SEH frames and exception identity, STL allocation failure, invalid-parameter
behavior, Lua longjmp, arbitrary Lua callbacks observing intermediate owner
counts, and game-level use. It adds no custom STL port, logical GUI registry
owner, or AC6600 page construction. Original vtable dispatch and dependent
GUI read/default operations remain for the caller to integrate.
