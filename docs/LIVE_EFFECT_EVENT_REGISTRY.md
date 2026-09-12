# Borrowed effect-event registration

Addresses: `00866A10`, `00866B00`.

The live manager's `+1C/+20/+24` array is populated by the registered type-1
and type-4 event constructors, and pruned by their destructors. The manager
does not own these references. `src/live_effect_event_registry.cpp` reconstructs
both complete methods over the actual manager, separate deletion lock, and
shared application lifetime domain.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| `00866A10` register | Complete `00866A10..00866AF5`, 230 bytes | ECX actual28h manager; stack raw event; RET4 at `00866AF3` |
| `00866B00` unregister | Complete `00866B00..00866B6E`, 111 bytes | ECX actual28h manager; stack raw event; RET4 at `00866B6C` |

Names are hypotheses. These typed interfaces are not native object/vtable ABI
replacements. The original executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
all body and EH bytes match saved `bsp.gpr` `/battlestationspacific.exe` and disk.

## Registry mutation

Both functions first call actual `00866500`, capture its section at `+04`,
enter the OS critical section, and increment the actual recursion counter.
This is the `F87654` deletion/registry lock, distinct from insertion lock
`F87650`. The captured counter is decremented before leaving that same section.

Registration reads capacity before count. On equality it computes unsigned
`2 * capacity + 2` modulo 2^32 and grows only when that value exceeds the
current capacity. It publishes capacity **before** allocating; byte-size
multiplication saturates to `FFFFFFFF` on unsigned overflow. If backing exists,
the function copies raw DWORDs with current backing/count reads, then frees the
current old backing. It publishes the new backing, reads the current count and
data, writes the supplied raw pointer, and increments the current count last.
Nulls and duplicates are accepted; no event count or pointer target is touched.

Allocation uses the existing host malloc/new-handler boundary, corresponding
to the observed `operator_new` retry/throw contract. Native CRT allocator and
exception-object ABI are not recreated. The native EH state only releases the
captured lock: handler `C94D68`, FuncInfo `DC6BA8`, map `DC6BA0`, unwind
`C94D60` (`LEA ECX,[EBP-14]; JMP 00411EE0`). Allocation failure leaves the
already-published capacity and previous count/backing intact.

Unregistration scans for the first matching pointer, including null. It
replaces that slot with the current last cell and decrements the live count;
it does not clear the stale last cell, shrink capacity, release either pointer,
or remove later duplicates. Missing input leaves the header/backing unchanged.
No local native EH frame exists in this method.

The registration decompiler originally stopped after `_free` at `00866AB7`.
Under the Ghidra write lock, its incorrect call-return flow override was cleared
and the missing `ADD ESP,4` at `00866ABC` disassembled. Re-decompilation now
includes backing publication and append after free. Prior override and repair
bytes are preserved in the report; the CRT callee's own no-return annotation
was not changed.

## Producers and frame consumption

The current direct call graph has exactly these registration/removal callers:

| Producer | Registry call |
| --- | --- |
| `008742A0` type-1 model-event constructor | `00866A10` |
| `008744A0` type-4 event constructor | `00866A10` |
| `00874430` type-1 destructor | `00866B00` |
| `00874540` type-4 destructor | `00866B00` |

This explains why serial child update `00867790` skips virtual `+28` for types
1 and 4: their registered raw pointers are submitted by phase-two `00866C60`
and execute through `008663B0`. Their completions and primary/auxiliary
ownership still belong to the serial child pass. Registration must therefore
last until actual event destruction, including while a stopped event remains
owned by the auxiliary list. The raw span must remain valid during frame jobs.

The earlier AH manager fixture explicitly supplied rumble events as controlled
borrowed-list input to exercise the queue. It proved the listed mechanics; it
did not establish that rumble constructors populate this registry. The native
producers above now identify the actual registered families.

## Validation and remaining work

The complete original 230/111-byte methods, including RET4 and ESP balance,
matched C++ in `local/effect-event-registry-probe-ai.log`. The probe uses the
actual live manager, deletion-lock singleton, shared lifetime domain, OS
critical section, and backing allocation/free. It covers capacity growth2 to6,
the repaired continuation after freeing old backing, null/duplicate/missing
inputs, first-match removal, and intentionally stale tail cells. Entire1Ch
event images and their actual atomic04 counts remain unchanged. Real domain
shutdown destroys the lock, manager, and backing.

A C++ saturated-size allocation failure invokes the actual new-handler boundary,
preserves published capacity, and unlocks without rollback. Original native EH
dispatch is not executed. Native manager/lock layout and CRT call bridges used
by the differential are explicit in the report.

Strict MSVC Win32 and both existing CTests passed. No permanent tests were added.
Concrete event producer integration is recorded in the type-1/type-4 reports.
Their model/particle runtime callees, native vtable compatibility, physical output,
and gameplay remain separate requirements; this registry alone does not make
those paths runnable.

## Correction from docs/REGISTERED_MODEL_EFFECT_BEHAVIOR.md and docs/REGISTERED_TYPE4_EFFECT_BEHAVIOR.md

AJ supplies the registered families' completion/deactivation entries and update
entry sequencing, actual particle-model pool behavior, option singleton and default
tracer curve evaluation. These routines pass the combined strict Win32 build and
focused fixtures within their individual reports' coverage.

Registry membership still borrows actual events. Successful physical model/tracer
construction, canonical companions installed before registration, current virtual
family dispatch and terminal owner teardown must be composed before these families
can run through the application. No frame or gameplay validation is implied.
