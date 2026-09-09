# Shader combiner table conversion

The Lua adapter now populates all14 combiner string slots using00b439c0 and
00b437f0 semantics. Outer lookup missing/non-table leaves the supplied slots
unchanged. Outer lua_next iteration accepts only keys passing the native
integer-number predicate; it does not sort or use indexed lookup. Each accepted
value is treated as an inner table.

Inner lua_next has a different ordinal rule from shader field records: every
entry advances the ordinal, including keys rejected by the integer-number test.
At ordinal0 an accepted key supplies the mode: exact Lua NUMBER is converted
through float32 and signed truncation, otherwise the default is13. At ordinal1
an accepted key supplies an exact string, otherwise its initially empty name
remains. String extraction uses the native C-string boundary. Extra entries
are ignored. The resulting string replaces the slot at that mode, so later
entries win, including an empty string overwriting an earlier value.

Native00b437f0 receives two stack arguments (destination array and Lua entry),
RET8.00b439c0 receives ECX descriptor plus three stack arguments (destination,
Shader reference, field name), RET0Ch. Descriptor strings begin at+48h with
eight-byte stride. The inner mode local is uninitialized if ordinal0 never
qualifies, and native indexing has no bounds check. The owning adapter rejects
those cases and non-table entry values explicitly; it does not reproduce unsafe
memory writes or native allocator behavior. Body evidence is in
`reports/shader_combiner_states.json`.

The installed debug shader fills modes0,1,3,4 with dummy.shfx. The probe selects
normal mode0 and sends its actual string to the resolver. The diagnostic resolver
explicitly maps that basename to shaderfx/lights/dummy.shfx; native basename and
overlay registration remain unported. It then evaluates the selected descriptor
in a fresh state as before. The effect filename is no longer selected independently
of Shader.Combiners, but other modes and full pass creation are still unvalidated.

Alongside this change, the adapter converts RenderStates from the verified31-key
registry and passes them into the recovered state-block binder before drawing.
The debug output is ZWRITE(ID14)=0 then ZTEST(ID7)=0; ZENABLE is unregistered and
ignored. Device readback confirms both zeros. The existing generated shader draw
still produces centerFF407FBF/outsideFF000000 and restores its state. Win32 build,
both existing CTests and the full D3D9/native-matrix probes pass; no new test target
was added. Registry/conversion details are in `SHADER_RENDER_STATE_TABLE.md`.
