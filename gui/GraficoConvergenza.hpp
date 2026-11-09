#ifndef APEX_LAB_GRAFICO_CONVERGENZA_HPP
#define APEX_LAB_GRAFICO_CONVERGENZA_HPP
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QRectF>
#include <algorithm>
#include <cmath>
#include <limits>

// Storico di fitness in secondi per generazione, senza Qt Charts.
// Il grafico mostra il miglior individuo corrente, la media dei validi
// e il migliore globale, con asse Y NON forzato a zero.
class GraficoConvergenza : public QWidget {
    QVector<int> n_;
    QVector<double> best_, mean_, global_;
public:
    explicit GraficoConvergenza(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(290);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    void impostaSerie(const QVector<int>& n, const QVector<double>& best,
                      const QVector<double>& mean, const QVector<double>& glob) {
        n_=n; best_=best; mean_=mean; global_=glob; update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#0B1726"));
        const QRectF box(70, 38, std::max(1,width()-93), std::max(1,height()-95));
        if(box.width()<=20 || box.height()<=20) return;
        p.setFont(QFont("Sans Serif",9));
        if(n_.isEmpty()) {
            p.setPen(QColor("#90A9C1"));
            p.drawText(box, Qt::AlignCenter, "Avvia un'ottimizzazione per vedere la convergenza");
            return;
        }
        double lo=std::numeric_limits<double>::infinity(), hi=-lo;
        for(const auto* serie : {&best_,&mean_,&global_})
            for(double y:*serie) if(std::isfinite(y)) {lo=std::min(lo,y);hi=std::max(hi,y);}
        if(!std::isfinite(lo) || !std::isfinite(hi)) return;
        const double pad=std::max(0.005,(hi-lo)*.10);
        lo-=pad; hi+=pad;
        const int first=n_.front(), last=n_.back();
        const double den=std::max(1,last-first);
        for(int k=0;k<=4;++k) {
            const double y=box.bottom()-box.height()*k/4.;
            p.setPen(QPen(QColor("#293B50"),1));
            p.drawLine(QPointF(box.left(),y),QPointF(box.right(),y));
            p.setPen(QColor("#A2B7CA"));
            p.drawText(QRectF(0,y-11,62,20),Qt::AlignRight|Qt::AlignVCenter,
                       QString::number(lo+(hi-lo)*k/4.,'f',3));
        }
        p.setPen(QColor("#91ADC5"));
        for(int k=0;k<=4;++k) {
            const int step=first+static_cast<int>(std::round((last-first)*k/4.));
            const double x=box.left()+box.width()*k/4.;
            p.drawText(QRectF(x-28,box.bottom()+7,56,16),Qt::AlignCenter,QString::number(step));
        }
        p.drawText(QRectF(box.left(),height()-19,box.width(),16),Qt::AlignCenter,"GENERAZIONE");
        p.drawText(QRectF(2,4,130,19),Qt::AlignLeft,"TEMPO GIRO [s]");
        auto draw=[&](const QVector<double>& ys, const QColor& color, Qt::PenStyle style){
            const int count=std::min(n_.size(),ys.size());
            QPainterPath path; bool begun=false;
            for(int i=0;i<count;++i) {
                if(!std::isfinite(ys[i])) {begun=false;continue;}
                QPointF pt(box.left()+box.width()*(n_[i]-first)/den,
                           box.bottom()-box.height()*(ys[i]-lo)/(hi-lo));
                if(!begun){path.moveTo(pt);begun=true;}else path.lineTo(pt);
            }
            QPen pen(color,1.8,style); pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);p.drawPath(path);
        };
        p.save();p.setClipRect(box.adjusted(-2,-2,2,2));
        draw(mean_,QColor("#88A8C1"),Qt::DotLine);
        draw(best_,QColor("#FFB062"),Qt::DashLine);
        draw(global_,QColor("#25E7DE"),Qt::SolidLine);
        p.restore();
        p.setPen(QColor("#FFB062"));p.drawText(QPointF(box.left()+5,24),"- - MIGLIORE GEN.");
        p.setPen(QColor("#88A8C1"));p.drawText(QPointF(box.left()+160,24),"· · MEDIA VALIDI");
        p.setPen(QColor("#25E7DE"));p.drawText(QPointF(box.left()+315,24),"── MIGLIORE GLOBALE");
    }
};
#endif
