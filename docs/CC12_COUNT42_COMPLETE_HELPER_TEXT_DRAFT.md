# Element-count42 complete fresh helper TEXT draft

This packet provides a self-contained fresh Root materializer, two-TU fixture,
controlled build recipe, linked-image inventory, structural gate, independent
byte reader, and recorded binary reader **as text only**. Source credit is zero.
No template was imported or executed. No compiler, linker, original Native,
provider, or target process ran. No Root receipt was created or accepted.

The owned files are this document and
`reports/cc12_count42_complete_helper_TEXT_draft.json`. The exact draft family is
`J:/PROG/battlestations-pacific-decompile-cc12_count42_complete_helper_TEXT_draft/local/count42draft`.
Its final `artifact_manifest.json` includes every other file, including all
utilities, logs, stop records, frozen selected inputs, metadata, and commit
records. The final manifest hash and exact family count are delivered after the
two-file commit so commit records are included without a circular commit hash.
The manifest itself is the only excluded row; its hash is supplied separately.

## Selected evidence and identity

The chosen prior family is
`J:/PROG/battlestations-pacific-decompile-cc12_property_element_count_fresh_case_review/local/countcases`:
exactly 17 files, original manifest SHA256
`f2e153b1e824a45a3638bb693797b06a2fb2688f83aec635ecc27eee619ead1a`.
All 17 original files and all frozen copies retain their size and SHA256.
Current Main source/header and the selected review document/report were also
frozen and checked before and after this work. Original snapshot Main was
`5e4f1ab18`; original file pins, rather than a stale branch-tip assumption, are
checked by the proposed Root materializer and recipe.

The source is
`src/native_scene_property_record_element_count.cpp`, with declaration in
`include/bsp/native_scene_property_record_element_count.hpp`. Its symbol is
`bsp::native_scene_property_record_element_count_008ef7f0`.
The complete Native interval is `[008EF7F0,008EF81A)`, 42 bytes, 17 instructions,
SHA256 `e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7`.
The frozen source and fixture Native `_emit` sequences both reproduce those
exact 42 bytes. Root must separately read and pin the original PE and verify
its physical bytes before any fresh compile; no worker PE read substitutes for
that check.

This is the original Win32 register entry: ECX is an opaque receiver, EDX is
unused on entry, there are no stack arguments, and all three exits are plain
RET. The body reads the DWORD at +04 and reads +24 only for tags 9, 10, and 11.
It has no CALL, write, child/list access, or +20 payload read. Tags 9/10 return
unsigned size >> 2. Tag 11 computes the unsigned high half of
`size * 0xAAAAAAAB`, then shifts that high half by 3. Other tags return zero.
ECX, nonvolatile registers, ESP, DF, and ES survive; EDX changes only for tag 11.

| Case | Tag | Size at +24 | Fill | EAX | EDX after raw call | Defined flags |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 9 | 7 | A7 | 1 | D00DFEED | mask C5, value 01 |
| 1 | 10 | FFFFFFFC | 5C | 3FFFFFFF | D00DFEED | mask C5, value 04 |
| 2 | 11 | FFFFFFFF | D2 | 15555555 | AAAAAAAA | mask C5, value 04 |
| 3 | FFFFFFFF | A76CD291, unread | 69 | 0 | D00DFEED | mask 8C5, value 44 |

AF is excluded in every case; OF is also excluded after multi-bit SHR. Default
+24 unread is established by the complete literal/static review, not inferred
from unchanged frame bytes. The opaque 40-byte frame has only tag +04 and size
+24 populated; all other bytes retain the case fill.

## Draft files and actual capture

`template/prepare_root.py.txt` performs stdlib-only, exclusive materialization
into the presently absent fixed Root family
`J:/PROG/battlestations-pacific-decompile/local/count42complete01`.
It checks the independently supplied draft manifest hash and exact membership,
all original selected pins, all actual selected tool/package pins, and the
fresh family path. It creates no receipt and runs no subprocess. Root supplies
a real `selection.json` with absolute paths and size/SHA256 for the compiler,
linker, Python, kernel32 import library, and original PE; the selected Capstone
package has full recursive membership/pins. The child environment is explicit.
There are no fabricated toolchain paths or executable hashes.

`template/probe.cpp.txt` is the complete fresh probe TU. A separate fresh
`leaf.cpp` is copied from the selected frozen Source TU. The Native entry is a
second literal42 in the probe; it has no allocator/provider rebinding. Raw mode
calls each leaf through a single naked capture dispatcher. Ordinary mode calls
separate noinline C++ fastcall wrappers that invoke their respective leaf using
ordinary C++ call syntax. Any emitted tail-call form is part of Root's actual
ordinary-path review. Raw DF is seeded to 1; ordinary DF is seeded to 0.
The dispatcher restores the host state before returning to C++.

Each capture contains two actual 40-byte images in PUSHAD order:
EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX, EFLAGS, ES. PUSHFD/PUSHAD collect the
processor state. Saved PUSHAD ESP is adjusted by exactly +4 to remove the
preceding PUSHFD; flags are taken from the saved word and ES from the actual
segment register. No frame or register observation is filled from expectations.
The before image is restored immediately before the indirect call. The after
image is saved immediately after its return, before CLD or copying can change
observed state. The capture helper's original host flags/registers are restored.

