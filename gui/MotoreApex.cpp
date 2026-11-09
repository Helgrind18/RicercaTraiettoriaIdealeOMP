#include "MotoreApex.hpp"

#include "AlgoritmoGenetico.hpp"
#include "GeneraTraiettorie.hpp"  // Nel tuo progetto questa intestazione contiene GeneratoreTraiettorie.
#include "ValutatoreTraiettoria.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"
#include "Punto2D.hpp"
#include "InterpolatorePista.hpp"
#include "ValidatoreGeometrico.hpp"

#include <QFile>
#include <QTextStream>
#include <QStringList>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
constexpr double eps = 1e-8;

double distanza(const QPointF &a, const QPointF &b) {
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

std::vector<SezionePista> creaSezioni(const GeometriaApex &g) {
    std::vector<SezionePista> out;
    out.reserve(static_cast<size_t>(g.centro.size()));
    for (qsizetype i = 0; i < g.centro.size(); ++i) {
        const auto &s = g.sinistro[i];
        const auto &d = g.destro[i];
        out.emplace_back(Punto2D(s.x(), s.y()), Punto2D(d.x(), d.y()));
    }
    return out;
}

QVector<QPointF> proietta(const Traiettoria &traiettoria, const GeometriaApex &g) {
    if (traiettoria.getNumeroPosizioni() != static_cast<int>(g.centro.size()))
        throw std::runtime_error("Numero campioni incompatibile con il circuito");
    QVector<QPointF> punti;
    punti.reserve(g.centro.size());
    for (int i = 0; i < traiettoria.getNumeroPosizioni(); ++i) {
        const double t = traiettoria.getPosizioneLaterale(i);
        punti.push_back(g.sinistro[i] * (1.0 - t) + g.destro[i] * t);
    }
    return punti;
}
QVector<double> estraiGeni(const Traiettoria &t) {
    QVector<double> g;
    g.reserve(t.getNumeroPosizioni());
    for (int i = 0; i < t.getNumeroPosizioni(); ++i)
        g.push_back(t.getPosizioneLaterale(i));
    return g;
}

// Si parte dal PROFILO VELOCITA originale del ValutatoreTraiettoria.
// Le grandezze derivate sono cinematiche stimate tra due sezioni, non
// comandi di acceleratore/freno ne' misure di telemetria reale.
void profiliPerGrafici(const QVector<QPointF>& punti, const std::vector<double>& velocita,
                      const std::vector<double>& curvatura,
                      QVector<double>& distanzaKm, QVector<double>& velocitaKmh,
                      QVector<double>& accelerazione, QVector<double>& frenata,
                      QVector<double>& laterale, QVector<double>& curvaturaPerKm) {
    const int n = static_cast<int>(punti.size());
    if (n < 3 || static_cast<int>(velocita.size()) != n ||
        static_cast<int>(curvatura.size()) != n)
        throw std::runtime_error("Velocita, curvatura e sezioni non corrispondenti");
    distanzaKm.resize(n); velocitaKmh.resize(n);
    accelerazione.resize(n); frenata.resize(n); laterale.resize(n);
    curvaturaPerKm.resize(n);
    double cumulata = 0.0;
    for (int i = 0; i < n; ++i) {
        const int succ = (i + 1) % n;
        const double ds = distanza(punti[i], punti[succ]);
        distanzaKm[i] = cumulata / 1000.0;
        velocitaKmh[i] = velocita[i] * 3.6;
        const double a = ds > 1e-8 ?
            (velocita[succ]*velocita[succ] - velocita[i]*velocita[i]) / (2.0 * ds) : 0.0;
        accelerazione[i] = std::max(0.0, a);
        frenata[i] = std::max(0.0, -a);
        // Curvatura prodotta dal ValutatoreTraiettoria: stessa formula,
        // nessun duplicato nel frontend. 1/m -> 1/km solo per il grafico.
        curvaturaPerKm[i] = 1000.0 * curvatura[i];
        laterale[i] = velocita[i]*velocita[i]*curvatura[i];
        cumulata += ds;
    }
}
}  // namespace

