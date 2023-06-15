/*
** EPITECH PROJECT, 2023
** Untitled (Workspace)
** File description:
** Connection
*/

#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "TSqueue.hpp"
#include "Packet.hpp"

template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>>
{
public:
	enum class Owner
	{
		Server,
		Client
	};

	Connection(Owner parent, asio::io_context &asioContext, asio::ip::tcp::socket socket, TSqueue<OwnedPacket<T>> &qIn) : context(asioContext),
																														  socketTCP(std::move(socket)),
																														  PacketIN(qIn)
																														//   endpointUDP(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
																														//   socketUDP(asio::ip::udp::socket(asioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)))
	{
		owner = parent;
		std::cout << "Connection Created: "
				  << "\n";
		// if (socketTCP.is_open())
		// {
		// 	std::cout << "TCP Socket Opened: "
		// 			  << "\n";
		// 	try
		// 	{
		// 		asio::ip::udp::endpoint remote_endpoint;
		// 		if (owner == Owner::Client)
		// 			remote_endpoint = asio::ip::udp::endpoint(socketTCP.remote_endpoint().address(), socketTCP.remote_endpoint().port());
		// 		else
		// 		{
		// 			socketUDP = asio::ip::udp::socket(asioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), socketTCP.local_endpoint().port()));
		// 		}

		// 		endpointUDP = remote_endpoint;
		// 	}
		// 	catch (std::exception &e)
		// 	{
		// 		// std::cerr << "[" << id << "] Error binding UDP socket: " << e.what() << std::endl;
		// 	}
		// }
	}

	virtual ~Connection() {}

	uint32_t getID() const
	{
		return id;
	}

	void setID(uint32_t uid)
	{
		id = uid;
	}

	void connectToClient(uint32_t uid = 0)
	{
		if (owner == Owner::Server)
		{
			if (socketTCP.is_open())
			{
				id = uid;
				readHeaderTCP();
			}
		}
	}

	void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints)
	{
		if (owner == Owner::Client)
		{
			asio::connect(socketTCP, endpoints);
			// asio::async_connect(socketTCP, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
			// 					{
			// 	if (!ec) {
			// 		readHeaderTCP();
			// 		//readHeaderUDP();
			// 	}
			// 	else {
			// 		std::cout << "Connection Failed: " << ec.message() << "\n";
			// } });
			readHeaderTCP();
		}
	}

	void disconnect()
	{
		if (isConnected())
			asio::post(context, [this]()
					   { socketTCP.close(); });
	}

	bool isConnected() const
	{
		return socketTCP.is_open() /*&& socketUDP.is_open()*/;
	}

	void sendTCP(const Packet<T> &msg)
	{
		asio::post(context, [this, msg]()
				   {
			bool bWritingMessage = !PacketOUT.empty();
		PacketOUT.push_back(msg);
		if (!bWritingMessage) {
			writeHeaderTCP();
		} });
	}

	void sendUDP(const Packet<T> &msg)
	{
		asio::post(context, [this, msg]()
				   {
			bool bWritingMessage = !PacketOUT.empty();
		PacketOUT.push_back(msg);
		if (!bWritingMessage) {
			writeHeaderUDP();
		} });
	}

	void forceClose()
	{
		asio::error_code error;
		socketTCP.shutdown(asio::ip::tcp::socket::shutdown_both, error);
		socketTCP.close();
		if (error)
			std::cout << "[" << id << "] Error: " << error.message() << "\n";
	}

