// APEX LAB - Test di controllo: convergenza numerica su una circonferenza.
// Il modello fisico e il valutatore sono quelli originali del progetto.
// Compilare dalla root come indicato nel README_CORREZIONE_C2.md.
#include "ValutatoreTraiettoria.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
constexpr double raggio = 100.0;
constexpr double larghezza = 12.0;

Veicolo veicoloPredefinito() {
    const std::array<Gomma, 4> gomme{
        Gomma(0, TipoGomma::SOFT, 0.95),
        Gomma(1, TipoGomma::SOFT, 0.95),
        Gomma(2, TipoGomma::SOFT, 0.95),
        Gomma(3, TipoGomma::SOFT, 0.95)
    };
    return Veicolo(798.0, 650000.0, 10.0, 15.0, 0.9, 1.5,
                   315.0 / 3.6, 18.0, gomme);
}

double valutaCerchio(int n, const Veicolo& veicolo,
                    const ValutatoreTraiettoria& valutatore) {
    const double pi = std::acos(-1.0);
    std::vector<SezionePista> sezioni;
    sezioni.reserve(n);
    for (int i = 0; i < n; ++i) {
        const double t = 2.0*pi*i/n;
        const double nx=std::cos(t), ny=std::sin(t);
        sezioni.emplace_back(Punto2D((raggio+larghezza/2)*nx, (raggio+larghezza/2)*ny),
                             Punto2D((raggio-larghezza/2)*nx, (raggio-larghezza/2)*ny));
    }
    Traiettoria centrale(std::vector<double>(n,0.5));
    const auto curvature=valutatore.profiloCurvaturaPerGrafici(centrale,sezioni);
    const auto [minimo,massimo]=std::minmax_element(curvature.begin(),curvature.end());
    if (std::abs(*minimo - 1.0/raggio) > 1e-6 ||
        std::abs(*massimo - 1.0/raggio) > 1e-6)
        throw std::runtime_error("Curvatura del cerchio non coerente con 1/R");
    return valutatore.valutaTraiettoria(centrale,sezioni,veicolo);
}
} // namespace

int main() {
    try {
        const auto veicolo=veicoloPredefinito();
        const ValutatoreTraiettoria valutatore;
        double tempoPrecedente=0;
        std::cout << "CERCHIO r=100 m - traiettoria centrale - senza genetico\n";
        for (int n : {80,160,300,600,1200,2400}) {
            const double tempo=valutaCerchio(n,veicolo,valutatore);
            std::cout << n << " sezioni: " << std::fixed << std::setprecision(6)
                      << tempo << " s\n";
            if (n == 2400 && tempoPrecedente > 0 &&
                std::abs(tempo-tempoPrecedente)/tempo > 0.001)
                throw std::runtime_error("Mancata convergenza del tempo sul cerchio");
            tempoPrecedente=tempo;
        }
        std::cout << "OK: valutatore coerente sulla geometria circolare\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERRORE: " << e.what() << '\n';
        return 1;
    }
}
