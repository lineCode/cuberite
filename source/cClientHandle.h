
// cClientHandle.h

// Interfaces to the cClientHandle class representing a client connected to this server. The client need not be a player yet





#pragma once
#ifndef CCLIENTHANDLE_H_INCLUDED
#define CCLIENTHANDLE_H_INCLUDED

#include "packets/cPacket.h"
#include "Vector3d.h"
#include "cSocketThreads.h"
#include "cChunk.h"

#include "packets/cPacket_KeepAlive.h"
#include "packets/cPacket_PlayerPosition.h"
#include "packets/cPacket_Respawn.h"
#include "packets/cPacket_RelativeEntityMoveLook.h"
#include "packets/cPacket_Chat.h"
#include "packets/cPacket_Login.h"
#include "packets/cPacket_WindowClick.h"
#include "packets/cPacket_PlayerMoveLook.h"
#include "packets/cPacket_TimeUpdate.h"
#include "packets/cPacket_BlockDig.h"
#include "packets/cPacket_Handshake.h"
#include "packets/cPacket_PlayerLook.h"
#include "packets/cPacket_ArmAnim.h"
#include "packets/cPacket_BlockPlace.h"
#include "packets/cPacket_Flying.h"
#include "packets/cPacket_Disconnect.h"
#include "packets/cPacket_PickupSpawn.h"
#include "packets/cPacket_ItemSwitch.h"
#include "packets/cPacket_EntityEquipment.h"
#include "packets/cPacket_CreativeInventoryAction.h"
#include "packets/cPacket_NewInvalidState.h"
#include "packets/cPacket_UseEntity.h"
#include "packets/cPacket_WindowClose.h"
#include "packets/cPacket_UpdateSign.h"
#include "packets/cPacket_Ping.h"
#include "packets/cPacket_PlayerListItem.h"





class cPlayer;
class cRedstone;





