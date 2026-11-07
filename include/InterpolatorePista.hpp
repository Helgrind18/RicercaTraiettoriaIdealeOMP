#ifndef RICERCATRAIETTORIAIDEALEOMP_INTERPOLATOREPISTA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_INTERPOLATOREPISTA_HPP

#include "Punto2D.hpp"

#include <vector>

// Campione su un asse centrale chiuso: la tangente e' un versore orientato
// nella direzione di percorrenza. Le coordinate sono espresse in metri.
struct CampionePista {
    Punto2D punto;
    Punto2D tangente;
};

// Ricostruisce una curva chiusa e C2 dai punti originali del CSV, quindi
// ricampiona per distanza d'arco. Non modifica ne' il modello fisico ne'
// la rappresentazione genetica delle traiettorie.
class InterpolatorePista {
public:
    static std::vector<CampionePista> campionaChiuso(
        const std::vector<Punto2D>& puntiOriginali,
        int numeroCampioni);
};

#endif // RICERCATRAIETTORIAIDEALEOMP_INTERPOLATOREPISTA_HPP
