#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QtCore>
#include "Structure/Request.h"
#include "Structure/Response.h"

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

public:

	static Response getDefaultResponse(int statusCode = 200){
		return Response(statusCode);
	}


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

		qDebug() << "[协议]:" << __func__ << ":此次收到了消息:" << content;

		int index = 0; // 光标位置
		// Step 1: Trim Write Spaces
		// First line must, maybe cutted at double /r/n, the /r/n will be need
		if(request->getCtrlAndVersion().isEmpty() || request->getFullUrl().isEmpty() || request->getMethod() == Request::Method_Unknown){
			while(index < content.length() && QChar(content.at(index)).isSpace()){
				index++;
			}
			if(index >= content.length()){
				return true;
			}
		}

		// Step 2: First line
		// If firstline part has writted, skipped.
		if(request->getCtrlAndVersion().isEmpty() || request->getFullUrl().isEmpty() || request->getMethod() == Request::Method_Unknown){
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

};

#endif // PROTOCOL_H
