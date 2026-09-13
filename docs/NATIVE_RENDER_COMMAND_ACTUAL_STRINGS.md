# Native command lifetime with actual string storage

Addresses: 00b1f170, 00b1f1f0, 00b1edc0, 00b1ddd0, 00b1e6b0

The existing full command constructors, initializer and destruction now accept
`NativeRenderCommandActualEnvironment`. Its string reference is the same
`ActualNativeStringPoolStorage` publication and pool used by B1DFF0 collection
and B1D760 group cleanup. These are five existing reconstructed bodies extended
to the actual string domain, not five new native routines.

| Native range, inclusive | Original ABI | Coverage |
|---|---|---|
| B1EDC0..B1EF56 | ECX command, four pointers, RET10h | Complete initializer |
| B1F1F0..B1F273 | ECX placement, four pointers, EAX same, RET10h | Complete constructor |
| B1F170..B1F1EF | ECX placement, three pointers, EAX same, RET0Ch | Complete constructor |
| B1DDD0..B1DFEC | ECX command, RET | Complete teardown |
| B1E6B0..B1E6CD | ECX command, flags, EAX original, RET4 | Complete scalar deletion |

The raw44h command, actual context and batch companions, retained owner domains
and group models are unchanged. The older `NativeRenderCommandEnvironment`
overloads remain available. Both interfaces share the original field/call
sequence; only the string allocation/return implementation differs.

When the diagnostic length differs from one, B1EDC0 first allocates two bytes,
then reloads the old data pointer and current length before returning old
storage. It publishes the replacement and length before terminating the string.
The final copy reloads the current pointer and length. Each actual allocation
or return obtains the current419CC0 publication through the existing full
actual-pool provider; the environment does not cache a raw pool pointer.

Teardown drops both actual batch references, walks the current indexed group
end, destroys each captured group and frees it before clearing the captured
cell. Group names use the same actual pool as their allocation. Scene/context
release and ordered/indexed arrays precede the current diagnostic return.
Constructors preserve the original array/name-only unwind schedule; no new
scene/context/batch rollback is added. Scalar free still occurs only after
successful teardown and only when flags bit0 is set.

Strict Win32 source compilation passed. One adapted original-byte command
fixture passed1459 normalized words across both constructors and populated
teardown, actual diagnostic/small-name ring returns, large group names, retained
model/context/batch owners, scalar flags2, batch reuse and shutdown. The older
interface replay passed1451 words, including an indexed-end mutation during
group-name cleanup. Its fixture lookup now excludes dead context addresses,
which can otherwise collide with a subsequently reused allocation. The actual
string probe does not repeat that mutation. These counts are observations,
not independent test cases. Final exact-source build and library-only replays
are recorded separately in the report.

Both fixtures adapt the declared CRT/singleton/string provider boundaries to
existing concrete source services. Original command EH tables and cleanup
states are reviewed statically, without original command FH3/SEH execution.
Actual pool release retains the established NativeStringStorage noexcept
returning-getter contract. The new interfaces do not establish original binary
ABI, application service composition, visible rendering or gameplay.

Full B1D950 execution, actual queue creation and the composed application render
path remain required. Descriptive names are hypotheses; complete byte spans,
call sites and validation details are in
`reports/native_render_command_actual_strings.json`.
