# WinMain COM and Game Explorer binding

Address: enclosing `008F81F0`; bounded `008F81F8..008F82CC` and denied
exit `008F82F0`. Evidence: existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and installed Windows SDK 10.0.26100.0 headers.

The earlier host skipped Game Explorer, returned access unconditionally,
and used different COM arguments and process termination. The actual binding is:

| Native site | Recovered behavior |
| --- | --- |
| `008F81F8` | Push `8`: `COINIT_MULTITHREADED(0) | COINIT_SPEED_OVER_MEMORY(8)`. SDK apartment-threaded is `2`; disable-OLE1DDE is `4`. |
| `008F820A..008F821C` | `CoInitializeSecurity(NULL,-1,NULL,NULL,0,3,NULL,0,NULL)`: default authentication and impersonate. The earlier NONE symbol is `1`, not native `0`. |
| `008F823D..008F824D` | Zero the actual interface output; `CoCreateInstance` with context `17h`; branch on signed HRESULT. |
| `008F82AA..008F82B1` | `IGameExplorer::VerifyAccess`; ignore HRESULT and test the actual post-call BOOL, with no preceding initialization of that stack DWORD. |
| `008F82BC..008F82CA` | Independently test the output pointer and call virtual `Release` if nonnull, including after a negative creation HRESULT. No pointer clearing. |
| `008F82CC` | `CoUninitialize` also runs after either initialization failure. |
| `008F82F0` | Full-cleanup CRT exit, before interface release and COM teardown. |

The SDK UUIDs match the live constants at `D16A94` and `D16844`:
class `{9a5ea990-3034-4d6f-9128-01f3c61022bc}` and interface
`{e7b2fb72-d728-49b3-a5f2-18ebf5f1349e}`. The host uses `__uuidof` on
the SDK declarations and calls only `VerifyAccess`, not title registration.
Native code supplies an ordinary zero-extended wide C string to the BSTR-typed
argument; there is no `SysAllocString` conversion here.

The actual interface output is retained for one startup sequence. Release stays
explicit; destruction adds no cleanup to the denied exit. The BOOL is supplied
without initialization, then read through an explicit machine load after the
API. This preserves actual current-process output/remainder bytes without a
C++ read of an indeterminate scalar. It does not claim those stack bytes equal
an original game trace. API failure is not replaced by a permitted result.

The saved library label at `BFBDBB` is `_exit` and remains unchanged. Assembly
determines the binding: this wrapper calls `BFBCD9(code,0,0)`. The zero second
argument takes `BFBD10..BFBD59`, decoding and invoking the on-exit table in
reverse order, followed by termination arrays. The zero third argument reaches
process termination. The corresponding current CRT operation is `std::exit`,
not `ExitProcess` or the current CRT's no-cleanup `_exit`. No CRT is ported.

The strict Win32 build and both existing CTests passed. One ignored fixture
uses the actual process-host implementation:

- An existing MTA followed by host initialization returns `S_FALSE(1)`;
  the previous apartment-threaded call would return `RPC_E_CHANGED_MODE`.
- Actual COM security returns `S_OK`, Game Explorer creation succeeds, and
  `VerifyAccess` returns BOOL `1` for the installed game binary. Explicit
  interface release precedes COM teardown.
- A separate process registers an `atexit` callback and invokes host exit.
  Exit code is zero and the callback marker is printed.

The fixture does not enter the original game or reconstructed startup loop.
No permanent tests were added. This proves the observed process bindings,
not original ABI compatibility or gameplay. Directory failure/overflow,
native pooled wide-string ownership, language acquisition and later startup
owners remain separately bounded. Commands and observations are in
`reports/startup_com_binding.json` and ignored `local/startup-com-l-*` files.
