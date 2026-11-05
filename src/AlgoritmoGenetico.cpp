#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>
#include <stdexcept>

#include "GeneraTraiettorie.hpp"
//
// Created by helgrind on 02/11/26.
//
using namespace std;

#include "AlgoritmoGenetico.hpp"

AlgoritmoGenetico::AlgoritmoGenetico(
    int dimensionePopolazione,
    int numeroGenerazioni,
    double probabilitaCrossover,
    double probabilitaMutazione,
    double intensitaMutazione,
    int dimensioneTorneo,
    int numeroElite,
    unsigned int seed
)
    : generatore(seed) {
    // Deve esserci almeno una popolazione con 2 individui.
    // Deve essere presente almeno una generazione.
    // Le probabilita devono essere comprese tra 0 e 1.
    // L'intensita della mutazione deve essere maggiore di 0
    // e minore o uguale a 1.
    // Il torneo deve contenere almeno 2 individui
    // e non puo superare la dimensione della popolazione.
    // Il numero di elite non puo essere negativo
    // e non puo occupare tutta la popolazione.

    if (
        dimensionePopolazione < 2 ||
        numeroGenerazioni <= 0 ||
        probabilitaCrossover < 0.0 ||
        probabilitaCrossover > 1.0 ||
        probabilitaMutazione < 0.0 ||
        probabilitaMutazione > 1.0 ||
        intensitaMutazione <= 0.0 ||
        intensitaMutazione > 1.0 ||
        dimensioneTorneo < 2 ||
        dimensioneTorneo > dimensionePopolazione ||
        numeroElite < 0 ||
        numeroElite >= dimensionePopolazione
    ) {
        throw invalid_argument(
            "Parametri algoritmo genetico invalidi"
        );
    }

    this->dimensionePopolazione = dimensionePopolazione;
    this->numeroGenerazioni = numeroGenerazioni;
    this->probabilitaCrossover = probabilitaCrossover;
    this->probabilitaMutazione = probabilitaMutazione;
    this->intensitaMutazione = intensitaMutazione;
    this->dimensioneTorneo = dimensioneTorneo;
    this->numeroElite = numeroElite;
}

vector<double> AlgoritmoGenetico::valutaPopolazione(const vector<Traiettoria> &popolazione,
                                                    const vector<SezionePista> &sezioni, const Veicolo &veicolo) const {
    if (popolazione.empty()) {
        throw invalid_argument("Non posso valutare la popolazione");
    }
    vector<double> tempi;
    tempi.reserve(popolazione.size());
    for (size_t i = 0; i < popolazione.size(); i++) {
        //Per ogni traiettoria devo fare la valutazione
        double tempo = valutatore.valutaTraiettoria(popolazione[i], sezioni, veicolo);
        tempi.push_back(tempo);
    }
    return tempi;
}

int AlgoritmoGenetico::trovaIndiceMigliore(const vector<double> &tempi) const {
    if (tempi.empty()) {
        throw invalid_argument("Vettore dei tempi vuoto");
    }
    int idxMigliore = 0;
    double tempoMigliore = tempi[idxMigliore];
    for (size_t i = 1; i < tempi.size(); i++) {
        if (tempi[i] < tempoMigliore) {
            tempoMigliore = tempi[i];
            idxMigliore = static_cast<int>(i);
        }
    }
    return idxMigliore;
}

const Traiettoria &AlgoritmoGenetico::selezionaGenitore(const vector<Traiettoria> &popolazione,
                                                        const vector<double> &tempi) {
    // implementa la selezione a torneo
    // idea: sveglie casualmente alcuni individui dalla popolazione e restituisci il migliore
    if (popolazione.empty() || tempi.empty()) {
        throw invalid_argument("Parametri per selezionare il genitori vuoti");
    }
    if (popolazione.size() != tempi.size()) {
        throw invalid_argument("Parametri per selezionare il genitore incompleti");
    }
    uniform_int_distribution<size_t> distribuzione(0, popolazione.size() - 1); // creo una distribuzione di dati
    size_t indiceMiglioreTorneo = distribuzione(this->generatore);
    double tempoMiglioreTorneo = tempi[indiceMiglioreTorneo];

    //Ora devo estrarre gli altri partecipanti
    for (size_t i = 1; i < this->getDimensioneTorneo(); i++) {
        size_t indiceEstratto = distribuzione(this->generatore);
        double tempoAvversario = tempi[indiceEstratto];
        if (tempoAvversario < tempoMiglioreTorneo) {
            tempoMiglioreTorneo = tempoAvversario;
            indiceMiglioreTorneo = indiceEstratto;
        }
    }
    return popolazione[indiceMiglioreTorneo];
}

