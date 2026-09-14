# Missing-resource callback dispatch (BH)

The combined raw VFS fixture initially read existing files successfully but
failed on a missing member with access violation `C0000005`, EIP and attempted
execution address `00530620`. That diagnostic was observed before the source
fix; the final probe log records the successful rerun.

Live Ghidra confirms `BDF423` reloads the current `0109CEEC` publication,
`BDF429` captures manager field `+90`, and `BDF432` calls EDX. Startup stores
`00530620` in that field. Its confirmed original body is a one-byte RET, already
reconstructed as `ignore_native_vfs_mount_failure_00530620`.

`NativeVfsRuntimeBindings::open_failure_entry` admits that exact numeric identity
and calls the existing source body. The open route retains its publication and
field reload order. A route without explicit native bindings keeps its original
callable callback contract; unknown numeric targets throw rather than execute.

The existing combined fixture now passes two missing-file reads, two full reads
and one capacity read against the installed member, then drains the actual
application singleton host. Native evidence and the exact validation revision
are in `reports/native_vfs_open_failure_bh.json` and the BH integration report.
No new callback body, original code mapping, FH3 or gameplay claim is introduced.
