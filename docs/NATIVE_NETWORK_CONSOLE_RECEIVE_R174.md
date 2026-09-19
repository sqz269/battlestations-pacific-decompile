# Native network-console receive classification and ACK release (R174)

Addresses: 00a3a6b0, 00a3b420, 00a3b5f0, 00a3baa0, 00a3adf0,
00a3caf0, 00a3c5e0

## Implemented scope

Five complete normal bodies, 1,069 original bytes, now implement receive-side
sequence/ACK processing, channel activation and the send worker's deferred close
request. Names are descriptive hypotheses, not recovered source symbols.
The source interfaces do not claim drop-in binary ABI equivalence.

| Entry | Original ABI | Complete behavior |
| --- | --- | --- |
| A3A6B0 (88B) | Stack header/reference, RET8/EAX sequence; ECX not an input | Decode the header's10-bit sequence around a DWORD reference, with exact unsigned half-window tests and wrapping additions/subtractions. |
| A3B420 (450B) | ECX49Ch channel, stack header, RET4/EAX released82Ch list | Decode ACK counter relative to sent counter70. Reject older counters; at an equal counter reject any incoming bit absent from cached mask68. Only then sample current AB0, publish counters60/68, remove acknowledged send records, update matching unused timing slots and return the removed records in reverse order. |
| A3B5F0 (232B) | ECX channel, stack header/stream-out/sequence-out, RET0C/EAX released list | Sample timestamp20, process the complete ACK, zero the sequence output, then use the two native type tables to reject old/duplicate sequences, admit a new sequenced packet, accept unsequenced packets as stream2, or reject type8. Clear byte28+stream only for an admitted new sequence. |
| A3BAA0 (138B) | ECX4003F0h owner, stack sockaddr16, RET4 | Capture section148 once, activate the first free of16 channels, copy four address DWORDs in order, make the discarded-result XSocketHTONS call, and leave the captured section. No channel reset or duplicate lookup; a full table only enters/leaves the lock. |
| A3ADF0 (161B) | ECX channel, stack float delay, RET4 | First request sets byte18 before sampling current AB0. Store the float-rounded timestamp plus delay; later requests select the earlier deadline with the original x87 spills and FCOMIP/JBE unordered behavior. |

## Arithmetic, data and ownership contracts

A3B420 retains the sampled float and double comparison limit on the x87 stack
throughout list traversal. It rounds elapsed samples to float before comparison,
caps only ordered values above the limit, preserves NaN behavior, then increments
the DWORD sample count and performs the original x87 float addition/store. The
source block avoids using EBX/EBP, which MSVC may need for an aligned stack frame.
It retains the native head, middle and tail unlink order and reversed release list.
An empty list still publishes accepted counters and performs the timestamp divide.
A rejected ACK never calls the clock.

The explicit context borrows the current actual80h clock publication and its real
method provider, distinct classify/ACK stack preimages, native type tables E0E360
and E0E370, and constants D049A8/CF4848. The80-byte tables and12 constant bytes were
matched against both the live program and PE. No inferred table defaults are
installed into a production owner. The current sequenced types use stream1;
the implementation retains the indexed table reads and valid stream0..2 contract.

A3ADF0's send-worker consumers subsequently invoke A3BC60 when the deadline is
reached; its A3B150 dependency returns channel queues to their pools and resets
the channel. The pending-close interpretation follows that consumer path, not
an inferred socket-close call inside A3ADF0.

The PE import directory identifies A4D542's CE2598 slot as **xlive.dll ordinal40**.
The existing network-console socket resolver now requires actual ordinals38 and40
from an already loaded module. It neither loads a DLL nor fabricates a successful
SDK implementation. Missing required exports are source binding failures.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. One local original-code
fixture compares146 cases and13,156,508 bytes, including48 sequence boundary inputs,
32 ACK and96 deadline precision/rounding/sample combinations, masked x87 exception
flags/stack balance, ACK admission/empty/all-removed paths, eight receive classifications,
and three real tracked-lock activations. Raw list pointers are checked before
normalization for archived observations. A source-only missing-clock check confirms
that the first close-request byte store survives that binding error.

The first fixture run failed because its relocation generator omitted the two
indexed table operands. The correction handles their displacement fields; the failed
exit record and old relocation set remain archived. The successful comparison uses
the corrected fixture and unchanged production implementation.

Evidence covers3,321 live/PE-matched bytes and seven direct calls, including both
A3ADF0 send-worker sites and receive-worker calls A3CCBD/A3CD0F. Five functions and
two parent dependency fragments are named/commented through the Ghidra write lock;
prior annotations, saved readback and refreshed exports are retained. No listing
repair or permanent test suite was needed. Reports and immutable archives pin the
tested and integrated source/object/fixture artifacts.

## Remaining work

The full A3CAF0 receive worker still requires its real socket/logging/process
composition. The send worker additionally needs A3A8F0, A3BC60/A3B150 and
A3C1A0/A3B210; the derived constructor and two thread entry wrappers remain unbound.
These helpers do not establish ordinary startup, a network peer, network exchange,
concurrent worker safety or gameplay. The fixture uses the already reconstructed
actual fixed-clock sampler, real Win32 sections, and a recording SDK conversion
boundary. Original allocator/new-handler identity, native exception/fault behavior,
unmasked floating-point traps and full binary ABI compatibility remain unproved.
