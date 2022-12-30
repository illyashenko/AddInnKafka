#include "OneCExtension.h"

KafkaProducer::KafkaProducer(std::string broker) : error_string_("")
{
	this->conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	this->conf->set("metadata.broker.list", broker.c_str(), this->error_string_);
	this->producer = RdKafka::Producer::create(this->conf, this->error_string_);
}

KafkaProducer::~KafkaProducer()
{
	delete this->conf;
	delete this->producer;
}

std::string KafkaProducer::send(std::string topic, std::string message)
{
	RdKafka::ErrorCode response = producer->produce(topic, RdKafka::Topic::PARTITION_UA,
		RdKafka::Producer::RK_MSG_COPY, (void*)(message.c_str()), message.length() + 1, NULL, 0, 0, NULL);
	
	std::string errs(RdKafka::err2str(response));

	producer->poll(0);
	producer->flush(10000);

	return errs;
}

std::string KafkaProducer::error()
{
	return this->error_string_;
}

bool KafkaProducer::connect()
{
	return producer ? true : false;
}
