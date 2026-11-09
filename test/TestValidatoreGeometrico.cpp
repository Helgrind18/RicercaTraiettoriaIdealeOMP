#include "ValidatoreGeometrico.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    try {
        constexpr double pi=3.14159265358979323846;
        const int n=120;
        std::vector<SezionePista> sezioni;
        for(int i=0;i<n;++i) {
            const double t=2*pi*i/n;
            const double c=std::cos(t),s=std::sin(t);
            sezioni.emplace_back(Punto2D(105*c,105*s),Punto2D(95*c,95*s));
        }
        const auto centrale=Traiettoria(std::vector<double>(n,.5));
        const auto buona=ValidatoreGeometrico::analizza(centrale,sezioni,2.0);
        if(!buona.geometricamenteValida() || std::abs(buona.margineMinimoMetri-5.0)>0.02)
            throw std::runtime_error("Cerchio regolare: esito inatteso");
        const auto vicina=Traiettoria(std::vector<double>(n,.02));
        const auto margine=ValidatoreGeometrico::analizza(vicina,sezioni,2.0);
        if(margine.puntiConMargineInsufficiente!=static_cast<std::size_t>(n) || margine.geometricamenteValida())
            throw std::runtime_error("Margine dal bordo non rilevato");
        // Controllo su polilinea con due segmenti incrociati non adiacenti.
        const std::vector<Punto2D> quadrilatero{{-2,-2},{2,2},{-2,2},{2,-2}};
        std::vector<SezionePista> s2;
        for(auto p:quadrilatero) s2.emplace_back(Punto2D(p.getX()-1,p.getY()),Punto2D(p.getX()+1,p.getY()));
        const auto incrocio=ValidatoreGeometrico::analizza(Traiettoria(std::vector<double>(4,.5)),s2);
        if(incrocio.autointersezioni==0) throw std::runtime_error("Autointersezione non rilevata");
        std::cout<<"OK: validatore geometrico, bordo e autointersezioni\n";
    } catch (const std::exception& e) {
        std::cerr<<"ERRORE: "<<e.what()<<"\n";return 1;
    }
}
