#include "DiscretizzatorePista.hpp"

#include <stdexcept>

using namespace std;


// Discretizza la pista creando il numero richiesto di sezioni.
//
// La pista viene considerata un circuito chiuso.
//
// Per evitare di duplicare la sezione iniziale, le posizioni
// vengono distribuite nell'intervallo [0,1).
//
// Esempio con 5 sezioni:
//
// 0.00
// 0.20
// 0.40
// 0.60
// 0.80
//
// La posizione 1.00 coinciderebbe infatti con 0.00
// perché il circuito è chiuso.
vector<SezionePista> DiscretizzatorePista::discretizza(
    const Pista& pista,
    int numeroSezioni
) const {

    if (numeroSezioni < 2) {
        throw invalid_argument(
            "Il numero di sezioni deve essere almeno 2"
        );
    }


    const vector<Punto2D>& bordoSinistro =
        pista.getBordoSinistro();

    const vector<Punto2D>& bordoDestro =
        pista.getBordoDestro();


    if (bordoSinistro.size() < 2) {
        throw invalid_argument(
            "Il bordo sinistro deve contenere almeno 2 punti"
        );
    }

    if (bordoDestro.size() < 2) {
        throw invalid_argument(
            "Il bordo destro deve contenere almeno 2 punti"
        );
    }


    if (calcolaLunghezzaBordo(bordoSinistro) <= 0.0) {
        throw invalid_argument(
            "Il bordo sinistro deve avere lunghezza maggiore di zero"
        );
    }

    if (calcolaLunghezzaBordo(bordoDestro) <= 0.0) {
        throw invalid_argument(
            "Il bordo destro deve avere lunghezza maggiore di zero"
        );
    }


    vector<SezionePista> sezioni;

    sezioni.reserve(numeroSezioni);


    for (int i = 0; i < numeroSezioni; i++) {

        // IMPORTANTE:
        //
        // Dividiamo per numeroSezioni e NON
        // per numeroSezioni - 1.
        //
        // In questo modo non generiamo mai 1.0,
        // che in un circuito chiuso corrisponderebbe
        // nuovamente al punto iniziale.
        double posizione =
            static_cast<double>(i) /
            static_cast<double>(numeroSezioni);


        Punto2D puntoSinistro =
            calcolaPuntoSulBordo(
                bordoSinistro,
                posizione
            );


        Punto2D puntoDestro =
            calcolaPuntoSulBordo(
                bordoDestro,
                posizione
            );


        SezionePista sezione(
            puntoSinistro,
            puntoDestro
        );


        sezioni.push_back(
            sezione
        );
    }


    return sezioni;
}


// Calcola la lunghezza totale di un bordo chiuso.
//
// Oltre alle distanze tra punti consecutivi:
//
// P0 -> P1
// P1 -> P2
// ...
//
// viene considerata anche:
//
// Pultimo -> P0
//
// Non è quindi necessario duplicare il primo punto
// alla fine del vector.
double DiscretizzatorePista::calcolaLunghezzaBordo(
    const vector<Punto2D>& bordo
) const {

    if (bordo.size() < 2) {
        throw invalid_argument(
            "Per calcolare la lunghezza servono almeno 2 punti"
        );
    }


    double lunghezzaTotale = 0.0;


    // Utilizziamo il modulo per fare in modo che
    // dopo l'ultimo punto venga nuovamente il primo.
    //
    // Esempio con 4 punti:
    //
    // i = 0 -> P0 -> P1
    // i = 1 -> P1 -> P2
    // i = 2 -> P2 -> P3
    // i = 3 -> P3 -> P0
    for (size_t i = 0; i < bordo.size(); i++) {

        const Punto2D& puntoIniziale =
            bordo[i];

        const Punto2D& puntoFinale =
            bordo[(i + 1) % bordo.size()];


        lunghezzaTotale +=
            puntoIniziale.distanza(
                puntoFinale
            );
    }


    return lunghezzaTotale;
}


// Trova il punto corrispondente ad una determinata
// posizione normalizzata lungo un bordo chiuso.
//
// posizione = 0.0 -> primo punto
//
// posizione = 0.5 -> metà della lunghezza totale
//
// posizione = 1.0 -> nuovamente primo punto
//
// Normalmente discretizza() utilizzerà solamente
// valori nell'intervallo [0,1).
Punto2D DiscretizzatorePista::calcolaPuntoSulBordo(
    const vector<Punto2D>& bordo,
    double posizione
) const {

    if (bordo.size() < 2) {
        throw invalid_argument(
            "Il bordo deve contenere almeno 2 punti"
        );
    }


    if (posizione < 0.0 || posizione > 1.0) {
        throw invalid_argument(
            "La posizione deve essere compresa tra 0 e 1"
        );
    }


    double lunghezzaTotale =
        calcolaLunghezzaBordo(
            bordo
        );


    if (lunghezzaTotale <= 0.0) {
        throw invalid_argument(
            "Il bordo deve avere lunghezza maggiore di zero"
        );
    }


    // In un circuito chiuso 0.0 e 1.0
    // rappresentano lo stesso punto.
    if (posizione == 0.0 || posizione == 1.0) {
        return bordo.front();
    }


    double distanzaObiettivo =
        posizione * lunghezzaTotale;

    double distanzaPercorsa = 0.0;


    // Percorriamo tutti i segmenti,
    // compreso quello tra ultimo e primo punto.
    for (size_t i = 0; i < bordo.size(); i++) {

        const Punto2D& puntoIniziale =
            bordo[i];

        const Punto2D& puntoFinale =
            bordo[(i + 1) % bordo.size()];


        double lunghezzaSegmento =
            puntoIniziale.distanza(
                puntoFinale
            );


        // Ignoriamo eventuali segmenti nulli.
        if (lunghezzaSegmento == 0.0) {
            continue;
        }


        if (
            distanzaPercorsa + lunghezzaSegmento
            >= distanzaObiettivo
        ) {

            double distanzaNelSegmento =
                distanzaObiettivo -
                distanzaPercorsa;


            double posizioneNelSegmento =
                distanzaNelSegmento /
                lunghezzaSegmento;


            double x =
                puntoIniziale.getX() +
                posizioneNelSegmento *
                (
                    puntoFinale.getX() -
                    puntoIniziale.getX()
                );


            double y =
                puntoIniziale.getY() +
                posizioneNelSegmento *
                (
                    puntoFinale.getY() -
                    puntoIniziale.getY()
                );


            return Punto2D(
                x,
                y
            );
        }


        distanzaPercorsa +=
            lunghezzaSegmento;
    }


    // Caso di sicurezza.
    //
    // Essendo il circuito chiuso, l'eventuale punto finale
    // coincide con quello iniziale.
    return bordo.front();
}