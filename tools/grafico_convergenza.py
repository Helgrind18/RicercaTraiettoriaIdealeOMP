#!/usr/bin/env python3
"""Disegna SVG della convergenza numerica usando solo la libreria standard.
Uso: python3 tools/grafico_convergenza.py /tmp/apex_validazione.csv /tmp/apex_grafici
"""
import csv
import math
import sys
from collections import defaultdict
from pathlib import Path
from xml.sax.saxutils import escape


def main():
    if len(sys.argv) != 3:
        raise SystemExit("Uso: python3 tools/grafico_convergenza.py RISULTATI.csv CARTELLA_OUTPUT")
    input_csv, dest = Path(sys.argv[1]), Path(sys.argv[2])
    if not input_csv.is_file():
        raise SystemExit(f"CSV non trovato: {input_csv}")
    dest.mkdir(parents=True, exist_ok=True)
    gruppi = defaultdict(lambda: defaultdict(list))
    with input_csv.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            try:
                campioni, tempo = int(row["campioni"]), float(row["tempo_s"])
                if campioni > 0 and math.isfinite(tempo):
                    gruppi[row["circuito"]][row["soluzione"]].append((campioni, tempo))
            except (KeyError, ValueError):
                continue
    if not gruppi:
        raise SystemExit("Nessuna riga valida in CSV")

    for circuito, curve in gruppi.items():
        dati = [pt for pts in curve.values() for pt in pts]
        nmin, nmax = min(n for n, _ in dati), max(n for n, _ in dati)
        tmin, tmax = min(t for _, t in dati), max(t for _, t in dati)
        pad = max(0.25, (tmax - tmin) * 0.12)
        tmin, tmax = tmin - pad, tmax + pad
        width, height = 980, 530
        l, r, top, bottom = 105, 45, 60, 92
        def x(n):
            return l + (math.log2(n) - math.log2(nmin)) / max(1e-8, math.log2(nmax) - math.log2(nmin)) * (width-l-r)
        def y(t):
            return height-bottom - (t-tmin) / (tmax-tmin) * (height-top-bottom)
        out = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 980 530">',
               '<rect width="980" height="530" fill="#0c1726"/>',
               f'<text x="105" y="34" fill="white" font-family="sans-serif" font-size="20">APEX LAB — {escape(circuito.upper())} | convergenza numerica</text>']
        for i in range(6):
            t=tmin+(tmax-tmin)*i/5
            py=y(t)
            out += [f'<line x1="{l}" x2="{width-r}" y1="{py:.2f}" y2="{py:.2f}" stroke="#273d55"/>',
                    f'<text x="{l-12}" y="{py+5:.2f}" text-anchor="end" fill="#c6d8ee" font-size="13" font-family="monospace">{t:.2f}</text>']
        for n in sorted(set(n for n,_ in dati)):
            px=x(n)
            out += [f'<line x1="{px:.2f}" x2="{px:.2f}" y1="{top}" y2="{height-bottom}" stroke="#23344a"/>',
                    f'<text x="{px:.2f}" y="{height-bottom+28}" text-anchor="middle" fill="#c6d8ee" font-size="13">{n}</text>']
        for nome, colore in [('iniziale','#f7a75e'), ('ottimizzata','#24d1c7')]:
            pts=sorted(curve.get(nome, []))
            if not pts: continue
            path=' '.join(f'{"M" if i==0 else "L"}{x(n):.2f},{y(t):.2f}' for i,(n,t) in enumerate(pts))
            out.append(f'<path d="{path}" fill="none" stroke="{colore}" stroke-width="3"/>')
            for n,t in pts:
                out.append(f'<circle cx="{x(n):.2f}" cy="{y(t):.2f}" r="4" fill="{colore}"/>')
            label = 'Iniziale' if nome=='iniziale' else 'Ottimizzata'
            lx=270 if nome=='iniziale' else 510
            out += [f'<line x1="{lx}" x2="{lx+28}" y1="{height-32}" y2="{height-32}" stroke="{colore}" stroke-width="4"/>',
                    f'<text x="{lx+38}" y="{height-27}" fill="#e1edff" font-family="sans-serif" font-size="14">{label}</text>']
        out += [f'<text x="{(width+l-r)/2:.0f}" y="{height-53}" text-anchor="middle" fill="#c6d8ee" font-size="15">Campioni per giro (scala log₂)</text>',
                '<text transform="translate(24 264) rotate(-90)" fill="#c6d8ee" font-size="15" text-anchor="middle">Tempo sul giro (s)</text>',
                '</svg>']
        path = dest / f'convergenza_{circuito}.svg'
        path.write_text('\n'.join(out), encoding='utf-8')
        print(f"Creato: {path}")


if __name__ == '__main__':
    main()
