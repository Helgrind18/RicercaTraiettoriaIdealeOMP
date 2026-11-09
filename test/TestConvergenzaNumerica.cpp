// APEX LAB - Diagnostica della convergenza numerica del tempo sul giro.
// Non utilizza il genetico e NON modifica il modello fisico.
// Due geometrie: (A) campioni annidati della stessa griglia fitta;
// (B) ricampionamento nativo della stessa spline dei CSV a ogni N.
// Due traiettorie fisse: centrale e laterale non costante (80 controlli).
#include "Gomma.hpp"
#include "InterpolatorePista.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include "ValutatoreTraiettoria.hpp"
#include "Veicolo.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr double PI = 3.141592653589793238462643383279502884;

struct Opzioni {
    std::string spa = "gui/data/spa.csv";
    std::string monza = "gui/data/monza.csv";
    std::string csv;
    double tolleranzaPercento = 0.5;
    bool esteso = false;
};

void guida(const char* exe) {
    std::cout << "Uso: " << exe << " [--spa FILE] [--monza FILE] [--csv FILE] "
              << "[--tolleranza PERCENTUALE] [--esteso]\n"
              << "Default: Spa/Monza dalla cartella gui/data; tolleranza 0.5%.\n"
              << "--esteso aggiunge 19200 campioni. Il test e' solo diagnostico.\n";
}

Opzioni leggiOpzioni(int argc, char** argv) {
    Opzioni o;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--aiuto" || arg == "--help" || arg == "-h") {
            guida(argv[0]);
            std::exit(0);
        }
        if (arg == "--esteso") { o.esteso = true; continue; }
        if (i + 1 >= argc) throw std::invalid_argument("Manca un valore per " + arg);
        const std::string valore = argv[++i];
        if (arg == "--spa") o.spa = valore;
        else if (arg == "--monza") o.monza = valore;
        else if (arg == "--csv") o.csv = valore;
        else if (arg == "--tolleranza") {
            std::size_t caratteriLetti = 0;
            o.tolleranzaPercento = std::stod(valore, &caratteriLetti);
            if (caratteriLetti != valore.size() || !std::isfinite(o.tolleranzaPercento) ||
                !(o.tolleranzaPercento > 0.0) || o.tolleranzaPercento > 100.0)
                throw std::invalid_argument("Tolleranza non valida: usare un numero in (0,100]");
        } else throw std::invalid_argument("Opzione sconosciuta: " + arg);
    }
    return o;
}

std::vector<Punto2D> caricaPuntiCsv(const std::string& percorso) {
    std::ifstream file(percorso);
    if (!file) throw std::runtime_error("Impossibile aprire il circuito: " + percorso);
    std::string riga;
    if (!std::getline(file, riga)) throw std::runtime_error("CSV vuoto: " + percorso);
    std::vector<Punto2D> punti;
    while (std::getline(file, riga)) {
        // Gestisce anche CSV Windows con terminatori CRLF.
        if (!riga.empty() && riga.back() == '\r') riga.pop_back();
        if (riga.empty()) continue;
        const auto separatore = riga.find(',');
        if (separatore == std::string::npos) throw std::runtime_error("Riga CSV senza virgola: " + percorso);
        std::size_t lettoX = 0, lettoY = 0;
        const auto sx = riga.substr(0, separatore);
        const auto sy = riga.substr(separatore + 1);
        const double x = std::stod(sx, &lettoX);
        const double y = std::stod(sy, &lettoY);
        if (lettoX != sx.size() || lettoY != sy.size() || !std::isfinite(x) || !std::isfinite(y))
            throw std::runtime_error("Coordinate CSV non valide: " + percorso);
        const Punto2D p(x, y);
        if (punti.empty() || punti.back().distanza(p) > 1e-4) punti.push_back(p);
    }
    if (punti.size() > 3 && punti.front().distanza(punti.back()) < 0.5) punti.pop_back();
    if (punti.size() < 3) throw std::runtime_error("Servono almeno tre punti nel CSV");
    return punti;
}

