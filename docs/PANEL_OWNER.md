# Panel construction and ownership

This packet supplies concrete construction and destruction for the canonical
`VoicePanelState`, including the actual Win32 palette tree used by the existing
configuration loader and palette lookup. Names are descriptive hypotheses.
The C++ owner is a semantic projection, not a binary-compatible 38h replacement.

## Construction and startup contract

At004DC849 startup requests38h through00BF681B, calls00452660 at004DC862,
publishes its result to game+21E4 at004DC872, then calls0044FA30 at004DC878.
Publication therefore precedes loader callbacks. Root owns that startup body.
The native allocation-null branch does not guard the following loader call;
the established concrete allocation transport returns storage or throws.

The host allocates `sizeof(VoicePanelState)` using the existing concrete
`singleton_lifetime_allocate` transport with native_bytes38h, then placement
constructs the C++ owner. `prepare_panel_owner_allocation(owner, words)` is a
new, unaddressed allocation adapter. Its REQUIRED `PanelOwnerAllocationWords`
contains the captured allocator words at native+4,+10,+1C and the original
pause+30/state+34 bits. These are input state, not inferred defaults.
Preparation copies the pause as integer bits, without a floating conversion.

`construct_panel_owner_00452660(owner)` writes the00CE4BC0 vtable identity,
initializes the real palette, resets queued count and the two current-string
words, and returns that same owner. It preserves all five supplied words.
Both preparation and construction set `palette_10` to the actual embedded
`owner_native.palette_10` header. The owner must retain its address while live.
Calling the constructor on existing resources is invalid usage; as in native,
its current-string stores do not first release old contents.

The native constructor initializes three empty map headers.0044AB00 allocates
a1Ch character sentinel;0044AB50 allocates a24h palette sentinel;0044AC00
allocates a58h sequence sentinel. Each writes only its three links and color1,
isnil0. The owner sets isnil1, root/minimum/maximum=self and count0. Native
allocator/comparator words, sentinel payload and padding are untouched.

Character and queue storage reuse the existing standard-map projections and
their real C++ constructors. They do not receive unused native shadow nodes.
Those containers' sentinel allocation sizes/order, debug-iterator storage,
native count/layout and allocation-exception paths are library boundaries.
In particular their host sentinels live until the C++ containers are destroyed;
this packet does not claim native sentinel deallocation timing for std::map.

`PanelPaletteTreeStorage` IS the actual0Ch Win32 header: opaque word0, head4,
count8.0044AB50 uses concrete allocation through the established malloc/new
handler retry transport, then writes links0/4/8=null,color20=1,isnil21=0.
It leaves key+C, four value words+10..1C and padding22..23 untouched. The owner
then installs the head with all three self links, isnil1 and count0. Existing
0044EC00/0044E7B0/0044D2C0 operations use this storage directly.

## Destruction and reached cleanup branches

00452040 writes the vtable identity and releases current NativeString+28 first.
The current header remains unchanged, including any writes made by its release
callback. It then clears/frees the queue, clears/frees the palette, and finally
clears/frees the character map. Pause/state and opaque words remain unchanged.

The owner constructs each range from the current container, current head's
minimum and that same head. The checked iterator owners are the same nonnull
object and no callback occurs before the full-range gates. Therefore the
begin..end branches of004517E0,0044E150 and0044F0A0 are the reached branches.
The reconstructed helpers deliberately do not accept arbitrary iterators;
they do not stub the other ranges or claim those branches reconstructed.

004512E0 walks right subtree, captures left, destroys the current sequence
entry through existing0044FFF0, releases its key, frees the node and walks
left. This is reverse key order. The typed projection invokes existing
00450110 (entry then key) on the CURRENT linked pair, then erases that library
node. It does not reuse00451020, which unlinks BEFORE the ownership callbacks.
Native00451838 resets exposed queued count only after the entire drain; the
projection likewise keeps canonical `field_24` unchanged throughout callbacks.
Its `std::map::size()` is library state and is not the native+24 count.

0044AC90 performs actual native palette subtree destruction: recursively clear
right, load left AFTER that recursion, free the current node, repeat at left.
Sentinel byte+21 terminates it. No key/value destruction occurs. The reached
0044E150 branch reloads the head for each root/minimum/maximum store and sets
count0 after subtree destruction.004520DD frees the head; only afterward does
004520E2/E5 zero its pointer and count. The opaque word remains untouched.
The canonical `palette_10` projection continues to identify the embedded header,
whose head/count are now zero; it is not a separately owned heap pointer.

0044E4F0 has the same reverse traversal with pooled NativeString key destruction.
The character projection releases each current key before erasing its node.
The previous unaddressed ascending `clear_panel_character_map` is intentionally
not used for native owner destruction.

