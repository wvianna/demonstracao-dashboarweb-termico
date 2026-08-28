#!/usr/bin/env python3
"""Gera src/web/dashboard.h a partir de src/web/dashboard.html, embutindo as
fontes woff2 (tools/fonts) como base64. Usado como extra_scripts pre-build
(reproduzivel). Se as fontes nao existirem, gera sem embed (fallback de sistema).

Uso:
    python3 tools/gen_dashboard.py
"""
import base64
import os
import pathlib

def _root() -> pathlib.Path:
    # SCons (extra_scripts do PlatformIO) nao define __file__ -> usa o cwd
    try:
        return pathlib.Path(__file__).resolve().parent.parent
    except NameError:
        return pathlib.Path(os.getcwd()).resolve()


ROOT = _root()
HTML = ROOT / "src" / "web" / "dashboard.html"
OUT = ROOT / "src" / "web" / "dashboard.h"
FONTS = {
    "__FONT_CHAKRA_B64__": ROOT / "tools" / "fonts" / "chakra-petch-600-latin.woff2",
    "__FONT_PLEX_B64__": ROOT / "tools" / "fonts" / "ibm-plex-sans-400-500-latin.woff2",
}


def main() -> int:
    html = HTML.read_text(encoding="utf-8")
    for placeholder, path in FONTS.items():
        if path.exists():
            data = path.read_bytes()
            html = html.replace(placeholder, base64.b64encode(data).decode("ascii"))
            print(f"[gen] {path.name}: {len(data)} bytes embutidos")
        else:
            html = html.replace(placeholder, "")
            print(f"[gen] AVISO: {path.name} ausente -> fallback de fonte do sistema")

    header = (
        "#pragma once\n"
        "// GERADO por tools/gen_dashboard.py - nao edite manualmente.\n"
        "// Fonte: src/web/dashboard.html + tools/fonts/*.woff2\n"
        '#include <Arduino.h>\n'
        "static const char DASHBOARD_HTML[] PROGMEM = R\"rawliteral("
        + html +
        ")rawliteral\";\n"
    )
    OUT.write_text(header, encoding="utf-8")
    print(f"[gen] {OUT} gerado ({len(header)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
