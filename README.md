# STM32F4 Custom Bootloader

A custom UART bootloader for STM32F4 (Nucleo-F446RE) that lets a host PC flash and query the target over a simple binary command protocol, while keeping human-readable debug logs on a separate UART.

Built while following the [STM32Fx Microcontroller Custom Bootloader Development](https://www.udemy.com/course/stm32f4-arm-cortex-mx-custom-bootloader-development/) course by FastBit Embedded Brain Academy / Kiran Nayak

## How it works

- On reset, the firmware checks the **B1 (user) button**.
  - **Pressed** → stays in bootloader mode and waits for commands over UART.
  - **Not pressed** → jumps straight to the user application flashed at `0x08008000` (Sector 2).
- The host talks to the bootloader using a length-prefixed, CRC32-protected binary command format (see below).
- Every command is CRC32-checked before it's processed. A good CRC gets an **ACK** + reply data; a bad CRC gets a **NACK**.

### Two-UART split

| UART | Role | Connected via |
|---|---|---|
| **USART2** (`C_UART`) | Binary command/response protocol it talks to `STM32_Programmer_V1.py` | ST-LINK Virtual COM Port (`/dev/ttyACM0`) |
| **USART3** (`D_UART`) | Human-readable debug logging (`printmsg`) | External USB-TTL adapter |

Keeping these on separate physical UARTs means the debug text never corrupts the binary protocol stream, and you can watch the debug console (e.g. moserial) live while the Python tool talks to the board.

![Wiring / setup screenshot here]()

---

## Memory map (Nucleo-F446RE, 512K flash)

The bootloader and the user app are **two separate STM32CubeIDE projects**, each with its own linker script, so they never overlap in flash:

| Region | Address range | Size | Used by |
|---|---|---|---|
| Bootloader | `0x08000000` – `0x08007FFF` | 32K (Sectors 0–1) | `Custom_Bootloader` project |
| User app | `0x08008000` – `0x0807FFFF` | 480K (Sectors 2–7) | `User_app` project |

`FLASH_SECTOR2_BASE_ADDR = 0x08008000U` in `main.c` (bootloader) must always match the user app's `FLASH.ld` origin, that's the address the bootloader reads the app's MSP and reset handler from before jumping to it.

### F446RE flash sector layout

Unlike the RAM, the F446RE's flash sectors are **not** uniform size and this matters once `BL_FLASH_ERASE` is implemented for real, since it erases whole sectors:

| Sector | Size | Address range |
|---|---|---|
| 0 | 16K | `0x08000000` – `0x08003FFF` |
| 1 | 16K | `0x08004000` – `0x08007FFF` |
| 2 | 16K | `0x08008000` – `0x0800BFFF` |
| 3 | 16K | `0x0800C000` – `0x0800FFFF` |
| 4 | 64K | `0x08010000` – `0x0801FFFF` |
| 5 | 128K | `0x08020000` – `0x0803FFFF` |
| 6 | 128K | `0x08040000` – `0x0805FFFF` |
| 7 | 128K | `0x08060000` – `0x0807FFFF` |

Sectors 0 and 1 hold the bootloader

**Bootloader linker script** : `Custom_Bootloader/STM32F446RETX_FLASH.ld`:

```ld
MEMORY
{
  RAM    (xrw)  : ORIGIN = 0x20000000,  LENGTH = 128K
  FLASH  (rx)   : ORIGIN = 0x8000000,   LENGTH = 512K
}
```

**User app linker script** : `User_app/STM32F446RETX_FLASH.ld`:

```ld
/* 512K total - 32K used by bootloader = 480K remaining for the app */
MEMORY
{
  RAM    (xrw)  : ORIGIN = 0x20000000,  LENGTH = 128K
  FLASH  (rx)   : ORIGIN = 0x08008000,  LENGTH = 480K
}
```

## Getting started

1. Flash the bootloader project to the board via STM32CubeIDE / ST-LINK.
2. Wire a USB-TTL adapter to the USART3 pins and open a terminal (moserial, PuTTY, etc.) at **115200 8N1** to view debug logs.
3. Leave the ST-LINK VCP (USART2, `/dev/ttyACM0`) free for the Python tool.
4. Hold the **B1** button while resetting the board to enter bootloader mode.
5. Run the host tool:
   ```bash
   python STM32_Programmer_V1.py
   ```
   Enter the port name for USART2 (e.g. `/dev/ttyACM0`) when prompted, then pick a command from the menu.

![Debug terminal screenshot here]()

![Python tool menu screenshot here]()


## Command protocol

Every command from the host has the form:

```
[len_to_follow] [command_code] [...params...] [crc32 (4 bytes, little-endian)]
```

`len_to_follow` is the number of bytes after itself (command code + params + CRC).

Reply format:
- **ACK:** `0xA5 <len_to_follow>` followed by `<len_to_follow>` bytes of reply data
- **NACK:** `0x7F`

| # | Command | Code (host) | Packet length | Description | Status |
|---|---|---|---|---|---|
| 1 | `BL_GET_VER` | `0x51` | 6 | Reads the bootloader version | ✅ Implemented |
| 2 | `BL_GET_HELP` | `0x52` | 6 | Lists all supported command codes | 🚧 Stub |
| 3 | `BL_GET_CID` | `0x53` | 6 | Reads the MCU chip ID | 🚧 Stub |
| 4 | `BL_GET_RDP_STATUS` | `0x54` | 6 | Reads flash Read-Protection (RDP) level | 🚧 Stub |
| 5 | `BL_GO_TO_ADDR` | `0x55` | 10 | Jumps execution to a given address | 🚧 Stub |
| 6 | `BL_FLASH_MASS_ERASE` | -- | -- | Erases all flash sectors | 🚧 Stub |
| 7 | `BL_FLASH_ERASE` | `0x56` | 8 | Erases one or more flash sectors | 🚧 Stub |
| 8 | `BL_MEM_WRITE` | `0x57` | 11 | Writes data to flash | 🚧 Stub |
| 9 | `BL_EN_R_W_PROTECT` | `0x58` | 8 | Enables read/write protection on sectors | 🚧 Stub |
| 10 | `BL_MEM_READ` | `0x59` | -- | Reads memory contents | 🚧 Stub (not supported in host tool) |
| 11 | `BL_READ_SECTOR_P_STATUS` | `0x5A` | 6 | Reads sector protection status | 🚧 Stub |
| 12 | `BL_OTP_READ` | `0x5B` | -- | Reads One-Time-Programmable memory | 🚧 Stub (not supported in host tool) |
| 13 | `BL_DIS_R_W_PROTECT` | `0x5C` | 6 | Disables read/write protection | 🚧 Stub |
| 14 | `BL_MY_NEW_COMMAND` | `0x5D` | 8 | Placeholder for a custom command | 🚧 Stub |

See [`Custom_Bootloader/COMMANDS.md`](Custom_Bootloader/COMMANDS.md) for a per-command reference with screenshots.


## Project structure

Two independent STM32CubeIDE projects, each with their own linker script and `.ioc`:

```
Custom_Bootloader/                    # The bootloader itself
├── Core/
│   └── Src/
│       └── main.c                    # Command dispatch, handlers, UART I/O
├── STM32F446RETX_FLASH.ld            # FLASH: 0x08000000, 512K (whole chip view)
└── STM32_Programmer_V1.py            # Host-side Python tool for sending commands

User_app/                             # The application flashed by the bootloader
├── Core/
│   └── Src/
│       └── main.c                    # Currently just prints "Hello from user app"
└── STM32F446RETX_FLASH.ld            # FLASH: 0x08008000, 480K (starts after bootloader)
```

## Known limitations / TODO

- Most command handlers (`bootloader_handle_*`) are currently empty stubs only `BL_GET_VER` is fully implemented end-to-end.
- `BL_MEM_READ` and `BL_OTP_READ` are not yet supported by the host script.
- No timeout/retry logic on the firmware side for malformed host packets beyond the CRC check.
- `BL_FLASH_ERASE` / mass erase must be scoped to sectors 2–7 only once implemented, to avoid erasing the bootloader itself (see sector table above).

