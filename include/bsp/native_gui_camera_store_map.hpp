#pragma once
#include "bsp/native_effect_handle_acquisition.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Same physical raw tree pair. key_00 stores FLOAT BITS, never a signed key
// comparison in this family. Links/allocator/iterator/node engine are shared.
using NativeGuiCameraStorePair = NativeIntPointerTree18Pair;
using NativeGuiCameraStoreInsertResult = NativeIntPointerTree18InsertResult;

struct NativeGuiCameraStoreFindScratch {
    void* tree_00;
    NativeIntPointerTree18Iterator iterator_04;
};
struct NativeGuiCameraStoreInsertFrame {
    NativeIntPointerTree18Iterator local_00;
    std::uint32_t unclaimed_return_gap_08;
    NativeGuiCameraStoreInsertResult* volatile output_argument_0c;
    // Initially the actual pair address. Nonempty search overwrites this SAME
    // argument word with raw key bits; the captured pair address survives.
    volatile std::uint32_t pair_argument_10;
};
struct NativeGuiCameraStoreCreateFrame {
    // This nested frame is live through AA4960's return. Later native name/
    // log calls reuse 04..13 for their private return/argument words; source
    // retains the insertion frontier here instead. Post-CREATE byte parity
    // covers outer pair/result1C..2F and caller arguments34..3F only.
    NativeGuiCameraStoreInsertFrame insert_00;
    std::byte unclaimed_saved_register_gap_14[8];
    NativeGuiCameraStorePair pair_1c;
    NativeGuiCameraStoreInsertResult result_24;
    std::uint32_t unclaimed_return_gap_30;
    void* volatile camera_argument_34;
    void* volatile scene_argument_38;
    const void* volatile descriptor_argument_3c;
};
static_assert(sizeof(NativeGuiCameraStoreFindScratch) == 12);
static_assert(sizeof(NativeGuiCameraStoreInsertFrame) == 0x14);
static_assert(sizeof(NativeGuiCameraStoreCreateFrame) == 0x40);
static_assert(offsetof(NativeGuiCameraStoreCreateFrame, pair_1c) == 0x1c);
static_assert(offsetof(NativeGuiCameraStoreCreateFrame, result_24) == 0x24);
static_assert(offsetof(NativeGuiCameraStoreCreateFrame, camera_argument_34) == 0x34);

struct NativeGuiCameraStoreMapBindings {
    const volatile std::uint32_t& actual_near_00d7a2f0;
    const volatile std::uint32_t& actual_far_00ce3804;
    const volatile std::uint32_t& actual_one_00d7a24c;
    const char* actual_empty_00f8bc60;
};
// Fresh caller-owned diagnostics per call, disjoint from native storage/scratch.
// No destructor, retain, registry admission or rollback. Insertion failure
// leaves allocated_store live exactly as AA5070 does; caller owns recovery.
struct NativeGuiCameraStoreAcquired {
    void* allocated_store{};
    bool inserted{};
};

// Exact aliases to existing 869A20/869810/86A2F0/86AB70/86EBE0 bodies.
// Native node18h is {left,parent,right,keybits,value,color14,nil15,pad16..17};
// tree0Ch is {opaque,head,count}. Valid same-CRT allocation domain only.
void __fastcall advance_native_gui_camera_store_iterator_004bda70(void* iterator);
void __fastcall rotate_native_gui_camera_store_right_00aa1080(
    void* tree, void* unused_edx, void* pivot);
void __fastcall rotate_native_gui_camera_store_left_00aa2180(
    void* tree, void* unused_edx, void* pivot);
// Whole AA46B0[679B]: same physical erase body as accepted86E8A0 after
// named CALL/EH operands. ECX tree, EDX unconsumed; stack output/owner/node,
// EAX output, RET0C. By-value iterator advances, original node alone is freed,
// current unsigned count decrements iffnonzero, then captured owner/node store.
// No payload release or extra owner==tree check. Reuses SAME Source CRT and
// owning invalid-iterator transport; private frames/FH3/SEH/Original heap and
// outer binary ABI remain the accepted86E8A0 boundaries, not new guarantees.
void* __fastcall erase_native_gui_camera_store_iterator_00aa46b0(
    void* actual_tree, void* unused_edx, void* actual_output,
    void* input_owner, void* input_node);

