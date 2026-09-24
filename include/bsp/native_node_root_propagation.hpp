#pragma once

namespace bsp {
class NativeNodeSceneChildDispatch;

// Complete B6D890[164], native ECX node, requested outer24h scene word, RET4.
// The request is captured before any node read. Store its actual identity at
// node+A4; outer+0C contains actual node links. No host root/scene companion.
// Root path captures current A4 and profile/slot50 address BEFORE B72110,
// then reads the slot's current target. Parent path captures parent first,
// then current node profile, parent170 and current50, in that order.
// Direct child recursion preserves the original request and rereads current
// child3C after each return. Equal A4 with nonnull parent returns immediately.
void propagate_native_node_root_storage_00b6d890(void* actual_node,
    void* captured_requested_outer24, NativeNodeSceneChildDispatch&);

// Dispatch uses the SAME pure actual-profile resolver and genuine target
// binding as raw node/light scene attachment. Here the target node may be self
// and recursion is0. Every reached binding supplies its own initialized,
// persistent attachment frame/Acquired and the same actual owner/import domain.
// No missing-target fallback, synthesized frame preimages, admission, retain
// or destructor installation. Callback-modified reached storage must remain
// valid. Attachment failure preserves prior root/list writes and callee
// diagnostics. No native private-stack/FH3/ABI or full terminal graph claim.
} // namespace bsp
