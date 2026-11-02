#ifndef RICERCATRAIETTORIAIDEALEOMP_TRAIETTORIA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_TRAIETTORIA_HPP

#include "Punto2D.hpp"
#include "SezionePista.hpp"

#include <vector>

using namespace std;

// Rappresenta una possibile traiettoria percorsa dal veicolo
// lungo la pista.
//
// La traiettoria viene rappresentata tramite un insieme di
// posizioni laterali, una per ogni SezionePista.
//
// Ogni posizione è compresa tra 0 e 1:
//
// 0.0 = bordo sinistro
// 0.5 = centro della pista
// 1.0 = bordo destro
//
// Esempio:
//
// Sezione 0 -> 0.50
// Sezione 1 -> 0.35
// Sezione 2 -> 0.20
// Sezione 3 -> 0.70
//
// Questi valori potranno successivamente essere modificati
// dall'algoritmo genetico tramite crossover e mutazione.
class Traiettoria {
private:

    // Posizione laterale del veicolo per ogni sezione della pista.
    vector<double> posizioniLaterali;

public:

    // Costruttore di default.
    // Crea una traiettoria senza posizioni.
    Traiettoria();

    // Costruisce una traiettoria utilizzando le posizioni
    // laterali specificate.
    Traiettoria(
        const vector<double>& posizioniLaterali
    );

    // Restituisce tutte le posizioni laterali senza copiarle.
    // Il riferimento è const, quindi non possono essere
    // modificate direttamente dall'esterno.
    const vector<double>& getPosizioniLaterali() const;

    // Restituisce la posizione laterale relativa
    // alla sezione indicata.
    double getPosizioneLaterale(int indice) const;

    // Restituisce il numero di posizioni presenti
    // nella traiettoria.
    int getNumeroPosizioni() const;

    // Aggiunge una nuova posizione laterale alla traiettoria.
    //
    // Il valore deve essere compreso tra 0 e 1.
    void aggiungiPosizioneLaterale(double posizione);

    // Modifica una posizione laterale già presente.
    //
    // L'indice deve essere valido.
    // La posizione deve essere compresa tra 0 e 1.
    void setPosizioneLaterale(
        int indice,
        double posizione
    );

    // Trasforma le posizioni laterali della traiettoria
    // nei corrispondenti punti 2D della pista.
    //
    // Il numero di sezioni deve coincidere con il numero
    // di posizioni laterali della traiettoria.
    vector<Punto2D> calcolaPunti(
        const vector<SezionePista>& sezioni
    ) const;
};

#endif