std::vector<SezionePista> generaSezioni(const std::vector<CampionePista>& campioni) {
    std::vector<SezionePista> sezioni;
    sezioni.reserve(campioni.size());
    for (const auto& c : campioni) {
        const double nx = -c.tangente.getY();
        const double ny =  c.tangente.getX();
        const double x = c.punto.getX(), y = c.punto.getY();
        sezioni.emplace_back(Punto2D(x + 6.0*nx, y + 6.0*ny),
                             Punto2D(x - 6.0*nx, y - 6.0*ny));
    }
    return sezioni;
}

Veicolo veicoloGui() {
    const std::array<Gomma,4> gomme{{Gomma(0, TipoGomma::SOFT, .95),
                                     Gomma(1, TipoGomma::SOFT, .95),
                                     Gomma(2, TipoGomma::SOFT, .95),
                                     Gomma(3, TipoGomma::SOFT, .95)}};
    // Valori predefiniti della GUI, nessuna modifica al modello fisico.
    return Veicolo(798.0, 650000.0, 10.0, 15.0, .90, 1.50, 315.0/3.6, 18.0, gomme);
}

Traiettoria puntiControllo(bool laterale) {
    constexpr int n = 80;
    std::vector<double> geni(n, .5);
    if (laterale) {
        // Un'unica funzione periodica, indipendente dalla risoluzione di valutazione.
        // Oscillazioni contenute nel corridoio e senza rumore casuale.
        for (int i = 0; i < n; ++i) {
            const double u = static_cast<double>(i) / n;
            geni[i] = .5 + .08 * std::sin(2.0*PI*2.0*u)
                         + .04 * std::sin(2.0*PI*5.0*u);
        }
    }
    return Traiettoria(geni);
}

template <class T>
std::vector<T> sottocampiona(const std::vector<T>& dati, int numero) {
    if (numero < 3 || dati.size() % static_cast<std::size_t>(numero) != 0)
        throw std::invalid_argument("Campioni non annidati: richiedono divisibilita' esatta");
    const std::size_t passo = dati.size() / static_cast<std::size_t>(numero);
    std::vector<T> risultato;
    risultato.reserve(numero);
    for (std::size_t i = 0; i < dati.size(); i += passo) risultato.push_back(dati[i]);
    return risultato;
}

struct Misura {
    double tempoSecondi = 0.0;
    double lunghezzaMetri = 0.0;
    double curvaturaMassima = 0.0;
    double curvaturaP99 = 0.0;
    double costoMillisecondi = 0.0;
};

Misura misura(const Traiettoria& percorso,
             const std::vector<SezionePista>& sezioni,
             const Veicolo& veicolo,
             const ValutatoreTraiettoria& valutatore) {
    const auto punti = percorso.calcolaPunti(sezioni);
    if (punti.size() < 3) throw std::runtime_error("Traiettoria con meno di 3 punti");
    double lunghezza = 0.0;
    for (std::size_t i = 0; i < punti.size(); ++i) {
        const double ds = punti[i].distanza(punti[(i+1)%punti.size()]);
        if (!std::isfinite(ds) || ds <= 1e-10)
            throw std::runtime_error("Segmento della traiettoria nullo o non finito");
        lunghezza += ds;
    }
    auto curvature = valutatore.profiloCurvaturaPerGrafici(percorso, sezioni);
    for (const double k : curvature) {
        if (!std::isfinite(k) || k < 0.0)
            throw std::runtime_error("Curvatura non valida");
    }
    std::sort(curvature.begin(), curvature.end());
    const std::size_t indice99 = static_cast<std::size_t>(.99 * (curvature.size()-1));
    const auto inizio = std::chrono::steady_clock::now();
    const double tempo = valutatore.valutaTraiettoria(percorso, sezioni, veicolo);
    const auto fine = std::chrono::steady_clock::now();
    if (!std::isfinite(tempo) || !(tempo > 0.0) || !std::isfinite(lunghezza))
        throw std::runtime_error("Tempo o lunghezza non validi");
    return {tempo, lunghezza, curvature.back(), curvature[indice99],
            std::chrono::duration<double,std::milli>(fine-inizio).count()};
}

