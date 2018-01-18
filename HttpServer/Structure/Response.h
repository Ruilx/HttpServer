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
		Private = 31,
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

private:
	QString ctrl = "HTTP";
	Version version;
	int statusCode = 0;
	QString statusReason;

	QHash<QString, QString> header;

	QByteArray content;

	bool valid = false;

	QString getTransferEncodingString(TransferEncoding transferEncoding){
		switch(transferEncoding){
			case Normal: return QString("identity");
			case Chunked: return QString("chunked");
			default: return QString();
		}
	}

	QString getContentTypeString(ContentType contentType){
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

	QString getConnectionString(Connection connection){
		switch(connection){
			case Keep_Alive: return QString("keep-alive");
			case Close: return QString("close");
			default: return QString();
		}
	}

	QString getCacheControlString(CacheControl cacheControl){
		switch(cacheControl){
			case 1: return QString("must-revalidate");
			case 2: return QString("no-cache");
			case 4: return QString("no-store");
			case 8: return QString("no-transform");
			case 16: return QString("public");
			case 31: return QString("private");
			case 64: return QString("proxy-revalidate");
			case 128: return QString("max-age=");
			case 256: return QString("s-maxage=");
			default: return QString();
		}
	}
public:
	Response(){
		this->setServer("Qt", QT_VERSION_STR);
		this->setDate(QDateTime::currentDateTimeUtc());
		this->setContentType(Text_Html, "utf-8");
		this->setConnection(Close);
		this->setXPoweredBy("RuilxHttpServer", "1.0");
		this->setCacheControl((Response::CacheControl)(No_Cache | No_Store));
		this->setPragma();
	}

	bool isValid(){
		return (this->statusCode ? true : false) && this->valid;
	}

	inline void setCtrl(const QString &ctrl){ this->ctrl = ctrl; }
	inline void setVersion(const Version &version){ this->version = version; }

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
			this->header.insert("Content-Type", this->getContentTypeString(contentType).append(";").append("charset=").append(charset));
		}
	}

	void setRawHeader(const QString &key, const QString &value){
		this->header.insert(key, value);
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
		if(cacheControls | Must_Revalidate){
			cacheControlsStr.append(this->getCacheControlString(Must_Revalidate));
		}
		if(cacheControls | No_Cache){
			cacheControlsStr.append(this->getCacheControlString(No_Cache));
		}
		if(cacheControls | No_Store){
			cacheControlsStr.append(this->getCacheControlString(No_Store));
		}
		if(cacheControls | No_Transform){
			cacheControlsStr.append(this->getCacheControlString(No_Transform));
		}
		if(cacheControls | Public){
			cacheControlsStr.append(this->getCacheControlString(Public));
		}
		if(cacheControls | Private){
			cacheControlsStr.append(this->getCacheControlString(Private));
		}
		if(cacheControls | Proxy_Revalidate){
			cacheControlsStr.append(this->getCacheControlString(Proxy_Revalidate));
		}
		if(cacheControls | Max_Age){
			if(secondsHash.contains(Max_Age)){
				cacheControlsStr.append(this->getCacheControlString(Max_Age).append(QString::number(secondsHash.value(Max_Age))));
			}
		}
		if(cacheControls | S_Maxage){
			if(secondsHash.contains(S_Maxage)){
				cacheControlsStr.append(this->getCacheControlString(S_Maxage).append(QString::number(secondsHash.value(S_Maxage))));
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


};

#endif // RESPONSE_H
