#include "InterpolatorePista.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
constexpr double epsilon = 1e-9;

struct Vettore {
    double x = 0.0;
    double y = 0.0;
};
Vettore operator+(Vettore a, Vettore b) { return {a.x+b.x, a.y+b.y}; }
Vettore operator-(Vettore a, Vettore b) { return {a.x-b.x, a.y-b.y}; }
Vettore operator*(Vettore a, double s) { return {a.x*s, a.y*s}; }
Vettore operator/(Vettore a, double s) { return {a.x/s, a.y/s}; }
double norma(Vettore a) { return std::hypot(a.x, a.y); }
Vettore comeVettore(const Punto2D& p) { return {p.getX(), p.getY()}; }
Punto2D comePunto(Vettore p) { return {p.x, p.y}; }

struct NodoArco {
    double distanza = 0.0;
    int segmento = 0;
    double parametro = 0.0;
};

// Risolve una matrice tridiagonale usando l'eliminazione di Thomas.
// sotto[0] e sopra[n-1] non sono utilizzati.
std::vector<double> risolviTridiagonale(const std::vector<double>& sotto,
                                        const std::vector<double>& diagonale,
                                        const std::vector<double>& sopra,
                                        const std::vector<double>& rhs) {
    const size_t n = rhs.size();
    std::vector<double> c(n, 0.0), d(n, 0.0), soluzione(n, 0.0);
    double divisore = diagonale[0];
    if (std::abs(divisore) < epsilon) throw std::runtime_error("Spline: sistema singolare");
    c[0] = sopra[0] / divisore;
    d[0] = rhs[0] / divisore;
    for (size_t i = 1; i < n; ++i) {
        divisore = diagonale[i] - sotto[i] * c[i-1];
        if (std::abs(divisore) < epsilon) throw std::runtime_error("Spline: sistema singolare");
        c[i] = i + 1 < n ? sopra[i] / divisore : 0.0;
        d[i] = (rhs[i] - sotto[i] * d[i-1]) / divisore;
    }
    soluzione[n-1] = d[n-1];
    for (size_t i = n-1; i-- > 0;) soluzione[i] = d[i] - c[i] * soluzione[i+1];
    return soluzione;
}

// Sistema ciclico tridiagonale: A[0,n-1]=angoloAlto,
// A[n-1,0]=angoloBasso. Sherman-Morrison, complessita' O(n).
std::vector<double> risolviCiclico(const std::vector<double>& sotto,
                                  const std::vector<double>& diagonale,
                                  const std::vector<double>& sopra,
                                  double angoloAlto, double angoloBasso,
                                  const std::vector<double>& rhs) {
    const size_t n = rhs.size();
    const double gamma = -diagonale[0];
    auto corretta = diagonale;
    corretta[0] -= gamma;
    corretta[n-1] -= angoloAlto*angoloBasso/gamma;
    const auto x = risolviTridiagonale(sotto, corretta, sopra, rhs);
    std::vector<double> u(n, 0.0);
    u[0] = gamma;
    u[n-1] = angoloBasso;
    const auto z = risolviTridiagonale(sotto, corretta, sopra, u);
    const double denominatore = 1.0 + z[0] + angoloAlto*z[n-1]/gamma;
    if (std::abs(denominatore) < epsilon) throw std::runtime_error("Spline: sistema ciclico singolare");
    const double fattore = (x[0] + angoloAlto*x[n-1]/gamma) / denominatore;
    auto soluzione = x;
    for (size_t i=0; i<n; ++i) soluzione[i] -= fattore * z[i];
    return soluzione;
}

struct SplineChiusa {
    std::vector<Vettore> punti;
    std::vector<Vettore> secondeDerivate;
    std::vector<double> intervalli;

    explicit SplineChiusa(std::vector<Vettore> p) : punti(std::move(p)) {
        const size_t n = punti.size();
        intervalli.resize(n);
        secondeDerivate.resize(n);
        for (size_t i = 0; i < n; ++i) {
            const double corda = norma(punti[(i+1)%n]-punti[i]);
            if (!(corda > epsilon)) throw std::invalid_argument("Punti originali coincidenti nel CSV");
            // Parametrizzazione centripeta per limitare le oscillazioni
            // quando i punti originali sono distribuiti in modo irregolare.
            intervalli[i] = std::sqrt(corda);
        }
        std::vector<double> sotto(n, 0.0), diag(n), sopra(n, 0.0), rx(n), ry(n);
        for (size_t i = 0; i < n; ++i) {
            const size_t prec = (i+n-1)%n, succ=(i+1)%n;
            const double hPrev=intervalli[prec], h=intervalli[i];
            const Vettore differenza = (punti[succ]-punti[i])/h - (punti[i]-punti[prec])/hPrev;
            diag[i] = 2.0*(hPrev+h);
            if (i > 0) sotto[i] = hPrev;
            if (i + 1 < n) sopra[i] = h;
            rx[i] = 6.0*differenza.x;
            ry[i] = 6.0*differenza.y;
        }
        const double bordo = intervalli[n-1];
        const auto mx = risolviCiclico(sotto,diag,sopra,bordo,bordo,rx);
        const auto my = risolviCiclico(sotto,diag,sopra,bordo,bordo,ry);
        for (size_t i = 0; i < n; ++i) secondeDerivate[i] = {mx[i],my[i]};
    }

