# Native particle shaders and model resources

Addresses: 00b089e0 00b06210 00b07c80 00af80e0 00b0a040 00af8350 00af8940 00af9660 00b80d70

AR reconstructs these complete normal native bodies over the existing actual
8-byte string storage, resource pointers and array descriptors. Names are
descriptive hypotheses. All nine complete spans match the saved executable and
the verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` program.
Source interfaces borrow services and are not drop-in native ABI replacements.

| Routine | Coverage and native ABI |
| --- | --- |
| B089E0 / B06210 / B07C80 | Complete Sprite/Axial/Floating shader setters; ECX definition, C-string stack argument, RET4. |
| AF80E0 / B0A040 | Complete Object/Tracer shader noops; three-byte RET4, arguments unread. |
| AF8350 | Complete model-pointer reserve; ECX actual0Ch descriptor, signed capacity, RET4. |
| AF8940 | Complete reverse model release; ECX definition, RET. |
| AF9660 | Complete Object model filename selection and loading sequence; ECX definition, C string, RET4. |
| B80D70 | Complete default-factory resource wrapper; ECX manager, actual8h name, RET4/EAX resource. |

The three shader setters construct the exact `Additive` counted string and
compare the actual incoming counted string case-insensitively through native
435C40/BF7FBF (_stricmp). Both
temporaries are released before the boolean word is written at definition+7C.
The Object and Tracer profile targets accept and ignore the shader argument.

AF9660 produces the Object definition's actual model array at +8C/+90/+94
(base/count/capacity). It first releases existing resources backwards through
physical +4 InterlockedDecrement and captured current virtual00. A callback can
change the descriptor: the source retains the captured cell and rereads the
current count after dispatch. Array allocation and capacity survive clearing.
AF8350's saved free fall-through needed correction: AF83A1..AF83A9 is reachable
after BF6989 and publishes the new base and capacity before the final RET4.

The loader first passes its mutable filename through current VFS BDF4C0 and
ignores this initial boolean. It separates the last extension, scans trailing
digits while the index is strictly greater than zero, and either tries one name
or increments a zero-padded numbered sequence until the first failed resolve.
Thus an all-numeric stem retains its first character as a prefix. The native
32-bit number increment wraps. Temporary-string release order is preserved.
Each successful name goes through the actual resource manager and appends the
returned pointer, including null, without another retain.

The factory publication F8D31C is tested before 4C1400. The explicit-factory
branch rereads it after that getter; the default branch invokes B80D70, which
reads current manager+4. This distinction survives publication changes made by
the getter. Both routes require real B80720, whose factory decides the resource
type and its current virtual08; this work establishes no unique Object model
instance profile. The VFS and resource service contracts remain required.

Property B015C0 now directly dispatches the five captured known shader targets.
Object parser AF8BD0 can directly call AF9660 when its optional resource binding
uses the same base/owner domain. Other current targets keep their required real
application dispatcher. These are concrete opt-in connections; application
wiring, native FH3 and gameplay remain separate validation boundaries.

Strict Win32 source compilation passed. Original-byte differential and combined
build results are recorded in `reports/native_particle_type_resources.json`.
