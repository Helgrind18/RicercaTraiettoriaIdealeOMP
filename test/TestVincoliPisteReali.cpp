// Test di regressione sui circuiti REALI (risoluzione ridotta per CTest veloce).
#include "AlgoritmoGenetico.hpp"
#include "InterpolatorePista.hpp"
#include "ValidatoreGeometrico.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<SezionePista> caricaSezioni(const std::string& path,int n) {
    std::ifstream f(path);if(!f)throw std::runtime_error("Pista non trovata: "+path);
    std::string line;std::getline(f,line);
    std::vector<Punto2D> punti;
    while(std::getline(f,line)) {
        if(line.empty())continue;
        std::replace(line.begin(),line.end(),';',',');
        const auto i=line.find(',');
        if(i==std::string::npos)throw std::runtime_error("CSV incompleto");
        Punto2D p(std::stod(line.substr(0,i)),std::stod(line.substr(i+1)));
        if(punti.empty()||punti.back().distanza(p)>1e-4)punti.push_back(p);
    }
    if(punti.size()>3&&punti.front().distanza(punti.back())<0.5)punti.pop_back();
    std::vector<SezionePista> s;
    for(const auto& c:InterpolatorePista::campionaChiuso(punti,n)) {
        const double dx=-6*c.tangente.getY(),dy=6*c.tangente.getX();
        s.emplace_back(Punto2D(c.punto.getX()+dx,c.punto.getY()+dy),
                       Punto2D(c.punto.getX()-dx,c.punto.getY()-dy));
    }
    return s;
}

int main(int argc,char** argv) {
    try {
        if(argc!=3)throw std::runtime_error("Uso: TestVincoliPisteReali SPA.csv MONZA.csv");
        const std::array<Gomma,4> gomme{{Gomma(0,TipoGomma::SOFT,.95),Gomma(1,TipoGomma::SOFT,.95),
                                        Gomma(2,TipoGomma::SOFT,.95),Gomma(3,TipoGomma::SOFT,.95)}};
        const Veicolo autoF1(798.,650000.,10.,15.,.9,1.5,315./3.6,18.,gomme);
        for(int k=1;k<=2;++k){
            const auto sezioni=caricaSezioni(argv[k],600);
            for(unsigned seed : {2026u,2027u}){
                AlgoritmoGenetico ga(25,5,.8,.05,.1,3,2,seed);
                ga.impostaMargineMinimo(1.0);
                const auto risultato=ga.ottimizzaConDettagli(sezioni,autoF1,40);
                const auto finale=risultato.miglioreFinale.ricampionaPeriodica(600);
                const auto geo=ValidatoreGeometrico::analizza(finale,sezioni,1.0);
                if(!geo.geometricamenteValida())throw std::runtime_error("Soluzione geometricamente non ammissibile");
                if(risultato.tempoFinale>risultato.tempoIniziale+1e-9)
                    throw std::runtime_error("Peggioramento rispetto al migliore iniziale");
                std::cout<<(k==1?"Spa":"Monza")<<" seed="<<seed
                         <<" tempo="<<risultato.tempoFinale<<"s margine="
                         <<geo.margineMinimoMetri<<"m\n";
            }
        }
        std::cout<<"OK: vincoli piste reali (due seed)\n";
    } catch(const std::exception& e){std::cerr<<"ERRORE: "<<e.what()<<'\n';return 1;}
}