QVector<QPointF> caricaCentroPista(const QString &percorso) {
    QFile file(percorso);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error(("Non riesco ad aprire la pista: " + percorso).toStdString());
    QTextStream stream(&file);
    QVector<QPointF> points;
    bool intestazione = true;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (intestazione) { intestazione = false; continue; }
        line.replace(';', ',');
        const QStringList columns = line.split(',');
        if (columns.size() < 2) throw std::runtime_error("CSV della pista non valido");
        bool okX = false, okY = false;
        const double x = columns[0].toDouble(&okX), y = columns[1].toDouble(&okY);
        if (!okX || !okY || !std::isfinite(x) || !std::isfinite(y))
            throw std::runtime_error("Coordinate del circuito non valide");
        const QPointF p(x, y);
        if (points.isEmpty() || distanza(points.back(), p) > 1e-4)
            points.push_back(p);
    }
    if (points.size() > 3 && distanza(points.front(), points.back()) < 0.5)
        points.pop_back();
    if (points.size() < 3) throw std::runtime_error("Il file pista non contiene abbastanza punti");
    return points;
}

GeometriaApex creaGeometriaPista(const QVector<QPointF> &input, int n, double larghezza) {
    GeometriaApex geo;
    if (!(larghezza > 0.0) || !std::isfinite(larghezza))
        throw std::invalid_argument("Larghezza pista non valida");

    std::vector<Punto2D> punti;
    punti.reserve(input.size());
    for (const QPointF &p : input)
        punti.emplace_back(p.x(), p.y());

    // Interpolazione periodica + distanza d'arco: una sola geometria
    // utilizzata sia dalla GUI sia dal valutatore C++ originale.
    const auto campioni = InterpolatorePista::campionaChiuso(punti, n);
    geo.centro.reserve(n);
    geo.sinistro.reserve(n);
    geo.destro.reserve(n);
    for (const auto &c : campioni) {
        const QPointF centro(c.punto.getX(), c.punto.getY());
        const QPointF direzione(c.tangente.getX(), c.tangente.getY());
        const QPointF normale(-direzione.y(), direzione.x());
        geo.centro.push_back(centro);
        geo.sinistro.push_back(centro + normale * (larghezza * 0.5));
        geo.destro.push_back(centro - normale * (larghezza * 0.5));
    }
    return geo;
}

