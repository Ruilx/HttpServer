#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QtCore>
#include "Structure/Request.h"
#include "Structure/Response.h"
/*
typedef struct Request_t{
	QString method;	//GET POST
	QString url;	// /
	QString ctrl;	// HTTP/1.1
	QHash<QString, QString> header; // content-type:xxx
	QByteArray content;		// Content
	bool valid = false;
	bool isValid(){
		return (!method.isEmpty() && !url.isEmpty() && !ctrl.isEmpty() && valid);
	}
	Request_t(){ valid = false; }
}Request;

typedef struct Response_t{
	QString ctrl; // HTTP/1.1
	int statusCode; // 200
	QString statusReason; // OK
	QHash<QString, QString> header; // Server: nginx/1.10.2
	QByteArray content;
	bool valid = false;
	bool isValid() const{
		return ((statusCode ? true : false) && valid);
	}
	Response_t(){ valid = false; statusCode = 0; }
}Response;

typedef struct{
	QString url;
	QHash<QString, QString> query;
}RequestQuery;
*/
class Protocol{

public:
	enum MakeupFlag{
		Normal = 0,
		NoMoreMakeups = 1,
	};
private:
	static inline int getCrlfLength(){
		return QByteArray("\x0D\x0A").length();
	}

	static inline int getNextCrlf(const QByteArray &content, int currentIndex){
		const QByteArray splitter("\x0D\x0A");
		return content.indexOf(splitter, currentIndex);
	}

//	static QString regenerateKey(QString str){
//		QStringList keySep = str.split("-", QString::KeepEmptyParts);
//		QStringList newKeySep;
//		foreach(QString key, keySep){
//			if(key.isEmpty()){
//				continue;
//			}
//			QChar firstLetter = key.at(0).toUpper();
//			QString restLetters = key.mid(1).toLower();
//			newKeySep.append(QString(firstLetter).append(restLetters));
//		}
//		return newKeySep.join("-");
//	}

//	static const QString hashHeader2String(const QHash<QString, QString> &header, const QByteArray &crlf = QByteArray("\x0D\x0A")){
//		QStringList headers;
//		QStringList keys = header.keys();
//		foreach(const QString &key, keys){
//			headers.append(QString("%1: %2").arg(key).arg(header.value(key)));
//		}
//		if(headers.isEmpty()){
//			return QString();
//		}else{
//			return headers.join(crlf);
//		}
//	}

//	static QString formatDate(const QDateTime &date){
//		const QStringList week = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
//		const QStringList month = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//		const QDate d = date.date();
//		const QTime t = date.time();
//		return QString("%1, %2 %3 %4 %5:%6:%7 GMT")
//				.arg(week.at(d.dayOfWeek()))
//				.arg(int(d.day()), 2, 10, QChar('0'))
//				.arg(month.at(d.month()))
//				.arg(int(d.year()), 4, 10, QChar('0'))
//				.arg(int(t.hour()), 2, 10, QChar('0'))
//				.arg(int(t.minute()), 2, 10, QChar('0'))
//				.arg(int(t.second()), 2, 10, QChar('0'));
//	}

public:
//	static QHash<QString, QString> getDefaultResponseHeader(){
//		QHash<QString, QString> header;
//		header.insert("Server", "Qt/" QT_VERSION_STR);
//		//header.insert("Date", QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy HH:mm:ss t"));
//		header.insert("Date", Protocol::formatDate(QDateTime::currentDateTimeUtc()));
//		header.insert("Content-Type", "text/html");
//		header.insert("Connection", "close");
//		header.insert("X-Powered-By", "RuilxHttpServer/1.0");
//		return header;

//		return Response::getDefaultResponseHeader();
//	}

//	static QByteArray makeupA404Response(const QByteArray &content = QByteArray("404 Not Found")){
//		Response response;
//		response.ctrl = QString("HTTP/1.1");
//		response.statusCode = 404;
//		response.statusReason = Protocol::getStatusCodeReason(response.statusCode);
//		response.header = Protocol::getDefaultResponseHeader();
//		response.header.insert("Content-Length", QString::number(content.length()));
//		response.content = content;
//		response.valid = true;
//		return Protocol::makeupResponse(response, NoMoreMakeups);
//	}

//	static QByteArray makeupA400Response(const QByteArray &content = QByteArray("400 Bad Request")){
//		Response response;
//		response.ctrl = QString("HTTP/1.1");
//		response.statusCode = 400;
//		response.statusReason = Protocol::getStatusCodeReason(response.statusCode);
//		response.header = Protocol::getDefaultResponseHeader();
//		response.header.insert("Content-Length", QString::number(content.length()));
//		response.content = content;
//		response.valid = true;
//		return Protocol::makeupResponse(response, NoMoreMakeups);
//	}

//	static QByteArray makeupA500Response(const QByteArray &content = QByteArray("500 Internal Error")){
//		Response response;
//		response.ctrl = QString("HTTP/1.1");
//		response.statusCode = 500;
//		response.statusReason = Protocol::getStatusCodeReason(response.statusCode);
//		response.header = Protocol::getDefaultResponseHeader();
//		response.header.insert("Content-Length", QString::number(content.length()));
//		response.content = content;
//		response.valid = true;
//		return Protocol::makeupResponse(response, NoMoreMakeups);
//	}

