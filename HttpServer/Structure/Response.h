#ifndef RESPONSE_H
#define RESPONSE_H

#include <QtCore>
//#include "../Protocol.h"
#include <QNetworkCookie>
#include <QLocale>

class Response
{
public:
	typedef struct{
		short majorVersion = 1;
		short minorVersion = 1;
	}Version;

	enum MakeupFlag{
		Strict = 0,
		NoMoreMakeups = 1
	};

	enum TransferEncoding{
		Normal = 0,
		Chunked = 1,
	};

	enum ContentType{
		Any_Any = 0,
		Text_Plain = 1,
		Text_Html = 2,
		Text_Css = 3,
		Text_Javascript = 4,
		Text_Json = 5,
		Image_Any = 100,
		Image_Gif = 101,
		Image_Png = 102,
		Image_Jpeg = 103,
		Image_Bmp = 104,
		Image_Webp = 105,
		Image_SvgPlusXml = 106,
		Audio_Any = 200,
		Audio_Midi = 201,
		Audio_Mpeg = 202,
		Audio_Webm = 203,
		Audio_Ogg = 204,
		Audio_Wav = 205,
		Audio_Mp3 = 206,
		Video_Any = 300,
		Video_Webm = 301,
		Video_Ogg = 302,
		Video_Mp4 = 303,
		Multipart_Any = 9000,
		Multipart_From_Data = 9001,
		Multipart_Byteranges = 9002,
		Application_Any = 10000,
		Application_Octet_Stream = 10001,
		Application_Pkcs12 = 10002,
		Application_VndDotMspowerpoint = 10003,
		Application_XhtmlPlusXml = 10004,
		Application_Xml = 10005,
		Application_Pdf = 10006,
		Application_Json = 10007,
	};

	enum Connection{
		Keep_Alive = 0,
		Close = 1,
	};

	enum CacheControl{
		Must_Revalidate = 1,
		No_Cache = 2,
		No_Store = 4,
		No_Transform = 8,
		Public = 16,
		Private = 32,
		Proxy_Revalidate = 64,
		Max_Age = 128,
		S_Maxage = 256,
	};
	Q_DECLARE_FLAGS(CacheControls, CacheControl)

	static QString formatDate(const QDateTime &date){
		const QStringList week = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
		const QStringList month = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

		const QDate d = date.date();
		const QTime t = date.time();
		return QString("%1, %2 %3 %4 %5:%6:%7 GMT")
				.arg(week.at(d.dayOfWeek()))
				.arg(int(d.day()), 2, 10, QChar('0'))
				.arg(month.at(d.month()))
				.arg(int(d.year()), 4, 10, QChar('0'))
				.arg(int(t.hour()), 2, 10, QChar('0'))
				.arg(int(t.minute()), 2, 10, QChar('0'))
				.arg(int(t.second()), 2, 10, QChar('0'));
	}

	static QHash<QString, QString> getDefaultResponseHeader(){
		Response r;
		return r.getHeader();
	}

	static const QString getStatusCodeReason(int statusCode){
		switch(statusCode){
			case 100: return QString("Continue");
			case 101: return QString("Switching Protocols");
			case 102: return QString("Processing");
			case 200: return QString("OK");
			case 201: return QString("Created");
			case 202: return QString("Accepted");
			case 203: return QString("Partial Information");
			case 204: return QString("No Response");
			case 205: return QString("Resetted");
			case 206: return QString("Parted");
			case 207: return QString("Multi Status");
			case 300: return QString("Multiple Choices");
			case 301: return QString("Moved Permanently");
			case 302: return QString("Moved Temporarily");
			case 303: return QString("See Other");
			case 304: return QString("Not Modified");
			case 305: return QString("Use Proxy");
			case 306: return QString("Switch Proxy");
			case 307: return QString("Temporary Redirect");
			case 400: return QString("Bad Request");
			case 401: return QString("Unauthorized");
			case 402: return QString("Payment Required");
			case 403: return QString("Forbidden");
			case 404: return QString("Not Found");
			case 405: return QString("Method Not Allowed");
			case 406: return QString("Not Acceptable");
			case 407: return QString("Proxy Authentication Required");
			case 408: return QString("Request Timeout");
			case 409: return QString("Conflict");
			case 410: return QString("Gone");
			case 411: return QString("Length Required");
			case 412: return QString("Precondition Failed");
			case 413: return QString("Request Entity Too Large");
			case 414: return QString("Request-URI Too Long");
			case 415: return QString("Unsupported Media Type");
			case 416: return QString("Requested Range Not Satisfiable");
			case 417: return QString("Expectation Failed");
			case 421: return QString("Too Many Connections");
			case 422: return QString("Unprocessable Entity");
			case 423: return QString("Locked");
			case 424: return QString("Failed Dependency");
			case 425: return QString("Unordered Collection");
			case 426: return QString("Upgrade Required");
			case 449: return QString("Retry With");
			case 451: return QString("Unavailable For Legal Reasons");
			case 500: return QString("Internal Error");
			case 501: return QString("Not Implemented");
			case 502: return QString("Bad Gateway");
			case 503: return QString("Service Unavailable");
			case 504: return QString("Gateway Timeout");
			case 505: return QString("HTTP Version Not Supported");
			case 506: return QString("Variant Also Negotiates");
			case 507: return QString("Insufficient Storage");
			case 509: return QString("Bandwidth Limit Exceeded");
			case 510: return QString("Not Extended");
			case 600: return QString("Unparseable Response Headers");
			default:  return QString("Unknown");
		}
	}

