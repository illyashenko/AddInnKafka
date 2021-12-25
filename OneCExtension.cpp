#include "OneCExtension.h"

Kafka::Kafka(std::string broker) : error_string_("")
{
	RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	conf->set("metadata.broker.list", broker.c_str(), this->error_string_);
	this->producer = RdKafka::Producer::create(conf, this->error_string_);
}

Kafka::~Kafka()
{
}

std::string Kafka::send(std::string topic, std::string message)
{
	RdKafka::ErrorCode response = producer->produce(topic, RdKafka::Topic::PARTITION_UA,
		RdKafka::Producer::RK_MSG_COPY, (void*)(message.c_str()), message.length() + 1, NULL, 0, 0, NULL);
	producer->flush(10);
	std::string errs(RdKafka::err2str(response));

	return errs;
}

std::string Kafka::read(std::string topik)
{
	return std::string();
}

std::string Kafka::error()
{
	return this->error_string_;
}

bool Kafka::connect()
{
	return producer ? true : false;
}
