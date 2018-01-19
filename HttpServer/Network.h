#ifndef NETWORK_H
#define NETWORK_H

#include <QtCore>
#include <QtConcurrent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork>
#include "Protocol.h"
#include "Handle.h"
#include "Helper.h"
#include <functional>

typedef struct{
	QThread *thread;
	Handle *handle;
}Client;

class Network : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Network)

	QHostAddress address;
	int port = -1;

	Helper *helper = new Helper(this);

	QTcpServer *server = new QTcpServer(this);
	QMap<qint64, Client> clientMap;
	QMap<qint64, Request> clientReqMap;

	std::function<Response(const Request &)> callBackFunc = nullptr;

	void insertClientMap(qint64 tid, const Client &client){
		this->clientMap.insert(tid, client);
		qDebug() << "客户端列表 增加一条:" << tid;
		qDebug() << "现在客户端列表中有:" << this->clientMap.keys();
	}

	void removeClientMap(qint64 tid){
		this->clientMap.remove(tid);
		qDebug() << "客户端列表 减少一条:" << tid;
		qDebug() << "现在客户端列表中有:" << this->clientMap.keys();
	}
public:
	explicit Network(const QHostAddress &hostAdress, int port, QObject *parent = nullptr) : QObject(parent){
		this->address = hostAdress;
		this->port = port;

		connect(this->helper, &Helper::timeout, this, [this](qint64 tid){
			Client client = this->clientMap.value(tid);
			if(client.handle == nullptr){
				qDebug() << "所传tid:" << tid << "没有在客户端列表中.";
				return;
			}
			client.handle->msgSendBadRequest();
			return;
		});
	}
	~Network(){
		foreach(Client p, this->clientMap){
			if(p.thread->isRunning()){
				p.thread->quit();
				p.thread->wait();
			}
		}
	}

	inline bool isRunning(){
		return this->server->isListening();
	}

	void setCallbackFunc(const std::function<Response(const Request &)> &callbackFunc){
		this->callBackFunc = callbackFunc;
	}
signals:
	void serverStarted();
	void serverStopped();
public slots:
	bool startServer(){
		if(this->isRunning()){
			qDebug() << "服务器正在运行, 不需要再次开启.";
			return false;
		}
		if(!this->server->listen(this->address, this->port)){
			qDebug() << "服务器不能绑定端口" << this->port << ".";
			return false;
		}
		connect(this->server, &QTcpServer::newConnection, this, &Network::newConnection);
		qDebug() << "服务器已开启在端口:" << this->port;
		emit this->serverStarted();
		return true;
	}

	void stopServer(){
		if(!this->isRunning()){
			qDebug() << "服务器未在运行, 不需要执行关闭.";
			return;
		}
		QList<qint64> tidList = this->clientMap.keys();
		foreach(qint64 tid, tidList){
			this->deleteConnection(tid);
		}
		this->server->close();
		qDebug() << "服务器已关闭.";
		emit this->serverStopped();
		return;
	}
private slots:
	void newConnection(){
		QThread *handleThread = new QThread(this);
		Client client;
		client.thread = handleThread;

		QTcpSocket *socket = this->server->nextPendingConnection();
		Handle *handle = new Handle(socket);
		client.handle = handle;
		connect(handle, &Handle::handleConnected, this, &Network::clientConnected);
		connect(handle, &Handle::handleDisconnected, this, &Network::clientDisconnected);
		connect(handle, &Handle::finished, this, &Network::deleteConnection);
		connect(handle, &Handle::msgReceived, this, &Network::receive);

		connect(handleThread, &QThread::started, handle, &Handle::open);
		connect(handleThread, &QThread::finished, handle, &Handle::close);

		handle->moveToThread(handleThread);
		handleThread->start();

		qint64 tid = (qint64)handle->thread();

		//this->clientMap.insert(tid, client);
		this->insertClientMap(tid, client);

		qDebug() << "Network: 有客户接入, 开启倒数";
		this->helper->insert(tid);
		this->helper->start(tid);
		qDebug() << "Network: 客户接入倒数设置成功.";
	}

	void deleteConnection(qint64 tid){
		qDebug() << "客户端断开! 来源: handle::finished";
		Client client = this->clientMap.value(tid);
		if(client.thread == nullptr){
			qDebug() << "所传tid:" << tid << "没有在客户端列表中.";
			qDebug() << "===== 客户端列表有:" << this->clientMap.keys();
			return;
		}
		client.handle->deleteLater();
		qDebug() << "Client Thread:" << tid << "quitting and waiting...";
		client.thread->quit();
		client.thread->wait();
		qDebug() << "Client Thread:" << tid << "quitted.";

		//this->clientMap.remove(tid);
		this->removeClientMap(tid);
		qDebug() << "Network: 客户端断开, 关闭倒数";
		this->helper->stop(tid);
		this->helper->remove(tid);
		qDebug() << "Network: 客户断开倒数取消成功.";
	}

	void receive(qint64 tid, const QByteArray &dat){
		Client client = this->clientMap.value(tid);
		if(client.handle == nullptr){
			qDebug() << "所传tid:" << tid << "没有在客户端列表中.";
			return;
		}
		//const Request request = Protocol::analysisRequest(newDat);
		Request request = this->clientReqMap.value(tid);
		bool analysisState = Protocol::analysisRequest(dat, &request);
		if(analysisState == false){
			// 协议解释失败 返回400
			client.handle->msgSendBadRequest();
			return;
		}
		if(request.isValid() == false){
			// 协议解释成功, 但是现在还不能用, 因为数据没有传送完, 留下等待下一次数据进来继续解释.
			this->clientReqMap.insert(tid, request);
			return;
		}else{
			// Means transferred completely.
			this->helper->restart(tid);
			if(this->callBackFunc == nullptr){
				qDebug() << "CallBack函数未设定.";
				client.handle->msgSendInternalError();
				return;
			}
			Response response = this->callBackFunc(request);
			if(response.isValid() == false){
				qDebug() << "CallBack函数返回response不可用.";
				client.handle->msgSendInternalError();
				return;
			}

			// 判断支持的HTTP类型
			/* 支持的协议:
			 * 方法: GET, POST, HEAD
			 * 位置: 必须以'/'开头来寻找地址
			 * 协议: HTTP/1.0以上
			 * 支持的内容: text/html, text/plain, application/javascript...
			 * 支持Transfer-Encoding: chunked
			 */


			//client.handle->msgSend(Protocol::makeupResponse(response));
			client.handle->msgSend(response.toByteArray());
			client.handle->close();
			this->clientReqMap.remove(tid);
		}
	}

	void clientConnected(qint64 tid){
		Q_UNUSED(tid);
	}
	void clientDisconnected(qint64 tid){
		Q_UNUSED(tid);
	}
};

#endif // NETWORK_H
