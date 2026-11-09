#ifndef APEX_VALIDATORE_GEOMETRICO_HPP
#define APEX_VALIDATORE_GEOMETRICO_HPP

#include "Punto2D.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include <cstddef>
#include <vector>

// Diagnostica del CENTRO della vettura. Usata come vincolo di fitness
// quando viene configurato un margine positivo nel genetico.
// Il margine euclideo dal bordo e' un'approssimazione conservativa
// dell'ingombro come disco di raggio larghezza/2: NON simula la
// sagoma orientata completa o i limiti sportivi FIA.
// Non certifica la validita' dei track limits regolamentari (larghezza vettura,
// cordoli, tolleranze e pista reale NON sono modellati qui).
struct EsitoGeometrico {
    bool coordinateFinite = true;
    std::size_t puntiFuoriSezione = 0;
    std::size_t puntiConMargineInsufficiente = 0;
    std::size_t trattiConMargineInsufficiente = 0;
    std::size_t segmentiDegeneri = 0;
    std::size_t attraversamentiBordo = 0;
    std::size_t autointersezioni = 0;
    double margineMinimoMetri = 0.0;
    double lunghezzaMetri = 0.0;

    bool geometricamenteValida() const {
        return coordinateFinite && puntiFuoriSezione == 0 &&
               puntiConMargineInsufficiente == 0 && trattiConMargineInsufficiente == 0 && segmentiDegeneri == 0 &&
               attraversamentiBordo == 0 && autointersezioni == 0;
    }
};

class ValidatoreGeometrico {
public:
    // Prefiltro O(N), senza indice spaziale: verifica margine sulle sezioni,
    // distacco dai bordi locali dei segmenti, dati finiti e degenerazioni.
    // NON verifica incroci con bordi lontani/autointersezioni globali.
    // I candidati migliori sono SEMPRE controllati con analizza().
    static bool ammissibileRapida(const Traiettoria& traiettoria,
                                 const std::vector<SezionePista>& sezioni,
                                 double margineRichiestoMetri);
    // Stessa risoluzione per traiettoria e sezioni; margine per il centro in metri.
    // Verifica anche incroci fra i segmenti, NON soltanto i geni compresi in [0,1].
    static EsitoGeometrico analizza(const Traiettoria& traiettoria,
                                   const std::vector<SezionePista>& sezioni,
                                   double margineRichiestoMetri = 0.0);
};

#endif
