# AirportMtkWlan

**Native macOS Wi-Fi for MediaTek MT7921 / RZ608 cards on Hackintosh (macOS Sequoia 15.2+).**

The card shows up as a real Wi-Fi interface: you pick networks from the Wi-Fi menu,
macOS keeps the passwords in the Keychain, and the private Wi-Fi address works. No
separate client app is needed.

> **Status: experimental.** Tested on one machine and one access point (see
> [Tested setup](#tested-setup)). It runs in the kernel; a bug can panic your Mac.
> Keep a bootable backup EFI before installing.

## Results on the tested machine

| | Download | Upload |
|---|---|---|
| Upstream `SimpleMtkWlan` (Ethernet-style, separate client app) | 14.6 Mbit/s | 2.6 Mbit/s |
| **AirportMtkWlan** (this repository) | **~290 Mbit/s** | **~155 Mbit/s** |

Measured with `networkQuality` forced through the Wi-Fi interface and with fast.com.
The link runs at 802.11ac (VHT) 80 MHz, 2 spatial streams, MCS 9, with A-MPDU
aggregation in both directions. What changed and why is in [docs/FIXES.md](docs/FIXES.md).

## Tested setup

| Item | Value |
|---|---|
| Card | MediaTek MT7921K (AMD RZ608), PCI `14c3:0608` |
| CPU / board | AMD Ryzen 5 5600G desktop |
| macOS | 15.8.1 (24H32), OpenCore |
| Access point | WPA2-PSK, 5 GHz channel 52, 80 MHz |

The kext also matches MT7921 (`14c3:7961`) and MT7922 (`14c3:7922`) like upstream, but
**they were not tested**. Reports are welcome.

## What works / what does not

Works on the tested setup:

- scanning and joining from the native Wi-Fi menu, private Wi-Fi address, DHCP
- WPA2-Personal (CCMP); 802.11n/ac, 20/40/80 MHz; RX and TX Block Ack (A-MPDU)
- preference for 5 GHz over a stronger 2.4 GHz AP of the same network

Not tested or not working yet:

- **sleep/wake** (not tested yet)
- WPA3, enterprise (802.1X), hidden networks, 2.4 GHz-only networks: not tested
- AirDrop / AWDL, Handoff and other Continuity features: not supported
- encryption is still done in software (works at the speeds above)
- macOS asks for background scans every few seconds while connected, which causes
  occasional latency spikes
- **Bluetooth is a separate device** on the same card (USB `0e8d:0608`) and is not handled
  by this kext. It works with [mnnobre/SimpleMtkBtFw](https://github.com/mnnobre/SimpleMtkBtFw)
  (tested on the same machine: pairing, headset audio and microphone).

## Install (OpenCore)

See [docs/INSTALL.md](docs/INSTALL.md). In short:

1. Back up your EFI (to another USB stick or computer).
2. Copy `AirportMtkWlan.kext` to `EFI/OC/Kexts/`.
3. Add it under `Kernel -> Add` with `MinKernel` `24.2.0` and `MaxKernel` `24.99.99`.
4. Disable or remove any other driver for the card (`SimpleMtkWlan.kext`, `itlwm`-style
   kexts for this device). Two drivers matching the same card will conflict.
5. Reboot and choose your network in the Wi-Fi menu.

## Build

Xcode is **not** required, only the Command Line Tools (`xcode-select --install`):

```sh
./build.sh            # build/Release/AirportMtkWlan.kext
./build.sh ethernet   # build/Release/SimpleMtkWlan.kext + tools/mtkctl (see below)
```

`build.sh` fetches [acidanthera/MacKernelSDK](https://github.com/acidanthera/MacKernelSDK)
at a pinned commit, reads the source list and settings from the Xcode project or
`AirportMtkWlan/build.json`, compiles with `clang` and links the kext
([tools/build-xcode-kext.py](tools/build-xcode-kext.py)).

### Ethernet-style variant

`SimpleMtkWlan.kext` is the original front end (the card appears as an Ethernet
interface and is controlled with `tools/mtkctl` or SimpleMTK.app). It shares the
MediaTek code with the native kext and has runtime tunables
(`mtkctl tune ht= ba= vht= bw= txba= sndq= txs= reassoc=1`). The build of this variant
that was tested on hardware is commit `2d09355`; built from this branch it is untested.

## Debugging

The driver keeps its own log and publishes it on the PCI device; IOLog output does not
reach the unified log on the tested machine.

```sh
tools/mtklog.sh | tail -100                 # driver log
/usr/bin/log show --last 10m --predicate 'process == "kernel" AND eventMessage CONTAINS "[wcl]"'
```

(In zsh use `/usr/bin/log`; `log` alone is a shell builtin.)

## How this was made

The native front end is the `AirportItlwm-Sequoia` target of
[laobamac/itlwm](https://github.com/laobamac/itlwm) put on top of the MediaTek HAL of
[SimpleMtkWlan](https://github.com/laobamac/SimpleMtkWlan) (via the
[KyatNguyen `mtk-dev`](https://github.com/KyatNguyen/SimpleMtkWlan/tree/mtk-dev) branch).
Both projects share the same HAL interface, so the port is mostly glue plus fixes in
the MediaTek driver. See [CREDITS.md](CREDITS.md).

The code changes in this repository after the upstream commits were written by
**Claude Code** (Anthropic's AI coding agent) and tested on hardware by the repository
owner. Commits carry a `Co-Authored-By: Claude` trailer.

## License

GPLv2, see [LICENSE](LICENSE). Individual files keep their own license headers
(BSD/ISC for OpenBSD-derived code). `tools/ClientKit` is from
[laobamac/SimpleMTKClient](https://github.com/laobamac/SimpleMTKClient) under the BSD
3-Clause license in `tools/ClientKit/LICENSE`. The MediaTek firmware embedded in the kext
is redistributed as in upstream SimpleMtkWlan.
