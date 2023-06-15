/*
** EPITECH PROJECT, 2023
** Untitled (Workspace)
** File description:
** Client
*/

#ifndef ICLIENT_HPP_
#define ICLIENT_HPP_

#include "Connection.hpp"
#include <thread>

template <typename T>
class IClient
{
public:
	IClient()
	{}

	virtual ~IClient()
	{
		disconnect();
	}

public:
	bool connect(const std::string& host, const uint16_t port) {
		try {
			asio::ip::tcp::resolver resolver(context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
			std::cout << "Connecting to server: " << host << ":" << port << std::endl;
			
			//asio::ip::tcp::socket socket(context);
			//asio::connect(socket, endpoints);
			std::cout << "Socket created and connected" << std::endl;

			connection = std::make_unique<Connection<T>>(Connection<T>::Owner::Client, context, asio::ip::tcp::socket(context), PacketIN);
			std::cout << "Connection created" << std::endl;

			connection->connectToServer(endpoints);

			contextThread = std::thread([this]() {
				context.run();
			});
		}
		catch (std::exception& e)
		{
			std::cerr << "Client Exception: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	// Disconnect from server
	void disconnect()
	{
		if (isConnected())
		{
			connection->disconnect();
		}
		
		context.stop();
		if (contextThread.joinable())
			contextThread.join();

		connection.release();
	}

	bool isConnected()
	{
		if (connection)
			return connection->isConnected();
		else
			return false;
	}

	void forceClose()
	{
		connection->forceClose();
	}

public:
	void sendTCP(const Packet<T>& msg)
	{
		if (isConnected())
			connection->sendTCP(msg);
	}

	void sendUDP(const Packet<T>& msg)
	{
		if (isConnected())
			connection->sendUDP(msg);
	}

	TSqueue<OwnedPacket<T>>& incoming()
	{
		return PacketIN;
	}

protected:
	asio::io_context context;
	std::thread contextThread;
	std::unique_ptr<Connection<T>> connection;

private:
	TSqueue<OwnedPacket<T>> PacketIN;
};

#endif /* !ICLIENT_HPP_ */