double deltaPct(double nuovo, double precedente) {
    return 100.0 * (nuovo - precedente) / precedente;
}

struct Riga {
    std::string pista;
    std::string traiettoria;
    int campioni = 0;
    Misura fissa;
    Misura diretta;
    double deltaFissa = std::numeric_limits<double>::quiet_NaN();
    double deltaDiretta = std::numeric_limits<double>::quiet_NaN();
};

void scriviCsvIntestazione(std::ofstream& out) {
    out << "pista,traiettoria,campioni,tempo_fisso_s,delta_fisso_pct,tempo_diretto_s,delta_diretto_pct,"
           "scarto_modalita_pct,lunghezza_fissa_m,lunghezza_diretta_m,curvatura_max_diretta_1_m,"
           "curvatura_p99_diretta_1_m,costo_fisso_ms,costo_diretto_ms\n";
}
void scriviCsv(std::ofstream& out, const Riga& r) {
    out << r.pista << ',' << r.traiettoria << ',' << r.campioni << ','
        << r.fissa.tempoSecondi << ',';
    if (std::isfinite(r.deltaFissa)) out << r.deltaFissa;
    out << ',' << r.diretta.tempoSecondi << ',';
    if (std::isfinite(r.deltaDiretta)) out << r.deltaDiretta;
    out << ',' << deltaPct(r.diretta.tempoSecondi, r.fissa.tempoSecondi) << ','
        << r.fissa.lunghezzaMetri << ',' << r.diretta.lunghezzaMetri << ','
        << r.diretta.curvaturaMassima << ',' << r.diretta.curvaturaP99 << ','
        << r.fissa.costoMillisecondi << ',' << r.diretta.costoMillisecondi << '\n';
}

bool eseguiCaso(const std::string& nomePista,
                const std::vector<Punto2D>& centro,
                const std::vector<int>& risoluzioni,
                bool laterale, double tolleranza,
                std::ofstream* csv) {
    const std::string nomeTraiettoria = laterale ? "laterale" : "centrale";
    const int massimoN = risoluzioni.back();
    const auto controllo = puntiControllo(laterale);
    const auto sezioniDense = generaSezioni(InterpolatorePista::campionaChiuso(centro, massimoN));
    const auto valoriDensi = controllo.ricampionaPeriodica(massimoN).getPosizioniLaterali();
    const Veicolo autoGui = veicoloGui();
    const ValutatoreTraiettoria valutatore;

    std::cout << "\n=== " << nomePista << " | " << nomeTraiettoria << " | "
              << "80 controlli fissi, spline C2 ===\n";
    std::cout << "N       T_annidato    dT%     T_diretto     dT%     scarto%   L_diretto_m   k99_diretto\n";
    std::vector<Riga> righe;
    for (const int n : risoluzioni) {
        const auto sezioniFisse = sottocampiona(sezioniDense, n);
        const Traiettoria traiettoriaFissa(sottocampiona(valoriDensi, n));
        const auto sezioniDirette = generaSezioni(InterpolatorePista::campionaChiuso(centro, n));
        const Traiettoria traiettoriaDiretta = controllo.ricampionaPeriodica(n);
        const Misura fisso = misura(traiettoriaFissa, sezioniFisse, autoGui, valutatore);
        const Misura diretto = misura(traiettoriaDiretta, sezioniDirette, autoGui, valutatore);
        Riga r{nomePista, nomeTraiettoria, n, fisso, diretto};
        if (!righe.empty()) {
            r.deltaFissa = deltaPct(fisso.tempoSecondi, righe.back().fissa.tempoSecondi);
            r.deltaDiretta = deltaPct(diretto.tempoSecondi, righe.back().diretta.tempoSecondi);
        }
        righe.push_back(r);
        if (csv) scriviCsv(*csv, r);
        std::cout << std::fixed << std::setprecision(3)
                  << std::setw(5) << n << "  " << std::setw(11) << fisso.tempoSecondi << "  ";
        if (std::isfinite(r.deltaFissa)) std::cout << std::showpos << std::setw(6) << std::setprecision(2) << r.deltaFissa << std::noshowpos;
        else std::cout << "   ---";
        std::cout << "   " << std::setw(11) << std::setprecision(3) << diretto.tempoSecondi << "  ";
        if (std::isfinite(r.deltaDiretta)) std::cout << std::showpos << std::setw(6) << std::setprecision(2) << r.deltaDiretta << std::noshowpos;
        else std::cout << "   ---";
        std::cout << "    " << std::showpos << std::setw(6) << std::setprecision(3)
                  << deltaPct(diretto.tempoSecondi, fisso.tempoSecondi) << std::noshowpos
                  << "   " << std::setw(11) << std::setprecision(2) << diretto.lunghezzaMetri
                  << "    " << std::setw(11) << std::setprecision(6) << diretto.curvaturaP99 << '\n';
    }
    // Criterio prudente, NON un certificato di accuratezza: due raffinamenti
    // successivi devono essere sotto soglia in entrambe le modalita'.
    if (righe.size() < 3) throw std::logic_error("Servono almeno tre risoluzioni");
    const bool stabile = [&] {
        for (std::size_t k = righe.size()-2; k < righe.size(); ++k) {
            if (std::abs(righe[k].deltaFissa) > tolleranza ||
                std::abs(righe[k].deltaDiretta) > tolleranza)
                return false;
        }
        return true;
    }();
    std::cout << "Esito diagnostico (ultimi 2 raffinamenti < " << tolleranza << "%): "
              << (stabile ? "STABILE NEL RANGE TESTATO" : "NON ANCORA STABILIZZATO") << '\n';
    return stabile;
}
} // namespace

