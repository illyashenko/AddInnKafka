#ifndef REDISEXT_H
#define REDISEXT_H

#include<hiredis/hiredis.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

using STRING = std::string;

class RedisContext
{
public:
	RedisContext(const char* host, int port);
	STRING ContextGet(STRING key);
	void ContextSet(STRING key, STRING value);
	bool ConnectOk();
private:
	redisContext* context_;
};

#endif

