# Whole raw UnitGroup direct views

This packet reconstructs four complete MSVC Win32 entries, **86 bytes and
26 instructions** in total. Every final Source COFF and linked byte equals
the corresponding installed-PE and live Ghidra byte. The four raw bodies
have no calls, global reads, relocations or patched bytes. Descriptive names
remain hypotheses rather than recovered source symbols.

| Entry | Bytes / instructions | Exact behavior |
| --- | --- | --- |
| 0070D060 | 14 / 4 | Load the member DWORD at actual group+18h+index*34h; RET 4 |
| 0070D070 | 14 / 4 | Return that actual record address; RET 4 |
| 0070D0F0 | 7 / 2 | FLD actual retained float at group+504h to ST0; plain RET |
| 0070D100 | 51 / 16 | Publish stacked float bits to every matching actual record+30h; RET 8 |

The first and only native run of the corrected candidate passed **370
checks, four cases, seven Original/Source pairs and fourteen calls**. It
uses three fresh TUs: the four new raw leaves, the current genuine Source
group constructor and one fixture. No BSP archive is consumed. The primary
integrator owns CMake, shared metadata/annotations and the full build.

## Admission and native details

All entries use actual group ECX with unused EDX retained in the C++
fastcall spelling. The caller supplies valid aligned 508h group storage,
actual 34h records, compatible floating state and any indexed backing that
will be accessed. Index multiplication wraps in 32 bits, and neither indexed
routine checks the active count. The member routine returns the actual
pointer-valued DWORD without dereferencing the pointed-to object.

The publisher first executes `cmp dword ptr [ecx+4F8h], eax` with EAX zero,
then JLE. Thus signed nonpositive count exits before the XMM0 load. Positive
count loads the exact stacked bits through MOVSS, saves ESI and walks real
records. Each match stores those bits at record+30h, including duplicate
and null member words. The loop uses the opposite operand order,
`cmp eax, dword ptr [ecx+4F8h]`, and rereads that signed count every
iteration. Positive count must fit genuinely valid record backing.

The speed argument is a DWORD so C++ performs no floating conversion before
the raw MOVSS. Positive-count XMM0 contains the exact low word and zero upper
96 bits. Nonpositive count preserves all XMM0 bits. These moves preserve
MXCSR and do not quiet a signalling NaN payload. Publisher EAX exits with
the traversed count or zero; EDX exits at group+48h+count*34h for positive
count, or retains its entry value on the early path. ESI is restored.

The getter needs at least one free x87 slot and returns the actual field in
ST0. Its input is **caller-retained group+504h storage**. The current Source
constructor explicitly leaves that word unchanged. This packet neither
supplies a default nor reconstructs the separate 0070DA00 speed calculation.
The getter preserves incoming integer volatile registers, XMM0 and MXCSR.
No production code resets floating state or validates invalid inputs.

## One small coherent family

The fixture uses one aligned 1472-byte backing: actual group at offset20h,
its full 24 records, guards and three distinct pieces of live backing named
by the pointer-valued member cells. Those member identities are never
dispatched or dereferenced as native classes. The same actual addresses are
restored between Original and Source. Both sides begin with the genuine
Source constructor's output and the caller's live field values.

The actual caller+504h input is written **before** each Source constructor
call. All 989 constructor write bytes, the 299 retained group bytes and
outer guards are checked. The constructor borrows actual CF4888 data bits
4479C000 (float999.0), pinned from PE and live Ghidra. This constant is
distinct from its CFD6F8 raw vtable stamp, which remains uncallable data.
The complete Source constructor compiles to 132 bytes/39 instructions and
stays unchanged before/after setup. It runs four times; Native constructor
execution remains zero. Source setup does not establish native class ABI,
allocation, construction or lifetime.