private:
	enum class ConnectionType
	{
		TCP,
		UDP
	};

	void writeHeaderTCP()
	{
		asio::async_write(socketTCP, asio::buffer(&PacketOUT.front().header, sizeof(PacketHeader<T>)), [this](std::error_code ec, std::size_t length)
						  {
			if (!ec) {
				if (PacketOUT.front().body.size() > 0) {
					writeBodyTCP();
				}
				else {
					PacketOUT.pop_front();
					if (!PacketOUT.empty()) {
						writeHeaderTCP();
					}
				}
			}
			else {
				std::cout << "[" << id << "] Write Header Fail.\n";
				try {
					//socketTCP.close();
				}
				catch (std::exception &e) {
					std::cout << "[" << id << "] Error: " << e.what() << "\n";
				}
			} });
	}

	void writeBodyTCP()
	{
		asio::async_write(socketTCP, asio::buffer(PacketOUT.front().body.data(), PacketOUT.front().body.size()), [this](std::error_code ec, std::size_t length)
						  {
			if (!ec) {
				PacketOUT.pop_front();
				if (!PacketOUT.empty()) {
					writeHeaderTCP();
				}
			}
			else {
				std::cout << "[" << id << "] Write Body Fail.\n";
				try {
					//socketTCP.close();
				}
				catch (std::exception &e) {
					std::cout << "[" << id << "] Error: " << e.what() << "\n";
				}
			} });
	}

	void readHeaderTCP()
	{
		asio::async_read(socketTCP, asio::buffer(&TMPPacketIN.header, sizeof(PacketHeader<T>)), [this](std::error_code ec, std::size_t length)
						 {
			if (!ec) {
				if (TMPPacketIN.header.size > 0) {
					TMPPacketIN.body.resize(TMPPacketIN.header.size);
					readBodyTCP();
				}
				else {
					addToIncomingMessageQueue(ConnectionType::TCP);
				}
			}
			else {
				//std::cout << "Client [" << id << "] has disconected.\n";
				try {
					//socketTCP.close();
				}
				catch (std::exception &e) {
					std::cout << "[" << id << "] Error: " << e.what() << "\n";
				}
			} });
	}

	void readBodyTCP()
	{
		asio::async_read(socketTCP, asio::buffer(TMPPacketIN.body.data(), TMPPacketIN.body.size()), [this](std::error_code ec, std::size_t length)
						 {
			if (!ec) {
				addToIncomingMessageQueue(ConnectionType::TCP);
			}
			else {
				std::cout << "[" << id << "] Read Body Fail.\n";
				try {
					//socketTCP.close();
				}
				catch (std::exception &e) {
					std::cout << "[" << id << "] Error: " << e.what() << "\n";
				}
			} });
	}

	void addToIncomingMessageQueue(ConnectionType type)
	{
		if (owner == Owner::Server)
			PacketIN.push_back({this->shared_from_this(), TMPPacketIN});
		else
			PacketIN.push_back({nullptr, TMPPacketIN});
		if (type == ConnectionType::TCP)
			readHeaderTCP();
		else
			readHeaderUDP();
	}

	void writeHeaderUDP()
	{
		// socketUDP.async_send_to(asio::buffer(&PacketOUT.front().header, sizeof(PacketHeader<T>)), endpointUDP, [this](std::error_code ec, std::size_t length)
		// 						{
		// 	if (!ec) {
		// 		if (PacketOUT.front().body.size() > 0) {
		// 			writeBodyUDP();
		// 		}
		// 		else {
		// 			PacketOUT.pop_front();
		// 			if (!PacketOUT.empty()) {
		// 				writeHeaderUDP();
		// 			}
		// 		}
		// 	}
		// 	else {
		// 		std::cout << "[" << id << "] Write Header UDP Fail.\n";
		// 		socketUDP.close();
		// 	} });
	}

	void writeBodyUDP()
	{
		// socketUDP.async_send_to(asio::buffer(PacketOUT.front().body.data(), PacketOUT.front().body.size()), endpointUDP, [this](std::error_code ec, std::size_t length)
		// 						{
		// 	if (!ec) {
		// 		PacketOUT.pop_front();
		// 		if (!PacketOUT.empty()) {
		// 			writeHeaderUDP();
		// 		}
		// 	}
		// 	else {
		// 		std::cout << "[" << id << "] Write Body UDP Fail.\n";
		// 		socketUDP.close();
		// 	} });
	}

	void readHeaderUDP()
	{
		// socketUDP.async_receive_from(asio::buffer(&TMPPacketIN.header, sizeof(PacketHeader<T>)), endpointUDP,
		// 							 [this](std::error_code ec, std::size_t length)
		// 							 {
		// 								 if (!ec)
		// 								 {
		// 									 if (TMPPacketIN.header.size > 0)
		// 									 {
		// 										 TMPPacketIN.body.resize(TMPPacketIN.header.size);
		// 										 readBodyUDP();
		// 									 }
		// 									 else
		// 									 {
		// 										 addToIncomingMessageQueue(ConnectionType::UDP);
		// 									 }
		// 								 }
		// 								 else
		// 								 {
		// 									 std::cout << "[" << id << "] Read Header UDP Fail.\n";
		// 								 }
		// 							 });
	}

	void readBodyUDP()
	{
		// socketUDP.async_receive_from(asio::buffer(TMPPacketIN.body.data(), TMPPacketIN.body.size()), endpointUDP,
		// 							 [this](std::error_code ec, std::size_t length)
		// 							 {
		// 								 if (!ec)
		// 								 {
		// 									 addToIncomingMessageQueue(ConnectionType::UDP);
		// 								 }
		// 								 else
		// 								 {
		// 									 std::cout << "[" << id << "] Read Body UDP Fail.\n";
		// 								 }
		// 							 });
	}

protected:
	asio::ip::tcp::socket socketTCP;
	// asio::ip::udp::endpoint endpointUDP;
	// asio::ip::udp::socket socketUDP;
	asio::io_context &context;
	TSqueue<Packet<T>> PacketOUT;
	TSqueue<OwnedPacket<T>> &PacketIN;
	Packet<T> TMPPacketIN;
	Owner owner = Owner::Server;
	uint32_t id = 0;
};

#endif /* !CONNECTION_HPP_ */