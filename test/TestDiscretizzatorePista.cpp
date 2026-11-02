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


// Funzioni utilizzate solamente all'interno
// di questo file di test.
namespace {

    bool doubleUguali(
        double a,
        double b
    ) {

        const double epsilon = 0.000001;

        return abs(a - b) < epsilon;
    }


    bool puntoCorretto(
        const Punto2D& punto,
        double xAttesa,
        double yAttesa
    ) {

        return
            doubleUguali(
                punto.getX(),
                xAttesa
            )
            &&
            doubleUguali(
                punto.getY(),
                yAttesa
            );
    }

}


void eseguiTestDiscretizzatorePista() {

    cout << "\n==============================================" << endl;
    cout << "      TEST CLASSE DISCRETIZZATORE PISTA" << endl;
    cout << "==============================================\n" << endl;


    DiscretizzatorePista discretizzatore;


    // ============================================================
    // TEST 1 - CIRCUITO CHIUSO CON 4 SEZIONI
    // ============================================================

    cout << "[Test 1] Circuito chiuso con 4 sezioni" << endl;


    Pista pistaQuadrata;


    // Bordo sinistro:
    //
    // (0,100) -------- (100,100)
    //    |                 |
    //    |                 |
    // (0,0) ---------- (100,0)

    pistaQuadrata.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaQuadrata.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 0.0)
    );

    pistaQuadrata.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 100.0)
    );

    pistaQuadrata.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 100.0)
    );


    // Bordo destro:
    //
    // Stessa forma, traslata di 10 unità verso l'alto.

    pistaQuadrata.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaQuadrata.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );

    pistaQuadrata.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 110.0)
    );

    pistaQuadrata.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 110.0)
    );


    vector<SezionePista> sezioni =
        discretizzatore.discretizza(
            pistaQuadrata,
            4
        );


    assert(sezioni.size() == 4);


    cout << "Numero sezioni ottenute: "
         << sezioni.size()
         << endl;

    cout << "Numero sezioni attese: 4" << endl;


    // Con 4 sezioni le posizioni normalizzate sono:
    //
    // 0.00
    // 0.25
    // 0.50
    // 0.75
    //
    // Essendo il quadrato composto da 4 lati uguali,
    // otteniamo esattamente i quattro vertici.


    assert(
        puntoCorretto(
            sezioni[0].getPuntoSinistro(),
            0.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[1].getPuntoSinistro(),
            100.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[2].getPuntoSinistro(),
            100.0,
            100.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[3].getPuntoSinistro(),
            0.0,
            100.0
        )
    );


    assert(
        puntoCorretto(
            sezioni[0].getPuntoDestro(),
            0.0,
            10.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[1].getPuntoDestro(),
            100.0,
            10.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[2].getPuntoDestro(),
            100.0,
            110.0
        )
    );

    assert(
        puntoCorretto(
            sezioni[3].getPuntoDestro(),
            0.0,
            110.0
        )
    );


    for (size_t i = 0; i < sezioni.size(); i++) {

        const Punto2D& sinistro =
            sezioni[i].getPuntoSinistro();

        const Punto2D& destro =
            sezioni[i].getPuntoDestro();


        cout << "Sezione " << i
             << ": ("
             << sinistro.getX()
             << ","
             << sinistro.getY()
             << ") -> ("
             << destro.getX()
             << ","
             << destro.getY()
             << ")"
             << endl;
    }


    cout << "[OK] Circuito chiuso discretizzato correttamente.\n"
         << endl;



    // ============================================================
    // TEST 2 - NESSUNA DUPLICAZIONE DEL PUNTO INIZIALE
    // ============================================================

    cout << "[Test 2] Nessuna duplicazione della sezione iniziale"
         << endl;


    const SezionePista& primaSezione =
        sezioni.front();

    const SezionePista& ultimaSezione =
        sezioni.back();


    // La prima sezione è a posizione 0.0.
    assert(
        puntoCorretto(
            primaSezione.getPuntoSinistro(),
            0.0,
            0.0
        )
    );


    // L'ultima è a posizione 0.75,
    // quindi NON deve coincidere con la prima.
    assert(
        !puntoCorretto(
            ultimaSezione.getPuntoSinistro(),
            0.0,
            0.0
        )
    );


    cout << "Prima sezione: ("
         << primaSezione.getPuntoSinistro().getX()
         << ","
         << primaSezione.getPuntoSinistro().getY()
         << ")"
         << endl;

    cout << "Ultima sezione: ("
         << ultimaSezione.getPuntoSinistro().getX()
         << ","
         << ultimaSezione.getPuntoSinistro().getY()
         << ")"
         << endl;


    cout << "[OK] Il punto iniziale non viene duplicato.\n"
         << endl;



    // ============================================================
    // TEST 3 - 8 SEZIONI: VERTICI E PUNTI MEDI
    // ============================================================

    cout << "[Test 3] Circuito quadrato con 8 sezioni" << endl;


    vector<SezionePista> ottoSezioni =
        discretizzatore.discretizza(
            pistaQuadrata,
            8
        );


    assert(ottoSezioni.size() == 8);


    // Perimetro = 400
    //
    // Ogni sezione è distante:
    //
    // 400 / 8 = 50
    //
    // Quindi sul bordo sinistro ci aspettiamo:
    //
    // (0,0)
    // (50,0)
    // (100,0)
    // (100,50)
    // (100,100)
    // (50,100)
    // (0,100)
    // (0,50)

    const double xAttese[8] = {
        0.0,
        50.0,
        100.0,
        100.0,
        100.0,
        50.0,
        0.0,
        0.0
    };

    const double yAttese[8] = {
        0.0,
        0.0,
        0.0,
        50.0,
        100.0,
        100.0,
        100.0,
        50.0
    };


    for (int i = 0; i < 8; i++) {

        const Punto2D& punto =
            ottoSezioni[i].getPuntoSinistro();


        cout << "Sezione " << i
             << ": ("
             << punto.getX()
             << ","
             << punto.getY()
             << ")"
             << endl;


        assert(
            puntoCorretto(
                punto,
                xAttese[i],
                yAttese[i]
            )
        );
    }


    cout << "[OK] Interpolazione lungo tutto il circuito corretta.\n"
         << endl;



    // ============================================================
    // TEST 4 - CONTROLLO DEL SEGMENTO DI CHIUSURA
    // ============================================================

    cout << "[Test 4] Utilizzo del segmento ultimo -> primo" << endl;


    // Con 8 sezioni, l'ultima posizione è:
    //
    // 7 / 8 = 0.875
    //
    // Sul quadrato corrisponde a metà del segmento:
    //
    // (0,100) -> (0,0)
    //
    // quindi deve essere:
    //
    // (0,50)

    const Punto2D& ultimoPunto =
        ottoSezioni[7].getPuntoSinistro();


    cout << "Ultimo punto discretizzato: ("
         << ultimoPunto.getX()
         << ","
         << ultimoPunto.getY()
         << ")"
         << endl;


    assert(
        puntoCorretto(
            ultimoPunto,
            0.0,
            50.0
        )
    );


    cout << "[OK] Il segmento tra ultimo e primo punto viene utilizzato.\n"
         << endl;



    // ============================================================
    // TEST 5 - NUMERO DIFFERENTE DI PUNTI SUI DUE BORDI
    // ============================================================

    cout << "[Test 5] Bordi con numero differente di punti"
         << endl;


    Pista pistaBordiDifferenti;


    // Bordo sinistro:
    // quadrato con 4 punti.

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 0.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 0.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(100.0, 100.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoSinistro(
        Punto2D(0.0, 100.0)
    );


    // Bordo destro:
    // stessa forma, ma con punti intermedi aggiuntivi.
    //
    // In questo modo verifichiamo che il discretizzatore
    // lavori sulla lunghezza e non sul numero di punti.

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 10.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(50.0, 10.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 10.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 60.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(100.0, 110.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(50.0, 110.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 110.0)
    );

    pistaBordiDifferenti.aggiungiPuntoBordoDestro(
        Punto2D(0.0, 60.0)
    );


    vector<SezionePista> sezioniDifferenti =
        discretizzatore.discretizza(
            pistaBordiDifferenti,
            4
        );


    assert(sezioniDifferenti.size() == 4);


    assert(
        puntoCorretto(
            sezioniDifferenti[0].getPuntoSinistro(),
            0.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioniDifferenti[0].getPuntoDestro(),
            0.0,
            10.0
        )
    );


    assert(
        puntoCorretto(
            sezioniDifferenti[1].getPuntoSinistro(),
            100.0,
            0.0
        )
    );

    assert(
        puntoCorretto(
            sezioniDifferenti[1].getPuntoDestro(),
            100.0,
            10.0
        )
    );


    assert(
        puntoCorretto(
            sezioniDifferenti[2].getPuntoSinistro(),
            100.0,
            100.0
        )
    );

    assert(
        puntoCorretto(
            sezioniDifferenti[2].getPuntoDestro(),
            100.0,
            110.0
        )
    );


    assert(
        puntoCorretto(
            sezioniDifferenti[3].getPuntoSinistro(),
            0.0,
            100.0
        )
    );

    assert(
        puntoCorretto(
            sezioniDifferenti[3].getPuntoDestro(),
            0.0,
            110.0
        )
    );


    cout << "Punti bordo sinistro: 4" << endl;
    cout << "Punti bordo destro: 8" << endl;

    cout << "[OK] Il numero differente di punti non modifica "
            "la corrispondenza delle sezioni.\n"
         << endl;



    // ============================================================
    // TEST 6 - NUMERO DI SEZIONI NON VALIDO
    // ============================================================

    cout << "[Test 6] Numero di sezioni minore di 2" << endl;


    bool eccezioneRicevuta = false;


    try {

        discretizzatore.discretizza(
            pistaQuadrata,
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
    // TEST 7 - BORDO CON MENO DI 2 PUNTI
    // ============================================================

    cout << "[Test 7] Bordo sinistro con meno di 2 punti"
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
            4
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
    // TEST 8 - BORDO DI LUNGHEZZA ZERO
    // ============================================================

    cout << "[Test 8] Bordo di lunghezza zero" << endl;


    Pista pistaLunghezzaZero;


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
            4
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