| Case | Publisher input | Verified result |
| --- | --- | --- |
| Duplicate nonnull | Count4, raw bits7F800123 | Both matching records receive the unchanged signalling-NaN bits |
| Duplicate null | Count4, raw bits80000000 | Both matching null records receive negative-zero bits |
| Zero count | Count0 | All storage and all XMM0 bits remain unchanged |
| Negative count | CountFFFFFFFF | Signed early exit; all storage and all XMM0 bits remain unchanged |

Only the first case adds the other three calls. Index40000017h multiplies
with 32-bit wrap to the offset of actual record23, which is valid retained
backing outside active count4. Both the member identity and record address
match; defined CF/OF overflow flags are set. The other IMUL flags are
compared as native incidental state, not interpreted as arithmetic results.
The getter then returns the actual retained7.25 as exact 80-bit ST0
`00000000000000E80140` (little-endian bytes), preserving two existing80-bit
canaries below it. This adds no unrelated coverage cases.

Every operation compares all 1472 backing bytes, including guards and the
retained outside-count record. Each call saves 220 bytes containing physical
registers, full108-byte x87 state, XMM0, MXCSR, actual entry ESP, 32 stack
guard bytes and actual arguments/count. All 220 bytes compare exactly
except the natural getter FPIP DWORD. All 80 x87 payload bytes compare;
RET8, RET4 and plain RET balance correctly at one physical adapter callsite.
The adapter calls through memory so the getter sees identical incoming EAX
instead of two different function addresses.

Non-floating leaves preserve the entire108-byte x87 state. The getter
changes TOP6 -> 5, tag0FFF -> 03FF and SW7520 -> 6D20 with the pushed result;
its data pointer is the actual group+504h. CW037F and MXCSR00001FA0 survive.
Observed EFLAGS is246 on positive/zero publication and the getter,286 on
negative publication, and A47 on wrapped indexing. Full Original/Source
flags match; incidental/undefined bits are observations for this fixture,
not portable semantic constants.

## Preserved correction and reproducibility

Primary review caught a reversed first CMP in candidate01 before any
native execution. That strict build succeeded, but its publisher byte2
was3B instead of Original39 and would reverse the signed early-exit
behavior. All original frozen inputs, objects, executable and full COFF/
linked mismatch evidence are preserved. Only the first CMP was corrected.
Candidate02 was built separately and all86 bytes were verified equal before
the sole native run. Neither candidate01 nor any older family was executed.
Both compiler runs succeeded; there was one rejected Source candidate and
no native execution failure.

`reports/cc11_unit_group_direct_views.json` indexes the sealed directory
`local/cc11_unit_group_direct_views_20261007_a`. Its recipe specifies the
corrected three-TU build, exact four-argument execution command, inputs,
whole-body/adapter evidence, runtime snapshots and artifact manifest.
Reproduction must use a new directory and preserve both candidates here.
The check count includes artifact I/O, setup and preservation checks.

Before compilation, the fixture pins its six actual Source/header inputs,
266 host headers, seven system libraries, three selected tool files and
the selected c1xx/c2 compiler backend files. Actual include/library traces
resolve within those pins. All hashes remain stable after compile and
execution. Flags include `/MD /O2 /Gy /W4 /WX /fp:strict /showIncludes`;
the I386 PE embeds `asInvoker` through `/MANIFEST:EMBED`. No mutable main
build library is pinned or consumed; its recorded hash is context only.

All 915 current prior paths, including the previous581-path baseline and
203 sealed leader artifacts, remained unchanged. Authorized publication
had already changed the leader documentation/report; exact worker versions
are newly preserved, with earlier publication changes separately recorded.
The installed/frozen original PE, all four raw bodies, Source constructor,
native constant and guards remain unchanged. Ghidra stayed read-only and
no tracked tests were added.

Full0070DA00 calculation, invalid pointers/counts/indexed backing, unmasked
floating faults, concurrent mutation, original class/vtable ABI, allocator,
EH, ownership/lifetime, enclosing callers, world integration and gameplay
remain outside this conditional raw-entry evidence.
