﻿#include "stdafx.h"
#include "CommandDef.h"
#include "AccountMsgHandler.h"
#include "CommonFunc.h"
#include "CommonEvent.h"
#include "PacketHeader.h"
#include "GameService.h"
#include "CommonSocket.h"
#include "../Message/Msg_ID.pb.h"
#include "../Message/Msg_RetCode.pb.h"
#include "../Message/Msg_Game.pb.h"
#include "Log.h"
#include "../ServerData/ServerDefine.h"




CAccountMsgHandler::CAccountMsgHandler()
{

}

CAccountMsgHandler::~CAccountMsgHandler()
{

}

BOOL CAccountMsgHandler::Init(UINT32 dwReserved)
{
	m_AccountManager.LoadCacheAccount();

	return TRUE;
}

BOOL CAccountMsgHandler::Uninit()
{
	m_AccountManager.Close();

	return TRUE;
}


BOOL CAccountMsgHandler::DispatchPacket(NetPacket* pNetPacket)
{
	switch(pNetPacket->m_dwMsgID)
	{
		default:
		{
			PROCESS_MESSAGE_ITEM(MSG_ACCOUNT_REG_REQ,	    OnMsgAccountRegReq);
			PROCESS_MESSAGE_ITEM(MSG_ACCOUNT_LOGIN_REQ,		OnMsgAccontLoginReq);
			PROCESS_MESSAGE_ITEM(MSG_SEAL_ACCOUNT_REQ,		OnMsgSealAccountReq);
		}
		break;
	}


	return TRUE;
}


BOOL CAccountMsgHandler::OnMsgAccountRegReq(NetPacket* pPacket)
{
	AccountRegReq Req;

	Req.ParsePartialFromArray(pPacket->m_pDataBuffer->GetData(), pPacket->m_pDataBuffer->GetBodyLenth());

	PacketHeader* pHeader = (PacketHeader*) pPacket->m_pDataBuffer->GetBuffer();

	ERROR_RETURN_TRUE(pHeader->dwUserData != 0);

	AccountRegAck Ack;

	CAccountObject* pAccount = m_AccountManager.GetAccountObjectByName(Req.accountname());
	if(pAccount != NULL)
	{
		Ack.set_retcode(MRC_ACCOUNT_EXIST);
		ServiceBase::GetInstancePtr()->SendMsgProtoBuf(pPacket->m_dwConnID, MSG_ACCOUNT_REG_ACK, 0, pHeader->dwUserData, Ack);
		CLog::GetInstancePtr()->LogError("Error CAccountMsgHandler::OnMsgAccountRegReq RetCode:MRC_ACCOUNT_EXIST");
		return TRUE;
	}

	UINT64 u64ID = 0;
	UINT32 dwChannel = 0;
	std::string strPwd;
	pAccount = m_AccountManager.CreateAccountObject(Req.accountname().c_str(), Req.password().c_str(), Req.channel());
	if(pAccount == NULL)
	{
		Ack.set_retcode(MRC_FAILED);
		CLog::GetInstancePtr()->LogError("Error CAccountMsgHandler::OnMsgAccountRegReq RetCode:MRC_FAILED");
	}
	else
	{
		Ack.set_retcode(MRC_SUCCESSED);
		Ack.set_accountid(pAccount->m_ID);
	}

	ServiceBase::GetInstancePtr()->SendMsgProtoBuf(pPacket->m_dwConnID, MSG_ACCOUNT_REG_ACK, 0, pHeader->dwUserData, Ack);

	return TRUE;
}

BOOL CAccountMsgHandler::OnMsgAccontLoginReq(NetPacket* pPacket)
{
	AccountLoginReq Req;
	Req.ParsePartialFromArray(pPacket->m_pDataBuffer->GetData(), pPacket->m_pDataBuffer->GetBodyLenth());

	PacketHeader* pHeader = (PacketHeader*) pPacket->m_pDataBuffer->GetBuffer();
	ERROR_RETURN_TRUE(pHeader->dwUserData != 0);

	AccountLoginAck Ack;
	//由于实际运营中存在不同渠道的accountname一样的情况，所有需要增加渠道参数
	//CAccountObject* pAccObj = m_AccountManager.GetAccountObject(Req.accountname(), Req.channel());

	CAccountObject* pAccObj = m_AccountManager.GetAccountObjectByName(Req.accountname());
	if(pAccObj != NULL)
	{
		ERROR_RETURN_FALSE(pAccObj->m_ID != 0);
		if(Req.password() == pAccObj->m_strPassword)
		{
			Ack.set_retcode(MRC_SUCCESSED);
			Ack.set_lastsvrid(pAccObj->m_dwLastSvrID);
			Ack.set_accountid(pAccObj->m_ID);
		}
	}
	else
	{
		Ack.set_retcode(MRC_FAILED);
		Ack.set_lastsvrid(0);
		Ack.set_accountid(0);
	}

	ServiceBase::GetInstancePtr()->SendMsgProtoBuf(pPacket->m_dwConnID, MSG_ACCOUNT_LOGIN_ACK, 0, pHeader->dwUserData, Ack);

	return TRUE;
}


BOOL CAccountMsgHandler::OnMsgSealAccountReq(NetPacket* pPacket)
{
	SealAccountReq Req;
	Req.ParsePartialFromArray(pPacket->m_pDataBuffer->GetData(), pPacket->m_pDataBuffer->GetBodyLenth());
	PacketHeader* pHeader = (PacketHeader*) pPacket->m_pDataBuffer->GetBuffer();
	ERROR_RETURN_TRUE(pHeader->dwUserData != 0);

	SealAccountAck Ack;
	CAccountObject* pAccObj = NULL;
	if(Req.accountid() == 0)
	{
		pAccObj = m_AccountManager.GetAccountObjectByName(Req.accountname());
	}
	else
	{
		pAccObj = m_AccountManager.GetAccountObjectByID(Req.accountid());
	}

	if(pAccObj != NULL)
	{
		if(Req.seal())
		{
			pAccObj->m_SealStatue = SS_NO;
			pAccObj->m_SealTime = CommonFunc::GetCurrTime() + Req.sealtime();
		}
		else
		{
			pAccObj->m_SealStatue = SS_OK;
		}

		Ack.set_retcode(MRC_SUCCESSED);
	}
	else
	{
		Ack.set_retcode(MRC_FAILED);
	}

	ServiceBase::GetInstancePtr()->SendMsgProtoBuf(pPacket->m_dwConnID, MSG_SEAL_ACCOUNT_ACK, 0, pHeader->dwUserData, Ack);

	return TRUE;
}