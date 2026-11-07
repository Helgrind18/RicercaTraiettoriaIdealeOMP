// APEX LAB - Diagnostica di convergenza del tempo sul giro
// Test non invasivo: non modifica algoritmo genetico o modello fisico.
// Esempio dalla root del progetto:
// g++ -std=c++20 -O2 -Iinclude test/TestConvergenzaTraiettoria.cpp \
//     src/Punto2D.cpp src/InterpolatorePista.cpp src/SezionePista.cpp \
//     src/Traiettoria.cpp src/Gomma.cpp src/Veicolo.cpp \
//     src/ValutatoreTraiettoria.cpp -o /tmp/test_apex_convergenza
// /tmp/test_apex_convergenza gui/data/spa.csv gui/data/monza.csv

#include "InterpolatorePista.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::vector<Punto2D> caricaCsv(const std::string& percorso) {
    std::ifstream file(percorso);
    if (!file) throw std::runtime_error("Impossibile aprire: " + percorso);
    std::string riga;
    std::getline(file, riga); // intestazione x_m,y_m
    std::vector<Punto2D> punti;
    while (std::getline(file, riga)) {
        if (riga.empty()) continue;
        const auto separatore = riga.find(',');
        if (separatore == std::string::npos) continue;
        const double x = std::stod(riga.substr(0, separatore));
        const double y = std::stod(riga.substr(separatore + 1));
        if (!std::isfinite(x) || !std::isfinite(y))
            throw std::runtime_error("Coordinata non valida nel CSV");
        const Punto2D p(x, y);
        if (punti.empty() || punti.back().distanza(p) > 1e-4)
            punti.push_back(p);
    }
    if (punti.size() > 3 && punti.front().distanza(punti.back()) < 0.5)
        punti.pop_back();
    if (punti.size() < 3) throw std::runtime_error("Troppi pochi punti nel CSV");
    return punti;
}

Veicolo creaVeicoloGuiPredefinito() {
    const std::array<Gomma, 4> gomme{
        Gomma(0, TipoGomma::SOFT, 0.95),
        Gomma(1, TipoGomma::SOFT, 0.95),
        Gomma(2, TipoGomma::SOFT, 0.95),
        Gomma(3, TipoGomma::SOFT, 0.95)
    };
    // Gli stessi valori predefiniti della GUI (MotoreApex.hpp):
    // massa kg, potenza W, accel, freno, Cd, area m2, velocita m/s,
    // accel. laterale, gomme.
    return Veicolo(798.0, 650000.0, 10.0, 15.0, 0.9, 1.5,
                   315.0 / 3.6, 18.0, gomme);
}

struct Risultato {
    int sezioni = 0;
    double lunghezza = 0.0;
    double curvaturaMassima = 0.0;
    double tempo = 0.0;
};

Risultato misura(const std::vector<Punto2D>& centroOriginale, int n,
                 const Veicolo& veicolo) {
    const auto campioni = InterpolatorePista::campionaChiuso(centroOriginale, n);
    std::vector<SezionePista> sezioni;
    sezioni.reserve(campioni.size());
    for (const auto& c : campioni) {
        const double x = c.punto.getX();
        const double y = c.punto.getY();
        const double nx = -c.tangente.getY();
        const double ny = c.tangente.getX();
        // Stesso offset di 6 m per lato usato dalla GUI.
        sezioni.emplace_back(Punto2D(x + 6.0*nx, y + 6.0*ny),
                             Punto2D(x - 6.0*nx, y - 6.0*ny));
    }
    // Sempre la stessa traiettoria concettuale: asse centrale della stessa
    // spline continua di partenza, campionato a differenti risoluzioni.
    const Traiettoria traiettoria(std::vector<double>(n, 0.5));
    const auto punti = traiettoria.calcolaPunti(sezioni);
    double lunghezza = 0.0;
    for (int i = 0; i < n; ++i)
        lunghezza += punti[i].distanza(punti[(i+1)%n]);

    const ValutatoreTraiettoria valutatore;
    const auto curvature = valutatore.profiloCurvaturaPerGrafici(traiettoria, sezioni);
    const double curvaturaMassima = *std::max_element(curvature.begin(), curvature.end());
    const double tempo = valutatore.valutaTraiettoria(traiettoria, sezioni, veicolo);
    if (!std::isfinite(tempo) || tempo <= 0 || !std::isfinite(curvaturaMassima))
        throw std::runtime_error("Il valutatore ha prodotto un risultato non valido");
    return {n, lunghezza, curvaturaMassima, tempo};
}

void esegui(const std::string& nome, const std::string& percorso) {
    const auto centro = caricaCsv(percorso);
    const auto veicolo = creaVeicoloGuiPredefinito();
    std::cout << "\n=== " << nome << " | traiettoria CENTRALE (senza genetico) ===\n";
    std::cout << "sezioni   lunghezza_m   curv_max_1/m     tempo_s    variazione_%\n";
    double tempoPrecedente = std::numeric_limits<double>::quiet_NaN();
    for (const int n : {80,160,300,600,1200,2400}) {
        const auto r = misura(centro, n, veicolo);
        const double delta = 100.0 * (r.tempo-tempoPrecedente) / tempoPrecedente;
        std::cout << std::fixed << std::setprecision(3)
                  << std::setw(7) << r.sezioni << "   "
                  << std::setw(11) << r.lunghezza << "   "
                  << std::setw(13) << std::setprecision(6) << r.curvaturaMassima << "   "
                  << std::setw(9) << std::setprecision(3) << r.tempo << "   ";
        if (std::isfinite(delta))
            std::cout << std::showpos << std::setprecision(2) << delta << "%" << std::noshowpos;
        else
            std::cout << "---";
        std::cout << '\n';
        tempoPrecedente = r.tempo;
    }
}
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Uso: test_apex_convergenza gui/data/spa.csv gui/data/monza.csv\n";
        return 2;
    }
    try {
        esegui("SPA", argv[1]);
        esegui("MONZA", argv[2]);
        std::cout << "\nOK: test diagnostico completato (non implica convergenza)\n";
    } catch (const std::exception& e) {
        std::cerr << "ERRORE: " << e.what() << '\n';
        return 1;
    }
}
