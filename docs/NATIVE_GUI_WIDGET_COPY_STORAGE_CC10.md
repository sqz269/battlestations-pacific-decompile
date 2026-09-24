# Raw widget base copy, AA9520

`construct_native_gui_widget_copy_00aa9520` covers the complete normal 515-byte
body `[AA9520,AA9723)` for a current source node whose actual Model virtual10
selects B752B0. Destination is the native constructor receiver ECX and the
source is the stacked argument; the native return is destination
in EAX and `RET4` at AA9720. The C++ interface has explicit source/destination
and binding contexts, and is not a binary replacement. Descriptive names are
reconstruction hypotheses, not recovered symbols.

The raw destination must be fresh, aligned, nonoverlapping storage of at least
E4 bytes. The source and all actual flag/profile/context cells must remain live
across callbacks. This is separate from the older logical GuiWidgetOwnerRuntime
implementation. It uses the existing actual identity, sentinel, Model clone
and lifetime providers without creating a logical widget or a second count.

## Exact storage schedule

The first eight bytes become CEB130, fresh atomic reference count1, then
D5C130. Eighteen float copies use x87 FLD/FSTP: 0C through48 inclusive, then
94 and C4. The48 load occurs before clearing destination4C; its store follows
that clear. These are conversions, not integer/memcpy copies: signaling NaNs
quiet according to the current x87 environment. Each field is read separately.

DWORD50/54/58/5C/60 copy before AA95E3 calls actual A9B720, which allocates
the real12-byte empty-list sentinel. Its pointer is published at68, count6C
becomes zero, and allocator64 remains untouched. The following source reads
are current values after that allocation:70; bytes74/75; forced byte76=1;
bytes77/78/79/84; float94; DWORDsA4..C0; floatC4; byteD4; DWORDsD8/DC/E0.
Destination88/8C/90 become zero before94. The native early parent0 push is an
argument to the later clone, not a parent-field write or a retain.

Unwritten ranges remain the slot's preimage:08..0B,64..67,7A..83,85..87,
98..A3,C8..D3,D5..D7, and E4 onward. This includes derived payload and the
real fixed-pool ID when present. There is no child copy or list insertion.

At AA96E4..AA96FC, read current source4C, current source60, the node profile,
actual D5C0B8[type] flags, then current virtual10. Source4C is unconditionally
dereferenced by native code. Valid current types index19 DWORDs:0..16=3E,
17..18=26. This table is flags, not name pointers; the separately published
copy-flags correction retains exact live/PE proof. No read is hoisted before
sentinel allocation. Unknown node targets have no substitute implementation.

The supported actual Model binding calls complete normal B752B0 with the read
flags and parent0. That provider allocates the real188h slot and owns its raw
name/base/geometry/pose work and its established dispatch limits. Its returned
sole creator goes directly into destination4C. The caller acquisition record's
model creator is cleared to record that transfer, with no retain/decrement.
The returned model's current138 flags are ANDed with FFFFFFFC. A successful
B752B0 returns nonnull; native's null-result mask skip is unreachable in this
supported normal domain. No source model or copied parent pointer is retained.

## Failure and evidence boundaries

CB73F1 is a verified10-byte handler boundary: MOV EAX,DEDC64; JMP BF6B43.
The map at DEDC4C is state0 -> -1/CB73D0/AA6E10, state1 ->0/CB73D8/A9BCE0
at+64, and state2 ->1/CB73E3/AA7F50 at+88. Normal code arms0 before sentinel
allocation and2 before virtual10. Host C++ cleanup uses those same actual
helpers, in timed/list/ref-base order for state2. It neither invokes the whole
widget destructor nor invents model rollback. Native FH3/SEH remains unported.

The one-shot caller frame records call site, native state and actual clone
acquisitions. A failure after the model constructor has completed leaves the
provider's acquired model/nested creators available for explicit recovery,
even though the widget's own list/ref-base have been unwound. Diagnostics do
not add ownership. Reusing a partially attempted frame is invalid.

Strict MSVC Win32 compilation and the three existing CTests pass. Exact live
515-byte body and handler definition evidence, native call rows and focused
composition comparison results are recorded in
`reports/native_gui_widget_copy_storage_cc10.json`. The comparison borrows
existing actual callees; it does not prove their independent original behavior.
Native exception transport, register/private-stack ABI, asynchronous fault-time
observations, unsupported virtual targets and application/game behavior remain
outside the evidence. No application startup or factory wiring is added here.
