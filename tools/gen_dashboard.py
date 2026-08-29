#!/usr/bin/env python3
"""Gera src/web/dashboard.h a partir de src/web/dashboard.html, embutindo as
fontes woff2 (tools/fonts) como arrays PROGMEM binarios servidos por rotas
separadas (/fonts/*.woff2), em vez de base64 inline no HTML (NFR-TIM-005:
mantem a resposta de "/" pequena e o carregamento fluido). Se as fontes nao
existirem, o dashboard usa fallback de fonte do sistema (arrays vazios).
Usado como extra_scripts pre-build (reproduzivel).

Uso:
    python3 tools/gen_dashboard.py
"""
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
    "FONT_CHAKRA_WOFF2": ROOT / "tools" / "fonts" / "chakra-petch-600-latin.woff2",
    "FONT_PLEX_WOFF2": ROOT / "tools" / "fonts" / "ibm-plex-sans-400-500-latin.woff2",
}


def _byte_array(name: str, data: bytes) -> str:
    body = ",".join(str(b) for b in data)
    return (
        f"static const uint8_t {name}[] PROGMEM = {{{body}}};\n"
        f"static const size_t {name}_LEN = {len(data)};\n"
    )


def main() -> int:
    html = HTML.read_text(encoding="utf-8")

    font_decls = []
    for name, path in FONTS.items():
        if path.exists():
            data = path.read_bytes()
            print(f"[gen] {path.name}: {len(data)} bytes (rota /fonts/{path.name})")
        else:
            data = b""
            print(f"[gen] AVISO: {path.name} ausente -> fallback de fonte do sistema")
        font_decls.append(_byte_array(name, data))

    header = (
        "#pragma once\n"
        "// GERADO por tools/gen_dashboard.py - nao edite manualmente.\n"
        "// Fonte: src/web/dashboard.html + tools/fonts/*.woff2\n"
        '#include <Arduino.h>\n'
        + "".join(font_decls)
        + "static const char DASHBOARD_HTML[] PROGMEM = R\"rawliteral("
        + html +
        ")rawliteral\";\n"
    )
    OUT.write_text(header, encoding="utf-8")
    print(f"[gen] {OUT} gerado ({len(header)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
