#include "stdafx.h"
#include "GBNRdtReceiver.h"
#include "Global.h"


GBNRdtReceiver::GBNRdtReceiver():
	seqSize(8)
{
	initState();
}

void GBNRdtReceiver::initState()
{
	expectedSeqNum = 0;
	lastAckPkt.acknum = 0; //初始状态下，上次发送的确认包的确认序号为0，使得当第一个接受的数据包出错时该确认报文的确认号为0
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::GBNRdtReceiver(int sSize) :
	seqSize(sSize)
{
	initState();
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(Packet & packet)
{
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{//数据包损坏，不作出应答
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//发送上次确认过的ack
		return;
	}
	if (packet.seqnum != expectedSeqNum)
	{//不是想要的数据包，不作出应答
		pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		return;
	}
	Message msg;
	memcpy(msg.data, packet.payload, sizeof(packet.payload));
	pns->delivertoAppLayer(RECEIVER, msg);
	lastAckPkt.acknum = packet.seqnum + 1; //确认序号等于收到的报文序号
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	pUtils->printPacket("接收方发送确认报文", lastAckPkt);
	pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	expectedSeqNum = (expectedSeqNum + 1) % seqSize;
}
