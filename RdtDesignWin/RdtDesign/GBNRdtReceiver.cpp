#include "stdafx.h"
#include "GBNRdtReceiver.h"
#include "Global.h"


GBNRdtReceiver::GBNRdtReceiver():
	seqsize(8)
{
	Init();
}

void GBNRdtReceiver::Init()
{
	expectedSeqNum = 0;
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为0，使得当第一个接受的数据包出错时该确认报文的确认号为0
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	memset(lastAckPkt.payload, '.', Configuration::PAYLOAD_SIZE);
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::GBNRdtReceiver(int sSize) :
	seqsize(sSize)
{
	Init();
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(Packet & packet)
{
	if (packet.checksum != pUtils->calculateCheckSum(packet))
	{//数据包损坏，不作出应答
		pUtils->printPacket("[Debug]接收方没有正确收到发送方的报文,数据校验错误", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//发送上次的
	}
	if (packet.seqnum != expectedSeqNum)
	{//不是想要的数据包，不作出应答
		pUtils->printPacket("[Debug]不是期望的数据分组", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);//发送上次的
	}
	else
	{
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		pUtils->printPacket("[Debug]接收方正确并发送到上层APP：", packet);
		//std::cout << "递交给应用数据分组：" << packet.seqnum << "\n\n";
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("[Debug]接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		expectedSeqNum = (expectedSeqNum + 1) % seqsize;//别忘了取模
	}
	std::cout << "\n[RECEIVER]确认号ack：" << lastAckPkt.acknum << "\n\n";
}
