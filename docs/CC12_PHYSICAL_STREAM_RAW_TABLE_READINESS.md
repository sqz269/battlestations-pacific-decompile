# CC12: physical-stream raw table and producer readiness

## Result

The two qualified raw physical-stream entries, BF5020 and BF4F20, still do
**not** establish a genuine Source class table for raw BF1090/BF10E0 dispatch.
The actual current producer is `construct_native_physical_stream_00bf50d0`
called by `acquire_native_physical_stream_00bf3770`. It initializes allocated
or reused 20h-byte storage and publishes the literal Original profile value
`00D691B0`. It does not publish a Source-owned function table containing the
new entries. The normal VFS dispatcher continues to interpret Original
numeric method words and call ordinary C++ helpers.

There is no `NativePhysicalStreamStorage` declaration or use in current
`src`, `include/bsp`, `tests` or `tools`. That name must not be treated as an
existing class or producer. The separate `PhysicalFile` class explicitly
describes itself as an owning projection without the native vtable/pool ABI;
its HANDLE is its first field, not the native receiver's +8 field.

This audit changes only this document and its JSON report. It introduces no
table, dispatcher, callback, wrapper, Source code, build registration, fixture
or Ghidra mutation. Previous fixtures were not replayed.

## Actual Original publications

The verified live `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` reports four incoming data references to
`00D691B0`:

| Publication instruction | Containing function | Evidence and scope |
| --- | --- | --- |
| BF50E1 | BF50D0, `BSP_PhysicalStream_Construct` | Complete call-free constructor, inspected below. |
| BF5568 | BF5520, still named `FUN_00bf5520` | Complete alternate constructor with an open call, inspected below. |
| BF5097 | BF5090, deleting destructor | Existing Source also republishes D691B0 before close/base teardown; provider body not expanded. |
| BF4FC7 | BF4FC0, still unnamed | Other table writer; callgraph reports BD30F0 base destruction. Consistent with teardown, but its complete body was not audited here. |

Only the two constructor bodies were added to the table audit's address
lease. Their complete live bytes equal the installed PE, with independent
full instruction decoding and direct-call enumeration. Names are descriptive
reconstruction hypotheses, not recovered class symbols. Empty live caller
results do not establish that a function is dead.

### BF50D0: the current Source producer's Original

The entire Original body is BF50D0..BF50FA, **43 bytes / 11 instructions**:

```text
MOV EAX,ECX
XOR ECX,ECX
MOV [EAX],00CEB130
MOV [EAX+04],1
MOV [EAX],00D691B0
MOV [EAX+08],FFFFFFFF
MOV [EAX+10],ECX
MOV [EAX+14],ECX
MOV [EAX+18],ECX
MOV [EAX+1C],ECX
RET
```

There are no calls, import dependencies or hidden allocation operations in
this body. ECX enters as the actual receiver; EAX returns the same address.
The intermediate base-profile publication precedes the reference count and
final physical-profile publication. Field +C is untouched. The two 64-bit
fields are reset as four separate DWORD writes. Live callers identify
BF3770, the physical pool acquisition routine.

The existing Source constructor has the same explicit field-write order,
including both profile writes. Current normal MSVC Win32 production COFF
contains the complete **62-byte / 13-instruction** ordinary C++ function
`?construct_native_physical_stream_00bf50d0@bsp@@YAPAXPAX@Z`.
It loads the receiver from `[EBP+8]`, not incoming ECX, and has no relocation
to a Source-owned profile: both `CEB130` and `D691B0` are literal immediates.
This is concrete evidence of an ordinary service interface, not an original
register-ABI constructor or a Source table publication. The exact complete
production object occurs once in the current `bsp_core.lib`.

Current Source BF3770 obtains the shared lifetime lock, pops a current pool
cell or allocates 20h bytes through `singleton_lifetime_allocate`, and invokes
that constructor on nonnull storage. It preserves its existing allocation
cleanup and unlock paths. The open-provider path acquires this storage,
builds the path, calls the ordinary BF5590/BF52A0 services, then checks the
numeric physical profile and validity slot. On a failed open, its existing
reference decrement/recycle path uses the ordinary lifetime services.
This identifies the real producer; it does not newly validate that larger
allocator/pool/open/recycle dependency chain at raw ABI.

### BF5520: alternate Original constructor, absent in Source

The complete body BF5520..BF558D is **110 bytes / 31 instructions**. It takes
ECX=receiver and two stack arguments forwarded to BF52A0, preserves ESI,
returns the receiver in EAX and executes `RET 8`. It builds an exception
frame referencing `00CC7D38`, publishes base profile CEB130, sets references
to one, zeros +10/+14/+18/+1C, then publishes D691B0 and invalid HANDLE+8
immediately before its sole direct call to **BF52A0**. Field +C is untouched.
There is no call to BF50D0: these initialization instructions are inline.

