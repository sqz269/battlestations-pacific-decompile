#pragma once

namespace bsp {

struct SingletonLifetimeCallbacks;

// Complete ordinary body 0082D700-0082D7D1 (210 bytes). Original ABI:
// ECX actual ship class, no stack arguments, RET, no stable result.
//
// The class borrows its model pointer at +50h. The model's actual type54 list
// starts at +54h, and the destination actual type54 list starts at class+6BCh.
// Existing destination entries are retained and every source pointer is
// appended in order. The source items remain borrowed: no item retain/release
// operation occurs in the native body.
void bind_native_ship_model_pointers_0082d700(
    void* actual_ship_class, const SingletonLifetimeCallbacks& invalid_parameters);

// New C++ source ABI. All class, model, header and backing storage is borrowed.
// The callback is required and may return after repairing storage; the source
// preserves the native captures, current-header reloads and continuations.
// Callers must supply valid storage for every native access reached after such
// a return. Native fault delivery and drop-in binary compatibility are outside
// this interface.

} // namespace bsp
