# Native movie filename preparation API

Packet codex-movie-native-filename-provider exposes exact 00BDD600 through the
existing GameNativeVfsRuntime and GameVfsHost. It is an unattached provider API:
no decoder, clock, widget or subtitle guard is changed.

The fresh tree followed fetched main 1e10d9de40bda6596ebdb89487a657cdba034d8d, which had
advanced from requested 5c2547156 through Font lifecycle publication. All four
VFS source paths are identical between those baselines before this packet.
The locale c30 tree remains clean and untouched.

## Exact operation and output

probe_and_replace_file_name(std::string&) calls existing
replace_native_vfs_file_name_00bdd600 with current actual_manager() and the
same retained Impl::lookup/D683E8 profile. Its header comes from the exact
lookup.physical.strings pool used by the kernel.

The new private length-aware PooledHeader constructor resizes a genuine
NativeString in that pool and copies the complete std::string byte span.
It preserves explicit length, including embedded NUL, instead of truncating
through a C-string scan. The existing C-string constructor is unchanged.
There is no fabricated header over std::string memory or alternate pool.

BDD600 normalizes the actual caller before constructing its name-probe visitor.
Success replaces it with the visitor's current name. Ordinary FALSE leaves
the normalized caller. Copy-out therefore runs before returning found on
both normal paths; it is not guarded by found. Only a genuine zero-length
native result clears the std::string.

This does not call BDD6E0 direct_resolve, BDF4C0 candidate resolution, alternate
extensions, filesystem substitutions or a working-directory fallback.
Existing exists/direct_resolve/resolve_existing/open methods are unchanged.
The report records eleven unchanged existing body hashes, including the
C-string constructor and the host's active-runtime/failure-latch functions.

## Ownership and failure custody

The host rejects unready core before native entry; runtime also checks core
registration. An existing retained name_resolution invocation is rejected,
never cleared or replaced by this API.

BDD600's existing kernel owns its D683E8 visitor and its normal/C++-exception
cleanup. The caller PooledHeader stays alive through native execution and
normal output copying, then releases through the same pool. Failed input
construction also releases its owned buffer. No BDD6E0 acquired frame or fake
completion state is synthesized for this different operation.

The host call uses its existing invoke_native boundary: propagated operation
errors set native_operation_failed_ and rethrow. Subsequent active_runtime
access is blocked and requires_process_retention retains its existing meaning.
No provider error is converted to FALSE, an empty path or parse-ready success.
Copy-out is only after normal native return. Existing interrupted-operation
and process-retention policies remain authoritative.

The API is a typed std::string adaptation, not an original register/SEH ABI
replacement. The native operation receives manager in ECX and mutable 8h string
header on the stack, returns its Boolean in AL and RET4. Assembly confirms
normalization at 00BDD623 before D683E8 setup at 00BDD62A; the false branch
destroys its visitor at 00BDD6B8 and returns AL0/RET4 at 00BDD6D0. Native Source,
the saved bsp.gpr and /battlestationspacific.exe were verified with bounded
wrappers (77631 live/snapshot functions). The existing kernel was not edited.

## Validation and remaining scope

The standard scripts/build.ps1 MSVC Win32 Release four-node build passed;
reconstructed_math and tool_tests both passed (2/2, 13.21 seconds).
No source was edited after build launch. Four source hashes, existing-body
checks, build/CTest logs and executable/DLL hashes are in
reports/cc16_movie_filename_preparation_binding.json.

The precise FALSE-output risk is mistakenly copying back only when found.
Source review confirms unconditional normal copy-out, so no new fixture or
test framework was added. No game or runtime probe was run. The new API's
runtime TRUE/FALSE/exception paths remain unexercised by this packet.
Earlier VFS CLI/read evidence and frozen 409 gameplay gates are not proof of
this new operation.

The API remains unattached until the real decoder owner/factory/dialog,
renderer lifetime admission, widgets and clock are ready. No fake producer,
default Boolean/path, shared metadata/ledger/CMake edit, Ghidra mutation or original
install/save/profile write was introduced. No new native-body, ABI, activation
or playback credit is added.
