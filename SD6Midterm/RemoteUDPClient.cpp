#include <algorithm>
#include <math.h>
#include "Primitives/Color3b.hpp"
#include "Systems/Time Utility.hpp"
#include "RemoteUDPClient.hpp"

extern SOCKET g_Socket;
extern Color3b g_itColor;

RemoteUDPClient::RemoteUDPClient()
{
	m_isDeclaringVictory = false;
	initializeTimeUtility();
}

void RemoteUDPClient::sendAllPendingPackets()
{
	double sendTime = getCurrentTimeSeconds();
	//,loop through all the packets and set the send time and order number

	//send the actual packets
	int WSAResult;
	for (unsigned int ii = 0; ii < m_pendingPacketsToSend.size(); ii++)
	{
		WSAResult = sendto(g_Socket, (char*)&m_pendingPacketsToSend[ii], sizeof(m_pendingPacketsToSend[ii]), 0, (const sockaddr*)&(m_sockAddr), sizeof(m_sockAddr));
	}

	m_pendingPacketsToSend.clear(); //possibly dangerous since sendto isn't blocking
}


const bool RemoteUDPClient::operator==(const RemoteUDPClient& rhs) const
{
	bool isEquivalent = true;
	if (m_sockAddr.sin_addr.S_un.S_addr != rhs.m_sockAddr.sin_addr.S_un.S_addr)
	{
		isEquivalent = false;
	}
	if (m_sockAddr.sin_port != rhs.m_sockAddr.sin_port)
	{
		isEquivalent = false;
	}
	//TODO: in the future, a unique user ID may be considered as a way to differentiate clients on the same IP and port,
	//though they may step over each other on the receiving end
	return isEquivalent;
}



void RemoteUDPClient::processUnprocessedPackets()
{
	//,sort prior to processing
	while(!m_unprocessedPackets.empty())
	{
		CS6Packet currentPacket = m_unprocessedPackets.back();
		m_unprocessedPackets.pop_back();

		switch(currentPacket.packetType)
		{
		case 0:
			{
				//do nothing, invalid packet
				break;
			}
		case TYPE_Acknowledge:
			{
				if (currentPacket.data.acknowledged.packetType == TYPE_Acknowledge || currentPacket.data.acknowledged.packetType == 0)
				{
					//prepare them on this end
					m_unit.m_color = Color4f(((float)rand())/RAND_MAX, ((float)rand())/RAND_MAX, ((float)rand())/RAND_MAX, 1.f);
					m_unit.m_position = Vector2f (rand()%500, rand()%500);
					//if there is no it, then THEY'RE IT!
					if (g_itColor == Color3b(0,0,0))
					{
						g_itColor = Color3b(m_unit.m_color);
					}

					//send them a reset
					//and tell them who IT is
					CS6Packet resetPacket;
					resetPacket.packetType = TYPE_GameStart;
					Color3b cleanedColor = Color3b(m_unit.m_color);
					memcpy(resetPacket.playerColorAndID, &cleanedColor, sizeof(cleanedColor));
					memcpy(resetPacket.data.reset.playerColorAndID, &cleanedColor, sizeof(cleanedColor));
					Color3b itColor = g_itColor;
					memcpy(resetPacket.data.reset.itPlayerColorAndID, &itColor, sizeof(itColor));
					resetPacket.data.reset.playerXPosition = m_unit.m_position.x;
					resetPacket.data.reset.playerYPosition = m_unit.m_position.y;
					//,increment and set packet num
					//,and make sure they get it
					m_pendingPacketsToSend.push_back(resetPacket);
				}
				//,else remove it from the non-acked list				
				break;
			}
		case TYPE_Update:
			{
				//,if more recent than the last
				//update the relevant player in our data
				Vector2f relativeVelocity = m_unit.m_position - Vector2f(currentPacket.data.updated.xPosition, currentPacket.data.updated.yPosition);
				m_unit.m_position.x = currentPacket.data.updated.xPosition;
				m_unit.m_position.y = currentPacket.data.updated.yPosition;
				m_unit.m_velocity.x = currentPacket.data.updated.xVelocity;
				m_unit.m_velocity.y = currentPacket.data.updated.yVelocity;
				m_unit.m_orientation = currentPacket.data.updated.yawDegrees;
				//,mark as most recent
				break;
			}
		case TYPE_Victory:
			{
				//DEPRECATED
				//send up the chain that the game is over
				m_isDeclaringVictory = true;
				//ack back
				break;
			}
		}
	}
}


bool RemoteUDPClient::hasTimedOut()
{
	double currentTime = getCurrentTimeSeconds();
	bool timedOut = currentTime > m_lastReceivedPacketTime + 10.f;
	return timedOut;
}