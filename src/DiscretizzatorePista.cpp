#include "DiscretizzatorePista.hpp"

#include <cmath>
#include <stdexcept>

using namespace std;


// Discretizza la pista creando il numero richiesto di sezioni.
//
// Per ogni sezione viene calcolata una posizione compresa
// tra 0 e 1 lungo entrambi i bordi.
//
// Esempio con 5 sezioni:
// 0.00
// 0.25
// 0.50
// 0.75
// 1.00
//
// Per ogni posizione viene trovato un punto sul bordo sinistro
// e un punto sul bordo destro. I due punti vengono poi utilizzati
// per creare una SezionePista.
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


    // Per poter interpolare un bordo servono almeno due punti.
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


    // Controlliamo che entrambi i bordi abbiano
    // una lunghezza maggiore di zero.
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

    // Sappiamo già quante sezioni dovremo inserire.
    sezioni.reserve(numeroSezioni);


    for (int i = 0; i < numeroSezioni; i++) {

        // Calcola una posizione normalizzata tra 0 e 1.
        //
        // Con 5 sezioni:
        // i = 0 -> 0.00
        // i = 1 -> 0.25
        // i = 2 -> 0.50
        // i = 3 -> 0.75
        // i = 4 -> 1.00
        double posizione =
            static_cast<double>(i) /
            static_cast<double>(numeroSezioni - 1);


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


        sezioni.push_back(sezione);
    }


    return sezioni;
}


// Calcola la lunghezza totale di un bordo.
//
// La lunghezza totale è data dalla somma delle distanze
// tra tutte le coppie di punti consecutivi.
//
// Esempio:
//
// P0 ---- P1 -------- P2
//
// lunghezza = distanza(P0,P1) + distanza(P1,P2)
double DiscretizzatorePista::calcolaLunghezzaBordo(
    const vector<Punto2D>& bordo
) const {

    if (bordo.size() < 2) {
        throw invalid_argument(
            "Per calcolare la lunghezza servono almeno 2 punti"
        );
    }


    double lunghezzaTotale = 0.0;


    for (size_t i = 0; i < bordo.size() - 1; i++) {

        double differenzaX =
            bordo[i + 1].getX() -
            bordo[i].getX();

        double differenzaY =
            bordo[i + 1].getY() -
            bordo[i].getY();


        double lunghezzaSegmento =
            sqrt(
                differenzaX * differenzaX +
                differenzaY * differenzaY
            );


        lunghezzaTotale += lunghezzaSegmento;
    }


    return lunghezzaTotale;
}


// Trova il punto che si trova ad una determinata percentuale
// della lunghezza totale del bordo.
//
// posizione = 0.0 -> primo punto
// posizione = 0.5 -> metà della lunghezza del bordo
// posizione = 1.0 -> ultimo punto
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
        calcolaLunghezzaBordo(bordo);


    if (lunghezzaTotale <= 0.0) {
        throw invalid_argument(
            "Il bordo deve avere lunghezza maggiore di zero"
        );
    }


    // Caso particolare: inizio del bordo.
    if (posizione == 0.0) {
        return bordo.front();
    }


    // Caso particolare: fine del bordo.
    if (posizione == 1.0) {
        return bordo.back();
    }


    // Trasformiamo la posizione normalizzata in una
    // vera distanza lungo il bordo.
    //
    // Esempio:
    // lunghezzaTotale = 100
    // posizione = 0.25
    //
    // distanzaObiettivo = 25
    double distanzaObiettivo =
        posizione * lunghezzaTotale;


    double distanzaPercorsa = 0.0;


    for (size_t i = 0; i < bordo.size() - 1; i++) {

        const Punto2D& puntoIniziale =
            bordo[i];

        const Punto2D& puntoFinale =
            bordo[i + 1];


        double differenzaX =
            puntoFinale.getX() -
            puntoIniziale.getX();

        double differenzaY =
            puntoFinale.getY() -
            puntoIniziale.getY();


        double lunghezzaSegmento =
            sqrt(
                differenzaX * differenzaX +
                differenzaY * differenzaY
            );


        // Se due punti consecutivi coincidono,
        // il segmento ha lunghezza zero e viene ignorato.
        if (lunghezzaSegmento == 0.0) {
            continue;
        }


        // Controlliamo se il punto cercato si trova
        // all'interno del segmento corrente.
        if (
            distanzaPercorsa + lunghezzaSegmento
            >= distanzaObiettivo
        ) {

            // Distanza da percorrere all'interno
            // del segmento corrente.
            double distanzaNelSegmento =
                distanzaObiettivo -
                distanzaPercorsa;


            // Posizione normalizzata all'interno
            // del singolo segmento.
            //
            // 0 -> punto iniziale
            // 1 -> punto finale
            double posizioneNelSegmento =
                distanzaNelSegmento /
                lunghezzaSegmento;


            // Interpolazione lineare della coordinata X.
            double x =
                puntoIniziale.getX() +
                posizioneNelSegmento *
                (
                    puntoFinale.getX() -
                    puntoIniziale.getX()
                );


            // Interpolazione lineare della coordinata Y.
            double y =
                puntoIniziale.getY() +
                posizioneNelSegmento *
                (
                    puntoFinale.getY() -
                    puntoIniziale.getY()
                );


            return Punto2D(x, y);
        }


        // Il punto non si trova in questo segmento,
        // quindi continuiamo con quello successivo.
        distanzaPercorsa += lunghezzaSegmento;
    }


    // Teoricamente non dovremmo mai arrivare qui.
    // Per sicurezza restituiamo l'ultimo punto del bordo.
    return bordo.back();
}