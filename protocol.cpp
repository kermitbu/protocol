
#include "protocol.h"

static unsigned int EncodeInt(int n)
{
	return (n << 1) ^ (n >> (sizeof(int) * 8 - 1 ));
}

static int DecodeInt(unsigned int n)
{
	return (n >> 1) ^ -static_cast<int>(n & 1);
}

static int VarintSize(unsigned long long value)
{
	if (value < (1 << 7)) {
		return 1;
	} else if (value < (1 << 14)) {
		return 2;
	} else if (value < (1 << 21)) {
		return 3;
	} else if (value < (1 << 28)) {
		return 4;
	} else if (value < (1 << 35)) {
		return 5;
	} else if (value < (1 << 42)) {
		return 6;
	} else if (value < (1 << 49)) {
		return 7;
	} else if (value < (1 << 56)) {
		return 8;
	} else {
		return 9;
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

int ReqReport::ByteSize(){
    int total_size = 0;

    total_size += sizeof(boolvar_);

    total_size += sizeof(charvar_);

    total_size += sizeof(bytevar_);

    total_size += sizeof(floatvar_);

    total_size += sizeof(doublevar_);

    total_size += sizeof(intvar_);

    total_size += sizeof(uintvar_);

    total_size += sizeof(_32var_);

    total_size += sizeof(_u32_);

    total_size += sizeof(_i64_);

    total_size += sizeof(_u64_);

    total_size += str_.length();

    return total_size;
}

int ReqReport::SerializeToArray(void* array_buffer, int size){
    int byte_size = ByteSize();
    if (size < byte_size) return -1;

    unsigned char* data = (unsigned char*)array_buffer;
    if (!data) return -1;

    (void)memset(data ,0x00, size);
    unsigned int offset = 0;

    offset = WireFormat::WriteBool(boolvar_, data); data += offset;

    offset = WireFormat::WriteSChar(charvar_, data); data += offset;

    offset = WireFormat::WriteUChar(bytevar_, data); data += offset;

    offset = WireFormat::WriteFloat(floatvar_, data); data += offset;

    offset = WireFormat::WriteDouble(doublevar_, data); data += offset;

    offset = WireFormat::WriteSInt32(intvar_, data); data += offset;

    offset = WireFormat::WriteUInt32(uintvar_, data); data += offset;

    offset = WireFormat::WriteSInt32(_32var_, data); data += offset;

    offset = WireFormat::WriteUInt32(_u32_, data); data += offset;

    offset = WireFormat::WriteSInt64(_i64_, data); data += offset;

    offset = WireFormat::WriteUInt64(_u64_, data); data += offset;

    offset = WireFormat::WriteString(str_, data); data += offset;

    return (int)(data - (unsigned char*)array_buffer);
}

int ReqReport::ParseFromArray(void* array_buffer){
    int offset = 0;

    unsigned char* data = (unsigned char*)array_buffer;
    if (!data) return -1;

    offset = WireFormat::ReadBool(boolvar_, data); data += offset;

    offset = WireFormat::ReadSChar(charvar_, data); data += offset;

    offset = WireFormat::ReadUChar(bytevar_, data); data += offset;

    offset = WireFormat::ReadFloat(floatvar_, data); data += offset;

    offset = WireFormat::ReadDouble(doublevar_, data); data += offset;

    offset = WireFormat::ReadSInt32(intvar_, data); data += offset;

    offset = WireFormat::ReadUInt32(uintvar_, data); data += offset;

    offset = WireFormat::ReadSInt32(_32var_, data); data += offset;

    offset = WireFormat::ReadUInt32(_u32_, data); data += offset;

    offset = WireFormat::ReadSInt64(_i64_, data); data += offset;

    offset = WireFormat::ReadUInt64(_u64_, data); data += offset;

    offset = WireFormat::ReadString(str_, data); data += offset;

    return (int)(data - (unsigned char*)array_buffer);
}

int RspReport::ByteSize(){
    int total_size = 0;

    total_size += VarintSize(EncodeInt(boolvararray_nums()));
    for (int i = 0; i < boolvararray_nums(); i++){total_size += VarintSize(EncodeInt(boolvararray(i)));} 

    total_size += VarintSize(EncodeInt(charvararray_nums()));
    for (int i = 0; i < charvararray_nums(); i++){total_size += VarintSize(EncodeInt(charvararray(i)));} 

    total_size += VarintSize(EncodeInt(bytevararray_nums()));
    for (int i = 0; i < bytevararray_nums(); i++){total_size += VarintSize(EncodeInt(bytevararray(i)));} 

    total_size += VarintSize(EncodeInt(floatvararray_nums()));
    for (int i = 0; i < floatvararray_nums(); i++){total_size += VarintSize(EncodeInt(floatvararray(i)));} 

    total_size += VarintSize(EncodeInt(doublevararray_nums()));
    for (int i = 0; i < doublevararray_nums(); i++){total_size += VarintSize(EncodeInt(doublevararray(i)));} 

    total_size += VarintSize(EncodeInt(intvararray_nums()));
    for (int i = 0; i < intvararray_nums(); i++){total_size += VarintSize(EncodeInt(intvararray(i)));} 

    total_size += VarintSize(EncodeInt(uintvararray_nums()));
    for (int i = 0; i < uintvararray_nums(); i++){total_size += VarintSize(EncodeInt(uintvararray(i)));} 

    total_size += VarintSize(EncodeInt(_32vararray_nums()));
    for (int i = 0; i < _32vararray_nums(); i++){total_size += VarintSize(EncodeInt(_32vararray(i)));} 

    total_size += VarintSize(EncodeInt(_u32array_nums()));
    for (int i = 0; i < _u32array_nums(); i++){total_size += VarintSize(EncodeInt(_u32array(i)));} 

    total_size += VarintSize(EncodeInt(_i64array_nums()));
    for (int i = 0; i < _i64array_nums(); i++){total_size += VarintSize(EncodeInt(_i64array(i)));} 

    total_size += VarintSize(EncodeInt(_u64array_nums()));
    for (int i = 0; i < _u64array_nums(); i++){total_size += VarintSize(EncodeInt(_u64array(i)));} 

    total_size += VarintSize(EncodeInt(strarray_nums()));
    for (int i = 0; i < strarray_nums(); i++){total_size += VarintSize(EncodeInt(strarray(i)));} 

    return total_size;
}

int RspReport::SerializeToArray(void* array_buffer, int size){
    int byte_size = ByteSize();
    if (size < byte_size) return -1;

    unsigned char* data = (unsigned char*)array_buffer;
    if (!data) return -1;

    (void)memset(data ,0x00, size);
    unsigned int offset = 0;

    offset += VarintSize(EncodeInt(boolvararray_nums()));
    for (int i = 0; i < boolvararray_nums(); i++){offset += VarintSize(EncodeInt(boolvararray(i)));} 

    offset += VarintSize(EncodeInt(charvararray_nums()));
    for (int i = 0; i < charvararray_nums(); i++){offset += VarintSize(EncodeInt(charvararray(i)));} 

    offset += VarintSize(EncodeInt(bytevararray_nums()));
    for (int i = 0; i < bytevararray_nums(); i++){offset += VarintSize(EncodeInt(bytevararray(i)));} 

    offset += VarintSize(EncodeInt(floatvararray_nums()));
    for (int i = 0; i < floatvararray_nums(); i++){offset += VarintSize(EncodeInt(floatvararray(i)));} 

    offset += VarintSize(EncodeInt(doublevararray_nums()));
    for (int i = 0; i < doublevararray_nums(); i++){offset += VarintSize(EncodeInt(doublevararray(i)));} 

    offset += VarintSize(EncodeInt(intvararray_nums()));
    for (int i = 0; i < intvararray_nums(); i++){offset += VarintSize(EncodeInt(intvararray(i)));} 

    offset += VarintSize(EncodeInt(uintvararray_nums()));
    for (int i = 0; i < uintvararray_nums(); i++){offset += VarintSize(EncodeInt(uintvararray(i)));} 

    offset += VarintSize(EncodeInt(_32vararray_nums()));
    for (int i = 0; i < _32vararray_nums(); i++){offset += VarintSize(EncodeInt(_32vararray(i)));} 

    offset += VarintSize(EncodeInt(_u32array_nums()));
    for (int i = 0; i < _u32array_nums(); i++){offset += VarintSize(EncodeInt(_u32array(i)));} 

    offset += VarintSize(EncodeInt(_i64array_nums()));
    for (int i = 0; i < _i64array_nums(); i++){offset += VarintSize(EncodeInt(_i64array(i)));} 

    offset += VarintSize(EncodeInt(_u64array_nums()));
    for (int i = 0; i < _u64array_nums(); i++){offset += VarintSize(EncodeInt(_u64array(i)));} 

    // 个数
	offset += VarintSize(EncodeInt(strarray_nums()));

	// 个数
    for (int i = 0; i < strarray_nums(); i++){

		offset += VarintSize(EncodeInt(strarray(i).length()));
		offset += VarintSize(EncodeString(strarray(i)));
	} 

    return (int)(data - (unsigned char*)array_buffer);
}

int RspReport::ParseFromArray(void* array_buffer){
    int offset = 0;

    unsigned char* data = (unsigned char*)array_buffer;
    if (!data) return -1;

    offset += VarintSize(EncodeInt(boolvararray_nums()));
    for (int i = 0; i < boolvararray_nums(); i++){offset += VarintSize(EncodeInt(boolvararray(i)));} 

    offset += VarintSize(EncodeInt(charvararray_nums()));
    for (int i = 0; i < charvararray_nums(); i++){offset += VarintSize(EncodeInt(charvararray(i)));} 

    offset += VarintSize(EncodeInt(bytevararray_nums()));
    for (int i = 0; i < bytevararray_nums(); i++){offset += VarintSize(EncodeInt(bytevararray(i)));} 

    offset += VarintSize(EncodeInt(floatvararray_nums()));
    for (int i = 0; i < floatvararray_nums(); i++){offset += VarintSize(EncodeInt(floatvararray(i)));} 

    offset += VarintSize(EncodeInt(doublevararray_nums()));
    for (int i = 0; i < doublevararray_nums(); i++){offset += VarintSize(EncodeInt(doublevararray(i)));} 

    offset += VarintSize(EncodeInt(intvararray_nums()));
    for (int i = 0; i < intvararray_nums(); i++){offset += VarintSize(EncodeInt(intvararray(i)));} 

    offset += VarintSize(EncodeInt(uintvararray_nums()));
    for (int i = 0; i < uintvararray_nums(); i++){offset += VarintSize(EncodeInt(uintvararray(i)));} 

    offset += VarintSize(EncodeInt(_32vararray_nums()));
    for (int i = 0; i < _32vararray_nums(); i++){offset += VarintSize(EncodeInt(_32vararray(i)));} 

    offset += VarintSize(EncodeInt(_u32array_nums()));
    for (int i = 0; i < _u32array_nums(); i++){offset += VarintSize(EncodeInt(_u32array(i)));} 

    offset += VarintSize(EncodeInt(_i64array_nums()));
    for (int i = 0; i < _i64array_nums(); i++){offset += VarintSize(EncodeInt(_i64array(i)));} 

    offset += VarintSize(EncodeInt(_u64array_nums()));
    for (int i = 0; i < _u64array_nums(); i++){offset += VarintSize(EncodeInt(_u64array(i)));} 

    offset += VarintSize(EncodeInt(strarray_nums()));
    for (int i = 0; i < strarray_nums(); i++){offset += VarintSize(EncodeInt(strarray(i)));} 

    return (int)(data - (unsigned char*)array_buffer);
}

