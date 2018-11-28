#pragma once

//定义窗口中元素的状态
enum SeqStateEnum
{
	CONFIRMED,					//已被确认
	AVAILABLE,					//可用，还未发送
	SENT,						//发送，还未确认
	UNAVAILABLE					//不可用
};