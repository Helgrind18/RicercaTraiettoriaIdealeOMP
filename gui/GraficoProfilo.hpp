#ifndef APEX_LAB_GRAFICO_PROFILO_HPP
#define APEX_LAB_GRAFICO_PROFILO_HPP

#include <QWidget>
#include <QSizePolicy>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QRectF>
#include <QString>
#include <algorithm>
#include <cmath>
#include <utility>

// Grafico C++/Qt senza Qt Charts: nessuna dipendenza aggiuntiva.
// Le due curve condividono gli assi, ma possono avere distanze percorse
// diverse, perche' le due traiettorie hanno lunghezze differenti.
class GraficoProfilo : public QWidget {
public:
    explicit GraficoProfilo(QString unitaY, QWidget* parent = nullptr)
        : QWidget(parent), unitaY_(std::move(unitaY)) {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    void impostaSerie(const QVector<double>& xIniziale, const QVector<double>& yIniziale,
                     const QVector<double>& xOttimizzata, const QVector<double>& yOttimizzata) {
        xi_ = xIniziale; yi_ = yIniziale;
        xo_ = xOttimizzata; yo_ = yOttimizzata;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor("#0B1726"));
        const QRectF area(61, 27, std::max(1, width()-84), std::max(1, height()-70));
        if (area.width() < 20 || area.height() < 20) return;

        const bool vuoto = (yi_.isEmpty() && yo_.isEmpty());
        double xmax = 0.0, ymax = 0.0;
        for (auto x : xi_) if (std::isfinite(x)) xmax = std::max(xmax, x);
        for (auto x : xo_) if (std::isfinite(x)) xmax = std::max(xmax, x);
        for (auto y : yi_) if (std::isfinite(y)) ymax = std::max(ymax, y);
        for (auto y : yo_) if (std::isfinite(y)) ymax = std::max(ymax, y);
        xmax = xmax > 0.0 ? xmax : 1.0;
        ymax = ymax > 0.0 ? ymax * 1.08 : 1.0;

        p.setFont(QFont("Sans Serif", 9));
        for (int i = 0; i <= 4; ++i) {
            const double f = double(i)/4.0;
            const double y = area.bottom() - f*area.height();
            p.setPen(QPen(QColor("#293B50"), 1));
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#8BA5BB"));
            p.drawText(QRectF(2, y-10, 52, 20), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(ymax*f, 'f', ymax > 30.0 ? 0 : 1));
        }
        p.setPen(QColor("#91ADC5"));
        p.drawText(QRectF(4, 2, 160, 19), Qt::AlignLeft, unitaY_);
        for (int i = 0; i <= 4; ++i) {
            const double x = area.left() + area.width()*double(i)/4.0;
            p.drawText(QRectF(x-24, area.bottom()+6, 48, 16), Qt::AlignCenter,
                       QString::number(xmax*i/4.0, 'f', 2));
        }
        p.drawText(QRectF(area.left(), height()-18, area.width(), 16), Qt::AlignCenter,
                   "DISTANZA DALLA PARTENZA [km]");
        if (vuoto) {
            p.setPen(QColor("#90A9C1"));
            p.drawText(area, Qt::AlignCenter, "Avvia un'ottimizzazione per mostrare il grafico");
            return;
        }
        auto disegna = [&](const QVector<double>& xs, const QVector<double>& ys,
                          const QColor& colore, bool tratteggiata) {
            const qsizetype n = std::min(xs.size(), ys.size());
            if (n < 2) return;
            QPainterPath path;
            bool iniziato = false;
            for (qsizetype i = 0; i < n; ++i) {
                if (!std::isfinite(xs[i]) || !std::isfinite(ys[i])) { iniziato = false; continue; }
                const QPointF pt(area.left() + area.width()*xs[i]/xmax,
                                 area.bottom() - area.height()*std::max(0., ys[i])/ymax);
                if (!iniziato) { path.moveTo(pt); iniziato = true; }
                else path.lineTo(pt);
            }
            QPen pen(colore, 1.7, tratteggiata ? Qt::DashLine : Qt::SolidLine);
            pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);
            p.drawPath(path);
        };
        p.save();
        p.setClipRect(area.adjusted(-2,-2,2,2));
        disegna(xi_, yi_, QColor("#FFB062"), true);
        disegna(xo_, yo_, QColor("#25E7DE"), false);
        p.restore();
        p.setPen(QColor("#FFB062"));
        p.drawText(QRectF(area.right()-203, 3, 100, 20), Qt::AlignLeft, "- - INIZIALE");
        p.setPen(QColor("#25E7DE"));
        p.drawText(QRectF(area.right()-94, 3, 100, 20), Qt::AlignLeft, "── OTTIM.");
    }

private:
    QString unitaY_;
    QVector<double> xi_, yi_, xo_, yo_;
};
#endif
