# SPC Gear GK650K Omnis

*Full-size RGB keyboard on a EVision VS11K09A, with a volume wheel.*

* Keyboard Maintainer: [Bonn333](https://github.com/Bonn333)
* Hardware Supported: SPC Gear GK650K Omnis, including the Pudding Edition (US ANSI)
* Hardware Availability: [spcgear.com](https://spcgear.com)

Make example for this keyboard (after setting up your build environment):

    make spcgear/gk650k_omnis:default

Flashing example for this keyboard:

    make spcgear/gk650k_omnis:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in following way:

* **Physical reset button**: short the `BOOT` pin (pin 3) to ground while plugging the keyboard in
## Fn layer

`Fn` is `MO(1)`: hold it to reach layer 1, where every key not listed below is
`KC_TRNS` and falls through to the base layer.

| Combo | Keycode | Effect |
| --- | --- | --- |
| `Fn` + `Esc` | `QK_BOOT` | Put the keyboard into bootloader mode for flashing |
| `Fn` + `F1` | `RM_TOGG` | Toggle RGB Matrix on or off |
| `Fn` + `F2` | `RM_NEXT` | Cycle through animations |
| `Fn` + `F3` | `RM_PREV` | Cycle through animations in reverse |
| `Fn` + `F4` | `RM_VALU` | Increase the brightness level |
| `Fn` + `F5` | `RM_VALD` | Decrease the brightness level |
| `Fn` + `F6` | `RM_HUEU` | Cycle through hue |
| `Fn` + `F7` | `RM_SATU` | Increase the saturation |
| `Fn` + `F8` | `RM_SPDU` | Increase the animation speed |

The volume wheel is an encoder on the base layer and sends `KC_VOLU`/`KC_VOLD`.
