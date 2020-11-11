#ifndef _ELAPSED_TIMER_H_
#define _ELAPSED_TIMER_H_

#include <QTimer>
#include <QTime>

class ElapsedTimer : public QTimer
{
    Q_OBJECT

    public:
        ElapsedTimer(QObject* parent) : QTimer(parent)
        {
            connect(this, SIGNAL(timeout()), this, SLOT(resettime()));
        }

        void start()
        {
            m_time.start();
            QTimer::start();
        }

        void start(int msec)
        {
            m_time.start();
            QTimer::start(msec);
        }

        double proportionLeft()
        {
            return (interval()-m_time.elapsed())/interval();
        }

        int timeLeft()
        {
            return interval()-m_time.elapsed();
        }

    private slots:

        void resettime()
        {
            m_time.restart();
        }

    private:
        QTime m_time;
};

#endif//_ELAPSED_TIMER_H_
