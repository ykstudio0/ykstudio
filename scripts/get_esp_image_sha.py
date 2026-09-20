# -------------------------------------------------------------
#  File : get_esp_image_sha.py
#  Author : JongOh Kim + ChatGPT
#  Date : 2026-09-20
#  Project : SVEMS
#  Version : 0.8.3
#  Description : get_esp_image_sha
# -------------------------------------------------------------

import hashlib
import sys
from pathlib import Path


def main():
    if len(sys.argv) != 2:
        print(
            "Usage: python get_esp_image_sha.py firmware.bin",
            file=sys.stderr
        )
        sys.exit(1)

    firmware_path = Path(
        sys.argv[1]
    )

    if not firmware_path.is_file():
        print(
            f"Firmware not found: {firmware_path}",
            file=sys.stderr
        )
        sys.exit(1)

    data = firmware_path.read_bytes()

    if len(data) <= 32:
        print(
            "Firmware file is too small.",
            file=sys.stderr
        )
        sys.exit(1)

    image_data = data[:-32]
    appended_hash = data[-32:]

    calculated_hash = hashlib.sha256(
        image_data
    ).digest()

    if calculated_hash != appended_hash:
        print(
            "ESP32 image SHA256 verification failed.",
            file=sys.stderr
        )
        sys.exit(1)

    print(
        appended_hash.hex()
    )


if __name__ == "__main__":
    main()