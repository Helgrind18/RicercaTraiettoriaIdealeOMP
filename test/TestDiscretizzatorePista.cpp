#include "TestDiscretizzatorePista.hpp"

#include "DiscretizzatorePista.hpp"
#include "Pista.hpp"
#include "Punto2D.hpp"
#include "SezionePista.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;


// Funzioni utilizzate solamente all'interno di questo file di test.
namespace {

    // Confronta due double considerando una piccola tolleranza.
    bool doubleUguali(double a, double b) {
        const double epsilon = 0.000001;

        return abs(a - b) < epsilon;
    }


    // Controlla che un Punto2D abbia le coordinate attese.
    bool puntoCorretto(
        const Punto2D& punto,
        double xAttesa,
        double yAttesa
    ) {
        return doubleUguali(punto.getX(), xAttesa) &&
               doubleUguali(punto.getY(), yAttesa);
    }

}


void eseguiTestDiscretizzatorePista() {

    cout << "\n==============================================" << endl;
    cout << "      TEST CLASSE DISCRETIZZATORE PISTA" << endl;
    cout << "==============================================\n" << endl;


    DiscretizzatorePista discretizzatore;


    // ============================================================
    // TEST 1 - PISTA RETTILINEA CON 5 SEZIONI
    // ============================================================

    cout << "[Test 1] Pista rettilinea con 5 sezioni" << endl;

    Pista pistaSemplice;

    pistaSemplice.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaSemplice.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 0.0)
    );


    pistaSemplice.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaSemplice.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );


    vector<SezionePista> sezioni =
        discretizzatore.discretizza(
            pistaSemplice,
            5
        );


    assert(sezioni.size() == 5);


    cout << "Numero sezioni ottenute: "
         << sezioni.size()
         << endl;

    cout << "Numero sezioni attese: 5" << endl;


    // Le coordinate X attese sono:
    //
    // 0
    // 25
    // 50
    // 75
    // 100

    const double coordinateXAttese[5] = {
        0.0,
        25.0,
        50.0,
        75.0,
        100.0
    };


    for (int i = 0; i < 5; i++) {

        const Punto2D& sinistro =
            sezioni[i].getPuntoSinistro();

        const Punto2D& destro =
            sezioni[i].getPuntoDestro();


        cout << "Sezione " << i
             << ": (" << sinistro.getX()
             << "," << sinistro.getY()
             << ") -> ("
             << destro.getX()
             << "," << destro.getY()
             << ")"
             << endl;


        assert(
            puntoCorretto(
                sinistro,
                coordinateXAttese[i],
                0.0
            )
        );


        assert(
            puntoCorretto(
                destro,
                coordinateXAttese[i],
                10.0
            )
        );


        // La pista ha sempre larghezza 10.
        assert(
            doubleUguali(
                sezioni[i].calcolaLarghezza(),
                10.0
            )
        );
    }


    cout << "[OK] Discretizzazione rettilinea corretta.\n"
         << endl;



    // ============================================================
    // TEST 2 - PRIMA, CENTRALE E ULTIMA SEZIONE
    // ============================================================

    cout << "[Test 2] Controllo prima, centrale e ultima sezione"
         << endl;


    // Prima sezione
    assert(
        puntoCorretto(
            sezioni[0].getPuntoSinistro(),
            0.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[0].getPuntoDestro(),
            0.0,
            10.0
        )
    );


    // Sezione centrale
    assert(
        puntoCorretto(
            sezioni[2].getPuntoSinistro(),
            50.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[2].getPuntoDestro(),
            50.0,
            10.0
        )
    );


    // Ultima sezione
    assert(
        puntoCorretto(
            sezioni[4].getPuntoSinistro(),
            100.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[4].getPuntoDestro(),
            100.0,
            10.0
        )
    );


    cout << "Prima sezione:   (0,0) -> (0,10)" << endl;
    cout << "Sezione centrale: (50,0) -> (50,10)" << endl;
    cout << "Ultima sezione:  (100,0) -> (100,10)" << endl;

    cout << "[OK] Estremi e centro corretti.\n"
         << endl;



    // ============================================================
    // TEST 3 - NUMERO DIFFERENTE DI PUNTI NEI DUE BORDI
    // ============================================================

    cout << "[Test 3] Bordi con numero differente di punti"
         << endl;


    Pista pistaBordiDifferenti;


    // Bordo sinistro con 3 punti.
    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(50.0, 0.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 0.0)
    );


    // Bordo destro con solamente 2 punti.
    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );


    vector<SezionePista> sezioniBordiDifferenti =
        discretizzatore.discretizza(
            pistaBordiDifferenti,
            5
        );


    assert(sezioniBordiDifferenti.size() == 5);


    for (int i = 0; i < 5; i++) {

        const Punto2D& sinistro =
            sezioniBordiDifferenti[i].getPuntoSinistro();

        const Punto2D& destro =
            sezioniBordiDifferenti[i].getPuntoDestro();


        cout << "Sezione " << i
             << ": (" << sinistro.getX()
             << "," << sinistro.getY()
             << ") -> ("
             << destro.getX()
             << "," << destro.getY()
             << ")"
             << endl;


        assert(
            puntoCorretto(
                sinistro,
                coordinateXAttese[i],
                0.0
            )
        );


        assert(
            puntoCorretto(
                destro,
                coordinateXAttese[i],
                10.0
            )
        );
    }


    cout << "Punti bordo sinistro: 3" << endl;
    cout << "Punti bordo destro: 2" << endl;

    cout << "[OK] I bordi possono avere numeri differenti di punti.\n"
         << endl;



    // ============================================================
    // TEST 4 - SEGMENTI DI LUNGHEZZA DIFFERENTE
    // ============================================================

    cout << "[Test 4] Segmenti di lunghezza differente" << endl;


    Pista pistaSegmentiDifferenti;


    // Il bordo sinistro contiene un punto a x = 10,
    // ma la metà della sua lunghezza totale rimane x = 50.
    //
    // (0,0) -> (10,0) -----------------> (100,0)

    pistaSegmentiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaSegmentiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(10.0, 0.0)
    );

    pistaSegmentiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 0.0)
    );


    pistaSegmentiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaSegmentiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );


    vector<SezionePista> treSezioni =
        discretizzatore.discretizza(
            pistaSegmentiDifferenti,
            3
        );


    assert(treSezioni.size() == 3);


    // La sezione centrale corrisponde al 50%
    // della lunghezza del bordo.
    const SezionePista& sezioneCentrale =
        treSezioni[1];


    cout << "Punto centrale bordo sinistro: ("
         << sezioneCentrale.getPuntoSinistro().getX()
         << ","
         << sezioneCentrale.getPuntoSinistro().getY()
         << ")"
         << endl;

    cout << "Punto centrale bordo destro: ("
         << sezioneCentrale.getPuntoDestro().getX()
         << ","
         << sezioneCentrale.getPuntoDestro().getY()
         << ")"
         << endl;


    assert(
        puntoCorretto(
            sezioneCentrale.getPuntoSinistro(),
            50.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioneCentrale.getPuntoDestro(),
            50.0,
            10.0
        )
    );


    cout << "[OK] La discretizzazione utilizza la lunghezza del bordo,"
         << endl;

    cout << "     non semplicemente gli indici dei punti.\n"
         << endl;



    // ============================================================
    // TEST 5 - NUMERO DI SEZIONI NON VALIDO
    // ============================================================

    cout << "[Test 5] Numero di sezioni minore di 2" << endl;


    bool eccezioneRicevuta = false;


    try {

        discretizzatore.discretizza(
            pistaSemplice,
            1
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Numero di sezioni non valido correttamente rifiutato.\n"
         << endl;



    // ============================================================
    // TEST 6 - BORDO SINISTRO CON MENO DI DUE PUNTI
    // ============================================================

    cout << "[Test 6] Bordo sinistro con meno di 2 punti"
         << endl;


    Pista pistaBordoInsufficiente;


    pistaBordoInsufficiente.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );


    pistaBordoInsufficiente.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaBordoInsufficiente.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );


    eccezioneRicevuta = false;


    try {

        discretizzatore.discretizza(
            pistaBordoInsufficiente,
            5
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Bordo con numero insufficiente di punti rifiutato.\n"
         << endl;



    // ============================================================
    // TEST 7 - BORDO DI LUNGHEZZA ZERO
    // ============================================================

    cout << "[Test 7] Bordo di lunghezza zero" << endl;


    Pista pistaLunghezzaZero;


    // Due punti uguali:
    // il bordo contiene formalmente due punti,
    // ma la sua lunghezza totale è zero.
    pistaLunghezzaZero.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaLunghezzaZero.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );


    pistaLunghezzaZero.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaLunghezzaZero.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );


    eccezioneRicevuta = false;


    try {

        discretizzatore.discretizza(
            pistaLunghezzaZero,
            5
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Bordo di lunghezza zero correttamente rifiutato.\n"
         << endl;



    // ============================================================
    // FINE TEST
    // ============================================================

    cout << "==============================================" << endl;
    cout << " TUTTI I TEST DISCRETIZZATORE PISTA SUPERATI!" << endl;
    cout << "==============================================\n"
         << endl;
} 