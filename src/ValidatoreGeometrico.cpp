#include "ValidatoreGeometrico.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace {
struct XY { double x, y; };
XY xy(const Punto2D& p) { return {p.getX(), p.getY()}; }
double dist(XY a, XY b) { return std::hypot(a.x-b.x, a.y-b.y); }
bool finito(XY p) { return std::isfinite(p.x) && std::isfinite(p.y); }
struct Segmento { XY a, b; };
double distanzaPuntoSegmento(XY p, const Segmento& s) {
    const double dx=s.b.x-s.a.x, dy=s.b.y-s.a.y;
    const double d2=dx*dx+dy*dy;
    if(d2<1e-20) return dist(p,s.a);
    const double t=std::clamp(((p.x-s.a.x)*dx+(p.y-s.a.y)*dy)/d2,0.0,1.0);
    return std::hypot(p.x-s.a.x-t*dx,p.y-s.a.y-t*dy);
}
double distanzaSegmenti(const Segmento& a,const Segmento& b) {
    return std::min({distanzaPuntoSegmento(a.a,b),distanzaPuntoSegmento(a.b,b),
                     distanzaPuntoSegmento(b.a,a),distanzaPuntoSegmento(b.b,a)});
}
double cross(XY a, XY b, XY c) { return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x); }
// Intersezione propria: la semplice tangenza non e' considerata attraversamento.
bool attraversano(const Segmento& u, const Segmento& v) {
    const double a=cross(u.a,u.b,v.a), b=cross(u.a,u.b,v.b);
    const double c=cross(v.a,v.b,u.a), d=cross(v.a,v.b,u.b);
    const double toll=1.e-11*std::max(1.,dist(u.a,u.b)*dist(v.a,v.b));
    return ((a>toll && b< -toll) || (a< -toll && b>toll)) &&
           ((c>toll && d< -toll) || (c< -toll && d>toll));
}
struct Chiave { std::int64_t x, y; bool operator==(const Chiave& o) const { return x==o.x && y==o.y; } };
struct Hash { std::size_t operator()(Chiave k) const {
    const auto a=std::hash<std::int64_t>{}(k.x), b=std::hash<std::int64_t>{}(k.y);
    return a^(b+0x9e3779b97f4a7c15ULL+(a<<6)+(a>>2));
} };
class IndiceSegmenti {
    static constexpr double cella=25.0; // coordinate espresse in metri
    std::unordered_map<Chiave,std::vector<int>,Hash> celle;
    std::vector<Segmento> segmenti;
    mutable std::vector<std::uint32_t> visto;
    mutable std::uint32_t epoca = 0;
    template<class F> static void celleBox(const Segmento& s, F f) {
        const auto x0=static_cast<std::int64_t>(std::floor(std::min(s.a.x,s.b.x)/cella));
        const auto x1=static_cast<std::int64_t>(std::floor(std::max(s.a.x,s.b.x)/cella));
        const auto y0=static_cast<std::int64_t>(std::floor(std::min(s.a.y,s.b.y)/cella));
        const auto y1=static_cast<std::int64_t>(std::floor(std::max(s.a.y,s.b.y)/cella));
        if (x1-x0>100000 || y1-y0>100000) throw std::runtime_error("Segmento geometrico troppo esteso");
        for(auto x=x0;x<=x1;++x) for(auto y=y0;y<=y1;++y) f(Chiave{x,y});
    }
public:
    explicit IndiceSegmenti(std::vector<Segmento> seg): segmenti(std::move(seg)), visto(segmenti.size(),0) {
        for (std::size_t i=0; i<segmenti.size(); ++i)
            celleBox(segmenti[i],[&](Chiave k){celle[k].push_back(static_cast<int>(i));});
    }
    double distanzaVicini(const Segmento& s, double raggio) const {
        if (++epoca == 0) {std::fill(visto.begin(),visto.end(),0); epoca=1;}
        double best=std::numeric_limits<double>::infinity();
        Segmento box{{std::min(s.a.x,s.b.x)-raggio,std::min(s.a.y,s.b.y)-raggio},
                     {std::max(s.a.x,s.b.x)+raggio,std::max(s.a.y,s.b.y)+raggio}};
        celleBox(box,[&](Chiave k) {
            const auto it=celle.find(k);
            if(it==celle.end())return;
            for(const int j:it->second) {
                if(visto[static_cast<std::size_t>(j)]==epoca)continue;
                visto[static_cast<std::size_t>(j)]=epoca;
                best=std::min(best,distanzaSegmenti(s,segmenti[static_cast<std::size_t>(j)]));
            }
        });
        return best;
    }
    template<class Predicate> std::size_t conta(const Segmento& s, Predicate pred) const {
        std::size_t n=0;
        if (++epoca == 0) { std::fill(visto.begin(), visto.end(), 0); epoca = 1; }
        celleBox(s,[&](Chiave k){
            const auto it=celle.find(k);
            if(it==celle.end())return;
            for(const int j:it->second) if(visto[static_cast<std::size_t>(j)] != epoca) {
                visto[static_cast<std::size_t>(j)] = epoca;
                if(pred(static_cast<std::size_t>(j)) && attraversano(s,segmenti[static_cast<std::size_t>(j)])) ++n;
            }
        });
        return n;
    }
};
}

