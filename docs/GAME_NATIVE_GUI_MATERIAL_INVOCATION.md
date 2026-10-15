# Guarded GUI material invocation

`GameNativeGuiEffectApplication::create_material_00535320` composes the existing
whole numeric `535320` factory and canonical material admission inside the
same retained effect application. It supplies the operation needed by a future
AB8530 `guidefault.mshd` consumer. This packet does not replace the legacy Text
callers, construct a Text, acquire a new cache, change compilation modes, or
run graphics. Source names remain hypotheses and the interface is new C++.

The exact Source/build/artifact receipt is
`reports/cc12_native_gui_material_invocation.json`.

## Admission and provenance

The caller supplies a stable initialized actual8h name header, the existing
`GuiNativeGeometryOwners`, a stable `NativeMaterialDestructionAccess`, and a
null `const GameNativeGuiMaterialFactoryOperation*` output slot. Entry rejects
reentry, a failed/draining/closed graph, or a previously published output slot.
It repeats the effect application's existing renderer/VFS/string/cache/compiler/
sampler/Lua domain checks before entering native work.

Material admission additionally requires all of the following identities:

- Geometry is exactly this renderer's
  `game_grid_context().graphics.streams.geometry`; its owners and registration
  context/callbacks are the renderer's canonical actual-owner registry.
- Destruction access borrows those same actual owners and the VFS's actual
  string service. Its material and parameter pools are the existing process
  `F8D3AC` and distinct `F8D3E4` pools. Their getters require completed explicit
  startup; this API does not initialize, substitute, or reset either pool.
- The renderer's new ready-only
  `borrow_gui_material_destruction_profile_00d5e520()` validates its existing
  graphics/geometry/owner domain and obtains exactly eight bytes through its
  existing `GameNativeReadOnlyData::data_at`. The selectors must be
  `{BD30E0,B194B0}`. A copied caller table is never admitted.

Fresh read-only `bsp.py ghidra bytes 00d5e520 --length 8` returned
`e0 30 bd 00 b0 94 b1 00`, matching `NativeMaterialReference`'s existing
destruction checks. The BSP wrapper verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base against the configured
`C:/Users/sqz269/bsp.gpr` target before the byte read. No Ghidra mutation occurred.

The renderer method is only a borrowed authenticated profile view. It does not
expose the private effect cache or create an owner, pool, program, or callable
vtable. The renderer/read-only-data owner must outlive the view's consumers.

## One invocation, one retained frame

After admission, the effect application allocates a fresh heap frame, appends
it to its own `material_operations`, and writes the caller's output pointer.
These publications precede Lua activation and every native factory effect.
The vector owns `unique_ptr` entries, so later vector growth does not move the
published frame. No copy or second native owner is introduced.

The graph then enters its existing `loading` phase, records the same Lua
binding, activates it synchronously, and observes the live actual renderer.
The existing `create_native_material_from_effect_cache_00535320` owns the
whole recovered factory schedule:

1. Capture current renderer and acquire the actual effect through numeric
   `D5F0A8/+48/B318B0` with the same private effect cache and one retained cache
   frame.
2. Allocate from the real material pool and run `B18900`. A constructor
   exception returns only its raw slot according to the existing factory.
   Acquisition/allocation/constructor errors gain no protective effect release.
3. On normal factory completion, release its temporary effect reference in the
   existing order, including a null material-slot result.
4. If a material was returned, call the existing
   `register_native_material_creator` on the same canonical geometry owner.
   This creates metadata only: no extra native retain, duplicate registry, or
   destructor rollback. A bind failure leaves its creator, entry and companion
   in the persistent factory frame.
5. Check the canonical material companion aliases that material's actual+04,
   record the observed profile/count, and transfer the single creator only on
   normal API return. A native null allocation returns null without invented
   success material or a new bad-allocation policy.

The published frame records input identity, borrow activity, renderer/Lua,
geometry/destruction/profile bindings, the entire `NativeMaterialFactoryAcquired`,
result observations, transfer status, and exception. It is inspectable through
the caller output and `material_operation()` even after a caught failure.
The result/factory pointers become diagnostic identities after caller transfer
or later retirement; neither alias is another owning reference.

## Names, failures, and drain

This API never consumes the caller's name. The header and live bytes remain
stable throughout synchronous execution. On return or unwind, its name borrow
is marked inactive. Recovered caller cleanup, including AB8530's name cleanup,
may then consume the header's contents. Keep the caller's header storage/frame
alive through failure, but do not treat the saved pointer as readable or replayable
after consumption. The inactive bit does not assert whether caller cleanup ran.

An exception after frame publication stores the exception and marks both
operation and application failed. It retains the complete factory/cache/
registration state. No API retry, name restoration, creator release, cleanup
replay or second drain is performed. Lua activation restores the previous
thread binding on normal return/C++ unwind; interrupted Lua is not resumable.
The same application phase drives `requires_process_retention()` for effect
and material work. Admission/allocation failures before publication have no
entered native operation to retain.

The **same destruction-access object**, its pools/string/owner references,
geometry, mapped profile, renderer, VFS and Lua owner must remain alive through
material final release and any failed invocation. A stack access object that
expires while the material survives is invalid: the canonical companion borrows
that exact access object. Caller-held creators must be transferred/released
through existing actual owner paths before shared renderer drain.

Completed material frames stay beside existing effect frames until the original
shared drain. The application destructor now requires drain closure if either
operation collection is nonempty. `after_native_renderer_drain` only takes its
never-entered shortcut when both collections are empty; otherwise it verifies
the original renderer drain and completed material/cache metadata without
dereferencing retired materials. It adds no cleanup order or native release.

## Remaining consumer binding and evidence limits

AB8530 still invokes the old geometry material factory, and AB8CE0 still has its
legacy renderer+48 effect call. A following consumer packet must retain the
new published operation pointer in its caller frame, provide the proven owner
domains above, preserve caller name cleanup, and connect constructor material
creation in the common preload/cache order. The separate font selection,
native Text construction, buffers, command/pass contexts, draw and retirement
remain unactivated. `GuiFont` preload or an existing frontend screenshot is not
evidence that this invocation ran.

Validation is the normal MSVC Win32 build and existing checks, plus bounded
source/profile/receipt review; see the receipt for actual results. There are no
new test fixtures or GPU launches. This does not establish Original register,
class layout, private-frame or FH3 ABI, a Native material invocation at runtime,
Native Text rendering, full startup, or gameplay equivalence.
