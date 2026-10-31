#ifndef RICERCATRAIETTORIAIDEALEOMP_SEZIONEPISTA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_SEZIONEPISTA_HPP

#include "Punto2D.hpp"
// Rappresenta una sezione trasversale della pista.
// Ogni sezione è definita da due punti:
// - un punto appartenente al bordo sinistro
// - un punto appartenente al bordo destro
//
// La sezione permette di conoscere la larghezza della pista
// in quel punto e di calcolare una posizione interna compresa
// tra i due bordi.
//
// Questa classe verrà usata successivamente per discretizzare
// la pista e per costruire le possibili traiettorie del veicolo.
class SezionePista {
private:
    // Punto appartenente al bordo sinistro della pista.
    Punto2D puntoSinistro;

    // Punto appartenente al bordo destro della pista.
    Punto2D puntoDestro;

public:
    SezionePista();

    SezionePista(
        const Punto2D& puntoSinistro,
        const Punto2D& puntoDestro
    );

    const Punto2D& getPuntoSinistro() const;

    const Punto2D& getPuntoDestro() const;

    void setPuntoSinistro(const Punto2D& puntoSinistro);

    void setPuntoDestro(const Punto2D& puntoDestro);

    // Calcola la distanza tra il bordo sinistro e il bordo destro
    // della sezione.
    double calcolaLarghezza() const;

    // Restituisce un punto interno alla sezione.
    //
    // posizione = 0.0 -> bordo sinistro
    // posizione = 0.5 -> centro della sezione
    // posizione = 1.0 -> bordo destro
    //
    // Se posizione non è compresa tra 0 e 1
    // viene generata un'eccezione.
    Punto2D calcolaPuntoInterno(double posizione) const;
};

#endif