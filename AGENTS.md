# Link zmk-config

- ZMK v0.3 (Zephyr 3.5). Board uses the legacy hardware model, so stay on v0.3-compatible modules
  (prospector-zmk-module `main`, not `feat/new-status-screens`).
- Board: `boards/arm/link` (copy of cannonkeys/zmk-cannonkeys-keyboards). Dongle shield: `boards/shields/link_dongle`.
- Keymap: `config/link.keymap` (used by halves and dongle). Dongle screen options: `config/dongle_screen.conf` (YADS, default) and `config/prospector_adapter.conf` (original Prospector screen); ZMK picks them up by shield name.

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

## Bongo cat (`boards/shields/yads_bongo_cat`)

Add-on shield for YADS, used as `link_dongle dongle_screen yads_bongo_cat`. YADS stays unmodified: its
`custom_status_screen.c` is compiled with `zmk_display_status_screen` renamed to `yads_display_status_screen`
(deferred `set_source_files_properties` in the shield's CMakeLists), and `src/bongo_cat.c` provides the real
`zmk_display_status_screen()` that builds the YADS screen and adds the cat. The cat reacts to key presses
(`zmk_position_state_changed`); `side_for_position()` encodes which keymap positions are on the left half. Re-check this if YADS is bumped.

## nice!view screens (zmk-nice-oled)

Halves use the `nice_epaper` shield from mctechnology17/zmk-nice-oled (nice!view support). Shared options in
`config/nice_epaper.conf`; each half's picture is picked via `cmake-args` in `build.yaml` (a `link_left.conf` would also
be applied to the settings-reset builds, which lack this module, and break them). Gotcha: when switching to the gem animation also set
`CONFIG_NICE_OLED_WIDGET_ANIMATION_PERIPHERAL_GEM=y`, otherwise `..._ANIMATION_PERIPHERAL_MS` has no value
and the build fails in `configs.c`.
