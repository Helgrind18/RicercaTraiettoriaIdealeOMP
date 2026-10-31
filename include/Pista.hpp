#ifndef RICERCATRAIETTORIAIDEALEOMP_PISTA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_PISTA_HPP

#include <vector>
#include "Punto2D.hpp"
using namespace std;
// Una pista è composta da due bordi indipendenti:
// bordo sinistro e bordo destro.
//
// I due bordi possono contenere un numero differente di punti.
class Pista {
private:
    vector<Punto2D> bordoSinistro;
    vector<Punto2D> bordoDestro;

public:
    Pista();

    // Aggiunta dei punti
    void aggiungiPuntoBordoSinistro(const Punto2D& punto);
    void aggiungiPuntoBordoDestro(const Punto2D& punto);

    // Getter dei bordi
    const vector<Punto2D>& getBordoSinistro() const;
    const vector<Punto2D>& getBordoDestro() const;

    // Numero di punti
    int getNumeroPuntiBordoSinistro() const;
    int getNumeroPuntiBordoDestro() const;
    int getNumeroPuntiTotali() const;
};

#endif // RICERCATRAIETTORIAIDEALEOMP_PISTA_HPP