Its publication order differs from BF50D0, and its post-call HANDLE/output
state is determined by the open body. The exception handler and open body
are exact dependencies, not newly admitted contracts. Neither was expanded.
No BF5520 reference exists in current `src` or `include/bsp`; the existing
ordinary acquire-then-open path is not a proof of this constructor's raw
ABI, exception frame, publication ordering or unwind equivalence.

## Profile words and dispatch identity

A fresh 64-byte window at D691B0 was compared byte-for-byte with the installed
image. The first thirteen observed words are:

| Offset | Original target | Current boundary |
| --- | --- | --- |
| +00 | BF55A0 | Ordinary recycle/lifetime service; raw class lifetime not admitted. |
| +04 | BF5090 | Ordinary deleting-destructor service; raw destructor not admitted. |
| +08 | BF4EE0 | No standalone function start in the current index; provider not expanded. |
| +0C | BF4FF0 | Ordinary query-type service borrows live ID storage. |
| +10 | 6F9D20 | Indexed shared return-zero/RET8 leaf; no new provider admission. |
| +14 | BF4EF0 | No standalone function start in the current index; provider not expanded. |
| +18 | BF5020 | Separately qualified raw entry exists; no table reference to it. |
| +1C | BF4F20 | Separately qualified raw entry exists; no table reference to it. |
| +20 | BF4F40 | Ordinary physical-position interface. |
| +24 | BF5030 | Ordinary read service requiring its real context. |
| +28 | BF4F50 | Ordinary physical-write interface. |
| +2C | BF4FA0 | Ordinary OS file-size query interface. |
| +30 | BF4F90 | Ordinary cached-size interface. |

This is an observed profile prefix, **not proof that thirteen words are the
complete class table**. The next three captured words are BE42E0, BE4300,
BE4340; their membership is unclassified. No missing slot was replaced by a
null, stub, generic callback or an Original numeric address presented as a
Source implementation. Complete table extent, base/type identity and each
published slot's real provider remain part of the class contract.

The previous receiver audit's installed image hash and complete BF1090,
BF10E0, BF5020 and BF4F20 span hashes were revalidated against the current PE,
without rerunning its fixture. Its earlier statement that the two physical
raw entries were absent is now superseded by their separate implementations.
Its class/table restriction remains relevant:

- Original BF1090 reloads source=`[substream+8]`, its current table, and
  table+18h, then tail-jumps. For physical D691B0 that word is Original BF5020.
- Original BF10E0 publishes the wrapping absolute substream position before
  reloading source/table/+1Ch and calling with ECX=source, absolute low/high
  and origin zero. It returns the provider's EAX and uses `RET 0Ch`. For the
  physical profile that target is Original BF4F20.

Those receiver bodies do not themselves restrict the source to a physical
profile. Current `NativeVfsRuntimeBindings::source_is_open/source_seek`
switch on Original entry numbers and invoke the ordinary physical helpers.
The two new raw physical symbols appear only in their declarations and
definitions in `src`/`include/bsp`. They are not registered by this dispatcher
or installed into a produced class table. The existing `CallableDispatch`
adapter can invoke a supplied entry but supplies neither class provenance nor
table ownership. Its C++ service vtable is not a physical-stream vtable.

## Next Source boundary and verification limits

The smallest concrete constructor candidate is BF50D0's complete 43-byte
body: no provider calls need expansion to preserve its receiver ABI and
field-write order. A separately named raw implementation preserving the
Original numeric profile writes could qualify that constructor contract on
actual storage. It would **still not produce a Source-owned physical class
table** or admit BF1090/BF10E0 composition.

To cross that later boundary, recover the complete profile extent and
base/type publications, bind all required slots to genuine ABI-qualified
Source providers, and establish which real constructor publishes that table
into the allocated owner with its reference/handle/recycle lifetime. A table
containing only the two convenient raw entries would bypass this work.
BF5520 additionally requires the actual BF52A0 and CC7D38 unwind contracts.
Borrowed backing accepted by BF5020/BF4F20 or the substream arithmetic leaves
does not establish any of these producer or ownership properties.

Fresh evidence lives in
`local/cc12_physical_stream_raw_table_readiness_20261008a/`.
`evidence.json` records complete constructor bytes/instructions/calls, current
COFF, exact archive membership, live xrefs, Source searches, and stable
pre/post hashes. The existing artifact's source/object identity was checked
against the prior sealed production build; no fresh build, CTest, native
execution, raw class composition or game validation is claimed here.
The tracked report is `reports/cc12_physical_stream_raw_table_readiness.json`.
