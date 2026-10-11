# Root locale numeric cleanup target body review

Root accepts the complete saved nine-byte range at C11C18:
`6A0AE812FFFFFF59C3`, PUSH10/CALL C11B31/POP ECX/RET. Root independently
checked all18 initial per-byte raw metadata responses and the2+5+1+1 whole-unit
partition before admitting bytes. The closing nine responses agree. Root manually
decoded all four opcode classes, signed E8 displacement -238 and numeric target,
then ran actual detailed Capstone5.0.7 and compared all instructions/operands/
encoding offsets. No callee, data, provider, prototype or adjacent range was queried.

On normal local completion, PUSH writes10 through currentSS[S-4], and E8
writes returnC11C1F through currentSS[S-8]. If the external call returns, its
entire register/ESP/EBP/flags/segment/memory state is unknown. With returnedESP Q,
POP reads currentSS[Q] and RET reads currentSS[Q+4], with finalESP Q+8. The
actual destination and relation to entry remain unknown. POP is not authenticated
as argument cleanup, and the saved name establishes no unlock/locking meaning.

Root checked81 core manifest members, six actual frozen Git objects and eight
finite immediate inputs. Final delivery has73 manifest members,12 object records/
ten distinct actual objects,eight unchanged inputs and exactly two final outputs.
Actual Git/index/physical bytes agree. Original and distinct-copy ZIPs match with
distinct file IDs. Both fully reviewed pinned pure Root replays passed with one
ZIP read,one helper execution,one receipt write and no denials. Root gates/helper
preimages and all10 Native command streams remain retained. Worker commit:
`69b5786b2bb278d679b8b3a54fd8c4b8757f5e4c`.

Private evidence: `local/cc12_locale_onexit_lock_initializer_cleanup_body_primary/`.
Core SHA-256: `1e958734366ccc067ea1256192a2d4806790679eecc0763fe91e400ec63b8d16`.
Delivery SHA-256: `e29b6fa611c71a32208a0f51628b0ee681ea5f0d21594d7698049162b99fa128`.
The public report pins Root gate, independent actual decode/Git reviews and
strict replays. No callee/provider semantics, Original ABI, Source, fresh build,
fixture, startup or gameplay credit is added. Worker Source IDs are frozen
revision identity only and are not asserted equal to current Root Source.
