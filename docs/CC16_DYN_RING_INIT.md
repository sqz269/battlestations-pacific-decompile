# Dyn ring index initializer at 00C33510

Packet `codex-dyn-ring-init` recovers the in-house body at
`00C33510..00C33645` in `src/cc16_dyn_ring_init.cpp`, with the native storage
view and required allocation providers in `include/bsp/cc16_dyn_ring_init.hpp`.
The names are descriptive hypotheses. This packet adds no vendor body, runtime
adapter, caller reconstruction, build registration, or function-credit change.

## Evidence and original ABI

The configured saved project is `C:/Users/sqz269/bsp.gpr`. Before the live
queries, the `bsp.py ghidra` client verified live project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and image base
`00400000` against that target configuration.
Live `proto` confirms the inclusive body `00C33510..00C33645` (310 bytes).
Recovery used the full saved assembly, paged through `bsp.py show --asm`, and
the live bytes of that same span. No Ghidra mutation was made.

The receiver arrives in ESI. There are no stack parameters and the last
instruction is a plain `RET` at `00C33645`. EBX and EDI are saved/restored;
ESI is unchanged. No semantic return value is established; EAX incidentally
holds the next-array base at exit. The new C++ reference/provider interface
is not a drop-in register-ABI replacement.

Only the allocator at `00BF55BE` and free entry at `00BF6989` leave the body.
The four exact call sites are recorded in `reports/cc16_dyn_ring_init.json`.
The allocator receives a single uint32 byte count on the stack; free receives
one block pointer, with the caller removing each argument.

## Recovered storage and behavior

| Receiver offset | Observed field |
|---|---|
| `+00h` | current ring count |
| `+04h` | untouched, unknown |
| `+08h` | owner pointer; the owner's `+04h` supplies the initial count |
| `+0Ch`, `+10h`, `+14h` | next-index array pointer, size, capacity |
| `+18h`, `+1Ch`, `+20h` | previous-index array pointer, size, capacity |

The body first stores the owner count at receiver `+00h`. It resizes the
previous array, then reloads receiver `+00h` and resizes the next array. Both
resizes compare sizes/capacities unsigned. On growth beyond capacity, capacity
is assigned before allocation, old entries are copied, the old nonnull block
is freed, and the new pointer is assigned. Newly exposed elements are zeroed.
Shrinking changes size without reallocating or clearing retained entries.

Each placement-copy/zero write has the listing's computed-slot null check;
an allocation returning null does not cause an early return. Allocation byte
sizes and element addresses use wrapping 32-bit arithmetic. The providers
are mandatory external contracts, with no invented global allocator or
fallback implementation.

For a positive signed count, the final loop writes `previous[i] = i - 1` and
`next[i] = i + 1`, then closes the ring with `previous[0] = count - 1` and
`next[count - 1] = 0`. A one-element ring therefore links to itself.
The count is reloaded at the original loop comparisons and endpoint stages.
The endpoint stores are unconditional: zero or signed-negative counts skip
the loop but still perform these stores with wrapping indices. No count,
pointer, allocation-failure or overflow guard was added. Such inputs can
fault in the original and require valid mapped storage for equivalent use.

## Reachability and validation boundary

Live xrefs identify one incoming call at `00C4D605`, inside Ghidra-defined
`00C4D5A0..00C4D971`. That caller has no incoming sites in the current indexed
census and has no reconstructed body in this packet. The earlier
`DYN_unreachable_00c33510` ledger row is metadata, not source evidence. This
packet establishes this initializer's body only. Caller recovery or a full
evidence-based dead-chain conclusion remains necessary before runtime
activation can be claimed.

The source was reviewed against every instruction in the span. Its single
translation unit compiled with MSVC x86, C++17, `/permissive- /EHsc /W4 /WX`;
this was an object-only compile, with no executable. The live call-report
verifier checked all four rows with zero failures. Build registration, the
common Win32 build, post-integration report checks, centralized ledger updates
and Ghidra annotation belong to the primary integrator. No new test suite or
ad hoc executable was added. There is no fixture, ABI or gameplay validation
claim.
