# Raw texture-source constants

This packet adopts the concrete providers from
`a6b2884f2:src/native_material_texture_source_constants.cpp`:

- `00C302F0` reads the current texture from the actual source's frame array.
  A zero count returns null; a nonzero count reads the current index and array
  without adding validation or retaining the returned texture.
- `00BBCC40` reads vertex-shader bytes `+1E/+1F`. A byte of `FF` skips its
  constant. The scale writes preserve the native order; color copies reload
  each DWORD between stores. The vertex bank starts four bytes after the
  caller's actual header. Aliasing effects are preserved.
- `00BBCBD0` is the original three-byte `RET 14h` body and performs no stores.

The source retains the complete native instruction schedules, including
`00BBCC40`'s saved ESI and five stacked arguments. The C++ declarations supply
ECX and an explicit unused EDX argument where needed to expose that stack
shape; this does not establish the whole caller or object ABI.

These functions borrow actual source/shader/bank storage. They introduce no
texture-source owner, constant bank, fallback, or renderer activation.
Exact current-byte, build and focused fixture evidence is recorded in
`reports/native_material_texture_source_constants_r43.json`.
