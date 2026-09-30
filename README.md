# SbusRx

Bibliothèque Arduino/PlatformIO pour décoder un signal **SBUS** (Futaba, FrSky, ELRS, ...) sur **ESP32**.
Aucune dépendance externe : l'UART est configuré en matériel (100 000 bauds, 8E2, inversion RX gérée par hardware).

## Câblage

| Récepteur | ESP32 |
|---|---|
| SBUS (signal) | GPIO libre, ex. GPIO2 |
| 5V / 3V3 | 5V ou 3V3 selon récepteur |
| GND | GND |

Le signal SBUS est *inversé* (logique négative) : c'est géré par l'UART en hardware,
pas besoin de transistor. Si le récepteur sort du SBUS non inversé (certains ELRS),
la bibliothèque bascule automatiquement la polarité après 1,5 s sans trame.

## Installation

**PlatformIO** (projet) :
```ini
lib_deps =
    https://github.com/NoePeterlongo/sbus.git
```
ou copier ce repo dans `lib/SbusRx`.

**Arduino IDE** : cloner ce repo dans `~/Arduino/libraries/SbusRx`.

## Format d'une trame SBUS

- 25 octets, envoyés toutes les ~7 à 14 ms (111 Hz standard, ELRS souvent ~130 Hz)
- Série : **100 000 bauds, 8 bits, parité paire, 2 bits de stop** (8E2), LSB en premier
- Le signal est inversé par rapport à une UART classique

```
Octet  Contenu
-----  ------------------------------------------------------------
0      Entête : toujours 0x0F
1-22   16 canaux de 11 bits, empilés en little-endian au niveau bit
23     Drapeaux
24     Octet de fin : 0x00
```

**Drapeaux (octet 23) :**

| Bit | Signification |
|---|---|
| 0 | Canal 17 (digital, 0/1) |
| 1 | Canal 18 (digital, 0/1) |
| 2 | **Failsafe** acté par le récepteur (pas de lien TX) |
| 3 | Trame perdue par le récepteur |

**Empilement des canaux (octets 1-22) :** 16 × 11 bits = 176 bits, sans alignement
d'octet. Le canal `i` commence au bit `11*i` :

```
canal 0 : octets 1-2      (bits 0-10)
canal 1 : octets 2-3      (bits 11-21)
canal 2 : octets 3-5      (bits 22-32)
...
canal 15 : octets 21-23   (bits 165-175)
```

**Valeurs :** chaque canal est sur 11 bits, donc 0-2047.

| Position manche | Valeur brute | Équivalent µs |
|---|---|---|
| Extrême bas | 172 | 1000 µs |
| Neutre | 992 | 1500 µs |
| Extrême haut | 1811 | 2000 µs |

Hors de ces bornes, c'est du failsafe (souvent 0 ou valeurs figées).
`channel_us(i)` fait la conversion brute → µs.

## API

```cpp
SbusRx sbus;
```

| Méthode | Description |
|---|---|
| `begin(rxPin, inverted = true, serial = Serial2)` | Démarre l'UART SBUS sur la pin donnée |
| `read()` | À appeler dans `loop()`. Lit l'UART, renvoie `true` si **au moins une nouvelle trame** a été décodée depuis le dernier appel (non bloquant) |
| `sbus[i]` ou `channel(i)` | Valeur brute du canal `i` (0-15), 11 bits |
| `channel_us(i)` | Idem converti en µs (~1000-2000) |
| `is_failsafe()` | Drapeau failsafe de la dernière trame |
| `is_linked()` | Trames récentes (< 100 ms) **et** pas de failsafe |
| `ch17()`, `ch18()` | Canaux digitaux 17/18 |
| `frame_rate()` | Trames par seconde (moyenne glissante) |
| `frame_count()` | Nombre total de trames décodées |
| `last_frame_ms()` | `millis()` de la dernière trame reçue |
| `rx_inverted()` | Polarité actuellement utilisée |

## Exemple

```cpp
#include <SbusRx.h>

SbusRx sbus;

uint32_t last_data = 0;
bool     fail      = false;
uint16_t  ch0, ch1;

void setup() {
  Serial0.begin(115200);
  sbus.begin(2);              // récepteur sur GPIO2
}

void loop() {
  if (sbus.read()) {          // nouvelle trame disponible
    last_data = millis();
    fail = sbus.is_failsafe();
    ch0  = sbus[0];           // gaz / roll, selon le mapping de la radio
    ch1  = sbus[1];
  }

  if (millis() - last_data > 200) {
    // aucune trame depuis 200 ms : lien perdu
  }
}
```

Exemple complet à flasher : [`examples/sbus_basic`](examples/sbus_basic).

```sh
pio run -d examples/sbus_basic -t upload --upload-port /dev/ttyUSB0
```

Sortie type (récepteur sous tension, émetteur éteint — d'où le failsafe) :

```
[SBUS] 130.0 fr/s  fs=1  link=0  CH: 1:997 2:1075 3:240 4:1022 ... 16:1024
```

## Notes

- `read()` n'est pas bloquante : à 130 Hz et 25 octets par trame, le buffer de 256 octets
  laisse ~80 ms de marge entre deux appels à `loop()`.
- La parité 8E2 fait que seul un signal SBUS bien formé produit des octets exploitables ;
  l'entête `0x0F` et un timeout inter-octets de 10 ms servent de resynchronisation.
- Testé sur ESP32-S3 (devkitC-1) avec un récepteur ELRS ; le code n'utilise rien de
  spécifique au S3 à part `HardwareSerial`, donc S2/C3/classiques devraient fonctionner.
