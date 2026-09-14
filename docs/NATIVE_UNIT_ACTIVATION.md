# Native unit activation and registry insertion

Addresses: 0077F0E0, 004CEE30

| Routine | Coverage | Result |
| --- | --- | --- |
| `activate_native_unit_0077f0e0` | complete normal body `0077F0E0..0077F24C`, 365 bytes | reconstructed caller, original-byte fixtures and Win32 build passed; complete providers required |
| `grow_native_unit_registry_count_004cee30` | complete `004CEE30..004CEEC2`, 147 bytes | existing canonical count/exception implementation reused with the native `3FFFFFFF` bound |

The caller's original ABI is ECX=canonical unit, preserved EBX/ESI/EDI,
bare RET. These are new C++ interfaces, not binary replacements. The registry,
unit and saved storage remain externally owned. The descriptive names are
hypotheses. No gameplay or executable-path admission is claimed.

## Registry and canonical count operation

`00F87194` is the actual checked-list object: opaque word at +0, sentinel
pointer at +4 (`00F87198`), count at +8. The existing semantic mission-load
candidate interface is not this storage. Its comment saying every reference,
including `0077F0E0`, is a reader missed the indirect writes recovered here.
`004C2220` is the node producer: allocate 0Ch bytes, store next at +0,
previous at +4 and the dereferenced canonical-unit argument at +8; RET0Ch,
EAX=node. It retains its native allocation/error behavior as a required provider.

The caller captures sentinel and sentinel.previous, allocates a node, grows
count, writes the **captured sentinel's** previous, reloads the new node's
previous, and writes that node's next. Count failure has no caller cleanup for
the allocated node. There is no new container, allocator, fallback list or
alternate unit owner. The list view's count reference must denote actual
owner+8; the canonical implementation uses that same cell.

The old `STL_xlen_throw_004cee30` metadata is misleading. Native normal flow
reads count, compares unsigned `3FFFFFFF-count < increment`, adds increment
and stores count, then RET4. Its complete 147-byte instruction sequence equals
`004CE780` after normalizing addresses and bound. The old routine's limit is
`1FFFFFFF`, and remains unchanged. The source uses one internal bound argument
and the existing `NativeAliasListLengthError` owner and string implementation.
No additional STL implementation is introduced.

Both error paths assign the same 16-byte `list<T> too long` literal using
`00408720`, construct through `00411700`, write vtable `D69260`, and throw
type `D83F98` through `00BF6885`. The FH3 maps have the same single armed state;
`C65A80` and `C65B40` both destroy `[EBP-50]` through `004072D0`.
The corresponding handlers both jump to `BF6B43`. The retained normalized
body/FuncInfo/map audit proves this reuse boundary. Source exceptions use the
existing owning C++ transport; original native FH3/SEH identity is unproved.

## Unit storage and holder branches

The caller reuses `NativeUnitObserverAlias` and the existing scene holder and
property-record views. It borrows actual side +54, holder +C0, nine role DWORDs
at +188, word +28C, tick +294, and marker +304. The base constructor `925CE0`
produces side=2 and holder=null; `77EED0` produces zero word+28C/tick+294.
The saved restore writes are themselves producers for these exact reached
fields. Unestablished saved-schema meanings retain address-based names.
No pose field is accessed directly; the required complete race operation
must use the same canonical unit and existing hierarchy storage.

`009277E0` tail-jumps `00923840`: when race+58 is negative and parent+3C is
nonnull it inherits that parent's race; it unconditionally sets +5C and +BD
to one. After this complete required provider returns, the caller captures
current holder+ C0, reads global `F876B0`, and writes unit+294. A null holder
returns. Holder+4 is a **kind**, not a reference count:

* Kind 2 captures saved holder+8, reads saved+B0, selects current unit virtual
  +144, writes unit+54, copies saved+40 to +28C, uses FLD/FSTP to transfer
  saved+108 to +304, then calls the captured entry with saved+10C. It copies
  nine live DWORDs from that same saved object's +64 to unit+188 after return.
  The copy follows REP MOVSD with ABI DF=0; it is sequential, without a snapshot
  or memmove behavior. Callback mutations remain visible.
