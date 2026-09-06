# Simply Knock - Special Edition build notes (2026-09-06)

Chesko's Simply Knock (Legendary Edition, MIT). Branch layout in this fork: `master` mirrors
`upstream` (chesko256), the Special Edition work lives on **`IcZ-SSE`** (remote `origin`, MrGezz).
`main` is an empty GitHub default branch.

The port itself landed on 2026-09-05 as `925668c "SSE Port"`: `src/main.cpp`,
`src/SimplyKnock.cpp`, `src/SimplyKnock.h`, `.gitattributes`. Its content is described in the
vault entity `simplyknock_sse_port` - one native, `GetLinkedDoor`, reading the reference's
`ExtraTeleport` with the x64 layout taken from CommonLibSSE and `static_assert`ed.

## Why this file exists: the DLL had no provenance

Until 2026-09-06 the DLL running in the build (`D:\Mosais\mods\Simply Knock SE\SKSE\Plugins\
SimplyKnock.dll`, 182,272 bytes, built 2026-09-03 20:56) was committed on no branch and had been
produced by a hand-run `cl.exe` / `link.exe` sequence whose working directory was not kept. The
only DLLs in the repository were Chesko's 32-bit LE `simplyknock.dll` and the old LE
`StorageUtil.dll`. `buildcheck.py` listed the mod as third party, "as downloaded" - false - and
could not have noticed a stale rebuild.

## The build, now reproducible

`SimplyKnock.vcxproj`, same shape as `PapyrusUtil.vcxproj` (the house pattern): every
dependency path is an overridable property with workspace-layout defaults.

    msbuild SimplyKnock.vcxproj -p:Configuration=Release -p:Platform=x64 -p:TrackFileAccess=false

- toolset **v145**, `stdcpp17`, `/MT`, `/O2`, whole-program optimisation + LTCG (the static SKSE
  library is LTCG).
- `SKSE64Root` = `..\..\skse64\`, `CommonRoot` = `..\..\common\` (ianpatt's checkouts at the
  workspace root; the root `skse64` already defines `RUNTIME_VERSION_1_7_104`).
- `SKSELibDir` = `..\Runtime 1.7.104\SKSE64 2.3.1\lib\`: `common.lib`, `skse64_common.lib` and
  **`skse64_1_7_104_static.lib`** - the SKSE DLL's own objects minus `skse64.obj`. The import
  library alone does not carry `LookupREFRByHandle` / `g_invalidRefHandle`.
- `RUNTIME_VERSION=0x1070680`; must match the objects in that static library.
- `ForcedIncludeFiles common/IPrefix.h` - without it every skse64 header fails with
  `_MEMBER_FN_BASE_TYPE is not a class or namespace`.
- link: the three SKSE libs + `user32 shell32 ole32 advapi32 version dbghelp shlwapi comdlg32`
  (`comdlg32`: `common.lib`'s `IFileStream` pulls `GetOpenFileNameA`).
- `-p:TrackFileAccess=false`: MSBuild's FileTracker failed on this workspace's path length in
  the 2026-09-03 attempt; the flag only disables incremental-build tracking.

Output `bin\x64\Release\SimplyKnock.dll` (git-ignored), 182,784 bytes.

### Verified equivalent to the orphan it replaces

| | new build | orphan (2026-09-03) |
|---|---|---|
| exports | `SKSEPlugin_Load` @ `19A0`, `SKSEPlugin_Version` @ `2BA90` | same names, **same RVAs** |
| imports | USER32, SHELL32, KERNEL32 (static CRT) | same |
| `SKSEPluginVersionData` | v1, plugin 1, "SimplyKnock"/"Chesko", vi 0, vix 0, compatible {0x1070680} | identical |
| `skseaudit.py` | OK (versionIndependence=0x0) | OK |

Only the PE timestamp and 512 bytes of link layout differ. Deployed 2026-09-06 22:0x; the
orphan is kept at `Project Bin\mo2-backups\Simply Knock SE-2026-09-06\SKSE\Plugins\`.
`buildcheck.py` now pairs the deployed DLL with `bin\x64\Release\` (ARTIFACTS), so the mod is
**tracked** and a stale rebuild is visible; the `mod-sources.json` line is gone.

Not version independent (`vi 0`, pinned to 1.7.104) on purpose: the addresses come from
skse64's own `RelocAddr` table, compiled per runtime. A game update means a rebuild against a
new static library, which is now a one-line command.

## Vendored PapyrusUtil headers removed

`Scripts/Source/JsonUtil.psc` and `Scripts/JsonUtil.pex` were Chesko's copies of the LE-era
PapyrusUtil 3.x headers. The deployed mod shipped the `.psc` loose, and it differs from
PapyrusUtil 4.7's (md5 `7494c221…` vs `65fd7fc7…`). No runtime effect - nothing executes a
`.psc` and the `.pex` was never deployed - but a stale header on any compile path. Removed
from the deployed mod (quarantined beside the orphan DLL) and from the working tree on
`IcZ-SSE`, for the same reason Wearable Lanterns dropped its `JsonUtil` / `StorageUtil` sources:
PapyrusUtil 4.7 is a runtime dependency shipped as its own MO2 mod.

## Legendary Edition leftovers still on the branch

Not touched, listed so nobody mistakes them for the SE build:

- `SKSE/Plugins/simplyknock.dll` - Chesko's 32-bit LE plugin (637,440 bytes). Case-insensitively
  the same name as the SE DLL, so it must never be deployed from here.
- `SKSE/Plugins/StorageUtil.dll` - LE PapyrusUtil's 32-bit StorageUtil.
- `simplyknock_skse_plugin/` - the LE Visual Studio project and its `Debug/` DLL.

The deployed mod (68 files) was hand-assembled from this tree plus the DLL; it is not a
`deploy-map.json` tree pairing because the deployed `SimplyKnock.esp` (2026-09-04) went through
the form-44 resave pipeline and no longer matches the LE one committed here.

## Not done

- Not yet launched in game on the new DLL (nor was the orphan): `SimplyKnock.log` in
  `Documents\My Games\Skyrim Special Edition\SKSE\` should say `SimplyKnock SE loaded (runtime
  01070680)` and `SimplyKnockSKSE registered`.
- The three LE leftovers above could be deleted from `IcZ-SSE`; the resaved `.esp` could be
  committed so the mod becomes a tree pairing.
