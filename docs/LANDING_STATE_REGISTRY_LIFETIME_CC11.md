# Landing state registry lifetime, CC11

Packet `cc11_land_state_registry_lifetime` closes three complete ordinary native
cleanup bodies over the existing actual Win32 registry/vector storage. These are
direct conditional Source services. They do not bind the original class profile,
private unwind frame, whole approach/task destructor, allocator ABI or game.

| Entry | End exclusive | Bytes | Complete disk instructions | Original ABI |
|---|---|---:|---:|---|
| `00411610` | `0041163A` | 42 | 13 | ECX actual vector; no arguments; RET |
| `004116D0` | `00411700` | 48 | 14 | ECX actual registry; no arguments; RET |
| `00411810` | `00411854` | 68 | 20 | ECX actual registry; stack DWORD flags; EAX original identity; RET 4 |

Total: **158 bytes, 47 instructions**, without padding or neighboring bodies.
All three complete PE byte ranges match live Ghidra memory and the fixture's
unrelocated copies. `bsp.py ghidra bytes` verifies the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before each request.
The original free is `00BF65AC`. The four returning-free continuations are
restored in [the primary repair report](../reports/state_registry_lifetime_flow_recovery_cc11.json)
(primary commit `ac77bffe0`); no worker Ghidra mutations were made. Instruction
counts above come from the full disk-byte disassembly, including all repaired
`ADD ESP,4` instructions, rather than cached function summary counts.

`411610` captures vector+4 once, skips free only for a null captured pointer,
calls the real free boundary, then zeros vector+4/+8/+C in that order. It leaves
vector+0 (the borrowed proxy) untouched. It does not destroy the borrowed names
or state pointees in the eight-byte elements.

`4116D0` first writes the raw numeric `00CE37DC` profile word, then captures
registry+8. After the optional array free returns, it zeros registry+8/+C/+10.
Registry+4 proxy and receiver storage remain alive; no receiver free occurs.

`411810` writes that same raw word and captures/frees the array. Native
`411829` tests **the low flags byte, bit 0, after array free and before all three
zero stores**. It then clears +8/+C/+10 and conditionally frees the captured
receiver itself. It returns the original identity even when that identity is
dangling. The Source volatile Boolean capture retains the same evaluation
ordering; it is not an instruction-identical stack/register bridge. Neither
Source nor fixture dereferences the identity after self free.

The live/disk 24-byte profile receipt at `00CE37DC` contains only one pointer,
`00411810`, immediately followed at `00CE37E0` by `vector<T> too long` and its
terminator. Numeric `CE37DC` remains **uncallable in Source**. No fabricated
callable vtable, default profile token, image mapping or virtual bridge is added.

The actual native approach destructor `009B2C80` reaches the ordinary registry
cleanup at `009B2D46`. Known canonical instruction boundaries are `009B2D36`
(`XOR ECX,ECX`), `009B2D38` (`CMP ESI,ECX`), `009B2D3E` (`JZ 009B2D46`),
`009B2D40` (`LEA ECX,[ESI+B8]`), then that call. This establishes conditional
native reachability, not a whole caller Source port, valid receiver guarantee or
null-path admission. Actual approach+B8 is task+4B0. The private constructor
unwind `00C5E090` loads its saved receiver, adds 4 and tail-jumps at `00C5E096`
to `411610`; this establishes EH reachability only. No private-EH integration
is reconstructed. The outer task destructor and complete approach caller still
require their own storage/profile/lifetime contracts.

The new header/source require stable, live, coherent actual storage and ordinary
returning free in the existing `singleton_lifetime_allocate/free` CRT domain.
For bit 0 set, the actual registry receiver must be separately allocated in
that same domain; embedded task/approach storage is excluded from self free.
Null array storage is admitted, null receiver storage is not. Proxy and pair
pointees are borrowed; no ownership is inferred from vector membership.
Structural reentry, concurrent mutation, faults, invalid allocations, native
CRT differences, private EH and gameplay remain outside the admitted domain.
There are no callback/free substitutes or arena services in the production API.

The one ignored fixture at `local/cc11_land_registry_lifetime_probe.cpp` connects
the existing real registry constructor, Add and eight-state registrar to these
cleanup paths. The same actual-shaped task storage/canonical+404 cell and
borrowed embedded states survive array cleanup. Nine prefix registrations plus
the eight actual approach state identities exercise real reserve16/growth24
before vector release, ordinary destruction and scalar flags0. A separately
CRT-allocated registry exercises scalar flags1; null-array repeats cover the
normal skipped-free continuations. The same connected flow then executes all
three full original bodies, relocating **only four `CALL 00BF65AC` operands** to
the genuine existing `singleton_lifetime_free` Source boundary.

Inside the ignored fixture only, the executable's own CRT `free` import is
temporarily traced, always forwards its captured real CRT operation, and is
restored after each body. Captured observations occur before the real free:
array free sees the published old headers, raw profile and proxy; receiver free
sees all three headers cleared. No allocation is accessed after real free and
no behavioral reentry or mutation is injected. This is Source fixture and
original-copy-to-Source-CRT evidence, not historical CRT/loader/EH/game proof.

MSVC Win32 `/O2 /MD /W4 /WX` compiled three fresh TUs: the new lifetime source,
existing complete registry source and ignored fixture. The embedded `asInvoker`
manifested probe passed **122 checks**. Source COFF independently shows the
stamp/capture/free/zero sequence, and scalar low-byte mask/capture before all
three zero stores followed by optional self free and captured identity return.
The original scalar `TEST` and Source `AND`/volatile capture differ in encoding;
the qualified Source observation order is preserved.

Fifteen inputs (three fresh TUs, nine Source headers, three pinned support
libraries) have identical hashes before compilation and after link/run. Unique
support copies were frozen from root's stable `d87f92504` build before its next
rebuild. Exact hashes, four native free call receipts plus the qualified caller,
manifest, compiler results
and artifact paths are in [the packet report](../reports/landing_state_registry_lifetime_cc11.json).
Worker verification includes `git diff --check`; root owns full main build,
CTest, source registration, annotations and integration. No tracked tests added.
