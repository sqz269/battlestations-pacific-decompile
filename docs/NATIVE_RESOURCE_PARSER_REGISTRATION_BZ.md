# Actual resource-parser registration

Addresses: `00B80A50`, `00B80290`, `00B7FD30`, `00B7F610`, `00B7F1B0`, `00B7F340`, `00B7CA80`, `00B7D590`, `00B7E930`, `00B7E950`. Support: `00B7F682`, `00CC2060`, `00CC2068`, `00CC2081`, `00CC20B0`, `00CC20C1`, `00CC2130`, `00CC2138`, `00CC21B0`, `00CC21B8`, `00CC21C0`, and the shared cache-pair handler `00CC2051`.

The new module supplies complete actual-storage registration and its parser-map insertion dependencies. Ten ordinary bodies total1740 bytes and634 instructions; twelve support entries add39 instruction starts. All match the installed original PE and saved Ghidra ownership. The report retains all49 direct transfers and both indirect name-getter calls. Names remain descriptive hypotheses. Older typed registration/map modules are projections and are not recast as raw storage.

## Registration semantics

B80A50 consumes ECX actual manager and one stacked parser pointer, returns AL0/1 and RET4. Its parser tree is manager+8, head at manager+C, count at manager+10. Native1Ch nodes have links0/4/8, owned nameC/10, borrowed parser14, color18 and nil19. No parser reference or manager+20/+24 change occurs.

The first virtual+4 call receives an uninitialized actual8h output header. Lookup uses the getter's RETURNED pointer, which need not equal that output address. The current tree head is captured after the getter but before B7E740; the iterator owner is validated with the returning invalid-parameter boundary, then the current result node is compared with that captured head. The original local name is returned through the current pool before branching. An existing name returns false. This first local is never armed in the caller's EH map, including during lookup or its normal return.

On a miss, the routine reloads the current parser table and slot4. The second getter writes an actual by-value8h name; its return pointer is ignored. B7F340 copies that name into a completed pair, stores the raw parser pointer and consumes the input string. Only after it returns does caller state0 arm the completed pair. The caller copies it into a second pair and arms state1 before B80290. It then disarms/returns the second pair, disarms/returns the first pair and returns true without inspecting inserted BYTE8. Consequently, different first and second names can yield a true registration result while the map retains another parser at the colliding second key.

The source preserves the native reused local slots for the first name/result and lookup iterator/insertion pair. It captures the insertion string pointer after resize, before the source nonempty branch, and keeps that pointer across insertion for normal return. There is no added parser retain, first-name cleanup on exception, partial-name rollback, or insertion-result check.

## Reusing the proven cache specialization

Eighteen complete code pairs are instruction-identical after audited internal targets and descriptor addresses are substituted. They cover parser/cache rotations, node construction, allocation and its catch, owned-name pair construction, insertion, unique lookup/insertion, iterator decrement, pair destruction and EH actions/handlers. Three full EH-data groups also agree after the same substitutions: pair52 bytes, allocator96 bytes (including try/catch metadata), insertion44 bytes. Both original byte hashes and explicit address mappings are retained in the equivalence evidence.

The new parser entry points therefore reuse the integrated raw cache algorithms. Their actual layouts, ordered volatile reads, result-store order, length bound15555554, red-black link/color updates, borrowed mapped pointer and source C++ cleanup contracts coincide. These source reuse calls are not recorded as invented native call edges. Existing parser decrement B7CEF0 is compared with cache B7CDF0 but is not claimed as a newly reconstructed body. No std::map replacement or new ownership domain is introduced.

## Exception and Ghidra evidence

B7F340's FuncInfo DFB1FC and mapDFB1EC match the cache pair: state1 cleans the current by-value input through CC2060; state0 checks/clears the completed flag before CC2068 calls B7E930. B7F610's DFB288 metadata preserves the placement-delete401130 no-op and catchB7F682, which frees the captured allocation then rethrows without returning a partial key. B7FD30's DFB36C map destroys only its completed length-error message through CC2130. Registration's DFB418/mapDFB408 has state0->-1/CC21B0 for pair EBP-18 and state1->0/CC21B8 for pair EBP-24.

Five missing10-byte dispatch handlers were defined: CC2051, CC2081, CC20C1, CC2138 and CC21C0. The allocation catch was truncated after its free call; its proven returning call override was cleared and the function recreated over all21 bytes, preserving its compiler name and prior comment. No instruction bytes or callee no-return flags changed. All prior plate comments are preserved and affected exports refreshed.

The source transports ordinary C++ exceptions and follows those cleanup states; a failure during unwind cleanup terminates. It does not supply the original CRT exception object, FH3/SEH handler ownership, private native spill aliases, hardware-fault delivery or a callable native ABI.

## Validation and follow-up

The strict MSVC Win32 build and both existing CTests pass. One ignored controlled-child fixture compares ten source/original registration steps and two pair-destruction cases. It relocates eleven original ordinary bodies (the ten new bodies plus existing B7CEF0) with39 direct-call/tail patches. String allocation/return, comparison, lookup and allocation services use their existing canonical source/library bindings. Native error/throw paths are guarded out. Normalized complete trees compare links, colors, nil bits, root/min/max, count, keys and borrowed parser pointers after every step. Cases cover case-folded duplicate, true-on-second-name-collision, returned-first-header identity, empty key, current name-getter target changes, recoloring and both rotation directions. Parser marker bytes and unrelated manager fields remain intact. Pair destruction leaves the stale name fields and parser word intact.

The source additionally checks the actual insertion count-limit exception without a map or parser change; this is not an original-FH3 oracle. Getter callbacks are explicit fixture functions, not reconstructed Mesh/Camera/etc. getters. Teardown of the fixture tree is explicit test cleanup, not the manager destructor. Concrete parser singleton/name/body dispatch, actual manager bootstrap and destruction, executable admission and gameplay remain follow-up work. No new repository tests or original-game execution were added.
