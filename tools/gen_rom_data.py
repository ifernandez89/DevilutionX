import base64
import os

base_dir = os.path.dirname(os.path.abspath(__file__))
pcvivaz_dir = os.path.join(base_dir, "..", "PCVIVAZ")

with open(os.path.join(pcvivaz_dir, "pcvivaz-unif.nes"), "rb") as f:
    unif_b64 = base64.b64encode(f.read()).decode("ascii")

with open(os.path.join(pcvivaz_dir, "pcvivaz.nes"), "rb") as f:
    nes_b64 = base64.b64encode(f.read()).decode("ascii")

with open(os.path.join(pcvivaz_dir, "pcvivaz-unif.srm"), "rb") as f:
    srm_b64 = base64.b64encode(f.read()).decode("ascii")

out_file = os.path.join(pcvivaz_dir, "rom_data.js")
with open(out_file, "w", encoding="utf-8") as f:
    f.write("// PC Vivaz embedded ROM & SRAM data (Offline / Standalone)\n")
    f.write(f'window.PCVIVAZ_UNIF_B64 = "{unif_b64}";\n')
    f.write(f'window.PCVIVAZ_NES_B64 = "{nes_b64}";\n')
    f.write(f'window.PCVIVAZ_SRM_B64 = "{srm_b64}";\n')

print(f"Generated {out_file} successfully! Size: {os.path.getsize(out_file)} bytes")
