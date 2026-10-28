# BEETLE BLUE V1.0.0 RC8 — PROJECT DATA ERROR fix

Gebruik **BEETLE-BLUE-Manager.html** of **BEETLE-BLUE-V1.0.0-RC8-WebInstaller.html**.
De nieuwe firmware is **BEETLE-BLUE-V1.0.0-RC8-PROJECT-SAVE-FIX.fwsc**.
Oudere bestanden zijn behouden als referentie; gebruik voor deze fix RC8.

## Oorzaak en correctie

De samplecache en het te bewaren project delen hetzelfde RAM. RC7 riep na
project-capture/decoding `beetle_bd4_live_invalidate()` aan. Die functie schrijft
cacheflags en wist chunks/attack-snapshots via de oude pointer. Daarmee werd de
klaargezette song beschadigd voordat de encoder hem controleerde. De fout is
op de originele RC7 SAVE-wrapper gereproduceerd: `TRACK_JOURNAL_INVALID`,
diagnose 4 (PATTERN / NOTES), bij de eerste opslag op zowel lege legacy-opslag als geformatteerde V2-opslag.
Hetzelfde gedeelde pad wordt gebruikt voor Storage V2 en projectimports.

RC8 ontkoppelt de cache met `beetle_bd4_live_bind(0,0u)` zonder projectbytes te
wijzigen. Na de transactie wordt de cache opnieuw opgebouwd. Capture ontkoppelt
eerst; eerste opslag na initialisatie maakt een verse capture na de bevestiging
in plaats van oude scratchdata te vertrouwen. Validatie, capaciteitcontroles,
CRC, schrijfautorisatie en opslagformaat blijven behouden. Geen extra grote
RAM-buffer; geen automatische wipe, migratie of projectnormalisatie toegevoegd.

## Validatie (software, geen aangesloten FM1)

- Nieuwe test `python3 tests/track/test_blue_rc8_save_workspace.py`: werkelijke
  SAVE-wrapper, cache, encoder/decoder en beide writers. 48 succesvolle saves:
  alle 12 slots op legacy én V2, elk tweemaal, afwisselend capture en decoded
  import. Teruggelezen serialisatie bytegelijk; andere slots behouden; ongeldige
  BPM geweigerd zonder NOR-mutatie. De backendselectie en hardware-I/O zijn mocks.
- Dezelfde test tegen ongewijzigde RC7-wrapper faalt bij de eerste SAVE.
- V2-writertests: 12 projecten / 13 samples, 71 gesimuleerde stroomonderbrekingen,
  transacties, volle opslag, corrupte metadata en orphan collection: PASS.
- Legacy workspace/journal: 12 slots en 9 onderbrekingspunten: PASS.
- FX-save regressie: PASS.
- DPCM4 flash-to-audio regressie: exacte 40000-frame referentie en zes stemmen
  gedurende 44000 frames: PASS in hostsimulatie.
- JieLi schrijfbare build: 536068 bytes; 45496 bytes ruimte over; app-audit PASS.
- JieLi read-only build en audit op verboden schrijffuncties: PASS.
- Firmwarepakket: beschermde eerste 16 KiB en OTA-loader identiek aan RC7;
  gedecrypteerde app gelijk aan build; containercontrole PASS.
- Manager en losse installer bevatten exact RC8, met overeenkomende SHA-256;
  JavaScript syntaxcontrole PASS. Browser/hardware-OTA niet getest.

## Gebruik

Pak de ZIP uit en gebruik de RC8 Manager/firmware. **Wipe All Storage is niet
nodig voor deze geheugenfix.** De bestaande opslag wordt niet door deze codefix
geformatteerd. Firmware-installatie herstart het apparaat: nog niet opgeslagen
werk in RAM blijft niet behouden. Maak waar mogelijk eerst een backup.

Deze fout is in software aangetoond en gecorrigeerd; een fysieke FM1-test moet
nog bevestigen dat opslaan op jouw apparaat werkt. Bij een resterende fout is
ook de onderste diagnoseregel van het scherm nodig; herhaal geen wipes.

Volledige broncode, eerdere documentatie en functies zijn behouden.
