# Unit class-model publisher and receiver readiness

The complete `00879590` body publishes the resource at actual `class+50`
before releasing its load-name temporary. Existing Source preserves this order,
the current VFS publication and the captured game-factory/manager sequence.
The application's real factory, manager, pooled strings and VFS services exist.
Production class-model composition is held on the actual class receiver.

The next single receiver prerequisite is **actual vehicle-class base construction
at `00749050`**, on the original `138h` storage domain and using the already
implemented damageable base `0087C640`. Current Source has no body for that
constructor and no production implementation of the descriptor factory host.
This packet does not call a semantic descriptor or Lua-row projection an actual
receiver. It opens no Native child body or data cell and adds no C++.

## Whole Native gate

Ghidra project `C:/Users/sqz269/bsp.gpr` and its sole program
`/battlestationspacific.exe` were verified before the live CLI batch. The program
has 64,730 functions, x86 little-endian 32-bit language and image base `00400000`.

`00879590..008797A4` is **533 bytes, 165 instructions and 22 direct calls over
nine targets**, with no indirect call in this body. Original PE and current
Ghidra bytes match; all saved/live listing rows and independent instruction
starts match. Body SHA-256:
`169b120c3eaa86c7fa352047a679852e299d4c06e09fb9e9971609f518a11acf`.
The whole PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Current analysis memory is not a running game's heap or an execution result.

Original ABI is ECX=actual class, one stacked DWORD whose low byte is the enemy
flag, and `RET 4` at `008797A2`. EBP/EDI are saved throughout; the enemy branch
also saves ESI and, during construction, EBX. The FS frame names handler
`00C96610`. The live zero-parameter prototype omits these inputs. No x87/SSE
arithmetic occurs in this body. Source is an ordinary Win32 C++ interface;
private stack aliases, Original ABI and FH3/SEH/fault behavior are not admitted.

## Publication and name schedule

| Native sites | Established order |
| --- | --- |
| `008795A9..BE` | Capture the receiver in EBP and its actual name header at +38 in EDI. Require current name length nonzero, then current +50 null. There is no later +50 recheck. |
| `008795C4..F9` | Test the flag's low byte. For a nonnull current name buffer, `strstr` finds the first dot. Reload the header data pointer for the subtraction and accept only a nonnegative signed offset. |
| `00879609..61` | Construct suffix `_enemy`, extension from the offset to the end, prefix before the offset, prefix+suffix, then the complete candidate. Do not reorder the actual string operations. |
| `0087966C..F2` | Advance cleanup states before returning joined/prefix/extension/suffix storage, in that order. Each nonnull return captures data and wrapped length+1 before the pool getter. |
| `008796F7..9712` | Read the current `0109CEEC` VFS publication after those returns; resolve the mutable candidate. Only a true AL result assigns the candidate's current header into actual class+38. |
| `00879717..3F` | Disarm candidate cleanup before normal destruction, then reload the actual class name's current data pointer. A false resolver result does not assign the candidate to the class. |
| `0087973F..63` | Copy the current C string, or the existing empty-literal domain for null data, into a distinct load-name header. Arm state 9 and call full `007188A0` with that header. |
| `00879768..8D` | Store the returned resource in the same actual receiver's +50, then disarm state 9 and return the load-name block. There is no retain, null-result gate, prior-resource release or publication rollback. |

`00419CC0` takes no native arguments. The already pushed
`(data, length+1, 1)` belongs to the following `00BD1510` call, whose ECX is the
returned pool and whose native return consumes `0Ch`. The decompiler's apparent
getter arguments are not the contract. These complete existing Source providers
resolve the live pool before every nonnull return, including large blocks and
disabled small returns; no cached pool is introduced.

The entry guards run only once. A provider changing `class+50` after those guards
does not suppress the final store or add a release of the intervening value.
Successful enemy-name assignment remains visible if later loading fails. A load
failure before `00879768` does not publish that call's return; it does not undo
other writes made by reached providers. A cleanup failure after `00879768` leaves
the published resource and current class name in place.

## Lifetime and exception contract

The actual class/header, name bytes, shared raw string cells and every borrowed
service must stay valid through the call and its cleanup. The caller neither
retains the class nor copies it into another owner. Source's acquired frame owns
only temporary headers and the existing nested VFS/cache invocation frames; it
requires a fresh invocation in the same context and cannot be replayed.

