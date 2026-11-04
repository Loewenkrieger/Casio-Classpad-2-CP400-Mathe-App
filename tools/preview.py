"""Convert the host-rendered framebuffers to PNG. Requires Pillow."""
from PIL import Image
from common import ROOT

folder = ROOT / "docs/preview"
for path in folder.glob("*.ppm"):
    with Image.open(path) as picture:
        picture.save(path.with_suffix(".png"))
selected = ["04-input", "13-help-direction", "15-help-plane", "16-help-lgs"]
out = Image.new("RGB", (320 * len(selected), 528), "white")
for i, name in enumerate(selected):
    with Image.open(folder / (name + ".png")) as picture:
        out.paste(picture, (320 * i, 0))
out.save(folder / "overview.png")
print(folder / "overview.png")
