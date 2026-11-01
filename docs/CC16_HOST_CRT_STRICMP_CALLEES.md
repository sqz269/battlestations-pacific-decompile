# Current-host CRT stricmp callees

Source145 supplies one stateless `bsp::game::GameNativeHostCrtStricmpCallees`
through the existing three-method `NativeCrtStricmpCallees` interface. It is
ordinary MSVC Win32 Release `/MD` Source composition, with zero new Native or
vendor body credit. No Root consumer, selector, CMake row or shared interface
is changed by the four-file worker patch.

| Existing service | Actual host endpoint | Admitted Source arguments |
| --- | --- | --- |
| `errno_cell_00bffb8b` | `::_errno()` on every invocation | No arguments |
| `invalid_parameter_00bf66ef` | returning `::_invalid_parameter_noinfo()` | Five DWORD zeros only |
| `compare_locale_00bf7eec` | `::_stricmp_l(left,right,nullptr)` | Null opaque third argument only |

The endpoint declarations come from the real SDK headers. Win32/type assertions
check pointer/DWORD widths, `int32_t == int` and all three declared function
pointer types. Compilation rejects a different architecture, static CRT or
debug CRT domain. No debug-private `_invalid_parameter` declaration is invented.

The errno result is returned directly with volatile qualification. The adapter
does not create a DWORD, cache a pointer or use the constructor thread's state.
The SDK reference UCRT implementation obtains the current thread's PTD, but has
its own `errno_no_memory` fallback if PTD allocation fails. Forwarding that real
API result does not promise an always-distinct TLS cell or Original CRT identity.

The five-zero method calls the returning noinfo entry after its Source-domain
check. It installs no handler and catches nothing. The actual current host CRT
thread/global/default policy may return, throw or terminate. The noreturn
wrapper would additionally force termination after a returning handler and is
not used. Nonzero words reject before the endpoint call; this is not a recovered
policy for other Original private arguments.

The locale method rejects a non-null opaque locale before its endpoint call;
it never reinterprets Original private locale storage as host `_locale_t`.
Both string pointers pass unchanged, including null strings. Host CRT validation,
errno, handler and current null-locale policy remain real library behavior.
There is no fixed locale, ASCII fallback, locale allocation or synchronization.

Canonical V3 game `0109DE1C` is distinct from the UCRT private locale-change
state. This adapter reads or resets neither cell. Root must supply the existing
strict canonical V3 live reference to the actual Source144/131/135 contexts;
there is no V1/V2 fallback, copied zero or new TLS/process bank. Root owns one
provider before those borrowers and retains it through actual Page/Runtime/
callback/worker use and shared drain, or the existing whole-Root retention path.
All endpoint calls execute on their reached caller thread.

The frozen plan145 manifest is
`c1590e8a238b14d505c6c0ef0b9bd7be7859a1005cc7846c0c4e5ca3203fb220`,
seal `98ab1d3bf7197bd3773ef44b228080d7dfeabe6cccfb125af75a199cc16b5746`.
Its finite SDK declaration/reference-source, x86 import-object, installed UCRT
export and existing Root import windows are the API evidence. The separately
pinned SDK reference source is not asserted byte-identical to the installed DLL.
`_errno` and noinfo use the runtime API-set contract; `_stricmp_l` uses the string
contract. No numeric export RVA is called or treated as callable authority.

Validation is recorded in the report and immutable worker receipt. First strict
compilation is object-only while the new TU is unregistered. Root separately
owns the later worker-only `bsp_game` registration and official build. A direct
object import proves only an object-bound dependency. If dead stripping removes
the unbound `_stricmp_l` endpoint from the EXE, actual production retention/link
and CRT resolution remain open until a genuine consumer is bound; no dummy
consumer or retention anchor is added.

The strict MSVC x86 `/MD /W4 /WX /fp:strict` object compile passed in 3.3404476s
while the TU was still unregistered. Root then supplied the separate worker-only
CMake row (postimage 54856beb455a10e35129b562d0491b801c9701bc07d1b226a5ee977e96dd5cb4).
The official registered Win32 build passed in 124.739588s with the two existing
checks passing 0.24s/13.66s. The actual `bsp_game` object is 014c and has all three
real import-cell relocations. The unused provider methods have no map definitions;
the EXE retains its existing `_errno`/noinfo imports but does not import
`_stricmp_l`. Thus production retention/link and CRT resolution remain open
until Root binds a genuine consumer. These checks do not execute the provider.

Original static-CRT private ABI, TLS/errno/handler/locale ownership, Origin,
fault/SEH/unmasked-FP and runtime equivalence remain unclaimed. All nine GUI gates
remain OFF. No new test, probe, Game run, Ghidra change, handler policy or cleanup
framework is introduced.