The standard maps preserve payload/key ownership order and the separate queue
count, but do not expose native freed tree links. Mutating tree topology from
a destruction callback, traversing native links to already-freed nodes, native
allocator interception and native STL/debug-iterator ABI are outside these
projections. Callbacks must leave the captured pair, command vector and slot
storage valid. Existing entry cleanup keeps its command-cell null store after
the deleting callback. Native SEH unwinding parity is not reconstructed.

00452720 always invokes00452040, then tests the low flag bit. When set, the C++
wrapper ends the empty standard containers' host lifetime and invokes concrete
CRT free. When clear, it leaves the host object storage available for inspection
and explicit host lifetime termination. Both paths return the original address;
the deleting result must not be dereferenced. No second owned-content cleanup
is performed after native destruction, since the current string header is stale.

## Evidence and exact bounds

All owner and allocator assembly was read; hidden ECX inputs and RET cleanup
were checked against the listing. Shared exports were refreshed after repairs.
For00452040, the complete native tail is decoded directly from disk and verified
against Ghidra memory by the bounded repair tool, rather than trusting its
truncated stored function body.

| Address | Native ABI / source scope | Last instruction and inclusive end |
|---|---|---|
|00452660|ECX owner; EAX=this; RET; typed constructor|00452718 RET, length1, end00452718|
|00452720|ECX owner; flags stack; EAX=this; RET4|0045273B RET4, length3, end0045273D|
|00452040|ECX owner; RET; typed full ownership sequence|0045212E RET, length1, end0045212E|
|0044AB50|ECX unused; EAX node; RET; actual allocator|0044AB86 RET, length1, end0044AB86|
|0044AC90|ECX tree, node stack; RET4; actual full subtree|0044ACC2 RET4, length3, end0044ACC4|
|004517E0|ECX queue, output and two8h iterators stack; RET14; full-range branch only|004518A6 RET14, length3, end004518A8|
|0044E150|same ABI for palette; full-range branch only|0044E216 RET14, length3, end0044E218|
|0044F0A0|same ABI for character map; full-range branch only|0044F166 RET14, length3, end0044F168|
|004512E0|ECX queue, subtree stack; RET4; full-root semantic drain only|00451372 RET4, length3, end00451374|
|0044E4F0|ECX character map, subtree stack; RET4; full-root semantic drain only|0044E53F RET4, length3, end0044E541|
|0044AB00|native1Ch empty-node allocator; library evidence only|0044AB36 RET, length1, end0044AB36|
|0044AC00|native58h empty-node allocator; library evidence only|0044AC36 RET, length1, end0044AC36|

The library/compiler nature of the cleanup and scalar-deleting helpers is
retained. In particular existing `CG_scalar_deleting_dtor_00452720` is not an
incorrect game-name classification requiring replacement. Central naming and
ledger integration belong to root; this worker supplies scope/evidence rows.

## Analysis repairs and remaining boundary

Scoped repairs were explicitly authorized when this packet was assigned.
`reports/panel_owner_flow.json` records all five locked repair operations and
the save. Internal free fallthroughs at00452735..37,00451355..5F,
0044ACB4..BE and0044E531..3B now have complete stored listings, with no remaining
call gaps. No callee no-return flags, scripts, imports, deletion or recreation
were used. After root restored worker read-only status, no further mutations
were performed.

00452040 still has stored body00452040..004520B3. Explicit decoding covers
004520B4..0045212E (123 bytes) and verifies final RET0045212E,length1. Overrides
at004520AF,004520DD and0045210F were cleared; tail bytes are instructions, but
the bridge does not extend that stored body. A separate supported body-extension
operation remains required. This is an analysis metadata limit, not omitted
source control flow. No missing function starts were found.

## Validation

`./scripts/build.ps1` passed MSVC Win32 Release and the existing configured
reconstructed_math test (1/1). Log: `local/panel-owner-build.log`.

One ignored fixture populates real palette nodes through0044EC00, installs two
owning queue entries with real deleting command callbacks and pooled slot/key
strings, and installs two character names. It verifies required word/NaN-bit
preservation, actual sentinel links, real palette insertion/teardown, current
string release before reverse queue callbacks, still-linked current entries,
unchanged queued count during callbacks, count reset before character cleanup,
reverse name release, no remaining pooled strings and scalar deleting-bit flow.
It passed against bsp_core.lib SHA256
0E6B600C2CC94A4C9661F5A1A09A3F424F2A02D2865E3E421DD347824180D421.

Source: `local/panel-owner-probe.cpp`; log: `local/panel-owner-probe.log`;
parameterized recipe: `./local/run-panel-owner-probe.ps1 -SourceRoot <checkout>`
with optional `-CoreLibrary <bsp_core.lib>`. No reconstructed source is copied.
This proves compiled and focused fixture behavior, not native ABI, complete
startup, native allocation-failure unwind or gameplay execution.