* Kind 1 captures bag+8, runs complete existence `0048E9F0("OwnerPlayer")`,
  and either finds the raw integer payload with `008F2260` or uses 9. Current
  virtual+144 receives that value. It then reloads holder+ C0 and bag+8 for
  `Invincible` lookup, without adding another kind check. A missing value ends
  the call. Type zero uses CVTSI2SS under current MXCSR; every nonzero type uses
  the payload bits as float. It selects current virtual+F4 after conversion,
  marshals through FLD/FSTP, and invokes that entry with one float.
* Kind 3 constructs a 14h-byte native LuaObject through `009238A0`, then
  constructs a 14h-byte reader through `004425C0` and destroys it through
  `00441A20`. There is no deserialization virtual call between them.
* Every other kind returns after the tick write.

## Required operations and native stack contracts

The report enumerates every outgoing and incoming direct call site plus all
three explicit indirect sites. Every direct provider body was read. Besides
node allocation and race activation, required complete providers are:

| Operation | Native contract |
| --- | --- |
| `009238A0` | ECX unit; stack pointer to fresh LuaObject; RET4; returns that pointer. Looks up global `_savedata._entities` and rereads current holder+8 index internally. |
| `004425C0` | ECX fresh reader; same 20-byte by-value LuaObject already constructed on native stack; RET14h. Pushes root into its vector and destroys the argument. |
| `00441A20` | ECX same reader; bare RET; full vector cleanup and base-table restoration. |
| `0048E9F0` | ECX captured bag; one key pointer, RET4, AL existence result; full pooled-string/hash operation. |
| `008F2260` | ECX bag; one key pointer, RET4, property or null; full lookup including dotted hierarchy. |
| current unit `+144` | ECX unit; one role DWORD, required RET4; checked destroyer table selects `0077F2D0`, full role assignment and suppression-map logic. |
| current unit `+F4` | ECX unit; one float, required RET4; checked destroyer table selects `0042ED80`, +150 store and recursive child propagation. |

The source's LuaObject is the existing `NativeLuaObjectStorage`, and reader
bytes are uninitialized stack scratch. The constructor provider owns the
consumed root's destruction and its own failure cleanup. This caller adds no
automatic exception guard absent from the original. Providers are pure virtual
requirements, with no stub/default or semantic substitute. Pure view/table
bindings must not mutate, callback, allocate or alter the FP environment.

## Verification boundary

Release Win32 `scripts/build.ps1` and both existing CTests pass. All eight
native seed spans match disk. Six full original-caller/source cases pass:
null holder, saved data with role callback mutation, present OwnerPlayer with
integer Invincible, default owner with signaling-NaN Invincible, kind 3 Lua
lifetime, and unknown kind. They verify list links/count, holder rebinding,
field/call ordering, x87 quieting and unrelated fixture bytes. The original
147-byte count success path executes inside each native caller case. Separate
source cases prove distinct old/new error bounds and unchanged count on throw.
Other native callees are explicit fixture bridges; they are not executed.

Production `/O2 /fp:strict` code retains side-read/table/store order, saved
FLD/FSTP before role read, all nine live copy steps, and Invincible conversion,
fresh table selection, then FLD/FSTP. Volatile borrowed accesses prevent cached
field values and reordered publications. Source/production objects and their
disassembly are retained, along with exact original bytes, compiler-discovered
headers, tools, searched libraries, fixtures and embedded-manifest executable.
`local/activation_review_manifest.json` maps content-addressed copies and hashes.
The mechanical report validates 39 direct rows; incoming call `007F1FFC`
remains a root listing-repair item because live Ghidra has no containing
function. The report explicitly bounds its raw caller at `007F1FE0..007F2280`
(641 bytes), with RET then INT3 padding, and records `no_ghidra_function`.
This missing incoming owner is the sole mechanical check failure.
Build and controlled fixture evidence do not establish a working unit owner,
native exception dispatch, actual callee bindings or gameplay equivalence.
