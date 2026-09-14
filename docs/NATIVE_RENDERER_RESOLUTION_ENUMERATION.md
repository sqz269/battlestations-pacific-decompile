# Actual renderer resolution enumeration

This packet implements the raw renderer path used by B32410. It does not use
the existing settings/options projection or its standard-container interface.

| Entry | Full inclusive range | Native ABI |
| --- | --- | --- |
| B27D80 enumeration | B27D80..B27E74, 245 bytes | ECX renderer; plain RET |
| 8D46C0 first-pair search | 8D46C0..8D4707, 72 bytes | ECX header, stack pair pointer; EAX index/-1, RET4 |
| 8D4750 pair reserve | 8D4750..8D47B7, 104 bytes | ECX header, signed stack request; RET4 |
| B1FFD0 comparator | B1FFD0..B1FFEC, 29 bytes | cdecl(two row pointers); EAX wrapped signed difference, RET |

All 450 bytes were verified between live Ghidra memory and the installed PE.
The raw search/reserve bodies are new providers. Existing projected enumeration
and comparator APIs are preserved; their additional raw APIs are recorded as
fragments to avoid claiming a second original function. Names are hypotheses.

The renderer's pair header is actual data +1C, count +20 and capacity +24; rows
are two DWORD values. Enumeration first captures IDirect3D9 from +1990, clears
opaque +19DC, then reads the captured interface's current table and calls slot18
with adapter0/format16h (X8R8G8B8). That returned unsigned mode count is captured
once. Each mode call reloads current renderer+1990 and its table, dispatching
slot1C with adapter0, format16h, current index, and the same 16-byte mode buffer.
No COM AddRef/Release or substitute query object is introduced.

The original mode buffer is uninitialized stack storage reused across calls.
The new source API exposes it as a separate borrowed 16-byte scratch argument
in EDX. Its caller must provide an initialized, valid, independent preimage;
the source does not clear it. COM writes persist across successful, failed and
positive nonzero HRESULT calls. Only HRESULT exactly zero is accepted. Width is
then read and checked unsigned >=640; only after that passes is height read and
checked unsigned >=480. Refresh rate and format output words are not consumed.
The explicit initial scratch input is a source interface, not a recovered
second native argument or retained diagnostic operation.

Accepted width/height are captured before search/reserve and remain the appended
values even if later allocation activity changes the mode scratch. Search uses
the actual pair header, preserving existing pairs and duplicates already present.
An absent pair grows only when current count equals current capacity; wrapping
doubled capacity is interpreted signed and clamped to one before reserve. After
reserve, current count and base are reloaded. A null computed destination skips
the row stores but still increments current count. Other renderer bytes retain
their existing values except side effects performed by actual providers.

Search captures base and wrapping end=base+count*8 once and compares addresses
unsigned. Empty or wrapping-backward intervals return -1 without reading the
key. For a nonempty interval it captures the key's first DWORD once, reads each
candidate first word, and only on equality reads candidate second then current
key second. The first match returns signed wrapping (cursor-base)>>3.

Reserve clamps signed request to one, compares signed current capacity, and
allocates the wrapping request*8 byte count through the existing shared lifetime
allocator. It reloads current count at each test and current base for each row.
Within a row, the source address is captured once; first DWORD load/store occurs
before second DWORD load/store. Destination-null checks are preserved. Current
old data is freed before replacement data and capacity are published; count is
not changed. No catch, bounds guard, rollback, or fresh-allocation cleanup is added.
Existing pair-array candidates add extent/overflow checks and rollback, use a
different minimum capacity, or own string rows; none supplies this raw contract
unchanged. The substantive shared allocation/free service is reused.

Enumeration always calls genuine CRT qsort, including when there are no modes
or fewer than two rows. It captures current count before current data, passes
row size8 and B1FFD0's raw comparator. Comparison loads both widths, returns their
signed wrapped difference when unequal, otherwise subtracts heights with the same
wrap. No sorting library is reconstructed. The host CRT's private algorithm,
error handling, and order under non-transitive overflow inputs are not asserted
identical to the original statically linked CRT.

An exception bypasses subsequent append/count/sort work without rollback of
prior mode writes, COM mutations, array publication or previous appended pairs.
Valid raw extents, initialized scratch and native wrapping-address reachability
remain caller obligations. Original native CRT/private-frame/SEH identity and
incidental register results remain outside the source compatibility claim.

The 8D4750 listing already reaches its RET4 through the early-return branch, but
free CALL8D47A5 leaves a nine-byte gap8D47AA..8D47B2 containing stack cleanup,
data/capacity publication and POP EBX. Root must clear that returning override and
recover the gap, preserving full endpoint8D47B7. The other three bodies are
complete. The worker made no Ghidra mutation; root owns names, saved old values,
listing repair, save and refreshed exports.

Verification is recorded in the report: eight verified seeds, strict Win32 build
with both CTests, full live/PE bytes, direct-call audit, and one focused fixture.
The fixture runs all four complete original bodies with only five direct CALL
operands and the pushed comparator address rebound to native child copies and
genuine host allocation/free/qsort. A scripted COM vtable exercises failed and
positive-nonzero HRESULT writes, later partial-output success, current-interface
rebinding, callback array mutation, duplicate suppression and sorted postimages.
Its first failed call writes the whole mode buffer; subsequent unwritten bytes
are compared as retained state. A real Direct3DCreate9 interface provides the
second original/source comparison. Empty search and wrapped comparator edges
are checked in the same executable. No original EXE address is invoked.

The archive freezes exact source, libraries, fixture, tool executables and logs.
Loaded runtime DLLs are separate from executable artifacts. Real COM slot target
addresses are mapped to their loaded modules inside the running x86 process;
canonical file paths come from handles opened by that process. PE machine is
measured from loaded images; later disk/archive hashes are not mapped-image
hashes. These checks do not establish game integration or gameplay correctness.
