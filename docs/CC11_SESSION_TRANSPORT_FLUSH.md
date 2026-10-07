# Native session transport flush and owned enqueue

`NativeUnitHealthMessageTransportCalls` extends the existing complete serializer
with six complete ordinary transport bodies. A serialized health message can now
reach the existing concrete network-console enqueue, which copies payload bytes
into an owned `82Ch` queue record. The class remains abstract: actual transport,
target, cursor, table, clock, network-console and routing bindings are required.
It constructs no transport, socket, queue, profile or default provider.

## Native scope and families

| Original body | Bytes | Native interface | Source operation |
|---|---:|---|---|
| `00783E00–00783E93` | 148 | ECX transport; target/cursor/delivery; RET0C | Host virtual +20 flush |
| `00783EA0–00783F1B` | 124 | Same | Client virtual +20 flush |
| `00A41AB0–00A41B98` | 233 | Same | Host virtual +24 delivery |
| `00A419B0–00A41AA0` | 241 | Same | Client virtual +24 delivery |
| `00A39120–00A39153` | 52 | ECX embedded transport; type/target/payload/length; RET10 | Direct enqueue adapter |
| `00A39160–00A39190` | 49 | Same | Normal enqueue adapter |

All 847 bytes were inspected as complete native listings. The two previously
undefined +24 bodies were defined, byte-verified, saved and exported by the
integrator; see `reports/health_transport_dispatch_definitions_cc11.json`.
Names are descriptive hypotheses, not recovered symbols. These C++ member
interfaces add an explicit transport argument and are not native ABI replacements.

`0076FAD0` mode 1 passes session+188 to `A42170`, which allocates `920h`, calls
`A41CE0`, publishes the result and calls current virtual+4. The constructor's final
primary profile is `D243EC`: +20 is `783E00`, +24 is `A41AB0`, and the embedded
transport starts at +A4. Mode 2 passes session+18C to `A41C70`, which allocates
`A60h` and calls `A414D0`. Its final profile is `D24448`: +20 is `783EA0`, +24 is
`A419B0`, with the embedded transport at +C8. The client constructor also creates
its `DC0h` target and publishes it at +93C. These are static factory/profile facts,
not evidence of a current game session or a complete source factory composition.

## Ordering and ownership

The serializer supplies the current +20 entry. The extension admits only the two
complete bodies above. Each wrapper then reloads the actual primary table at the
native +24 call point and independently admits its current host/client target.
No family is cached across the clock/provider boundary; no table or address map
is fabricated. Unsupported targets throw `logic_error` as a source-contract
failure. This does not reproduce the original invalid-call/fault behavior.

Delivery 1 samples current `01090AB0` using the existing
`NativeFrameClockPublicationContext` and an explicit timestamp-stack preimage.
The **returned pointer** supplies two signed qwords to `FILD/FILD/FDIVP/FSTP`.
The only binary32 spill is into actual target+D68. Only then is current
`00E188A8` reloaded and its +217C DWORD incremented modulo32. The source does not
change the x87 control word or assume an empty stack. Host delivery 0 checks its
actual +A0 byte and may return before cursor, observer or current-table reads.

`00786CA0` is an original RET0Ch no-op. Its argument preparation still observes
actual fields. Host reads cursor bit, base, global `F871B4`, then current byte;
client reads bit, base, current byte, then that global. The source folds the no-op
and its unused integer argument calculations while preserving those ordered
volatile loads. It adds no observer callback or diagnostic policy.

Deliveries 0 and 1 enqueue normal types 5 and 6. Host uses the caller target.
Client uses its current +93C target: delivery 1 reads that pointer between cursor
base and current; delivery 0 reads it after current. Both bodies subsequently
reread cursor bit, the **original caller target's** +D6C index, cursor base, and
current byte. They add the resulting rounded length to transport+4C/+50+index*8,
even when enqueue returns false or drops/returns the record. Delivery 2 uses the
caller target and direct type 5, without byte accounting. Other deliveries return
from +24. The direct helper itself ignores types other than 5 before all object
accesses; the normal helper accepts its passed type unchanged.

The helpers capture embedded+858 (host socket+8FC, client socket+920), then reload
the `NativeNetworkConsoleSendContext`'s actual `F8ABDC` publication. The direct
helper forms target+D74 before that global load; the normal helper forms it after.
Both call the existing complete `enqueue_native_network_console_packet_00a3c1a0`
and discard its Boolean. That implementation copies normal/direct payload bytes
into owned queue storage or returns a record to the pool. It does not retain the
borrowed payload or cursor. These six bodies perform no cursor reset; the caller's
existing serializer resets only after the complete flush returns.

All added accessors are pure aliases: no snapshots, callbacks, ownership changes,
FP changes or early field-value observations. Actual storage and contexts must
remain live throughout serialization, enqueue and the post-call counter reads.

## Verification

