# Native particle record resize CY

`004DC410..004DC4DB` is the 204-byte record-array resize body. CW and the
committed orch5 sampler-owner lifetime source already recovered its normal
algorithm; CY exposes that algorithm as a separate actual-storage interface.
It adds **zero** function or byte credit. The installed PE and live
`/battlestationspacific.exe` span both hash to
`92fe6a11dfd0a9384aac242baf0cc794edd44777f5ebb0994fe9203421b0682d`.

The original receives the actual 0Ch vector header in ECX, a signed DWORD
requested count in its public stack word, and returns with RET4. CY keeps
those positions and borrows `NativeParticleRecordResizeContext` in EDX. The
context references the existing `ActualNativeStringPoolStorage` and
`SingletonLifetimeCallbacks`; it owns neither. The extra EDX argument and C++
EH frame make this a source interface, not a binary replacement.

The count read at `004DC429` is captured before DA180 reserve and remains the
bound across that call. After reserve the code freshly reads vector count.
Each growth iteration recomputes its record address from the current vector
data and captured index. A nonnull record gets only the original field stores:
name length/data zero, C3020 sentinel at +0C, alias count zero, then the five
tail DWORDs +24 through +14. The native `004DC47F` read reloads the *same
public count slot* after sentinel construction and before the tail stores.
The null-address branch skips those stores and keeps the previous bound. The
index increments before the next signed comparison.

Shrink compares the last retained bound with current vector count; each pass
uses one memory ADD to decrement current count, rereads it and current data,
then calls D45A0 on
the resulting 2Ch record. The final count store follows the loop. DWORD
offset products and pointer sums wrap as on Win32. There are no null repairs,
prefix rollbacks, owner-admission conditions or one-shot operation guards.
The source uses existing full raw DA180 reserve, C3020 sentinel allocation,
D45A0 record destruction, and the actual string-pool bridge.

The Win32 object confirms that the public fastcall adapter takes
`[EBP+8]`, pushes its address into the helper, and ends in `RET 4`. The helper
loads through that address at entry and again after C3020. Its three normal
provider call relocations name DA180, C3020 and D45A0. Source/native machine
code differs because this is a context-bearing C++ implementation; the report
pins the generated COMDAT hashes, instruction evidence and provider hashes.

Root's independent FH3 recovery identifies state 1 name cleanup and state 0
placement cleanup. On C++ construction failure CY returns the retained
current name header, then rereads current vector data and computes the
placement address before rethrowing. Native `00401130` is a RET-only target;
CY supplies no stub or array cleanup. The host C++ catch cannot claim original
FH3/SEH fault handling, cleanup-exception identity, or game validation.
