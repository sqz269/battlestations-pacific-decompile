# Opaque vector-storage tidy

Address: 00468840.

| Native address | Source API | Native span | Coverage |
|---|---|---|---|
| 00468840 | tidy_native_vector_storage_00468840 | 00468840..00468869, 42B/13ops | complete |

The body accepts actual opaque16B storage through ECX, ignores incoming EDX,
preserves ESI and uses plain RET without stack arguments. It captures DWORD+4
once, tests that captured value, and only if nonzero passes it to the actual
current-CRT free service. The call uses one pointer stack argument and ADD ESP,4.
After either returning path it writes immediate zero to+4, then+8, then+0Ch.
There is no field reload after free and no null-path early return. The Source
fastcall void API retains that physical receiver shape and ordered raw volatile
memory schedule without claiming an original binary ABI or semantic EAX result.
The native null path leaves EAX0; the nonnull path retains ambient free EAX.

The body never directly accesses+0 or any element payload. Live xrefs are empty,
so no actual producer/caller binds an element type or the semantic meanings of
+0/+8/+0Ch. These are opaque field offsets, not asserted allocator/begin/end/
capacity members. The correct STL_Vector_Tidy native library name is preserved;
the Source name describes the observed storage schedule, not a recovered type.
No destructor, diagnostic, callback, owner overlay or added validation is used.

The sole native edge is `0046884B CALL 00BF65AC`. Native free is a JMP thunk;
the concrete accepted Source binding is singleton_lifetime_free over actual
std::free. This is the existing current-CRT service boundary, not a stub or a
reconstruction of the original CRT body/heap/exception policy. Valid taken raw
backing and current-CRT release validity remain required. Zero+4 skips free but
still writes all three zero fields; no state narrowing is introduced.

All42 live bytes match the installed PE. The complete typed body has one42B
range, returning/non-thunk, with no gaps. Typed conditional flow at8848 branches
to8853 with fallthrough884A; the free CALL at884B has unchanged ordinary call
flow, target00BF65AC and fallthrough8850. No flow or prototype repair is made.
There are no named-incomplete dependencies at the accepted free boundary, and
no existing indexed reconstruction or Source implementation was found.

Guarded live batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000. Annotation takes the shared
write lock, records prior values, preserves correct names/comments, saves the
project and refreshes the owned export, with a subsequent live readback.
The Source draft carries full assembly, typed flow and original byte evidence;
the normal Win32 build and existing checks are pending. No new routine tests
are added. Compilation/static evidence do not establish private-frame,
register/flags/mangling/fault/CRT/FH3/SEH binary identity, direct native
differential execution, application adoption or game behavior.
