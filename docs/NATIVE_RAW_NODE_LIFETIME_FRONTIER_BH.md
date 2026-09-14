# Raw structured-node lifetime discovery BH

Addresses: 00BEA680, 00BEA250, 00BE9ED0; bounded provider bodies listed in the report.
Base: e55c2a254e08d8593ce868ddc07af66d09c57c47. Discovery only; no C++, build,
probe, game execution or Ghidra write. The companion JSON has every inspected
CALL site, native operand, containing function, stack/register evidence and the
complete local evidence inventory. All 27 code/vtable/EH spans match the installed
PE SHA256 b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.

## Recovered raw contracts

BEA680 takes ECX as a valid four-byte parent wrapper and one output-wrapper
pointer on the stack; RET4 returns that output pointer. It calls BF681B(24h),
constructs with ECX allocation and stack parent=[wrapper], and publishes the
returned node. It has no parent/null/remaining/attachment guard. Its explicit
null-allocation branch publishes null, although inspected BF681B attempts
allocation/new-handler retries and ultimately constructs a thrown exception.
Do not replace that exceptional path with a silently failed unique_ptr.

BEA250 initializes this exact 24h object, in observed order:

| Offset | Producer and meaning |
|---|---|
| 00 | Base profile CEB130, then concrete profile D68BB4 |
| 04 | Intrusive refcount 1 |
| 08 | Parent+08 reader pointer, borrowed; no retain |
| 0C | Parent node pointer, borrowed; no retain |
| 10/14 | Native eight-byte {DWORD length, data pointer}, initially zero |
| 18 | Parent+18 plus one, wrapping DWORD depth |
| 1C | Declared payload from BF0280 |
| 20 | Same BF0280 return, remaining payload |

The constructor first reads a temporary counted name through BF0510(reader,
temporary, &parent.remaining). It resizes the name header through 41DD40(length,1)
and copies the current destination length bytes with BF7680. It frees the
temporary through 419CC0/BD1510(data,length+1,1). It then calls external BF0280
with the same reader and parent budget, publishes +1C/+20, captures path slot
reader+10h+8*old(reader+60h), increments reader+60h BEFORE resizing/copying that
slot from the node name. No path capacity test, rollback, std::string, early
short-read guard or unique parent ownership occurs here.

BF0510 dispatches [reader+0].vslot48 with ECX actual stream, stack output8h
header and actual-count pointer. The count pointer aliases its incoming output
argument stack slot, not the separately zeroed local. The provider must overwrite
it. After return BF0510 subtracts that DWORD from *budget unchecked and returns
the original output header; RET8. This packet establishes the dispatch contract,
not a concrete source provider for arbitrary stream profiles.

BE9ED0 captures node=[ECX wrapper]. Null skips everything. Otherwise it calls
InterlockedDecrement(node+4), invokes node slot00 only if the new count is exactly
zero, then clears the wrapper after the call returns. Negative counts also clear
the wrapper without deletion. It does not clear before dispatch, loop, retain,
throw on underflow, or pass a deleting flag itself. If the dispatch unwinds, the
post-call wrapper clear has not executed.

D68BB4 contains slot00=BD30E0 and slot04=BE9FC0. BD30E0 receives only ECX,
loads the current slot04, pushes flags1 and calls it. BE9FC0 preserves owner,
calls BE9DF0, frees through BF65AC only when flags&1, returns the original
pointer in EAX and RET4. BF65AC is a tail jump to BF9DC8. Native address words
are profile evidence; they are not callable pointers into the reconstructed EXE.

BE9DF0 stamps D68BB4. If reader is nonnull it subtracts the FULL declared
node+1Ch from nonnull parent+20h, decrements reader+60h, and clears node+08.
It neither seeks nor clears remaining, and does not destroy the path copy.
It releases nonnull node.name.data through the sized pool, then BD30F0 stamps
CEB130 without changing count or freeing allocation. This is distinct from
explicit BE9C40 detach; BE9C40 was reserved but its body was not inspected here.

## Cleanup and allocation boundaries

FH3 maps and all action bytes were read through verified BSP bytes queries,
compared in full to PE, and locally decoded. Handler/action fragments were not
defined in Ghidra; failed handler disassembly attempts are retained in history.

* BEA680 E01C64: state0 -> -1 frees saved allocation through CC71F0/BF65AC.
* BEA250 E01B94: state0 -> -1 base cleanup CC7150/BD30F0; state1 -> 0
  node-name cleanup CC7158/41DD20; state2 -> 1 temporary-name cleanup
  CC7163/41DD20. State1 is armed before BF0510 and state2 after its return.
  Temporary cleanup disarms state2 first. No action restores reader path state.
* BE9DF0 E01A90: state0 -> -1 invokes base cleanup CC70A0/BD30F0. Therefore
  base stamping still occurs if the string-pool getter/return path throws.

Node storage comes from BF681B; counted name and path buffers use the published
01090AA8 sized pool via 419CC0. BD1120 takes (size,1), RET8; size >=150 uses
BF9F1A, smaller sizes use the concrete pool. BD1510 takes (pointer,size,1), RET12;
size >=150 goes to BF9DC8, small returns may be disabled by 01090AA4. These are
separate allocation domains. Inspected getter/pool bodies and their direct
provider frontier are in the report; no newly fabricated allocator is proposed.

## Smallest faithful next source packet

Ready: BE9DF0 alone, with new `include/bsp/native_structured_node_destruction.hpp`
and `src/native_structured_node_destruction.cpp`. Accept a valid actual24h node
and existing `NativeStringRawPoolContext`; implement the exact schedule above,
including base cleanup on a string cleanup exception. Reuse the raw overload
of `destroy_native_string_header_0041dd20` in `src/native_string.cpp` and the
existing actual Win32 `destroy_native_ref_counted_base_00bd30f0`. This closes
a source-level routine without invented calls or globals. Native FH3/SEH identity
and drop-in ABI compatibility remain separate work.

The complete allocation/release chain is not source-ready: BF0510 needs a concrete
stream vslot48 provider; BF0280 remains the external scalar packet; BEA680 needs
authentic allocation/EH composition; BE9FC0 needs matching free. Existing
`NativeRefCountedDeleteCalls` exposes a provider boundary, so BE9ED0 cannot be
called source-closed by merely supplying that interface. The existing typed
StructuredReader path has readiness guards, host budgets, std::string and
unique_ptr ownership that do not satisfy these contracts. Predicate715BF0's
unmerged source is independent and supplies none of these missing lifetimes.

## Verification boundary

`verify_report_calls.py`: 36 direct call rows checked, zero failures; 11 indirect
rows are explicitly outside its target verification. Slot00/04 resolution is
instead supported by the complete D68BB4 bytes and provider bodies. The report
contains 47 call rows. No fixture, compiler or gameplay result is claimed.
Local evidence under `local/raw_node_lifetime_bh/` is retained in this worktree;
the report embeds sizes, SHA256 and SHA512 for every evidence file, outside the
inventoried folder to avoid self-hashing. Early failed exploratory commands are
summarized honestly; their exact stdout remains in the conversation transcript.
