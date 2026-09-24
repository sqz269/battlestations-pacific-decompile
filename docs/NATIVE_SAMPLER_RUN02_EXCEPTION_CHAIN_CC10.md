# Run02 invalid-handle call chain

The retained run02 x86 context and stack identify a concrete `CloseHandle(00400000)` path whose caller return lies inside the copied `xlive.dll`. Matching disk bodies corroborate both exact CALL return boundaries. This narrows the obstruction in `NATIVE_SAMPLER_ORIGINAL_CAPTURE_RUN02_CC10.md`; it does not recover the sampler input or establish a successful continuation.

| Captured value | Interpretation supported by matching code |
| --- | --- |
| EIP `77879C7C` | NtClose+0C, RET4 |
| ESP `0162F420` contains `75B1E1D7`; next DWORD `00400000` | CloseHandle+57 return from imported NtClose; current argument |
| EBP `0162F434` contains `0162F548`; next DWORD `6AE1C270`; argument `00400000` | Saved xlive EBP, return at xlive+17C270 immediately after its CloseHandle CALL, same argument |
| Caller EBP-2C contains `00E2F000`; EBP-38 contains `2` | PE image-size local and VirtualProtect old-protection local |

Both exception records have identical 716-byte WOW64 contexts and 320-byte WOW64 stack windows. The 172-byte xlive body at RVA `17C1F0..17C29C` calls GetModuleHandleA(NULL), stores its result, reads that module's PE SizeOfImage, calls VirtualProtect with protection `40h`, then passes the saved module to CloseHandle. Its intervening stack-check thunk preserves EAX. The frame allocation and saved registers reproduce the captured frame locations exactly. The next xlive return is outside the captured window. Live xlive instruction bytes and IAT values were not captured; imported API identities and this body come from the hash-matching copied file.

Microsoft documents that GetModuleHandleA(NULL) returns the calling executable's module handle, and invalid/pseudo handles passed to CloseHandle raise an exception under a debugger. These contracts support the observed API path. They do not prove what resuming this particular event would return. [GetModuleHandleA](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea), [CloseHandle](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle).

The xlive VirtualProtect call is a supported explanation for initial entry protection `20h` versus `40h` observed during cleanup. The captured size matches the executable's `00E2F000` SizeOfImage. Old-protection value `2` refers to the module's first page, not its executable entry page. No API call/result or writer trace was captured, so this remains an inference.

The first native context is at ntdll KiRaiseUserExceptionDispatcher+3A, RIP `7FF9CFC64EBA` / RSP `AE610`. Its exact unwind entry describes a C8-byte allocation. Captured [RSP+C0] is `C0000008`; [RSP+C8] is `777F1F42`, the exact wow64cpu return after its syscall-thunk CALL. The dispatcher continuation reloads EAX from that saved status, restores RSP and returns. KernelBase's matching CloseHandle body tests the returned status and has a negative-status path returning BOOL zero. Actual resumed BOOL/GetLastError remains unobserved.

The second raw exception address `CFC64EBA` is MEM_FREE and is distinct from its native RIP `7FF9CF5E7401` in wow64.dll. The matching wrapper/helper uses RtlCaptureContext, lookup/unwind and NtRaiseException. A bounded wrapper unwind reaches the exact caller return at RVA `6EA1`; the next required slot lies outside the captured window. The matching caller's 32-to-64-bit record conversion zero-extends its 32-bit address field. This is consistent with the observed low 32-bit record address, but the input record and handler trace were not captured; it does not prove corrupt execution at that address.

All five nonempty live code windows match the matching disk bytes after relocation. The x86 windows differ from raw disk only in relocated immediates of neighboring syscall stubs. The primary independently decoded the raw contexts, concrete frame slots/arguments, saved native status and unwind encoding, and verified all 59 indexed artifacts. The immutable archive and review receipt are indexed in `reports/native_sampler_run02_exception_chain_cc10.json`.

This audit launched no process and changed no game, profile, helper, policy, overlay or Ghidra state. A separate benign fixture will compare the actual pre-entry API call with and without debugging before any original-specific continuation policy is considered. No future compiler frame exists in these pre-entry windows, so the sampler's source0 word remains unknown. Complete native unwind, full XSTATE, successful original startup and gameplay remain unclaimed.
