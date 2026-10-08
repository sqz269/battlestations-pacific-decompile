# Raw property lookup readiness (CC12)

Whole Original `0043B8B0` is established; whole current raw Source is **NOT_READY**. The first missing whole code dependency is `0043B760`. Lookup calls it before reading a bucket, including an empty-map miss. Its body was not expanded in this packet. A new hash audit needs its own lease and Root agreement.

The clean worker branch was refreshed to `c71d6d188030e581cafca72f5e16ac7e54562c26`. Seventeen selected Source/evidence inputs match that main revision, with checkout newline conversion accounted for. This packet owns only `0043B8B0` and this document plus `reports/cc12_property_lookup_readiness.json`.

## Complete Original evidence

The actual body is `0043B8B0..0043B918` inclusive (`0043B919` exclusive): 105 bytes, 48 instructions, three direct calls, zero indirect calls and zero instruction gaps. Complete saved pseudocode and assembly were inspected. Assembly resolves the stack cleanup, output publication, conditional EBP save and pointer comparison order; no repair or mutation was needed.

Installed PE bytes at file offset 243888 in `.text` equal the current saved Ghidra bytes. Body SHA-256 is `be85d862f57d3ffbcd558f9e00282d6aa313ec36b7b069afb9e50a128b465145`. The project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; each live wrapper batch verifies project/program/language/image base. The installed executable is `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`. Local receipts also pin its complete file identity.

Both actual fallback cells have a first byte of zero in installed PE and saved Ghidra: node-key fallback `00E177E4` at PE offset 10581988 and query fallback `00E17654` at 10581588. These are `.data` raw file bytes in this PE, despite older accessor prose calling the latter BSS. This proves the inspected initial NUL byte, not runtime immutability, an entire global object, or a current Source binding. Their pointer roles remain distinct.

## Actual ABI and every path

ECX receives the actual embedded `108h` map (`114h` property bag +4). Two stack DWORDs are pointers to an actual `8h` query string header and writable bucket output. EAX returns a borrowed actual node or zero; both returns are `RET 8`. EBX, ESI and EDI are saved/restored. EBP is saved/restored only on the nonnull-head path and is untouched on the null-head path. ECX, EDX and flags have no additional preservation guarantee.

1. Save the query in EBX and map in ESI; push the query and call `0043B760` at `0043B8BA`. ECX still contains the incoming map at the call. EAX is used as the bucket. Stack flow requires that call to consume its one pushed DWORD; the hash body's other register inputs and algorithm remain unproved here.
2. Write the bucket to the caller output at `0043B8C3`, then load map head `[map+8+4*bucket]`. A null head returns zero immediately. Map count and profile are never read; no bounds check occurs.
3. With a nonnull head, save EBP and capture the query length once from query+0. At each node, compare node+0 length first. A mismatch advances through node+0C.
4. On a length match, use node+4 key data or fixed `00E177E4`; call actual query accessor `00419CA0` at `0043B8E2` with ECX=query. Pointer equality succeeds immediately. Otherwise retain the explicit query/node null checks, then call `__stricmp` (`00BF7FBF`) at `0043B8F5`, query first and node string second, with caller `ADD ESP,8`. A zero comparison succeeds; a nonzero comparison advances.
5. Return the first matching node, or zero at chain exhaustion. Restore the conditional EBP and other saved registers. There is no allocation, record access or lifetime dispatch, and the only owned-body write is the bucket output.

Actual reads are query+0/+4 through the accessor, map+8+4*bucket and node+0/+4/+0C, plus comparison string bytes. Lookup does not read node mapped record+8, node allocator page identity+10, map count/profile, bag owner/ordinal or record profile. It does not copy a key, mutate a chain, own the result or destroy a record. There are no x87, REP or new DF requirements in this body.

The genuine insertion caller already sealed in the prior audit is `008F28F0`: at `008F28FF`, ECX is the actual map, EBX retains the actual query header, and the output pointer aliases the caller's first argument stack cell after its value has been retained in EBX. The lookup output therefore does not alias a query/map/node field. All ten direct caller references are saved; other caller bodies were not expanded or admitted merely from their names.

## Current providers and admission limits

| Dependency or input | Current evidence and readiness |
| --- | --- |
| `0043B760` | No whole actual Source implementation or reconstruction record. Ledger names `_toupper` (`00BF924E`) as its callee; both bodies stay unexpanded. Different enum hashes `004895B0`/`00489610` cannot establish this algorithm or placement. |
| `00419CA0` | Actual `native_string_data_or_00419ca0` is present in `src/native_lua_script_overrides.cpp`; reads actual header data and accepts an explicit external query fallback. Its historical tests were not replayed. |
| `00BF7FBF` | Genuine current `_stricmp` CRT use exists in raw string/enum Source. Its established closed NUL-free ASCII/C-locale comparison domain can supply a boundary; it does not prove whole Original CRT, other locales or non-ASCII behavior. |
| Empty cells | Query-role Source bindings exist for `00E17654`; an actual property-node `00E177E4` binding was not found. An enum fallback `00E186ED` is a different cell. No Source global/default was manufactured. |
| Actual map/node storage | Raw `008F41A0` constructor, actual raw key/string services and genuine `14h` node-pool services are current. The constructor creates an empty actual map. Whole property insertion and a current populated property-map producer remain unclosed. A semantic bag or an enum map does not establish property hash placement. |

A complete admitted borrowed-input domain would require actual stable `8h` headers; valid terminated key storage; finite actual bucket chains with nodes at the native offsets; compatible actual hash placement in buckets 0..63; valid writable output separated from fields subsequently read; explicit fallback bindings; and a comparison domain covered by the genuine current CRT boundary. Null data is admitted only for an empty valid key. The present evidence does not close that entire domain. Pointer identity checks and publication order must survive any eventual raw Source translation; query length is captured once while query data is obtained for every matching-length candidate.

Empty-map, nonempty fresh-key and replacement lookup paths all require the missing hash. Lookup itself treats node+8 as opaque. After lookup returns a hit, `008F28F0` can call the old record's profile slot zero and replace node+8. The previously admitted type-6 replacement path still needs whole current raw `004E6730 -> 008F0DE0 -> 008F0640`; other profiles remain unresolved. Those are downstream insertion dependencies, not lookup callees, and were not expanded here.

No independent implementation packet is ready. The precise next bounded dependency packet is a whole read-only `0043B760` hash audit: recover its complete bytes, physical ABI/register use, actual `_toupper` boundary and bucket algorithm, then establish compatibility with real property producers and the current admitted string/CRT domain. Closing it would still leave property fallback binding and genuine populated-map admission to resolve; it would not close insertion replacement lifetime, recursive destruction, profiles, class/EH/factory/world, game ABI or gameplay.

Raw evidence is under `local/cc12_property_lookup_readiness/`: complete assembly/pseudocode, bytes, flow, all xrefs, call contexts, installed-PE receipt, current Source searches/17 input pins and artifact manifest. The report's four direct call rows include the prior genuine insertion binding and are mechanically checked against live Ghidra. This audit executed no Source/native code, build, new test, fixture replay or game; it changed no Source, CMake, ledger, config, shared export or Ghidra data.
