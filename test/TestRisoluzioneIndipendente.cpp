// Test di regressione: una configurazione del genetico non deve
// modificare il numero di campioni con cui viene valutata la pista.
#include "AlgoritmoGenetico.hpp"
#include "InterpolatorePista.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "Gomma.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static std::vector<Punto2D> leggi(const std::string& nome) {
    std::ifstream f(nome);
    if (!f) throw std::runtime_error("Non posso leggere " + nome);
    std::string linea;
    std::getline(f, linea);
    std::vector<Punto2D> out;
    while (std::getline(f, linea)) {
        if (linea.empty()) continue;
        const auto p=linea.find(',');
        if (p!=std::string::npos) out.emplace_back(std::stod(linea.substr(0,p)),std::stod(linea.substr(p+1)));
    }
    if (out.size()>3 && out.front().distanza(out.back())<0.5) out.pop_back();
    return out;
}

static std::vector<SezionePista> costruisci(const std::vector<Punto2D>& centro, int n) {
    std::vector<SezionePista> s;
    for (const auto& c : InterpolatorePista::campionaChiuso(centro,n)) {
        const double x = -6.0*c.tangente.getY(), y=6.0*c.tangente.getX();
        s.emplace_back(Punto2D(c.punto.getX()+x,c.punto.getY()+y),
                       Punto2D(c.punto.getX()-x,c.punto.getY()-y));
    }
    return s;
}

static Veicolo veicolo() {
    const std::array<Gomma,4> gomme{{Gomma(0,TipoGomma::SOFT,.95),Gomma(1,TipoGomma::SOFT,.95),
                                     Gomma(2,TipoGomma::SOFT,.95),Gomma(3,TipoGomma::SOFT,.95)}};
    return Veicolo(798.,650000.,10.,15.,.9,1.5,315./3.6,18.,gomme);
}

int main(int argc, char** argv) {
    try {
        if (argc!=3) throw std::invalid_argument("Uso: test_risoluzione spa.csv monza.csv");
        const auto autoVeicolo=veicolo();
        for (int k=1;k<argc;++k) {
            const auto sezioni=costruisci(leggi(argv[k]),600);
            // Cambia solo il numero dei geni; traiettoria centrale geometrica IDENTICA.
            const auto t80=Traiettoria(std::vector<double>(80,.5)).ricampionaPeriodica(600);
            const auto t300=Traiettoria(std::vector<double>(300,.5)).ricampionaPeriodica(600);
            for(int j=0;j<600;++j) {
                assert(std::abs(t80.getPosizioneLaterale(j)-.5)<1e-12);
                assert(std::abs(t80.getPosizioneLaterale(j)-t300.getPosizioneLaterale(j))<1e-12);
            }
            ValutatoreTraiettoria v;
            const double primo=v.valutaTraiettoria(t80,sezioni,autoVeicolo);
            const double secondo=v.valutaTraiettoria(t300,sezioni,autoVeicolo);
            if (!(std::isfinite(primo) && std::abs(primo-secondo)<1e-8))
                throw std::runtime_error("La valutazione dipende dal numero di geni");
            std::cout << argv[k] << " | 80/300 geni, 600 campioni fissi: " << primo << " / " << secondo << " s\n";
        }
        // Anche per una traiettoria non costante il campionamento resta
        // periodico, finito e sempre nell'intervallo [0,1].
        std::vector<double> geni(30);
        for (int i=0;i<30;++i) geni[i]=.5+.4*std::sin(2.*3.141592653589793*i/30.);
        const auto nuova=Traiettoria(geni).ricampionaPeriodica(1200);
        assert(nuova.getNumeroPosizioni()==1200);
        for (double g:nuova.getPosizioniLaterali()) assert(std::isfinite(g) && g>=0 && g<=1);
        // Determinismo, preservazione del miglior individuo iniziale,
        // e compatibilita' delle API preesistenti.
        const auto sezioni=costruisci(leggi(argv[1]),160);
        AlgoritmoGenetico a(8,3,.8,.05,.1,3,2,2026);
        AlgoritmoGenetico b(8,3,.8,.05,.1,3,2,2026);
        const auto ra=a.ottimizzaConDettagli(sezioni,autoVeicolo,24);
        const auto rb=b.ottimizzaConDettagli(sezioni,autoVeicolo,24);
        assert(ra.miglioreIniziale.getNumeroPosizioni()==24);
        assert(ra.miglioreFinale.getNumeroPosizioni()==24);
        assert(std::isfinite(ra.tempoIniziale) && std::isfinite(ra.tempoFinale));
        assert(ra.tempoFinale <= ra.tempoIniziale + 1e-9);
        assert(std::abs(ra.tempoIniziale-rb.tempoIniziale)<1e-10);
        assert(std::abs(ra.tempoFinale-rb.tempoFinale)<1e-10);
        std::cout << "Genetico: iniziale " << ra.tempoIniziale << " s, finale " << ra.tempoFinale << " s\n";
        std::cout << "OK: risoluzione indipendente, riproducibilita' e confronto iniziale/finale\n";
    } catch (const std::exception& e) {
        std::cerr << "ERRORE: " << e.what() << "\n";
        return 1;
    }
}
