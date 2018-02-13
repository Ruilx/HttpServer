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
		this->checkClientMapState();
		qDebug() << "客户端列表 减少一条:" << tid;
		qDebug() << "现在客户端列表中有:" << this->clientMap.keys();
	}

	void checkClientMapState(){
		if(this->clientMap.isEmpty()){
			emit this->emptyClientMap();
		}
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
//				p.thread->quit();
//				p.thread->wait();
				p.handle->close();
			}
		}
		QEventLoop eventLoop;
		if(!this->clientMap.isEmpty()){
			connect(this, &Network::emptyClientMap, &eventLoop, &QEventLoop::quit);
			qDebug() << "Waiting for all thread quitted...";
			eventLoop.exec();
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
	void emptyClientMap();
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
//		QList<qint64> tidList = this->clientMap.keys();
//		foreach(qint64 tid, tidList){
//			this->deleteConnection(tid);
//		}
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
		connect(handleThread, &QThread::finished, handle, &Handle::deleteLater);

		handle->moveToThread(handleThread);
		handleThread->start();

		qint64 tid = (qint64)handle->thread();

		//this->clientMap.insert(tid, client);
		this->insertClientMap(tid, client);

//		qDebug() << "Network: 有客户接入, 开启倒数";
		this->helper->insert(tid);
		this->helper->start(tid);
//		qDebug() << "Network: 客户接入倒数设置成功.";
	}

	void deleteConnection(qint64 tid){
//		qDebug() << "客户端断开! 来源: handle::finished";
		Client client = this->clientMap.value(tid);
		if(client.thread == nullptr){
			qDebug() << "所传tid:" << tid << "没有在客户端列表中.";
			qDebug() << "===== 客户端列表有:" << this->clientMap.keys();
			return;
		}
		client.handle->deleteLater();
//		qDebug() << "Client Thread:" << tid << "quitting and waiting...";
		client.thread->quit();
		client.thread->wait();
//		qDebug() << "Client Thread:" << tid << "quitted.";

		//this->clientMap.remove(tid);
		this->removeClientMap(tid);
//		qDebug() << "Network: 客户端断开, 关闭倒数";
//		this->helper->stop(tid);
		this->helper->remove(tid); // remove stopping automatically.
//		qDebug() << "Network: 客户断开倒数取消成功.";
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

			do{
				// Check request standard protocol
				if(request.getCtrl() != "HTTP"){
					// HTTP check failed
					qDebug() << "Request Ctrl 'HTTP' not matched: " << request.getCtrl();
					client.handle->msgSendBadRequest();
					break;
				}
				if(request.getVersion().majorVersion + request.getVersion().minorVersion == 0){
					// no version found
					qDebug() << "Request HTTP version check failed.";
					client.handle->msgSendBadRequest();
					break;
				}
				if((request.getVersion().majorVersion == 1 && request.getVersion().minorVersion >= 1) ||
						(request.getVersion().majorVersion > 1)){
					// version upd HTTP/1.1 requires Host header.
					if(request.getHeader("Host").isEmpty()){
						qDebug() << "This request HTTP version requires Host header.";
						client.handle->msgSendBadRequest();
						break;
					}
				}

				if(this->callBackFunc == nullptr){
					qDebug() << "CallBack函数未设定.";
					client.handle->msgSendInternalError();
					break;
				}
				Response response = this->callBackFunc(request);
				if(response.isValid() == false){
					qDebug() << "CallBack函数返回response不可用.";
					client.handle->msgSendInternalError();
					break;
				}

				// 判断支持的HTTP类型
				/* 支持的协议:
				 * 方法: GET, POST, HEAD
				 * 位置: 必须以'/'开头来寻找地址
				 * 协议: HTTP/1.0以上
				 * 支持的内容: text/html, text/plain, application/javascript...
				 * 支持Transfer-Encoding: chunked
				 */

				//if(response.getHeader("Transfer-Encoding") == Response::getTransferEncodingString(Response::Chunked)){
				if(response.getTransferEncoding() == Response::Chunked){
					// Chunked
					QByteArray buffer(8192, 0);
					// Using Chunked mode cannot set Content-Length header.
					response.removeHeader("Content-Length");
					if(response.getContentMode() == Response::Content_Text){
						// Chunked Content:Text
						// Send Header.
						client.handle->msgSend(response.headerToByteArray());
						QDataStream bufferStream(response.getContent());
						while(!bufferStream.atEnd()){
							int length = bufferStream.readRawData(buffer.data(), buffer.length());
							client.handle->msgSend(QString::number(length, 16).append("\r\n").toUtf8());
							client.handle->msgSend(buffer.left(length));
							client.handle->msgSend(QByteArray("\r\n"));
						}
						client.handle->msgSend(QByteArray("0\r\n"), true);
					}else if(response.getContentMode() == Response::Content_File){
						// Chunked Content:File
						if(response.getContent().isEmpty()){
							qDebug() << "[网络]:" << __func__ << ":response指定的文件路径为空.";
							client.handle->msgSend(Response::makeup500Response(), true);
						}else{
							QFile file(QString::fromUtf8(response.getContent()));
							if(!file.open(QIODevice::ReadOnly)){
								qDebug() << "[网络]:" << __func__ << ":无法打开response指定的文件:" << file.fileName();
								client.handle->msgSend(Response::makeup404Response(), true);
							}else{
								client.handle->msgSend(response.headerToByteArray());
								QDataStream bufferStream(&file);
								while(!bufferStream.atEnd()){
									int length = bufferStream.readRawData(buffer.data(), buffer.length());
									client.handle->msgSend(QString::number(length, 16).append("\r\n").toUtf8());
									client.handle->msgSend(buffer.left(length));
									client.handle->msgSend(QByteArray("\r\n"));
								}
								file.close();
								client.handle->msgSend(QByteArray("0\r\n"), true);
							}
						}

					}else{
						// Chunked Cannot_determined_what_content_is
						qDebug() << "[网络]:" << __func__ << ":未知的ContentMode:" << response.getContentMode();
						client.handle->msgSend(Response::makeup500Response(), true);
					}
				}else if(response.getTransferEncoding() == Response::Normal){
					// Normal(identity)
					// Using Normal mode cannot set Transfer_Encoding header.
					response.removeHeader("Transfer_Encoding");
					if(response.getContentMode() == Response::Content_Text){
						// Normal Content:Text
						if(response.getHeader("Content-Length").isEmpty() || response.getHeader("Content-Length") == "-1"){
							response.insertHeader("Content-Length", QString::number(response.getContentLength()));
						}
						client.handle->msgSend(response.toByteArray(), true);
					}else if(response.getContentMode() == Response::Content_File){
						// Normal Content:File
						if(response.getContent().isEmpty()){
							qDebug() << "[网络]:" << __func__ << ":response指定的文件路径为空.";
							client.handle->msgSend(Response::makeup500Response(), true);
						}else{
							QFile file(QString::fromUtf8(response.getContent()));
							if(!file.open(QIODevice::ReadOnly)){
								qDebug() << "[网络]:" << __func__ << ":无法打开response指定的文件:" << file.fileName();
								client.handle->msgSend(Response::makeup404Response(), true);
							}else{
								client.handle->msgSend(response.headerToByteArray());
								client.handle->msgSend(file.readAll(), true);
								file.close();
							}
						}
					}else{
						// Normal Cannot_determined_what_content_is
						qDebug() << "[网络]:" << __func__ << ":未知的ContentMode:" << response.getContentMode();
						client.handle->msgSend(Response::makeup500Response(), true);
					}
				}

				if(request.getHeader("Connection") == "keep-alive" && response.getHeader("Connection") == "keep-alive"){
					client.handle->setResponseFinished();
					this->helper->restart(tid);
				}else{
					client.handle->close();
				}
			}while(false);

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
