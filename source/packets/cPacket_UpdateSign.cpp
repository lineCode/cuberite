
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "cPacket_UpdateSign.h"





int cPacket_UpdateSign::Parse(cByteBuffer & a_Buffer)
{
	int TotalBytes = 0;
	HANDLE_PACKET_READ(ReadBEInt,           m_BlockX, TotalBytes);
	HANDLE_PACKET_READ(ReadBEShort,         m_BlockY, TotalBytes);
	HANDLE_PACKET_READ(ReadBEInt,           m_BlockZ, TotalBytes);
	HANDLE_PACKET_READ(ReadBEUTF16String16, m_Line1, TotalBytes);
	HANDLE_PACKET_READ(ReadBEUTF16String16, m_Line2, TotalBytes);
	HANDLE_PACKET_READ(ReadBEUTF16String16, m_Line3, TotalBytes);
	HANDLE_PACKET_READ(ReadBEUTF16String16, m_Line4, TotalBytes);
	return TotalBytes;
}





void cPacket_UpdateSign::Serialize(AString & a_Data) const
{
	AppendByte    (a_Data, m_PacketID);
	AppendInteger (a_Data, m_BlockX);
	AppendShort   (a_Data, m_BlockY);
	AppendInteger (a_Data, m_BlockZ);
	AppendString16(a_Data, m_Line1);
	AppendString16(a_Data, m_Line2);
	AppendString16(a_Data, m_Line3);
	AppendString16(a_Data, m_Line4);
}




