#include "TestAlgoritmoGenetico.hpp"

#include "AlgoritmoGenetico.hpp"
#include "GeneraTraiettorie.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "Traiettoria.hpp"
#include "SezionePista.hpp"
#include "Punto2D.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {

    constexpr int DIMENSIONE_POPOLAZIONE = 12;
    constexpr int NUMERO_SEZIONI = 20;
    constexpr unsigned int SEED_TEST = 2026u;

    // Costruisce un semplice circuito chiuso a forma di anello.
    // Ogni sezione collega il bordo interno (raggio 40 m)
    // a quello esterno (raggio 60 m).
    vector<SezionePista> creaSezioniTest(int numeroSezioni) {
        vector<SezionePista> sezioni;
        sezioni.reserve(static_cast<size_t>(numeroSezioni));

        const double pi = acos(-1.0);
        for (int i = 0; i < numeroSezioni; i++) {
            double angolo = 2.0 * pi * i / numeroSezioni;

            Punto2D bordoSinistro(40.0 * cos(angolo), 40.0 * sin(angolo));
            Punto2D bordoDestro(60.0 * cos(angolo), 60.0 * sin(angolo));

            sezioni.emplace_back(bordoSinistro, bordoDestro);
        }
        return sezioni;
    }

    // Crea un veicolo con caratteristiche fisiche valide.
    Veicolo creaVeicoloTest() {
        Veicolo veicolo;
        veicolo.setMassa(800.0);
        veicolo.setPotenzaMassima(500000.0);
        veicolo.setAccelerazioneMassima(8.0);
        veicolo.setDecelerazioneMassima(12.0);
        veicolo.setCoefficienteResistenzaAerodinamica(0.8);
        veicolo.setAreaFrontale(1.5);
        veicolo.setVelocitaMassima(90.0);
        veicolo.setAccelerazioneLateraleMassima(18.0);

        for (int i = 0; i < 4; i++) {
            veicolo.cambiaGomma(i, Gomma(i, TipoGomma::SOFT, 1.0));
        }
        return veicolo;
    }

    // Pista chiusa piu' articolata, con curve di raggio variabile.
    // Il raggio varia dolcemente, mantenendo una larghezza di 20 metri.
    vector<SezionePista> creaSezioniVariabili(int numeroSezioni) {
        vector<SezionePista> sezioni;
        sezioni.reserve(static_cast<size_t>(numeroSezioni));
        const double pi = acos(-1.0);
        for (int i = 0; i < numeroSezioni; i++) {
            const double angolo = 2.0 * pi * i / numeroSezioni;
            const double raggio = 55.0 + 8.0 * sin(3.0 * angolo);
            sezioni.emplace_back(
                Punto2D((raggio - 10.0) * cos(angolo), (raggio - 10.0) * sin(angolo)),
                Punto2D((raggio + 10.0) * cos(angolo), (raggio + 10.0) * sin(angolo))
            );
        }
        return sezioni;
    }

    // Calcola in modo indipendente il miglior tempo della popolazione
    // iniziale, ricreando il primo seed consumato da ottimizza().
    double calcolaMiglioreIniziale(
        unsigned int seed,
        int dimensione,
        const vector<SezionePista>& sezioni,
        const Veicolo& veicolo,
        const ValutatoreTraiettoria& valutatore
    ) {
        mt19937 generatoreSeed(seed);
        GeneratoreTraiettorie generatoreTraiettorie(generatoreSeed());
        const vector<Traiettoria> iniziali = generatoreTraiettorie.generaPopolazione(
            dimensione, static_cast<int>(sezioni.size()));
        double migliore = numeric_limits<double>::infinity();
        for (const Traiettoria& traiettoria : iniziali) {
            const double tempo = valutatore.valutaTraiettoria(traiettoria, sezioni, veicolo);
            if (tempo < migliore) {
                migliore = tempo;
            }
        }
        return migliore;
    }

    // Crea un algoritmo con parametri costanti, salvo quelli
    // che vogliamo modificare nei singoli test.
    AlgoritmoGenetico creaAlgoritmo(
        int numeroGenerazioni,
        unsigned int seed = SEED_TEST,
        int numeroElite = 2,
        double probabilitaCrossover = 0.80,
        double probabilitaMutazione = 0.15
    ) {
        return AlgoritmoGenetico(
            DIMENSIONE_POPOLAZIONE,
            numeroGenerazioni,
            probabilitaCrossover,
            probabilitaMutazione,
            0.10, // intensitaMutazione
            3,    // dimensioneTorneo
            numeroElite,
            seed
        );
    }

    // Controlla che una funzione lanci invalid_argument.
    template <typename Funzione>
    bool lanciaInvalidArgument(Funzione funzione) {
        try {
            funzione();
        } catch (const invalid_argument&) {
            return true;
        }
        return false;
    }

    // Verifica che i geni di due traiettorie coincidano.
    bool traiettorieUguali(const Traiettoria& a, const Traiettoria& b) {
        if (a.getNumeroPosizioni() != b.getNumeroPosizioni()) {
            return false;
        }
        for (int i = 0; i < a.getNumeroPosizioni(); i++) {
            if (abs(a.getPosizioneLaterale(i) - b.getPosizioneLaterale(i)) > 1e-12) {
                return false;
            }
        }
        return true;
    }

    // Verifica dimensione e intervallo consentito dei geni.
    void verificaTraiettoriaValida(const Traiettoria& traiettoria, int numeroSezioni) {
        assert(traiettoria.getNumeroPosizioni() == numeroSezioni);
        for (int i = 0; i < traiettoria.getNumeroPosizioni(); i++) {
            double gene = traiettoria.getPosizioneLaterale(i);
            assert(isfinite(gene));
            assert(gene >= 0.0 && gene <= 1.0);
        }
    }

} // namespace


