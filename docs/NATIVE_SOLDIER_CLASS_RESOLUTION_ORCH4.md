# SoldierClass resolution prerequisites

Addresses: 00443E20, 00489E60, 00489E80, 004AF520, 004AF950, 004AFED0,
004B11A0, 004B12A0. Audited consumer: 004B1400.

## Result and evidence boundary

Eight complete bodies (544 original bytes) now have MSVC Win32 source in
`src/native_soldier_class_resolution.cpp`. These are constructors, base cleanup,
blank-node allocation and a constant virtual predicate. The factory, registry
insertion/teardown and Lua reader remain source-absent. Their actual table
identities are borrowed inputs; supplying a table does not implement its targets.

Evidence uses the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete body bytes matched the installed image;
hashes, original ABIs and direct call sites are in
`reports/native_soldier_class_resolution_orch4.json`. Descriptive names are
hypotheses, not recovered symbols. Existing library names are preserved.

| Entry | Complete source behavior |
| --- | --- |
| 00443E20 / 004AFED0 | Allocate 1Ch; independently guard and clear links 0/4/8, set color18=1 and nil19=0; leave other bytes untouched. |
| 00489E60 | Write base table CE660C; clear name+4/+8, byte+C and two distinct handle words+10/+14. Leave padding and +18 onward untouched. |
| 00489E80 | Write base table; release current handle+10, then current handle+14, then destroy current name+4. Preserve remaining cleanup on C++ exceptions. |
| 004AF950 | Clear current E187F0 publication, then stamp singleton-base CE3818. No free or node destruction. |
| 004B11A0 | Write registry CE7160, allocate sentinel, publish head+8, reload it for each self-link, clear count+C. Preserve opaque+4. |
| 004B12A0 | Construct real base, write SoldierClass CE7150, allocate sentinel at+2C, clear count+30. Preserve +18..2B. |
| 004AF520 | `MOV AL,1; RET`: ignores receiver and has no ownership side effect. |

The warning-owner allocator now calls the complete 00443E20 provider; its older
fragment record remains valid as a wrapper. Both blank-node providers use the
existing real allocation boundary. Base cleanup uses the established 0041DE40
intrusive-handle provider and 0041DD20 raw string-pool provider, including member
changes made by callbacks. No host map, fake registry, or placeholder reader was
introduced.

## Exception state evidence

00489E80 uses handler C629D6, FuncInfo D8A508 and unwind map D8A4F8. State1->0
releases member+14 through C629CB; state0->-1 destroys name+4 through C629C0.
The ordinary path sets state0 after the first handle and state-1 before name
cleanup. If a true unwind action throws a second C++ exception, source terminates.

004B11A0 uses C649F8 / D8CDFC / D8CDF4: state0 invokes C649F0 -> 004AF950.
Thus failed allocation clears the current publication even before this constructor
has published its new instance; a new-handler can have changed that publication.

004B12A0 uses C64A38 / D8CE54 / D8CE4C: state0 invokes C64A30 -> 00489E80.
Constructor failure observes current base members, including new-handler changes.
Second exceptions during this unwind terminate. These C++ translations do not
implement original FH3, SEH, hardware-fault unwinding or an original ABI bridge.

## Factory audit: remaining source closure

The complete 312-byte 004B1400 body takes NativeString* in ECX, returns the class
in EAX and uses RET. It gets registry 004B1330 and mapped cell 004B0CB0; a cached
class skips construction. On a miss it obtains globals from current E188A8+1A0C,
selects SoldierClass and the named row, allocates 34h, calls 004B12A0, copies the
name through real 0041DD40, then invokes virtual slot+4 at 004B14DC. Its target
in CE7150 is 004B0DE0. The factory reacquires the registry and cell, publishes the
class, destroys the row/table/globals in order, then invokes slot+Ch at 004B1523
on both hit and miss paths. CE715C points to the constant predicate 004AF520;
the factory ignores its return. The earlier damageable-reader audit intentionally
left this slot's ownership meaning unresolved; this body now resolves it.

Allocation cleanup state3 is disarmed before name copy and virtual reading.
Do not invent deletion of the class if those later operations throw. Handler
C64A93 / FuncInfo D8CECC / map D8CEAC owns the three Lua temporaries and the
earlier raw-allocation cleanup. The actual reader 004B0DE0 calls unresolved
0048F670 and uses the LoopLengths string/float tree. Existing host-storage tree
interfaces need a verified raw-pool adaptation before claiming that closure.

Ready follow-up closures, all source-absent here:

- Registry lookup/insertion: 004AFE50, 004B0CB0, 004B0A70, 004B0660, 004B0880,
  004AF530, 004AFB50, 004AFBE0 and their genuine allocation/compare providers.
- Registry lifetime: 004B1330, 004B1210, 004B1280 and 004B09A0 erase-range.
- Class lifetime/reader: 004B1120, 004B1310, 004B0DE0, 0048F670; verify existing
  00444B10 cleanup and 00444BE0 insertion against actual string-pool requirements.
- 004AF890 creates instances and is not called by the factory; it remains a
  separate dependency, not an invented factory step.

## Validation

Strict `scripts/build.ps1` and all three existing parent CTests passed.
A focused ignored Win32 probe compared copied original constructor instructions
with source, including sentinel bytes and untouched holes. It also compared
base cleanup while a real virtual callback replaced the second handle, confirming
events 1 then 3. Allocator/raw-pool boundaries were explicit fixture substitutions;
the genuine source 0041DE40 provider was compiled into the probe.

Source-only failure checks cover allocator/new-handler member changes, registry
unpublication, remaining base cleanup and propagation of the first C++ exception.
Original FH3 was not executed. The original base-cleanup comparison used an empty
name, so raw string-pool behavior is established by its existing provider, not
that comparison. Independent source review found no concrete field/order defect.
No binary replacement, game startup or gameplay validation is claimed.
