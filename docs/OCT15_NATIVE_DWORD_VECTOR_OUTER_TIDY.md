# Actual outer DWORD-vector storage tidies

| Native entry | Preserved library identity | Coverage | Source interface |
| --- | --- | --- | --- |
| `004DC1C0` | `STL_Vector_TidyDestroy_004dc1c0` | Complete, 61 bytes / 21 instructions | `tidy_native_dword_vector_outer_storage_004dc1c0` |
| `004DCA30` | `STL_Vector_TidyDestroy_004dca30` | Complete, 61 bytes / 21 instructions | `tidy_native_dword_vector_outer_storage_004dca30` |

Both new interfaces are void fastcall functions with the actual header in ECX,
an explicit ignored EDX slot, no stacked inputs and plain RET. The physical shape
does not recover an original C++ owner class, mangled declaration or binary ABI.
The descriptive Source names are hypotheses; correct library names and earlier
input-configuration, typed and AV projections remain unchanged.

Capture DWORD[header+4] as initial begin. Only if nonzero, capture current end at
+8 and destroy that captured range of 16-byte inner headers through the accepted
actual raw range service. Then reload CURRENT outer begin from +4 and invoke the
actual current-CRT free unconditionally inside that initial-nonnull branch,
including if the reloaded pointer is null. Freeing the saved initial begin or
conditionally skipping a current-null free would change the native schedule.

Both paths then clear DWORD+4, +8 and +0Ch in order. There is no direct opaque+0
access, null-header/capacity guard, preclear, element model, temporary owner,
snapshot, rollback or header hook. The inner range service frees inner backings;
the later free releases CURRENT outer backing. The supplied header itself is
not directly deallocated by these routines.

Native entry PUSH ECX initializes the private saved word with the original
receiver bits. Its later [ESP+4] load supplies one dead child word; PUSH ESI
supplies the other. Both metadata words therefore contain known receiver bits.
Source passes those same defined bits without fabricating a native frame helper.
Unused Source EDX is canonicalized only in the second entry's delegation.

The complete native bodies differ solely within independently decoded call
operands; all other 61 bytes and the 21-instruction schedules agree. The second
Source entry composes the first based on that full equivalence proof. The actual
normal transfers are:

```text
004DC1D4 CALL 004D49B0
004DC1DD CALL 00BF65AC
004DCA44 CALL 004D49B0
004DCA4D CALL 00BF65AC
```

Separate read-only caller support, excluded from Source ownership:

```text
00C7EC91 JMP 004DCA30
```

That action obtains the receiver from [EBP-10h]+4D0h. Its physical E9 jump has an
existing CALL_RETURN flow override: typed default flow is UNCONDITIONAL_JUMP,
effective flow CALL_TERMINATOR, both target the same tidy. No listing repair,
action implementation, public EBP interface or parent/EH readiness is claimed.
The two ordinary tidies contain no local EH setup, catches or actions.

Fresh typed metadata verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 122 owned PE/live bytes and 42 current starts
agree with complete offline Capstone decoding. The immutable readiness receipt
keeps the 14-byte / 3-start caller support separate. Raw header fields and the
accepted range service avoid borrowing a host vector, callback or count bridge.

The destroy-range predecessor `c563c8ef4577557dfe97cf4cec4bbffbb92a93dc` was
primary-accepted but absent from the accepted Main snapshot used at sync.
Worker merge `2b84fe1186c1656108d1eb5aa2180ba522e54a0a` retained that pin and
merged only accepted Main `2bbcd20d18c8c48c73f9615cb0a0b3cd718c58d7`.
No unaccepted Root staging was merged or Main publication performed. Source and
header fingerprints of the predecessor remain unchanged. Later Main/ref
observations are recorded separately in the JSON report.

Per `docs/COORDINATION.md:93-96`, the worker appended only its local deferred
source line without leasing or queuing the shared append-only CMake registry.
The required default `./scripts/build.ps1` passed all three existing Win32 suites
in 12.35 seconds. No new test was added; these suites do not specifically test
the new raw interfaces.

The actual CMake object contains two identical 55-byte / 18-instruction bodies,
110 bytes / 36 instructions total. Ten focused checks verify fastcall/plain RET,
conditional end/range work, both receiver metadata words, the fresh current-begin
reload, unconditional outer free and ordered clears. Exact i386 relocations name
the accepted raw destroy and current-free services; the second entry is inlined.
No local C++ EH metadata, allocator, pool, callback, guard or extra cleanup appears.

The header must remain valid and writable through all work. Captured nonempty
ranges and every backing passed to the actual services must meet their admitted
raw-storage/current-CRT contracts. Progressive alias effects and distinct
captures remain visible. Initial-null native EAX retains ambient input bits;
otherwise final void-free residue remains. Source void adds no pointer result.
Original CRT/heap, private frames/registers, FH3/SEH/fault behavior, mangling,
drop-in ABI and game/runtime equivalence remain unproved. All five transfer rows
and five Markdown claims pass verification; final staged diff checks are also
required. Primary frozen-pin review remains pending; no push or Ghidra write.
