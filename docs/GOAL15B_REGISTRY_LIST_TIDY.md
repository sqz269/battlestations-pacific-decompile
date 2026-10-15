# Registry interior-list tidy

`0041D710` destroys the list nodes and sentinel of an existing `0Ch` list
header. `0041E290` is its exact five-byte tail-jump thunk. This differs from
the payload-owning registry clear at `0041DED0`: tidy never reads node payloads.
The source uses the genuine `singleton_lifetime_free` CRT service and introduces
no host callback, allocator policy, outer owner record or replacement list.

| Native routine | Inclusive body | Original ABI | Coverage | Source entry |
| --- | --- | --- | --- | --- |
| `STL_List_Tidy_0041d710` | `0041D710..0041D757` | ECX list header; no stack arguments; plain RET; ESI saved, EDI saved on node-loop path | complete | `tidy_native_registry_list_0041d710` |
| `STL_List_Clear_0041E290` (saved old name) | `0041E290..0041E294` | tail JMP to `0041D710`, same input | complete | `tidy_native_registry_list_0041e290` |

The source interfaces use MSVC Win32 fastcall with one pointer, but do not
claim original register/stack equivalence, emitted tail-jump bytes, or the
original CRT heap identity. Native return registers are not a defined result.

## Producer and caller reconciliation

The native list header is operated on as actual bytes, not a new struct:
`+0h` is an untouched proxy word, `+4h` the sentinel pointer and `+8h` the
count. This agrees with the existing registry declaration in `cc13_avoid1.hpp`
when the header starts at registry `+4h`; no inference about another outer
class is needed.

Registry construction `00424730` establishes the observed header:
`00424752` computes registry `+4h`; `00424763` obtains a sentinel from
`0041BFD0`; `00424768` stores it at header `+4h`; `0042476B` zeroes header
`+8h`. The node producers were checked in pseudocode and assembly:

* `0041BFD0..0041BFE9` allocates `0Ch` bytes and makes the `+0h/+4h` links
  self-referential. The PUSH 0Ch allocation is balanced by ADD ESP,4 at
  `0041BFD7`; the payload word is not initialized by this producer.
* `0041C1F0..0041C220` allocates `0Ch` bytes, then stores supplied next,
  previous and payload words at `+0h/+4h/+8h`. Its three stack arguments are
  established by RET 0Ch at `0041C220`; the allocation's own one argument is
  balanced by ADD ESP,4 at `0041C1F7`.

All native references were inspected. Registry destruction supplies the
interior address with `0041F100 LEA ECX,[ESI+4]` immediately before
`0041F103 CALL 0041D710`. Both unwind entries also obtain an outer object from
`[EBP-10h]`, add four and tail-jump through the thunk:

* `00C5E278`: `00C5E27E JMP 0041E290`, after ADD ECX,4 at `00C5E27B`.
* `00C5E668`: `00C5E66E JMP 0041E290`, after ADD ECX,4 at `00C5E66B`.
* Thunk: `0041E290 JMP 0041D710`.

These body instructions establish tail jumps even though the xref output
classifies the unwind references as calls. The report marks them `tail_jump`.

## Complete cleanup schedule

1. `0041D713..0041D716` loads the sentinel and captures its old next node.
   A zero count does not bypass these reads, and a null sentinel is not guarded.
2. `0041D718` makes sentinel.next self-referential. `0041D71A..0041D71D`
   reloads header `+4h` before making sentinel.previous self-referential.
3. `0041D720` compares the captured old front with the current sentinel.
   `0041D723` writes count zero without changing the comparison flags; its
   branch is therefore based on node identity, not count.
4. If there are nodes, `0041D730` captures node.next before freeing the node
   at `0041D733`. `0041D73B` compares that captured next with the freshly read
   header `+4h` after the external free. Repeat while they differ. The source
   preserves the reload and does not dereference the released node.
5. `0041D743` reloads the current sentinel; `0041D747` frees it. Finally
   `0041D74F` nulls header `+4h` after the external free returns.

Both free calls use the retained library `_free` entry `00BF65AC`; each has
one pointer stack argument, confirmed by ADD ESP,4 at `0041D738` and
`0041D74C`. Its previously inspected thunk jumps to CRT `_free` at `00BF9DC8`.
The composed `singleton_lifetime_free` body invokes `std::free`; it is not a
no-op. Memory must belong to the matching source CRT heap. No allocation,
payload release, null-sentinel recovery or retry policy is added.

Header `+0h` is not read or written. Count is zeroed once before freeing nodes,
with no final overwrite of any hypothetical external changes to that word.
The current sentinel is reloaded after node frees, and the final pointer null
store follows sentinel release. This packet asserts normal valid-list behavior
and the explicit access schedule; it does not invent supported mutation hooks
inside CRT free or promise recovery from malformed chains.

## Name correction, verification and limits

`STL_List_Tidy_0041d710` is retained as the established STL helper name.
The thunk's saved old name was `STL_List_Clear_0041E290`, with old ledger
evidence: "Hypothesis, not a recovered symbol. docs/CC13_BAND_0040_0044.md.
same as 0041E200 for the 0041BFD0 list type; Unwind callers". The justified new
name is `STL_List_Tidy_0041e290`: it jumps to tidy, which frees the sentinel and
nulls the header, rather than leaving a usable empty list. Names remain
descriptive hypotheses, not recovered template symbols. This packet changes
only the two leased ledger records; primary handles saved Ghidra annotation.

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Live
identity/prototypes, xrefs, complete owned assembly, producer assembly and all
caller argument setup were checked. No Ghidra mutation or flow repair occurred.
Focused MSVC x86 `/std:c++17 /EHsc /W4 /permissive- /c /Iinclude` compilation
passed with zero warnings. The live checker passed all eight concrete report
rows and four markdown call/jump claims with zero failures or warnings.
`git diff --check` passed. Primary registers `src/native_registry_list_tidy.cpp` and runs the full Win32
build and existing checks. No new tests, native differential test, game
execution, ABI compatibility or runtime-equivalence claim is included.
