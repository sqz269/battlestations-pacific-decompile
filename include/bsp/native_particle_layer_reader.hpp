#pragma once

namespace bsp {
struct NativeParticleParameterBuilderRawContext;

// Complete AFAD00[1284]. Native ECX actual 40h Layer, stack actual 1Ch text
// buffer, RET4/AL=1 on every normal return. Writes only +20, byte+24, +28,
// +30,+34,+38,+3C. Borrow the application's SAME raw string cells, builder
// constants/CRT and shared F8C2C8 scratch; no independent pool or scratch.
//
// Searches for an exact opening brace then consumes through closing brace or
// EOF. Sort uses signed atol >0; other parameters construct/parse the genuine
// temporary builder and take its first value. Parse failure is ignored, and
// missing tokens, aliases and malformed inputs receive no validation/rollback.
// Current CRT parsing/comparison and literal pointer identity remain boundaries.
// True unwind owns only the recovered states; secondary cleanup exceptions
// terminate. This source interface does not preserve native stack-slot aliases,
// calling convention, hardware SEH or establish gameplay compatibility.
bool read_native_particle_layer_00afad00(void* actual_layer,
    void* actual_text_buffer, NativeParticleParameterBuilderRawContext& context,
    char* actual_shared_scratch_00f8c2c8);
} // namespace bsp
