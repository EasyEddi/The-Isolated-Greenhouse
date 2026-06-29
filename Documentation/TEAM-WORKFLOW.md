# Team Workflow

## Branches

Aktuelle Entwicklungsarbeit laeuft auf:

```text
eddis-beta-stuff
```

`main` soll nur aktualisiert werden, wenn das explizit gesagt wird oder ein getesteter Stand gemerged werden soll.

## Grundregel

Nicht zwei Personen oder Agenten gleichzeitig dieselbe `.umap` oder dieselben `.uasset` Dateien bearbeiten lassen. Unreal-Dateien sind binaer und erzeugen schnell Konflikte.

## Vor jeder Aufgabe

1. `git status` pruefen.
2. Klaeren, ob offene Aenderungen von jemand anderem sind.
3. Nur die Dateien anfassen, die zur Aufgabe gehoeren.
4. Bei Map-/Asset-Aenderungen besonders vorsichtig sein.

## Agent-Sessions

Wenn mehrere Codex/cmux-Sessions laufen, sollte jede Session eine klare Verantwortung haben:

- `map-layout`: Map, Wande, Boden, Licht, Spawnpoint.
- `inventory-items`: Inventar, Hotbar, Item-Handling.
- `models-assets`: Importierte Models, Materialien, Scale/Rotation.
- `gameplay-loop`: Planting, Wachstum, Interaktion.
- `git-sync`: Status, Commit, Push, Merge.

Eine Session darf nicht spontan Aufgaben einer anderen Session uebernehmen.

## Commit-Regeln

- Kleine, beschreibende Commits.
- Keine zufaelligen Unreal-Speicherungen mitcommiten, wenn sie nicht zur Aufgabe gehoeren.
- Wenn lokale User-Aenderungen existieren, nicht resetten oder ueberschreiben.
- Vor Push kurz `git status` und `git log -1 --oneline` pruefen.

## Merge-Regeln

- Erst Branch pushen.
- Danach `main` nur aktualisieren, wenn es explizit gewuenscht ist.
- Wenn `main` auf denselben Stand gebracht werden soll:

```bash
git push origin eddis-beta-stuff:main
```

Das ist nur dann sinnvoll, wenn der Branch der gewollte aktuelle Hauptstand ist.

## Reviews und Tests

Vor einem wichtigen Merge pruefen:

- Projekt oeffnet im Unreal Editor.
- Standard-Map ist `L_Greenhouse_MVP`.
- Spieler kann laufen und sich umsehen.
- Inventar/Hotbar laedt.
- Git LFS Assets sind vorhanden.