// Borrowed opaque pointer identities passed only to genuine4254B0 RET. No
// strings are read/copied or defaults supplied. Metadata remains independent
// of retired store/node backing and stable through externally serialized calls.
struct NativeGuiCameraStoreRemoveLiterals {
    const char* const remove_format_00d5bf80;
    const char* const removed_format_00d5bf54;
    const char* const empty_00f8bc60;
    NativeGuiCameraStoreRemoveLiterals(const char* remove_format,
        const char* removed_format, const char* empty) noexcept
        : remove_format_00d5bf80(remove_format),
          removed_format_00d5bf54(removed_format), empty_00f8bc60(empty) {}
};

// Whole AA4B30[237B]: ECX manager, stack requested store, RET4/void. New Source
// API supplies only borrowed literal metadata in addition. Actual manager+8
// tree/head+0C, by-value iterator and overlapping private16B output/end storage;
// preserve current head/payload checks and real checked increment. First match
// only: CURRENT store+18 camera/node -> SAME B6D800 name header+4 -> fallback
// empty only for genuine RET logger; free CURRENT nonnull payload through SAME
// Source CRT, clear captured node+10 AFTER free, then whole raw erase above.
// Valid initialized raw storage and actual NativeNodeStorage/name lifetime are
// required, not typed map/SceneResource stand-ins. No camera/scene retain/release,
// count gate, ownership owner, rollback or new cleanup is added. The raw tree,
// active metadata and surviving nodes outlive reached reads and output stores.
// Returning SDK validation follows native continuation. Private Source frames,
// Original CRT/FH3/SEH/heap/encoded-handler and outer ABI limits remain explicit.
void remove_native_gui_camera_store_00aa4b30(void* actual_manager,
    void* requested_store, const NativeGuiCameraStoreRemoveLiterals&);

void* allocate_native_gui_camera_store_tree_node_00aa2960(
    void* left, void* parent, void* right,
    const NativeGuiCameraStorePair*, std::uint8_t color);
NativeIntPointerTree18Iterator* link_native_gui_camera_store_tree_node_00aa41b0(
    void* tree, NativeIntPointerTree18Iterator* output, std::uint8_t insert_left,
    void* parent, const NativeGuiCameraStorePair*);

// Actual caller-owned stable frames, with initialized preimages. Named native
// algorithm writes and result padding are preserved at each routine's return
// frontier. Gaps and the enclosing CREATE's later reuse of its nested frame
// are NOT original return-address/saved-register/private-stack ABI claims.
// Insert captures pair_argument_10, optionally overwrites that cell, and reads
// current output_argument_0c AFTER allocation/link callbacks. x87 FCOMIP keeps
// the incoming float live across descent; equal/unordered both route right.
NativeGuiCameraStoreInsertResult* insert_native_gui_camera_store_00aa4960(
    void* actual_tree, NativeGuiCameraStoreInsertFrame&);
// Native ECX manager, stack descriptor, RET4. Actual manager+8 tree traversal,
// integer flags and four ordered x87 equalities; priority+10 is not compared.
void* find_native_gui_camera_store_00aa3280(void* actual_manager,
    const void* captured_descriptor, NativeGuiCameraStoreFindScratch&,
    const char* actual_empty_00f8bc60);
// Native ECX manager, stack camera/scene/descriptor, RET0C. Capture manager;
// current argument cells are read AFTER allocation/defaults at original sites.
// Four x87 stores can quiet sNaNs; later raw descriptor14 reread supplies the
// tree key independently. Scene1C precedes camera18 and count20=0 publication.
// No camera/scene retain/release or EH rollback. Raw manager getter4C12B0 and
// page registrationAA52A0 are separate existing providers, not invoked here.
void* create_native_gui_camera_store_00aa5070(void* actual_manager,
    NativeGuiCameraStoreCreateFrame&, NativeGuiCameraStoreMapBindings&,
    NativeGuiCameraStoreAcquired&);
// New source ABI. FH3/SEH, malformed/concurrent tree mutation, populated-store
// AA4B30 ownership cleanup and gameplay are outside this packet.
} // namespace bsp