The fixed binary stream is little endian: a 16-byte header
`[0x32344343,1,16,192]`, followed by exactly 16 records of 192 bytes. Each record
contains eight DWORD identities (ordinal, case, path, tag, size, fill, EDX seed,
DF seed), a 40-byte actual before frame, a 40-byte actual after frame, and the
80-byte capture. Total length is exactly 3088 bytes; trailing data is rejected.
Order is four cases, each with raw Native, raw Source, ordinary Native, ordinary
Source. The writer uses only GetStdHandle, WriteFile, and ExitProcess from the
kernel32 import domain; the gate checks the entire actual import table.

The reader validates all identities, full frame snapshots, actual pre-seeds,
EAX, nonvolatiles, ESP, DF, ES, and raw ECX/EDX/defined flags. It compares the two
raw paths while retaining actual address-valued fields. Ordinary volatile
ECX/EDX/flags remain recorded; they are not incorrectly assigned the raw leaf's
preservation contract. Ordinary EAX/frame/nonvolatile/ESP/DF/ES are asserted.

## Root sequence and receipt gates

1. Review the entire sealed TEXT family. Prepare actual selection data and run
   the materializer with the selected Python, `-B -E`, the selection path, and
   the independently checked manifest SHA256. Do not materialize over an
   existing family. Review every materialized file, all selection/environment
   data, the selected cases, and both call paths. Root alone then authors
   `ROOT_TEXT_RECEIPT.json` according to `receipt_contract.json.txt`.
2. Invoke `recipe.py build` with selected Python and `-B -E` from a sanitized
   environment. It verifies the actual original PE literal, performs exactly
   one fresh compile per TU and one fresh link, then runs `inventory.py`.
   `/showIncludes` diagnostics are retained. The link has `/NODEFAULTLIB`, a
   custom probe entry, `/OPT:NOICF`, fixed Win32 base, and `/MANIFEST:EMBED`.
   The safe executable name is `count42_complete_probe.exe`.
3. Inspect `linked_inventory.json`: complete COFF sections/symbols/auxiliaries,
   weak records, relocation records, directive bytes, map text/public/static
   symbols, PE executable bytes, imports and directories, and linear inspection
   disassembly. Linear decoding is explicitly not a function-boundary oracle.
   Root determines every actual normal, cold, helper, thunk, wholeMain, capture,
   ordinary, Native, Source, and padding range. Root authors
   `coverage_plan.json` with complete actual bytes and evidence for those ranges.
4. Invoke `recipe.py gate`. `gate.py` rejects weak externals, TLS/delay/CLR
   domains, unexpected imports, unaccounted indirect transfers, broken branch
   targets or fallthrough, incomplete decoding, missing COFF public definitions,
   and overlapping or uncovered executable bytes. It accepts only the four
   named capture targets and named IAT imports. Native and Source ranges must
   each be the exact complete42. The independent stdlib
   `actual_code_reader.py` then walks PE bytes separately and verifies every
   executable byte, range, and both literals against the plan and gate result.
5. Root reviews all actual machine code and helper closure, COFF/weak/relocation
   and directive evidence, original42/Source42, opaque/default cases, and actual
   gate/inventory/reader environments. The independent reader proves byte
   coverage, not semantic correctness. Root then authors
   `ROOT_LINKED_RECEIPT.json`, pinning every prelaunch family file and the
   explicit semantic decisions in the receipt contract. Structural success
   cannot create or imply this receipt.
6. Invoke `recipe.py launch`. Only this phase may create the single target
   process, followed by the recorded reader. Its actual stdout/stderr, complete
   process environment, argv, timestamps, return code, attempt files, and reader
   entry are retained. Root reviews the recorded result and seals the whole
   fresh family, including every failure/stop if a stage failed.

Every controlled Python entry checks `sys.flags.optimize == 0` with explicit
conditional exceptions; no Python assert is used. It checks the fixed family
and Root TEXT receipt before importing helper code. Actual entry environment,
interpreter identity, flags, and argv are recorded. Entries require `-B -E` and
reject CL/_CL_/LINK/_LINK_/PYTHONOPTIMIZE/PYTHONPATH/PYTHONHOME injection variables.
The actual Capstone native DLL must be in the pinned package. These checks expose
and constrain dependencies; they do not claim a hermetic interpreter/toolchain.

The sole subprocess gateway repeats optimization, text receipt, and tool-pin
checks immediately before each dispatch. Target and recorded-reader dispatches
also recheck the linked receipt. Every dispatch has exclusive attempt/log/output
files, fixed argv, explicit cwd/environment, timeout, and no shell. A failure
consumes the attempt; it is not retried. Build/gate/launch phases have separate
exclusive entry records and selected-input after records. A timeout/exception
records a stop. A new attempt needs a new fixed family and reviewed template
revision, not edits to an accepted receipt or reuse of old objects/processes.

## Worker result and remaining qualification

The stdlib worker inspection passes 20 text/data checks: all Python templates
parse without assert statements, all JSON contracts parse, both emitted42
sequences and their hash match, scalar expectations agree, and selected original
and frozen pins remain exact. Only worker-owned writers and static inspection
utilities executed. `inspection_result.json` distinguishes algebra from absent
observations. `stops.json` records the zero-execution boundary.

No concrete blocking TEXT contradiction was identified by this worker review.
Root still owns full independent TEXT acceptance. All real toolchain selection,
fresh TU objects/map, actual helper and normal/cold closure, physical caller ABI,
ordinary emitted call paths, runtime capture records, and final admission remain
pending. Import names are a complete proposed fixture domain; live module binding
is not claimed by this text packet. There is no game/startup/gameplay validation,
Source mutation, ledger change, Ghidra mutation, or runtime replay in this packet.
