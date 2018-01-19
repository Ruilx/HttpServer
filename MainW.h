#ifndef MAINW_H
#define MAINW_H

#include <QMainWindow>
#include <QtWidgets>

#include "HttpServer/Protocol.h"
#include "HttpServer/Network.h"

class MainW : public QMainWindow
{
	Q_OBJECT

	QPlainTextEdit *textEdit = new QPlainTextEdit(this);
	Network *network = new Network(QHostAddress::Any, 23333, this);

	QMenu *protocolMenu = new QMenu("Protocol", this);
	QAction *checkHeaderAct = new QAction("Check Header", this);
	QAction *getA200Response = new QAction("Get A 200 Response", this);
	QAction *getA400Response = new QAction("Get A 400 Response", this);
	QAction *getA500Response = new QAction("Get A 500 Response", this);

	QMenu *serverMenu = new QMenu("Server", this);
	QAction *startServerAct = new QAction("Start Server", this);
	QAction *stopServerAct = new QAction("Stop Server", this);

	QMenu *testMenu = new QMenu("Test", this);
	QAction *testAct = new QAction("Test", this);

	void createMenus(){
		this->menuBar()->addMenu(this->protocolMenu);
		this->protocolMenu->addAction(this->checkHeaderAct);
		this->protocolMenu->addAction(this->getA200Response);
		this->protocolMenu->addAction(this->getA400Response);
		this->protocolMenu->addAction(this->getA500Response);
		this->menuBar()->addMenu(this->serverMenu);
		this->serverMenu->addAction(this->startServerAct);
		this->serverMenu->addAction(this->stopServerAct);
		this->menuBar()->addMenu(this->testMenu);
		this->testMenu->addAction(this->testAct);
	}

	void closeEvent(QCloseEvent *){
		if(this->network->isRunning()){
			this->network->stopServer();
		}
	}

	Response makeA404Response(){
		Response res = Protocol::getDefaultResponse(404);
		res.setContent("<html><head><title>404 Not Found</title></head><body><center><h1>您访问了一个不存在的页面 [404]</h1></center></body></html>");
		res.setFinished(true);
		return res;
	}

	const QByteArray readFile(const QString &filename){
		QFile file(filename);
		if(!file.open(QIODevice::ReadOnly)){
			qDebug() << "[主要]:" << __func__ << "读取文件失败:" << filename;
			return QByteArray();
		}
		const QByteArray content = file.readAll();
		if(file.isOpen()){
			file.close();
		}
		return content;
	}
public:
	MainW(QWidget *parent = 0): QMainWindow(parent){
		this->setCentralWidget(textEdit);
		this->createMenus();

		connect(this->checkHeaderAct, &QAction::triggered, this, [this](bool){
			if(textEdit->document()->isEmpty()){
				return;
			}
			Request rh;
			bool ok = Protocol::analysisRequest(this->textEdit->toPlainText().toUtf8().replace('\n', "\r\n"), &rh);
			if(ok == false){
				QMessageBox::information(this, QApplication::applicationName(), "Request Header:: Bad Request.", QMessageBox::Ok);
				return;
			}
			if(rh.isValid() == false){
				QMessageBox::information(this, QApplication::applicationName(), "Request Header:: Request not full.", QMessageBox::Ok);
				return;
			}else{
				QString str = "Success:: \n";
				str.append(QString("%1 %2 %3").arg(rh.getMethodString(), rh.getFullUrl(), rh.getCtrlAndVersion()));
				QStringList keys = rh.getHeaderKeys();
				foreach(const QString &key, keys){
					str.append(QString("\n %1 = %2").arg(key, rh.getHeader(key)));
				}
				str.append("\n").append(rh.getContent());
				QMessageBox::information(this, QApplication::applicationName(), str, QMessageBox::Ok);
			}
		});

		connect(this->getA200Response, &QAction::triggered, this, [this](bool){
			Response res;

			res.setContent("Yeah! Return 200 OK!");
			res.setStatusCode(200);
			res.setFinished(true);

			QMessageBox::information(this, QApplication::applicationName(), QString(res.toByteArray()), QMessageBox::Ok);
		});

		connect(this->getA400Response, &QAction::triggered, this, [this](bool){
			QMessageBox::information(this, QApplication::applicationName(), QString(Response::makeup400Response()), QMessageBox::Ok);
		});

		connect(this->getA500Response, &QAction::triggered, this, [this](bool){
			QMessageBox::information(this, QApplication::applicationName(), QString(Response::makeup500Response()), QMessageBox::Ok);
		});

		connect(this->startServerAct, &QAction::triggered, this, [this](bool){
			if(!this->network->startServer()){
				QMessageBox::critical(this, QApplication::applicationName(), QString("Server starting fails."), QMessageBox::Ok);
				return;
			}else{
				QMessageBox::information(this, QApplication::applicationName(), QString("Server startups."), QMessageBox::Ok);
				return;
			}
		});

		connect(this->stopServerAct, &QAction::triggered, this, [this](bool){
			this->network->stopServer();
			QMessageBox::information(this, QApplication::applicationName(), QString("Server closed."), QMessageBox::Ok);
		});

		connect(this->testAct, &QAction::triggered, this, [this](bool){
			QDir dir = QDir::current();
			qDebug() << "[主要]:" << __func__ << "dir.exists():" << dir.exists();
			qDebug() << "[主要]:" << __func__ << "dir.cd('webroot'):" << dir.cd("webroot");
			qDebug() << "[主要]:" << __func__ << "dir.exists('./'):" << dir.exists("./");
			qDebug() << "[主要]:" << __func__ << "dir.exists('./index.html'):" << dir.exists("./index.html");
			qDebug() << "[主要]:" << __func__ << "dir.exists('./demoB/demoB.html'):" << dir.exists("./demoB/demoB.html");
			qDebug() << "[主要]:" << __func__ << "dir.absoluteFilePath('index.html'):" << dir.absoluteFilePath("index.html");
			qDebug() << "[主要]:" << __func__ << "dir.absoluteFilePath('index.htm'):" << dir.absoluteFilePath("index.htm");
		});

		this->network->setCallbackFunc([this](const Request &request){
			return this->callBackFunction(request);
		});

	}


