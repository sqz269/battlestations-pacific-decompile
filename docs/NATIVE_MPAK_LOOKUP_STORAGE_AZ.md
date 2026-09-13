# Actual MPAK lookup storage (AZ)

`NativeMpakLookupStorage` is one source provider for the existing `NativeMpakOpenLibrary` and `NativeMpakDirectorySearchLibrary` interfaces. It borrows actual storage and a returning `00BF6713` callback. It neither constructs a provider/vector nor changes any archive record.

| Root/helper | Original ABI and covered behavior | Boundary |
| --- | --- | --- |
| `00BB4140` | ECX actual file vector, stack index, EAX 24h record pointer, RET4. Read begin at vector+4, compute signed DWORD byte difference `(end-begin)/24h`, then compare index unsigned. Null/out-of-range calls `00BF6713`. After a returning callback, reload vector+4 and compute `begin+index*24h` without a second check. | The caller supplies a returning invalid-parameter callback. If it returns without repair, native also continues with invalid storage; no safe fallback is added. |
| `00BB4F40` | ECX captured first directory row, EDX captured end, stack temporary key, EAX found row/end, RET4. Advance by 14h and invoke `005EFBA0` on each row+8 member vector. First nonnegative member index wins. | The directory row's own name is never compared. Captured end is not reloaded. |
| `005EFBA0` | ECX actual member vector `{begin,count}`, stack key name header, signed index/-1, RET4. Each 8h member is `{length,pointer}`. Compare lengths, then accept empty/empty or `_stricmp(member,key)==0`; return first index. | Uses current CRT case-insensitive comparison and actual pointer/length preimages. It does not normalize, terminate, or copy strings. Other native callers of this generic helper remain outside this MPAK source integration. |

The native listings were read from the verified `battlestationspacific.exe` program. `00BB4140` body ends at `00BB417B`; `00BB4F40` at `00BB4F73`; `005EFBA0` at `005EFC15`. The direct CALL evidence in the report includes all known direct xrefs to these three roots. Ghidra was read only. The new class uses C++ virtual interfaces, so it is not a binary ABI replacement or an original exception handler.

The native MPAK runtime can borrow this same class for its open and directory-search references. `NativeMpakContainerLibrary`, provider construction, raw VFS host wiring, numeric table availability, real archive execution and game behavior remain separate work. A Win32 build checks source integration only.
