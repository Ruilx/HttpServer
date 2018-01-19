#ifndef HANDLE_H
#define HANDLE_H

#include <QtCore>
#include <QtNetwork>
#include "Protocol.h"

class Handle : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Handle)

	QTcpSocket *socket = nullptr;
	int timerId = -1;
	qint64 byteAvailable = 0;
	bool alreadyReplied = false;

public:
	explicit Handle(QTcpSocket *socket) : QObject(nullptr){
		if(socket == nullptr){
			qDebug() << "[HS:H]:" << __func__ << ": Socket is nullptr.";
			return;
		}
		this->socket = socket;
	}

	~Handle(){
		if(this->isRunning()){
			this->socket->waitForBytesWritten();
			this->socket->close();
		}
		if(this->socket != nullptr){
			this->socket->deleteLater();
		}
	}

	bool isRunning(){
		return this->socket->isOpen();
	}

signals:
	void msgReceived(qint64 threadId, const QByteArray &dat);
	void finished(qint64 threadId);
	void handleConnected(qint64 threadId);
	void handleDisconnected(qint64 threadId);
	void msgBadRequest(qint64 threadId);
private slots:
	void disconnected(){
		qDebug() << "[HS:H]:" << __func__ << ": ClientId:" << (qint64)QThread::currentThread() << "Disconnected.";
		emit this->handleDisconnected((qint64)QThread::currentThread());
		emit this->finished((qint64)QThread::currentThread());
	}
	void doJoinSelf(){
		qDebug() << "[HS:H]:" << __func__ << ": ClientId:" << (qint64)QThread::currentThread() << "Self's not vaild.";
		emit this->handleDisconnected((qint64)QThread::currentThread());
		emit this->finished((qint64)QThread::currentThread());
	}
	void read(){
		if(this->timerId == -1){
			this->timerId = this->startTimer(10);
		}
	}
public slots:
	bool open(){
		if(this->socket == nullptr){
			qDebug() << "[HS:H]:" << __func__ << ": Socket is nullptr.";

			return false;
		}

		this->byteAvailable = 0;
		this->timerId = this->startTimer(10);
		connect(this->socket, &QTcpSocket::disconnected, this, &Handle::disconnected);
		connect(this->socket, &QTcpSocket::readyRead, this, &Handle::read);
		emit this->handleConnected((qint64)QThread::currentThread());
		qDebug() << "[HS:H]:" << __func__ << ": ClientId:" << (qint64)QThread::currentThread() << "Connected.";
		return true;
	}
	void msgSend(const QByteArray &dat, bool isLast = false){
		if(dat.isNull() || dat.isEmpty()){
			qDebug() << "[HS:H]:" << __func__ << ": Will send an empty or null message document. Abandoned.";
			return;
		}
//		if(QThread::currentThread() != this->thread()){
//			QMetaObject::invokeMethod(this, "msgSend", Qt::QueuedConnection, Q_ARG(QByteArray, dat));
//			return;
//		}
		if(this->alreadyReplied){
			qDebug() << "[HS:H]:" << __func__ << ": This session is already replied.";
			return;
		}
		this->alreadyReplied = true;
		this->socket->write(dat);
	}
	void msgSendBadRequest(){
		QByteArray dat = Response::makeup400Response();
		this->msgSend(dat);
		this->close();
	}
	void msgSendInternalError(){
		QByteArray dat = Response::makeup500Response();
		this->msgSend(dat);
		this->close();
	}
	void close(){
		if(!this->socket->isOpen()){
			qDebug() << "[HS:H]:" << __func__ << ": socket is already closed.";
			return;
		}
		this->socket->waitForBytesWritten();
		this->socket->close();
	}
protected:
	void timerEvent(QTimerEvent *){
		qint64 bytes = this->byteAvailable;
		this->byteAvailable = this->socket->bytesAvailable();
		if(this->byteAvailable != bytes){
			return;
		}
		killTimer(this->timerId);
		this->timerId = -1;
		if(this->byteAvailable == 0){
			return;
		}
		this->byteAvailable = 0;

		QByteArray dat = this->socket->readAll();
		if(dat.isNull() || dat.isEmpty()){
			qDebug() << "[HS:H]:" << __func__ << ": Received an empty or null message document. Abandoned.";
			return;
		}
		qDebug() << "Receive: tid:" << (qint64)QThread::currentThread() << "Messages.";
		emit this->msgReceived((qint64)QThread::currentThread(), dat);
	}
};

#endif // HANDLE_H
