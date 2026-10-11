# Root CRT first direct target body review

Root accepts exactly 50 physical bytes and 16 instructions at `00C05070`.
All 150 raw responses from two complete pre-read metadata sweeps and a closing
sweep agree at epoch 98/count 64730. Root independently checked the full initial
metadata gate, reviewed the pure opcode/ModRM decoder and ran actual detailed
Capstone 5.0.7 over the admitted bytes. All 16 boundaries, operands and encoding
fields agree. Root does not claim a separately authored manual decoder here.

The physical calls at C05077 and C05098 read current dispatch cells CE20BC and
CE20B4 before pushing their return addresses. Saved EXTERNAL scalars 104/102
are different metadata objects, with no authenticated provider meaning. The
direct E8 at C05089 selects numeric address C04FDE only. All three returned
states, stack/frame/register preservation and memory effects remain unknown.
The two E15B00 loads occur at their own instruction times after preceding pushes.
On a return from the final C05098 call, MOV EAX,ESI uses the returned ESI;
it does not promise a value from before that call. POP ESI and RET read the
stack memory actually current. No TLS, allocation, locking, caching, provider,
argument/return type or original ABI meaning follows from routing names.

Root checked 84 manifest members, eight actual frozen Git objects and twelve
finite immediate positive input blobs with actual Git. The final worker commit,
index and working files agree for exactly two outputs, and staged/final receipts
retain their exact pins. Original and distinct-copy ZIPs agree with distinct
physical file IDs. Root executed the fully reviewed pinned parser under a
restricted audit: exactly one supplied ZIP read, one helper execution, one
receipt write, no denials. Helper bodies, preimages, Root gate and all Native
streams remain retained. Worker commit: `f387187f5fdd35b440a649a46f48875fb1dcfa0f`.

Private evidence is under
`local/cc12_application_crt_getptd_first_direct_target_body_primary/`.
Archive SHA-256: `b3502946501de284814b07c492c15e577e0e1cd312a2dab83b066fd6693f7a98`.
Code SHA-256: `c56216e301258d74d1f73f4e30645d811c2e7a3cda5bef15b09e94735394ba81`.
The public report pins Root review, strict replay and conditional instruction
contract. No Source change, fresh build, fixture, Original ABI, startup or gameplay
credit is added. Frozen worker Source IDs are revision identity only.
