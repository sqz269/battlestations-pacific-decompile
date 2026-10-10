# DamageableClass section iterator advance Source

The ordinary Source fragment implements only `[0087D1A0,0087D1CF)`, 47 bytes /
12 retained instructions inside parent `0087CA80`. It advances the existing
Sections iterator through the genuine protected Lua provider, then returns
whether the same key remains bound. The preceding 741-byte field body and its
effect-provider gap remain outside this implementation.

The new function is
`advance_native_damageable_class_section_iterator_0087d1a0`. Its two references
borrow the successful existing iterator setup owner and that owner's same
actual scratch. The owner reference records a well-formed lifetime precondition;
the function does not inspect private state, add accessors or verify identity.
Registration and the normal project build remain Root's integration work.

## Retained boundary and actual objects

The exact 47-byte hash is
`7ace1a66d9c3f246cda60da0bf50fe88192baa7ced611137a71a8418ee781865`.
Only these 12 instructions were decoded for this packet. Root's previously
reviewed 3,238-byte parent buffer, decoded index and accepted typed receipt were
copied from the frozen readiness bundle. Their hashes were verified; the other
parent instructions and 858-entry index are retained context, not a new analysis
window. No installed PE, live Ghidra, new callee, data, table or handler was read.

S is ESP after the parent's E4h locals and four saves. D1A0..D1B1 passes actual
value S+2Ch, key S+58h and Sections S+80h to B67190. The apparent ESP offsets
change with argument pushes; they do not identify different objects. D1B6..D1C2
passes the same key to B66420. Its existing contract ignores ECX/Sections and
tests the stacked key's kind. D1C7 tests AL and D1C9 branches to CE00 when the
key is still bound.

The Source result is consequently `!native_lua_is_unbound_00b66420(key)`:

| Result | Meaning and continuation |
| --- | --- |
| true | A bound key corresponds to the excluded CE00 back edge. |
| false | The caller reaches the already established D1CF pair cleanup. The owner remains live until that matching close. |
| exception | Propagate through the existing ordinary enclosing owners, retaining provider effects. No result or retry is supplied. |

The successful setup owner must still be in state13. All inner Lua fields,
strings and effect-handle owners must already have closed normally before this
tail. The same Damage/Sections and outer Unique owners remain alive. The tail
does not open, close or reconstruct any owner and does not write the row, vector,
saved cursor, field storage or private state counters.

## Genuine provider and failure contract

`native_lua_iterate_next_protected` receives the actual existing objects. Its
current preparation first releases the old value. For a top key it detaches the
tracked object while keeping the working stack value; otherwise it copies the
key to the top and releases/removes the old key. Completed tracked-index shifts
remain. The protected operation then executes real Lua next and publishes the
actual key/value addresses into the existing tracking slots on success.

The adapter starts protection after preparation, with one detached working key.
On Lua failure it removes that key and later operation/error temporaries, then
throws the existing `NativeLuaOperationError`. It does not restore the old pair
or the entry stack. The fragment adds no catch, guard, cleanup or rollback.
Ordinary enclosing owner unwinding handles the still-live pair once; its existing
noexcept secondary-failure behavior is unchanged and is not Native FH3 proof.

Keep the pair exclusively owned, with no surviving alias to the detached working
key. Actual object lifetimes, owner/interpreter identity, table, caller stack,
tracking capacity and scratch bindings must remain valid and stable. A surviving
inherited error handler must be below every removed/consumed slot, including
surrounding cleanup. Existing ignored-capacity-check and non-reentry contracts
remain caller obligations. No invented check, copied index, generic iterator,
fallback service or replacement error policy is supplied.

The existing setup and Damage owner bodies and their headers are frozen in the
receipt. Their source and prior accepted evidence establish the borrowed owner
contract; this free function does not add a new owner API or claim to complete
the full DamageableClass reader.

## Win32 compilation and complete object evidence

Both command files and compiler/include inputs are frozen. The first audit used
the older worker-generated Release project settings without an explicit `/Oy`
override. Root's actual Core command log additionally specifies `/Oy-`; a
separate read-only snapshot preserves its complete Lua provider command.
The second audit matches its relevant options, macros and include roots,
including `_MBCS`, with local output/listing paths and evidence options.

Both use `/O2 /Ob2 /MD /W4 /WX /fp:strict /EHsc /std:c++17` on MSVC Win32.
The second adds `/Oy-`. Neither audit links, runs a probe, regenerates the
project or substitutes for Root's registered normal build.

| Audit | Candidate physical sections / code | Actual Lua provider physical sections / code |
| --- | --- | --- |
| Worker Release, no explicit `/Oy` override | 6 / one function, 33 bytes, 13 instructions | 175 / 66 code sections, 4,163 bytes |
| Root command options, `/Oy-` | 5 / one function, 36 bytes, 16 instructions | 118 / 66 code sections, 4,240 bytes |

Every candidate code byte, physical section, symbol including AUX records,
relocation and non-code section is retained and reviewed. Complete raw objects,
section contents, raw symbol/string tables and every provider code decode are
included in the JSON. The candidate has exactly two project externals, both
resolved by exact decorated name to fresh actual Lua definitions:

| Provider | No override | `/Oy-` |
| --- | --- | --- |
| protected next | physical108, 65 bytes | physical64, 60 bytes |
| key unbound predicate | physical100, 12 bytes | physical60, 15 bytes |

The no-override candidate calls next at +0Fh, calls the predicate on the same
saved key at +15h and inverts AL at +1Dh. The `/Oy-` candidate does so at +11h,
+17h and +1Fh. Both contain no branch, owner-state store, local exception handler,
destructor or other call. Ordinary C++ argument/return conventions replace the
interior parent's register/stack control flow; no binary ABI claim follows.

The no-override object has one 16-byte FPO section with a SECREL relocation to
its defined function. The `/Oy-` object needs no FPO section. Neither has an
unwind map or SafeSEH handler of its own. The unreferenced four-byte weak
`__Avx2WmemEnabledWeakValue` is emitted by MSVC headers, not a reconstructed
Native global. Compiler directives/debug/checksum sections remain in the proof.

The complete selected preparation, protection, Lua-next publication, tracked
release and Source exception graph was inspected. The `/Oy-` provider's 66 code
sections and indexed relocations also exactly match the retained genuine setup
provider receipt. This is Source object evidence; no Native helper was reopened.
Weak exception aliases resolve through their actual AUX fallback symbols to the
existing deleting destructors. Real Lua 5.1.1 and MSVC imports remain library
dependencies. Full translation-unit external inventories distinguish unrelated
provider functions from the selected iterator/exception graph.

The receipt freezes both complete object variants, all 127 compiler include
files, current Source/dependency snapshots, baseline Git pins, the Root command
snapshot and five complete bounded Source/registration queries. The candidate
remains unregistered; no new test was added or run.

Root Source634's 634 selected inputs, 88 Core / 3 App objects, 228 positive
definitions and three existing checks are historical integration context only.
They are not a full-project count or new execution credit for this packet.
Production binding, whole-parent completion, original register ABI/FH3/SEH/
longjmp/fault identity, startup and gameplay remain held.
