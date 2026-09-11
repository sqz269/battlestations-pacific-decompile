# Runtime startup owners

Addresses: 0073d410, 005547d0, 006ab6b0, 006a7be0, 0073c960,
00aa09d0, 00aa0d30, 00aa06d0. Dependency evidence is in
LUA_RUNTIME_GLOBALS.md, RENDERER_CAPABILITIES.md and GUI_LOCALE_REFRESH.md.
Descriptive names remain hypotheses, not recovered symbols.

The executable now retains the recovered input-script state, renderer query
state and localization table owner. These run through the mounted VFS and the
actual Lua 5.1.1 and Direct3D9 implementations. Missing world, scene, device-input,
font and GUI resource bindings remain explicit; this does not establish gameplay.

## Native order and persistent ownership

The application listing at0073da66..0073daa5 constructs the renderer, calls the
input singleton005547d0, calls its loader006a7be0 and then loads settings008d8190.
The input constructor006ab6b0 already invokes the loader. Its second call is a
guarded no-op. GameScriptHost retains its LuaRuntimeGlobals, VfsLuaScriptFiles,
LuaScriptRuntime and InputScriptStartup in dependency order. The presets
interpreter and parsed InputSettings survive the frame loop. Its destructor
closes Lua before the DoFile runtime/files and mounted VFS disappear.

The startup globals are the image's zero-filled0108ff20 byte and0108ff24/28
NativeString: X360COMP=false and no REGION global in the first interpreter.
The geography helper00439100 and compatibility setter write those globals later.
They are not inputs from the initial options file. The globals remain mutable for
later settings/profile consumers; existing open Lua states retain their original
bootstrap values. See the worker's zero-fill, write-xref and call-order evidence.

GameStartupHost owns one IDirect3D9 interface. Resolution enumeration00b27d80,
the constructor's adapter check00b32874, full capability gather00b2c8e0, settings
AA probes and CreateDevice all borrow it. The previous two independent API
objects are removed. GameDeviceHost releases only its device. The parameter
region initialized by the five native stores00b32512..00b32534 is retained too;
device creation updates that same region instead of a temporary local copy.
This is still not the complete native1d94h renderer or its resource initialization.

The host also restores the relative order of existing platform/save setup before
command-line parsing/factory-tail setup, and parser registration after settings.
See APP_INIT_PLATFORM.md for0073d8f3/0073d921/0073d94a and the application listing
for0073db41..0073db69. Unimplemented phases still prevent a claim that the entire
application sequence has been reconstructed.

## Localization and GUI refresh

Constructor0073c960 (ECX=this, EAX=this, RET at0073c9ec) initializes the language
and registered-name vectors,4096 hash bucket heads/count, and two sidecar vectors.
LocaleTables already represents those values. The executable now allocates and
retains it at the corresponding startup point. Native vtable/global publication,
allocation and singleton lifetime ABI are not reproduced by this C++ owner.

At0073e08f the application selects the settings language;0073e103 registers the
literal `globals`;0073e135 reloads with force=false. The first setter sees an empty
registered-name vector and therefore does not load files or request GUI refresh.
The subsequent reload reads the installed table through VfsLocaleRuntime.
GameLocaleHost follows this order and owns a lazy GuiLocaleRefreshManager for
later language changes. That manager uses the existing GuiPageRegistry, not a
second widget graph or a fabricated callback. Text nodes require live GuiTextWidget
and GuiTextHost bindings. The initial empty registry does not create GUI resources.

The old executable log called00aa06d0 `gui_startup`; that address is the locale
reload. The remaining fonts/GUI resource entry is now identified as0073bae0.
The executable does not yet load or render those pages or populate the text bindings.

## Validation and limits

Final build, focused owner fixture, isolated executable run, source/evidence hashes,
saved Ghidra changes and worker commits are recorded in
reports/runtime_startup_owners.json. The fixture checks live input Lua globals and
retention, API borrowing/lifetime, installed locale loading and the lazy GUI registry.
The process check uses an isolated personal options root and checks the installed
game executable and original personal options hashes, sizes and modification times.

Native evidence, host fixtures, successful compilation and startup process execution
are separate from original-binary differential coverage, native ABI compatibility
and visual/gameplay validation. The frame loop still clears/presents the existing
background. Input device polling/application, profile restoration, font/GUI resources,
world/scene rendering and game-entry behavior remain required work.
