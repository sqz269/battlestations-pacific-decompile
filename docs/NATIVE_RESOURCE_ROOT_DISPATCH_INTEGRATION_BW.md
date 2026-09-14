# Resource root dispatch integration BW

Addresses: 00B7F430, 00B7E970, 00B87AA0, 00B1FE40, 00B28570,
00CC1FD0, 00CC1FD8, 00CC1FE3, 00CC2090, 00CC2098.

Five ordinary bodies implement actual root/item traversal, raw item append and
the normal behavior of the two concrete D3D9 renderer hooks. The existing raw
tree, stream, node, hierarchy, fallback and allocation domains are reused.
See `NATIVE_RESOURCE_ROOT_DISPATCH_BW.md` for contracts and limitations.

The free-unwind CC1FD8 previously stopped at its returning BF65AC call. Its two
epilogue instructions were byte-checked, its call-site override cleared and its
function membership rebuilt without deleting bytes or changing callee no-return
flags. Existing name/comments were preserved. The ten-byte root FH3 handler at
CC2098 was defined from matching installed/live bytes. All changes were serialized
with the shared Ghidra write lock and restricted to leased addresses.

The integration report records the tested revision, copied libraries, source
manifest, build logs, fixture receipts and evidence boundaries. This closes the
two dispatch bodies, not the raw manager, native parsers, B80720 loader or complete
resource ownership. It adds no production loading admission or gameplay claim.
