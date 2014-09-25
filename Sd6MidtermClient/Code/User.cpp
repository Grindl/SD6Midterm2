#include "User.hpp"
#include "Connection.hpp"
#include "Primitives/Color3b.hpp"
#include "Systems/Time Utility.hpp"

User::User()
{
	m_isInGame = false;
	m_lastSentPacketNum = 0;
	initializeTimeUtility();
}

CS6Packet User::sendInput()
{
	if (m_userType == USER_REMOTE || !m_isInGame)
	{
		return CS6Packet();
	}
	else
	{
		CS6Packet outPacket = m_unit.packForSend();
		m_lastSentPacketNum++;
		outPacket.packetNumber = m_lastSentPacketNum;
		g_serverConnection->sendPacket(outPacket);
		return outPacket;
	}
}

void User::processUpdatePacket(CS6Packet newData)
{
	m_lastReceivedPacketTime = getCurrentTimeSeconds();
	switch(newData.packetType)
	{
	case 0:
		{
			//BAD
			break;
		}
	case TYPE_Update:
		{
			//,if this packet is applicable for processing
			Vector2f nextTarget = Vector2f(newData.data.updated.xPosition, newData.data.updated.yPosition);
			//,prepare the unit for autonomous movement
			m_unit.m_target = nextTarget;
			m_unit.m_velocity.x = newData.data.updated.xVelocity;
			m_unit.m_velocity.y = newData.data.updated.yVelocity;
			m_unit.m_orientationDegrees = newData.data.updated.yawDegrees;
			Color3b tempColor;
			memcpy(&tempColor, newData.playerColorAndID, sizeof(tempColor));
			m_unit.m_color = Color4f(tempColor);
			//jump the player if the distance is too far
			if (m_unit.m_position == Vector2f(0,0) || m_unit.m_position.distanceSquared(m_unit.m_target) > 400)
			{
				m_unit.m_position.x = newData.data.updated.xPosition;
				m_unit.m_position.y = newData.data.updated.yPosition;

			}
			//,and decide which packet should be processed next
			break;
		}
	}
}

void User::update(float deltaSeconds)
{
	m_unit.update(deltaSeconds);
	sendInput();
}

void User::render()
{
	m_unit.render();
}