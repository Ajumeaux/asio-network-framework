/*
** EPITECH PROJECT, 2023
** Untitled (Workspace)
** File description:
** IServer
*/

#ifndef ISERVER_HPP_
#define ISERVER_HPP_

#include "Connection.hpp"
#include "Packet.hpp"
#include "TSqueue.hpp"

template<typename T>
class IServer {
public:
	enum class ConnectionType
	{
		TCP,
		UDP
	};

	IServer(uint16_t port) : m_asioAcceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {

	}

	virtual ~IServer() {
		stop();
	}

	// Starts the Server!
	bool start() {
		try {
			waitForClientConnection();

			m_threadContext = std::thread([this]() {
				context.run();
			});
		} catch (std::exception& e) {
			std::cerr << "[SERVER] Exception: " << e.what() << "\n";
			return false;
		}

		std::cout << "[SERVER] Started! on port: " << m_asioAcceptor.local_endpoint().port() << " and address: " << m_asioAcceptor.local_endpoint().address() << "\n";
		return true;
	}

	void stop() {
		context.stop();

		if (m_threadContext.joinable())
			m_threadContext.join();

		std::cout << "[SERVER] Stopped!\n";
	}

	// ASYNC - Instruct asio to wait for Connection
	void waitForClientConnection() {
		m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec) {
				std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

				std::shared_ptr<Connection<T>> newconn = std::make_shared<Connection<T>>(Connection<T>::Owner::Server, context, std::move(socket), PacketIN);

				if (onClientConnect(newconn)) {
					_clients.push_back(std::move(newconn));
					_clients.back()->connectToClient(nIDCounter++);
					std::cout << "[" << _clients.back()->getID() << "] Connection Approved\n";
				} else {
					std::cout << "[-----] Connection Denied\n";
				}
			} else {
				std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
			}
			waitForClientConnection();
		});
	}

	void disconnectClient(std::shared_ptr<Connection<T>> client) {
		onClientDisconnect(client);
		//client->disconnect();
		for (auto& iclient : _clients) {
			if (iclient->getID() >= client->getID())
				iclient->setID(client->getID() - 1);
		}
 
		for (auto it = _clients.begin(); it != _clients.end(); it++) {
			if (*it == client) {
				_clients.erase(it); // 0 1 2 3 4
				break;
			}
		}
		nIDCounter--;
	}

	void disconnectAllClients() {
		for (auto& client : _clients) {
			onClientDisconnect(client);
			//client->disconnect();
		}
		_clients.clear();
		nIDCounter = 0;
	}

	void sendClient(ConnectionType type, std::shared_ptr<Connection<T>> client, const Packet<T>& msg) {
		if (client && client->isConnected()) {
			type == ConnectionType::TCP ? client->sendTCP(msg) : client->sendUDP(msg);
		}
		else {
			onClientDisconnect(client);
			client.reset();
			_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
		}
	}

	// Send Packet to all clients
	void sendAllClients(ConnectionType type, const Packet<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
		bool bInvalidClientExists = false;

		for (auto& client : _clients) {
			if (client && client->isConnected()) {
				if (client != pIgnoreClient)
					type == ConnectionType::TCP ? client->sendTCP(msg) : client->sendUDP(msg);
			} else {
				onClientDisconnect(client);
				//nIDCounter--;
				client.reset();
				//std::cout << "[SERVER] Removing client [" << client->getID() << "]\n";
				//_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
				//std::cout << "[SERVER] Client removed\n";
				bInvalidClientExists = true;
			}
		}
		if (bInvalidClientExists)
			_clients.erase(std::remove(_clients.begin(), _clients.end(), nullptr), _clients.end());
	}

	// Force Server to respond to incoming Packets
	void update(size_t nMaxMessages = -1, bool bWait = false) {
		if (bWait)
			PacketIN.wait();
		size_t nMessageCount = 0;
		// for (auto& client : _clients) {
		// 	if (client && client->isConnected()) {
		// 		continue;
		// 	} else {
		// 		onClientDisconnect(client);
		// 		nIDCounter--;
		// 		client.reset();
		// 		//std::cout << "[SERVER] Removing client [" << client->getID() << "]\n";
		// 		//_clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
		// 		//std::cout << "[SERVER] Client removed\n";
		// 	}
		// }
		while (nMessageCount < nMaxMessages && !PacketIN.empty()) {
			auto msg = PacketIN.pop_front();

			onMessage(msg.remote, msg.packet);

			nMessageCount++;
		}
	}

protected:
	virtual bool onClientConnect(std::shared_ptr<Connection<T>> client)
	{
		return false;
	}

	virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client)
	{

	}

	virtual void onMessage(std::shared_ptr<Connection<T>> client, Packet<T>& msg)
	{

	}


protected:
	TSqueue<OwnedPacket<T>> PacketIN;

	std::deque<std::shared_ptr<Connection<T>>> _clients;

	asio::io_context context;
	std::thread m_threadContext;

	asio::ip::tcp::acceptor m_asioAcceptor;

	uint32_t nIDCounter = 0;
};

#endif /* !ISERVER_HPP_ */