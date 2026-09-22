"""Convert 480 x 320 artwork to the panel's native RGB888 layout."""

import argparse
from pathlib import Path

from PIL import Image

LANDSCAPE_SIZE = (480, 320)
NATIVE_SIZE = (320, 480)
IMAGE_BYTES = 320 * 480 * 3


def convert(source: Path, destination: Path) -> None:
    source = Path(source)
    destination = Path(destination)
    if source.resolve() == destination.resolve():
        raise ValueError("Source and output must be different files")

    with Image.open(source) as image:
        # Reject resizing so text and QR modules keep their original pixels
        if image.size != LANDSCAPE_SIZE:
            raise ValueError(f"Expected 480 x 320 artwork, got {image.size}")
        if getattr(image, "n_frames", 1) != 1:
            raise ValueError("Use a single-frame image")

        # Flatten transparent artwork onto white before removing alpha
        rgba = image.convert("RGBA")
        background = Image.new("RGBA", image.size, (255, 255, 255, 255))
        rgb = Image.alpha_composite(background, rgba).convert("RGB")
        native = rgb.transpose(Image.Transpose.ROTATE_90)
        pixels = native.tobytes()

    if native.size != NATIVE_SIZE or len(pixels) != IMAGE_BYTES:
        raise ValueError("Unexpected converted image layout")

    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(pixels)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        convert(args.source, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Conversion failed: {error}\n")
    print(f"Wrote {args.output}: {IMAGE_BYTES} bytes")


if __name__ == "__main__":
    main()