RisultatoApex eseguiOttimizzazioneApex(const ParametriApex &p, const QString &fileCsv,
        const std::function<void(int, int, double)> &progresso) {
    if (p.sezioni < 3 || p.campioniValutazione < 3 || p.campioniValutazione > 20000 ||
        p.popolazione < 2 || p.generazioni < 1 ||
        p.torneo < 2 || p.torneo > p.popolazione || p.elite < 0 || p.elite >= p.popolazione)
        throw std::invalid_argument("Configurazione dell'algoritmo non valida");
    if (!std::isfinite(p.larghezzaVeicoloMetri) || p.larghezzaVeicoloMetri <= 0.0 ||
        !std::isfinite(p.margineExtraMetri) || p.margineExtraMetri < 0.0)
        throw std::invalid_argument("Parametri geometrici della vettura non validi");
    if (!(p.grip > 0 && p.grip <= 1))
        throw std::invalid_argument("Il grip deve essere nell'intervallo (0,1]");

    RisultatoApex risultato;
    risultato.parametri = p;
    risultato.geometria = creaGeometriaPista(caricaCentroPista(fileCsv), p.campioniValutazione);
    const auto sezioni = creaSezioni(risultato.geometria);

    const std::array<Gomma, 4> gomme = {
        Gomma(0, TipoGomma::SOFT, p.grip), Gomma(1, TipoGomma::SOFT, p.grip),
        Gomma(2, TipoGomma::SOFT, p.grip), Gomma(3, TipoGomma::SOFT, p.grip)
    };
    const Veicolo veicolo(p.massaKg, p.potenzaKW * 1000.0, p.accelerazione,
                          p.frenata, p.cd, p.areaM2, p.velocitaKmh / 3.6,
                          p.accelerazioneLaterale, gomme);
    ValutatoreTraiettoria valutatore;

    AlgoritmoGenetico algoritmo(p.popolazione, p.generazioni, p.crossover,
                               p.mutazione, p.intensita, p.torneo, p.elite, p.seed);
    const double margineRichiesto = p.larghezzaVeicoloMetri * 0.5 + p.margineExtraMetri;
    algoritmo.impostaMargineMinimo(margineRichiesto);
    const auto start = std::chrono::steady_clock::now();
    // Una sola generazione e valutazione iniziale, gestita dal genetico.
    // Il valutatore riceve sempre p.campioniValutazione sezioni.
    const RisultatoGenetico risultatoGA = algoritmo.ottimizzaConDettagli(
        sezioni, veicolo, p.sezioni,
        [&progresso, &start](int completate, int totale) {
            if (!progresso) return;
            const double secondi = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start).count();
            progresso(completate, totale, secondi);
        });
    const auto stop = std::chrono::steady_clock::now();

    const Traiettoria inizialeDensa = risultatoGA.miglioreIniziale.ricampionaPeriodica(p.campioniValutazione);
    const Traiettoria ottimaDensa = risultatoGA.miglioreFinale.ricampionaPeriodica(p.campioniValutazione);
    risultato.tempoIniziale = risultatoGA.tempoIniziale;
    risultato.tempoFinale = risultatoGA.tempoFinale;
    risultato.durataMs = std::chrono::duration<double, std::milli>(stop-start).count();
    const auto checkIniziale = ValidatoreGeometrico::analizza(inizialeDensa, sezioni, margineRichiesto);
    const auto checkFinale = ValidatoreGeometrico::analizza(ottimaDensa, sezioni, margineRichiesto);
    risultato.margineMinimoIniziale = checkIniziale.margineMinimoMetri;
    risultato.margineMinimoFinale = checkFinale.margineMinimoMetri;
    risultato.validaIniziale = checkIniziale.geometricamenteValida();
    risultato.validaFinale = checkFinale.geometricamenteValida();
    if (!risultato.validaIniziale || !risultato.validaFinale)
        throw std::runtime_error("Controllo finale dei bordi fallito: traiettoria non ammissibile");
    for (const auto& st : risultatoGA.storico) {
        risultato.generazioniStorico.push_back(st.generazione);
        risultato.miglioriGenerazione.push_back(st.miglioreGenerazione);
        risultato.medieGenerazione.push_back(st.mediaValidi);
        risultato.miglioriGlobali.push_back(st.miglioreGlobale);
        risultato.individuiValidi.push_back(st.individuiValidi);
        risultato.individuiNonValidi.push_back(st.individuiNonValidi);
    }
    risultato.iniziale = proietta(inizialeDensa, risultato.geometria);
    risultato.ottimizzata = proietta(ottimaDensa, risultato.geometria);
    risultato.geniIniziali = estraiGeni(risultatoGA.miglioreIniziale);
    risultato.geniOttimizzati = estraiGeni(risultatoGA.miglioreFinale);
    risultato.posizioniCampionateIniziali = estraiGeni(inizialeDensa);
    risultato.posizioniCampionateOttimizzate = estraiGeni(ottimaDensa);
    // I profili numerici e il disegno usano esattamente la stessa geometria densa.
    const auto vI = valutatore.profiloVelocitaPerGrafici(inizialeDensa, sezioni, veicolo);
    const auto vO = valutatore.profiloVelocitaPerGrafici(ottimaDensa, sezioni, veicolo);
    const auto kI = valutatore.profiloCurvaturaPerGrafici(inizialeDensa, sezioni);
    const auto kO = valutatore.profiloCurvaturaPerGrafici(ottimaDensa, sezioni);
    profiliPerGrafici(risultato.iniziale, vI, kI, risultato.distanzaInizialeKm,
                     risultato.velocitaInizialeKmh, risultato.accelerazioneIniziale,
                     risultato.frenataIniziale, risultato.lateraleIniziale,
                     risultato.curvaturaInizialePerKm);
    profiliPerGrafici(risultato.ottimizzata, vO, kO, risultato.distanzaOttimizzataKm,
                     risultato.velocitaOttimizzataKmh, risultato.accelerazioneOttimizzata,
                     risultato.frenataOttimizzata, risultato.lateraleOttimizzata,
                     risultato.curvaturaOttimizzataPerKm);
    if (!std::isfinite(risultato.tempoFinale) || !std::isfinite(risultato.tempoIniziale))
        throw std::runtime_error("Il valutatore ha restituito un tempo non finito");
    return risultato;
}