	static Response getDefaultResponse(int statusCode = 200){
		return Response(statusCode);
	}

//	static QString getStatusCodeReason(int statusCode){
//		switch(statusCode){
//			case 100: return QString("Continue");
//			case 101: return QString("Switching Protocols");
//			case 102: return QString("Processing");
//			case 200: return QString("OK");
//			case 201: return QString("Created");
//			case 202: return QString("Accepted");
//			case 203: return QString("Partial Information");
//			case 204: return QString("No Response");
//			case 205: return QString("Resetted");
//			case 206: return QString("Parted");
//			case 207: return QString("Multi Status");
//			case 300: return QString("Multiple Choices");
//			case 301: return QString("Moved Permanently");
//			case 302: return QString("Moved Temporarily");
//			case 303: return QString("See Other");
//			case 304: return QString("Not Modified");
//			case 305: return QString("Use Proxy");
//			case 306: return QString("Switch Proxy");
//			case 307: return QString("Temporary Redirect");
//			case 400: return QString("Bad Request");
//			case 401: return QString("Unauthorized");
//			case 402: return QString("Payment Required");
//			case 403: return QString("Forbidden");
//			case 404: return QString("Not Found");
//			case 405: return QString("Method Not Allowed");
//			case 406: return QString("Not Acceptable");
//			case 407: return QString("Proxy Authentication Required");
//			case 408: return QString("Request Timeout");
//			case 409: return QString("Conflict");
//			case 410: return QString("Gone");
//			case 411: return QString("Length Required");
//			case 412: return QString("Precondition Failed");
//			case 413: return QString("Request Entity Too Large");
//			case 414: return QString("Request-URI Too Long");
//			case 415: return QString("Unsupported Media Type");
//			case 416: return QString("Requested Range Not Satisfiable");
//			case 417: return QString("Expectation Failed");
//			case 421: return QString("Too Many Connections");
//			case 422: return QString("Unprocessable Entity");
//			case 423: return QString("Locked");
//			case 424: return QString("Failed Dependency");
//			case 425: return QString("Unordered Collection");
//			case 426: return QString("Upgrade Required");
//			case 449: return QString("Retry With");
//			case 451: return QString("Unavailable For Legal Reasons");
//			case 500: return QString("Internal Error");
//			case 501: return QString("Not Implemented");
//			case 502: return QString("Bad Gateway");
//			case 503: return QString("Service Unavailable");
//			case 504: return QString("Gateway Timeout");
//			case 505: return QString("HTTP Version Not Supported");
//			case 506: return QString("Variant Also Negotiates");
//			case 507: return QString("Insufficient Storage");
//			case 509: return QString("Bandwidth Limit Exceeded");
//			case 510: return QString("Not Extended");
//			case 600: return QString("Unparseable Response Headers");
//			default:  return QString("Unknown");
//		}
//	}

//	static const Request analysisRequest(const QByteArray &content){
//		/**
//		GET / HTTP/1.1
//		Accept: * / *
//		Host: xx.xxx.com
//		User-Agent: Mozilla/5.0 ...
//		*/

//		Request rh;
//		QHash<QString, QString> header;

//		int index = 0;
//		// Remove first spaces
//		while(index < content.length() && QChar(content.at(index)).isSpace()){
//			index++;
//		}
//		if(index >= content.length()){
//			return Request();
//		}

//		// For the first line.
//		int firstlineEndIndex = Protocol::getNextCrlf(content, index);
//		if(firstlineEndIndex < 0){
//			qDebug() << "[HS:P]:" << __func__ << "firstlineSep '\\r\\n' not found.";
//			return Request();
//		}
//		QString firstline = QString(content.mid(index, firstlineEndIndex - index)).trimmed();
//		QRegExp firstlineRx("([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)");
//		int pos = firstlineRx.indexIn(firstline);
//		if(pos == -1){
//			qDebug() << "[HS:P]:" << __func__ << "firstlinePartern '\\s' not found.";
//			return Request();
//		}
//		rh.method = firstlineRx.cap(1); // GET
//		rh.url = firstlineRx.cap(2); // /
//		rh.ctrl = firstlineRx.cap(3); // HTTP/1.1
//		index = firstlineEndIndex + Protocol::getCrlfLength();
//		// For the following lines
//		int headerLineEndIndex = Protocol::getNextCrlf(content, index);
//		if(headerLineEndIndex < 0){
//			qDebug() << "[HS:P]:" << __func__ << "headerlineSep '\\r\\n' not found.";
//			return Request();
//		}
//		while(headerLineEndIndex > 0){
//			if(index == headerLineEndIndex){
//				// EmptyLine, means end of the header
//				index += Protocol::getCrlfLength();
//				//headerLineEndIndex = Protocol::getNextCrlf(content, index); // Useless headerLineEndIndex for the next time.
//				break;
//			}
//			QString headerLine = QString(content.mid(index, headerLineEndIndex - index)).trimmed();
//			if(headerLine.isEmpty()){
//				// EmptyLine, means above.
//				index = headerLineEndIndex + Protocol::getCrlfLength(); // maybe headerLineEndIndex == index
//				//headerLineEndIndex = Protocol::getNextCrlf(content, index); // Useless headerLineEndIndex for the next time.
//				break;
//			}else{
//				// Header line is not empty like "Accept: text/html"
//				/* Header analysis formula:
//				 * DETECTED      | HASH
//				 * 'Accept'      | => Hash('Accept': '')
//				 * ': text/html' | => Abandoned.
//				 * 'Acce pt: xxx'| => Abandoned.
//				 * 'Accept::xxx' | => Hash('Accept': ':xxx')
//				 */
//				int firstColonIndex = headerLine.indexOf(QChar(':'));
//				if(firstColonIndex < 0){
//					// Detected: 'Accept'
//					header.insert(Protocol::regenerateKey(headerLine), QString());
//					index = headerLineEndIndex + Protocol::getCrlfLength();
//					headerLineEndIndex = Protocol::getNextCrlf(content, index);
//					continue;
//				}
//				QString key = headerLine.left(firstColonIndex).trimmed();
//				QString value = headerLine.mid(firstColonIndex +1).trimmed();
//				if(key.isEmpty() || key.contains(QRegExp("\\s"))){
//					// Detected: ': xxx' or 'Acce\spt: xxx'
//					index = headerLineEndIndex + Protocol::getCrlfLength();
//					headerLineEndIndex = Protocol::getNextCrlf(content, index);
//					continue;
//				}
//				if(key.contains(QRegExp("[^A-Za-z0-9_-]"))){
//					// Detected: Special Characters
//					index = headerLineEndIndex + Protocol::getCrlfLength();
//					headerLineEndIndex = Protocol::getNextCrlf(content, index);
//					continue;
//				}
//				header.insert(Protocol::regenerateKey(key), value);
//				index = headerLineEndIndex + Protocol::getCrlfLength();
//				headerLineEndIndex = Protocol::getNextCrlf(content, index);
//				continue;
//			}
//		}
//		// For the body.
//		int contentLength = header.value("Content-Length", QString("0")).toInt();
//		// rh.content = content.mid(index).left(contentLength);
//		QByteArray requestContent = content.mid(index);
//		if(requestContent.length() < contentLength){
//			rh.content = requestContent;
//			rh.valid = false; // hang on for continuing data
//		}else{
//			rh.content = requestContent.left(contentLength);
//			if(!content.endsWith("\r\n")){
//				// if it is not end with /r/n means this is particularly not endding.
//				rh.valid = false;
//			}else{
//				rh.valid = true; // content valid
//			}
//		}
//		rh.header = header;

//		return rh;
//	}

