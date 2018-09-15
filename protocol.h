
#pragma once
#include <vector>
#include <string>
#include <memory.h>

#define SERIALIZE_DECLARATION 		public: virtual int ByteSize(); 		public: virtual int SerializeToArray(void* data, int size);		public: virtual int ParseFromArray(void* data); 
#define ONLY_VAR_DEFINED(type, name) 	public: inline void set_##name(type a){name##_ = a;} inline type name(){return name##_;} 	private: type name##_;

#define REPEATED_VAR_DEFINED(type, name) 	public: inline int name##_nums() {return name##_.size();} 	public: inline void clear_##name() {std::vector< type > v; name##_.swap(v);} 	public: inline type& add_##name() {type a; name##_.push_back(a); return name##_.back(); } 	public: inline type& name(int index) {return name##_.at(index);}	public: std::vector< type > name() {return name##_;} 	private: std::vector< type > name##_;

typedef struct stMessageHeader
{
	int command_id;
	int data_param;
	int data_len;
}MessageHeader;

class Message
{
public:
	virtual int ByteSize() = 0;
	virtual int SerializeToArray(void* data, int size) = 0;
	virtual int ParseFromArray(void* data) = 0;

	void* buffer_;
	Message() {buffer_ = NULL;}
	virtual ~Message() {if (buffer_) {free(buffer_);}}
};

class WireFormat
{
public:
	static int WriteBool(bool value, void* buffer);
	static int ReadBool(bool &value, void* buffer);

	static int WriteUChar(unsigned char value, void* buffer);
	static int ReadUChar(unsigned char &value, void* buffer);

	static int WriteSChar(char value, void* buffer);
	static int ReadSChar(char &value, const void* buffer);

	static int WriteUInt32(unsigned int value, void* buffer);
	static int ReadUInt32(unsigned int &value, const void* buffer);

	static int WriteUInt64(unsigned long long value, void* buffer);
	static int ReadUInt64(unsigned long long &value, const void* buffer);

	static int WriteSInt32(int value, void* buffer);
	static int ReadSInt32(int &value, const void* buffer);

	static int WriteSInt64(int64_t value, void* buffer);
	static int ReadSInt64(int64_t &value, const void* buffer);

	static int WriteFloat(float value, void* buffer);
	static int ReadFloat(float &value, const void* buffer);

	static int WriteDouble(double value, void* buffer);
	static int ReadDouble(double &value, const void* buffer);

	static int WriteString(std::string& value, void* buffer);
	static int ReadString(std::string& value, void* buffer);

	static int WriteMessage(Message& value, void* buffer, int buffer_size);
	static int ReadMessage(Message& value, void* buffer);
};

class MessageFactory
{
public:
	static void* GenerateMessage(char id, Message& message, int &size);
	static bool ExplainMessage(void* buffer, Message& message);
};

class ReqReport: public Message {
    SERIALIZE_DECLARATION;

    //第一个字段的描述BOOL类型
    ONLY_VAR_DEFINED(bool, boolvar); 

    //第二个字段的描述CHAR类型
    ONLY_VAR_DEFINED(char, charvar); 

    //第三个字段的描述BYTE类型
    ONLY_VAR_DEFINED(unsigned char, bytevar); 

    //第四个字段的描述FLOAT类型
    ONLY_VAR_DEFINED(float, floatvar); 

    //第四个字段的描述DOUBLE类型
    ONLY_VAR_DEFINED(double, doublevar); 

    //第四个字段的描述INT类型
    ONLY_VAR_DEFINED(int, intvar); 

    //第四个字段的描述UINT类型
    ONLY_VAR_DEFINED(unsigned int, uintvar); 

    //第四个字段的描述INT32类型
    ONLY_VAR_DEFINED(int, _32var); 

    //第四个字段的描述UINT32类型
    ONLY_VAR_DEFINED(unsigned int, _u32); 

    //第四个字段的描述INT64类型
    ONLY_VAR_DEFINED(long long, _i64); 

    //第四个字段的描述UINT64类型
    ONLY_VAR_DEFINED(unsigned long long, _u64); 

    //第四个字段的描述STRING类型
    ONLY_VAR_DEFINED(std::string, str); 

};

class RspReport: public Message {
    SERIALIZE_DECLARATION;

    //第一个字段的描述BOOL类型
    REPEATED_VAR_DEFINED(bool, boolvararray); 
    //第二个字段的描述CHAR类型
    REPEATED_VAR_DEFINED(char, charvararray); 
    //第三个字段的描述BYTE类型
    REPEATED_VAR_DEFINED(unsigned char, bytevararray); 
    //第四个字段的描述FLOAT类型
    REPEATED_VAR_DEFINED(float, floatvararray); 
    //第四个字段的描述DOUBLE类型
    REPEATED_VAR_DEFINED(double, doublevararray); 
    //第四个字段的描述INT类型
    REPEATED_VAR_DEFINED(int, intvararray); 
    //第四个字段的描述UINT类型
    REPEATED_VAR_DEFINED(unsigned int, uintvararray); 
    //第四个字段的描述INT32类型
    REPEATED_VAR_DEFINED(int, _32vararray); 
    //第四个字段的描述UINT32类型
    REPEATED_VAR_DEFINED(unsigned int, _u32array); 
    //第四个字段的描述INT64类型
    REPEATED_VAR_DEFINED(long long, _i64array); 
    //第四个字段的描述UINT64类型
    REPEATED_VAR_DEFINED(unsigned long long, _u64array); 
    //第四个字段的描述STRING类型
    REPEATED_VAR_DEFINED(std::string, strarray); 
};