	const Response callBackFunction(const Request &request){
		Response res(200);

		qDebug() << "[主要]:" << __func__ << "获得本地相对地址:" << request.getUrlLocalRelativePath();

		QDir dir = QDir::current();
		if(!dir.cd("webroot")){
			// 'webroot' doesn't exists return 404 page
			qDebug() << "webroot不存在, 返回404页面";
			return this->makeA404Response();
		}
		if(dir.exists(request.getUrlLocalRelativePath())){
			// like '/root/demoA', demoA is a dir name
			qDebug() << "访问地址是一个目录地址...可能后面还会有指定的文件";
			if(!dir.cd(request.getUrlLocalRelativePath())){
				// '/root/demoA' doesn't exists return 404 page
				qDebug() << "访问地址的目录不存在, 返回404页面";
				return this->makeA404Response();
			}else{
				// '/root/demoA' exists, attach if there has index page.
				qDebug() << "访问地址的目录存在, 看看有木有index页面...";
				if(dir.exists("index.html")){
					// '.../index.html' exists
					res.setContent(this->readFile(dir.absoluteFilePath("index.html")));
					res.setFinished(true);
					return res;
				}else if(dir.exists("index.htm")){
					// '.../index.htm' exists
					res.setContent(this->readFile(dir.absoluteFilePath("index.htm")));
					res.setFinished(true);
					return res;
				}else{
					qDebug() << "没有找到index页面. 返回404页面";
					return this->makeA404Response();
				}
			}
		}else{
			// like 'root/demoA/xxx.html'
			qDebug() << "访问地址是一个文件.";
			QFileInfo info(dir, request.getUrlLocalRelativePath());
			if(info.exists()){
				res.setContent(this->readFile(info.absoluteFilePath()));
				res.setFinished(true);
				return res;
			}else{
				qDebug() << "没有找到该页面, 返回404页面.";
				return this->makeA404Response();
			}
		}

//			QString content = QString("You parse a %1 request to %2 page, using %3 method, it's success.<br>Request Headers:<br>").arg(request.ctrl, request.url, request.method);
//			QStringList keys = request.header.keys();
//			foreach(const QString &key, keys){
//				content.append(key).append(":").append(request.header.value(key)).append("<br>");
//			}

//			res.content = content.toUtf8();
//			res.header.insert("Content-Length", QString::number(res.content.length()));
//			res.valid = true;
		return res;
	}
	~MainW();
};

#endif // MAINW_H
