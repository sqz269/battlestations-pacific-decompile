# Root locale target body review

Root accepts 56 physical instructions in three saved ranges totaling 168 bytes
at `00C11B5E`. The excluded `00C11BE7..00C11BF7` and `00C11C02` remain unread.
Calls at C11BE2 and C11BFD physically push return addresses into those gaps.
Their saved CALL_RETURN overrides do not establish actual non-return or padding,
so complete actual control flow remains unresolved.

Root independently checked the paired whole-unit metadata gate for every admitted
byte. Root fully reviewed the independent worker opcode/ModRM/SIB decoder and
ran actual detailed Capstone 5.0.7 over all three ranges at their actual bases,
comparing all 56 instruction bytes, operands and encoding fields. Root does not
claim to have authored a second independent manual decoder.
Every external call remains an unknown returned-state boundary. Every later
memory access uses the EBP, ESP, registers, segment and contents current at that
instruction; pointer, table, locking, CRT and exception-frame meanings are not
established by names or literals.

Root checked the core archive's 130 manifest members and six actual frozen Git
objects with nine finite inputs. The final delivery has 74 manifest members,
12 object records representing ten distinct actual Git objects, nine unchanged
inputs and exactly two outputs. Root compared actual Git, index and working
bytes with those retained objects. Original/copy ZIPs match with distinct file
IDs. Both Root core and delivery restricted replays passed, each with exactly
one ZIP read, one receipt write, three pinned pure helper executions and no audit
denials. The same four fully reviewed helper bodies were pinned in both archives.
Worker commit: `8c5fe18e359fad8bbe99b8abd28cf136137cbe73`.

The failed original second-pass local atomic replacement (WinError 5) retains
46 complete raw responses; an earlier successful chunk retains another 64.
The two accepted complete sweeps plus closing sweep contain 504 authoritative
responses. All 614 retained responses are checked, with failure streams and
producer preimages preserved. The cause of the local file lock remains unknown.
This does not claim an error-free collection or Native identity failure.

Private evidence is under `local/cc12_locale_onexit_lock_initializer_body_primary/`.
Core SHA-256: `c92d5d9e6d3eddb1b02b071cf1c46525614088a65eccd87a3c959128d93e7610`.
Delivery SHA-256: `435a0cd2012d3fed5c8ba8f52585eded9ab5f7ff9b4dcbb06ed0a8085c5f06ec`.
The public report pins Root metadata, actual decode/Git reviews and guarded
replays. No Source, fresh build, fixture, original ABI, startup or gameplay credit
is added. Source subtree IDs identify the frozen worker revision only.
