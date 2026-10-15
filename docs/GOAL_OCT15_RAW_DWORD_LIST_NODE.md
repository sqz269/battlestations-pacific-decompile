# Actual three-DWORD node allocation

The complete `00467BA0..00467BCA` body is 43 bytes / 15 instructions, with three
blocks and one physical transfer. Existing descriptive library name
`STL_List_BuyNodeC_00467ba0` is retained as a provisional shape hypothesis;
no caller or code/data xref establishes a nominal payload class or list owner.
No indexed or unindexed whole-body Source was found before this reconstruction.

Native ABI is one stacked DWORD at entry ESP+4, RET4 on both tails. Incoming
ECX/EDX are unused, no nonvolatile register is modified, and raw EAX retains the
allocation on the nonnull path. The null path clears EAX and would return zero
if its low-address write returns. The ordinary Source API exposes that raw
pointer result and binds the live parameter DWORD through `actual_parameter4`;
it does not establish the native register/stack calling convention or return type.

At `00467BA2`, `00467BA0` calls `00BF681B` with size `0Ch`.
The accepted concrete `singleton_lifetime_allocate` service uses an object
request with native/host size12, actual malloc, `_callnewh(12)` retry and
`std::bad_alloc`. Current service returns nonnull or throws. Original static
exception/global-handler/CRT identity remains a qualified boundary. No allocator
callback, stub, substitution with a nominal host node, or new policy is added.

Allocation precedes every parameter read and node write. After nonnull return,
zero DWORD node+0, then DWORD node+4. Only afterward read the CURRENT actual
parameter DWORD at `00467BB5`, store it into node+8 and return. Thus an allocation
handler or a valid parameter/node alias can affect the stored value; Source does
not snapshot the payload before allocation or before the two zero stores.

The complete native null branch reads the CURRENT parameter at `00467BBF`,
clears EAX, writes the value to literal address8 at `00467BC5`, and executes
RET4. Source retains that entire branch despite the current provider's established
nonnull-or-throw behavior. No early null return, null recovery or positive-only
wrapper removes it. Reached raw storage/access must be valid; ordinary C++ does
not claim native access-violation/SEH/instruction/fault-address identity.

The explicit parameter backing address is stable; its contents may change through
the described actual accesses. Binding metadata and Source/native private frames,
argument homes, return addresses and EH records have no implicit aliases to that
backing or allocation. No concurrency is supplied. All three DWORDs are raw words;
there is no invented signed key, object pointer ownership or payload destructor.
The body has no local EH frame, temporary, release, cleanup funclet, rollback or
additional validation. An allocator throw performs no caller-argument read or
node store, while all concrete allocator behavior remains composed.

## Verification

Whole assembly, original ABI, caller absence and the actual allocator contract
were checked. All 43 live bytes match the installed PE; the single typed native
transfer is verified separately from Source composition. Correct library naming,
prior comments/evidence and old values are preserved through locked annotation,
save, export and readback recorded in the report. Unique CMake registration was
appended only after an immediate unleased check, without an exclusive file lease.
The prior-value annotation log is `local/ghidra-annotations-20261015T215200Z.json`.

Required normal `scripts/build.ps1` at
`b95fe68d1b2ef79eb8a3cfdb343d030731f2b254` passed strict Win32 and all 3 existing
CTests in 7.78s. Log: `local/goal_oct15_raw_dword_list_node_build.log`.
Source/header hashes are unchanged from that draft. The primary integrator and
a second worker independently reviewed the entire native listing and exact
Source/header, without a mismatch; their reviewed Source hashes are in the report.

Focused emitted-code inspection confirms the real allocator transfer+29h precedes
all actual parameter reads. On nonnull, node zero stores+38h/+3Eh precede the
current parameter DWORD read+45h and node+8 publication+47h. The separate null
tail reads the current DWORD+5Bh, emits a literal DS:[00000008h] write+5Dh,
then clears EAX+62h and returns. The pure EAX clear moved after the write compared
with native code; the Source exposes the required zero result if the write
returns. Native fault-register/instruction/SEH/private-frame identity is unproved.
The actual 114-byte entry and 1750-byte object hash/relocations are recorded in
the report and `local/goal_oct15_raw_dword_list_node_object.txt`.
No new tests were added; packet-specific native differential/game and original
ABI remain unproved. Existing native math checks do not execute this new entry.
