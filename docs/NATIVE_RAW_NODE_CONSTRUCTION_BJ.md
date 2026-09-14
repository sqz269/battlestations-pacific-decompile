# In-place raw child node construction BJ

Address: `00BEA250`. The complete 300-byte installed PE body matches the
live `/battlestationspacific.exe` Ghidra body at 63954 functions. PE path and
digest, per-call listing windows, unwind map and a local double-hash inventory
are retained in `reports/native_raw_node_construction_bj.json`. The pinned
discovery is `NATIVE_RAW_NODE_CONSTRUCTOR_FRONTIER_BJ.md` at
`7e99c90a9f0311081dbb622122ade6f5c4b24d20`; its report SHA-256 is
`88fce4f0c041bbab02d74bfa63c980683000a9ae9d6e4a831cc32834cf959a22`.

The native constructor takes ECX as writable 24h node storage, one stack
parent-node pointer, returns the node in EAX, and uses RET4. The C++ interface
is context-bearing and Win32-only; it calls the existing raw `BF0510` name
reader, `BF0280` scalar reader, `0041DD40` raw string resize,
`00419CC0` pool getter, `BD1510` pool return, and `BD30F0` base
destructor. Name source comes from reviewed dependency
`e956fc7ac39f1b79084e998da51146e5ac8422f8`. No host string,
virtual reader, or allocation callback is introduced.

Construction first stamps `CEB130`, sets refcount one, stamps `D68BB4`,
borrows parent reader and parent pointer, zeroes the name header, and writes
depth as parent depth plus one modulo 32 bits. The name provider publishes
actual bytes before constructing its output. On normal return `BF0510`
debits parent remaining bytes, then this constructor copies the returned
temporary into node name. The returned header address is compared to node name;
after resize, current source length is tested and current destination length
and both data pointers are loaded for the overlap-compatible copy.

The actual local temporary's data and length are captured. State 2 is disarmed
to state 1 before resolving the current pool and returning that block. Next,
`BF0280` debits the parent budget and its result is written to declared
payload `+1C`, then remaining payload `+20`. The current reader's path
index selects `reader+10+8*index`; `reader+60` is incremented before
resizing or copying the path name. The path header must already be valid,
initialized, writable raw 8-byte storage supplied by the caller. The native
body supplies no capacity, range, null, or full-read guard.

The native FH3 map is state 2 local temporary, state 1 node name, then state 0
`BD30F0` base stamp. Source unwind follows that sequence. After temporary
cleanup is disarmed, a pool-getter failure does not retry it. Failure after
parent debit or path-index publication does not roll either change back. The
general node destructor `BE9DF0` is not part of constructor unwind.

The allocating wrapper `BEA680`, native allocator/free ownership, root
reader/path storage creation, and the full `B7EB90` producer remain open.
This new source interface is neither an original native ABI replacement nor
FH3/asynchronous-fault or gameplay parity proof. Build and existing CTest
results, exact object/archive-member comparison, and the retained compiler
command are recorded in the report.
