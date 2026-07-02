# Release Builds

GitHub Actions baut Release-Zips automatisch, wenn auf `main` gepusht oder gemerged wird.

Workflow:

```text
.github/workflows/release-builds.yml
```

## Ergebnis

Der Workflow erstellt einen GitHub Release mit:

- `IsolatedGreenhouse-Windows.zip`
- `IsolatedGreenhouse-macOS.zip`

Windows enthaelt den packaged Unreal-Build mit `.exe`. macOS enthaelt den packaged Mac-Build.

## Runner-Anforderung

Unreal Engine 5.8 ist nicht auf normalen GitHub-hosted Runnern installiert. Deshalb braucht der Workflow selbst gehostete Runner:

- Windows Runner mit Labels: `self-hosted`, `Windows`
- macOS Runner mit Labels: `self-hosted`, `macOS`

Auf beiden Runnern muss Unreal Engine 5.8 installiert sein.

Erwartete Standardpfade:

```text
Windows: C:\Program Files\Epic Games\UE_5.8
macOS: /Users/Shared/Epic Games/UE_5.8
```

Wenn Unreal woanders liegt, die Pfade in `.github/workflows/release-builds.yml` anpassen:

```text
WINDOWS_UE_ROOT
MAC_UE_ROOT
```

## Git LFS

Der Workflow checkt LFS-Dateien aus und fuehrt `git lfs pull` aus. Die Runner brauchen daher Git LFS.

## Manuell starten

Der Workflow kann auch manuell ueber GitHub gestartet werden:

```text
Actions -> Build Game Releases -> Run workflow
```