	static const QByteArray makeup404Response(const QByteArray &content = QByteArray("404 Not Found")){
		Response response(404);
		response.setContent(content);
		response.setFinished(true);
		return response.toByteArray(NoMoreMakeups);
	}

	static const QByteArray makeup400Response(const QByteArray &content = QByteArray("400 Bad Request")){
		Response response(400);
		response.setContent(content);
		response.setFinished(true);
		return response.toByteArray(NoMoreMakeups);
	}

	static const QByteArray makeup500Response(const QByteArray &content = QByteArray("500 Internal Error")){
		Response response(500);
		response.setContent(content);
		response.setFinished(true);
		return response.toByteArray(NoMoreMakeups);
	}

private:
	QString ctrl = "HTTP";
	Version version;
	int statusCode = 0;
	QString statusReason;

	QHash<QString, QString> header;

	QByteArray content;

	bool valid = false;

	static const QString getTransferEncodingString(TransferEncoding transferEncoding){
		switch(transferEncoding){
			case Normal: return QString("identity");
			case Chunked: return QString("chunked");
			default: return QString();
		}
	}

	static const QString getContentTypeString(ContentType contentType){
		switch(contentType){
			case 0: return QString("*/*");
			case 1: return QString("text/plain");
			case 2: return QString("text/html");
			case 3: return QString("text/css");
			case 4: return QString("text/javascript");
			case 5: return QString("text/json");
			case 100: return QString("image/*");
			case 101: return QString("image/gif");
			case 102: return QString("image/png");
			case 103: return QString("image/jpeg");
			case 104: return QString("image/bmp");
			case 105: return QString("image/webp");
			case 106: return QString("image/svg+xml");
			case 200: return QString("audio/*");
			case 201: return QString("audio/midi");
			case 202: return QString("audio/mpeg");
			case 203: return QString("audio/webm");
			case 204: return QString("audio/ogg");
			case 205: return QString("audio/wav");
			case 206: return QString("audio/mp3");
			case 300: return QString("video/any");
			case 301: return QString("video/webm");
			case 302: return QString("video/ogg");
			case 303: return QString("video/mp4");
			case 9000: return QString("multipart/*");
			case 9001: return QString("multipart/from-data");
			case 9002: return QString("multipart/byteranges");
			case 10000: return QString("application/*");
			case 10001: return QString("application/octet-stream");
			case 10002: return QString("application/pkcs12");
			case 10003: return QString("application/vnd.mspowerpoint");
			case 10004: return QString("application/xhtml+xml");
			case 10005: return QString("application/xml");
			case 10006: return QString("application/pdf");
			case 10007: return QString("application/json");
			default: return QString();
		}
	}

	static const QString getConnectionString(Connection connection){
		switch(connection){
			case Keep_Alive: return QString("keep-alive");
			case Close: return QString("close");
			default: return QString();
		}
	}

	static const QString getCacheControlString(CacheControl cacheControl){
		switch(cacheControl){
			case 1: return QString("must-revalidate");
			case 2: return QString("no-cache");
			case 4: return QString("no-store");
			case 8: return QString("no-transform");
			case 16: return QString("public");
			case 32: return QString("private");
			case 64: return QString("proxy-revalidate");
			case 128: return QString("max-age=");
			case 256: return QString("s-maxage=");
			default: return QString();
		}
	}

