#!/usr/bin/env python3
"""Riepilogo multi-seed e grafici SVG senza librerie esterne.

Esempio: python3 tools/analizza_esperimenti_v5.py /tmp/apex_esperimenti_v5.csv /tmp/apex_report_v5
"""
import csv
import math
import statistics
import sys
from collections import defaultdict
from pathlib import Path
from xml.sax.saxutils import escape


def main():
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} RISULTATI.csv CARTELLA_REPORT")
        return 2
    with open(sys.argv[1], newline="", encoding="utf-8") as src:
        rows = list(csv.DictReader(src))
    if not rows:
        raise SystemExit("CSV vuoto")
    out = Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
    groups = defaultdict(list)
    for row in rows:
        groups[(row['pista'], row['esperimento'], row['valore'])].append(row)
    summary = []
    for key, rr in sorted(groups.items()):
        final = [float(x['finale_s']) for x in rr]
        dt = [float(x['tempo_esecuzione_s']) for x in rr]
        clear = [float(x['margine_minimo_m']) for x in rr]
        diffs = [float(x['guadagno_s']) for x in rr]
        if not all(math.isfinite(x) for x in final+dt+clear+diffs):
            raise SystemExit("CSV contiene valori non finiti")
        summary.append(dict(pista=key[0], parametro=key[1], valore=key[2], semi=len(rr),
                            tempo_medio_s=statistics.mean(final),
                            tempo_std_s=statistics.stdev(final) if len(rr)>1 else 0.0,
                            miglioramento_medio_s=statistics.mean(diffs),
                            tempo_esecuzione_medio_s=statistics.mean(dt),
                            margine_minimo_osservato_m=min(clear)))
    dest = out/'riepilogo_esperimenti.csv'
    with dest.open('w',newline='',encoding='utf-8') as f:
        wr=csv.DictWriter(f,fieldnames=list(summary[0]));wr.writeheader();wr.writerows(summary)
    print(f"Riepilogo: {dest}")
    for pista in ('spa','monza'):
        values=[r for r in summary if r['pista']==pista]
        if not values: continue
        high=max(r['tempo_medio_s'] for r in values)
        low=min(r['tempo_medio_s'] for r in values)
        span=max(.1, high-low)
        height=160+44*len(values)
        lines=[f'<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="{height}" viewBox="0 0 1000 {height}">',
               '<rect width="100%" height="100%" fill="#0B1726"/>',
               f'<text x="40" y="42" font-family="sans-serif" font-size="24" fill="#E9F2FF">{pista.upper()} · Media tempi su seed</text>',
               f'<text x="40" y="70" font-family="sans-serif" font-size="14" fill="#91ADC5">Valori più bassi sono migliori. Le differenze possono dipendere dai seed.</text>']
        for i,row in enumerate(values):
            y=105+44*i
            lab=f"{row['parametro']}={row['valore']}"
            bar=round(455 * (row['tempo_medio_s']-low) / span)
            lines += [f'<text x="40" y="{y+15}" font-family="monospace" font-size="14" fill="#D8E4F4">{escape(lab)}</text>',
                      f'<rect x="280" y="{y}" width="{max(4,bar)}" height="22" fill="#25CFC7" rx="3"/>',
                      f'<text x="{300+max(4,bar)}" y="{y+16}" font-family="monospace" font-size="14" fill="#FFB062">{row["tempo_medio_s"]:.3f} s ± {row["tempo_std_s"]:.3f}</text>']
        lines.append('</svg>')
        svg=out/f'esperimenti_{pista}.svg';svg.write_text('\n'.join(lines),encoding='utf-8')
        print(f"Grafico: {svg}")
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
