from xml.dom.minidom import parse
import xml.dom.minidom

def convert_type(t) :
    if t == "BOOL":
        return "bool"
    elif t == "CHAR":
        return "char"
    elif t == "BYTE":
        return "unsigned char"
    elif t == "DOUBLE":
        return "double"
    elif t == "FLOAT":
        return "float"
    elif t == "UINT32" or t == "UINT":
        return "unsigned int"
    elif t == "INT" or t == "INT32":
        return "int"
    elif t == "INT64" :
        return "long long"
    elif t == "UINT64":
        return "unsigned long long"
    elif t == "STRING" :
        return "std::string"
    else:
        return "void"

def convert_write_format(t, name) :
    if t == "BOOL":
        return "::WriteBool(" + name + "_"
    elif t == "CHAR":
        return "::WriteSChar(" + name + "_"
    elif t == "BYTE":
        return "::WriteUChar(" + name + "_"
    elif t == "DOUBLE":
        return "::WriteDouble(" + name + "_"
    elif t == "FLOAT":
        return "::WriteFloat(" + name + "_"
    elif t == "UINT32" or t == "UINT":
        return "::WriteUInt32(" + name + "_"
    elif t == "INT" or t == "INT32":
        return "::WriteSInt32(" + name + "_"
    elif t == "INT64" :
        return "::WriteSInt64(" + name + "_"
    elif t == "UINT64":
        return "::WriteUInt64(" + name + "_"
    elif t == "STRING" :
        return "::WriteString(" + name + "_"
    else:
        return "void"


def convert_read_format(t, name) :
    if t == "BOOL":
        return "::ReadBool(" + name + "_"
    elif t == "CHAR":
        return "::ReadSChar(" + name + "_"
    elif t == "BYTE":
        return "::ReadUChar(" + name + "_"
    elif t == "DOUBLE":
        return "::ReadDouble(" + name + "_"
    elif t == "FLOAT":
        return "::ReadFloat(" + name + "_"
    elif t == "UINT32" or t == "UINT":
        return "::ReadUInt32(" + name + "_"
    elif t == "INT" or t == "INT32":
        return "::ReadSInt32(" + name + "_"
    elif t == "INT64" :
        return "::ReadSInt64(" + name + "_"
    elif t == "UINT64":
        return "::ReadUInt64(" + name + "_"
    elif t == "STRING" :
        return "::ReadString(" + name + "_"
    else:
        return "void"


DOMTree = xml.dom.minidom.parse("movies.xml")
collection = DOMTree.documentElement
if collection.hasAttribute("desc"):
    print ("Root element : %s" % collection.getAttribute("desc"))

messages = collection.getElementsByTagName("message")

h_head = '''
#pragma once
#include <vector>
#include <string>
#include <memory.h>

#define SERIALIZE_DECLARATION \
		public: virtual int ByteSize(); \
		public: virtual int SerializeToArray(void* data, int size);\
		public: virtual int ParseFromArray(void* data); \

#define ONLY_VAR_DEFINED(type, name) \
	public: inline void set_##name(type a){name##_ = a;} inline type name(){return name##_;} \
	private: type name##_;

#define REPEATED_VAR_DEFINED(type, name) \
	public: inline int name##_nums() {return name##_.size();} \
	public: inline void clear_##name() {std::vector< type > v; name##_.swap(v);} \
	public: inline type& add_##name() {type a; name##_.push_back(a); return name##_.back(); } \
	public: inline type& name(int index) {return name##_.at(index);}\
	public: std::vector< type > name() {return name##_;} \
	private: std::vector< type > name##_;

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

'''

all_proto_h_context = h_head
for msg in messages:
    if msg.hasAttribute("name"):
        all_proto_h_context += "class " + msg.getAttribute("name") + ": public Message {\n"
        all_proto_h_context += "    SERIALIZE_DECLARATION;\n\n"

        fields = msg.getElementsByTagName('field')        
        for field in fields: 
            field_type = field.getAttribute("type")
            all_proto_h_context += "    //" + field.getAttribute("desc") + "\n"
            if field.getAttribute("attr") == "only":
                all_proto_h_context += "    ONLY_VAR_DEFINED(" + convert_type(field_type) + ", " + field.getAttribute("name") + "); \n\n"
            elif field.getAttribute("attr") == "repeated" :
                all_proto_h_context += "    REPEATED_VAR_DEFINED(" + convert_type(field_type) + ", " + field.getAttribute("name") + "); \n"
    all_proto_h_context+="};\n\n"


