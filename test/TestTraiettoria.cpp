#include "TestTraiettoria.hpp"

#include "Traiettoria.hpp"
#include "SezionePista.hpp"
#include "Punto2D.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;


namespace {

    bool doubleUguali(double a, double b) {
        const double epsilon = 0.000001;

        return abs(a - b) < epsilon;
    }


    bool puntoCorretto(
        const Punto2D& punto,
        double xAttesa,
        double yAttesa
    ) {
        return doubleUguali(punto.getX(), xAttesa) &&
               doubleUguali(punto.getY(), yAttesa);
    }

}


void eseguiTestTraiettoria() {

    cout << "\n========================================" << endl;
    cout << "       TEST CLASSE TRAIETTORIA" << endl;
    cout << "========================================\n" << endl;


    // ============================================================
    // TEST 1 - COSTRUTTORE DI DEFAULT
    // ============================================================

    cout << "[Test 1] Costruttore di default" << endl;

    Traiettoria traiettoriaDefault;

    assert(traiettoriaDefault.getNumeroPosizioni() == 0);
    assert(traiettoriaDefault.getPosizioniLaterali().empty());

    cout << "Numero posizioni: "
         << traiettoriaDefault.getNumeroPosizioni()
         << endl;

    cout << "Valore atteso: 0" << endl;

    cout << "[OK] Traiettoria inizializzata correttamente vuota.\n"
         << endl;


    // ============================================================
    // TEST 2 - COSTRUTTORE PARAMETRICO
    // ============================================================

    cout << "[Test 2] Costruttore parametrico" << endl;

    vector<double> posizioni = {
        0.50,
        0.20,
        0.80,
        1.00
    };

    Traiettoria traiettoria(posizioni);

    assert(traiettoria.getNumeroPosizioni() == 4);

    cout << "Numero posizioni: "
         << traiettoria.getNumeroPosizioni()
         << endl;

    cout << "Posizioni: ";

    for (double posizione : traiettoria.getPosizioniLaterali()) {
        cout << posizione << " ";
    }

    cout << endl;

    assert(doubleUguali(traiettoria.getPosizioneLaterale(0), 0.50));
    assert(doubleUguali(traiettoria.getPosizioneLaterale(1), 0.20));
    assert(doubleUguali(traiettoria.getPosizioneLaterale(2), 0.80));
    assert(doubleUguali(traiettoria.getPosizioneLaterale(3), 1.00));

    cout << "[OK] Costruttore parametrico corretto.\n"
         << endl;


    // ============================================================
    // TEST 3 - AGGIUNTA POSIZIONE
    // ============================================================

    cout << "[Test 3] Aggiunta posizione laterale" << endl;

    Traiettoria traiettoriaAggiunta;

    traiettoriaAggiunta.aggiungiPosizioneLaterale(0.25);
    traiettoriaAggiunta.aggiungiPosizioneLaterale(0.50);
    traiettoriaAggiunta.aggiungiPosizioneLaterale(0.75);

    assert(traiettoriaAggiunta.getNumeroPosizioni() == 3);

    assert(
        doubleUguali(
            traiettoriaAggiunta.getPosizioneLaterale(0),
            0.25
        )
    );

    assert(
        doubleUguali(
            traiettoriaAggiunta.getPosizioneLaterale(1),
            0.50
        )
    );

    assert(
        doubleUguali(
            traiettoriaAggiunta.getPosizioneLaterale(2),
            0.75
        )
    );

    cout << "Posizioni aggiunte: ";

    for (
        double posizione :
        traiettoriaAggiunta.getPosizioniLaterali()
    ) {
        cout << posizione << " ";
    }

    cout << endl;

    cout << "[OK] Posizioni aggiunte correttamente.\n"
         << endl;


    // ============================================================
    // TEST 4 - MODIFICA POSIZIONE
    // ============================================================

    cout << "[Test 4] Modifica posizione laterale" << endl;

    cout << "Posizione precedente indice 1: "
         << traiettoriaAggiunta.getPosizioneLaterale(1)
         << endl;

    traiettoriaAggiunta.setPosizioneLaterale(
        1,
        0.35
    );

    cout << "Nuova posizione indice 1: "
         << traiettoriaAggiunta.getPosizioneLaterale(1)
         << endl;

    assert(
        doubleUguali(
            traiettoriaAggiunta.getPosizioneLaterale(1),
            0.35
        )
    );

    cout << "[OK] Posizione modificata correttamente.\n"
         << endl;


    // ============================================================
    // TEST 5 - COSTRUTTORE CON POSIZIONE MINORE DI ZERO
    // ============================================================

    cout << "[Test 5] Costruttore con posizione negativa" << endl;

    bool eccezioneRicevuta = false;

    try {

        vector<double> posizioniErrate = {
            0.5,
            -0.2,
            0.8
        };

        Traiettoria traiettoriaErrata(
            posizioniErrate
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Posizione negativa correttamente rifiutata.\n"
         << endl;


    // ============================================================
    // TEST 6 - COSTRUTTORE CON POSIZIONE MAGGIORE DI 1
    // ============================================================

    cout << "[Test 6] Costruttore con posizione maggiore di 1"
         << endl;

    eccezioneRicevuta = false;

    try {

        vector<double> posizioniErrate = {
            0.5,
            1.2,
            0.8
        };

        Traiettoria traiettoriaErrata(
            posizioniErrate
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Posizione maggiore di 1 correttamente rifiutata.\n"
         << endl;


    // ============================================================
    // TEST 7 - AGGIUNTA POSIZIONE NON VALIDA
    // ============================================================

    cout << "[Test 7] Aggiunta posizione non valida" << endl;

    eccezioneRicevuta = false;

    try {

        traiettoriaAggiunta.aggiungiPosizioneLaterale(
            1.5
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Aggiunta non valida correttamente rifiutata.\n"
         << endl;


    // ============================================================
    // TEST 8 - ACCESSO CON INDICE NON VALIDO
    // ============================================================

    cout << "[Test 8] Accesso con indice fuori range" << endl;

    eccezioneRicevuta = false;

    try {

        traiettoria.getPosizioneLaterale(10);

    }
    catch (const out_of_range& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Indice fuori range correttamente rifiutato.\n"
         << endl;


    // ============================================================
    // TEST 9 - MODIFICA CON INDICE NON VALIDO
    // ============================================================

    cout << "[Test 9] Modifica con indice fuori range" << endl;

    eccezioneRicevuta = false;

    try {

        traiettoria.setPosizioneLaterale(
            10,
            0.5
        );

    }
    catch (const out_of_range& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Modifica con indice fuori range rifiutata.\n"
         << endl;


    // ============================================================
    // TEST 10 - MODIFICA CON POSIZIONE NON VALIDA
    // ============================================================

    cout << "[Test 10] Modifica con posizione non valida" << endl;

    eccezioneRicevuta = false;

    try {

        traiettoria.setPosizioneLaterale(
            0,
            -0.5
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Nuova posizione non valida rifiutata.\n"
         << endl;


    // ============================================================
    // TEST 11 - CALCOLO DEI PUNTI REALI DELLA TRAIETTORIA
    // ============================================================

    cout << "[Test 11] Conversione traiettoria in Punto2D" << endl;


    // Creiamo tre sezioni verticali:
    //
    // Sezione 0: (0,0)  -> (10,0)
    // Sezione 1: (0,10) -> (10,10)
    // Sezione 2: (0,20) -> (10,20)

    vector<SezionePista> sezioni = {

        SezionePista(
            Punto2D(0.0, 0.0),
            Punto2D(10.0, 0.0)
        ),

        SezionePista(
            Punto2D(0.0, 10.0),
            Punto2D(10.0, 10.0)
        ),

        SezionePista(
            Punto2D(0.0, 20.0),
            Punto2D(10.0, 20.0)
        )
    };


    // Posizioni:
    //
    // 0.5 -> centro della prima sezione
    // 0.2 -> 20% della seconda sezione
    // 0.8 -> 80% della terza sezione

    vector<double> posizioniPunti = {
        0.5,
        0.2,
        0.8
    };


    Traiettoria traiettoriaPunti(
        posizioniPunti
    );


    vector<Punto2D> punti =
        traiettoriaPunti.calcolaPunti(
            sezioni
        );


    assert(punti.size() == 3);


    // Ci aspettiamo:
    //
    // (5,0)
    // (2,10)
    // (8,20)

    assert(
        puntoCorretto(
            punti[0],
            5.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            punti[1],
            2.0,
            10.0
        )
    );

    assert(
        puntoCorretto(
            punti[2],
            8.0,
            20.0
        )
    );


    for (size_t i = 0; i < punti.size(); i++) {

        cout << "Punto traiettoria " << i
             << ": ("
             << punti[i].getX()
             << ","
             << punti[i].getY()
             << ")"
             << endl;
    }


    cout << "Valori attesi:" << endl;
    cout << "  (5,0)" << endl;
    cout << "  (2,10)" << endl;
    cout << "  (8,20)" << endl;

    cout << "[OK] Coordinate della traiettoria corrette.\n"
         << endl;


    // ============================================================
    // TEST 12 - NUMERO SEZIONI DIVERSO DAL NUMERO DI POSIZIONI
    // ============================================================

    cout << "[Test 12] Numero sezioni diverso dal numero di posizioni"
         << endl;


    vector<SezionePista> sezioniErrate = {

        SezionePista(
            Punto2D(0.0, 0.0),
            Punto2D(10.0, 0.0)
        ),

        SezionePista(
            Punto2D(0.0, 10.0),
            Punto2D(10.0, 10.0)
        )
    };


    eccezioneRicevuta = false;


    try {

        traiettoriaPunti.calcolaPunti(
            sezioniErrate
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Numero incoerente di sezioni correttamente rifiutato.\n"
         << endl;


    // ============================================================
    // FINE TEST
    // ============================================================

    cout << "========================================" << endl;
    cout << "   TUTTI I TEST TRAIETTORIA SUPERATI!" << endl;
    cout << "========================================\n"
         << endl;
}