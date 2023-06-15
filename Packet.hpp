/*
** EPITECH PROJECT, 2023
** Untitled (Workspace)
** File description:
** Packet
*/

#ifndef PACKET_HPP_
#define PACKET_HPP_

#include <iostream>
#include <vector>
#include <cstring>
#include <type_traits>
#include "Connection.hpp"

template <typename T>
struct PacketHeader {
	T id{};
	uint32_t size = 0;
};

template <typename T>
struct Packet {
	PacketHeader<T> header{};
	std::vector<uint8_t> body;

	size_t size() const {
		return body.size();
	}

	friend std::ostream& operator<<(std::ostream& os, const Packet<T>& p) {
		os << "ID:" << int(p.header.id) << " Size:" << p.header.size;
		return os;
	}

	template<typename DataType>
	friend Packet<T>& operator<<(Packet<T>& p, const DataType& data) {
		//		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

		size_t i = p.body.size();
		p.body.resize(p.body.size() + sizeof(DataType));
		std::memcpy(p.body.data() + i, &data, sizeof(DataType));
		p.header.size = p.size();
		return p;
	}

	template<typename DataType>
	friend Packet<T>& operator>>(Packet<T>& p, DataType& data) {
		//static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

		size_t i = p.body.size() - sizeof(DataType);
		std::memcpy(&data, p.body.data() + i, sizeof(DataType));
		p.body.resize(i);
		p.header.size = p.size();
		return p;
	}
};

template <typename T>
class Connection;

template <typename T>
struct OwnedPacket {
	std::shared_ptr<Connection<T>> remote = nullptr;
	Packet<T> packet;

	friend std::ostream& operator<<(std::ostream& os, const OwnedPacket<T>& p) {
		os << p.packet;
		return os;
	}

	template<typename DataType>
	friend OwnedPacket<T>& operator<<(OwnedPacket<T>& p, const DataType& data) {
		p.packet << data;
		return p;
	}

	template<typename DataType>
	friend OwnedPacket<T>& operator>>(OwnedPacket<T>& p, DataType& data) {
		p.packet >> data;
		return p;
	}
};


#endif /* !PACKET_HPP_ */