with open("protocol.h", 'w', encoding="utf-8") as f:
    f.write(all_proto_h_context)

#########################################
cpp_head = '''
#include "protocol.h"

static unsigned int EncodeInt(int n)
{
	return (n << 1) ^ (n >> (sizeof(int) * 8 - 1 ));
}

static int DecodeInt(unsigned int n)
{
	return (n >> 1) ^ -static_cast<int>(n & 1);
}

static int VarintSize(unsigned int value)
{
	if (value < (1 << 7)) {
		return 1;
	} else if (value < (1 << 14)) {
		return 2;
	} else if (value < (1 << 21)) {
		return 3;
	} else if (value < (1 << 28)) {
		return 4;
	} else {
		return 5;
	}
}

unsigned int EncodeFloat(float value)
{
	union {float f; unsigned int i;};
	f = value;
	return i;
}

float DecodeFloat(unsigned int value) 
{
	union {float f; unsigned int i;};
	i = value;
	return f;
}

unsigned long long EncodeDouble(double value)
{
	union {double f; unsigned long long i;};
	f = value;
	return i;
}

double DecodeDouble(unsigned long long value) 
{
	union {double f; unsigned long long i;};
	i = value;
	return f;
}

int WireFormat::WriteBool(bool value, void* buffer)
{
	int size = 0;
	unsigned char data = value ? 1 : 0;
	((unsigned char*)buffer)[size++] = static_cast<unsigned char>(data);

	return size;
}

int WireFormat::ReadBool(bool &value, void* buffer)
{
	int size = 0;

	unsigned char data = ((unsigned char*)buffer)[size++];

	value = (data == 0) ? false : true;

	return size;
}

int WireFormat::WriteUChar(unsigned char value, void* buffer)
{
	int size = 0;

	((unsigned char*)buffer)[size++] = static_cast<unsigned char>(value);

	return size;
}

int WireFormat::ReadUChar(unsigned char &value, void* buffer)
{
	int size = 0;

	value = ((unsigned char*)buffer)[size++];

	return size;
}

int WireFormat::WriteSChar(char value, void* buffer)
{
	int size = 0;

	((char*)buffer)[size++] = static_cast<char>(value);

	return size;
}

int WireFormat::ReadSChar(char &value, const void* buffer)
{
	int size = 0;

	value = ((char*)buffer)[size++];

	return size;
}

int WireFormat::WriteUInt32(unsigned int value, void* buffer)
{
	int size = 0;
	unsigned int data = value;

	while (data > 0x7F)
	{
		((unsigned char*)buffer)[size++] = (static_cast<unsigned char>(data) & 0x7F) | 0x80;
		data >>= 7;
	}
	((unsigned char*)buffer)[size++] = static_cast<unsigned char>(data) & 0x7F;
	return size;
}

int WireFormat::ReadUInt32(unsigned int &value, const void* buffer)
{
	int size = 0;

	const unsigned char* ptr = (const unsigned char*)buffer;
	unsigned int result = 0;
	unsigned int b;

	b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |=  b         << 28; if (!(b & 0x80)) goto done;

done:
	size = ptr - (const unsigned char*)buffer;
	value = result;
	return size;
}

int WireFormat::WriteUInt64(unsigned long long value, void* buffer)
{
	int size = 0;
	unsigned long long data = value;

	while (data > 0x7F)
	{
		((unsigned char*)buffer)[size++] = (static_cast<unsigned char>(data) & 0x7F) | 0x80;
		data >>= 7;
	}
	((unsigned char*)buffer)[size++] = static_cast<unsigned char>(data) & 0x7F;
	return size;
}

int WireFormat::ReadUInt64(unsigned long long &value, const void* buffer)
{
	int size = 0;

	const unsigned char* ptr = (const unsigned char*)buffer;
	unsigned long long result = 0;
	unsigned long long b;

	b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 28; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 35; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 42; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 49; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |=  b         << 56; if (!(b & 0x80)) goto done;

done:
	size = ptr - (const unsigned char*)buffer;
	value = result;
	return size;
}


int WireFormat::WriteSInt32(int value, void* buffer)
{
	unsigned int data = EncodeInt(value);
	int size = 0;

	size = WriteSInt32(data, buffer);

	return size;
}

int WireFormat::ReadSInt32(int &value, const void* buffer)
{
	int size = 0;
	unsigned int data = 0;

	size =ReadUInt32(data, buffer);

	value = DecodeInt(data);

	return size;
}

int WireFormat::WriteFloat(float value, void* buffer)
{
	unsigned int data = EncodeFloat(value);
	int size = 0;

	size = WriteSInt32(data, buffer);

	return size;
}

int WireFormat::ReadFloat(float &value, const void* buffer)
{
	int size = 0;
	unsigned int data = 0;

	size =ReadUInt32(data, buffer);

	value = DecodeFloat(data);

	return size;
}


int WireFormat::WriteDouble(double value, void* buffer)
{
	unsigned long long data = EncodeDouble(value);
	int size = 0;

	size = WriteUInt64(data, buffer);

	return size;
}

int WireFormat::ReadDouble(double &value, const void* buffer)
{
	int size = 0;
	unsigned long long data = 0;

	size = ReadUInt64(data, buffer);

	value = DecodeFloat(data);

	return size;
}


int WireFormat::WriteString(std::string& value, void* buffer)
{
	unsigned char* output = (unsigned char*)buffer;
	int total_size = 0;

	total_size = WriteSInt32(value.length(), output);
	output += total_size;

	memcpy(output, value.c_str(), value.length());

	return (total_size + value.length());
}

int WireFormat::ReadString(std::string& value, void* buffer)
{
	int buffer_size = 0;

	unsigned char* input = (unsigned char*)buffer;
	int offset = ReadSInt32(buffer_size, input);

	input += offset;
    
    char* data = (char*)malloc(buffer_size + 1);
    if (!data) return -1;

    memset(data, 0x00, buffer_size + 1);
	memcpy(data, input, buffer_size);

    value = data;
    
    free(data);
	
    return (offset + buffer_size);
}

int WireFormat::WriteMessage(Message& value, void* buffer, int buffer_size) 
{
	int size = value.ByteSize();

	if(size > buffer_size) return -1;

	value.SerializeToArray(buffer, size);

	return size;
}

int WireFormat::ReadMessage(Message& value, void* buffer) 
{
	int size = value.ParseFromArray(buffer);

	return size;
}

void* MessageFactory::GenerateMessage(char id, Message& message, int &size)
{
	int header_size = sizeof(MessageHeader);
	int data_size = message.ByteSize();

	if(message.buffer_) free(message.buffer_);

	message.buffer_ = malloc(header_size + data_size);
	if(!message.buffer_ )return NULL;

	memset(message.buffer_, 0x00, header_size + data_size);

	((MessageHeader*)message.buffer_)->command_id = (id | 0x01 << 16);
	((MessageHeader*)message.buffer_)->data_len = data_size;

	unsigned char* data_buffer = (unsigned char*)message.buffer_ + header_size;

	data_size = message.SerializeToArray(data_buffer, data_size);

	size = header_size + data_size;

	return message.buffer_;
}

bool MessageFactory::ExplainMessage(void* buffer, Message& message)
{
	int header_size = sizeof(MessageHeader);	
	int data_size = ((MessageHeader*)buffer)->data_len;

	unsigned char* data_buffer = (unsigned char*)buffer + header_size;

	int len = message.ParseFromArray(data_buffer);
    // If add new data, the len will longer than data_size.
    // the data is bad when len shorter than data_size.
	return (data_size >= len);
}

'''

