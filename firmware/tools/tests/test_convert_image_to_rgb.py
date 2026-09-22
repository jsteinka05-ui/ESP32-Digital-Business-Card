import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from convert_image_to_rgb import IMAGE_BYTES, convert


class ConversionTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.source = Path(self.directory.name) / "source.png"
        self.output = Path(self.directory.name) / "output.rgb"

    def test_rotation_and_channel_order(self):
        image = Image.new("RGB", (480, 320))
        corners = [((0, 0), (255, 1, 2)), ((479, 0), (3, 255, 4)),
                   ((0, 319), (5, 6, 255)), ((479, 319), (255, 254, 253))]
        for position, color in corners:
            image.putpixel(position, color)
        image.save(self.source)
        convert(self.source, self.output)
        raw = self.output.read_bytes()
        self.assertEqual(len(raw), IMAGE_BYTES)
        # Source top-right becomes native top-left after the CCW rotation
        for position, color in [((0, 0), corners[1][1]), ((319, 0), corners[3][1]),
                                ((0, 479), corners[0][1]), ((319, 479), corners[2][1])]:
            offset = (position[1] * 320 + position[0]) * 3
            self.assertEqual(raw[offset:offset + 3], bytes(color))

    def test_invalid_size_preserves_existing_output(self):
        Image.new("RGB", (320, 480)).save(self.source)
        self.output.write_bytes(b"existing")
        with self.assertRaises(ValueError):
            convert(self.source, self.output)
        self.assertEqual(self.output.read_bytes(), b"existing")

    def test_transparency_uses_white(self):
        Image.new("RGBA", (480, 320), (0, 0, 0, 0)).save(self.source)
        convert(self.source, self.output)
        self.assertEqual(self.output.read_bytes(), bytes([255]) * IMAGE_BYTES)

    def test_source_cannot_be_overwritten(self):
        Image.new("RGB", (480, 320)).save(self.source)
        original = self.source.read_bytes()
        with self.assertRaises(ValueError):
            convert(self.source, self.source)
        self.assertEqual(self.source.read_bytes(), original)

    def test_repository_assets_are_reproducible(self):
        images = Path(__file__).resolve().parents[2] / "images"
        for name in ("business_card", "linkedin_qr"):
            with self.subTest(name=name):
                convert(images / f"{name}.png", self.output)
                self.assertEqual(self.output.read_bytes(), (images / f"{name}.rgb").read_bytes())

    def test_multiframe_input_is_rejected(self):
        source = self.source.with_suffix(".gif")
        first = Image.new("RGB", (480, 320), "white")
        second = Image.new("RGB", (480, 320), "black")
        first.save(source, save_all=True, append_images=[second])
        with self.assertRaises(ValueError):
            convert(source, self.output)
        self.assertFalse(self.output.exists())

    def test_qr_layout_preserves_quiet_border(self):
        images = Path(__file__).resolve().parents[2] / "images"
        with Image.open(images / "linkedin_qr.png") as source:
            panel = source.convert("RGB").crop((105, 25, 375, 295))
        # Recoloring expects black modules inside a white four-module border
        self.assertEqual(set(panel.get_flattened_data()), {(0, 0, 0), (255, 255, 255)})
        for bounds in [(0, 0, 270, 24), (0, 246, 270, 270),
                       (0, 0, 24, 270), (246, 0, 270, 270)]:
            border = panel.crop(bounds)
            self.assertEqual(border.tobytes(), bytes([255]) * border.width * border.height * 3)


if __name__ == "__main__":
    unittest.main()
