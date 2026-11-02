#include "TestValutatoreTraiettoria.hpp"

#include "ValutatoreTraiettoria.hpp"
#include "Traiettoria.hpp"
#include "SezionePista.hpp"
#include "Punto2D.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;


namespace {

    bool doubleUguali(
        double a,
        double b,
        double epsilon = 0.000001
    ) {
        return abs(a - b) < epsilon;
    }


    // Crea una sezione larga 2 unità il cui punto centrale
    // coincide con le coordinate (x, y).
    //
    // Utilizzando posizione laterale 0.5 otteniamo quindi
    // esattamente Punto2D(x, y).
    SezionePista creaSezioneCentrata(
        double x,
        double y
    ) {

        return SezionePista(
            Punto2D(x - 1.0, y),
            Punto2D(x + 1.0, y)
        );
    }


    // Crea un veicolo utilizzabile nei test.
    Veicolo creaVeicoloTest(
        double velocitaMassima,
        double accelerazioneLateraleMassima,
        double grip
    ) {

        Veicolo veicolo;

        veicolo.setMassa(800.0);
        veicolo.setPotenzaMassima(500000.0);

        veicolo.setAccelerazioneMassima(100.0);
        veicolo.setDecelerazioneMassima(100.0);

        veicolo.setCoefficienteResistenzaAerodinamica(
            0.8
        );

        veicolo.setAreaFrontale(
            1.5
        );

        veicolo.setVelocitaMassima(
            velocitaMassima
        );

        veicolo.setAccelerazioneLateraleMassima(
            accelerazioneLateraleMassima
        );


        // Impostiamo lo stesso grip sulle quattro gomme.
        for (int i = 0; i < 4; i++) {

            veicolo.cambiaGomma(
                i,
                Gomma(
                    i,
                    TipoGomma::SOFT,
                    grip
                )
            );
        }


        return veicolo;
    }

}


