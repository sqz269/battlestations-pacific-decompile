# Model actual-header/raw-pool extension

The existing B75030 model constructor now has an overload receiving the actual 8-byte name header and `NativeNodeRawConstants`. It forwards that same address through the raw B6F5A0 provider and its persistent current-pool context. This removes the `NativeStringStorage::release` noexcept adapter from this constructor path and subsequent direct node cleanup.

Before native construction, the owner must be prepared, its node runtime must use raw names, `environment.actual_names` must be null, and node/model bounds must reference the same CE4970/CE4ADC cells. Keep this environment stable through destruction. Failed admission leaves the prepared owner intact. A node-construction failure uses the existing node cleanup, ends only model host tail/scene association, and leaves physical slot return to the caller. After node success, the literal original model continuation preserves volatile loads, profile publication, tail stores and signed-zero pose words.

The older NativeString interface is unchanged. This extends an already reconstructed 132-byte body and adds no native-body credit. Existing raw-node materialization, C++ exception and canonical noexcept callback limits remain; no new native exception or gameplay equivalence is inferred. The separate 43C130 prefix helper still has its inherited nonthrowing cleanup admission.

Corrected source commit `8f2c9521e5c62f314f8fed1e3a729f569d404565` passed the combined MSVC Win32 build and both existing math CTests with 2553 unchanged tracked build inputs. [Evidence](../reports/native_model_raw_name_bp.json) pins the live/original-PE body, unchanged legacy source body and provider sources. Historical model fixtures retain their original scope.

The initial focused trajectory exposed a cleanup integration error after successful construction: the two-argument node destructor always selects the semantic pool. The shared model cleanup now explicitly selects the raw three-argument overload for a raw runtime. It preserves the existing actual_names adapter precedence and semantic fallback. This affects existing B750C0/B75290 cleanup routing and adds no native-body credit. The initial passing build and failing source execution are recorded separately from the corrected attempt.


## Corrected validation

One adapted source trajectory now constructs a nonempty model name through the actual-header/raw-pool overload, deletes through the same raw pool, and verifies exact returned-buffer reuse in the same size ring. The existing actual_names adapter trajectory also passed against the corrected library. Both probes compiled with MSVC Win32 `/W4 /WX /fp:strict`; their embedded manifests were verified. These checks use the already-published actual pool and execute reconstructed source, not original model bytes. They do not inject a cleanup exception or establish slow getter/FH3 equivalence.

COFF extraction found the old NativeString constructor unchanged at 383 bytes with all 15 symbolic relocations identical. Whole objects differ because the new overload is present. Both actual model/node profile tables match the installed PE and live Ghidra bytes. All three affected original body spans were checked; their existing names remain, their thiscall signatures were recovered and read back, and their comments/exports were refreshed. The primary authored and reviewed this extension; no independent peer review is claimed. Historical model native fixtures remain scoped to their recorded interfaces.
