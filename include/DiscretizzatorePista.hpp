#ifndef RICERCATRAIETTORIAIDEALEOMP_DISCRETIZZATOREPISTA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_DISCRETIZZATOREPISTA_HPP

#include "Pista.hpp"
#include "SezionePista.hpp"

#include <vector>
using namespace std;
// Trasforma una Pista composta da due bordi indipendenti
// in una sequenza ordinata di SezionePista
//
// I bordi originali possono avere numeri differenti di punti
// Il discretizzatore interpola entrambi i bordi in modo da
// ottenere lo stesso numero di punti e creare sezioni trasversali
// corrispondenti lungo tutta la pista
class DiscretizzatorePista {
public:
    // Discretizza la pista creando il numero richiesto di sezioni
    vector<SezionePista> discretizza(const Pista &pista,int numeroSezioni) const;

private:
    // Calcola la lunghezza totale di un bordo sommando
    // le distanze tra tutti i punti consecutivi
    double calcolaLunghezzaBordo(const vector<Punto2D> &bordo) const;


    // Restituisce il Punto2D che si trova ad una determinata
    // percentuale della lunghezza totale del bordo
    //
    // posizione = 0.0 -> inizio bordo
    // posizione = 0.5 -> metà del bordo
    // posizione = 1.0 -> fine bordo
    Punto2D calcolaPuntoSulBordo(const vector<Punto2D> &bordo,double posizione) const;
};

#endif
