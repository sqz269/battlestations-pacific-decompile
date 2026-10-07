# Landing queue producer boundaries

Primary reviewed all 141 native instructions in `006C0B50..006C0D1D`
(exclusive) and the complete 76-instruction append helper
`006BF720..006BF7E5` (exclusive), after the recorded flow repair. This is native
evidence and dependency recovery; a complete source producer is still unbound.

The producer preserves distinct identities: cached task+404h, the fresh input
plane+9D4h queue argument used by lower constructor009AFE70, and the returned
landing-holder identity. The source host registry/index cannot substitute for
those actual objects or lifetimes.

Queue records are five DWORDs. The scan stops at the first matching squadron.
If record+4 is nonnull, the function returns that VALUE, not a queue-record
address or index. If it is null, scanning stops and another record is appended;
the new sequence is computed only from preceding records. The current host
deduplicates matching squadrons and computes a maximum over the whole vector,
so even that portion does not implement the native producer.

The new record captures block+80h in record+4 before append, stores the actual
input squadron, sequence, positive-zero and one payloads, and calls the append
helper on block+84h. The function subsequently updates every matching launch
slot, notifies the current owner with event4/value0, constructs message84 and
routes it. Only after those effects and the captured slot-lock release does it
return FRESH block+80h. Provider effects may therefore make the return differ
from the newly stored record+4; that possibility is not native reentry proof.

The queue and slot lock pointers are separately captured from block+4 and +Ch.
Their ordinary release uses the respective captured sections. This differs
from the health transport wrapper's fresh-field exit contract. Native private
EH, invalid ownership and exceptional cleanup remain outside a source claim.

Append first registers the actual squadron endpoint with the actual callback
owner at block+84h through `00694A60` (ECX endpoint, EDX callback owner). That
complete ordinary observer registration already exists as a reconstructed
storage routine, but still requires its real endpoint/lifetime/lock/allocator
context. Growth uses capacity2*n+2 and 14h-byte records; it copies all five
DWORDs, frees the old allocation, publishes the replacement and then appends
the new record/count. Actual native allocation/free services and all backing
lifetimes are required; a vector/default allocator or no-op observer is absent.

The stored Ghidra listing skipped bytes `83 C4 04` at006BF7A9 after the returning
CRT free call006BF7A4. Primary retained prior documentation and exact instruction
context, verified disk/live gap bytes, and used the supported write-lock flow
repair. Its response records the prior CALL_RETURN override and new NONE.
`ADD ESP,4` is decoded; zero call gaps remain. Names, comments and callee
no-return flags are unchanged. The project was saved and exports refreshed.
The optional exact flow-property script query was disabled by the bridge;
no mutation fallback was attempted for that query.

Remaining complete boundaries are named explicitly: observer slot0C dispatch
`00696350 -> 00696120` (existing source only provides other slots), message84
construction `006BD520` with actual class resolver `0095B9C0` and live slot
fields, and session/peer/sender-copy routing `0077C7B0` with actual message
lifetime. A useful source packet must expose/adopt the actual lower constructor
queue boundary and keep those operations complete and required. It cannot skip
the tail or return a host registry index. No queue source, original full game
ABI, provider binding or gameplay validation is claimed by this audit.

Flow receipt: `reports/landing_queue_append_flow_recovery_cc11.json`.

Primary holder-destructor listing repair: complete006C23D0..006C23FA (exclusive) now contains all14 instructions. The3-byte disk/live ADD ESP,4 at006C23F1 follows free CALL006C23EC; the supported locked repair clears its local flow override, saves the project and refreshes the export. Zero CALL gaps remain; existing scalar-deleting-destructor name and callee global NoReturn are preserved. The body stampsCF86A4, invokes006BF880 on globalE19948 and frees only for flagsbit0. This is analysis/listing evidence only, not a holder-lifetime reconstruction or evidence that queue-record/task-cache pointers own or retain the holder. Report: reports/landing_holder_destructor_flow_recovery_cc11.json.

Primary message-profile definition prerequisite: byte-verified, saved, unnamed complete entries006BD600..006BD61F (31B),006BD710..006BD79D (141B),006BD7A0..006BD7F4 (84B), exclusive ends; report reports/landing_message_profile_definitions_cc11.json. CF8610 contains those scalar/writer/reader/type/validity entries; no profile interpretation is supplied for type49/46. Validity7A0 is conditional, not always true: WORD34 zero returns AL1, otherwise signed threshold F89A10 chooses the actual F89A0C/F89A54 or F89A60/F89AA8 16-byte record bank and tests record+0C. No clamp, shadow bank or fabricated default is justified. Native0095B9C0 is a null-tolerant vehicle-class lookup, not a destructor: call actual437F50 singleton getter, then freshly load original identity+70 and registry+2010+index*4. Definitions and refreshed exports are analysis evidence; complete message/base/cursor/current-bank/registry bindings, actual scratch lifetime, routing, queue caller and original ABI/game remain separate reconstruction work.