int main(int argc, char** argv) {
    try {
        const Opzioni o = leggiOpzioni(argc, argv);
        std::vector<int> n = {600, 1200, 2400, 4800, 9600};
        if (o.esteso) n.push_back(19200);
        std::ofstream csv;
        if (!o.csv.empty()) {
            csv.open(o.csv);
            if (!csv) throw std::runtime_error("Non riesco a scrivere CSV: " + o.csv);
            csv << std::setprecision(12);
            scriviCsvIntestazione(csv);
        }
        std::cout << "APEX LAB - TEST DI CONVERGENZA NUMERICA (solo diagnostico)\n"
                  << "Stessi CSV, stesso modello fisico e stessi 80 geni a ogni N.\n"
                  << "Annidato = una geometria densa fissa, sottocampionata; "
                  << "diretto = ricampionamento normale per ogni N.\n"
                  << "Tolleranza: " << o.tolleranzaPercento << "% sugli ultimi due raffinamenti.\n";
        int stabili = 0;
        const auto spa = caricaPuntiCsv(o.spa);
        const auto monza = caricaPuntiCsv(o.monza);
        for (const auto& [nome, punti] : std::array<std::pair<std::string,std::vector<Punto2D>>,2>{{
                {"SPA",spa}, {"MONZA",monza}}}) {
            for (const bool laterale : {false,true}) {
                if (eseguiCaso(nome, punti, n, laterale, o.tolleranzaPercento,
                               o.csv.empty() ? nullptr : &csv)) ++stabili;
            }
        }
        if (csv && !csv.good()) throw std::runtime_error("Errore durante la scrittura del CSV");
        std::cout << "\nRiepilogo: " << stabili << "/4 casi stabili nel range testato.\n"
                  << "OK: diagnostica completata. Non implica convergenza matematica o accuratezza fisica.\n";
        if (!o.csv.empty()) std::cout << "CSV salvato in: " << o.csv << '\n';
        return 0; // Nessuna soglia non raggiunta rende il test 'fallito': e' diagnostico.
    } catch (const std::exception& e) {
        std::cerr << "ERRORE: " << e.what() << '\n';
        return 1;
    }
}