The focused ignored fixture is
`local/cc11_transport_flush_20261007_a/probe.cpp`. It compiles the actual new
translation unit plus 19 existing source units with MSVC Win32 `/O2 /MD /Gy /W4
/WX /fp:strict`, embeds an asInvoker manifest and performs no network sends.
The 61 source/header inputs are copied to a read-only snapshot. Freshly compiled
clock/serializer/enqueue bodies are used; three verified immutable existing
libraries resolve their other support references. Snapshot and workspace hashes,
and copied-library hashes before/after linking, match.

The probe passed **180 checks**: 17 focused fixture cases and 72 original-kernel
comparisons. Fixture cases use original PE profile entries and constants, genuine
fixed-clock storage, actual Win32 critical sections and two local UDP socket
handles. They exercise normal/direct owned queue records, original type/stream
classification, profile drop, false enqueue with wrapping counters, host gate,
unsupported delivery/type, client alternate destination, fresh game/network-owner
publications, and unsupported source-entry rejection. One recording SDK conversion
boundary changes actual cursor fields, original-target index and publication
cells, proving the source's post-enqueue rereads and captured enqueue owner. These
are controlled source-fixture effects, not observed native SDK reentry.

A connected D2 message uses the existing executable profile, real target lock,
nonlocal selector and complete threshold serializer. Its 380h-byte payload is
copied by the existing enqueue, survives the serializer's cursor reset, and
remains intact after the original backing is overwritten.

The original PE's identical `783E27–783E33` / `783EC8–783ED4` kernels were copied
read-only into isolated executable memory and compared with the actual source
flush's timestamp store. The matrix covers PC24/53/64, all four rounding modes,
empty and three-value ambient stacks, and three signed/inexact/large timestamp
pairs. Stored float bits, x87 control/status/tag and register contents match in
all 72 cases. Exceptions are masked; frequencies are positive and there are at
least two available x87 stack entries. This is arithmetic/component evidence,
not full-body native ABI or fault-path equivalence.

Actual emitted assembly was inspected for the returned-pointer x87 sequence,
post-store game reload, distinct observer orders, client +93C ordering, fresh
+24 dispatch, helper socket/publication order and post-enqueue counter reads.
MSVC removes the unused observer argument arithmetic while retaining its native
ordered memory observations. Original EFLAGS/register identity is not claimed.

The concrete fixed-clock provider has no reentry callback. A fixture captures
host+20 and then supplies the original client table before source entry, proving
independent current+24 dispatch. It does **not** demonstrate a table change during
the actual clock call. No artificial clock hook was added.

## Remaining boundary

The integrator owns source registration, shared metadata, Ghidra annotations and
the full main build. Worker verification is the fresh actual-TU compile and
focused fixture; integration/build status is recorded separately by the primary.
Actual transport factories/publication/lifetime, complete network-owner/thread
composition, packet recording, sockets and `A3C5E0` send-worker composition remain
required. Actual queue copying does not establish network exchange or gameplay.
Native private exception/unwind/fault behavior, unmasked FP traps, concurrent
ownership changes and whole-method binary ABI remain unverified. Native fitting
payloads, finite valid lists/profile indices and actual backing are required;
the source adds no bounds clamp, successful-delivery result or substitute policy.

See `reports/cc11_session_transport_flush.json` for pinned hashes, call evidence,
artifact paths and the complete validation boundaries.

Primary integration 9319c93251d0b8d3c438b78bc3a9e40e0772e65c: full MSVC Win32/all three existing CTests passed. Independent root fresh actual-main20TU manifested probe repeated all180checks,17fixture cases and72original-x87 comparisons with zero failures. All61 main source/header snapshot inputs and3 current-main support libraries matched pre/post-link hashes; emitted returned-pointer kernel and post-store game reload inspected. Machine014C/resource24/id1/asInvoker and original PE identity verified. Ten direct native CALL rows passed. Queue ownership and arithmetic remain bounded component evidence; actual transport/lifetime/network thread/socket delivery/private fault paths/full native ABI/game remain unbound. Executable SHA256 3289fd463eb26ded9b3609542440d132ae865b0af1a40b477d05238c4849033b. Probe local/cc11_transport_root.cmd; build log local/cc11_transport_integrated_build.log.

Buffer/target lifetime prerequisite analysis: full raw/live bodies7830F0..783119(41B),783970..78399B(43B),783B90..783C75(229B), exclusive ends, were independently read. Returning CRT free callsite overrides were cleared and verified native tails disassembled/saved. The stored Ghidra function bodies still stop short of their final RETs; this is partial listing recovery, not full flow repair. The bridge refuses inline scripts because script execution is disabled; no body extension ran and the unexecuted tool change was reverted. Names/comments and global callee NoReturn were retained. See reports/transport_buffer_destructor_flow_recovery_cc11.json. The raw constructor783080..7830E3(99B) and destructor41B define a bounded next Source buffer-lifetime pair, independent of complete target/history/derived transport lifetimes; no new Source lifetime or game proof is claimed here.
