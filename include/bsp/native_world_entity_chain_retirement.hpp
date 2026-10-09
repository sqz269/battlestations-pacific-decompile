#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

// Complete 009041A0 machine-level refinement of the existing C++ interfaces.
// ECX is the actual retained 12-byte {first,last,count} chain header. The full
// count word gates the loop; the unlinked-node membership comparison is signed.
// Current links and header fields retain their native read/write ordering.
// Current entity slot0 receives DWORD1 by CALL and must consume four stack bytes
// and preserve ESI/EDI. The header must survive every callback and each reached
// entity/link/table must be valid through its accesses. Coherent hierarchy and
// callback changes must provide progress. No owner, table, free domain, fault/EH,
// binary-entry, production binding or gameplay compatibility is supplied here.
void __fastcall retire_native_world_entity_chain_009041a0(void* actual_header);

} // namespace bsp

#endif
