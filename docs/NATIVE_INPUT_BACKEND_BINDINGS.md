# Native input backend binding changes

Addresses: 00A91620, 00A90EE0, 00BEBF30, 00A983C0.

These four game bodies now operate on the actual F8h backend, its fixed device
slots and its original active/accepted-ID vector headers. They create no typed
InputFocusBackendState/InputDevice projection, second backend, shadow vector or
singleton manager. Descriptive names are hypotheses. Source control flow is
complete over valid actual storage with the required captured-target providers;
the added C++ interfaces do not reproduce native stack, FH3/SEH or binary vtables.

## Layout and ownership

A908A0/A91570/A982D0 are the existing raw producers. Fixed slots are at
`backend+4+(class*8+slot)*4`, three rows of eight pointers. Class records start
at `backend+68h+class*24h`: requested count+0, active-vector header+4, accepted-ID
header+14h. Both actual10h headers contain an untouched leading DWORD, begin+4,
end+8 and capacity-end+C. A914C0 initializes their six pointer fields; A90DC0
frees accepted-ID storage before active-pointer storage. This packet borrows
these exact records. Active pointers add no reference and own no device.

Activation/removal change active storage and dirty byte+D4 only as specified;
accepted IDs and requested counts are read, not invented or populated. Class
deletion owns the fixed slots. It does not clear active vectors or decrement
references. SDK/device/effect COM references remain owned by the existing
external trackers through all raw callbacks and final explicit release.

## Bodies, arguments and complete extents

| Entry | Native input / result use | Inclusive end | Coverage |
|---|---|---|---|
| A91620 | ECX backend; class/slot DWORDs stack; RET8; result unused | A917D5 | Complete game body |
| A90EE0 | ECX backend; raw device pointer stack; RET4; result unused | A90F8F | Complete game body |
| BEBF30 | ECX backend; class DWORD stack; RET4; result unused | BEBF63 | Complete game body |
| A983C0 | ECX backend; no stack arguments; RET; EAX retains ignored SDK status | A983DF | Complete game body through explicit enumeration provider |

The final instructions are A917D3 RET8 (3 bytes), A90F8D RET4 (3), BEBF61 RET4
(3), and A983DF RET (1). No missing starts or excluded tails occur in the four
owned bodies. Class/slot arithmetic retains native DWORD bits and wrap; no new
signed bounds checks are inserted. A983C0 is the derived backend D5B72C slot0C,
not a direct call in the focus-reset caller.

## Activation A91620

The candidate pointer is captured once from its fixed slot. The accepted-ID
iterator begins at the captured begin; every subsequent end check reloads at
the native point. All IDs are visited even after acceptance. `FFFFFFFF` accepts
without a virtual query. Otherwise the candidate's current profile supplies
slot34, and the ID is reread after that call before comparison. An identifier
callback may change the fixed slot or ID record; that does not replace the
captured candidate. Empty filters reject everything.

If accepted, the active vector is scanned for that captured pointer. A match
returns without dirtying/appending/callback. An absent pointer sets dirty+D4
before storage work and captures the previous count before insertion. Available
capacity uses the native inline DWORD store/end advance. Otherwise the checked
STL one-pointer insertion contract is supplied by the stateless source adapter
described below. A wildcard may accept a null fixed pointer; no new null guard
or reference operation is inserted.

Only after insertion does the routine capture/test the current +D8 identity.
At A917CA it receives class in ECX, previous count in EDX, no stack arguments.
The original backend is preserved across the insertion iterator-output writes.
Allocation/provider failure propagates; dirty is not rolled back and no success
callback is synthesized.

## Removal A90EE0, class destruction BEBF30, enumeration A983C0

Removal captures current device profile/slot08 and uses the returned class to
select its active vector. The first matching pointer is erased in order.
Dirty+D4 precedes the real `memmove_s`, whose destination capacity and byte count
are the same remaining-tail byte count. The return is ignored. End is read
again and decremented after the move, including a returning invalid handler.
The current +D8 is then captured and called with the captured class and EDX=-1
at A90F86. A missing active pointer changes nothing. There is no device release.
The A90F21 validation CALL is physically in the native body but unreachable:
`CMP ESI,ESI; JZ A90F26` always skips it. The report marks it accordingly.