Traiettoria AlgoritmoGenetico::crossover(const Traiettoria &genitore1, const Traiettoria &genitore2) {
    // Crossover aritmetico: sceglie un unico coefficiente alpha e
    // combina tutte le posizioni dei due genitori.
    // Se entrambi i genitori sono regolari, non nasce uno spigolo
    // artificiale nel punto di taglio (come nel crossover a un punto).
    if (genitore1.getNumeroPosizioni() == 0 || genitore2.getNumeroPosizioni() == 0) {
        throw invalid_argument("Almeno un genitore e' vuoto");
    }
    if (genitore1.getNumeroPosizioni() != genitore2.getNumeroPosizioni()) {
        throw invalid_argument("Dimensioni genitori non identiche");
    }
    if (genitore1.getNumeroPosizioni() < 2) {
        throw invalid_argument("I genitori devono avere almeno due geni");
    }

    uniform_real_distribution<double> distribuzionePeso(0.0, 1.0);
    const double alpha = distribuzionePeso(this->generatore);

    vector<double> figlio;
    figlio.reserve(genitore1.getNumeroPosizioni());
    for (int i = 0; i < genitore1.getNumeroPosizioni(); ++i) {
        const double valore =
            alpha * genitore1.getPosizioneLaterale(i) +
            (1.0 - alpha) * genitore2.getPosizioneLaterale(i);
        figlio.push_back(clamp(valore, 0.0, 1.0));
    }
    return Traiettoria(figlio);
}

void AlgoritmoGenetico::muta(Traiettoria &traiettoria) {
    // Mutazione locale: quando un gene viene selezionato, la stessa
    // variazione viene distribuita anche sulle sezioni vicine con
    // ampiezza gaussiana decrescente (campana). Questo evita picchi
    // isolati e rispetta la chiusura ciclica della pista.
    const int numeroGeni = traiettoria.getNumeroPosizioni();
    if (numeroGeni == 0) {
        throw invalid_argument("Non posso mutare una traiettoria vuota");
    }
    uniform_real_distribution<double> distribuzioneProbabilita(0.0, 1.0);
    uniform_real_distribution<double> distribuzioneVariazione(
        -this->getIntensitaMutazione(), this->getIntensitaMutazione());

    // Massimo 20 vicini per lato: costo lineare rispetto a N.
    // Con 300 sezioni il raggio e' 7, cioe' circa 15 geni interessati.
    const int raggio = min({20, max(3, numeroGeni / 40), (numeroGeni - 1) / 2});
    const double sigma = max(1.0, static_cast<double>(raggio) / 2.5);

    for (int i = 0; i < numeroGeni; ++i) {
        if (distribuzioneProbabilita(this->generatore) >= this->getProbabilitaMutazione()) {
            continue;
        }
        const double variazione = distribuzioneVariazione(this->generatore);
        for (int distanza = -raggio; distanza <= raggio; ++distanza) {
            const int indice = (i + distanza + numeroGeni) % numeroGeni;
            const double d = static_cast<double>(distanza) / sigma;
            const double peso = exp(-0.5 * d * d);
            const double vecchio = traiettoria.getPosizioneLaterale(indice);
            const double nuovo = clamp(vecchio + variazione * peso, 0.0, 1.0);
            traiettoria.setPosizioneLaterale(indice, nuovo);
        }
    }
}

// Serve a far "sopravvivere" le migliori traiettorie fra tutte quelle generate
void AlgoritmoGenetico::aggiungiElite(const vector<Traiettoria> &popolazione, const vector<double> &tempi,
                                      vector<Traiettoria> &nuovaPopolazione) const {
    if (popolazione.empty() || popolazione.size() != tempi.size() || numeroElite > popolazione.size()) {
        throw invalid_argument("Parametri per aggiunta elite errati");
    }
    if (nuovaPopolazione.size() + numeroElite > dimensionePopolazione) {
        throw invalid_argument("La nuova popolazione supererebbe la dimensione massima");
    }
    if (numeroElite == 0) {
        return;
    }
    vector<size_t> indici(popolazione.size());
    // Ora ordino in modo efficiente con funzioni builtin di C
    iota(indici.begin(), indici.end(), 0);
    std::partial_sort(
        indici.begin(),
        indici.begin() + numeroElite,
        indici.end(),
        [&tempi](size_t a, size_t b) {
            return tempi[a] < tempi[b];
        }
    );
    for (size_t i = 0; i < this->numeroElite; i++) {
        size_t indice = indici[i];
        nuovaPopolazione.push_back(popolazione[indice]);
    }
}