    Vettore valuta(int i, double t) const {
        const int j = (i+1)%static_cast<int>(punti.size());
        const double h = intervalli[i], a = 1.0-t, b=t;
        return punti[i]*a + punti[j]*b
             + (secondeDerivate[i]*(a*a*a-a) + secondeDerivate[j]*(b*b*b-b))*(h*h/6.0);
    }
    Vettore tangente(int i, double t) const {
        const int j = (i+1)%static_cast<int>(punti.size());
        const double h = intervalli[i], a = 1.0-t, b=t;
        return (punti[j]-punti[i])/h
             + (secondeDerivate[i]*(1.0-3.0*a*a) + secondeDerivate[j]*(3.0*b*b-1.0))*(h/6.0);
    }
};
} // namespace

std::vector<CampionePista> InterpolatorePista::campionaChiuso(
    const std::vector<Punto2D>& puntiOriginali, int numeroCampioni) {
    if (numeroCampioni < 3) throw std::invalid_argument("Servono almeno tre sezioni della pista");

    std::vector<Vettore> punti;
    punti.reserve(puntiOriginali.size());
    for (const auto& p : puntiOriginali) {
        const Vettore v = comeVettore(p);
        if (!std::isfinite(v.x) || !std::isfinite(v.y))
            throw std::invalid_argument("Coordinate non finite nel CSV della pista");
        if (punti.empty() || norma(v-punti.back()) > epsilon) punti.push_back(v);
    }
    if (punti.size()>3 && norma(punti.front()-punti.back())<0.5) punti.pop_back();
    if (punti.size()<3) throw std::invalid_argument("Servono almeno tre punti distinti");
    SplineChiusa spline(std::move(punti));
    const int n=static_cast<int>(spline.punti.size());

    std::vector<NodoArco> tabella;
    tabella.push_back({0.0,0,0.0});
    double totale=0.0;
    Vettore precedente=spline.punti.front();
    for (int i=0; i<n; ++i) {
        const double corda=norma(spline.punti[(i+1)%n]-spline.punti[i]);
        const int passi=std::max(16,static_cast<int>(std::ceil(corda/1.0)));
        for (int k=1; k<=passi; ++k) {
            const double t=static_cast<double>(k)/passi;
            const Vettore punto=spline.valuta(i,t);
            totale+=norma(punto-precedente);
            tabella.push_back({totale,i,t});
            precedente=punto;
        }
    }
    if (!(totale>epsilon) || !std::isfinite(totale)) throw std::runtime_error("Curva pista degenerata");

    std::vector<CampionePista> campioni;
    campioni.reserve(numeroCampioni);
    auto it=tabella.begin()+1;
    for (int i=0; i<numeroCampioni; ++i) {
        const double s=totale*static_cast<double>(i)/numeroCampioni;
        it=std::lower_bound(it,tabella.end(),s,
                            [](const NodoArco& n,double valore){return n.distanza<valore;});
        if (it==tabella.end()) throw std::runtime_error("Errore nel campionamento della pista");
        const NodoArco& prima=*(it-1);
        const double tratto=it->distanza-prima.distanza;
        const double alpha=tratto>epsilon ? (s-prima.distanza)/tratto : 0.0;
        const int segmento=it->segmento;
        const double t0=prima.segmento==segmento ? prima.parametro : 0.0;
        const double t=std::clamp(t0+(it->parametro-t0)*alpha,0.0,1.0);
        const Vettore pos=spline.valuta(segmento,t);
        Vettore dir=spline.tangente(segmento,t);
        const double len=norma(dir);
        if (!(len>epsilon) || !std::isfinite(len)) throw std::runtime_error("Tangente nulla nella pista interpolata");
        dir=dir/len;
        campioni.push_back({comePunto(pos),comePunto(dir)});
    }
    return campioni;
}
