// Benchmark sequenziale riproducibile su SPA e MONZA, con margine geometrico.
// Varia un parametro alla volta, esegue piu' seed e produce righe CSV.
#include "AlgoritmoGenetico.hpp"
#include "InterpolatorePista.hpp"
#include "ValidatoreGeometrico.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Opt {
    std::string spa="gui/data/spa.csv", monza="gui/data/monza.csv", csv="/tmp/apex_esperimenti_v5.csv";
    int campioni=1200, geni=80, generazioni=50, popolazione=80, semi=3;
    double margine=1.0;
    bool soloBaseline=false;
    std::string solo="entrambe";
    std::string parametro="tutti";
};
int entero(const std::string& x) {size_t n=0;int a=std::stoi(x,&n);if(n!=x.size()) throw std::runtime_error("Intero non valido");return a;}
Opt opzioni(int argc,char** argv){
    Opt o;
    for(int i=1;i<argc;++i){
        const std::string k=argv[i];
        if(k=="--solo-baseline"){o.soloBaseline=true;continue;}
        if(k=="--help"||k=="-h"){
            std::cout<<"Uso: --spa PATH --monza PATH --csv FILE --campioni N --geni N "
                         "--popolazione N --generazioni N --semi N --margine METRI --solo-baseline --solo spa|monza|entrambe --parametro tutti|popolazione|mutazione|geni\n";
            std::exit(0);
        }
        if(i+1>=argc)throw std::runtime_error("Manca valore per "+k);
        const std::string v=argv[++i];
        if(k=="--solo")o.solo=v;
        else if(k=="--parametro")o.parametro=v;
        else if(k=="--spa")o.spa=v;
        else if(k=="--monza")o.monza=v;
        else if(k=="--csv")o.csv=v;
        else if(k=="--campioni")o.campioni=entero(v);
        else if(k=="--geni")o.geni=entero(v);
        else if(k=="--popolazione")o.popolazione=entero(v);
        else if(k=="--generazioni")o.generazioni=entero(v);
        else if(k=="--semi")o.semi=entero(v);
        else if(k=="--margine")o.margine=std::stod(v);
        else throw std::runtime_error("Opzione sconosciuta: "+k);
    }
    if(o.solo!="entrambe"&&o.solo!="spa"&&o.solo!="monza")throw std::runtime_error("--solo invalido");
    if(o.parametro!="tutti"&&o.parametro!="popolazione"&&o.parametro!="mutazione"&&o.parametro!="geni")
        throw std::runtime_error("--parametro invalido");
    if(o.campioni<80||o.campioni>20000||o.geni<3||o.geni>2000||
       o.popolazione<10||o.popolazione>10000||o.generazioni<1||o.generazioni>100000||
       o.semi<1||o.semi>100||!std::isfinite(o.margine)||o.margine<0||o.margine>=6)
       throw std::runtime_error("Parametri di benchmark fuori intervallo");
    return o;
}
std::vector<Punto2D> leggi(const std::string& path){
    std::ifstream f(path);if(!f)throw std::runtime_error("CSV non leggibile: "+path);
    std::vector<Punto2D> p;std::string s;std::getline(f,s);
    while(std::getline(f,s)){
        if(s.empty())continue;
        std::replace(s.begin(),s.end(),';',',');
        const auto i=s.find(',');if(i==std::string::npos)throw std::runtime_error("CSV invalido");
        const Punto2D q(std::stod(s.substr(0,i)),std::stod(s.substr(i+1)));
        if(p.empty()||p.back().distanza(q)>1e-4)p.push_back(q);
    }
    if(p.size()>3&&p.front().distanza(p.back())<0.5)p.pop_back();
    return p;
}
std::vector<SezionePista> pista(const std::string& path,int n){
    std::vector<SezionePista> s; s.reserve(n);
    for(const auto& p:InterpolatorePista::campionaChiuso(leggi(path),n)) {
        const double dx=-6*p.tangente.getY(),dy=6*p.tangente.getX();
        s.emplace_back(Punto2D(p.punto.getX()+dx,p.punto.getY()+dy),
                       Punto2D(p.punto.getX()-dx,p.punto.getY()-dy));
    }
    return s;
}
Veicolo veicolo(){
    const std::array<Gomma,4> g{{Gomma(0,TipoGomma::SOFT,.95),Gomma(1,TipoGomma::SOFT,.95),
                                 Gomma(2,TipoGomma::SOFT,.95),Gomma(3,TipoGomma::SOFT,.95)}};
    return Veicolo(798.,650000.,10.,15.,.9,1.5,315./3.6,18.,g);
}
struct Ris {double tempo,sec,margine;};
}
int main(int argc,char** argv) {
    try {
        const auto o=opzioni(argc,argv);
        std::ofstream csv(o.csv);if(!csv)throw std::runtime_error("CSV output non scrivibile");
        csv<<"pista,esperimento,valore,seed,geni,campioni,popolazione,generazioni,mutazione,iniziale_s,finale_s,guadagno_s,tempo_esecuzione_s,margine_minimo_m,validi_ultima,non_validi_ultima\n";
        csv<<std::setprecision(15);
        for(const auto& [nome,file]:std::array<std::pair<std::string,std::string>,2>{{{"spa",o.spa},{"monza",o.monza}}}){
            if(o.solo!="entrambe"&&o.solo!=nome)continue;
            const auto sezioni=pista(file,o.campioni);
            const auto vettura=veicolo();
            // Include baseline; cambia una sola dimensione per volta.
            const std::vector<std::pair<std::string,double>> esperimenti=o.soloBaseline ?
                std::vector<std::pair<std::string,double>>{{"baseline",0}} :
                std::vector<std::pair<std::string,double>>{
                {"baseline",0}, {"popolazione",40}, {"popolazione",160},
                {"mutazione",.02}, {"mutazione",.10},
                {"geni",40}, {"geni",160}
            };
            for(const auto& [tipo,valore]:esperimenti){
                if(tipo!="baseline"&&o.parametro!="tutti"&&tipo!=o.parametro)continue;
                const int P=tipo=="popolazione"?static_cast<int>(valore):o.popolazione;
                const int G=tipo=="geni"?static_cast<int>(valore):o.geni;
                const double mut=tipo=="mutazione"?valore:.05;
                for(int k=0;k<o.semi;++k){
                    const unsigned seed=2026u+static_cast<unsigned>(k);
                    AlgoritmoGenetico ga(P,o.generazioni,.8,mut,.1,3,4,seed);
                    ga.impostaMargineMinimo(o.margine);
                    const auto begin=std::chrono::steady_clock::now();
                    const auto out=ga.ottimizzaConDettagli(sezioni,vettura,G);
                    const double tempo=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
                    const auto valid=ValidatoreGeometrico::analizza(out.miglioreFinale.ricampionaPeriodica(o.campioni),sezioni,o.margine);
                    if(!valid.geometricamenteValida())throw std::runtime_error("Il genetico ha restituito una soluzione illegale");
                    const auto& ultima=out.storico.back();
                    csv<<nome<<','<<tipo<<','<<valore<<','<<seed<<','<<G<<','<<o.campioni<<','<<P<<','
                       <<o.generazioni<<','<<mut<<','<<out.tempoIniziale<<','<<out.tempoFinale<<','
                       <<out.tempoIniziale-out.tempoFinale<<','<<tempo<<','<<valid.margineMinimoMetri<<','
                       <<ultima.individuiValidi<<','<<ultima.individuiNonValidi<<'\n';
                    csv.flush();
                    std::cout<<nome<<' '<<tipo<<'='<<valore<<" seed="<<seed<<" tempo="<<out.tempoFinale
                             <<"s margine="<<valid.margineMinimoMetri<<"m in "<<tempo<<"s"<<std::endl;
                }
            }
        }
        std::cout<<"OK: esperimenti riproducibili esportati in "<<o.csv<<"\n";
    } catch(const std::exception& e){std::cerr<<"ERRORE: "<<e.what()<<'\n';return 1;}
}
