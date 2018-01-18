#ifndef HELPER_H
#define HELPER_H

#include <QtCore>

class Helper : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Helper)
private:
	typedef struct TimerCom_t{
		QTimer *timer = nullptr;
	}TimerCom;

	QMap<qint64, TimerCom> timerTable; // QMap<tid, TimerCom>
	QMutex lock;

	inline void insertTimerTable(qint64 tid, const TimerCom &timerCom){
		this->timerTable.insert(tid, timerCom);
		qDebug() << "时间倒数表 插入一条:" << tid;
		qDebug() << "现在时间倒数表中有:" << this->timerTable.keys();
	}

	inline void removeTimerTable(qint64 tid){
		this->timerTable.remove(tid);
		qDebug() << "时间倒数表 删除一条:" << tid;
		qDebug() << "现在时间倒数表中有:" << this->timerTable.keys();
	}
public:
	explicit Helper(QObject *parent = nullptr): QObject(parent){
		this->timerTable.clear();
	}

	~Helper(){
		foreach(const TimerCom &timerc, this->timerTable){
			if(timerc.timer != nullptr && timerc.timer->isActive()){
				timerc.timer->stop();
			}
		}
	}

	void insert(qint64 tid, int timeout = 30000, bool singleShot = true){
		QTimer *timer = new QTimer(this);
		timer->setInterval(timeout);
		timer->setSingleShot(singleShot);

		TimerCom timerc;
		timerc.timer = timer;
		//this->timerTable.insert(tid, timerc);
		this->insertTimerTable(tid, timerc);
	}

	void start(qint64 tid){
		TimerCom timerCom = this->timerTable.value(tid);
		if(timerCom.timer == nullptr){
			qDebug() << "[HS:HELPER]:" << __func__ << ":timerID is nullptr:" << tid;
			return;
		}
		this->connect(timerCom.timer, &QTimer::timeout, this, [this, tid]{
			TimerCom timerCom = this->timerTable.value(tid);
			if(timerCom.timer == nullptr){
				qDebug() << "Timer表中不存在" << tid << "的项目, 不能执行发射超时指令.";
				return;
			}
			this->lock.lock();
			if(timerCom.timer->isSingleShot()){
				//timerCom.timer->disconnect();
				//timerCom.timer->deleteLater();
				//this->timerTable.remove(tid);
				//this->removeTimerTable(tid); // 这一条在Network里控制是否要删掉
			}
			this->lock.unlock();
			emit this->timeout(tid);
		});
		timerCom.timer->start();
	}

	void stop(qint64 tid){
		TimerCom timerCom = this->timerTable.value(tid);
		if(timerCom.timer == nullptr){
			qDebug() << "[HS:HELPER]:" << __func__ << ":timerID is nullptr:" << tid;
			return;
		}
		this->lock.lock();
		timerCom.timer->stop();
		this->lock.unlock();
	}

	void restart(qint64 tid){
		TimerCom timerCom = this->timerTable.value(tid);
		if(timerCom.timer == nullptr){
			qDebug() << "[HS:HELPER]:" << __func__ << ":timerID is nullptr:" << tid;
			qDebug() << "[HS:HELPER]:" << __func__ << ":timerTable:" << this->timerTable.keys();
			return;
		}
		this->lock.lock();
		timerCom.timer->start(timerCom.timer->interval());
		this->lock.unlock();
	}

	void remove(qint64 tid){
		TimerCom timerCom = this->timerTable.value(tid);
		if(timerCom.timer == nullptr){
			qDebug() << "[HS:HELPER]:" << __func__ << ":timerID is nullptr:" << tid;
			return;
		}
		this->lock.lock();
		timerCom.timer->stop();
		timerCom.timer->disconnect();
		//timerCom.timer->deleteLater();
		//this->timerTable.remove(tid);
		this->removeTimerTable(tid);
		this->lock.unlock();
	}

signals:
	void timeout(qint64 tid);
public slots:
};

#endif // HELPER_H