BEBF30 visits eight fixed cells in address order. Each current nonnull device
supplies its current scalar slot04 and receives flags1 directly at BEBF4F.
This does **not** call BD30E0 or InterlockedDecrement. After a normal return it
zeros that same cell, even if the scalar callback replaced it, then reads the
next cell. A throw aborts the remaining loop; no extra owner cleanup is added.

A983C0 captures backend+E0 before clearing byte+F4. It then calls the actual
DirectInput interface's EnumDevices slot10 with `(interface,0,A982B0,backend,1)`.
Five stdcall DWORD arguments are consumed by the SDK. The required provider
routes through existing raw enumeration and its callback-exception transport;
it must use the captured interface and the same backend. No fallback enumerator,
synthetic device, filter or automatic reference release is supplied.

## Explicit providers and returning CRT behavior

`NativeInputBackendBindingsCalls` has five methods: captured profile class08,
identifier34, scalar04(flags), captured DirectInput enumeration, and captured+D8
callback identity. The existing AE runtime supplies the device/SDK methods;
startup's `invoke_native_input_startup_callback_004b4630` supplies the supported
bare-RET callback identity. Other identities remain explicit binding errors or
required providers; numeric native addresses are never invoked in source.

Every reachable BF6713 check in the game bodies calls the real source CRT
`_invalid_parameter_noinfo`, which may return. Captured iterator/end values and
later reloads are retained rather than replacing validation with a throw or
container normalization. This uses the source CRT handler/errno domain; equality
with the original static CRT's handler storage or exception ABI is not claimed.

## Checked-library storage boundary

A91430/A91260 and their allocator/copy/fill helpers are recognized checked STL
contracts, not newly ported or renamed native library bodies. The new
`insert_input_active_pointer_storage` is a stateless source one-element insertion
adapter over the caller's actual10h header and actual8h iterator output. It
preserves the header's leading word and uses the existing
`singleton_lifetime_allocate/free` domain consumed by A90DC0.

The adapter captures the pointed-to value before allocation. It retains the
checked insertion index, 3FFFFFFF element bound and capacity-growth contract.
After allocation it reads current begin for the prefix copy, current end after
copy/fill for the suffix copy, then current begin/count for free/publication.
The header is published begin, capacity-end, end; the iterator is written
position then header. The in-capacity path supplies valid single-element insert
semantics. This is a library boundary, not an instruction-by-instruction trace
claim for every original STL specialization or malformed iterator/heap state.
Complete DWORD ranges, sufficient backing storage after callbacks and valid
allocation identities are required; native compiler EH/CRT heap identity and
arbitrary saved-stack mutation are not reproduced. No library source was imported.

The unowned library A91260 export had a free fallthrough gap A91361..A91363:
aligned installed bytes are `ADD ESP,4`, following CALL BF65AC at A9135C and
preceding A91364. Its full final RET10 is A91411..A91413. This was reported to
primary for a separate repair; no library body completion claim or Ghidra write
was made by this worker.

## Evidence and validation boundaries

Read-only batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, then refreshed the four bodies and bounded library
dependencies. All direct CALLs, six indirect sites and known incoming sites are
recorded in the report. Incoming activation sites are BECAB6 (class1/slot0),
A91902 (loop class/slot), and 67D77C (class2/raw A3E470 result in EDI). Removal is
called by BECA8D and 67C9B5; BEBF30 by BECA9A. Derived backend slot0C is captured
at BECAA5..BECAAA before enumeration. No callee contract was inferred solely
from a descriptive prior typed implementation.

The focused ignored manifested fixture uses actual F8h/class headers, native CRT
pointer allocations and genuine raw mouse scalar deletion. Controlled providers
exercise the returning CRT invalid handler, two identifier calls after a wildcard,
post-query ID reload, captured candidate despite fixed-slot replacement, ordered
removal, direct scalar flags1 with unchanged reference7, same-cell overwrite and
later-cell reload. It deliberately leaves SDK enumeration unreachable. Win32
Release, all eight native seeds, both existing CTests and the fixture passed.
The report audit passed all 29 direct/incoming rows; seven explicitly indirect
rows have separate native profile/ABI and containing-function evidence.
No application window, device polling, input, force or gameplay effects are part
of this packet's validation.