	const QString headerString(const QByteArray &crlf = QByteArray("\x0D\x0A")){
		QStringList headers;
		if(this->header.isEmpty()){
			return QString();
		}
		QStringList keys = this->header.keys();
		foreach(const QString &key, keys){
			headers.append(QString("%1: %2").arg(key).arg(header.value(key)));
		}
		return headers.join(crlf);
	}

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
public:
	Response(int statusCode = 200){
		this->setStatusCode(statusCode);
		this->setServer("Qt", QT_VERSION_STR);
		this->setDate(QDateTime::currentDateTimeUtc());
		this->setContentType(Text_Html, "utf-8");
		this->setConnection(Close);
		this->setXPoweredBy("RuilxHttpServer", "1.0");
		this->setCacheControl((Response::CacheControl)(No_Cache | No_Store));
		this->setPragma();
	}
	inline void setFinished(bool valid){
		this->valid = valid;
	}

	bool isValid(){
		return (this->statusCode ? true : false) && (!this->ctrl.isEmpty()) && (this->version.majorVersion + this->version.minorVersion) && this->valid;
	}

	inline void setCtrl(const QString &ctrl){ this->ctrl = ctrl; }
	inline void setVersion(const Version &version){ this->version = version; }
	const QString getCtrlAndVersionString() const{
		return QString(this->ctrl).append("/%1.%2").arg(this->version.majorVersion).arg(this->version.minorVersion);
	}

	inline void setStatusCode(int statusCode){
		this->statusCode = statusCode;
		this->statusReason = this->getStatusCodeReason(statusCode);
	}

	inline int getStatusCode(){
		return this->statusCode;
	}

	void setTransferEncodingEnable(bool enable){
		if(enable){
			this->header.remove("Content-Length");
			this->header.insert("Transfer-Encoding", this->getTransferEncodingString(Chunked));
		}else{
			this->header.remove("Transfer-Encoding");
			this->header.insert("Content-Length", QString::number(this->content.length()));
		}
	}

	void setContentType(ContentType contentType, const QString &charset = QString()){
		if(charset.isEmpty()){
			this->header.insert("Content-Type", this->getContentTypeString(contentType));
		}else{
			this->header.insert("Content-Type", QString(this->getContentTypeString(contentType)).append(";").append("charset=").append(charset));
		}
	}

	void setRawHeader(const QString &key, const QString &value){
		this->header.insert(this->regenerateKey(key), value);
	}

	inline void setHeader(const QString &key, const QString &value){
		this->setRawHeader(key, value);
	}

	void setCookie(const QNetworkCookie &cookie){
		this->header.insert("Set-Cookie", cookie.toRawForm());
	}

	void setServer(const QString &serverName, const QString &version = QString()){
		if(version.isEmpty()){
			this->header.insert("Server", serverName);
		}else{
			this->header.insert("Server", QString(serverName).append("/").append(version));
		}
	}

	void setDate(const QDateTime &datetime){
		this->header.insert("Date", Response::formatDate(datetime));
	}

	void setExpires(const QDateTime &datetime){
		this->header.insert("Expires", Response::formatDate(datetime));
	}

	void setAcceptLanguage(const QLocale &locale = QLocale(), float q = -1.0f){
		QString name = QString(locale.name()).replace('_', '-');
		if(q <= 0.0f || q > 1.0f){
			this->header.insert("Accept-Language", name);
		}else{
			this->header.insert("Accept-Language", name.append(";q=%1").arg(q, 0, 'f', 1));
		}
	}

	void setAcceptLanguage(const QString &locale = QString(), float q = -1.0f){
		QString t;
		if(locale.isEmpty()){
			t = "*";
		}else{
			t = locale;
		}
		if(q > 0.0f && q <= 1.0f){
			t.append(";q=%1").arg(q, 0, 'f', 1);
		}
		this->header.insert("Accept-Language", t);
	}