void eseguiTestAlgoritmoGenetico() {

    cout << "\n==============================================" << endl;
    cout << "           TEST ALGORITMO GENETICO" << endl;
    cout << "==============================================\n" << endl;

    const vector<SezionePista> sezioni = creaSezioniTest(NUMERO_SEZIONI);
    const Veicolo veicolo = creaVeicoloTest();
    const ValutatoreTraiettoria valutatore;

    // ============================================================
    // TEST 1 - COSTRUTTORE E GETTER
    // ============================================================
    cout << "[Test 1] Costruttore e getter" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(6);
        assert(algoritmo.getDimensionePopolazione() == DIMENSIONE_POPOLAZIONE);
        assert(algoritmo.getNumeroGenerazioni() == 6);
        assert(algoritmo.getProbabilitaCrossover() == 0.80);
        assert(algoritmo.getProbabilitaMutazione() == 0.15);
        assert(algoritmo.getIntensitaMutazione() == 0.10);
        assert(algoritmo.getDimensioneTorneo() == 3);
        assert(algoritmo.getNumeroElite() == 2);
        cout << "[OK] Parametri memorizzati correttamente.\n" << endl;
    }

    // ============================================================
    // TEST 2 - PARAMETRI NON VALIDI NEL COSTRUTTORE
    // ============================================================
    cout << "[Test 2] Controllo dei parametri non validi" << endl;
    {
        auto costruttoreNonValido = [](int dimensione, int generazioni,
                                      double crossover, double mutazione,
                                      double intensita, int torneo, int elite) {
            return lanciaInvalidArgument([&]() {
                AlgoritmoGenetico algoritmo(dimensione, generazioni, crossover,
                                            mutazione, intensita, torneo, elite,
                                            SEED_TEST);
                (void)algoritmo;
            });
        };

        assert(costruttoreNonValido(1,  6, 0.8, 0.15, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 0, 0.8, 0.15, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 6, -0.1, 0.15, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 6, 1.1, 0.15, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 6, 0.8, -0.1, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 6, 0.8, 1.1, 0.1, 3, 2));
        assert(costruttoreNonValido(12, 6, 0.8, 0.15, 0.0, 3, 2));
        assert(costruttoreNonValido(12, 6, 0.8, 0.15, 0.1, 1, 2));
        assert(costruttoreNonValido(12, 6, 0.8, 0.15, 0.1, 13, 2));
        assert(costruttoreNonValido(12, 6, 0.8, 0.15, 0.1, 3, -1));
        assert(costruttoreNonValido(12, 6, 0.8, 0.15, 0.1, 3, 12));

        cout << "[OK] Il costruttore rifiuta i parametri errati.\n" << endl;
    }

    // ============================================================
    // TEST 3 - SEZIONI NON VALIDE
    // ============================================================
    cout << "[Test 3] Ottimizzazione con meno di tre sezioni" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(3);
        vector<SezionePista> vuote;
        vector<SezionePista> dueSezioni = creaSezioniTest(2);

        assert(lanciaInvalidArgument([&]() { algoritmo.ottimizza(vuote, veicolo); }));
        assert(lanciaInvalidArgument([&]() { algoritmo.ottimizza(dueSezioni, veicolo); }));

        cout << "[OK] Le piste con meno di tre sezioni vengono rifiutate.\n" << endl;
    }

    // ============================================================
    // TEST 4 - UNA GENERAZIONE: RESTITUISCE IL MIGLIORE INIZIALE
    // ============================================================
    cout << "[Test 4] Una generazione: migliore della popolazione iniziale" << endl;
    double tempoMiglioreIniziale;
    {
        // Ricreiamo esattamente il generatore usato da ottimizza().
        mt19937 generatoreSeed(SEED_TEST);
        GeneratoreTraiettorie generatoreTraiettorie(generatoreSeed());
        vector<Traiettoria> iniziale = generatoreTraiettorie.generaPopolazione(
            DIMENSIONE_POPOLAZIONE, NUMERO_SEZIONI
        );

        int indiceMigliore = 0;
        tempoMiglioreIniziale = valutatore.valutaTraiettoria(iniziale[0], sezioni, veicolo);
        for (int i = 1; i < DIMENSIONE_POPOLAZIONE; i++) {
            double tempo = valutatore.valutaTraiettoria(iniziale[i], sezioni, veicolo);
            if (tempo < tempoMiglioreIniziale) {
                tempoMiglioreIniziale = tempo;
                indiceMigliore = i;
            }
        }

        AlgoritmoGenetico algoritmo = creaAlgoritmo(1, SEED_TEST);
        Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        double tempoRisultato = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);

        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        assert(traiettorieUguali(risultato, iniziale[indiceMigliore]));
        assert(abs(tempoRisultato - tempoMiglioreIniziale) < 1e-9);
        cout << "Miglior tempo iniziale: " << tempoMiglioreIniziale << " s" << endl;
        cout << "Tempo restituito:     " << tempoRisultato << " s" << endl;
        cout << "[OK] Con una generazione restituisce il migliore iniziale.\n" << endl;
    }

    // ============================================================
    // TEST 5 - TRAIETTORIA VALIDA DOPO PIU GENERAZIONI
    // ============================================================
    cout << "[Test 5] Validita' dei geni dopo l'evoluzione" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(8);
        Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);

        double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        cout << "Numero di geni: " << risultato.getNumeroPosizioni() << endl;
        cout << "Tempo sul giro: " << tempo << " s" << endl;
        cout << "[OK] La traiettoria ottenuta e' valida.\n" << endl;
    }

    // ============================================================
    // TEST 6 - RIPRODUCIBILITA' CON LO STESSO SEED
    // ============================================================
    cout << "[Test 6] Riproducibilita' con lo stesso seed" << endl;
    {
        AlgoritmoGenetico algoritmo1 = creaAlgoritmo(8, 12345u);
        AlgoritmoGenetico algoritmo2 = creaAlgoritmo(8, 12345u);

        Traiettoria risultato1 = algoritmo1.ottimizza(sezioni, veicolo);
        Traiettoria risultato2 = algoritmo2.ottimizza(sezioni, veicolo);

        assert(traiettorieUguali(risultato1, risultato2));
        cout << "[OK] Stesso seed e stessi parametri: stesso risultato.\n" << endl;
    }

    // ============================================================
    // TEST 7 - IL MIGLIORE GLOBALE NON PEGGIORA
    // ============================================================
    cout << "[Test 7] Miglior tempo globale dopo piu generazioni" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(12, SEED_TEST);
        Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        double tempoFinale = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);

        assert(isfinite(tempoFinale));
        assert(tempoFinale <= tempoMiglioreIniziale + 1e-9);

        cout << "Migliore iniziale: " << tempoMiglioreIniziale << " s" << endl;
        cout << "Migliore finale:   " << tempoFinale << " s" << endl;
        cout << "[OK] Il migliore globale non viene perso.\n" << endl;
    }

    // ============================================================
    // TEST 8 - NESSUN CROSSOVER E NESSUNA MUTAZIONE
    // ============================================================
    cout << "[Test 8] Evoluzione senza crossover e mutazione" << endl;
    {
        // Senza crossover e mutazione, i figli sono copie dei
        // genitori: non possono comparire nuovi valori dei geni.
        AlgoritmoGenetico algoritmo = creaAlgoritmo(6, SEED_TEST, 0, 0.0, 0.0);
        Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);

        assert(abs(tempo - tempoMiglioreIniziale) < 1e-9);
        cout << "Migliore iniziale: " << tempoMiglioreIniziale << " s" << endl;
        cout << "Migliore finale:   " << tempo << " s" << endl;
        cout << "[OK] Senza operatori genetici il record non cambia.\n" << endl;
    }

    // ============================================================
    // TEST 9 - NUMERO DI ELITE ELEVATO
    // ============================================================
    cout << "[Test 9] Numero di elite pari a N - 1" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(
            5, SEED_TEST, DIMENSIONE_POPOLAZIONE - 1
        );
        Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);

        double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        assert(tempo <= tempoMiglioreIniziale + 1e-9);
        cout << "Elite conservate: " << DIMENSIONE_POPOLAZIONE - 1 << endl;
        cout << "Tempo restituito: " << tempo << " s" << endl;
        cout << "[OK] Funziona anche conservando N - 1 elite.\n" << endl;
    }


    // ============================================================
    // TEST 10 - POPOLAZIONE MINIMA CONSENTITA: DUE INDIVIDUI
    // ============================================================
    cout << "[Test 10] Popolazione minima di 2 traiettorie" << endl;
    {
        const vector<SezionePista> sezioniMinime = creaSezioniTest(3);
        AlgoritmoGenetico algoritmo(2, 5, 0.80, 0.20, 0.10, 2, 0, 17u);
        const Traiettoria risultato = algoritmo.ottimizza(sezioniMinime, veicolo);
        verificaTraiettoriaValida(risultato, 3);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioniMinime, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        cout << "Tempo con 2 individui e 3 sezioni: " << tempo << " s" << endl;
        cout << "[OK] I valori minimi consentiti funzionano.\n" << endl;
    }

    // ============================================================
    // TEST 11 - ELITISMO DISATTIVATO, RECORD GLOBALE CONSERVATO
    // ============================================================
    cout << "[Test 11] Nessuna elite: migliore globale conservato" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(10, SEED_TEST, 0, 1.0, 1.0);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(tempo <= tempoMiglioreIniziale + 1e-9);
        cout << "Migliore iniziale: " << tempoMiglioreIniziale << " s" << endl;
        cout << "Migliore globale:  " << tempo << " s" << endl;
        cout << "[OK] Anche senza elitismo il record iniziale non va perso.\n" << endl;
    }

    // ============================================================
    // TEST 12 - CROSSOVER SEMPRE ATTIVO E MUTAZIONE DISATTIVATA
    // ============================================================
    cout << "[Test 12] Probabilita' crossover 1, mutazione 0" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(9, SEED_TEST, 2, 1.0, 0.0);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        assert(tempo <= tempoMiglioreIniziale + 1e-9);
        cout << "Tempo ottenuto: " << tempo << " s" << endl;
        cout << "[OK] Funziona con crossover sempre attivo.\n" << endl;
    }

    // ============================================================
    // TEST 13 - NESSUN CROSSOVER, SOLO MUTAZIONE
    // ============================================================
    cout << "[Test 13] Probabilita' crossover 0, mutazione positiva" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(9, SEED_TEST, 2, 0.0, 0.50);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        assert(tempo <= tempoMiglioreIniziale + 1e-9);
        cout << "Tempo ottenuto: " << tempo << " s" << endl;
        cout << "[OK] Funziona utilizzando soltanto mutazioni.\n" << endl;
    }

    // ============================================================
    // TEST 14 - CROSSOVER E MUTAZIONE SEMPRE ATTIVI
    // ============================================================
    cout << "[Test 14] Crossover e mutazione entrambi al 100%" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(8, 77u, 2, 1.0, 1.0);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        cout << "Tempo ottenuto: " << tempo << " s" << endl;
        cout << "[OK] Funziona con entrambi gli operatori sempre attivi.\n" << endl;
    }

    // ============================================================
    // TEST 15 - INTENSITA' MUTAZIONE MASSIMA E CLAMP DEI GENI
    // ============================================================
    cout << "[Test 15] Intensita' mutazione massima: geni in [0, 1]" << endl;
    {
        // Con intensita' 1.0 le variazioni possono essere grandi:
        // ogni gene della soluzione restituita deve restare valido.
        AlgoritmoGenetico algoritmo(12, 12, 0.80, 1.0, 1.0, 3, 0, 994u);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        cout << "[OK] La soluzione restituita rispetta i limiti laterali.\n" << endl;
    }

    // ============================================================
    // TEST 16 - DIMENSIONE TORNEO MINIMA: DUE
    // ============================================================
    cout << "[Test 16] Torneo con dimensione minima 2" << endl;
    {
        AlgoritmoGenetico algoritmo(12, 6, 0.8, 0.15, 0.1, 2, 2, 101u);
        assert(algoritmo.getDimensioneTorneo() == 2);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        cout << "[OK] Funziona con tornei a due partecipanti.\n" << endl;
    }

    // ============================================================
    // TEST 17 - DIMENSIONE TORNEO PARI ALLA POPOLAZIONE
    // ============================================================
    cout << "[Test 17] Torneo con dimensione pari alla popolazione" << endl;
    {
        AlgoritmoGenetico algoritmo(12, 6, 0.8, 0.15, 0.1, 12, 2, 101u);
        assert(algoritmo.getDimensioneTorneo() == 12);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
        cout << "[OK] Funziona con dimensione torneo massima.\n" << endl;
    }

    // ============================================================
    // TEST 18 - MONOTONIA DEL MIGLIORE GLOBALE
    // ============================================================
    cout << "[Test 18] Migliore globale con numero crescente di generazioni" << endl;
    {
        const vector<int> generazioniDaProvare = {1, 2, 3, 6, 10};
        double recordPrecedente = numeric_limits<double>::infinity();
        for (int generazioni : generazioniDaProvare) {
            AlgoritmoGenetico algoritmo = creaAlgoritmo(generazioni, 42u);
            const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
            verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
            const double record = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
            assert(isfinite(record) && record > 0.0);
            assert(record <= recordPrecedente + 1e-9);
            cout << "  Generazioni: " << generazioni << " - record: " << record << " s" << endl;
            recordPrecedente = record;
        }
        cout << "[OK] A parita' di seed il migliore globale non peggiora.\n" << endl;
    }

    // ============================================================
    // TEST 19 - DIVERSI NUMERI DI SEZIONI
    // ============================================================
    cout << "[Test 19] Piste con numeri di sezioni differenti" << endl;
    {
        for (int n : {3, 4, 8, 20, 40, 60}) {
            const vector<SezionePista> pista = creaSezioniTest(n);
            AlgoritmoGenetico algoritmo = creaAlgoritmo(4, 1024u);
            const Traiettoria risultato = algoritmo.ottimizza(pista, veicolo);
            verificaTraiettoriaValida(risultato, n);
            const double tempo = valutatore.valutaTraiettoria(risultato, pista, veicolo);
            assert(isfinite(tempo) && tempo > 0.0);
            cout << "  Sezioni: " << n << " - tempo: " << tempo << " s" << endl;
        }
        cout << "[OK] Funziona con discretizzazioni diverse.\n" << endl;
    }

    // ============================================================
    // TEST 20 - DIVERSE DIMENSIONI DELLA POPOLAZIONE
    // ============================================================
    cout << "[Test 20] Popolazioni di dimensioni differenti" << endl;
    {
        for (int n : {2, 3, 8, 25, 40}) {
            const int dimensioneTorneo = n < 3 ? 2 : 3;
            const int numeroElite = n == 2 ? 0 : 1;
            AlgoritmoGenetico algoritmo(n, 4, 0.75, 0.20, 0.10,
                                        dimensioneTorneo, numeroElite, 123u);
            const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
            verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
            const double tempo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
            assert(isfinite(tempo) && tempo > 0.0);
            cout << "  Popolazione: " << n << " - tempo: " << tempo << " s" << endl;
        }
        cout << "[OK] Funziona con popolazioni di dimensioni diverse.\n" << endl;
    }

    // ============================================================
    // TEST 21 - MOLTI SEED, MIGLIORE SEMPRE ALMENO COME INIZIALE
    // ============================================================
    cout << "[Test 21] Record globale con seed differenti" << endl;
    {
        const vector<unsigned int> seeds = {
            0u, 1u, 42u, 2026u, 123456u,
            numeric_limits<unsigned int>::max()
        };
        for (unsigned int seed : seeds) {
            const double prima = calcolaMiglioreIniziale(
                seed, DIMENSIONE_POPOLAZIONE, sezioni, veicolo, valutatore);
            AlgoritmoGenetico algoritmo = creaAlgoritmo(7, seed, 0);
            const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
            verificaTraiettoriaValida(risultato, NUMERO_SEZIONI);
            const double dopo = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
            assert(isfinite(prima) && isfinite(dopo));
            assert(dopo <= prima + 1e-9);
            cout << "  Seed " << seed << ": " << prima << " -> " << dopo << " s" << endl;
        }
        cout << "[OK] Nessun seed fa perdere il migliore iniziale.\n" << endl;
    }

    // ============================================================
    // TEST 22 - SEED DIVERSI: POPOLAZIONI INIZIALI DIVERSE
    // ============================================================
    cout << "[Test 22] Seed differenti generano soluzioni differenti" << endl;
    {
        // Una sola generazione: confrontiamo i migliori estratti
        // da due popolazioni iniziali diverse (senza evoluzione).
        AlgoritmoGenetico algoritmoA = creaAlgoritmo(1, 1u);
        AlgoritmoGenetico algoritmoB = creaAlgoritmo(1, 2u);
        const Traiettoria a = algoritmoA.ottimizza(sezioni, veicolo);
        const Traiettoria b = algoritmoB.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(a, NUMERO_SEZIONI);
        verificaTraiettoriaValida(b, NUMERO_SEZIONI);
        assert(!traiettorieUguali(a, b));
        cout << "[OK] Seed diversi portano a soluzioni iniziali diverse.\n" << endl;
    }

    // ============================================================
    // TEST 23 - PIU' OTTIMIZZAZIONI SULLA STESSA ISTANZA
    // ============================================================
    cout << "[Test 23] Due chiamate a ottimizza sullo stesso oggetto" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(6, 31337u);
        const Traiettoria prima = algoritmo.ottimizza(sezioni, veicolo);
        const Traiettoria seconda = algoritmo.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(prima, NUMERO_SEZIONI);
        verificaTraiettoriaValida(seconda, NUMERO_SEZIONI);
        const double tempo1 = valutatore.valutaTraiettoria(prima, sezioni, veicolo);
        const double tempo2 = valutatore.valutaTraiettoria(seconda, sezioni, veicolo);
        assert(isfinite(tempo1) && tempo1 > 0.0);
        assert(isfinite(tempo2) && tempo2 > 0.0);
        // Lo stato del generatore avanza: NON richiediamo risultati identici.
        cout << "  Chiamata 1: " << tempo1 << " s | Chiamata 2: " << tempo2 << " s" << endl;
        cout << "[OK] L'oggetto puo' essere riutilizzato.\n" << endl;
    }

    // ============================================================
    // TEST 24 - PISTA CON CURVE DI RAGGIO VARIABILE
    // ============================================================
    cout << "[Test 24] Pista non circolare con curvature variabili" << endl;
    {
        const vector<SezionePista> pistaVariabile = creaSezioniVariabili(48);
        const double prima = calcolaMiglioreIniziale(
            818u, DIMENSIONE_POPOLAZIONE, pistaVariabile, veicolo, valutatore);
        AlgoritmoGenetico algoritmo = creaAlgoritmo(10, 818u, 2);
        const Traiettoria risultato = algoritmo.ottimizza(pistaVariabile, veicolo);
        verificaTraiettoriaValida(risultato, 48);
        const double dopo = valutatore.valutaTraiettoria(risultato, pistaVariabile, veicolo);
        assert(isfinite(dopo) && dopo > 0.0);
        assert(dopo <= prima + 1e-9);
        cout << "  Migliore iniziale: " << prima << " s" << endl;
        cout << "  Migliore finale:   " << dopo << " s" << endl;
        cout << "[OK] Funziona anche su geometrie meno regolari.\n" << endl;
    }

    // ============================================================
    // TEST 25 - DUE VEICOLI CON VELOCITA' MASSIME DIFFERENTI
    // ============================================================
    cout << "[Test 25] Veicoli con velocita' massima diversa" << endl;
    {
        Veicolo veicoloLento = veicolo;
        veicoloLento.setVelocitaMassima(20.0);
        AlgoritmoGenetico algoritmoLento = creaAlgoritmo(1, 404u);
        AlgoritmoGenetico algoritmoVeloce = creaAlgoritmo(1, 404u);
        const Traiettoria risultatoLento = algoritmoLento.ottimizza(sezioni, veicoloLento);
        const Traiettoria risultatoVeloce = algoritmoVeloce.ottimizza(sezioni, veicolo);
        verificaTraiettoriaValida(risultatoLento, NUMERO_SEZIONI);
        verificaTraiettoriaValida(risultatoVeloce, NUMERO_SEZIONI);
        const double tempoLento = valutatore.valutaTraiettoria(risultatoLento, sezioni, veicoloLento);
        const double tempoVeloce = valutatore.valutaTraiettoria(risultatoVeloce, sezioni, veicolo);
        assert(isfinite(tempoLento) && isfinite(tempoVeloce));
        assert(tempoLento >= tempoVeloce - 1e-9);
        cout << "  Veicolo lento:  " << tempoLento << " s" << endl;
        cout << "  Veicolo veloce: " << tempoVeloce << " s" << endl;
        cout << "[OK] Il risultato tiene conto delle caratteristiche del veicolo.\n" << endl;
    }

    // ============================================================
    // TEST 26 - VALUTAZIONE DETERMINISTICA DEL RISULTATO
    // ============================================================
    cout << "[Test 26] Valutazione ripetuta della stessa soluzione" << endl;
    {
        AlgoritmoGenetico algoritmo = creaAlgoritmo(6, 999u);
        const Traiettoria risultato = algoritmo.ottimizza(sezioni, veicolo);
        const double tempo1 = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        const double tempo2 = valutatore.valutaTraiettoria(risultato, sezioni, veicolo);
        assert(isfinite(tempo1) && isfinite(tempo2));
        assert(abs(tempo1 - tempo2) <= 1e-12);
        cout << "  Prima valutazione:   " << tempo1 << " s" << endl;
        cout << "  Seconda valutazione: " << tempo2 << " s" << endl;
        cout << "[OK] Il valutatore restituisce sempre lo stesso tempo.\n" << endl;
    }

    // ============================================================
    // TEST 27 - ESECUZIONE CON INPUT PIU' GRANDI (SMOKE TEST)
    // ============================================================
    cout << "[Test 27] Popolazione e pista piu' grandi" << endl;
    {
        const int popolazioneGrande = 80;
        const int sezioniGrandi = 80;
        const vector<SezionePista> pista = creaSezioniVariabili(sezioniGrandi);
        AlgoritmoGenetico algoritmo(popolazioneGrande, 6, 0.85, 0.08, 0.1,
                                    4, 4, 8008u);
        const Traiettoria risultato = algoritmo.ottimizza(pista, veicolo);
        verificaTraiettoriaValida(risultato, sezioniGrandi);
        const double tempo = valutatore.valutaTraiettoria(risultato, pista, veicolo);
        assert(isfinite(tempo) && tempo > 0.0);
        cout << "  Popolazione: " << popolazioneGrande
             << " | Sezioni: " << sezioniGrandi
             << " | Generazioni: 6" << endl;
        cout << "  Migliore tempo: " << tempo << " s" << endl;
        cout << "[OK] Esecuzione completata su un input piu' grande.\n" << endl;
    }

    cout << "==============================================" << endl;
    cout << "     TUTTI I 27 TEST SONO STATI SUPERATI" << endl;
    cout << "==============================================\n" << endl;
}