# iDiRT MUD

iDiRT is a Multi-User Dungeon (MUD) server -- a text-based multiplayer online role-playing game descended from AberMUD. Originally written in 1994-1996 by Shawn Hill (Illusion), it is built on the AberMUD DIRT 3.1.2 codebase by Alf and Nicknack (1990, 1993).

Players connect via telnet to explore a fantasy world with 41 zones, over 1,400 locations, 384 NPCs, and 740 objects. The game features combat, magic, quests, player progression, and a full wizard administration system.

## Quick Start (Docker)

The fastest way to get iDiRT running:

```bash
docker build -t idirt .
docker run -d --name idirt -p 6715:6715 --network host idirt
```

Connect with any telnet client:

```bash
telnet localhost 6715
```

## Building from Source

### Prerequisites

- GCC
- Make
- C preprocessor (cpp)

### Build Steps

```bash
cd src
make depend
make
cd ../bin
./aberd
```

The `aberd` binary is the MUD daemon. Run it from the `bin/` directory so it can find the data files in `../data/`.

### Command-Line Options

| Flag | Description |
|------|-------------|
| `-f` | Run in foreground (don't daemonize) |
| `-k` | Start and check mode |
| `-u` | Update mode |
| `-r <pid>` | Restart with previous PID |
| `-n <num>` | Set max players (default: 32, max: 1000) |
| `-v` | Show version information |

## Configuration

Edit `include/config.h` before compiling:

| Setting | Default | Description |
|---------|---------|-------------|
| `PORT` | 6715 | TCP port the MUD listens on |
| `MASTERUSER` | "YourName" | Name of the master admin character |
| `UNVEIL_PASS` | "Password" | Admin unveil password |
| `MUD_NAME` | "iDiRT" | Display name of the MUD |
| `IDLE_MAX` | 3600 | Idle timeout in seconds (0 = disabled) |
| `AUTOSAVE_TIME` | 600 | Autosave interval in seconds |
| `GLOBAL_MAX_MOBS` | 10000 | Maximum mobiles in game |
| `GLOBAL_MAX_LOCS` | 10000 | Maximum locations in game |
| `GLOBAL_MAX_OBJS` | 10000 | Maximum objects in game |

## Project Structure

```
idirt/
├── bin/            # Compiled binaries (aberd, generate, Dump, pfilter)
├── data/           # Game data files
│   ├── ZONES/      # Zone definition files (41 zones)
│   ├── HELP/       # In-game help files
│   ├── INFO/       # Information files
│   ├── LOGS/       # Game logs
│   ├── POLICY/     # Policy files
│   ├── WIZ_ZONES/  # Wizard-created zones
│   ├── bootstrap   # Bootstrap data pointers
│   ├── generate.conf # World generator config
│   ├── uaf_rand    # Player account database (binary)
│   └── verbs.src   # Verb definitions
├── doc/            # Documentation and license
├── include/        # Header files
├── src/            # C source code
│   └── utils/      # Utility programs (Dump, pfilter, setlevel, convert)
├── utils/          # Build scripts (makedep, backup)
└── Dockerfile      # Docker build file (Ubuntu 22.04)
```

## Utilities

| Utility | Description |
|---------|-------------|
| `aberd` | Main MUD daemon |
| `generate` | World file generator (builds zone data from ZONES/) |
| `Dump` | Dumps binary game data for inspection |
| `pfilter` | Filters and displays player records from the UAF |
| `setlevel` | Sets a player's level: `setlevel <uaf_file> <name> <level>` |

### Player Levels

| Level | Value |
|-------|-------|
| Guest | 0 |
| Novice - Legend | 1 - 11 |
| Wizard | 12 |
| Full Wizard | 2,000 |
| Prophet | 10,000 |
| ArchWizard | 20,000 |
| Advisor | 40,000 |
| Avatar | 65,000 |
| God | 90,000 |
| Master | 95,000 |

## Zones

The game world includes 41 zones: Ancient, Blizzard, Camelot, Castle, Catacomb, Cave, Church, East Forest, Fantasy, Forest, Frobozz, Hick, Home, Ice Cave, Island, Labyrinth, Ledge, Mithdan, Moapt, Moor, Mountain, Mrealms, Oaktree, Orchold, Playhouse, Quarry, Ruins, Sea, Sherwood, Talon, Tower, Treehouse, Valley, Village, Waste, Xlimbo, and Zodiac.

## Tested Platforms

- Ubuntu 22.04 (Docker, native)
- Ubuntu 18.04 (native, WSL)
- Debian 12 (UGREEN NAS via Docker)

## License

GPL v2. Copyright (c) 1994-1996 Shawn Hill. Derived from AberMUD DIRT 3.1.2, Copyright (c) 1990, 1993 by Alf and Nicknack. See `doc/LICENSE` for full terms.
