#pragma once

#include "bsp/native_scene_property_insertion.hpp"

namespace bsp {

// Ordinary Source binding to the SAME actual canonical process owners used by
// insertion and recursive retirement. Explicit CC8A30 startup must have returned.
// The clone-specific F89450 role is a distinct permanent const-zero byte in the
// retained string process; Original writable .data is not universally immutable.
class NativeScenePropertyCloneContext final {
public:
    NativeScenePropertyCloneContext(game::GameNativeStringProcess& actual_strings,
        game::GameNativePhysicalPoolProcess& actual_physical);
    NativeScenePropertyCloneContext(const NativeScenePropertyCloneContext&) = delete;
    NativeScenePropertyCloneContext& operator=(const NativeScenePropertyCloneContext&) = delete;

    NativeScenePropertyInsertionContext& insertion() noexcept { return insertion_; }
    const char* clone_empty() const noexcept { return clone_empty_; }

private:
    NativeScenePropertyInsertionContext insertion_;
    const char* const clone_empty_;
};

// Complete ordinary successful-domain reconstructions with new C++ interfaces.
// Names are hypotheses. Borrow actual stable raw61-produced 114h bags: real
// map at+4, consistent count/64 buckets, finite live14h nodes, canonical owning
// pooled keys, valid nonnull records tagged0..11. Closed NUL-free ASCII keys,
// including empty, current C locale and stable-zero F89450 Source role only.
// Record/bag ownership graphs must be finite, unshared and acyclic. All payload
// spans and NUL-terminated texts meet their selected raw constructor contracts;
// Type8 count is positive, Type9/10 byte_size=4*n with1<=n<=3FFFFFFFh,
// Type11 byte_size=12*n with1<=n<=15555555h. New roots/children are distinct
// actual owning allocations from the same successful canonical current heap.
// Keep canonical pool membership, keys, fallback cells and processes live until
// recursive retirement. Require DF0, sufficient stack and no provider failure,
// invalid memory/alias/overflow, concurrent mutation/trim or reentry.
//
// Type1 executes actual FLD m32/FSTP m32 on source+0C. At EVERY conversion the
// x87 stack is empty, all six exception masks already1, PC24/53/64 and any RC,
// with a valid CW and no pending unmasked exception. All binary32 bit patterns
// are allowed; hardware conversion/NaN quieting/sticky status are retained.
// Production code never resets CW/status/MXCSR or adds C++ FP conversions.
// Net x87 depth is zero; undefined condition codes, popped register contents,
// FIP/FDP/FOP addresses and Original register/stack/FS identity are not oracles.
//
// These functions do not supply Original parent ECX/RET/FS/SEH/EH/OOM transport,
// historical private CRT, class/vtable dispatch, startup/World or game ABI.
// Retire output through the existing SAME canonical ordinary lifetime context;
// source roots, keys, ordinals, payloads and owner backlinks remain borrowed.

// Whole [008F41F0,008F428A),154 bytes/49 instructions. Allocate and initialize
// the real bag with counter/owner0, then traverse the actual source map with
// 00480690/0047E480. Capture each node key before recursive record cloning;
// null key uses the distinct canonical F89450 Source role. Full008F33F0 inserts
// each clone, so actual bucket-head order and zero-ordinal stamping are retained.
// Source owner/counter are not copied; child owner+110 stays0 until attachment.
void* clone_native_scene_property_bag_008f41f0(const void* actual_source_bag,
    NativeScenePropertyCloneContext& context);

// Whole [008F4F60,008F52B6),854 bytes/274 instructions, all twelve arms.
// Read tag, allocate actual38h root BEFORE arm payload reads, call its exact raw
// constructor, then physically reread source+34 and copy it to result+34.
// Type6 allocates this record before the fixed recursive whole bag producer and
// attaches its actual empty/populated child with unchanged raw008EF780.
// Preserve each constructor's partial writes; no root zero-fill or owner repair.
// Invalid unsigned tag>11 returns nullptr without allocation; populated bag
// callers require valid tags and never skip or repair a null clone.
void* clone_native_scene_property_record_008f4f60(const void* actual_source_record,
    NativeScenePropertyCloneContext& context);

} // namespace bsp
