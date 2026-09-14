# MPKG factory deletion on the canonical raw manager

The MPKG factory getter registers its secondary pointer at allocation+4 with
the raw singleton manager. Its identity is `CFEA10`, whose slot zero contains
`00735D00`. The shared source deletion dispatcher previously lacked this
profile, preventing a fully registered factory graph from draining.

`NativeSingletonDeletionBindings::mpkg_factory` now retains the existing
`NativeMpkgFactoryContext`. The dispatcher passes the popped secondary owner
to `delete_native_mpkg_factory_secondary_00735d00`. That existing source thunk
subtracts four and invokes the existing primary deleter `00737090`, which
clears the same `010904F4` publication, writes the base identities and optionally
frees the primary allocation. A changed publication is cleared without being
used as the deletion owner. Missing context remains a source contract failure.

Primary review verified complete disk/live envelopes for `00736A90` (206 bytes),
`00737090` (58 bytes) and `00735D00` (8 bytes), every saved instruction owner,
the secondary table word and the adjustment/tail jump. Existing body code is
unchanged. Names, native ABI annotations and evidence comments are saved with
prior comments retained and exports refreshed.

The ignored raw-owner fixture now registers six owners on one actual source
manager. It checks that MPKG registered primary+4, changes the factory
publication to a separate sentinel, then drains through the real shared
dispatcher. All six publications clear, the manager vector empties, and the
replacement sentinel remains unchanged. This validates the source binding;
original ABI/exception identity, production startup and archive/game behavior
remain unproven. Exact combined hashes are recorded in
`reports/vfs_storage_owner_bb_validation.json`.