class cClientHandle :  // tolua_export
	public cSocketThreads::cCallback
{											// tolua_export
public:
	enum ENUM_PRIORITY
	{
		E_PRIORITY_LOW,
		E_PRIORITY_NORMAL
	};

	static const int MAXBLOCKCHANGEINTERACTIONS = 10; // 5 didn't help, 10 seems to have done the trick
	
	static const int DEFAULT_VIEW_DISTANCE = 9;  // The default ViewDistance (used when no value is set in Settings.ini)
	static const int MAX_VIEW_DISTANCE = 10;
	static const int MIN_VIEW_DISTANCE = 4;

	cClientHandle(const cSocket & a_Socket, int a_ViewDistance);
	~cClientHandle();

	const cSocket & GetSocket(void) const {return m_Socket; }
	cSocket &       GetSocket(void)       {return m_Socket; }
	
	cPlayer* GetPlayer() { return m_Player; }	// tolua_export

	void Kick(const AString & a_Reason);		//tolua_export
	void Authenticate(void);  // Called by cAuthenticator when the user passes authentication

	void StreamChunks(void);
	
	// Removes the client from all chunks. Used when switching worlds or destroying the player
	void RemoveFromAllChunks(void);
	
	void ChunkJustSent(cChunk * a_ChunkSent);  // Called by cChunk when it is loaded / generated and sent to all clients registered in it

	inline bool IsLoggedIn(void) const { return m_State >= csAuthenticating; }

	void Tick(float a_Dt);

	bool IsDestroyed() { return m_bDestroyed; }
	void Destroy();
	
	bool IsPlaying(void) const {return (m_State == csPlaying); }

	void Send(const cPacket & a_Packet, ENUM_PRIORITY a_Priority = E_PRIORITY_NORMAL);

	const AString & GetUsername(void) const;		//tolua_export
	
	inline short GetPing() const { return m_Ping; }	//tolua_export
	
	void SetViewDistance(int a_ViewDistance);		//tolua_export

	int GetUniqueID() const { return m_UniqueID; }	//tolua_export

private:

	int m_ViewDistance;  // Number of chunks the player can see in each direction; 4 is the minimum ( http://wiki.vg/Protocol_FAQ#.E2.80.A6all_connecting_clients_spasm_and_jerk_uncontrollably.21 )
	
	static const int GENERATEDISTANCE = 2; // Server generates this many chunks AHEAD of player sight. 2 is the minimum, since foliage is generated 1 step behind chunk terrain generation

	int m_ProtocolVersion;
	AString m_Username;
	AString m_Password;
	
	AString m_ReceivedData;  // Accumulator for the data received from the socket, waiting to be parsed; accessed from the cSocketThreads' thread only!

	cCriticalSection m_CSPackets;
	PacketList       m_PendingNrmSendPackets;
	PacketList       m_PendingLowSendPackets;

	cCriticalSection m_CSChunkLists;
	cChunkCoordsList m_LoadedChunks;  // Chunks that the player belongs to
	cChunkCoordsList m_ChunksToSend;  // Chunks that need to be sent to the player (queued because they weren't generated yet or there's not enough time to send them)

	cSocket m_Socket;

	cCriticalSection m_CriticalSection;

	Vector3d m_ConfirmPosition;

	cPacket * m_PacketMap[256];

	bool      m_bDestroyed;
	cPlayer * m_Player;
	bool      m_bKicking;
	
	// Chunk position when the last StreamChunks() was called; used to avoid re-streaming while in the same chunk
	int m_LastStreamedChunkX;
	int m_LastStreamedChunkZ;

	float m_TimeLastPacket;
	
	short m_Ping;
	int   m_PingID;
	long long m_PingStartTime;
	long long m_LastPingTime;
	static const unsigned short PING_TIME_MS = 1000; //minecraft sends 1 per 20 ticks (1 second or every 1000 ms)

	enum eState
	{
		csConnected,         // The client has just connected, waiting for their handshake / login
		csAuthenticating,    // The client has logged in, waiting for external authentication
		csDownloadingWorld,  // The client is waiting for chunks, we're waiting for the loader to provide and send them
 		csConfirmingPos,     // The client has been sent the position packet, waiting for them to repeat the position back
		csPlaying,           // Normal gameplay
		
		// TODO: Add Kicking and Destroyed here as well
	} ;
	
	eState m_State;

	bool m_bKeepThreadGoing;

	void HandlePacket(cPacket * a_Packet);
	
	// Packets handled in csConnected:
	void HandlePing            (void);
	void HandleHandshake       (cPacket_Handshake *      a_Packet);
	void HandleLogin           (cPacket_Login *          a_Packet);
	void HandleUnexpectedPacket(cPacket *                a_Packet);  // the default case -> kick
	
	// Packets handled while in csConfirmingPos:
	void HandleMoveLookConfirm(cPacket_PlayerMoveLook * a_Packet);  // While !m_bPositionConfirmed
	
	// Packets handled while in csPlaying:
	void HandleCreativeInventory(cPacket_CreativeInventoryAction * a_Packet);
	void HandlePlayerPos        (cPacket_PlayerPosition *          a_Packet);
	void HandleBlockDig         (cPacket_BlockDig *                a_Packet);
	void HandleBlockPlace       (cPacket_BlockPlace *              a_Packet);
	void HandlePickupSpawn      (cPacket_PickupSpawn *             a_Packet);
	void HandleChat             (cPacket_Chat *                    a_Packet);
	void HandlePlayerLook       (cPacket_PlayerLook *              a_Packet);
	void HandlePlayerMoveLook   (cPacket_PlayerMoveLook *          a_Packet);  // While m_bPositionConfirmed (normal gameplay)
	void HandleAnimation        (cPacket_ArmAnim *                 a_Packet);
	void HandleItemSwitch       (cPacket_ItemSwitch *              a_Packet);
	void HandleWindowClose      (cPacket_WindowClose *             a_Packet);
	void HandleWindowClick      (cPacket_WindowClick *             a_Packet);
	void HandleUpdateSign       (cPacket_UpdateSign *              a_Packet);
	void HandleUseEntity        (cPacket_UseEntity *               a_Packet);
	void HandleRespawn          (void);
	void HandleDisconnect       (cPacket_Disconnect *              a_Packet);
	void HandleKeepAlive        (cPacket_KeepAlive *               a_Packet);

	/// Returns true if the rate block interactions is within a reasonable limit (bot protection)
	bool CheckBlockInteractionsRate(void);
	
	/// Checks whether all loaded chunks have been sent to the client; if so, sends the position to confirm
	void CheckIfWorldDownloaded(void);
	
	/// Sends the PlayerMoveLook packet that the client needs to reply to for the game to start
	void SendConfirmPosition(void);
	
	/// Adds a single chunk to be streamed to the client; used by StreamChunks()
	void StreamChunk(int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	// cSocketThreads::cCallback overrides:
	virtual void DataReceived   (const char * a_Data, int a_Size) override;  // Data is received from the client
	virtual void GetOutgoingData(AString & a_Data) override;  // Data can be sent to client
	virtual void SocketClosed   (void) override;  // The socket has been closed for any reason

	static int s_ClientCount;
	int m_UniqueID;
};										// tolua_export




#endif  // CCLIENTHANDLE_H_INCLUDED




