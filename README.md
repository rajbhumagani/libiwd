# libiwd: C++20 iwd Wrapper Library

`libiwd` is a synchronous, embedded-friendly C++20 wrapper around Linux iwd for STA/client connectivity management via a mockable transport adapter.

## Architecture

Public API facade:
- `libiwd::IwdClient`

Core modules:
- `ScanManager`: scan orchestration (fresh/cached mode)
- `NetworkManager`: saved profile CRUD + library-priority updates
- `ConnectionManager`: connect/disconnect and best-candidate workflow
- `SelectionEngine`: deterministic ranking policy
- `PriorityStore`: JSON policy metadata persistence (`/opt/misc/config` default)
- `JournalLogger`: singleton logger to journald (stderr fallback)

Transport boundary:
- `IIwdAdapter` abstracts D-Bus/iwd calls so production and tests can use different backends.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Dependencies

Required:
- C++20 toolchain
- CMake >= 3.20

Optional:
- systemd (for journald output through `sd_journal_print`)

## Example usage

```cpp
auto adapter = std::make_shared<MyIwdDbusAdapter>();
libiwd::LibraryConfig cfg;
cfg.metadataStorePath = "/opt/misc/config/libiwd_priority_store.json";
cfg.alwaysScanBeforeConnect = true;
cfg.selectionPolicy.enforceMinRssi = true;
cfg.selectionPolicy.minRssi = -80;

libiwd::IwdClient client(adapter, cfg);
client.addNetwork({.id="home", .ssid="HomeWifi", .security=libiwd::SecurityType::Wpa2Psk, .psk="secret"});
client.setNetworkPriority("home", 100);
auto selected = client.connectBestNetwork();
```

## Priority JSON store

Default location is `/opt/misc/config/libiwd_priority_store.json`.

Schema:

```json
{
  "version": 1,
  "entries": [
    {"id":"home","priority":100,"enabled":true,"last_success":1712000000}
  ]
}
```

Stored metadata drives library policy (priority, enabled state, last-success tie-breaker).

## Selection behavior

Ranking order:
1. Eligible and enabled saved network
2. Meets optional minimum RSSI threshold
3. Higher configured priority first
4. Stronger signal among equal priority
5. Optional last-success tie-breaker
6. Stable lexical network-id ordering

## Logging

`JournalLogger` emits info/warning/error/debug messages for scanning, CRUD, priority updates, selection decisions, and connect attempts.

## Limitations / design notes

- Current code provides a clean transport abstraction; a concrete production D-Bus adapter must be added for full iwd integration.
- APIs are synchronous in v1 to keep integration simple for embedded backends.
- Current security support targets Open + WPA2/WPA3 PSK, with extension points for future 802.1X.
- Structure is designed for future async APIs, telemetry, and advanced roaming logic.