EsitoGeometrico ValidatoreGeometrico::analizza(const Traiettoria& traiettoria,
                                               const std::vector<SezionePista>& sezioni,
                                               double margineRichiestoMetri) {
    if(sezioni.size()<3 || static_cast<std::size_t>(traiettoria.getNumeroPosizioni())!=sezioni.size())
        throw std::invalid_argument("Validatore: numero campioni e sezioni non coerente");
    if(!std::isfinite(margineRichiestoMetri) || margineRichiestoMetri<0)
        throw std::invalid_argument("Validatore: margine richiesto non valido");
    const std::size_t n=sezioni.size();
    EsitoGeometrico esito;
    std::vector<XY> punti, sx, dx;
    punti.reserve(n); sx.reserve(n); dx.reserve(n);
    esito.margineMinimoMetri=std::numeric_limits<double>::infinity();
    for (std::size_t i=0;i<n;++i) {
        const auto a=xy(sezioni[i].getPuntoSinistro()), b=xy(sezioni[i].getPuntoDestro());
        const double t=traiettoria.getPosizioneLaterale(static_cast<int>(i));
        const XY p{a.x*(1-t)+b.x*t,a.y*(1-t)+b.y*t};
        if(!finito(a)||!finito(b)||!finito(p)||!std::isfinite(t)) {
            esito.coordinateFinite=false;
            return esito;
        }
        const double larghezza=dist(a,b);
        if(!(larghezza>1e-8)) { ++esito.segmentiDegeneri; }
        const double margine=std::min(t,1.-t)*larghezza;
        esito.margineMinimoMetri=std::min(esito.margineMinimoMetri,margine);
        if(t<-1e-12 || t>1.+1e-12) ++esito.puntiFuoriSezione;
        if(margine+1e-9<margineRichiestoMetri) ++esito.puntiConMargineInsufficiente;
        punti.push_back(p);sx.push_back(a);dx.push_back(b);
    }
    std::vector<Segmento> tratti, bordi;
    tratti.reserve(n); bordi.reserve(2*n);
    for(std::size_t i=0;i<n;++i) {
        const auto j=(i+1)%n;
        const double len=dist(punti[i],punti[j]);
        if(len<1e-7) ++esito.segmentiDegeneri;
        esito.lunghezzaMetri+=len;
        tratti.push_back({punti[i],punti[j]});
        bordi.push_back({sx[i],sx[j]});
        bordi.push_back({dx[i],dx[j]});
    }
    const IndiceSegmenti indiceBordi(std::move(bordi));
    const IndiceSegmenti indiceTraiettoria(tratti);
    // Verifica il minimo distacco euclideo dell'intero segmento rispetto
    // ai segmenti del bordo, non soltanto alla sezione trasversale campionata.
    const double raggio=esito.margineMinimoMetri+1e-5;
    for(std::size_t i=0;i<n;++i) {
        const double d=indiceBordi.distanzaVicini(tratti[i],raggio);
        if(std::isfinite(d)) {
            esito.margineMinimoMetri=std::min(esito.margineMinimoMetri,d);
            if(d+1e-9<margineRichiestoMetri) ++esito.trattiConMargineInsufficiente;
        }
        esito.attraversamentiBordo+=indiceBordi.conta(tratti[i],[](std::size_t){return true;});
        esito.autointersezioni+=indiceTraiettoria.conta(tratti[i],[&](std::size_t j){
            return j>i && j!=(i+1)%n && i!=(j+1)%n;
        });
    }
    return esito;
}

bool ValidatoreGeometrico::ammissibileRapida(const Traiettoria& traiettoria,
        const std::vector<SezionePista>& sezioni, double margine) {
    const std::size_t n=sezioni.size();
    if(n<3 || static_cast<std::size_t>(traiettoria.getNumeroPosizioni())!=n ||
       !std::isfinite(margine) || margine<0.0) return false;
    auto punto=[&](std::size_t i)->XY {
        const XY a=xy(sezioni[i].getPuntoSinistro());
        const XY b=xy(sezioni[i].getPuntoDestro());
        const double t=traiettoria.getPosizioneLaterale(static_cast<int>(i));
        return XY{a.x*(1.0-t)+b.x*t,a.y*(1.0-t)+b.y*t};
    };
    for(std::size_t i=0;i<n;++i) {
        const std::size_t j=(i+1)%n;
        const XY a=xy(sezioni[i].getPuntoSinistro());
        const XY b=xy(sezioni[i].getPuntoDestro());
        const XY c=xy(sezioni[j].getPuntoSinistro());
        const XY d=xy(sezioni[j].getPuntoDestro());
        const double t=traiettoria.getPosizioneLaterale(static_cast<int>(i));
        if(!finito(a)||!finito(b)||!finito(c)||!finito(d)||!std::isfinite(t)||t<0||t>1 ||
           std::min(t,1-t)*dist(a,b)+1e-9<margine) return false;
        const Segmento segmento{punto(i),punto(j)};
        if(!finito(segmento.a)||!finito(segmento.b)||dist(segmento.a,segmento.b)<1e-7)
            return false;
        for(const Segmento bordo : {Segmento{a,c}, Segmento{b,d}}) {
            if(attraversano(segmento,bordo) || distanzaSegmenti(segmento,bordo)+1e-9<margine)
                return false;
        }
    }
    return true;
}
