#ifndef REQUEST_H
#define REQUEST_H

#include <QtCore>

class Request
{
public:
	enum Method{
		Method_Unknown = 0,
		Method_Get = 1,
		Method_Post = 2,
		Method_Head = 3,
	};

	typedef struct{
		short majorVersion = 0;
		short minorVersion = 0;
	}Version;

	static QString regenerateKey(const QString &str){
		QStringList keySep = str.split("-", QString::KeepEmptyParts);
		QStringList newKeySep;
		foreach(QString key, keySep){
			if(key.isEmpty()){
				continue;
			}
			QChar firstLetter = key.at(0).toUpper();
			QString restLetters = key.mid(1).toLower();
			newKeySep.append(QString(firstLetter).append(restLetters));
		}
		return newKeySep.join("-");
	}
private:
	Method method = Method_Unknown;
	QString url;
	QString fullUrl;
	QString ctrl;
	Version version;

	QHash<QString, QString> query; // ?a=b&c=d
	QHash<QString, QString> header; // Accept: xxx
	QHash<QString, QString> cookie; // cookie: name=value
	QByteArray content;

	bool vaild = false;

	void analysisCookie(const QString &cookieStr){
		QStringList cookieKVList = cookieStr.split("; ", QString::SkipEmptyParts);
		foreach(const QString &cookiePart, cookieKVList){
			int cookieEqualIndex = cookiePart.indexOf("=");
			if(cookieEqualIndex < 0){
				this->cookie.insert(cookiePart.trimmed(), QString());
			}else{
				this->cookie.insert(cookiePart.left(cookieEqualIndex).trimmed(), cookiePart.mid(cookieEqualIndex +1).trimmed());
			}
		}
	}

public:
	bool _headerComplete = false;
	QString _halfLine;

	Request(){ }

	void setMethod(Method method){ this->method = method; }
	void setMethod(const QString &method){
		if(method == "GET"){
			this->method = Method_Get;
		}else if(method == "POST"){
			this->method = Method_Post;
		}else if(method == "HEAD"){
			this->method = Method_Head;
		}else{
			this->method = Method_Unknown;
		}
	}
	const Method getMethod(){ return this->method; }

	bool setUrl(const QString &url){
		QUrl _url = QUrl::fromPercentEncoding(url.toUtf8());
		if(!_url.isValid()){
			return false;
		}
		this->url = _url.path();
		this->fullUrl = url;
		if(_url.query().isEmpty()){
			return true;
		}
		QStringList queryList = _url.query(QUrl::EncodeDelimiters).split("&");
		foreach(const QString &query, queryList){
			int valueIndex = query.indexOf("=");
			if(valueIndex < 0){
				this->query.insert(query, QString());
				continue;
			}else{
				this->query.insert(
					query.left(valueIndex),
					QByteArray::fromPercentEncoding(query.mid(valueIndex +1).toUtf8())
				);
				continue;
			}
		}
		return true;
	}

	const QString getUrl() const{
		return this->url;
	}

	const QString getFullUrl() const{
		return this->fullUrl;
	}

	bool setCtrl(const QString &ctrl){
		int spliter = ctrl.indexOf("/");
		if(spliter < 0){
			this->ctrl = ctrl;
			return false;
		}else{
			this->ctrl = ctrl.left(spliter);
			QString version = ctrl.mid(spliter +1);
			QStringList versionList = version.split(".");
			if(versionList.length() != 2){
				return false;
			}else{
				this->version.majorVersion = versionList.at(0).toInt();
				this->version.minorVersion - versionList.at(1).toInt();
				return true;
			}
		}
	}

	void setCtrl(const QString &ctrl, const Version &version){
		this->ctrl = ctrl;
		this->version = version;
	}

	const QString getCtrl() const{
		return this->ctrl;
	}

	const QString getCtrlAndVersion() const{
		if(this->version.minorVersion){
			return QString("%1/%2.%3").arg(this->ctrl).arg(this->version.majorVersion).arg(this->version.minorVersion);
		}else{
			return QString("%1/%2").arg(this->ctrl).arg(this->version.majorVersion);
		}

	}

	void insertHeader(const QString &key, const QString &value){
		this->header.insert(this->regenerateKey(key), value);
		if(key == "Cookie"){
			this->analysisCookie(value);
		}

	}

	void removeHeader(const QString &key){
		this->header.remove(key);
		if(key == "Cookie"){
			this->cookie.clear();
		}
	}

	void getHeader(const QString &key, const QString &defaultValue = QString()){
		return this->header.value(key, defaultValue);
	}

	void getHeaderLength(){
		return this->header.count();
	}

	void setContent(const QByteArray &content){
		this->content = content;
	}

	void appendContent(const QByteArray &content){
		this->content.append(content);
	}

	const QByteArray &getContent(){
		return this->content;
	}

	int getContentLength(){
		return this->content.length();
	}

	const QHash<QString, QString> getContentQuery(){
		QHash<QString, QString> contentQuery;
		if(this->content.isEmpty()){
			return contentQuery;
		}
		QList<QByteArray> queryList = this->content.split('&');
		foreach(const QByteArray &query, queryList){
			QString decodeQuery = QString(QByteArray::fromPercentEncoding(query));
			int valueIndex = decodeQuery.indexOf("=");
			if(valueIndex < 0){
				contentQuery.insert(decodeQuery, QString());
				continue;
			}else{
				contentQuery.insert(decodeQuery.left(valueIndex), decodeQuery.mid(valueIndex +1));
				continue;
			}
		}
		return contentQuery;
	}

	void setFinished(bool valid){
		this->vaild = vaild;
	}

	bool isValid(){
		return (!this->method == Method_Unknown &&
				!this->url.isEmpty() &&
				!this->ctrl.isEmpty() &&
				this->version.majorVersion != 0 &&
				this->valid);
	}

	QString getCookie(const QString &key){
		return this->cookie.value(key);
	}
};

#endif // REQUEST_H