Current Source represents the ten-state cleanup map documented by the historical
class-model receipt. States 0..3 own successive temporary strings. State 4 is in
that historical map but is not installed by this caller. States 5..8 progressively
remove normal-cleanup ownership while retaining the candidate; state 9 owns only
the load name. Cleanup advances its state before each destruction and terminates
if a second cleanup exception escapes. A failed nested VFS resolution retains its
existing frame/header lifetime obligation. Substring/concat/header assignment
still use the existing `NativeStringStorage::release` noexcept boundary.
The handler, maps and funclets were not reopened in this packet.

Existing `007188A0` Source first gets and captures the game factory, then gets
the resource manager, then calls full `00B80720` with that captured factory and
the same load-name header. Both raw publication domains and deletion contexts
must outlive invocation and shared drain. The separate WinMain factory alias
`00F8D31C` is not a substitute for the current `00E19B90` getter domain.

`00B80720` Source publishes the supplied factory at manager+20 before cache
lookup. A hit retains the matched resource's +4 and returns current manager+24.
A miss reads the current factory from +20, captures its current +4 target,
creates/parses/caches through the actual providers, and captures its final
manager+24 result before temporary cleanup. Existing known-target dispatch does
not guarantee that a cache hit, existing class resource or callback-mutated
publication has the game-resource profile. The earlier conditional
`CFD8CC+8 -> 007137F0` witness remains conditional.

## Available owners and the exact next prerequisite

| Current Source evidence | What it supplies |
| --- | --- |
| `game_hosts_singletons.hpp:117-122`, `game_hosts_singletons.cpp:24,106-110` | Real game-factory publication, stable factory context and registered deletion binding; startup separately publishes the WinMain alias. |
| `game_native_resource_application.cpp:25-82,120-127` | Actual manager/parser publication cells, shared raw strings, deletion bindings and a borrowable raw manager context. |
| `game_native_vfs_application.cpp:39-100`, `game_native_vfs_runtime.hpp:37-56` | One retained VFS, actual string and singleton domains, current resolver/open services and common type-counter/root/node services. |
| `game_native_type_storage.hpp`, `game_native_type_storage.cpp` | Stable resource/stream type storage. Model descriptor storage is still absent; the accepted CRT-placement receipt has already resolved relative placement for a qualified model insertion. These process type cells do not create a vehicle-class receiver. |
| `native_damageable_class_construction.cpp:108-146` | Complete borrowed-storage base construction, including zeroed +38/+3C and +50. It supplies neither an allocated vehicle descriptor nor an authored Mesh name. |
| `vehicle_class.hpp:158-170,195-246`, `vehicle_class.cpp:200` | A semantic descriptor and required `VehicleClassHost::construct_descriptor`/cache/lifetime operations. No production implementation was found. |
| `game_hosts_units.cpp:14628-14637` | Current host reads an installed Lua `VehicleClass` row and its kind; it does not construct the actual descriptor passed to this publisher. |

The indexed `00749050` entry identifies a `138h` actual vehicle base, the
`0087C640` child, primary/secondary profile writes and seven derived callers.
It has no reconstruction record; current Source contains only references and
metadata for this constructor. The next bounded packet should whole-gate that
one body and recover its complete actual-storage construction/cleanup contract,
then assess composition with existing `0087C640`. No child bytes were read here,
and this packet does not pre-authorize a constructor implementation or claim that
construction alone supplies the full class registry/loaded receiver.

A specific downstream limit is already recorded: `native_damageable_class_lua.hpp`
explicitly leaves `0087CA80` source absent. Its historical whole-body audit places
the actual Mesh header writes at `0087CB44..CBCA`. Supplying a hand-built name
or casting `VehicleClassDescriptor` would bypass that contract. The production
load/cache dispatch context is also still uncomposed; its explicit required
providers cannot be replaced by a fallback. These limits remain separate from
the selected next constructor prerequisite.

Thirty-seven inspected Source/context files match current Root after LF
normalization. The separate 509-input primary review remains build authority:
three checks, 43 Core plus one App whole objects, 51 roots, and bounded startup
at PressStartPoll with three ticks/two Presents and zero mission frames. This
worker read that receipt, executed no build/test/probe and compared no historical
receipt to current binary artifacts. This packet adds readiness credit only.

Evidence: `reports/cc12_unit_class_model_publisher_owner_readiness.json`.
