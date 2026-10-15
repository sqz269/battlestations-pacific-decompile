# Grid list node producer

`STL_List_BuyNode_0041c0b0` allocates a raw `0Ch` node and stores three DWORDs.
Its established STL library-shaped name is retained as a descriptive hypothesis,
not a recovered template declaration. No hull node or payload class is inferred.

| Native entry | Inclusive byte body | Original/Source input shape | Coverage |
| --- | --- | --- | --- |
| `STL_List_BuyNode_0041c0b0` | `0041C0B0..0041C0E2`, 51 bytes | stdcall three DWORDs; EAX node; RET 0Ch; incoming ECX unused | complete |

The public Source entry is `buy_native_grid_list_node_0041c0b0(next,previous,value_source)`.
The first two inputs are raw DWORDs. The third is a readable DWORD-slot address,
not the payload itself. The allocated result is raw storage, not a fabricated
owner or payload lifetime. Its next/previous/value fields are `+0h/+4h/+8h`.

## Complete native schedule and aliasing

1. `0041C0B0` pushes `0Ch`; `0041C0B2 CALL 00BF681B` returns the allocation
   in EAX. ADD ESP,4 at `0041C0B7` balances the one cdecl allocation argument.
2. `0041C0BA..0041C0C2` checks EAX. Only if nonzero, load the CURRENT first
   stack cell `[ESP+4]` and store it at node `+0h`.
3. `0041C0C4..0041C0CF` computes node+4 with DWORD wrap and checks that address.
   Only if nonzero, load CURRENT `[ESP+8]` and store it in the previous word.
4. `0041C0D1..0041C0DE` computes/guards node+8. Only if nonzero, load CURRENT
   `[ESP+0Ch]`, dereference that source DWORD, then store it in the value word.
5. `0041C0E0..0041C0E2` returns with RET 0Ch. The allocation remains in EAX;
   no other instruction replaces it. The pseudocode's void result is incorrect.

The naked Source kernel retains every load/store, register, branch and stack
operation in that schedule. A returning allocation handler can change current
argument cells before their late loads; a next/previous store can affect a
later aliased input or value-source word. No by-value C++ snapshot is used.
The value-source stack cell and pointed DWORD are read only after both earlier
stores, and only when the value destination is nonzero. The conditional guards
apply to each computed destination, not a single graceful-null-return policy.
The kernel preserves modulo-32-bit address wrap. Reached spans and current
argument cells must be valid; no shape check, lock, fence, rollback or alternate
failure policy is added.

## Genuine allocation service and limits

The sole external kernel call is redirected to a concrete noinline cdecl
adapter. The captured byte-count argument is `12`, and the adapter invokes
`singleton_lifetime_allocate({object,bytes,bytes})`, hence native/source `12/12`.
That genuine existing provider implements malloc, new-handler retry and
`std::bad_alloc` on exhausted allocation. Its body and native `00BF681B`
counterpart were read in this worker stream. No fake allocator, substituted
success result, payload construction or allocator control hook is introduced.
The normal source provider returns valid nonnull raw storage or throws; the
kernel's original computed-address guards are still retained exactly.

The resulting allocation is compatible with existing `singleton_lifetime_free`
(`std::free`) in the source CRT domain. Native heap/new-handler/exception object
identity, provider private frames, fault/SEH addresses, original program binding
and gameplay remain qualified. Matching the owned kernel's bytes after call
relocation normalization does not prove those external properties. Source input
cells are its actual current cells, not an assertion that external native-frame
addresses or external frame walkers have identical backing.

## Sole caller and schema boundary

Live xrefs identify one caller: `004238B4 CALL 0041C0B0`, inside
`BSP_AvoidZoneManager_GenerateGridPaths_00423640` (`00423640`). The read argument
setup at `004238A2..004238B4` establishes:

* ESI is the current record+18h raw list header.
* EDI is header+4's sentinel; EAX is sentinel.previous.
* EDX is the address of the local payload slot at current ESP+1Ch.
* PUSH EDX, PUSH EAX, PUSH EDI supply value-source address, previous, next.

The callee's RET 0Ch proves all three consumed arguments; the caller's incoming
ECX header is unused by this complete producer. `004238BD` moves returned EAX
to EBP, then count increment and two link stores attach that node. The Source
does not name the pointed payload or equate this list with any hull-node class.
Only the observed `0Ch` node and raw list roles are used.

## Verification and transfer

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; live
prototype, complete assembly and sole-call argument/result setup were checked.
Saved prototype `undefined STL_List_BuyNode_0041c0b0(void)` and exported void
result are recorded as old values; primary owns any Ghidra prototype/evidence
annotation and refreshed exports. The correct existing STL name is preserved.

Strict MSVC x86 `/std:c++17 /EHsc /W4 /WX /permissive- /O2 /Gy /FAcs /c`
compilation passed without warnings. Actual COFF function bytes match all 51
native bytes after masking only the CALL rel32 operand at function offset+3h.
The relocation was also checked to target the concrete allocator adapter.
The normalized SHA256 is
`57c3a10fa9e65695622c202059d491accfd78f1421284ea00c0a775eeabed74c`.
The ignored `local/goal15d_grid_node/compare_grid_node_bytes.py` accepts optional
COFF-object and captured-native-byte-file paths for primary build rechecking.

The ignored real-provider Source probe passed allocation, three DWORD contents,
EAX result and matching free. It is rebuilt by
`local/goal15d_grid_node/probe.cmd`, using `grid_node_probe.cpp` in that same
directory, and links the actual singleton allocator/free with an embedded
manifest. It does not force OOM or separately exercise argument-cell aliases;
the complete normalized kernel comparison establishes their instruction
schedule. All two report rows and two markdown call claims passed with no
failures, unchecked claims or warnings. `git diff --check` passed.
No repository test target/framework, native differential or game-runtime proof
is added. Primary registers `src/native_grid_list_node.cpp`, builds, annotates
and integrates the packet. The lease is released on final transfer.

## Primary integration result

The registered Win32 build and all three existing CTests passed. PASS actual registered archive node allocation, three DWORDs, matching free. PASS51 native bytes except1 redirected allocator CALL operand. Worker-stage pending checks above are superseded by these actual registered results. Ghidra annotations are saved and exports refreshed; exact archive/object/fixture hashes, prototype rollback and evidence limits are in `reports/goal15d_reconstruction_batch.json`. No dynamic unwind, original ABI or gameplay proof is claimed.
