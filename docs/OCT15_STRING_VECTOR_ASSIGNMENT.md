# Actual checked pooled-string vector assignment

`assign_native_string_header_vector_00506c80` reconstructs the complete
ordinary `00506C80..00506DC1` body over actual checked-vector storage. The
owner is10h bytes: opaque/proxy+0, begin+4, end+8, capacity end+0Ch; each
element is an actual8h length/data string header. It constructs no replacement
host vector or overlay. Original ECX is destination, the one stacked word is
source, RET4, EAX destination. The extra explicit `NativeStringRawPoolContext`
and ordinary Source C++ signature are not that original ABI. The saved Ghidra
prototype has no recovered parameters; the ABI here comes from the complete
144-instruction listing and concrete caller setups, not that prototype.

The project/program was verified as `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe` before live queries. Both halves of the complete
listing, full322 saved bytes and installed PE bytes were read. There are nine
normal direct calls,28 basic blocks, and no local EH registration, catch or
rollback. Descriptive Source names remain hypotheses; the established Ghidra
name `BSP_StringVector_Assign` is retained.

| Path | Preserved schedule |
|---|---|
| Self | Compare owner addresses before all header reads; return destination. |
| Empty | Null source begin skips source end; otherwise captured end-begin SAR3 zero also clears. Invoke actual checked clear; retain backing. |
| Shrink/reuse | Compare unsigned source count to destination size. Assign captured source range into captured destination begin; destroy from returned assignment end to CURRENT destination end. Then reread source begin, conditionally source end, and destination begin before publishing the recomputed end. |
| Grow/reuse | If count fits capacity, reread destination end to form size and split against initial source begin. Assign prefix. Read CURRENT destination end, then CURRENT source end; uninitialized-copy suffix and publish its returned end after success. |
| Replace | For nonnull captured begin, destroy captured old range and free CURRENT begin. Reread source begin/count; Buy clears pointers before allocation. Test AL only. On success read CURRENT destination begin, source end, source begin in that order, construct, then publish returned end. |

DWORD subtraction/addition and SAR3 sign extension are explicit unsigned
operations. Counts and capacity decisions use native unsigned comparisons;
no signed C++ conversion or cross-object pointer subtraction is introduced.
Raw assembly load/store helpers retain unaligned DWORD addressing and read
order. Header+0 is untouched by this function. External owner/data aliases are
not rejected; every taken read/write and backing lifetime must remain valid.
The extra Source context and private frame differ from original stack aliases.

All dependencies compose existing actual services: raw-pool range assignment,
raw-pool range destruction, checked erase/clear, native byte-count allocation
and owning legacy length-error transport, and uninitialized-copy with its
completed-prefix cleanup. `singleton_lifetime_free` supplies the accepted
current-CRT byte domain corresponding to the original free leaf. No arbitrary
allocation/copy/clear/throw callback or generic STL vector is added. Current
CRT allocation/new-handler policy, omitted zero-byte copy calls and child
Source C++ exception boundaries remain qualified. No local cleanup is added
when a child throws, even after old backing was freed or Buy cleared fields.
The original FH3/SEH/catch-type and fault identity are not proved by Source.

Two representative incoming loading-screen setups establish ECX destination
and the stacked source at concrete global owners `00E08780` and `00E0879C`.
The caller inventory contains21 functions; a capped40-site xref query was
used for orientation. This packet does not claim all incoming sites reviewed.
All normal calls within the owned body are checked below.

```text
00506CA5 CALL 00498FF0
00506CCD CALL 0048F400
00506CE0 CALL 00432050
00506D44 CALL 0048F400
00506D57 CALL 00505AD0
00506D73 CALL 00432360
00506D7C CALL 00BF65AC
00506D9A CALL 00432090
00506DB1 CALL 00505AD0
0057CFFA CALL 00506C80
0057D06A CALL 00506C80
```

Win32 Release and all three existing suites passed after the accepted allocation
and erase/clear packets were integrated. All11 call-site claims passed. The
existing native math differential suite covers its seeds, not this vector.

One local probe compares the full original322B parent body against the new
Source parent for reuse/grow from one to two empty strings in three-slot actual
checked storage. Its two taken calls use fixed adapters to the same accepted
Source range-assignment and uninitialized-copy services with a shared raw-pool
context. The other seven calls target a fail-fast sentinel and remain untaken.
It verifies payload equality, destination opaque/begin/capacity preservation,
end publication, initialized suffix, untouched spare slot, unchanged source,
return identity and one invocation of each child. The empty strings require no
pool initialization, allocation or release. All checks pass. This is bounded
original-parent composition evidence, with Source children; it is not an
independent original dependency-chain, other-path, exception, ABI or gameplay
comparison. No tracked test suite was added. Exact inputs, installed byte match,
driver path and output are embedded in the packet report.
