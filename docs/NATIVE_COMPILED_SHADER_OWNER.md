# Actual compiled-shader reflection owner

Addresses: `00B38310`, `00B38390`, `00B38490`, `00B3A660`, `00B3B1E0`,
`00B3B260`, `00B5BB40`; read-only producer evidence inside `00B3B3C0`.

The new module constructs and destroys the actual 88h `D61810` reflection
metadata owner. It also implements the actual 20h constant-record constructors
and the owning array helpers used at owner+78. This owner is distinct from the
10h COM shader wrappers stored in the material pass base. No COM operation is
present in this owner's destructor. Names below are reconstruction hypotheses.

| Entry or fragment | Original ABI | Coverage |
| --- | --- | --- |
| B3B57C..B3B5B7 and B3B5D6..B3B611 | Inline B3B3C0: ESI=fresh88h, EBP=0 | Complete constructor writes; allocation and pass publication remain the caller's responsibility |
| B5BB40..B5BBB2 | ECX=raw20h record; EAX=same; RET | Complete normal body |
| B38310..B38381 | ECX=destination record; stack source; EAX=same; RET4 | Complete normal body, including destructive self-construction semantics |
| B38390..B38486 | ECX=actual0Ch array; signed requested capacity; RET4 | Complete normal valid-domain body, including missing saved post-free block |
| B38490..B38546 | ECX=array; signed requested count; RET4 | Complete normal valid-domain body |
| B3A660..B3A6CF | ECX=array; stack source record; RET4 | Complete normal valid-domain body |
| B3B1E0..B3B23C | ECX=actual88h owner; RET | Complete normal valid-domain destruction, including missing saved tail |
| B3B260..B3B27D | ECX=owner; stack flags; EAX=original pointer; RET4 | Complete normal scalar deletion |

The C++ APIs have new typed ABIs. They do not emit the native private SEH tables
or reproduce native exception unwinding. Invalid extents, signed negative
counts and multiplication overflow are outside the supported readable domain.
No faulting native pointer path is converted into successful work.

The producer was verified from the actual B3B3C0 listing. EBP is zeroed at
B3B3F9 and is not assigned again before either constructor; later reassignment
at B3B7BE cannot supply the constructor values. Allocation calls B3B56E and
B3B5C8 push88h and clean four bytes. Each successful allocation writes CEB130,
count04=1, then D61810, byte74=0, DWORDs78/7C/80/84=0, then memset(+08,FF,36h).
The memset calls clean0Ch at B3B5B5/B3B60F. Pass+70 and pass+74 are published
at B3B5C5 and B3B61C. Bytes3E..73 and75..77 remain untouched. Word84 is not
given a speculative meaning by this module. The existing semantic
`ShaderConstantBindings` initializes a host counts array and is not this raw
producer or a source for its preimage.

Record B5BB40 leaves00..13 untouched, sets the actual string14/18 to zero,
calls the existing equal-length resize(0,false), and writes word1C=37h.
B38310 copies five words in order before clearing the destination string,
then uses resize(source length,true), re-reads source length/data and destination
length/data after that call, and finally copies word1C. It does not call a
semantic string or constant conversion. All names use the caller's SAME
`NativeStringStorage`; production composition must supply the existing actual
pool/publication/lifetime domain.

Reserve B38390 clamps to16, allocates requested*20h via the existing CRT/new
service, and copies while re-reading the current source count and data.
It releases old names in forward order while re-reading count/data, frees
the current old data, then publishes replacement data followed by capacity.
Growth in B38490 constructs each new record but publishes count only at the
end. Shrink decrements the current count before every reverse-order string
release and leaves released headers stale. Append B3A660 reserves
max(capacity+8,16) only when count equals capacity, copies the caller's record,
then increments the current count. Known callers of the shared record helpers
were also checked: B5BD10/B5BED0 use a different array growth policy, and
B5BE10 calls the same default record constructor. Those caller bodies are not
reimplemented here. B3A750 constructs a temporary20h record then passes it to
B3A660 with ECX=owner+78; its actual record-producing B5BC60 path remains a
separate dependency.

`NativeCompiledShaderArrayOperation` retains the acquired replacement pointer,
completed copy/default counts, current failing record and append-source identity.
A failed call keeps its original array and acquired rows; no destructor silently
rolls those effects back. Destruction of a running/failed operation terminates
rather than silently dropping its acquired identities. A diagnostic harness
may explicitly clean acquisitions, clear both tracked pointers and acknowledge
`diagnostic_retired`; that state is never native completion or a production
cleanup implementation. The outer compiler/reflection frame must retain this
operation, the owner, the original array and the append source/name until an
explicitly reconstructed completion/cleanup path is available. It must prevent
owner retirement while the operation is running or failed. This is a host
continuation limit, not native SEH parity. The module adds no successful default
provider or independent owner registry.

D61810 contains exactly BD30E0/B3B260 followed by the `ShadowTexture` literal.
BD30E0 reads the current vtable, pushes1 and invokes current slot04. B3B1E0
shrinks owner+78 to zero, frees its current data, then BD30F0 writes CEB130.
It does not reset the stale data/capacity, reference count, byte74 or word84.
B3B260 always destroys and frees iff flags&1; direct deletion does not decrement
or test the count. `NativeCompiledShaderReference` borrows the same raw+04,
uses the existing zero-reference thunk/owner domain, and retires its companion
only after destruction/free. It permits explicit scalar deletion at nonzero
count. Callers must keep the same bind/find entry live through native teardown.

Saved Ghidra body gaps were verified read-only by proto and raw disk assembly:
B38466..B38474 (replacement publication), B3B21A..B3B23C (base destructor and
return), B3B275..B3B277 (free argument cleanup). They are recorded as inclusive
`no_ghidra_function` ranges. The call B3B227 -> BD30F0 is a numeric raw-body row,
separate from the report's live-verifier rows. Primary integration must repair
these missing body blocks before claiming complete saved analysis. No Ghidra
mutation, re-import or game-installation write was performed.

Validation and artifacts are recorded in `reports/native_compiled_shader_owner.json`.
The ignored original/source fixture uses actual pool/CRT services and original
instruction bytes from nine ranges (1014 bytes,329 instructions). Its producer
adapters establish ESI/EBP and restore the host call frame; the seven complete
body adapters preserve their native argument cleanup. Native private exception
handlers are not executed. No compiler success, D3DX reflection success,
complete shader execution, game reachability or rendering parity is claimed.
The same failed-reserve portion is also run in a child process whose terminate
handler exits77: destroying the failed operation reaches that expected exit.
Normal diagnostic cleanup instead acknowledges the distinct retired state.
Required next work remains the real B3B3C0 compiler tail and B3AEA0/B3A750/
B5BC60 reflection population, along with the separate actual COM shader owners.

## Integrator saved-body repair

All three omitted post-free blocks were repaired under the Ghidra write lock.
B3B1E0 was recreated through B3B23C, preserving prior metadata: 28 instructions
and no gaps. B38390 and B3B260 have no remaining call gaps; the three unreachable
padding bytes after B3842B remain untouched. B3B227 to BD30F0 is now owned
by the full destructor and included in numeric call verification.
