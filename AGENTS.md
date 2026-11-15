# Link zmk-config

- ZMK v0.3 (Zephyr 3.5). Board uses the legacy hardware model, so stay on v0.3-compatible modules
  (prospector-zmk-module `main`, not `feat/new-status-screens`).
- Board: `boards/arm/link` (copy of cannonkeys/zmk-cannonkeys-keyboards). Dongle shield: `boards/shields/link_dongle`.
- Keymap: `config/link.keymap` (used by halves and dongle). Dongle-only options: `config/link_dongle.conf`.

## Local build (mirrors GitHub Actions)

```sh
mkdir -p /tmp/linkws && rsync -a --exclude .git ./ /tmp/linksrc/ && cp -r config /tmp/linkws/
docker run --rm -v /tmp/linkws:/ws -v /tmp/linksrc:/src -w /ws zmkfirmware/zmk-build-arm:3.5 sh -c '
  [ -d .west ] || west init -l config; west update --fetch-opt=--filter=tree:0; west zephyr-export
  west build -p -s zmk/app -d build/dongle -b seeeduino_xiao_ble -S studio-rpc-usb-uart -- \
    -DZMK_CONFIG=/src/config -DSHIELD="link_dongle prospector_adapter" -DZMK_EXTRA_MODULES=/src -DCONFIG_ZMK_STUDIO=y'
```

Swap board/shield/cmake-args per entry in `build.yaml`. Keep the west workspace separate from the repo
(the repo's `zephyr/module.yml` would collide with the `zephyr/` checkout).