	void setAcceptLanguage(const QHash<QString, float> &locale = QHash<QString, float>()){
		if(locale.isEmpty()){
			this->header.insert("Accept-Language", "*");
			return;
		}
		QStringList t = locale.keys();
		for(QString &tp: t){
			float q = locale.value(tp);
			if(q > 0.0f && q <= 1.0f){
				tp.append(";q=%1").arg(q, 0, 'f', 1);
			}
		}
		this->header.insert("Accept-Language", t.join(", "));
	}

	void setConnection(Connection connection){
		this->header.insert("Connection", this->getConnectionString(connection));
	}

	void setCacheControl(CacheControls cacheControls, const QHash<CacheControl, int> &secondsHash = QHash<CacheControl, int>()){
		QStringList cacheControlsStr;
		if(cacheControls & Must_Revalidate){
			cacheControlsStr.append(this->getCacheControlString(Must_Revalidate));
		}
		if(cacheControls & No_Cache){
			cacheControlsStr.append(this->getCacheControlString(No_Cache));
		}
		if(cacheControls & No_Store){
			cacheControlsStr.append(this->getCacheControlString(No_Store));
		}
		if(cacheControls & No_Transform){
			cacheControlsStr.append(this->getCacheControlString(No_Transform));
		}
		if(cacheControls & Public){
			cacheControlsStr.append(this->getCacheControlString(Public));
		}
		if(cacheControls & Private){
			cacheControlsStr.append(this->getCacheControlString(Private));
		}
		if(cacheControls & Proxy_Revalidate){
			cacheControlsStr.append(this->getCacheControlString(Proxy_Revalidate));
		}
		if(cacheControls & Max_Age){
			if(secondsHash.contains(Max_Age)){
				cacheControlsStr.append(QString(this->getCacheControlString(Max_Age)).append(QString::number(secondsHash.value(Max_Age))));
			}
		}
		if(cacheControls & S_Maxage){
			if(secondsHash.contains(S_Maxage)){
				cacheControlsStr.append(QString(this->getCacheControlString(S_Maxage)).append(QString::number(secondsHash.value(S_Maxage))));
			}
		}

		this->header.insert("Cache-Control", cacheControlsStr.join(", "));
	}

	void setLocation(const QString &location){
		this->header.insert("Location", location);
	}

	void setPragma(const QString &noCache = QString("no-cache")){
		this->header.insert("Pragma", noCache);
	}

	void setXPoweredBy(const QString &xPowerBy, const QString &version = QString()){
		if(version.isEmpty()){
			this->header.insert("X-Powered-By", xPowerBy);
		}else{
			this->header.insert("X-Powered-By", QString(xPowerBy).append("/").append(version));
		}
	}

	QHash<QString, QString> getHeader(){
		return this->header;
	}

	void setContent(const QByteArray &content){
		this->content = content;
		if(this->header.value("Transfer-Encoding") != this->getTransferEncodingString(Chunked)){
			this->header.insert("Content-Length", QString::number(this->content.length()));
		}
	}

	void appendContent(const QByteArray &content){
		this->content.append(content);
		if(this->header.value("Transfer-Encoding") != this->getTransferEncodingString(Chunked)){
			this->header.insert("Content-Length", QString::number(this->content.length()));
		}
	}

	const QByteArray getContent(){
		return this->content;
	}

	const QByteArray toByteArray(MakeupFlag flag = Strict){
		QByteArray responseFrame = this->headerToByteArray(flag);
		if(flag != Strict){
			return responseFrame;
		}
		if(!this->content.isEmpty()){
			return responseFrame.append(this->content).append("\r\n");
		}else{
			return responseFrame;
		}
	}

	const QByteArray headerToByteArray(MakeupFlag flag = Strict){
		if(!this->isValid() && flag == Strict){
			return makeup500Response();
		}else if(!this->isValid() && flag == NoMoreMakeups){
			qDebug() << "[HS:P]:" << __func__ << ":response is invalid and Offical 404 Response called recursively!";
			return QByteArray();
		}

		if(!this->header.contains("Content-Length") && !this->content.isEmpty()){
			this->header.insert("Content-Length", QString::number(this->content.length()));
		}

		QString responseFrame = QString("%1 %2 %3\r\n%4\r\n\r\n")
				.arg(this->getCtrlAndVersionString())
				.arg(this->statusCode)
				.arg(this->statusReason.isEmpty() ? this->getStatusCodeReason(this->statusCode) : this->statusReason)
				.arg(this->headerString());

		return responseFrame.toUtf8();
	}
};

#endif // RESPONSE_H