all_proto_cpp_context = cpp_head
for msg in messages:
    if msg.hasAttribute("name"):
        
        # ByteSize
        all_proto_cpp_context += "int " + msg.getAttribute("name") + "::ByteSize(){\n"
        all_proto_cpp_context += "    int total_size = 0;\n\n"

        fields = msg.getElementsByTagName('field')        
        for field in fields: 
            field_type = field.getAttribute("type")
            if field.getAttribute("attr") == "only":
                if field_type != "STRING":
                    all_proto_cpp_context += "    total_size += sizeof("+ field.getAttribute("name") + "_);\n\n"
                else:                    
                    all_proto_cpp_context += "    total_size += " + field.getAttribute("name") +"_.length();\n\n"

            elif field.getAttribute("attr") == "repeated" : 
                all_proto_cpp_context += "    total_size += VarintSize(EncodeInt(" + field.getAttribute("name") +"_nums()));\n"
                all_proto_cpp_context += "    for (int i = 0; i < " + field.getAttribute("name") +  "_nums(); i++){total_size += VarintSize(EncodeInt("+ field.getAttribute("name") + "(i)));} \n\n"

        all_proto_cpp_context+="    return total_size;\n"
        all_proto_cpp_context+="}\n\n"

        # SerializeToArray
        all_proto_cpp_context += "int " + msg.getAttribute("name") + "::SerializeToArray(void* array_buffer, int size){\n"

        all_proto_cpp_context += "    int byte_size = ByteSize();\n"
        all_proto_cpp_context += "    if (size < byte_size) return -1;\n"
        all_proto_cpp_context += "\n"	
        all_proto_cpp_context += "    unsigned char* data = (unsigned char*)array_buffer;\n"
        all_proto_cpp_context += "    if (!data) return -1;\n"
        all_proto_cpp_context += "\n"	
        all_proto_cpp_context += "    (void)memset(data ,0x00, size);\n"
        all_proto_cpp_context += "    unsigned int offset = 0;\n\n"	

        fields = msg.getElementsByTagName('field')        
        for field in fields: 
            field_type = field.getAttribute("type")
            if field.getAttribute("attr") == "only":
                all_proto_cpp_context += "    offset = WireFormat" + convert_write_format(field.getAttribute("type"),field.getAttribute("name")) +", data); data += offset;\n\n"

            elif field.getAttribute("attr") == "repeated" : 
                all_proto_cpp_context += "    offset += VarintSize(EncodeInt(" + field.getAttribute("name") +"_nums()));\n"
                all_proto_cpp_context += "    for (int i = 0; i < " + field.getAttribute("name") +  "_nums(); i++){offset += VarintSize(EncodeInt("+ field.getAttribute("name") + "(i)));} \n\n"

        all_proto_cpp_context+="    return (int)(data - (unsigned char*)array_buffer);\n"
        all_proto_cpp_context+="}\n\n"


        # ParseFromArray
        all_proto_cpp_context += "int " + msg.getAttribute("name") + "::ParseFromArray(void* array_buffer){\n"

        all_proto_cpp_context += "    int offset = 0;\n\n"
        all_proto_cpp_context += "    unsigned char* data = (unsigned char*)array_buffer;\n"
        all_proto_cpp_context += "    if (!data) return -1;\n"
        all_proto_cpp_context += "\n"

        fields = msg.getElementsByTagName('field')        
        for field in fields: 
            field_type = field.getAttribute("type")
            if field.getAttribute("attr") == "only":
                all_proto_cpp_context += "    offset = WireFormat" + convert_read_format(field.getAttribute("type"),field.getAttribute("name")) +", data); data += offset;\n\n"

            elif field.getAttribute("attr") == "repeated" : 
                all_proto_cpp_context += "    offset += VarintSize(EncodeInt(" + field.getAttribute("name") +"_nums()));\n"
                all_proto_cpp_context += "    for (int i = 0; i < " + field.getAttribute("name") +  "_nums(); i++){offset += VarintSize(EncodeInt("+ field.getAttribute("name") + "(i)));} \n\n"

        all_proto_cpp_context+="    return (int)(data - (unsigned char*)array_buffer);\n"
        all_proto_cpp_context+="}\n\n"

with open("protocol.cpp", 'w', encoding="utf-8") as f:
    f.write(all_proto_cpp_context)


print(all_proto_cpp_context)
