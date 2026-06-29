# Product Brief

## Projektname

Isolated Greenhouse

## Kurzbeschreibung

Isolated Greenhouse ist ein ruhiges First-Person-Spiel in Unreal Engine. Der Spieler lebt und arbeitet in einer abgeschotteten Halle mit Gewaechshausbereich, Arbeitsplatz, Lagerbereich und kleinem Wohnbereich. Er zieht Pflanzen, pflegt sie, verarbeitet Ableger und erfuellt spaeter Bestellungen ueber einen Computer.

Das Spiel soll friedlich, isoliert und leicht melancholisch wirken, aber nicht nach Horror oder Survival.

## Zielgruppe

Spieler, die ruhige Management-, Farming- oder Cozy-Games moegen und Interesse an Pflanzen haben.

## Warum die Idee funktioniert

Das Spiel kombiniert Pflanzenpflege, kleine Arbeitsablaeufe und eine ungewoehnliche isolierte Halle. Der Reiz liegt nicht in Stress, Kampf oder Horror, sondern in Fortschritt, Ordnung, Wachstum und dem Ausbau des Gewaechshausbereichs.

## Spielziel

Der Spieler baut nach und nach eine groessere Pflanzenzucht auf, verkauft Ableger oder Pflanzen und schaltet bessere Ausstattung frei.

## Aktueller technischer Stand

- Engine: Unreal Engine 5.8
- Projekt: `game/UnrealProject/TheIsolatedGreenhouse.uproject`
- Standard-Map: `L_Greenhouse_MVP`
- Projektart: First-Person Unreal-Projekt mit C++-Grundsystemen und Unreal-Assets
- Branch fuer laufende Arbeit: `eddis-beta-stuff`

## Aktueller Gameplay-Stand

- First-Person-Bewegung funktioniert.
- Spawnpoint und Map-Grundform existieren.
- Inventar mit 5 Hotbar-Slots und 3x9 Inventar-Slots existiert.
- Debug-Buttons koennen Lily und Watering Can geben.
- Lily stackt bis 10 pro Slot.
- Watering Can ist einzigartig.
- Lily kann am Planting Plot gepflanzt werden.
- Gepflanzte Lily waechst ueber Zeit sichtbar.

## MVP

### Muss funktionieren

- Spieler kann die Map im Editor starten.
- Spieler kann laufen, sich umsehen und die Hotbar nutzen.
- Spieler kann Lily ins Inventar bekommen.
- Spieler kann Lily an einem markierten Platz pflanzen.
- Lily-Wachstum ist sichtbar.
- Die Map hat eine klare Raumaufteilung nach dem Grundriss.

### Danach

- Watering Can als Pflege-Interaktion verwenden.
- Computer/Order-System anschliessen.
- Einfache Bestellung abschliessen.
- Geld/Shop erst einbauen, wenn Planting und Order-Loop stabil sind.

### Nicht Teil des ersten MVP

- Mutationen.
- Multiplayer.
- komplexes Speichern.
- viele Pflanzenarten.
- komplette Wirtschaftssimulation.

## Raumaufteilung

- Wohnbereich: Bett, Kueche, einfache Alltagsobjekte.
- Arbeitsbereich: Schreibtisch und PC.
- Gewaechshausbereich: Platz fuer Greenhouse-Modelle und Pflanz-Interaktionen.
- Storage: Regale, Werkzeug-/Materiallager.
- Freier Bereich: Platz fuer spaetere Erweiterungen.

Die originale Skizze bleibt Referenz fuer die grobe Aufteilung, aber die Map soll nicht wie farbige Rechtecke aussehen.

## Verantwortlichkeiten

### Eddi

- Map, Raumaufteilung und statische Umgebung.
- First-Person-Gefuehl.
- Inventar/Hotbar-Grundsystem.
- erste Planting-Interaktion.
- Unreal-Asset-Integration fuer Map-nahe Modelle.

### Tarek

- Pflanzeninhalte und Pflanzenlogik, sobald die Grundinteraktion bereit ist.
- Items/Tools, die nicht nur statische Map-Umgebung sind.
- spaeter Computer-/Order-/Shop-Flows nach Absprache.

### Gemeinsam

- Keine parallelen Aenderungen an derselben Map-Datei.
- Branches sauber halten.
- Vor groesseren Merges testen, ob das Projekt im Editor startet.
- Bugs konkret mit Screenshot, erwarteter Position und aktuellem Verhalten beschreiben.

## Design-Regeln

- Ruhig, warm, verlassen, aber nicht gruselig.
- Statische Halle zuerst stabil halten, dann Gameplay ausbauen.
- Keine unnoetigen Props generieren, wenn sie spaeter von Hand/als Model kommen sollen.
- Map-Generatoren duerfen manuell gesetzte Objekte nicht ueberschreiben.
