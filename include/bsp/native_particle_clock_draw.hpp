#pragma once

namespace bsp {

// Complete B19A10, actual 1Ch clock owner. Original ECX owner, one float32
// stack word, RET4. Unused EDX reserves that original stack slot in this
// source fastcall declaration. +18 receives the original raw input bits.
// Each record's actual virtual sink receives a fresh x87 load/spill of the
// current public argument, preserving x87 status/NaN behavior and callbacks
// that modify the argument. No typed clock, callback projection or mode clamp.
void __fastcall set_native_particle_clock_time_00b19a10(
    void* actual_clock, void* unused_edx, float frame_time);

// Array start+8 and count+Ch define 2Ch records with sink pointer+28h.
// Initial cursor is captured before the +18 store. After each virtual call,
// current count and base form the wrapped end BEFORE advancing the cursor.
// Existing native storage/callable vtable and valid traversal are required.
} // namespace bsp
