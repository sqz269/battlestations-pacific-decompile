# Native enum node pool

Addresses: `00411050`, `00410A60`, `00410CD0`, `004E6370`, `004E7C00`, `0043B0A0`.

This adds the six complete ordinary normal bodies for the actual 64-slot enum-node
pool through new Win32 interfaces. It closes the previously missing `14h` slot
allocator/lifecycle dependency of `008F2850` and `008F17E0`. It does not bind their
insertion/replacement, native CEnum ownership, declaration identity or traffic loader.
The semantic `PropertyLibrary` is unchanged.

| Entry | Inclusive end | End exclusive | Bytes | Coverage |
| --- | --- | --- | ---: | --- |
| `00411050` | `00411122` | `00411123` | 211 | complete ordinary constructor |
| `00410A60` | `00410AE9` | `00410AEA` | 138 | complete ordinary destructor |
| `00410CD0` | `00410D6F` | `00410D70` | 160 | complete trim |
| `004E6370` | `004E63AF` | `004E63B0` | 64 | complete page initialization |
| `004E7C00` | `004E7D3B` | `004E7D3C` | 316 | complete ordinary allocation |
| `0043B0A0` | `0043B104` | `0043B105` | 101 | complete valid-slot return |

All 990 bytes match installed PE and live Ghidra, pinned individually in the report.
The primary repaired five returning-free listing gaps, saved and exported the four
affected functions. Their reports are pinned in `caller_witnesses/root_flow_repair/`.
The current listings include the normal tails. Counts from complete byte decoding
are 68/51/58/23/111/33, including alignment instructions; live stored listings show
68/51/55/23/109/33. Historical metadata `instruction_count` fields are not used as
complete-body evidence.

The constructor writes the actual `38h` owner: list header `0/4/8`, real Win32
critical section `+0C`, nesting `+24`, page-pointer table `+28`, count `+2C`, capacity
`+30`, earliest nonfull page `+34`. It prepends to the same `00E188B4` allocator-list
domain, changes base profile `00D7A0C0` to `00CE37A4`, initializes the section, and
reserves 32 pointer cells. The recovered virtual-zero cell `00CE37A4` points exactly
to the complete `00410CD0`; Source host setup binds that real implementation through
the existing `AllocatorListDomain`, without adding physical fields or a foreign profile.

Each `584h` page contains 64 `14h` slots. `004E6370` preserves every payload byte
`0..F`, sets allocator-owned slot `+10` to its page index, fills the ushort free stack
`+500..57F` with `63..0`, and sets ushort `+580` to 64. Padding `+582..583` is untouched.
This is a recovered slot/page contract, not a claim that every dictionary node class
has a `14h` allocation or that the unrelated `178h`/32-slot pool is compatible.

Allocation enters the real section and increments depth. Before allocating a page it
publishes the current page index. Capacity growth uses native DWORD `2*capacity+2`
and `capacity*4`, publishes capacity before allocation, reloads table/count during
copy, frees the old pointer table before publishing its replacement, then appends the
page. Allocation pops the ushort free stack and scans later pages when exhausted.
Return reads the slot's actual `+10` owner ID, stores its aligned index back in the
free stack, increments the count, and lowers the earliest-page index. Its native
signed difference/division by 20 agrees on admitted valid page offsets; malformed
addresses and double returns are outside the contract.

Trim has no internal lock. For each wholly free page it frees, reloads table/count,
moves the last page into the hole before decrementing count, rewrites **all 64** moved
slot IDs, retries the moved page, and finally rescans for the first nonfull page.
Destructor frees all remaining pages and the pointer table, drains positive signed
depth with real `LeaveCriticalSection`, deletes the section, restores the base profile
and unlinks the same list element. It leaves the native metadata/link fields stale;
it neither destroys key payloads nor frees the borrowed owner. The host binding is
explicitly removed by its owner after destruction.

Native producer witnesses are `008F2877/287C` (symbol pool `00E17578`) and
`008F1820/1825` (table pool `00E175E8`). Their key-owning header is `+0/+4`, opaque
mapped word `+8`, chain link `+C`; insertion leaves allocator `+10` intact. Fresh byte
pins identify static construction at `00CC8A50` and `00CC8A90`, with registered
destructor tails `00CD9280` and `00CD92D0`. `00CC8A70` uses `00E17540`, not the table
pool. These wrapper/insertion witnesses are read-only context, not reconstructed
or original-runtime registration proof.

The ignored connected fixture is `local/cc11_scene_enum_node_pool/enum_pool_probe.cpp`,
recipe `run_probe.py --out local/cc11_scene_enum_node_pool/<fresh-directory>`.
Successful immutable receipts are under `run01/`. It uses the real current raw-string
constructor/pool, two genuinely constructed raw enum pools in one allocator-list
domain, current CRT allocations, real OS sections and the exact trim binding. It
allocates 2,049 symbol slots, grows to 33 pages/capacity 66, returns a middle page,
checks LIFO reuse, trims to 32 pages and verifies every moved ID, payload and page-tail
byte. Owning installed keys remain at the same addresses across movement.

Installed `global.enums` supplies 22 `LandVehicleClasses` and six `SoldierTypes`
symbols. Two table-word plus 28 symbol-word reads and 28 memberships run before and
after movement: 60 words, 56 memberships, and 116 complete primed temporary-string
pool-prefix restoration checks pass. It then destroys keys, returns all slots, trims
empty pages, exercises final live-page destruction/positive-depth drain, and checks
non-head/head unlink and retained metadata. Receiver/bucket fields and publication
are explicitly external fixture backing; original map insertion/owner identity is
not simulated as a recovered provider.

All 11 fresh TUs pass `/W4 /WX /fp:strict`; link, COFF checks, manifested probe and
embedded `asInvoker` extraction pass. The inventory is 40 actual project headers plus
10 production CPPs (50 production inputs, 52 including fixture/recipe). Three frozen
primary b728 libraries from baseline `085519c25` have equal original pre/copy/post
and pinned pre/post-link/run hashes. All 307 prior lookup `run05`, word `run02`, and
membership `run02` receipt files remain unchanged. Nine owned direct-call rows pass
the live report verifier; Win32 imports, the register-indirect leave call and profile
dispatch are reported separately.

Admission is coherent, externally synchronized, nonoverflowing successful ordinary
storage, valid unique aligned slots and nonreentrant mutation. Current CRT/OS Source
providers are exercised; original allocation failures, new-handler reentry, FH3/EH/SEH,
overflow/alias/concurrency/fault behavior and historical CRT parity remain unbound.
Original bodies/wrappers were **not executed**. Observed ECX/stack/RET boundaries are
evidence, not drop-in class/vtable ABI. Full main build/CMake registration are the
primary's pending integration checks. No native declaration/enum identity, traffic,
mission or gameplay validation is claimed.
