#include "TestVeicolo.hpp"
#include "Veicolo.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace std;


void eseguiTestVeicolo() {

    cout << "\n========================================" << endl;
    cout << "          TEST CLASSE VEICOLO" << endl;
    cout << "========================================\n" << endl;


    // ============================================================
    // TEST 1 - COSTRUTTORE DI DEFAULT
    // ============================================================

    cout << "[TEST 1] Costruttore di default" << endl;

    Veicolo veicoloDefault;

    assert(veicoloDefault.getMassa() == 0.0);
    assert(veicoloDefault.getPotenzaMassima() == 0.0);
    assert(veicoloDefault.getAccelerazioneMassima() == 0.0);
    assert(veicoloDefault.getDecelerazioneMassima() == 0.0);
    assert(
        veicoloDefault.getCoefficienteResistenzaAerodinamica()
        == 0.0
    );

    cout << "  Massa: "
         << veicoloDefault.getMassa()
         << endl;

    cout << "  Potenza massima: "
         << veicoloDefault.getPotenzaMassima()
         << endl;

    cout << "  Accelerazione massima: "
         << veicoloDefault.getAccelerazioneMassima()
         << endl;

    cout << "  Decelerazione massima: "
         << veicoloDefault.getDecelerazioneMassima()
         << endl;

    cout << "  Coefficiente aerodinamico: "
         << veicoloDefault.getCoefficienteResistenzaAerodinamica()
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 2 - GOMME DEL VEICOLO DI DEFAULT
    // ============================================================

    cout << "[TEST 2] Gomme del veicolo di default" << endl;

    for (int i = 0; i < 4; i++) {

        const Gomma& gomma = veicoloDefault.getGomma(i);

        cout << "  Gomma posizione " << i
             << " -> id = " << gomma.getId()
             << ", grip = " << gomma.getCoefficienteGrip()
             << ", mescola = " << gomma.toString()
             << endl;

        assert(gomma.getId() == i);
        assert(gomma.getTipo() == TipoGomma::SOFT);
        assert(gomma.getCoefficienteGrip() == 1.0);
        assert(gomma.toString() == "SOFT");
    }

    cout << "  -> Tutte le gomme sono correttamente inizializzate\n"
         << endl;


    // ============================================================
    // TEST 3 - COSTRUTTORE PARAMETRICO
    // ============================================================

    cout << "[TEST 3] Costruttore parametrico" << endl;

    array<Gomma, 4> gomme = {
        Gomma(0, TipoGomma::MEDIUM, 0.90),
        Gomma(1, TipoGomma::MEDIUM, 0.90),
        Gomma(2, TipoGomma::MEDIUM, 0.90),
        Gomma(3, TipoGomma::MEDIUM, 0.90)
    };

    Veicolo veicolo(
        800.0,
        500000.0,
        12.0,
        15.0,
        0.90,
        gomme
    );

    assert(veicolo.getMassa() == 800.0);
    assert(veicolo.getPotenzaMassima() == 500000.0);
    assert(veicolo.getAccelerazioneMassima() == 12.0);
    assert(veicolo.getDecelerazioneMassima() == 15.0);
    assert(
        veicolo.getCoefficienteResistenzaAerodinamica()
        == 0.90
    );

    cout << "  Massa: "
         << veicolo.getMassa()
         << endl;

    cout << "  Potenza: "
         << veicolo.getPotenzaMassima()
         << endl;

    cout << "  Accelerazione: "
         << veicolo.getAccelerazioneMassima()
         << endl;

    cout << "  Decelerazione: "
         << veicolo.getDecelerazioneMassima()
         << endl;

    cout << "  Coefficiente aerodinamico: "
         << veicolo.getCoefficienteResistenzaAerodinamica()
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 4 - GETGOMME
    // ============================================================

    cout << "[TEST 4] getGomme()" << endl;

    const array<Gomma, 4>& gommeVeicolo = veicolo.getGomme();

    for (int i = 0; i < 4; i++) {

        const Gomma& gomma = gommeVeicolo[i];

        assert(gomma.getId() == i);
        assert(gomma.getTipo() == TipoGomma::MEDIUM);
        assert(gomma.getCoefficienteGrip() == 0.90);
        assert(gomma.toString() == "MEDIUM");

        cout << "  Gomma posizione " << i
             << " -> id = " << gomma.getId()
             << ", grip = " << gomma.getCoefficienteGrip()
             << ", mescola = " << gomma.toString()
             << endl;
    }

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 5 - SETTER DEL VEICOLO
    // ============================================================

    cout << "[TEST 5] Setter del veicolo" << endl;

    veicolo.setMassa(850.0);
    veicolo.setPotenzaMassima(550000.0);
    veicolo.setAccelerazioneMassima(13.0);
    veicolo.setDecelerazioneMassima(16.0);
    veicolo.setCoefficienteResistenzaAerodinamica(0.95);

    assert(veicolo.getMassa() == 850.0);
    assert(veicolo.getPotenzaMassima() == 550000.0);
    assert(veicolo.getAccelerazioneMassima() == 13.0);
    assert(veicolo.getDecelerazioneMassima() == 16.0);
    assert(
        veicolo.getCoefficienteResistenzaAerodinamica()
        == 0.95
    );

    cout << "  Nuova massa: "
         << veicolo.getMassa()
         << endl;

    cout << "  Nuova potenza: "
         << veicolo.getPotenzaMassima()
         << endl;

    cout << "  Nuova accelerazione: "
         << veicolo.getAccelerazioneMassima()
         << endl;

    cout << "  Nuova decelerazione: "
         << veicolo.getDecelerazioneMassima()
         << endl;

    cout << "  Nuovo coefficiente aerodinamico: "
         << veicolo.getCoefficienteResistenzaAerodinamica()
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 6 - CAMBIO GOMMA VALIDO
    // ============================================================

    cout << "[TEST 6] Cambio di una gomma valido" << endl;

    Gomma nuovaGomma(
        2,
        TipoGomma::MEDIUM,
        1.0
    );

    veicolo.cambiaGomma(2, nuovaGomma);

    assert(veicolo.getGomma(2).getId() == 2);
    assert(veicolo.getGomma(2).getTipo() == TipoGomma::MEDIUM);
    assert(
        veicolo.getGomma(2).getCoefficienteGrip()
        == 1.0
    );
    assert(
        veicolo.getGomma(2).toString()
        == "MEDIUM"
    );

    cout << "  Gomma 2 sostituita:"
         << endl;

    cout << "    id = "
         << veicolo.getGomma(2).getId()
         << endl;

    cout << "    grip = "
         << veicolo.getGomma(2).getCoefficienteGrip()
         << endl;

    cout << "    mescola = "
         << veicolo.getGomma(2).toString()
         << endl;

    cout << "  -> OK\n" << endl;


    // ============================================================
    // TEST 7 - GETGOMMA CON ID NON VALIDO
    // ============================================================

    cout << "[TEST 7] getGomma() con id fuori range" << endl;

    bool eccezioneRicevuta = false;

    try {
        veicolo.getGomma(5);
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
    // TEST 8 - COSTRUTTORE CON PARAMETRI NON VALIDI
    // ============================================================

    cout << "[TEST 8] Costruttore con massa non valida" << endl;

    eccezioneRicevuta = false;

    try {

        Veicolo veicoloErrato(
            -100.0,
            500000.0,
            12.0,
            15.0,
            0.90,
            gomme
        );
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
    // TEST 9 - ID GOMME NELLE POSIZIONI SBAGLIATE
    // ============================================================

    cout << "[TEST 9] Costruttore con ID gomme non coerenti" << endl;

    array<Gomma, 4> gommeIdErrati = {
        Gomma(1, TipoGomma::SOFT, 1.0),
        Gomma(0, TipoGomma::SOFT, 1.0),
        Gomma(2, TipoGomma::SOFT, 1.0),
        Gomma(3, TipoGomma::SOFT, 1.0)
    };

    eccezioneRicevuta = false;

    try {

        Veicolo veicoloErrato(
            800.0,
            500000.0,
            12.0,
            15.0,
            0.90,
            gommeIdErrati
        );
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
    // TEST 10 - MESCOLE DIFFERENTI
    // ============================================================

    cout << "[TEST 10] Costruttore con mescole differenti" << endl;

    array<Gomma, 4> gommeMiste = {
        Gomma(0, TipoGomma::SOFT, 1.0),
        Gomma(1, TipoGomma::SOFT, 1.0),
        Gomma(2, TipoGomma::MEDIUM, 1.0),
        Gomma(3, TipoGomma::SOFT, 1.0)
    };

    eccezioneRicevuta = false;

    try {

        Veicolo veicoloErrato(
            800.0,
            500000.0,
            12.0,
            15.0,
            0.90,
            gommeMiste
        );
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
    // TEST 11 - CAMBIO GOMMA CON ID NON COERENTE
    // ============================================================

    cout << "[TEST 11] Cambio gomma con id non coerente" << endl;

    Gomma gommaIdErrato(
        3,
        TipoGomma::MEDIUM,
        1.0
    );

    eccezioneRicevuta = false;

    try {

        // Provo a mettere una gomma con id 3
        // nella posizione 2.
        veicolo.cambiaGomma(
            2,
            gommaIdErrato
        );
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
    // TEST 12 - CAMBIO GOMMA CON MESCOLA DIVERSA
    // ============================================================

    cout << "[TEST 12] Cambio gomma con mescola diversa" << endl;

    Gomma gommaSoft(
        1,
        TipoGomma::SOFT,
        1.0
    );

    eccezioneRicevuta = false;

    try {

        // Il veicolo utilizza gomme MEDIUM.
        veicolo.cambiaGomma(
            1,
            gommaSoft
        );
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
    cout << "     TUTTI I TEST VEICOLO SUPERATI" << endl;
    cout << "========================================\n" << endl;
}