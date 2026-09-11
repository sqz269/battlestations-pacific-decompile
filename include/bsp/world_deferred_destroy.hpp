#pragma once

#include <cstddef>
#include <cstdint>

// The world's vtable slot 4 drain over the entity chain header at world+4h.
//
// Addresses: 00904390, 009041A0, 00904C40, 00CE7784 (the world vtable).
// Evidence, original ABI and uncertainty: docs/WORLD_DEFERRED_DESTROY.md.
//
// Not a binary-compatible replacement. 009041A0 is a __thiscall member on the
// header object; the nodes it destroys are polymorphic entities whose layouts
// are only partly recovered.

namespace bsp {

// ---------------------------------------------------------------------------
// The 0Ch header 009037F0 allocates for world+4h.
// ---------------------------------------------------------------------------
//
// The drain settles the field order that the update walk only half showed.
// 00904BF7 established head at +0h; 009041D3 writes the same field, 009041E7
// writes +4h with the removed node's predecessor, and 009041F0 decrements +8h.
//
// This is NOT the 0Ch list object that 00484540 builds, whose fields are count
// at +0h, head at +4h and tail at +8h and whose nodes are separately allocated
// 0Ch cells. The two kinds of list are both 0Ch bytes and are easy to confuse;
// see the Corrections section of docs/WORLD_DEFERRED_DESTROY.md.

inline constexpr std::size_t kDeferredDestroyHeadOffset = 0x00;  // 009041B0
inline constexpr std::size_t kDeferredDestroyTailOffset = 0x04;  // 009041E7
inline constexpr std::size_t kDeferredDestroyCountOffset = 0x08; // 009041A6

// The intrusive links inside each node. These are kUnitOffSiblingPrev and
// kUnitOffSiblingNext of bsp/unit_instance.hpp; the constants are not
// redeclared here.
//   node+34h prev, 009041B2
//   node+38h next, 009041B9

// The world vtable, 00CE7784, has five slots. Slot 4 is the drain.
inline constexpr std::size_t kWorldDeferredDestroyVtableSlot = 0x04; // 00CE7788

// The flag the node's own slot 0 is called with. A scalar deleting destructor's
// low bit means "free the storage as well as destruct".
inline constexpr int kScalarDeletingDestructorFreeFlag = 1; // 009041F8, PUSH 1

struct DeferredDestroyChain {
    std::uint32_t head{0};  // +0h
    std::uint32_t tail{0};  // +4h
    std::int32_t count{0};  // +8h
};

// One method per native call site inside 009041A0, in call order.
struct WorldDeferredDestroyHost {
    virtual ~WorldDeferredDestroyHost() = default;
    // [node+34h]. 009041B2 and 009041DC.
    virtual std::uint32_t prev_sibling(std::uint32_t node) = 0;
    // [node+38h]. 009041B9, 009041C8, 009041D0, 009041D5.
    virtual std::uint32_t next_sibling(std::uint32_t node) = 0;
    // MOV [node+34h],value. 009041CB writes a successor's link, 009041ED clears
    // the removed node's own.
    virtual void set_prev_sibling(std::uint32_t node, std::uint32_t value) = 0;
    // MOV [node+38h],value. 009041DF and 009041EA.
    virtual void set_next_sibling(std::uint32_t node, std::uint32_t value) = 0;
    // node->vtable[0](1), the scalar deleting destructor. 009041F4..009041FA.
    // It may itself touch the chain: see the termination note below.
    virtual void destroy_node(std::uint32_t node) = 0;
};

struct WorldDeferredDestroyResult {
    std::size_t destroyed{0};      // nodes whose slot 0 was called
    std::size_t unlinked{0};       // nodes the membership test accepted
    std::size_t skipped_unlink{0}; // nodes destroyed without being unlinked
    // Not a native field. The native loop has no iteration bound: it repeats
    // while the count is nonzero and re-reads the head each time. An iteration
    // that neither decrements the count nor moves the head cannot make
    // progress, which natively can only happen if a destructor leaves the
    // header inconsistent. The reconstruction stops there and reports it
    // instead of spinning.
    bool stalled{false};
};

// 009041A0, void __thiscall(header), RET (no stack arguments),
// body 009041A0..00904203.
//
// The loop removes the head, then destroys it. The membership test before the
// unlink (009041B5..009041C2) is the part worth keeping: a node is treated as
// linked when it has a predecessor, or a successor, or the count is 1. A node
// with neither link while the count is above 1 is destroyed without any header
// write, because it is not on this chain.
WorldDeferredDestroyResult drain_entity_chain_009041a0(WorldDeferredDestroyHost& host,
                                                       DeferredDestroyChain& chain);

// 00904390, void __thiscall(world), `MOV ECX,[ECX+4]; JMP 009041A0`,
// body 00904390..00904394. The world's vtable slot 4. It does nothing but
// re-point ECX at the chain header, so the reconstruction is the drain applied
// to the header the world holds at +4h; there is no separate rule to model.

} // namespace bsp
