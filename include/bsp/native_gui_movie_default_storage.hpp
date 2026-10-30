#pragma once
#include <cstdint>

namespace bsp {

// Whole normal AAFE20,142B/32 instruction starts. Native ECX is the actual
// destination, EAX returns that same destination, plain RET/no stack arguments.
// This explicit two-argument Source API is not that register ABI or native FH3.
//
// Borrow fresh DWORD-aligned native storage covering +00..+12F. The existing
// Movie pool has a 130h payload and a separate trailing slab-index DWORD; this
// function neither allocates/returns its slot nor value-initializes the payload.
// Required actual D7A24C storage is initialized, stable, disjoint and retained
// through the call. The real AA9390(type10) base reads it separately before
// and after its sentinel allocation, with its existing failure contract.
//
// Writes the native profile identity and exactly the observed derived fields.
// F4/F5..F7,106..107,10C,11C,120 and every other unwritten byte stay untouched.
// The profile word is evidence metadata, never a callable PE table. No semantic
// GuiMovieFields/MovieWidgetState overlay, copy/destructor, caller or consumer
// admission, rollback, guard change or ownership transfer is supplied here.
void* construct_native_gui_movie_default_00aafe20(void* actual_storage,
    const volatile std::uint32_t& actual_one_00d7a24c);

} // namespace bsp