	static bool analysisRequest(const QByteArray &content, Request *request){
		// First, trim the write space from head
		// Second, analysis first line information
		// Third, analysis headers until empty line occurred
		// Fourth, if not attach crlf but EOF, waiting next request parts.
		// Fifth, refer to Content-Length to waiting the content transfer finished.

		// return: TRUE: if the request's valid == true this request can be use for job, otherwise, waiting for new data receive.
		//         FALSE: Bad Request, Invalid arguments Or request already invalid.

		if(request == nullptr || request->isValid()){
			// request must be a new Request() and valid must be false.
			qDebug() << "[协议]: 所给request的valid已被置为true.";
			return false;
		}

		int index = 0; // 光标位置
		// Step 1: Trim Write Spaces
		// First line must, maybe cutted at double /r/n, the /r/n will be need
		if(request->getCtrlAndVersion().isEmpty() || request->getFullUrl().isEmpty() || request->getMethod() != Request::Method_Unknown){
			while(index < content.length() && QChar(content.at(index)).isSpace()){
				index++;
			}
			if(index >= content.length()){
				return true;
			}
		}

		// Step 2: First line
		// If firstline part has writted, skipped.
		if(request->getCtrlAndVersion().isEmpty() || request->getFullUrl().isEmpty() || request->getMethod() != Request::Method_Unknown){
			int firstLineEndIndex = Protocol::getNextCrlf(content, index);
			if(firstLineEndIndex < 0){
				qDebug() << "[协议]: 发送内容不足一行(协议规定至少一行)";
				return false;
			}
			QString firstLine = QString(content.mid(index, firstLineEndIndex - index)).trimmed();
			QRegExp firstLineRx("([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)");
			int pos = firstLineRx.indexIn(firstLine);
			if(pos == -1){
				qDebug() << "[协议]: 协议第一行不符合HTTP/1.1规定.";
				return false;
			}
			request->setMethod(firstLineRx.cap(1));
			request->setUrl(firstLineRx.cap(2));
			request->setCtrl(firstLineRx.cap(3));
			index = firstLineEndIndex + Protocol::getCrlfLength();
			if(index >= content.length()){
				return true;
			}
		}

		// Step 3: analysis headers until empty line occurred

		while(index < content.length()){
			int headerLineEndIndex = Protocol::getNextCrlf(content, index);
			if(headerLineEndIndex < 0){
				// no such crlf found
				// 1: transfer at header
				// 2: transfer at body
				if(!request->_headerComplete){
					// Header not complete, so trade it as header.
					if(request->_halfLine.isEmpty()){
						// means here is the first of a header;
						QString halfLine = QString(content.mid(index));
						request->_halfLine = halfLine;
						return true;
					}else{
						// means here already have an halfLine
						QString halfLine = request->_halfLine;
						halfLine.append(content.mid(index));
						request->_halfLine = halfLine;
						return true;
					}
				}else{
					// Header completed, trade it as body
					int contentLength = request->getHeader("Content-Length", "0").toInt();
					QByteArray currentContent = content.mid(index);
					if(contentLength > request->getContentLength() + currentContent.length()){
						request->appendContent(currentContent);
						return true;
					}else{
						request->appendContent(currentContent.left(contentLength - request->getContentLength()));
						request->_halfLine.clear();
						request->setFinished(true);
						return true;
					}
				}
			}else{
				// found a next crlf
				// 1: whole line header
				// 2: half line header
				// 3: in body text
				if(!request->_headerComplete){
					// Header not complete, so trade it as header.
					if(request->_halfLine.isEmpty()){
						// means here is the first of header
						if(index == headerLineEndIndex){
							// EMPTY LINE
							request->_headerComplete = true;
							index += Protocol::getCrlfLength();
							int contentLength = request->getHeader("Content-Length", "0").toInt();
							if(contentLength > 0){
								// Header says there's still have bodys and it's length, waiting for body data
								continue;
							}else{
								// Header not mentioned content length, so deal with 0 and returnd valid request.
								request->_halfLine.clear();
								request->setFinished(true);
								return true;
							}
						}else{
							// not empty
							QString line = QString(content.mid(index, headerLineEndIndex - index)).trimmed();
							// ANALYSIS HEADER LINE ==========================
							int firstColonIndex = line.indexOf(QChar(':'));
							if(firstColonIndex < 0){
								// Detect a no colon header: like 'Accept'
								// Note: the whole 'line' shows the [key]
								request->insertHeader(line, QString());
								index = headerLineEndIndex + Protocol::getCrlfLength();
								continue;
							}
							QString key = line.left(firstColonIndex).trimmed();
							QString value = line.mid(firstColonIndex +1).trimmed();
							if(key.isEmpty() || key.contains(QRegExp("\\s"))){
								// Detected: ": xxx" or "Acce\spt: xxx"
								index = headerLineEndIndex + Protocol::getCrlfLength();
								continue;
							}
							if(key.contains(QRegExp("[^A-Za-z0-9_-]"))){
								// Detected: Special Characters
								index = headerLineEndIndex + Protocol::getCrlfLength();
								continue;
							}
							request->insertHeader(key, value);
							index = headerLineEndIndex + Protocol::getCrlfLength();
							continue;
							// ========================== ANALYSIS HEADER LINE
						}
					}else{
						// means here already have an halfLine
						QString line;
						if(index == headerLineEndIndex){
							// EMPTY LINE means halfline is a request header line
							line = request->_halfLine;
						}else{
							// Not an empty line means halfline & content makes a request header line.
							line = QString(content.mid(index, headerLineEndIndex - index)).trimmed().prepend(request->_halfLine);
						}
						// ANALYSIS HEADER LINE ==========================
						int firstColonIndex = line.indexOf(QChar(':'));
						if(firstColonIndex < 0){
							// Detect a no colon header: like 'Accept'
							// Note: the whole 'line' shows the [key]
							if(line.contains(QRegExp("[^A-Za-z0-9_-]"))){
								index = headerLineEndIndex + Protocol::getCrlfLength();
								continue;
							}
							request->insertHeader(line, QString());
							index = headerLineEndIndex + Protocol::getCrlfLength();
							continue;
						}
						QString key = line.left(firstColonIndex).trimmed();
						QString value = line.mid(firstColonIndex +1).trimmed();
						if(key.isEmpty() || key.contains(QRegExp("\\s"))){
							// Detected: ": xxx" or "Acce\spt: xxx"
							index = headerLineEndIndex + Protocol::getCrlfLength();
							continue;
						}
						if(key.contains(QRegExp("[^A-Za-z0-9_-]"))){
							// Detected: Special Characters
							index = headerLineEndIndex + Protocol::getCrlfLength();
							continue;
						}
						request->insertHeader(key, value);
						index = headerLineEndIndex + Protocol::getCrlfLength();
						continue;
						// ========================== ANALYSIS HEADER LINE
					}
				}else{
					// Means header was completed
					int contentLength = request->getHeader("Content-Length", "0").toInt();
					QByteArray currentContent = content.mid(index);
					if(contentLength > request->getContentLength() + currentContent.length()){
						request->appendContent(currentContent);
						return true;
					}else{
						request->appendContent(currentContent.left(contentLength - request->getContentLength()));
						request->_halfLine.clear();
						request->setFinished(true);
						return true;
					}
				}
			}
		}
		// 获得了一整行消息并且已经处理, 但是消息还没有传递完全, 等待下一次传递.
		return true;
	}

//	static const QByteArray makeupResponse(const Response &res, MakeupFlag flag = Normal){
//		if(!res.isValid() && flag == Normal){
//			return makeupA500Response();
//		}else if(!res.isValid() && flag == NoMoreMakeups){
//			qDebug() << "[HS:P]:" << __func__ << ":response is invalid and makeupA404Response called recursively!";
//			return QByteArray();
//		}

//		QHash<QString, QString> _header;
//		if(!res.header.contains("Content-Length")){
//			_header = QHash<QString, QString>(res.header);
//			_header.insert("Content-Length", QString::number(res.content.length()));
//		}

//		QString responseFrame = QString("%1 %2 %3\r\n%4\r\n\r\n")
//				.arg(res.ctrl)
//				.arg(res.statusCode)
//				.arg(res.statusReason.isEmpty() ? Protocol::getStatusCodeReason(res.statusCode) : res.statusReason)
//				.arg(Protocol::hashHeader2String(_header.isEmpty() ? res.header : _header));

//		return responseFrame.toUtf8().append(res.content).append("\r\n");
//	}

//	static const RequestQuery analysisRequestQuery(const QString &urlStr){
//		RequestQuery requestQuery;
//		QUrl url = QUrl::fromPercentEncoding(urlStr.toUtf8());
//		if(!url.isValid()){
//			return requestQuery;
//		}
//		requestQuery.url = url.path();
//		if(url.query().isEmpty()){
//			return requestQuery;
//		}
//		QStringList queryList = url.query().split("&");
//		foreach(const QString &query, queryList){
//			int valueIndex = query.indexOf("=");
//			if(valueIndex == -1){
//				requestQuery.query.insert(query, QString());
//				continue;
//			}else{
//				requestQuery.query.insert(query.left(valueIndex), query.mid(valueIndex +1));
//				continue;
//			}
//		}
//		return requestQuery;
//	}
};

#endif // PROTOCOL_H
