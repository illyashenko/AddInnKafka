#include "OneCExtension.h"

RedisContext::RedisContext(const char* host, int port)
{
	context_ = redisConnect(host, port);
}

STRING RedisContext::ContextGet(STRING key)
{
	redisReply* reply;
	STRING return_str;
	reply = static_cast<redisReply*>(redisCommand(context_, "GET %s", key.c_str()));
	return_str = (reply->str) ? reply->str : STRING("ERROR: NOT KEY");
	freeReplyObject(reply);

	return return_str;
}

void RedisContext::ContextSet(STRING key, STRING value)
{
	redisReply* reply;
	reply = static_cast<redisReply*>(redisCommand(context_, "SET %s %s", key.c_str(), value.c_str()));
	freeReplyObject(reply);
}

bool RedisContext::ConnectOk()
{
	return (context_ == NULL || context_->err) ? false : true;
}
