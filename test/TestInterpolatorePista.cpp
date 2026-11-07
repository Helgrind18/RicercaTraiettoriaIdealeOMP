// Test autonomo (dalla root del progetto):
// g++ -std=c++20 -Iinclude test/TestInterpolatorePista.cpp src/Punto2D.cpp
// src/InterpolatorePista.cpp -o /tmp/test_apex_pista
#include "InterpolatorePista.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static std::vector<Punto2D> carica(const std::string& file) {
    std::ifstream f(file);
    if (!f) throw std::runtime_error("CSV non trovato: "+file);
    std::string r;
    std::getline(f,r); // intestazione
    std::vector<Punto2D> punti;
    while (std::getline(f,r)) {
        if (r.empty()) continue;
        const auto virgola=r.find(',');
        if (virgola==std::string::npos) continue;
        punti.emplace_back(std::stod(r.substr(0,virgola)),
                           std::stod(r.substr(virgola+1)));
    }
    return punti;
}
static void verifica(const std::vector<Punto2D>& input,
                     const std::string& nome) {
    double lunghezzaPrecedente = 0.0;
    for (int n : {80,160,300,600,1200}) {
        const auto curva=InterpolatorePista::campionaChiuso(input,n);
        assert(static_cast<int>(curva.size())==n);
        double lunghezza=0.0, maxCurvatura=0.0;
        for (int i=0;i<n;++i) {
            const auto& a=curva[i].punto;
            const auto& b=curva[(i+1)%n].punto;
            const double ds=a.distanza(b);
            assert(std::isfinite(ds) && ds>0.0);
            lunghezza+=ds;
            const auto& t=curva[i].tangente;
            assert(std::abs(std::hypot(t.getX(),t.getY())-1.0)<1e-8);
            const auto& prev=curva[(i+n-1)%n].punto;
            const double dx1=a.getX()-prev.getX(),dy1=a.getY()-prev.getY();
            const double dx2=b.getX()-a.getX(),dy2=b.getY()-a.getY();
            const double d1=std::hypot(dx1,dy1), d2=std::hypot(dx2,dy2);
            const double cross=std::abs(dx1*dy2-dy1*dx2);
            const double curvatura=2*cross/(d1*d2*prev.distanza(b));
            maxCurvatura=std::max(maxCurvatura,curvatura);
        }
        // Aumentando i campioni la lunghezza deve stabilizzarsi.
        if (n==1200) {
            assert(lunghezzaPrecedente>0.0);
            assert(std::abs(lunghezza-lunghezzaPrecedente)/lunghezza < 0.01);
        }
        lunghezzaPrecedente = lunghezza;
        std::cout << nome << " n=" << n << " lunghezza=" << lunghezza
                  << " maxCurvatura=" << maxCurvatura << '\n';
    }
}
int main(int argc,char**argv) {
    if (argc!=3) {
        std::cerr << "Uso: test_apex_pista spa.csv monza.csv\n";
        return 2;
    }
    verifica(carica(argv[1]),"Spa");
    verifica(carica(argv[2]),"Monza");
    std::cout << "OK: test geometrici completati\n";
}
