// APEX LAB - rivalutazione della STESSA soluzione genetica a diverse risoluzioni.
// Nessuna modifica al modello fisico, agli operatori genetici o alla GUI.
#include "AlgoritmoGenetico.hpp"
#include "Gomma.hpp"
#include "InterpolatorePista.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "ValidatoreGeometrico.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct Opzioni {
    std::string spa="gui/data/spa.csv", monza="gui/data/monza.csv";
    std::string solo="entrambe", csv, geniCsv, importaGeni;
    int geni=80, popolazione=80, generazioni=25, campioniGA=4800;
    int torneo=3, elite=4;
    unsigned int seed=2026;
    double crossover=.80, mutazione=.05, intensita=.10;
    double tolleranza=.5, margine=0.;
    bool esteso=false, vincoloGA=false;
};
void help(const char* exe) {
    std::cout << "Uso: " << exe << " [--spa PATH] [--monza PATH] [--solo spa|monza|entrambe] "
                 "[--geni N] [--popolazione N] [--generazioni N] [--campioni-ga N] "
                 "[--seed N] [--tolleranza PCT] [--margine METRI] [--csv PATH] "
                 "[--geni-csv PATH] [--importa-geni PATH] [--esteso]\n"
              << "Le soluzioni vengono ottimizzate UNA SOLA VOLTA, oppure caricate da CSV, "
                 "e poi rivalutate senza riottimizzarle.\n";
}
int intero(const std::string& s,int minimo,int massimo) {
    std::size_t n=0; const long long v=std::stoll(s,&n);
    if(n!=s.size() || v<minimo || v>massimo) throw std::invalid_argument("Intero non valido: "+s);
    return static_cast<int>(v);
}
double reale(const std::string& s,double minimo,double massimo) {
    std::size_t n=0;const double v=std::stod(s,&n);
    if(n!=s.size() || !std::isfinite(v) || v<minimo || v>massimo)
        throw std::invalid_argument("Valore non valido: "+s);
    return v;
}
Opzioni opzioni(int argc,char** argv) {
    Opzioni o;
    for(int i=1;i<argc;++i) {
        const std::string k=argv[i];
        if(k=="--help"||k=="-h") {help(argv[0]);std::exit(0);}
        if(k=="--esteso") {o.esteso=true;continue;}
        if(k=="--vincolo-ga") {o.vincoloGA=true;continue;}
        if(i+1>=argc)throw std::invalid_argument("Manca un valore per "+k);
        const std::string v=argv[++i];
        if(k=="--spa")o.spa=v;
        else if(k=="--monza")o.monza=v;
        else if(k=="--solo")o.solo=v;
        else if(k=="--csv")o.csv=v;
        else if(k=="--geni-csv")o.geniCsv=v;
        else if(k=="--importa-geni")o.importaGeni=v;
        else if(k=="--geni")o.geni=intero(v,3,20000);
        else if(k=="--popolazione")o.popolazione=intero(v,2,100000);
        else if(k=="--generazioni")o.generazioni=intero(v,1,1000000);
        else if(k=="--campioni-ga")o.campioniGA=intero(v,80,20000);
        else if(k=="--seed")o.seed=static_cast<unsigned int>(intero(v,0,2147483647));
        else if(k=="--torneo")o.torneo=intero(v,2,100000);
        else if(k=="--elite")o.elite=intero(v,0,99999);
        else if(k=="--crossover")o.crossover=reale(v,0.,1.);
        else if(k=="--mutazione")o.mutazione=reale(v,0.,1.);
        else if(k=="--intensita")o.intensita=reale(v,1e-12,1.);
        else if(k=="--tolleranza")o.tolleranza=reale(v,0.00001,100.);
        else if(k=="--margine")o.margine=reale(v,0.,100.);
        else throw std::invalid_argument("Opzione sconosciuta: "+k);
    }
    if(o.solo!="spa" && o.solo!="monza" && o.solo!="entrambe")
        throw std::invalid_argument("--solo deve essere spa, monza o entrambe");
    if(o.vincoloGA && o.margine<=0.0)
        throw std::invalid_argument("--vincolo-ga richiede --margine METRI > 0");
    if(o.elite>=o.popolazione||o.torneo>o.popolazione)
        throw std::invalid_argument("Elite/torneo incompatibili con la popolazione");
    return o;
}
std::vector<Punto2D> caricaPista(const std::string& path) {
    std::ifstream f(path);
    if(!f) throw std::runtime_error("Impossibile leggere "+path);
    std::string r;std::getline(f,r);
    std::vector<Punto2D> p;
    while(std::getline(f,r)) {
        if(!r.empty()&&r.back()=='\r')r.pop_back();
        if(r.empty())continue;
        std::replace(r.begin(),r.end(),';',',');
        const auto k=r.find(',');
        if(k==std::string::npos)throw std::runtime_error("CSV pista non valido: "+path);
        const double x=std::stod(r.substr(0,k)), y=std::stod(r.substr(k+1));
        if(!std::isfinite(x)||!std::isfinite(y))throw std::runtime_error("Coordinate non finite");
        const Punto2D q(x,y);
        if(p.empty()||p.back().distanza(q)>1e-4)p.push_back(q);
    }
    if(p.size()>3&&p.front().distanza(p.back())<.5)p.pop_back();
    if(p.size()<3)throw std::runtime_error("Circuito con meno di tre punti");
    return p;
}
std::vector<SezionePista> sezioni(const std::vector<Punto2D>& centro,int n) {
    std::vector<SezionePista> s; s.reserve(n);
    for(const auto& c:InterpolatorePista::campionaChiuso(centro,n)) {
        const double x=-6.*c.tangente.getY(),y=6.*c.tangente.getX();
        s.emplace_back(Punto2D(c.punto.getX()+x,c.punto.getY()+y),
                       Punto2D(c.punto.getX()-x,c.punto.getY()-y));
    }
    return s;
}
Veicolo veicolo() {
    const std::array<Gomma,4> gomme{{Gomma(0,TipoGomma::SOFT,.95),Gomma(1,TipoGomma::SOFT,.95),
                                    Gomma(2,TipoGomma::SOFT,.95),Gomma(3,TipoGomma::SOFT,.95)}};
    return Veicolo(798.,650000.,10.,15.,.9,1.5,315./3.6,18.,gomme);
}
using GeniSalvati=std::map<std::string,std::map<std::string,std::vector<double>>>;
GeniSalvati leggiGeni(const std::string& path) {
    GeniSalvati dati;
    std::ifstream f(path);
    if(!f)throw std::runtime_error("Impossibile aprire geni: "+path);
    std::string r;
    std::getline(f,r);
    if(r!="circuito,soluzione,indice,valore")throw std::runtime_error("Intestazione geni CSV non riconosciuta");
    while(std::getline(f,r)) {
        if(!r.empty() && r.back()=='\r')r.pop_back();
        if(r.empty())continue;
        const auto a=r.find(','),b=r.find(',',a+1),c=r.find(',',b+1);
        if(a==std::string::npos||b==std::string::npos||c==std::string::npos)
            throw std::runtime_error("Riga geni CSV non valida");
        const auto pista=r.substr(0,a), soluzione=r.substr(a+1,b-a-1);
        const int indice=intero(r.substr(b+1,c-b-1),0,20000);
        const double valore=reale(r.substr(c+1),0.,1.);
        auto& g=dati[pista][soluzione];
        if(indice!=static_cast<int>(g.size()))throw std::runtime_error("Indici geni non consecutivi");
        g.push_back(valore);
    }
    return dati;
}
void salvaGeni(std::ostream& f,const std::string& pista,const std::string& nome,const Traiettoria& t) {
    for(int i=0;i<t.getNumeroPosizioni();++i)
        f<<pista<<','<<nome<<','<<i<<','<<t.getPosizioneLaterale(i)<<'\n';
}
struct Risoluzione {
    int n=0;
    double tempo=0;
    EsitoGeometrico geometria;
};
std::vector<Risoluzione> misura(const Traiettoria& t,const std::vector<Punto2D>& pista,
                               const Veicolo& v,const std::vector<int>& livelli,double margine) {
    std::vector<Risoluzione> r;
    const ValutatoreTraiettoria valutatore;
    for(int n:livelli) {
        const auto s=sezioni(pista,n);
        const auto d=t.ricampionaPeriodica(n);
        const double tempo=valutatore.valutaTraiettoria(d,s,v);
        if(!std::isfinite(tempo)||tempo<=0.)throw std::runtime_error("Tempo sul giro non valido");
        const auto geo=ValidatoreGeometrico::analizza(d,s,margine);
        r.push_back({n,tempo,geo});
    }
    return r;
}
void stampa(const std::string& pista,const std::string& nome,
            const std::vector<Risoluzione>& r,const Opzioni& o,int numeroGeni,std::ostream* csv) {
    const double riferimento=r.back().tempo;
    std::cout<<"\n=== "<<pista<<" | "<<nome<<" | stessa traiettoria ===\n";
    std::cout<<"N       tempo_s   scarto_rif%  margine_m  incroci_bordo  auto_inc  valida\n";
    bool potenzialmenteStabile=false;
    for(const auto& a:r) {
        const double scarto=100.*(a.tempo-riferimento)/riferimento;
        std::cout<<std::setw(6)<<a.n<<' '<<std::setw(11)<<std::fixed<<std::setprecision(4)<<a.tempo
                 <<' '<<std::showpos<<std::setw(11)<<std::setprecision(3)<<scarto<<std::noshowpos
                 <<' '<<std::setw(10)<<std::setprecision(3)<<a.geometria.margineMinimoMetri
                 <<' '<<std::setw(13)<<a.geometria.attraversamentiBordo
                 <<' '<<std::setw(9)<<a.geometria.autointersezioni
                 <<' '<<(a.geometria.geometricamenteValida()?"SI":"NO")<<'\n';
        if(a.n==4800)potenzialmenteStabile=std::abs(scarto)<=o.tolleranza;
        if(csv) {
            *csv<<pista<<','<<nome<<','<<numeroGeni<<','<<o.campioniGA<<','<<a.n<<','
                <<a.tempo<<','<<scarto<<','<<a.geometria.lunghezzaMetri<<','
                <<a.geometria.margineMinimoMetri<<','<<a.geometria.puntiFuoriSezione<<','
                <<a.geometria.puntiConMargineInsufficiente<<','<<a.geometria.segmentiDegeneri<<','
                <<a.geometria.attraversamentiBordo<<','<<a.geometria.autointersezioni<<','
                <<(a.geometria.geometricamenteValida()?1:0)<<'\n';
        }
    }
    std::cout<<"4800 campioni entro "<<o.tolleranza<<"% dal riferimento "<<r.back().n
             <<": "<<(potenzialmenteStabile?"SI":"NO")<<" (solo diagnostica)\n";
}
}

