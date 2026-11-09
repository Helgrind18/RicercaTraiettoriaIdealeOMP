#include "AlgoritmoGenetico.hpp"
#include "ValidatoreGeometrico.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

// Test sintetico: pista circolare regolare. Verifica vincoli nel FITNESS,
// assenza di peggioramento rispetto al migliore iniziale e riproducibilita'.
int main() {
    try {
        constexpr double pi=3.14159265358979323846;
        constexpr int n=480;
        std::vector<SezionePista> sezioni;
        sezioni.reserve(n);
        for(int i=0; i<n; ++i) {
            const double t=2*pi*i/n;
            const double x=std::cos(t), y=std::sin(t);
            sezioni.emplace_back(Punto2D(106*x,106*y), Punto2D(94*x,94*y));
        }
        const std::array<Gomma,4> gomme{{
            Gomma(0,TipoGomma::SOFT,.95),Gomma(1,TipoGomma::SOFT,.95),
            Gomma(2,TipoGomma::SOFT,.95),Gomma(3,TipoGomma::SOFT,.95)}};
        const Veicolo veicolo(798.,650000.,10.,15.,.9,1.5,315./3.6,18.,gomme);
        auto prova=[&]() {
            AlgoritmoGenetico ga(20,8,.8,.05,.1,3,2,2026);
            ga.impostaMargineMinimo(1.0);
            return ga.ottimizzaConDettagli(sezioni,veicolo,40);
        };
        const auto a=prova(), b=prova();
        if(a.storico.size()!=8 || b.storico.size()!=8 ||
            std::abs(a.tempoFinale-b.tempoFinale)>1e-9 ||
            a.tempoFinale>a.tempoIniziale+1e-9)
            throw std::runtime_error("Riproducibilita', storico o elitismo non corretti");
        double precedente=std::numeric_limits<double>::infinity();
        for(const auto& g:a.storico) {
            if(g.individuiValidi+g.individuiNonValidi!=20 ||
                g.individuiValidi<1 ||
                g.miglioreGlobale>precedente+1e-9 ||
                !std::isfinite(g.mediaValidi))
                throw std::runtime_error("Statistiche per generazione non coerenti");
            precedente=g.miglioreGlobale;
        }
        for(const auto& t:{a.miglioreIniziale,a.miglioreFinale}) {
            const auto geo=ValidatoreGeometrico::analizza(t.ricampionaPeriodica(n),sezioni,1.0);
            if(!geo.geometricamenteValida())throw std::runtime_error("Traiettoria finale non ammissibile");
        }
        // La fitness non deve accettare un margine impossibile.
        bool scartata=false;
        try {
            AlgoritmoGenetico bad(20,3,.8,.05,.1,3,2,2026);
            bad.impostaMargineMinimo(6.01);
            (void)bad.ottimizzaConDettagli(sezioni,veicolo,40);
        } catch(const std::invalid_argument&) {scartata=true;}
        if(!scartata)throw std::runtime_error("Margine impossibile accettato");
        std::cout<<"OK: limiti geometrici, storico, riproducibilita', elite e parametri impossibili\n";
    } catch(const std::exception& e) {
        std::cerr<<"ERRORE: "<<e.what()<<'\n';return 1;
    }
}
