# Native file, memory-file, and physical-file type descriptors

Addresses: `00BE4530`, `00CD8FC0`, `00CD9030`.

`NativeStreamTypeIds` binds the existing process guards and descriptors and consumes
the same published type-ID counter as `LightTypeBootstrap`. Construction retains
references only. IDs are read from current storage and assigned from the current
counter; none are fixed constants. Name words retain original image addresses.
The new C++ methods are typed interfaces, not drop-in native ABI replacements.

| Routine | Coverage | Original ABI and bounds |
| --- | --- | --- |
| `initialize_file_00be4530` | complete | ECX = descriptor target, no stack arguments, ESI saved/restored, RET; `00BE4530..00BE456D` inclusive, end `00BE456E`, 62 bytes |
| `initialize_memory_00cd8fc0` | complete raw entry | No inputs or stack arguments, RET; `00CD8FC0..00CD900E` inclusive, end `00CD900F`, 79 bytes; undefined in Ghidra at worker verification |
| `initialize_physical_00cd9030` | complete raw entry | No inputs or stack arguments, RET; `00CD9030..00CD907E` inclusive, end `00CD907F`, 79 bytes; undefined in Ghidra at worker verification |

The physical entry starts after the `00CD9010..00CD9025` initializer's RET and
ten INT3 padding bytes. Its conditional branch targets its own RET at `00CD907E`;
the next independent entry starts at `00CD907F`. The memory entry also branches to
its final RET. Ghidra DATA references at `00CE3698` and `00CE36A0` point to these
entries, respectively. Those references do not establish that startup executes them.

## Storage and order

| Family | Guard byte | DWORD layout | Name |
| --- | --- | --- | --- |
| file | `0109DB54` | `0109DB58`: own, root, name | `00D6888C`, `cFile` |
| memory file | `0109DB94` | `0109DBA0`: own, file, root, name | `00D68D7C`, `cMemoryFile` |
| physical file | `0109DC2C` | `0109DC30`: own, file, root, name | `00D69218`, `cFileX86` |

Every nonzero guard skips all descriptor writes and counter access. For a zero
file guard, `00BE4530` sets the process guard to 1, writes the target's name,
initializes the canonical root at `0109DB84`, reads its current own ID into the
target's root word, then consumes a counter ID for the target's own word. ECX may
identify a different file descriptor; that does not change the canonical guard.
A prior call with an alternate target can therefore leave the canonical file
descriptor untouched while preventing subsequent calls from initializing it.

Each child sets its own guard and name before calling the file initializer with
ECX = `0109DB58`. It loads the file own ID into EAX and file root ID into ECX
before either destination parent word is stored. Then it calls the counter getter,
loads the current counter at owner + 4, writes counter + 1 modulo 2^32, and stores
the captured value as its own ID. The implementation retains this load/store
order, including the counter increment before the descriptor's own-ID store.
It adds no lock or once flag; native process guards are ordinary byte guards.

## Existing callee contracts

| Call site | Native callee | Established contract and argument setup |
| --- | --- | --- |
| `00BE454F` | `00BEA780` | ECX = `0109DB84` from `00BE4543`; initialize root, no stack args, RET |
| `00BE455C` | `006FAC20` | No arguments; EAX = current shared counter owner; RET |
| `00CD8FDF` | `00BE4530` | ECX = `0109DB58` from `00CD8FC9`; no intervening ECX write, no stack args, RET |
| `00CD8FFA` | `006FAC20` | No arguments; EAX = current shared counter owner; RET |
| `00CD904F` | `00BE4530` | ECX = `0109DB58` from `00CD9039`; no intervening ECX write, no stack args, RET |
| `00CD906A` | `006FAC20` | No arguments; EAX = current shared counter owner; RET |

The whole `00BE4530` body and both call sites to it were read, including register
setup. Live prototype queries agree for the file body and its call sites. The four
child call sites have no containing Ghidra function yet; exact installed bytes,
matching Ghidra memory bytes, and bounded disk disassembly establish their calls.

`TypeIdCounterLifetime::get_006fac20` was checked against the complete
`006FAC20..006FACD4` body: it returns the first non-null load from publication
slot `0109DB7C` on its fast path. The existing slow path obtains the shared
`SingletonLifetimeDomain` manager, locks its captured section, rechecks publication,
allocates an eight-byte owner with counter +4 = 0, publishes and registers it, then
unlocks and reloads the slot. The stream code does not add a counter or registry.

`LightTypeBootstrap::initialize_root_00bea780` was checked against the complete
`00BEA780..00BEA7AB` body: actual guard `0109DB80`, actual canonical descriptor
`0109DB84`, and the same `006FAC20` getter. Its guard precedes counter access and
its own ID precedes the `cRoot` name store. The caller must pass a counter adapter
and root bootstrap belonging to the same process lifetime domain. That composition
is an integration requirement, not something separate adapter references enforce.

## Verification and limits

Evidence and final command results are in `reports/native_stream_type_ids.json`.
The focused ignored probe executes bytes copied read-only from the installed PE in
a separate process. It shifts code and storage by `30000000` and adjusts only decoded
absolute operands that reference the process-storage range `01080000..0109FFFF`.
Relative calls, instructions, control flow, and name constants otherwise remain
unchanged; there are no callee hooks. It compares against the source methods bound to
that same publication slot and descriptor storage. Preset
nonzero guards, nonzero/wrapping counters, both child orders, a noncanonical file
target, repeated calls, and changing the published counter owner are covered.
All 1,152 comparisons passed (192 construction-preservation checks and 960 state
comparisons following calls or publication changes). The strict Win32 build and
both existing CTest suites passed; eight native math seed bodies matched Ghidra and
the installed PE. It uses an already published owner: the counter's allocation/registration slow
path is reused from the existing implementation and is not exercised by this probe.
No permanent test suite, game installation, consumer runtime context, or Ghidra
database is changed by this packet.

Ghidra names are proposed in the address ledger and remain pending primary-agent
annotation. The worker is restricted to read-only Ghidra use. In particular,
`verify_report_calls.py` cannot validate four genuine child calls until the primary
agent defines the two missing function bodies; the report retains all six call
rows and records that limitation. No startup execution or gameplay result is claimed.