int main(int argc,char** argv) {
    try {
        const Opzioni o=opzioni(argc,argv);
        if(!o.importaGeni.empty() && o.importaGeni==o.geniCsv)
            throw std::invalid_argument("Input e output dei geni devono essere file diversi");
        const Veicolo v=veicolo();
        const std::vector<int> livelli=o.esteso?std::vector<int>{600,1200,2400,4800,9600,19200}
                                               :std::vector<int>{600,1200,2400,4800,9600};
        std::ofstream csv,geni;
        if(!o.csv.empty()) {
            csv.open(o.csv);if(!csv)throw std::runtime_error("Impossibile creare CSV risultati");
            csv<<"circuito,soluzione,geni,campioni_ga,campioni,tempo_s,scarto_riferimento_pct,"
                  "lunghezza_m,margine_minimo_m,punti_fuori_sezione,punti_margine_insufficiente,"
                  "segmenti_degeneri,attraversamenti_bordo,autointersezioni,validita_geometrica\n";
            csv<<std::setprecision(17);
        }
        if(!o.geniCsv.empty()) {
            geni.open(o.geniCsv);if(!geni)throw std::runtime_error("Impossibile creare CSV geni");
            geni<<"circuito,soluzione,indice,valore\n"<<std::setprecision(17);
        }
        const auto caricati=o.importaGeni.empty()?GeniSalvati{}:leggiGeni(o.importaGeni);
        std::cout<<"APEX LAB - convergenza su traiettoria ottimizzata (DIAGNOSTICA)\n";
        std::cout<<"Modello fisico invariato; livello di riferimento: "<<livelli.back()<<" campioni\n";
        const std::vector<std::pair<std::string,std::string>> piste{{"spa",o.spa},{"monza",o.monza}};
        for(const auto& [nome,path]:piste) {
            if(o.solo!="entrambe"&&o.solo!=nome)continue;
            const auto pista=caricaPista(path);
            Traiettoria iniziale, finale;
            if(o.importaGeni.empty()) {
                const auto s=sezioni(pista,o.campioniGA);
                AlgoritmoGenetico genetico(o.popolazione,o.generazioni,o.crossover,o.mutazione,
                                          o.intensita,o.torneo,o.elite,o.seed);
                if(o.vincoloGA) genetico.impostaMargineMinimo(o.margine);
                std::cout<<"\n"<<nome<<": ottimizzazione "<<o.popolazione<<"x"<<o.generazioni
                         <<" a "<<o.campioniGA<<" campioni..."<<std::flush;
                const auto start=std::chrono::steady_clock::now();
                const auto ris=genetico.ottimizzaConDettagli(s,v,o.geni);
                std::cout<<" finita ("<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()
                         <<" s). Iniziale="<<ris.tempoIniziale<<", finale="<<ris.tempoFinale<<"\n";
                iniziale=ris.miglioreIniziale; finale=ris.miglioreFinale;
                // Verifica che il nostro test ripeta esattamente la fitness del GA.
                const ValutatoreTraiettoria val;
                const double controllo=val.valutaTraiettoria(finale.ricampionaPeriodica(o.campioniGA),s,v);
                if(std::abs(controllo-ris.tempoFinale)>1e-7)
                    throw std::runtime_error("Incoerenza fitness genetico/rivalutazione");
            } else {
                const auto it=caricati.find(nome);
                if(it==caricati.end()||it->second.count("iniziale")==0||it->second.count("ottimizzata")==0)
                    throw std::runtime_error("Mancano i geni iniziali/finali di "+nome);
                iniziale=Traiettoria(it->second.at("iniziale"));
                finale=Traiettoria(it->second.at("ottimizzata"));
                if(iniziale.getNumeroPosizioni()!=finale.getNumeroPosizioni())
                    throw std::runtime_error("Numero geni non coerente");
                std::cout<<"\n"<<nome<<": importati "<<finale.getNumeroPosizioni()<<" geni (nessuna GA)\n";
            }
            if(geni) {salvaGeni(geni,nome,"iniziale",iniziale);salvaGeni(geni,nome,"ottimizzata",finale);}
            stampa(nome,"iniziale",misura(iniziale,pista,v,livelli,o.margine),o,iniziale.getNumeroPosizioni(),csv?&csv:nullptr);
            stampa(nome,"ottimizzata",misura(finale,pista,v,livelli,o.margine),o,finale.getNumeroPosizioni(),csv?&csv:nullptr);
        }
        std::cout<<"\nOK: diagnostica terminata (non prova ottimo globale o validita fisica).\n";
        if(csv)std::cout<<"CSV risultati: "<<o.csv<<'\n';
        if(geni)std::cout<<"CSV geni: "<<o.geniCsv<<'\n';
    } catch(const std::exception& e) {
        std::cerr<<"ERRORE: "<<e.what()<<'\n';return 1;
    }
}
