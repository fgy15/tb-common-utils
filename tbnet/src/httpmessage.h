#ifndef TBNET_HTTP_MESSAGE_H
#define TBNET_HTTP_MESSAGE_H

#include <map>

namespace tbnet {

class HttpMessage : public Packet {
public:
    /*
     * 构造函数
     */
    HttpMessage(int pcode);
    
    /*
     * 构造函数
     */
    HttpMessage();

    /*
     * 析构函数
     */
    ~HttpMessage();

    /*
     * 计算出数据包的长度
     */
    void countDataLen();

    /*
     * 组装
     */
    bool encode(DataBuffer *output);

    /*
     * 解开
     */
    bool decode(DataBuffer *input, PacketHeader *header);

    /*
     * 查询请求url
     */
    char *getQuery();

    /*
     * 查询请求包body
     */
    char *getbody();

    /*
     * 查询请求方法
     */
    int getmethod();

    /*
     * 查询请求为mutipart时的boundary
     */
    //char* getboundary();

    /*
     * 寻找其他头信息
     */
    const char *findHeader(const char *name);

	/*
	 * 设置包的头域是否默认
	 */
	//void setDefault(bool isDefault);

    /*
     * 设置header
     */
    void setHeader(const char *name, const char *value);

    /*
     * 设置请求行内容
     */
   // void setHeadLine(const char *headerline);  

    /*
     * 设置状态
     */
    void setStatus(bool status, const char *statusMessage = NULL);
     
    /*
     * 设置响应包body内容
     */
     void setBody(const char *body, int len);   

	/*
     * 设置响应包多表单内容
     */
    //void setMutiPartBody(const std::map<std::string, std::string> parameters,const std::string upload_contents,const std::string boundary);

    /*
     * 是否keepalive
     */
    void setKeepAlive(bool keepAlive);

    /*
     * 是否keepalive
     */
    bool isKeepAlive();

    /*
     * 取回Connection
     */
    Connection *getConnection();

    /*
     * 设置connection
     */
    void setConnection(Connection *connection);


    /*
     * 是否Request
     */
    bool isRequest() {
        if (_pcode==1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /*
     * 是否Response
     */
    bool isResponse() {
        if (_pcode==0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /*
    * 返回_pcode
    */
    int getpcode()
    {
        return _pcode;
    }


private:
	
    //request
	char *_strHeader;       // 保存头内容的buffer
    char *_strQuery;        // 查询串
    int _method;            // get - 1,post- 2,delete - 3,put - 4
    PSTR_MAP _headerMap;    // 请求包其他头信息的map
    tbnet::Connection * _connection; // 存connection

    //新增
    //char *_getbody;			//请求的body
    //char *_headerline;		//请求的请求行
    
    //response
    bool _status;                   // 返回的状态, true => 200, false => 404
    char *_statusMessage;           // 状态
    char *_body;                    // 返回的内容
    int _bodyLen;                   // 返回内容找长度
    STRING_MAP _headerMap2;          // 响应包头信息
    
    //共有
    bool _isKeepAlive;              // 是否keepalive

    //新增
    int _pcode;             //请求为1，响应为0 未定义为-1
    //bool _isDefault;			//true则默认包含固定字段，false则头域字段无固定字段
    //char *_boundary;            //mutipart类型时的边界字符串
    //std::string _mutipartbody;      //mutipartbody
};

}

#endif
