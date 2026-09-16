# Application render-entry cache lifetime (R70)

Addresses: 00bebf00, 00bec240, 00bec3e0, 00bec590, 00bec630, 00bec6f0,
00bec870, 00bec8e0, 00bec910, 00becee0, 00bd0400.

The application now calls the existing `00BED1E8..00BED222` cache startup
fragment after device startup. `GameNativeRendererApplication` retains its
actual `14h` owner publication and `NativeRenderEntryCacheContext`, borrowing
the application's canonical singleton manager and mapped `00D7A24C` literal.
The fragment allocates and registers the owner, creates its tracked critical
section, and allocates 10,000 actual `28h` records. It does not create another
singleton manager or substitute a C++ container for the native record array.

The shared singleton dispatcher now admits the two cache profiles:

| Current owner profile | Actual scalar deleter |
| --- | --- |
| `00D68CBC` | `00BEC6F0`, base |
| `00D68CC0` | `00BEC910`, derived |

Dispatch passes the popped owner and original flags into the existing complete
providers. They retain their current-publication removal and clearing rules.
The source binding table grows by one pointer, appended at offset 132; its
existing offsets remain unchanged. The application retains all bindings until
the raw manager drain and checks that the cache publication was cleared.
An interrupted cache initialization uses the existing process-retention policy.

## Evidence and validation

Fresh live bytes from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, match the installed PE for 1,883 bytes: all nine
cache bodies, the manager destructor, the enclosing window body, both profile
slots and the one literal. The window body's complete capture is context;
the application binding claims only its 59-byte cache fragment. The preceding
native instruction at `00BED1E6` calls the renderer device method. Manager site
`00BD0485` calls the current owner profile's slot zero with flags one.

The MSVC Win32 build and all three existing CTests pass. No repository test
case was added. A focused local probe links the current game objects and
libraries and uses the real `GameSingletonHost`, canonical manager cell and
mapped original literal. It verifies all defined fields in 10,000 records,
the actual header/section and one registration, then executes the real manager
drain. A second lifetime exercises the base profile. Both flags-one dispatch
routes pass and clear the same cache and manager publications; the child exits
zero. This is source execution, not execution of original machine bodies.

The application launch stopped before window/device/cache initialization with
`FMOD bank raw-length output unavailable`. Its renderer reported zero modes;
the independent D3D9 probe reported zero adapters. Windows reported the running
user session disconnected. These observations are preserved separately: the
exact FMOD error and causal relationship to session state were not established.
The application cache call and its combined renderer shutdown remain unexecuted
in this packet. The focused lifetime result does not replace that missing check.

## Remaining work

The cache is required by EndFrame's unconditional `0108FE88 + 8` store, but
native BeginFrame/EndFrame are still unbound in the application. The actual
render-command queue needs complete publication/destruction composition, and
nonempty command/debug paths need their substantive contexts and persistent
frames. Renderer-resource service construction, online startup, original
ApplyAll and gameplay also remain open. The current application still draws
through its existing D3D bridge.

No full window reconstruction, original register/stack ABI, FH3/SEH, failing
allocation, concurrent mutation, visual parity or gameplay claim is made.
Machine-readable native evidence, runtime logs, and scoped artifact receipts
are in `reports/native_render_entry_application_r70.json`.
