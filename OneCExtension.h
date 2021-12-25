#ifndef REDISEXT_H
#define REDISEXT_H

#include <iostream>
#include <librdkafka/rdkafkacpp.h>

class Kafka
{
public:
	Kafka(std::string broker);
	~Kafka();
	std::string send(std::string topic, std::string message);
	std::string read(std::string topic);
	std::string error();
	bool connect();

private:
	RdKafka::Producer* producer;
	std::string error_string_;
};

#endif

