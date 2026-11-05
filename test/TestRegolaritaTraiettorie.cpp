#include "GeneraTraiettorie.hpp"
#include "AlgoritmoGenetico.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "SezionePista.hpp"
#include "Gomma.hpp"
#include "Veicolo.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

namespace {
// Misura oscillazioni laterali, compreso il raccordo fra ultimo e primo gene.
double massimoSaltoCiclico(const Traiettoria& t) {
    double massimo = 0.0;
    const auto& valori = t.getPosizioniLaterali();
    for (size_t i = 0; i < valori.size(); ++i) {
        massimo = max(massimo, abs(valori[i] - valori[(i + 1) % valori.size()]));
    }
    return massimo;
}

vector<SezionePista> creaPistaCircolare(int n) {
    vector<SezionePista> s;
    s.reserve(n);
    constexpr double pi = 3.141592653589793;
    for (int i = 0; i < n; ++i) {
        const double a = 2.0 * pi * i / n;
        s.emplace_back(Punto2D(99.0*cos(a),99.0*sin(a)),
                       Punto2D(111.0*cos(a),111.0*sin(a)));
    }
    return s;
}
} // namespace

void eseguiTestRegolaritaTraiettorie() {
    cout << "\n=== TEST TRAIETTORIE REGOLARI ===\n";
    for (const int n : {1, 2, 3, 8, 80, 300}) {
        GeneratoreTraiettorie a(2026), b(2026);
        auto t = a.generaTraiettoriaCasuale(n);
        auto t2 = b.generaTraiettoriaCasuale(n);
        assert(t.getNumeroPosizioni() == n);
        assert(t.getPosizioniLaterali() == t2.getPosizioniLaterali());
        for (double p : t.getPosizioniLaterali()) {
            assert(isfinite(p) && p >= 0.0 && p <= 1.0);
        }
        if (n >= 80) {
            const double salto = massimoSaltoCiclico(t);
            cout << "  N=" << n << ", massimo salto ciclico=" << salto << "\n";
            assert(salto < 0.16);
        }
    }

    const auto sezioni = creaPistaCircolare(80);
    const array<Gomma, 4> gomme = {Gomma(0, TipoGomma::SOFT, 1.0),
                                  Gomma(1, TipoGomma::SOFT, 1.0),
                                  Gomma(2, TipoGomma::SOFT, 1.0),
                                  Gomma(3, TipoGomma::SOFT, 1.0)};
    const Veicolo macchina(1000.0, 400000.0, 7.0, 10.0, 0.45,
                           2.0, 80.0, 18.0, gomme);
    ValutatoreTraiettoria valutatore;
    AlgoritmoGenetico gaA(30, 12, 0.85, 0.12, 0.15, 3, 3, 42);
    AlgoritmoGenetico gaB(30, 12, 0.85, 0.12, 0.15, 3, 3, 42);
    const auto tA = gaA.ottimizza(sezioni, macchina);
    const auto tB = gaB.ottimizza(sezioni, macchina);
    assert(tA.getPosizioniLaterali() == tB.getPosizioniLaterali());
    assert(tA.getNumeroPosizioni() == 80);
    for (double p : tA.getPosizioniLaterali()) assert(isfinite(p) && p >= 0.0 && p <= 1.0);
    assert(isfinite(valutatore.valutaTraiettoria(tA, sezioni, macchina)));
    cout << "[OK] Generazione periodica, range, seed e GA verificati.\n";
}
