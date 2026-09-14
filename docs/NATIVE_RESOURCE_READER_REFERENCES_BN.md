# Actual reader and reference leaves BN

This packet reconstructs four complete ordinary bodies over actual Win32
storage: `BF09A0` (16 bytes), `BF0430` (57), `BE9ED0` (41), and `483850`
(43). Their 157 installed-PE bytes match fresh Ghidra bytes; all 70 listed
instructions belong to the four complete bodies, with zero gaps. These are
new C++ interfaces, not drop-in machine ABI replacements. No CMake, runtime
dispatch, ledger or Ghidra mutation is included.

## Storage and original ABI

| Entry | Native input and result | Actual accessed storage |
|---|---|---|
| BF09A0 | ECX reader; EAX original reader; RET | Four DWORDs at reader+0/+4/+8/+C |
| BF0430 | ECX reader; stack new stream; RET4; no stable result | Stream word at reader+0; referent LONG at +4 |
| BE9ED0 | ECX handle; RET; no stable result | One DWORD slot; referent LONG at +4 |
| 483850 | ECX resource slot; EAX captured slot; RET | One DWORD slot; referent LONG at +4 |

The 10h reader base contains an attached stream and three optional-buffer
words. Construction simply writes four zero DWORDs in increasing address
order, returning the original base; it does not release previous contents.
The release slots occupy 4 bytes. Each nonnull referenced object has a DWORD
table pointer at +0 and an aligned signed 32-bit reference count at +4.
The C++ interfaces borrow this actual storage directly. They create no
projected reader, handle, resource owner, allocation or automatic lifetime.

## Publication, reference and callback order

`BF0430` captures old stream in ESI, compares it with the stack input, and
does nothing when equal, including null/null. Otherwise it publishes the new
pointer at reader+0, interlocked-increments new+4 if nonnull, and only then
interlocked-decrements captured old+4 if nonnull. Only a zero decrement reads
the old object's **current** table and slot0 and invokes it. There is no
post-callback reader write. A callback can replace the reader word without
this leaf restoring the published pointer or releasing the replacement.

`BE9ED0` captures slot in EDI and pointee in ESI. `483850` captures slot in
ESI and pointee in EDI. Both leave null contents untouched. For a nonnull
pointee they decrement its +4, call current slot0 only at zero, and then clear
the **captured slot**, even if a returning callback changed that word.
`483850` returns the captured slot; `BE9ED0` supplies no stable EAX result.
Neither re-fetches the referent to release a callback's replacement.

All three terminal sites load EDX from the captured referent's current +0,
load EAX from [EDX], put that referent in ECX, then CALL EAX without stack
arguments. This exactly fits existing
`NativeAdoptedSubstreamDispatch::source_zero_reference(entry, owner, table)`.
Its callable adapter passes ECX=owner and EDX=table. Numeric native profile
entries still require an actual reconstructed dispatcher; they cannot be
called as host code merely because a DWORD holds a native address.

The only other transfers are four imported calls: `BF0445` through IAT
`CE221C` (`KERNEL32!InterlockedIncrement`), and `BF0453`, `BE9EDE`, `48385E`
through `CE2220` (`KERNEL32!InterlockedDecrement`). Each receives the actual
captured referent+4 as its one stdcall argument. The source uses the real
Windows interlocked operations. No direct CALL, tail dispatch or external
branch exists in these four bodies. Eight conditional branches stay inside
their owning bodies; the four returns are recorded separately from calls.

## Terminal failure and ownership limits

There is no local native EH registration, unwind action, allocation, base
profile reset, seek or implicit teardown in these four bodies. If terminal
dispatch throws through the new C++ interface, assignment retains its new
publication and both completed atomic changes; the slot helpers skip their
post-call clear. Callback changes also persist. The source deliberately adds
no rollback, compensating decrement, rethrow helper or synthesized native
unwind. The fixture checks these source-visible effects; it does not establish
native CRT/FH3 interoperability or hardware-fault behavior.

Whether a failing terminal has destroyed or freed its referent is unknown to
these leaves. The captured pointer must not be treated as a surviving owned
reference merely because a failure skipped clearing its slot. Callers supply
live valid storage for all required accesses, including the captured slot's
post-callback write on normal return. Nonzero counts do not require a readable
virtual table. No extra null-slot guard, underflow clamp or synchronization
around ordinary pointer words is introduced.

This packet does not reconstruct full reader teardown, the structured-node
terminal, root dispatch, the resource manager, or concrete game-resource
deletion. In particular, it does not assume missing `718C20 -> 718810 ->
B88430` is callable. The resource ownership and cache-removal gates remain
those recorded in `NATIVE_RESOURCE_LOADING_FRONTIER_BN.md`.

## Validation and retained evidence

Fresh read-only `bsp.py ghidra` queries verify `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 Win32 and image base 00400000 before each
batch. The report includes the full four spans and hashes, installed-PE import
identities, all seven indirect call rows, eight local branch rows and four
returns. Whole-report call verification reports zero direct rows and zero
failures, and explicitly skips all seven indirect rows. Independently, each
indirect site's bytes, native listing instruction and owning complete body
were checked; a zero-direct-row result alone is not that proof.

The new translation unit compiled with MSVC Win32 `/std:c++17 /EHsc /MD /W4
/WX /fp:strict /permissive-`. One ignored actual-storage fixture passed:
constructor boundaries and return, publication/retain/release order, same
pointer and null cases, zero-only table dispatch, actual callable ECX/EDX,
callback replacement, and throwing-terminal effects in assignment and both
slot leaves. It links the previously frozen `bsp_core.lib`, SHA-256
`134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`;
the existing adopted-substream source/header are unchanged from that packet's
base. Its safe-named `probe.exe` embeds a manifest. Artifacts and provenance
are under ignored `local/native_resource_reader_references_bn/`.

No permanent test was added. The worker did not run a repository build that
omits this unregistered translation unit; the primary owns integration and
the combined build. This is source and fixture validation, not execution of
the original four bodies, game-resource destruction or gameplay validation.
