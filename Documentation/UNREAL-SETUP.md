# Unreal Setup

## Version

Verwendet Unreal Engine 5.8. Beide Teammitglieder sollten dieselbe Engine-Version nutzen.

## Projekt oeffnen

1. Repo klonen oder pullen.
2. Git LFS installieren und aktivieren:

```bash
git lfs install
git lfs pull
```

3. Projektdatei oeffnen:

```text
game/UnrealProject/TheIsolatedGreenhouse.uproject
```

4. Wenn Unreal fragt, Projektdateien/Module neu zu bauen, bestaetigen.
5. Die Standard-Map ist:

```text
Content/Maps/L_Greenhouse_MVP
```

## C++ Build

Wenn C++ geaendert wurde, den Editor-Build ausfuehren:

```bash
'/Users/Shared/Epic Games/UE_5.8/Engine/Build/BatchFiles/Mac/Build.sh' TheIsolatedGreenhouseEditor Mac Development -Project='/Users/eddi/The-Isolated-Greenhouse/game/UnrealProject/TheIsolatedGreenhouse.uproject' -WaitMutex
```

Auf Windows wird derselbe Build ueber die Unreal-/Visual-Studio-Toolchain ausgefuehrt. Wichtig ist, dass `TheIsolatedGreenhouseEditor` erfolgreich baut.

## Git LFS

Unreal-Assets sind binaer. Diese Dateitypen muessen ueber Git LFS laufen:

- `.uasset`
- `.umap`
- grosse `.fbx`/Model-Dateien

Nach einem Pull auf einem neuen Rechner immer ausfuehren:

```bash
git lfs pull
```

## Wichtige Pfade

- Projekt: `game/UnrealProject/TheIsolatedGreenhouse.uproject`
- Map: `game/UnrealProject/Content/Maps/L_Greenhouse_MVP.umap`
- C++ Code: `game/UnrealProject/Source/TheIsolatedGreenhouse`
- importierte Unreal-Modelle: `game/UnrealProject/Content/models`
- rohe/source Modelle: `game/UnrealProject/models`
- Documentation: `Documentation`

## Vorsicht bei Map- und Asset-Dateien

- `.umap` und `.uasset` lassen sich schlecht mergen.
- Immer `git status` pruefen, bevor Map/Asset-Dateien gespeichert oder generiert werden.
- Wenn jemand die Map im Editor geaendert hat, keine Generator-Scripts drueberlaufen lassen, ohne vorher abzusprechen.
- `item list.md` ist manuell gepflegt und soll nicht automatisch veraendert werden.
