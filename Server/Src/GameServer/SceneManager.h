﻿#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
#include "Scene.h"
//#include "AVLTree.h"

//typedef AVLTree<UINT32, CScene*> SceneMap;
typedef Hash_Map<UINT32, CScene*>  SceneMap;

class CSceneManager
{
public:
	CSceneManager();

	~CSceneManager();

	BOOL		Init(BOOL bMainLand);

	BOOL		Uninit();
public:
	BOOL		CreateScene(UINT32 dwCopyID, UINT32 dwCopyGuid, UINT32 dwCopyType, UINT32 dwPlayerNum);

	CScene*		GetSceneByCopyGuid(UINT32 dwCopyGuid);

	BOOL		LoadMainScene();

	BOOL        DispatchPacket(NetPacket* pNetPack);

	BOOL		OnUpdate( UINT32 dwTick );

	UINT32      MakeCopyID();

	BOOL SendCityReport();

	//*********************消息处理定义开始******************************
public:
	BOOL OnMsgCreateSceneReq(NetPacket* pNetPack);


	//*********************消息处理定义结束******************************

protected:
	SceneMap	m_mapSceneList;
	UINT32      m_MaxCopyBaseID;
};

#endif //_SCENE_MANAGER_H_
