# Point-effect child lookup

Addresses: `00866CD0`, `00866E60`, `0086B100`.

The three complete bodies now have typed C++ implementations in
`point_effect_lookup.hpp/.cpp`. Names are descriptive hypotheses. The code uses
the existing actual definition/component owners, point storage, borrowed child
references, current row factory, and canonical `00866440` manager/lifetime domain.
It introduces no second count, parallel definition array, or singleton cache.

| Routine | Native body, inclusive | Original ABI | Coverage |
|---|---|---|---|
| Find/create child by component name | `00866CD0..00866E5F`, 400 bytes | ECX point; stack output slot, native name header; EAX output slot; `00866E5D RET8` | complete |
| Find existing child by type | `00866E60..00866F41`, 226 bytes | ECX point; stack output slot, type DWORD; EAX output slot; `00866EFC` and `00866F3F RET8` | complete |
| Find named definition component | `0086B100..0086B1B3`, 180 bytes | ECX definition; stack native name, component output, index output; AL boolean; `0086B16C` and `0086B1B1 RET0C` | complete |

`86B100` captures the actual definition count before its backing pointer and
captures the end once. Each actual component has a native eight-byte string at
`+08`: length at `+08`, data at `+0C`. This is producer established:
`86BC80` initializes both words, and `870400` writes the Lua string key through
`41E350` into that same component header before retained array append. Tracer
uses its existing secondary component base; no allocation-base substitution occurs.
The lookup compares lengths first. Equal zero lengths match without inspecting
either data pointer; otherwise `BF7FBF __stricmp` compares data (its call is followed
by `86B150 ADD ESP,8`). The matching component slot is reloaded after comparison.
Failure writes neither output. Success publishes and retains the incoming actual
owner, releases the captured prior output, then writes the index. Identity skips
all reference operations. A terminal callback may replace the output; that
replacement is preserved. If it throws, the index is not written.

Both point methods capture the actual `866440` manager's `+04` section and keep
that section through completion; its physical `+18` recursion count is incremented
after Enter and decremented before Leave. Neither borrows the lifetime manager's
different `+10` lock. Name lookup loads the current point definition `+84` after
locking and retains the selected component while it might create a child.
Only a null indexed child invokes the current component virtual `+18`; there is
no autostart, admission, active-byte, point-option, or type gate. The factory
transfers one owned result. The destination backing is reloaded after the factory,
canonical `6FBEB0` assigns it, the captured result is released, and backing is
reloaded again before output publication. Native index/allocation preconditions
remain required; missing rows or short point arrays are not silently repaired.

The two output constructors intentionally differ when their output aliases the
selected entry. `866CD0` computes the source address, clears output, then loads
the source (`866DF2..866DF7`), so that alias returns null. `866E60` captures the
selected value first (`866EFF`), then clears output (`866F08`), so that alias
retains and republishes the child. Neither releases prior output contents; callers
normally supply fresh result storage. `866E60` captures signed count in EDX and
backing in ESI (`866E9D..866EAD`), scans that span for the first nonnull exact
`type+18` match, and never reloads point count/backing during iteration. A
nonpositive count returns null. The decompiler's repeated owner-field expressions
do not describe those register captures.

The name-call EH handler `C94DB1` references FuncInfo `DC6BEC` and map `DC6BCC`:
state0 conditionally releases constructed output (`C94D90 ->6CF070`), state1
unlocks (`C94D80 ->411EE0`), state2 releases/clears component
(`C94D88 ->866C30`), and state3 clears the event temporary
(`C94DA9 ->6CF070`). Normal component cleanup disarms state2 before release.
Consequently a factory exception releases component before unlock and leaves
unconstructed output untouched. A normal component-cleanup exception unlocks
before clearing constructed output and never retries component release. These
orders are implemented in the C++ exception domain. The existing event companion's
nonthrowing assignment/terminal contract makes native state3 unobservable here;
no claim covers throwing event terminal mappings. A second component exception
during unwind terminates. `866E60` has no throwing child projection and uses the
same captured real lock; its raw handler is `C94DC8 -> DC6C18`.

| Required step | Call-site evidence | Binding |
|---|---|---|
| Actual effect manager | `866CF4`, `866E86 ->866440` | existing canonical lifetime getter |
| Definition/name component search | `866D3E ->86B100` | this implementation and a pure current definition identity projection |
| Current component factory | `866DA6`, current row virtual `+18` | existing `PointEffectRowRuntime` |
| Retained child assignment | `866DC2 ->6FBEB0` | existing canonical reference assignment |
| Name comparison | `86B14B ->BF7FBF` | host CRT `_stricmp`, no length normalization |
| Component terminal release | `866D80`, `866E34`, `86B1A3`, current virtual `+00` | existing `GameplayEffectComponentLifetime`, after sole actual `+04` decrement |
| Child type field | `866EC0` | existing pure `PointEffectChildEvents::child_fields` |

All six known `866CD0` callers were checked: `6CFCFC`, `7EAFA0`, `822E77`,
`823052`, `82324F`, `823457`. Each supplies a native name header and fresh result
slot; retained assignments happen after the call. Four contained `866E60` calls
in `7C4DD0` pass type3. The additional calls `85734D` and `857417` both pass
type4 and lie outside any recorded Ghidra function. The API therefore
retains the full DWORD selector. The sole `86B100` caller is `866D3E`.

Validation used the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified before the read/export batches. Complete
live and installed-image byte spans agree; hashes are in the report. Strict
MSVC Win32 `/W4 /WX /permissive- /O2` compilation passed. Two ignored probes
executed complete original bodies against the reconstruction: seven component
name/ownership states, and seven paired wrapper states covering existing/missing
names, factory and terminal callback backing replacement, both alias behaviors,
type selection, and nonpositive count. Wrapper comparisons use the real canonical
manager creation, shared lifetime, Win32 section, recursion word and shutdown;
native stack pointers balance across both RET8 paths. Two additional C++ exception
states check factory partial mutation and normal component cleanup failure.
The original-byte probes relocate external calls and IAT operands; native manager
dispatch uses the real reconstructed manager, native child assignment uses a
controlled reference helper, and row factory/terminal bindings are fixture owners.

This is build-tested and fixture-tested C++ with controlled original-instruction
comparison, not a drop-in native vtable/SEH replacement or gameplay validation.
The host CRT's non-ASCII locale/invalid-parameter behavior was not compared with
the original VS2005 CRT. Real game child factories remain required bindings.
No new permanent tests, ledger edits, Ghidra mutations, or CMake edits belong to
this worker packet. The primary integrator owns the combined build and annotations.

The AI primary integration passed the combined strict Win32 build and both
existing CTests, then replayed both original-byte probes against the combined
library. All seven component states, seven wrapper states and two C++ exception
states passed. Ghidra signatures explicitly reflect the native stack arguments
and AL-only component result; names/evidence were saved with prior comments
preserved. Missing consumed exception dispatchers `C94DB1` and `C94DC8` are now
defined. These additions do not execute the original exception dispatcher or
establish the remaining game-specific factory and gameplay bindings.
