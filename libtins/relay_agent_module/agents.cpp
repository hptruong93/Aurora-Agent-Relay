#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "agents.h"
#include <thread>

#include <zmq.hpp>
#include <iostream>

using namespace RelayAgents;
using namespace std;

Agents::Agents(string init_port) : port(init_port)
{
	this->context = unique_ptr<zmq::context_t>(new zmq::context_t(1));
	this->socket = unique_ptr<zmq::socket_t>(new zmq::socket_t(*(this->context.get()), ZMQ_REP));

	string address = "tcp://*:" + this->port;
	(this->socket.get())->bind(address.c_str());
}

void Agents::spin()
{
	zmq::message_t received_msg;
	zmq::message_t send_message(DEFAULT_MSG_SIZE);
	while(true)
	{
		this->socket.get()->recv(&(received_msg));

		// Do parsing

		send_message.rebuild();
		this->socket.get()->send(send_message);
	}
}

int main(int argc, char *argv[])
{
	// Test
	WlanToWarpAgent test_wtp_agent;
	thread t1(&WlanToWarpAgent::run, &test_wtp_agent, argc, argv);

	t1.join();

	return 0;
}