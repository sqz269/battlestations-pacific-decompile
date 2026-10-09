# Count42 complete05 actual-code peer review

The selected `count42complete05` executable passes this independent static
review. Source credit remains **0**. This is neither a Root linked-code receipt
nor permission to execute the target, and establishes no runtime, general ABI,
owning-lifetime, or game acceptance.

## Exact scope and evidence

The peer froze 38 explicitly selected Root files before analysis: the actual
3072-byte executable, both objects, map and recorded inventory, Source/probe/header,
selected Native42 bytes, passive helper text, selection/materialization/TEXT
metadata, and compile/link/inventory entry, attempt, process and log records.
Root retained ownership of its entire mutable family and could append later
evidence. Only the selected subset was guarded here.

A fresh dependency freeze added 1746 files, including all 95 Capstone package
files and seven distribution metadata files, current interpreter/stdlib/native
companions, and site-startup files. The peer decoder ran with `-I -B -E -S`, an
empty bytecode lookup prefix, absent `LIBCAPSTONE_PATH`, and an explicit loader
for the pinned package and packaged DLL. Pre-interpreter guards checked original
and frozen files; imported modules were checked against the fresh pins. No Root
helper was imported or executed. The peer's own metadata/arithmetic checks used
guarded standard-library code and PowerShell.

Final content and membership checks passed for all **1784 original/frozen pairs
across eight scopes**, including the exact Root subset; the private process
environment and empty bytecode prefix were unchanged. The ignored local family
contains **1815 files**, including its seal, with 1814 manifest entries. Its seal
SHA-256 is `24a13f1b1d0922e4064c1634e18d0ec374afa7be0dc69813065599ac521d8c63`.
The [report](../reports/cc12_count42_complete05_actual_code_peer.json) identifies
the exact local family and evidence artifacts.

## Actual bytes and control flow

Independent PE/COFF parsing and relocation application reproduce both retained
object code sections byte for byte in the linked image. All 33 primary symbols
(43 records including auxiliaries), ten section-definition auxiliary records,
34 executable relocations, and three noncode `.debug$F` DIR32NB relocations are
accounted for. There are no weak or COMDAT records. Both `.drectve` sections are
exactly three spaces. The logical 3148-byte COFF BSS has zero stored bytes and
maps to the zero-file PE data section.

All ten actual functions were reviewed: Source42, Native42, both ordinary
wrappers, capture, Main, entry, and the three import thunks. The executable
section's 1024 raw bytes comprise **771 function bytes and 253 padding bytes**;
240 reachable instructions and all 35 control edges are classified. The 44
declared padding bytes are separate from 209 zero alignment bytes. There is no
unexplained executable byte, cold body, helper, or control operand. Entry's
trailing `INT3` is unreachable after `ExitProcess`; noncode FPO metadata includes
that byte in its length of 13, while the entry instructions total 12 bytes.

The image imports only `KERNEL32!GetStdHandle`, `WriteFile`, and `ExitProcess`.
The single embedded manifest requests `asInvoker` with `uiAccess=false`. TLS,
delay import, CLR, load-config, relocation, export, and exception directories are
absent. Independent section/map/import results were compared with Root's recorded
inventory only after derivation and agree. The four recorded build processes
returned zero; log hashes and exact selected child environments agree. These
processes were not rerun by the peer.

## Instruction semantics

Both actual 42-byte leaves equal the selected Native42 file, SHA-256
`e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7`.
They read tag DWORD `[ECX+4]`; tags 9/10 return unsigned `[ECX+0x24] >> 2`;
tag 11 returns unsigned division by 12 via high `MUL AAAAAAAB` then `SHR 3`.
Defaults return zero without reading `[ECX+0x24]`. There are no leaf stores,
calls, allocations or provider paths. ECX, nonvolatile registers, ES and DF are
preserved; EDX changes only on tag 11 to the high product. Default defined flags
use mask `8C5` and result `44`; SHR uses mask `C5`. AF is undefined in both
cases, and OF is undefined for these multi-bit shifts.

Both ordinary wrappers are single five-byte tail jumps. The actual capture
saves host GPRs/flags, seeds registers and mode DF, and records two 40-byte images
in order `EDI ESI EBP ESP EBX EDX ECX EAX EFLAGS ES`. For capture entry stack R,
Q=R-36 and target entry T=Q-4. The saved PUSHAD ESP is corrected by four in each
image. Seeded EBP `EB900003` is not a stack frame. Pre-capture POPAD/POPFD restores
the target seeds before its call; final host restoration returns the conforming
Win32 fixture's DF=0 before Main/output. Ordinary volatile behavior is asserted
only for these actual wrappers.

Main constructs exactly four cases by four modes, with 192-byte records, 40-byte
pre/post frame images, and 80-byte captures. Mode targets and DF seeds are correct.
The selected EAX results are `1`, `3FFFFFFF`, `15555555`, and `0`; their defined
masked flags are `01`, `04`, `04`, and `44`. Tag 11 produces EDX `AAAAAAAA`; the
other cases preserve `D00DFEED`. Main writes exactly 3088 bytes and returns zero
only for a successful full write, otherwise two; entry forwards this to
`ExitProcess`.

## Remaining boundary

There is no static blocker in the reviewed subset. Root must independently own
Native provenance, current tool/header provenance, linked acceptance and any
execution authorization. This peer did not read the original game PE, query
Ghidra or Native providers, replay prior attempts, issue receipts, or execute
target code. The explicit pinned DLL path is decoder-loader evidence, not OS
mapped-module enumeration. Readable input memory, ordinary Win32 flat DS/ES and
host calling conventions remain fixture assumptions until separately exercised.
No C++ changed; no build or new test was required for this metadata-only packet.
