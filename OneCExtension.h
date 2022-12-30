#ifndef KAFKAEXT_H
#define KAFKAEXT_H

#include <iostream>
#include <librdkafka/rdkafkacpp.h>

class KafkaProducer
{
public:
	KafkaProducer(std::string broker);
	~KafkaProducer();
	std::string send(std::string topic, std::string message);
	std::string error();
	bool connect();

private:
	RdKafka::Producer* producer;
	RdKafka::Conf* conf;
	std::string error_string_;
};

#endif

