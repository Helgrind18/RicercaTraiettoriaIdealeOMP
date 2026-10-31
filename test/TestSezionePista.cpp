#include "TestSezionePista.hpp"
#include "SezionePista.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace std;


// Funzione di supporto per confrontare due double.
// Evitiamo di confrontare direttamente i double con ==
// quando sono il risultato di calcoli matematici.
bool doubleUguali(double a, double b) {
    const double epsilon = 0.000001;

    return abs(a - b) < epsilon;
}


void eseguiTestSezionePista() {

    cout << "\n========================================" << endl;
    cout << "       TEST CLASSE SEZIONE PISTA" << endl;
    cout << "========================================\n" << endl;


    // ============================================================
    // TEST 1 - COSTRUTTORE DI DEFAULT
    // ============================================================

    cout << "[TEST 1] Costruttore di default" << endl;

    SezionePista sezioneDefault;

    const Punto2D& sinistroDefault =
        sezioneDefault.getPuntoSinistro();

    const Punto2D& destroDefault =
        sezioneDefault.getPuntoDestro();

    assert(sinistroDefault.getX() == 0.0);
    assert(sinistroDefault.getY() == 0.0);

    assert(destroDefault.getX() == 0.0);
    assert(destroDefault.getY() == 0.0);

    cout << "  Punto sinistro: ("
         << sinistroDefault.getX()
         << ", "
         << sinistroDefault.getY()
         << ")"
         << endl;

    cout << "  Punto destro: ("
         << destroDefault.getX()
         << ", "
         << destroDefault.getY()
         << ")"
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 2 - COSTRUTTORE PARAMETRICO
    // ============================================================

    cout << "[TEST 2] Costruttore parametrico" << endl;

    Punto2D puntoSinistro(0.0, 0.0);
    Punto2D puntoDestro(10.0, 0.0);

    SezionePista sezione(
        puntoSinistro,
        puntoDestro
    );

    assert(sezione.getPuntoSinistro().getX() == 0.0);
    assert(sezione.getPuntoSinistro().getY() == 0.0);

    assert(sezione.getPuntoDestro().getX() == 10.0);
    assert(sezione.getPuntoDestro().getY() == 0.0);

    cout << "  Punto sinistro: ("
         << sezione.getPuntoSinistro().getX()
         << ", "
         << sezione.getPuntoSinistro().getY()
         << ")"
         << endl;

    cout << "  Punto destro: ("
         << sezione.getPuntoDestro().getX()
         << ", "
         << sezione.getPuntoDestro().getY()
         << ")"
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 3 - SETTER
    // ============================================================

    cout << "[TEST 3] Setter dei punti" << endl;

    Punto2D nuovoSinistro(2.0, 3.0);
    Punto2D nuovoDestro(8.0, 3.0);

    sezione.setPuntoSinistro(nuovoSinistro);
    sezione.setPuntoDestro(nuovoDestro);

    assert(sezione.getPuntoSinistro().getX() == 2.0);
    assert(sezione.getPuntoSinistro().getY() == 3.0);

    assert(sezione.getPuntoDestro().getX() == 8.0);
    assert(sezione.getPuntoDestro().getY() == 3.0);

    cout << "  Nuovo punto sinistro: ("
         << sezione.getPuntoSinistro().getX()
         << ", "
         << sezione.getPuntoSinistro().getY()
         << ")"
         << endl;

    cout << "  Nuovo punto destro: ("
         << sezione.getPuntoDestro().getX()
         << ", "
         << sezione.getPuntoDestro().getY()
         << ")"
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 4 - CALCOLO LARGHEZZA ORIZZONTALE
    // ============================================================

    cout << "[TEST 4] Calcolo larghezza sezione orizzontale" << endl;

    // Sinistro = (2,3)
    // Destro   = (8,3)
    // Distanza = 6

    double larghezza = sezione.calcolaLarghezza();

    assert(doubleUguali(larghezza, 6.0));

    cout << "  Larghezza calcolata: "
         << larghezza
         << endl;

    cout << "  Larghezza attesa: 6" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 5 - CALCOLO LARGHEZZA SEZIONE INCLINATA
    // ============================================================

    cout << "[TEST 5] Calcolo larghezza sezione inclinata" << endl;

    // Triangolo 3-4-5:
    //
    // Sinistro = (0,0)
    // Destro   = (3,4)
    //
    // distanza = sqrt(3^2 + 4^2) = 5

    SezionePista sezioneInclinata(
        Punto2D(0.0, 0.0),
        Punto2D(3.0, 4.0)
    );

    double larghezzaInclinata =
        sezioneInclinata.calcolaLarghezza();

    assert(
        doubleUguali(
            larghezzaInclinata,
            5.0
        )
    );

    cout << "  Larghezza calcolata: "
         << larghezzaInclinata
         << endl;

    cout << "  Larghezza attesa: 5" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 6 - PUNTO INTERNO IN POSIZIONE 0
    // ============================================================

    cout << "[TEST 6] Punto interno con posizione = 0" << endl;

    SezionePista sezioneInterna(
        Punto2D(0.0, 0.0),
        Punto2D(10.0, 0.0)
    );

    Punto2D punto0 =
        sezioneInterna.calcolaPuntoInterno(0.0);

    assert(doubleUguali(punto0.getX(), 0.0));
    assert(doubleUguali(punto0.getY(), 0.0));

    cout << "  Punto calcolato: ("
         << punto0.getX()
         << ", "
         << punto0.getY()
         << ")"
         << endl;

    cout << "  Punto atteso: (0, 0)" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 7 - PUNTO INTERNO IN POSIZIONE 0.5
    // ============================================================

    cout << "[TEST 7] Punto interno con posizione = 0.5" << endl;

    Punto2D puntoCentro =
        sezioneInterna.calcolaPuntoInterno(0.5);

    assert(doubleUguali(puntoCentro.getX(), 5.0));
    assert(doubleUguali(puntoCentro.getY(), 0.0));

    cout << "  Punto calcolato: ("
         << puntoCentro.getX()
         << ", "
         << puntoCentro.getY()
         << ")"
         << endl;

    cout << "  Punto atteso: (5, 0)" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 8 - PUNTO INTERNO IN POSIZIONE 1
    // ============================================================

    cout << "[TEST 8] Punto interno con posizione = 1" << endl;

    Punto2D punto1 =
        sezioneInterna.calcolaPuntoInterno(1.0);

    assert(doubleUguali(punto1.getX(), 10.0));
    assert(doubleUguali(punto1.getY(), 0.0));

    cout << "  Punto calcolato: ("
         << punto1.getX()
         << ", "
         << punto1.getY()
         << ")"
         << endl;

    cout << "  Punto atteso: (10, 0)" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 9 - PUNTO INTERNO IN POSIZIONE 0.25
    // ============================================================

    cout << "[TEST 9] Punto interno con posizione = 0.25" << endl;

    Punto2D punto25 =
        sezioneInterna.calcolaPuntoInterno(0.25);

    assert(doubleUguali(punto25.getX(), 2.5));
    assert(doubleUguali(punto25.getY(), 0.0));

    cout << "  Punto calcolato: ("
         << punto25.getX()
         << ", "
         << punto25.getY()
         << ")"
         << endl;

    cout << "  Punto atteso: (2.5, 0)" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 10 - INTERPOLAZIONE SU SEZIONE INCLINATA
    // ============================================================

    cout << "[TEST 10] Punto interno su sezione inclinata" << endl;

    // Sinistro = (2,4)
    // Destro   = (10,8)
    //
    // A posizione 0.5 ci aspettiamo:
    // x = 6
    // y = 6

    SezionePista sezioneDiagonale(
        Punto2D(2.0, 4.0),
        Punto2D(10.0, 8.0)
    );

    Punto2D puntoDiagonale =
        sezioneDiagonale.calcolaPuntoInterno(0.5);

    assert(
        doubleUguali(
            puntoDiagonale.getX(),
            6.0
        )
    );

    assert(
        doubleUguali(
            puntoDiagonale.getY(),
            6.0
        )
    );

    cout << "  Punto calcolato: ("
         << puntoDiagonale.getX()
         << ", "
         << puntoDiagonale.getY()
         << ")"
         << endl;

    cout << "  Punto atteso: (6, 6)" << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 11 - POSIZIONE NEGATIVA
    // ============================================================

    cout << "[TEST 11] Posizione minore di 0" << endl;

    bool eccezioneRicevuta = false;

    try {
        sezioneInterna.calcolaPuntoInterno(-0.1);
    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "  Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 12 - POSIZIONE MAGGIORE DI 1
    // ============================================================

    cout << "[TEST 12] Posizione maggiore di 1" << endl;

    eccezioneRicevuta = false;

    try {
        sezioneInterna.calcolaPuntoInterno(1.1);
    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "  Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }

    assert(eccezioneRicevuta);

    cout << "  -> OK\n" << endl;


    // ============================================================
    // FINE TEST
    // ============================================================

    cout << "========================================" << endl;
    cout << "  TUTTI I TEST SEZIONE PISTA SUPERATI" << endl;
    cout << "========================================\n" << endl;
}