#include "TestGeneratoreTraiettorie.hpp"

#include "GeneraTraiettorie.hpp"
#include "Traiettoria.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;


void eseguiTestGeneratoreTraiettorie() {

    cout << "\n==============================================" << endl;
    cout << "       TEST GENERATORE TRAIETTORIE" << endl;
    cout << "==============================================\n" << endl;


    // ============================================================
    // TEST 1 - GENERAZIONE DI UNA TRAIETTORIA CASUALE
    // ============================================================

    cout << "[Test 1] Generazione traiettoria casuale" << endl;

    GeneratoreTraiettorie generatore(42);

    Traiettoria traiettoria =
        generatore.generaTraiettoriaCasuale(5);

    assert(traiettoria.getNumeroPosizioni() == 5);

    cout << "Numero posizioni ottenute: "
         << traiettoria.getNumeroPosizioni()
         << endl;

    cout << "Numero posizioni attese: 5" << endl;

    cout << "Posizioni generate: ";

    for (
        double posizione :
        traiettoria.getPosizioniLaterali()
    ) {
        cout << posizione << " ";
    }

    cout << endl;

    cout << "[OK] Traiettoria casuale generata correttamente.\n"
         << endl;


    // ============================================================
    // TEST 2 - CONTROLLO INTERVALLO [0,1]
    // ============================================================

    cout << "[Test 2] Controllo posizioni nell'intervallo [0,1]"
         << endl;

    for (
        double posizione :
        traiettoria.getPosizioniLaterali()
    ) {
        assert(posizione >= 0.0);
        assert(posizione <= 1.0);

        cout << "Posizione "
             << posizione
             << " valida."
             << endl;
    }

    cout << "[OK] Tutte le posizioni sono comprese tra 0 e 1.\n"
         << endl;


    // ============================================================
    // TEST 3 - RIPRODUCIBILITA' DEL SEED
    // ============================================================

    cout << "[Test 3] Riproducibilita' con lo stesso seed" << endl;

    GeneratoreTraiettorie generatoreA(42);
    GeneratoreTraiettorie generatoreB(42);

    Traiettoria traiettoriaA =
        generatoreA.generaTraiettoriaCasuale(10);

    Traiettoria traiettoriaB =
        generatoreB.generaTraiettoriaCasuale(10);

    assert(
        traiettoriaA.getNumeroPosizioni() ==
        traiettoriaB.getNumeroPosizioni()
    );

    for (
        int i = 0;
        i < traiettoriaA.getNumeroPosizioni();
        i++
    ) {

        double posizioneA =
            traiettoriaA.getPosizioneLaterale(i);

        double posizioneB =
            traiettoriaB.getPosizioneLaterale(i);

        cout << "Posizione " << i
             << ": " << posizioneA
             << " | " << posizioneB
             << endl;

        assert(posizioneA == posizioneB);
    }

    cout << "[OK] Lo stesso seed genera la stessa sequenza.\n"
         << endl;


    // ============================================================
    // TEST 4 - GENERAZIONE TRAIETTORIA CENTRALE
    // ============================================================

    cout << "[Test 4] Generazione traiettoria centrale" << endl;

    Traiettoria centrale =
        generatore.generaTraiettoriaCentrale(5);

    assert(centrale.getNumeroPosizioni() == 5);

    cout << "Posizioni della traiettoria centrale: ";

    for (
        double posizione :
        centrale.getPosizioniLaterali()
    ) {

        cout << posizione << " ";

        assert(posizione == 0.5);
    }

    cout << endl;

    cout << "[OK] Tutte le posizioni sono uguali a 0.5.\n"
         << endl;


    // ============================================================
    // TEST 5 - GENERAZIONE POPOLAZIONE
    // ============================================================

    cout << "[Test 5] Generazione popolazione" << endl;

    const int numeroTraiettorie = 10;
    const int numeroSezioni = 20;

    vector<Traiettoria> popolazione =
        generatore.generaPopolazione(
            numeroTraiettorie,
            numeroSezioni
        );

    assert(
        popolazione.size() ==
        static_cast<size_t>(numeroTraiettorie)
    );

    cout << "Numero traiettorie ottenute: "
         << popolazione.size()
         << endl;

    cout << "Numero traiettorie attese: "
         << numeroTraiettorie
         << endl;


    for (
        size_t i = 0;
        i < popolazione.size();
        i++
    ) {

        assert(
            popolazione[i].getNumeroPosizioni()
            == numeroSezioni
        );

        cout << "Traiettoria " << i
             << " -> "
             << popolazione[i].getNumeroPosizioni()
             << " posizioni"
             << endl;
    }

    cout << "[OK] Popolazione generata con dimensioni corrette.\n"
         << endl;


    // ============================================================
    // TEST 6 - CONTROLLO VALORI DELLA POPOLAZIONE
    // ============================================================

    cout << "[Test 6] Controllo valori della popolazione" << endl;

    for (
        const Traiettoria& individuo :
        popolazione
    ) {

        for (
            double posizione :
            individuo.getPosizioniLaterali()
        ) {

            assert(posizione >= 0.0);
            assert(posizione <= 1.0);
        }
    }

    cout << "[OK] Tutte le posizioni della popolazione "
            "sono comprese tra 0 e 1.\n"
         << endl;


    // ============================================================
    // TEST 7 - TRAIETTORIA CASUALE CON 0 SEZIONI
    // ============================================================

    cout << "[Test 7] Traiettoria casuale con 0 sezioni" << endl;

    bool eccezioneRicevuta = false;

    try {

        generatore.generaTraiettoriaCasuale(0);

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Numero di sezioni non valido rifiutato.\n"
         << endl;


    // ============================================================
    // TEST 8 - TRAIETTORIA CENTRALE CON NUMERO NEGATIVO
    // ============================================================

    cout << "[Test 8] Traiettoria centrale con numero negativo"
         << endl;

    eccezioneRicevuta = false;

    try {

        generatore.generaTraiettoriaCentrale(-5);

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Numero di sezioni negativo rifiutato.\n"
         << endl;


    // ============================================================
    // TEST 9 - POPOLAZIONE CON 0 TRAIETTORIE
    // ============================================================

    cout << "[Test 9] Popolazione con 0 traiettorie" << endl;

    eccezioneRicevuta = false;

    try {

        generatore.generaPopolazione(
            0,
            10
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Numero di traiettorie non valido rifiutato.\n"
         << endl;


    // ============================================================
    // TEST 10 - POPOLAZIONE CON 0 SEZIONI
    // ============================================================

    cout << "[Test 10] Popolazione con 0 sezioni" << endl;

    eccezioneRicevuta = false;

    try {

        generatore.generaPopolazione(
            10,
            0
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "[OK] Numero di sezioni non valido rifiutato.\n"
         << endl;


    // ============================================================
    // TEST 11 - POPOLAZIONE RIPRODUCIBILE
    // ============================================================

    cout << "[Test 11] Riproducibilita' dell'intera popolazione"
         << endl;

    GeneratoreTraiettorie generatorePopA(123);
    GeneratoreTraiettorie generatorePopB(123);

    vector<Traiettoria> popolazioneA =
        generatorePopA.generaPopolazione(
            5,
            10
        );

    vector<Traiettoria> popolazioneB =
        generatorePopB.generaPopolazione(
            5,
            10
        );

    assert(
        popolazioneA.size() ==
        popolazioneB.size()
    );


    for (
        size_t i = 0;
        i < popolazioneA.size();
        i++
    ) {

        assert(
            popolazioneA[i].getNumeroPosizioni()
            ==
            popolazioneB[i].getNumeroPosizioni()
        );


        for (
            int j = 0;
            j < popolazioneA[i].getNumeroPosizioni();
            j++
        ) {

            assert(
                popolazioneA[i].getPosizioneLaterale(j)
                ==
                popolazioneB[i].getPosizioneLaterale(j)
            );
        }
    }

    cout << "[OK] Con lo stesso seed viene generata "
            "la stessa popolazione.\n"
         << endl;


    // ============================================================
    // FINE TEST
    // ============================================================

    cout << "==============================================" << endl;
    cout << " TUTTI I TEST GENERATORE TRAIETTORIE SUPERATI!" << endl;
    cout << "==============================================\n"
         << endl;
}