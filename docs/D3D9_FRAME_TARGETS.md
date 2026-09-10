# Retained frame targets

`D3D9StateCache::bind_frame_targets_00b24e70` reconstructs the concrete renderer
virtual `+98`. It enters the existing optional guard before comparing identities,
publishes and retains the requested target group before releasing the previous
group, and returns without GPU changes when the requested group is null.

A nonnull group supplies four color-surface wrapper identities at `+08..14`,
depth at `+18`, and the exact sRGB-write byte at `+3C`. Application global
`00F8D398` decides whether to issue render state194; it is not a device-capability
inference. The routine unbinds color slots1..3, binds colors0..3, then binds depth.
It continues these calls after a COM failure, matching the native sequence.

`00B23D80` binds an explicit color wrapper, or the retained default color when
slot0 is null. Null higher slots unbind. Only an explicit nonnull wrapper
increments renderer `+1BA0`, even when its COM pointer is null or its call fails.
Depth binding reuses `00B21690` and its separate counter. No surface-wrapper
identity cache or implicit wrapper retention is added to either leaf.

The new HRESULT result reports the first **surface-binding** failure. The
existing void render-state setter intentionally drops its HRESULT, as native
does. `S_FALSE` indicates target-group identity skip. These diagnostics are a
new interface, not the native function's return contract.

Original ABIs: target binding is ECX renderer, one stack pointer, RET4; color
binding is ECX renderer, stack slot/wrapper, RET8. Getters `00B1F6D0`, `00B1F6E0`
and `00B1F710` are direct borrowed field reads. The entire concrete renderer
command hook `00B20210` is RET4 and does not inspect its argument. The two missing
functions `00B24E70` and `00B20210` were defined in the existing saved bsp program;
six full ranges matched the unchanged installed PE and live program bytes.

The installed-model probe uses actual D3D9 surfaces and checks default fallback,
all color/depth identities, counters, identity skip, null-group preservation,
and sRGB state. It captures and restores all four prior render targets, depth,
and the device state block. The independent synthetic COM review additionally
checked publication-before-release and continued surface calls after failure;
that fixture is not real GPU failure injection.

Surface wrappers use retained C++ owners; native intrusive object ABI, complete
renderer frame lifecycle and game rendering remain separate. The raw linear
readback is not original-game or final-display visual parity. See
`reports/d3d9_frame_targets_audit.json` and the integration validation report.
