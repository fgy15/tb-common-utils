#include "tbnet.h"
#include "stdlib.h"

namespace tbnet {

/*
 * 构造函数
 */
HttpMessage::HttpMessage(int pcode) {
	_pcode = pcode; //
	_strHeader = NULL;
    _strQuery = NULL;
    _method = 0;
    _status = true;
    _body = NULL;
    _bodyLen = 0;
    _isKeepAlive = false;
    _statusMessage = NULL;
    //_headerline = NULL; //
    //_isDefault = true; //
    //_boundary = NULL; //
    //_mutipartbody.clear();
}
/*
 * 构造函数
 */
HttpMessage::HttpMessage() {
    _pcode = -1; //
    _strHeader = NULL;
    _strQuery = NULL;
    _method = 0;
    _status = true;
    _body = NULL;
    _bodyLen = 0;
    _isKeepAlive = false;
    _statusMessage = NULL;
    //_headerline = NULL; //
    //_isDefault = true; //
    //_boundary = NULL; //
    //_mutipartbody.clear();
}

/*
 * 析构函数
 */
HttpMessage::~HttpMessage() {
    if (_body) {
        ::free(_body);
    }
    if (_statusMessage) {
        ::free(_statusMessage);
        _statusMessage = NULL;
    }
    if (_strHeader) {
        ::free(_strHeader);
    }
    /*if (_boundary) {
        ::free(_boundary);
    }
    if (_headerline) {
        ::free(_headerline);
    }*/
}

/*
 * 组装
 */
bool HttpMessage::encode(DataBuffer *output) {
    if (isRequest())
    {
        return true;
    }
	/*
    if (_pcode){	//请求包的首行
		output->writeBytes(_headerline, strlen(_headerline));
	    output->writeBytes("\r\n", 2);
	} 
    else 
    {
    */	
    //响应包的首行
	    if (_statusMessage) {
	        output->writeBytes(_statusMessage, strlen(_statusMessage));
	        output->writeBytes("\r\n", 2);
	    } else if (_status) { //HTTP/1.1 200 OK
	        output->writeBytes(TBNET_HTTP_STATUS_OK, strlen(TBNET_HTTP_STATUS_OK));
	    } else { // HTTP/1.1 404 Not Found
	        output->writeBytes(TBNET_HTTP_STATUS_NOTFOUND, strlen(TBNET_HTTP_STATUS_NOTFOUND));
	    }
	//}

    //默认固定字段
    //if (_isDefault)
    //{
    	if (_isKeepAlive) {
        	output->writeBytes(TBNET_HTTP_KEEP_ALIVE, strlen(TBNET_HTTP_KEEP_ALIVE));
    	} else {
        	output->writeBytes(TBNET_HTTP_CONN_CLOSE, strlen(TBNET_HTTP_CONN_CLOSE));
    	}
    	if (_headerMap2.find("Content-Type") == _headerMap2.end()) {
        	output->writeBytes(TBNET_HTTP_CONTENT_TYPE, strlen(TBNET_HTTP_CONTENT_TYPE));
    	}
    	char tmp[64];
    	int len = sprintf(tmp, TBNET_HTTP_CONTENT_LENGTH, _bodyLen);
    	output->writeBytes(tmp, len);
    //}


    // 用户自定义长度
    for (STRING_MAP_ITER it=_headerMap2.begin(); it!=_headerMap2.end(); it++) {
        output->writeBytes(it->first.c_str(), strlen(it->first.c_str()));
        output->writeBytes(": ", 2);
        output->writeBytes(it->second.c_str(), strlen(it->second.c_str()));
        output->writeBytes("\r\n", 2);
    }

    // 空行
    output->writeBytes("\r\n", 2);
    // bodyLen
    if (_body)  //普通body
    {
        output->writeBytes(_body, _bodyLen);
    }
    /*if (!_mutipartbody.empty())       //mutipartbody
    {
        _mutipartbody.append("--" + _boundary + "--\r\n");  //加一个mutipart的结尾分界
        _body=(char *) malloc(strlen(_mutipartbody.c_str()));
        _bodyLen = sprintf(_body, "%s", _mutipartbody);
        output->writeBytes(_body, _bodyLen);
    }*/
    
    //assert(_packetHeader._dataLen == output->getDataLen());

    return true;
}

/*
 * 解开
 */
bool HttpMessage::decode(DataBuffer *input, PacketHeader *header) {
    if (isResponse())
    {
        return true;
    }
    int len = header->_dataLen;
    _strHeader = (char*) malloc(len+1);
    input->readBytes(_strHeader, len);
    _strHeader[len] = '\0';
    int line = 0;
    int bodylen = 0;
    int first = 1;

    char *p, *name = NULL, *value;
    p = value = _strHeader;
    while (*p) {
        // 找每一行
        if (*p == '\r' && *(p+1) == '\n') {
            if (value == p && line > 0) { // header 结束了
                break;
            }
            *p = '\0';
            // 去前空格
            while (*value == ' ') value ++;
            if (line > 0) {
                if (strcmp(name, "Connection") == 0 && strcasecmp(value, "Keep-Alive") == 0) {
                    _isKeepAlive = true;
                } else {
                    _headerMap[name] = value;
                }
            } else {
                _strQuery = value;
            }
            value = p + 2;
            line ++;
            first = 1;
        } else if (line == 0 && *p == ' ') { // 首行
            if (_method) {
                *p = '\0';
            } else if (strncmp(value, "GET ", 4) == 0) {    // 是GET 方法
                _method = 1;
                value = p + 1;
            } else if (strncmp(value, "POST ", 5) == 0){	//POST
            	_method = 2;
            	value = p + 1;
            } else if (strncmp(value, "DELETE ", 7) == 0){
            	_method = 3;
            	value = p + 1;
            } else if (strncmp(value, "PUT ", 4) == 0){
            	_method = 4;
            	value = p + 1;
            }
        } else if (*p == ':' && first == 1) {	//每一行的第一个冒号时
            *p = '\0';
            name = value;
            value = p + 1;
            first = 0;
        }
        p ++;
    }

    //body
    if (findHeader("Content-Length") != NULL)
    {
    	bodylen = atoi(findHeader("Content-Length"));
    	if (bodylen > 0)
    	{
	    	_body = (char*) malloc(bodylen+1);
	    	input->readBytes(_body, bodylen);
	    	_body[bodylen] = '\0';
    	}
    }      

    return true;
}

/*
 * 查询请求url串
 */
char *HttpMessage::getQuery() {
    return _strQuery;
}

/*
 * 查询请求包body
 */
char *HttpMessage::getbody() {
    return _body;
}

/*
 * 查询请求为mutipart时的boundary
 */
/*char* HttpMessage::getboundary(){
    char *_contenttype;
    if (findHeader("Content-Type")!=NULL)
    {
        _contenttype = strdup(findHeader("Content-Type"));
        if (strncmp(_contenttype, "multipart", 9) == 0)        //如果是mutipart类型
        {
            if (_boundary) {
                ::free(_boundary);
                _boundary = NULL;
            }
            _boundary = strdup(_contenttype + 30);          //从第30个开始就是boundary的内容，如multipart/form-data; boundary=xx
        }
    }
    if (_contenttype)
    {
        ::free(_contenttype);
        _contenttype = NULL;
    }
    return _boundary;  
}*/

/*
 * 查询请求方法
 */
int HttpMessage::getmethod() {
	return _method;
}

/*
 * 是否keepalive
 */
bool HttpMessage::isKeepAlive() {
    return _isKeepAlive;
}

/*
 * 寻找其他头信息
 */
const char *HttpMessage::findHeader(const char *name) {
    PSTR_MAP_ITER it = _headerMap.find(name);
    if (it != _headerMap.end()) {
        return it->second;
    }
    return NULL;
}

// Connection
Connection *HttpMessage::getConnection() {
    return _connection;
}

// connection
void HttpMessage::setConnection(Connection *connection) {
    _connection = connection;
}

/*
 * 设置包的头域是否默认
 */
/*void HttpMessage::setDefault(bool isDefault) {
    _isDefault = isDefault;
}*/

/*
 * 设置header
 */
void HttpMessage::setHeader(const char *name, const char *value) {
    //if (_isDefault)		//默认含固定字段时则排除connection和content-length
    //{
    	if (name[0] == 'C') {
        	if (strcmp(name, "Connection") == 0 || strcmp(name, "Content-Length") == 0) {
            	return;
        	}
    	}
    //}
    _headerMap2[name] = value;
}

/*
 * 设置请求行内容
 */
/*void HttpMessage::setHeadLine(const char *headerline){
	if (_headerline) {
        ::free(_headerline);
        _headerline = NULL;
    }
    if (headerline) {
        _headerline = strdup(headerline);
    }
}*/

/*
 * 设置状态
 */
void HttpMessage::setStatus(bool status, const char *statusMessage) {
    _status = status;
    if (_statusMessage) {
        ::free(_statusMessage);
        _statusMessage = NULL;
    }
    if (statusMessage) {
        _statusMessage = strdup(statusMessage);
    }
}

/*
 * 设置响应包body内容
 */
void HttpMessage::setBody(const char *body, int len) {
    if (body) {
        _body = (char *) malloc(len);
        memcpy(_body, body, len);
        _bodyLen = len;
    }
}

/*
 * 设置多表单内容一个part
 */
/*bool HttpMessage::setMutiPartBody(const std::map<std::string, std::string> parameters,const std::string upload_contents,const std::string boundary_str) 
{
    if (boundary_str.empty()) {
        return false;
    }
 
    _mutipartbody->clear();
 
    _mutipartbody->append("--" + boundary_str + "\r\n");    //边界线

    //添加mutipart的body部分的
    for (std::map<std::string, std::string>::const_iterator pos = parameters.begin();
        pos != parameters.end(); ++pos) {    
            _mutipartbody->append(pos->first + ": " + pos->second + "\r\n");
    }
 
    //添加包一个part的内容 
    _mutipartbody->append("\r\n");
    
    if (!upload_contents.empty()) {
        _mutipartbody->append(upload_contents);
    }
    _mutipartbody->append("\r\n");
    //_mutipartbody->append("--" + boundary_str + "--\r\n");
    return true;
}*/

/*
 * 是否keepalive
 */
void HttpMessage::setKeepAlive(bool keepAlive) {
    _isKeepAlive = keepAlive;
}

}