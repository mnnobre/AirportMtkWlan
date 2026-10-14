# Installing AirportMtkWlan with OpenCore

Requirements: macOS Sequoia 15.2 or newer (Darwin 24.2+), OpenCore, a MediaTek MT7921 /
RZ608 card. SIP and AMFI settings are not changed by this kext.

## 1. Back up

Copy your whole `EFI` folder somewhere else (another USB stick or computer). If the
system does not boot, you restore `config.plist` from there.

## 2. Copy the kext

Put `AirportMtkWlan.kext` in `EFI/OC/Kexts/`.

## 3. config.plist

Add an entry under `Kernel -> Add` (it does not need Lilu; order does not matter):

```xml
<dict>
    <key>Arch</key>
    <string>x86_64</string>
    <key>BundlePath</key>
    <string>AirportMtkWlan.kext</string>
    <key>Comment</key>
    <string>MediaTek MT7921 native Wi-Fi</string>
    <key>Enabled</key>
    <true/>
    <key>ExecutablePath</key>
    <string>Contents/MacOS/AirportMtkWlan</string>
    <key>MaxKernel</key>
    <string>24.99.99</string>
    <key>MinKernel</key>
    <string>24.2.0</string>
    <key>PlistPath</key>
    <string>Contents/Info.plist</string>
</dict>
```

Disable any other kext that drives the same card (for example `SimpleMtkWlan.kext`):
set its `Enabled` to `false`.

## 4. Reboot and connect

Open the Wi-Fi menu, choose your network and enter the password. The first connection
after boot can take a few seconds longer while macOS applies the private Wi-Fi address.

If Ethernet is also connected, macOS sends normal traffic over Ethernet. Change
the order in System Settings -> Network -> ... -> Set Service Order, or unplug the
cable, to use Wi-Fi.

## If something goes wrong

- **Does not boot / kernel panic**: on another computer, put back the backed-up
  `config.plist` (or set `Enabled` of `AirportMtkWlan.kext` to `false`).
- **Wi-Fi does not appear**: check that the kext is loaded:
  `kmutil showloaded | grep AirportMtkWlan`.
- **Does not connect**: collect `tools/mtklog.sh` output and
  `/usr/bin/log show --last 10m --predicate 'process == "kernel" AND eventMessage CONTAINS "[wcl]"'`
  and open an issue with both.
