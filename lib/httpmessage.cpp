/* 
 * File:   httprequest.cpp
 * Author: Christian Surlykke <christian@surlykke.dk>
 * 
 * Created on 15. marts 2015, 14:36
 */

#include <string.h>
#include <unistd.h>
#include <iosfwd>

#include "httpmessage.h"
#include "errorhandling.h"

using namespace std;

namespace org_restfulipc
{
	HttpMessage::HttpMessage()
	{
	}


	HttpMessage::~HttpMessage()
	{
	}

	void HttpMessage::clear()
	{
		method = Method::UNKNOWN;
		path = 0;
		queryString = 0;
		for (int i = 0; i < (int) Header::unknown; i++)
			headers[i] = 0;
		body = 0;
		contentLength = 0;
	}

	HttpMessageReader::HttpMessageReader(int socket, HttpMessage& message) : 
		_socket(socket), 
		_message(message),
		_bufferEnd(0),
		_currentPos(-1)
	{
	}

	void HttpMessageReader::readRequest()
	{
		clear();
		readRequestLine();
		readHeaders();
		if (_message.headers[(int) Header::content_length]) {
			readBody();
		}
	}

	void HttpMessageReader::readResponse()
	{
		clear();
		readStatusLine();
		readHeaders();
		if (_message.status >= 200 && 
		    _message.status != 204 && 
		    _message.status != 304) {
			if (_message.headers[(int) Header::content_length]) {
				readBody();
			}
		}
	}



	void HttpMessageReader::readRequestLine()
	{
		while (nextChar() != ' ');
		
		_message.method = string2Method(_message.buffer);
        throwHttpStatusUnless(Status::Http406, _message.method != Method::UNKNOWN);
		_message.path = _message.buffer + _currentPos + 1;

		while (! isspace(nextChar()))
		{
			if (_message.buffer[_currentPos] == '?')
			{
				_message.queryString = _message.buffer + _currentPos + 1;
				_message.buffer[_currentPos] = '\0';
			}
		};

		_message.buffer[_currentPos] = '\0';

		if (_message.queryString == 0)
		{
			_message.queryString = _message.buffer + _currentPos;
		}

		int protocolStart = _currentPos + 1;

		while (! isspace(nextChar()));
		
        throwHttpStatusUnless(Status::Http400, _message.buffer[_currentPos] == '\r' && nextChar() == '\n');
	}

	void HttpMessageReader::readStatusLine()
	{
		while (!isspace(nextChar()));
        throwHttpStatusUnless(Status::Http400, _currentPos > 0);
        throwHttpStatusUnless(Status::Http400, strncmp("HTTP/1.1", _message.buffer, 8) == 0);
		while (isspace(nextChar()));
		int statuscodeStart = _currentPos;
        throwHttpStatusUnless(Status::Http400, isdigit(currentChar()));
		while (isdigit(nextChar()));
		errno = 0;
		long int status = strtol(_message.buffer + statuscodeStart, 0, 10);
        throwHttpStatusUnless(Status::Http400, status > 100 && status < 600);
		_message.status = (int) status;

		// We ignore what follows the status code. This means that a message like
		// 'HTTP/1.1 200 Completely f**cked up' will be interpreted as 
		// 'HTTP/1.1 200 Ok'
		// (Why does the http protocol specify that both the code and the text is sent?)
		while (currentChar() != '\r')
		{
            throwHttpStatusUnless(Status::Http400, currentChar() != '\n');
			nextChar();
		}

        throwHttpStatusUnless(Status::Http400, nextChar() == '\n');

	}

	// On entry: currentPos points to character just before next header line
	void HttpMessageReader::readHeaders() 
	{
		while (true) 
		{
			if (nextChar() == '\r')	
			{
                throwHttpStatusUnless(Status::Http400, nextChar() == '\n');
				return;
			}
			
			readHeaderLine();
		}
	}

	/* On entry - _currentPos points to first character of line
	 * 
	 * TODO: Full implementation of spec
	 *  - multiline header definitions
	 *  - Illegal chars in names/values
	 */	
	bool HttpMessageReader::readHeaderLine()
	{
		int startOfHeaderLine = _currentPos;
		int startOfHeaderValue = -1;
		int endOfHeaderValue = -1;

		while (isTChar(currentChar())) nextChar();
        throwHttpStatusUnless(Status::Http400, currentChar() == ':');
        throwHttpStatusUnless(Status::Http400, _currentPos > startOfHeaderLine);
		_message.buffer[_currentPos] = '\0';

		while (isblank(nextChar()));
		endOfHeaderValue = startOfHeaderValue = _currentPos;

		while (currentChar() != '\r')
		{
			if (!isblank(currentChar()))
			{
				endOfHeaderValue = _currentPos + 1;
			}
			nextChar();
		}

        throwHttpStatusUnless(Status::Http400, nextChar() == '\n');
		_message.buffer[endOfHeaderValue] = '\0';
		Header h = string2Header(_message.buffer + startOfHeaderLine);

		if (h != Header::unknown)
		{
			_message.headers[(int) h] = _message.buffer + startOfHeaderValue;
		}

	}

	bool HttpMessageReader::isTChar(char c)
	{
		return c != ':'; // FIXME
	}

	void HttpMessageReader::readBody()
	{

		errno = 0;
		_message.contentLength = strtoul(_message.headerValue(Header::content_length), 0, 10);
		throwErrnoUnless(errno == 0);

		int bodyStart = _currentPos + 1;	
		while (bodyStart + _message.contentLength > _bufferEnd)	
		{
			receive();
		}
		
		_message.buffer[bodyStart + _message.contentLength] = '\0';
		_message.body = _message.buffer + bodyStart;
	}

	char HttpMessageReader::currentChar()
	{
		return _message.buffer[_currentPos];
	}


	char HttpMessageReader::nextChar()
	{
		_currentPos++;	

		while ( _currentPos >= _bufferEnd)
		{
			receive();
		}

		
		return _message.buffer[_currentPos];
	}

	void HttpMessageReader::receive()
	{
		int bytesRead = read(_socket, _message.buffer + _bufferEnd, 8190 - _bufferEnd);
		if (bytesRead > 0) {
			_message.buffer[_bufferEnd + bytesRead] = '\0';
			_bufferEnd += bytesRead;
		}
		else 
		{
			throw errno;
		}
		
	}

	void HttpMessageReader::clear()
	{
		_message.clear();	
		
		_bufferEnd = 0;
		_currentPos = -1;	
	}



	

}