vector<Traiettoria> AlgoritmoGenetico::generaNuovaPopolazione(const vector<Traiettoria> &popolazione,
                                                              const vector<double> &tempi) {
    if (popolazione.empty() || popolazione.size() != tempi.size() || popolazione.size() != this->
        getDimensionePopolazione()) {
        throw invalid_argument("Parametri per generare una nuova popolazione non corretti");
    }
    // Creo la nuova popolazione
    vector<Traiettoria> nuovaPopolazione;
    nuovaPopolazione.reserve(this->getDimensionePopolazione());
    // aggiungo l'elite
    aggiungiElite(popolazione, tempi, nuovaPopolazione);
    uniform_real_distribution<double> distribuzioneProbabilita(0.0, 1.0);
    //Continuo finchè non ho finito di generare la nuova popolazione:
    /*
     * Estraggo i genitor
     * Estraggo la probabilità causale
     * Se è minore di quella di crossover allora lo faccio
     */
    while (nuovaPopolazione.size() < static_cast<size_t>(dimensionePopolazione)) {
        const Traiettoria &genitore1 = this->selezionaGenitore(popolazione, tempi);
        const Traiettoria &genitore2 = this->selezionaGenitore(popolazione, tempi);
        double probabilita = distribuzioneProbabilita(this->generatore);
        Traiettoria figlio = Traiettoria();
        if (probabilita < this->getProbabilitaCrossover()) {
            figlio = crossover(genitore1,genitore2);
        }
        else {
            figlio = genitore1;
        }
        muta(figlio);
        nuovaPopolazione.push_back(figlio);
    }
    return nuovaPopolazione;
}

Traiettoria AlgoritmoGenetico::ottimizza(const vector<SezionePista> &sezioni, const Veicolo &veicolo) {
    return ottimizza(sezioni, veicolo, {});
}

Traiettoria AlgoritmoGenetico::ottimizza(const vector<SezionePista> &sezioni,
                                        const Veicolo &veicolo,
                                        const std::function<void(int, int)> &progresso) {
    if (sezioni.size() < 3) {
        throw invalid_argument("Parametri non corretti per ottimizzare");
    }
    GeneratoreTraiettorie generatoreTraiettorie(this->generatore());
    vector<Traiettoria> popolazione;
    popolazione = generatoreTraiettorie.generaPopolazione(this->getDimensionePopolazione(),static_cast<int>(sezioni.size()));
    Traiettoria miglioreTraiettoria;
    double migliorTempo = numeric_limits<double>::infinity(); // conterrà il tempo più basso trovato
    int indiceMigliore = 0;
    double tempoMiglioreGenerazione = 0.0;
    for (int generazione = 0; generazione < this->getNumeroGenerazioni(); generazione++) {
        vector<double> tempi = this->valutaPopolazione(popolazione, sezioni, veicolo);
        indiceMigliore = this->trovaIndiceMigliore(tempi);
        tempoMiglioreGenerazione = tempi[indiceMigliore];
        //Confronto e genero la prossima popolazione
        if (tempoMiglioreGenerazione < migliorTempo) {
            migliorTempo = tempoMiglioreGenerazione;
            miglioreTraiettoria = popolazione[indiceMigliore];
        }
        if (generazione < this->getNumeroGenerazioni() - 1) {
            popolazione = this->generaNuovaPopolazione(popolazione,tempi);
        }
        // Segnala la generazione appena completata (nessun dato condiviso della GA).
        if (progresso) progresso(generazione + 1, this->getNumeroGenerazioni());
    }
    return miglioreTraiettoria;
}

int AlgoritmoGenetico::getDimensionePopolazione() const {
    return this->dimensionePopolazione;
}

int AlgoritmoGenetico::getNumeroGenerazioni() const {
    return this->numeroGenerazioni;
}

double AlgoritmoGenetico::getProbabilitaCrossover() const {
    return this->probabilitaCrossover;
}

double AlgoritmoGenetico::getProbabilitaMutazione() const {
    return this->probabilitaMutazione;
}

double AlgoritmoGenetico::getIntensitaMutazione() const {
    return this->intensitaMutazione;
}

int AlgoritmoGenetico::getDimensioneTorneo() const {
    return this->dimensioneTorneo;
}

int AlgoritmoGenetico::getNumeroElite() const {
    return this->numeroElite;
}
