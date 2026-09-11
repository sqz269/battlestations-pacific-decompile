# Native material factory and pool CRT wrappers

The actual-storage entry `create_native_material_for_effect_00535320` connects
the current renderer's callable virtual `+48` to the established material pool,
constructor, and actual reference-count release. It borrows the same actual
8h effect-name header and captures `F8D394` once. The returned effect must be a
nonnull owned actual identity with its count at `+04` and writable byte `+B4`.
This is an explicit binding requirement: the semantic effect cache and a
numeric original vtable do not provide callable reconstructed acquisition.

The full caller `00535320..0053539B` has ECX=name, EAX=material, plain RET. It
calls renderer `+48` before `B18780` allocation, arms state0 only after that
allocation returns, calls `B18900` for a nonnull slot, disarms cleanup, then
unconditionally releases the acquired effect. Null allocation therefore still
drops the temporary. Acquisition and allocation failures have no armed cleanup.
The original `C6C240` state0 funclet loads `[EBP-10]` and jumps to `B17D70`;
that 12-byte helper supplies the canonical material pool to `B17A80`, returning
only the raw slot. No extra effect release or material destructor belongs in
this constructor-failure cleanup. The new C++ catch preserves this resource
ordering without claiming the original SEH representation or drop-in ABI.

The two 22-byte startup functions at `CD78D0` and `CD78F0` initialize the actual
material and parameter pools respectively, then call CRT `atexit` with
`CE0BF0` or `CE0C00` and return its result. Those 10-byte exit wrappers supply
`F8D3AC`/`F8D3E4` and tail-call `B180D0`/`B18470`. Their C++ bindings reference
the existing canonical pool companions and shared allocator list; they create
no replacement globals, pool storage, or private exit registry. Initialization
is not rolled back when callback registration fails. Binding/storage lifetime
and shutdown after all payloads have died are caller preconditions.

Names are descriptive hypotheses, not recovered symbols. Exact validation,
source and artifact hashes, and fixture boundaries are recorded in
`reports/native_material_factory.json`. MSVC Win32 and both existing CTest cases
pass. One ignored fixture executes the complete original factory on successful
construction and null allocation, comparing a full 114h material slot, retained
count, effect dirty byte, and acquisition/allocation events. It executes the
original raw-return helper through the actual state0 funclet with an explicit
EBP-10 frame slot, verifies pool LIFO reuse, and checks the host allocation
failure's unarmed cleanup. Full original SEH exception delivery is not tested.

Four original pool CRT wrappers route to the same rebuilt pool bodies, checking
their selected globals, callback order and registration-result propagation.
The host wrappers register real process-exit callbacks; a final observer runs
after both and verifies the shared allocator list is empty and both elements
have returned to their base profile. No permanent tests were added.

The factory fixture uses a controlled borrowed effect prefix with actual +04
and +B4 fields, retains an external count throughout, and rejects terminal
effect-owner resolution. The original caller uses the same rebuilt material
constructor and pool dependencies as the host path. This checks the factory's
ordering; it does not independently prove the constructor, shader identity,
effect lifetime, native allocator, or original CRT implementation.

Real renderer effect acquisition and
effect destruction, GUI ownership, drawing, and gameplay remain unvalidated.
