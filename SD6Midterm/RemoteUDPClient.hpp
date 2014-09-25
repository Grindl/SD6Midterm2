#pragma once
#ifndef include_REMOTEUDPCLIENT
#define include_REMOTEUDPCLIENT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <queue>

#include "CS6Packet.hpp"
#include "Entity.hpp"


#pragma comment(lib, "Ws2_32.lib")

class RemoteUDPClient
{
public:
	RemoteUDPClient();

	struct sockaddr_in m_sockAddr;
	Entity m_unit;
	std::vector<CS6Packet> m_pendingPacketsToSend;
	std::vector<CS6Packet> m_unprocessedPackets;
	bool m_isDeclaringVictory;
	double m_lastReceivedPacketTime;

	void sendAllPendingPackets();
	void processUnprocessedPackets();
	const bool operator==(const RemoteUDPClient& rhs) const;
	bool hasTimedOut();
};

#endif