void eseguiTestValutatoreTraiettoria() {

    cout << "\n==============================================" << endl;
    cout << "        TEST VALUTATORE TRAIETTORIA" << endl;
    cout << "==============================================\n" << endl;


    ValutatoreTraiettoria valutatore;


    // ============================================================
    // TEST 1 - CIRCUITO LINEARE CON TEMPO NOTO
    // ============================================================

    cout << "[Test 1] Circuito lineare con tempo noto" << endl;


    // Creiamo tre punti allineati:
    //
    // P0 -------- P1 -------- P2
    //
    // 0           100         200
    //
    // Essendo il circuito chiuso:
    //
    // P0 -> P1 = 100
    // P1 -> P2 = 100
    // P2 -> P0 = 200
    //
    // Lunghezza totale = 400 metri.

    vector<SezionePista> sezioniLineari = {

        creaSezioneCentrata(
            0.0,
            0.0
        ),

        creaSezioneCentrata(
            100.0,
            0.0
        ),

        creaSezioneCentrata(
            200.0,
            0.0
        )
    };


    Traiettoria traiettoriaLineare(
        {
            0.5,
            0.5,
            0.5
        }
    );


    // Curvatura = 0.
    //
    // Quindi la velocità viene limitata solamente
    // dalla velocità massima del veicolo: 50 m/s.
    Veicolo veicolo50 =
        creaVeicoloTest(
            50.0,
            30.0,
            1.0
        );


    double tempoLineare =
        valutatore.valutaTraiettoria(
            traiettoriaLineare,
            sezioniLineari,
            veicolo50
        );


    // 400 metri / 50 m/s = 8 secondi.
    double tempoAtteso = 8.0;


    cout << "Tempo ottenuto: "
         << tempoLineare
         << " s"
         << endl;

    cout << "Tempo atteso: "
         << tempoAtteso
         << " s"
         << endl;


    assert(
        doubleUguali(
            tempoLineare,
            tempoAtteso
        )
    );


    cout << "[OK] Tempo sul circuito lineare corretto.\n"
         << endl;



    // ============================================================
    // TEST 2 - VELOCITA MASSIMA PIU BASSA
    // ============================================================

    cout << "[Test 2] Effetto della velocita massima" << endl;


    Veicolo veicolo25 =
        creaVeicoloTest(
            25.0,
            30.0,
            1.0
        );


    double tempoVeicolo25 =
        valutatore.valutaTraiettoria(
            traiettoriaLineare,
            sezioniLineari,
            veicolo25
        );


    // 400 / 25 = 16 secondi.
    assert(
        doubleUguali(
            tempoVeicolo25,
            16.0
        )
    );


    assert(
        tempoVeicolo25 >
        tempoLineare
    );


    cout << "Velocita massima 50 m/s -> "
         << tempoLineare
         << " s"
         << endl;

    cout << "Velocita massima 25 m/s -> "
         << tempoVeicolo25
         << " s"
         << endl;


    cout << "[OK] Riducendo la velocita massima aumenta il tempo sul giro.\n"
         << endl;



    // ============================================================
    // TEST 3 - CIRCUITO CON CURVE
    // ============================================================

    cout << "[Test 3] Circuito quadrato" << endl;


    // P3 ---------------- P2
    // |                    |
    // |                    |
    // |                    |
    // P0 ---------------- P1

    vector<SezionePista> sezioniQuadrato = {

        creaSezioneCentrata(
            0.0,
            0.0
        ),

        creaSezioneCentrata(
            100.0,
            0.0
        ),

        creaSezioneCentrata(
            100.0,
            100.0
        ),

        creaSezioneCentrata(
            0.0,
            100.0
        )
    };


    Traiettoria traiettoriaQuadrato(
        {
            0.5,
            0.5,
            0.5,
            0.5
        }
    );


    Veicolo veicoloQuadrato =
        creaVeicoloTest(
            100.0,
            30.0,
            1.0
        );


    double tempoQuadrato =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            veicoloQuadrato
        );


    cout << "Tempo circuito quadrato: "
         << tempoQuadrato
         << " s"
         << endl;


    assert(tempoQuadrato > 0.0);
    assert(isfinite(tempoQuadrato));


    cout << "[OK] Il circuito con curve produce un tempo valido.\n"
         << endl;



    // ============================================================
    // TEST 4 - EFFETTO DEL GRIP
    // ============================================================

    cout << "[Test 4] Effetto del grip delle gomme" << endl;


    Veicolo gommeNuove =
        creaVeicoloTest(
            100.0,
            30.0,
            1.0
        );


    Veicolo gommeDegradate =
        creaVeicoloTest(
            100.0,
            30.0,
            0.5
        );


    double tempoGommeNuove =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            gommeNuove
        );


    double tempoGommeDegradate =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            gommeDegradate
        );


    cout << "Grip 1.0 -> "
         << tempoGommeNuove
         << " s"
         << endl;

    cout << "Grip 0.5 -> "
         << tempoGommeDegradate
         << " s"
         << endl;


    // Con meno grip dobbiamo percorrere
    // le curve più lentamente.
    assert(
        tempoGommeDegradate >
        tempoGommeNuove
    );


    cout << "[OK] Il degrado del grip aumenta il tempo sul giro.\n"
         << endl;



    // ============================================================
    // TEST 5 - ACCELERAZIONE LATERALE MASSIMA
    // ============================================================

    cout << "[Test 5] Effetto dell'accelerazione laterale massima"
         << endl;


    Veicolo veicoloPocoGripLaterale =
        creaVeicoloTest(
            100.0,
            15.0,
            1.0
        );


    Veicolo veicoloMoltoGripLaterale =
        creaVeicoloTest(
            100.0,
            30.0,
            1.0
        );


    double tempoPocoGripLaterale =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            veicoloPocoGripLaterale
        );


    double tempoMoltoGripLaterale =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            veicoloMoltoGripLaterale
        );


    cout << "Accelerazione laterale 15 m/s^2 -> "
         << tempoPocoGripLaterale
         << " s"
         << endl;

    cout << "Accelerazione laterale 30 m/s^2 -> "
         << tempoMoltoGripLaterale
         << " s"
         << endl;


    assert(
        tempoPocoGripLaterale >
        tempoMoltoGripLaterale
    );


    cout << "[OK] Una maggiore accelerazione laterale "
            "permette tempi migliori.\n"
         << endl;



    // ============================================================
    // TEST 6 - NUMERO POSIZIONI DIVERSO DAL NUMERO DI SEZIONI
    // ============================================================

    cout << "[Test 6] Numero posizioni diverso dal numero di sezioni"
         << endl;


    Traiettoria traiettoriaErrata(
        {
            0.5,
            0.5,
            0.5
        }
    );


    bool eccezioneRicevuta = false;


    try {

        valutatore.valutaTraiettoria(
            traiettoriaErrata,
            sezioniQuadrato,
            veicoloQuadrato
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Incoerenza tra traiettoria e sezioni rifiutata.\n"
         << endl;



    // ============================================================
    // TEST 7 - MENO DI TRE SEZIONI
    // ============================================================

    cout << "[Test 7] Numero insufficiente di sezioni" << endl;


    vector<SezionePista> dueSezioni = {

        creaSezioneCentrata(
            0.0,
            0.0
        ),

        creaSezioneCentrata(
            100.0,
            0.0
        )
    };


    Traiettoria traiettoriaDuePunti(
        {
            0.5,
            0.5
        }
    );


    eccezioneRicevuta = false;


    try {

        valutatore.valutaTraiettoria(
            traiettoriaDuePunti,
            dueSezioni,
            veicoloQuadrato
        );

    }
    catch (const invalid_argument& errore) {

        eccezioneRicevuta = true;

        cout << "Eccezione correttamente ricevuta: "
             << errore.what()
             << endl;
    }


    assert(eccezioneRicevuta);


    cout << "[OK] Numero insufficiente di sezioni rifiutato.\n"
         << endl;



    // ============================================================
    // TEST 8 - RIPETIBILITA DELLA VALUTAZIONE
    // ============================================================

    cout << "[Test 8] Ripetibilita della valutazione" << endl;


    double tempoA =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            veicoloQuadrato
        );


    double tempoB =
        valutatore.valutaTraiettoria(
            traiettoriaQuadrato,
            sezioniQuadrato,
            veicoloQuadrato
        );


    cout << "Prima valutazione: "
         << tempoA
         << " s"
         << endl;

    cout << "Seconda valutazione: "
         << tempoB
         << " s"
         << endl;


    assert(
        doubleUguali(
            tempoA,
            tempoB
        )
    );


    cout << "[OK] La stessa traiettoria produce lo stesso tempo.\n"
         << endl;



    // ============================================================
    // TEST 9 - EFFETTO DELLA POTENZA MASSIMA
    // ============================================================

    cout << "[Test 9] Effetto della potenza massima" << endl;

    // Circuito con rettilinei e curve di raggio differente.
    // In questo modo, dopo i punti piu lenti, il veicolo deve
    // realmente accelerare e la potenza entra nel calcolo.
    vector<SezionePista> sezioniAerodinamica = {
        creaSezioneCentrata(0.0, 0.0),
        creaSezioneCentrata(100.0, 0.0),
        creaSezioneCentrata(200.0, 0.0),
        creaSezioneCentrata(300.0, 0.0),
        creaSezioneCentrata(350.0, 50.0),
        creaSezioneCentrata(300.0, 100.0),
        creaSezioneCentrata(200.0, 100.0),
        creaSezioneCentrata(100.0, 100.0),
        creaSezioneCentrata(0.0, 100.0),
        creaSezioneCentrata(-50.0, 50.0)
    };

    Traiettoria traiettoriaAerodinamica(
        {
            0.5, 0.5, 0.5, 0.5, 0.5,
            0.5, 0.5, 0.5, 0.5, 0.5
        }
    );

    Veicolo veicoloPotenzaBassa =
        creaVeicoloTest(100.0, 30.0, 1.0);

    Veicolo veicoloPotenzaAlta =
        creaVeicoloTest(100.0, 30.0, 1.0);

    veicoloPotenzaBassa.setPotenzaMassima(100000.0);
    veicoloPotenzaAlta.setPotenzaMassima(800000.0);

    double tempoPotenzaBassa =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloPotenzaBassa
        );

    double tempoPotenzaAlta =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloPotenzaAlta
        );

    cout << "Potenza 100 kW -> "
         << tempoPotenzaBassa
         << " s"
         << endl;

    cout << "Potenza 800 kW -> "
         << tempoPotenzaAlta
         << " s"
         << endl;

    assert(tempoPotenzaBassa > tempoPotenzaAlta);

    cout << "[OK] Una maggiore potenza migliora la capacita di accelerazione.\n"
         << endl;



    // ============================================================
    // TEST 10 - EFFETTO DEL COEFFICIENTE AERODINAMICO
    // ============================================================

    cout << "[Test 10] Effetto del coefficiente aerodinamico" << endl;

    Veicolo veicoloCdBasso =
        creaVeicoloTest(100.0, 30.0, 1.0);

    Veicolo veicoloCdAlto =
        creaVeicoloTest(100.0, 30.0, 1.0);

    veicoloCdBasso.setCoefficienteResistenzaAerodinamica(0.50);
    veicoloCdAlto.setCoefficienteResistenzaAerodinamica(1.50);

    double tempoCdBasso =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloCdBasso
        );

    double tempoCdAlto =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloCdAlto
        );

    cout << "Cd 0.50 -> "
         << tempoCdBasso
         << " s"
         << endl;

    cout << "Cd 1.50 -> "
         << tempoCdAlto
         << " s"
         << endl;

    assert(tempoCdAlto > tempoCdBasso);

    cout << "[OK] Un coefficiente aerodinamico maggiore aumenta la resistenza.\n"
         << endl;



    // ============================================================
    // TEST 11 - EFFETTO DELL'AREA FRONTALE
    // ============================================================

    cout << "[Test 11] Effetto dell'area frontale" << endl;

    Veicolo veicoloAreaPiccola =
        creaVeicoloTest(100.0, 30.0, 1.0);

    Veicolo veicoloAreaGrande =
        creaVeicoloTest(100.0, 30.0, 1.0);

    veicoloAreaPiccola.setAreaFrontale(1.00);
    veicoloAreaGrande.setAreaFrontale(3.00);

    double tempoAreaPiccola =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloAreaPiccola
        );

    double tempoAreaGrande =
        valutatore.valutaTraiettoria(
            traiettoriaAerodinamica,
            sezioniAerodinamica,
            veicoloAreaGrande
        );

    cout << "Area frontale 1.00 m^2 -> "
         << tempoAreaPiccola
         << " s"
         << endl;

    cout << "Area frontale 3.00 m^2 -> "
         << tempoAreaGrande
         << " s"
         << endl;

    assert(tempoAreaGrande > tempoAreaPiccola);

    cout << "[OK] Un'area frontale maggiore aumenta la resistenza aerodinamica.\n"
         << endl;



    // ============================================================
    // FINE TEST
    // ============================================================

    cout << "==============================================" << endl;
    cout << " TUTTI I TEST VALUTATORE TRAIETTORIA SUPERATI!" << endl;
    cout << "==============================================\n"
         << endl;
}