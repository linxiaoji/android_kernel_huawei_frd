

#ifndef __CNAS_XSD_PROC_NVIM_H__
#define __CNAS_XSD_PROC_NVIM_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "CnasXsdCtx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

VOS_VOID CNAS_XSD_ReadMruNvim(VOS_VOID);


VOS_VOID CNAS_XSD_WriteMruNvim(
    CNAS_XSD_MRU_LIST_STRU             *pstMruList
);

VOS_VOID CNAS_XSD_ReadPrlNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadBandClassNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadHomeSidNidNvim(VOS_VOID);

VOS_VOID CNAS_XSD_Read1xOosSysAcqStrategyCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadTestConfigNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadAddAvoidListCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadNegPrefSysCmpCtrlNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadCallBackCfgNvim( VOS_VOID );

VOS_VOID CNAS_XSD_ReadCdmaStandardChannelsNvim(VOS_VOID);

/* CNAS_XSD_ReadHomeSidNidDependOnCfgNvim */

VOS_VOID CNAS_XSD_ReadOperLockSysWhiteListInfoNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadCTCCCustomizeFreqNvim(VOS_VOID);

VOS_UINT32 CNAS_XSD_WriteOperLockWhiteSidListInfoNvim(
    MSCC_XSD_OPER_LOCK_SYS_WHITE_STRU  *pstOPerLockWhiteSid
);

VOS_VOID CNAS_XSD_ReadCdma1XCustomPrefChannelsNvim( VOS_VOID );

VOS_VOID CNAS_XSD_WriteCdma1XPrefChannelsNvim(
    CNAS_PRL_FREQENCY_CHANNEL_STRU     *pstFreq
);

VOS_VOID CNAS_XSD_Read1xSupportBandClassMaskNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadNoCardModeCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadEmcRedialSysAcqCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_Read1xAvoidScheduleInfoNvim(VOS_VOID);

VOS_VOID CNAS_XSD_Read1xPowerOffCampOnCtrlFlgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadMru0SwitchOocStrategyCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_Read1xPRLRoamIndStrategyNvim(VOS_VOID);

VOS_VOID CNAS_XSD_Read1xDoSysAcqNoRfProtectTimerCfgNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadLteOrDoConn1xSysAcqSyncDelayInfoNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadCbtPrlNvim(VOS_VOID);

VOS_VOID CNAS_XSD_ReadSrlteSyncOneChanAddTimerLen(VOS_VOID);
#endif


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of CnasXsdProcNvim.h */