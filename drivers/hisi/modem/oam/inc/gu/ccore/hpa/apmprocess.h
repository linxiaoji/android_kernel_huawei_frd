

#ifndef __APMPROCESS_H__
#define __APMPROCESS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define APM_LDF_LOAD_PHY_DELAY_TIME         (15)   /* 上移DSP时加载DSP需要延时的时间 400ms */

/* 芯片地址定义 */
#if (VOS_WIN32 != VOS_OS_VER)
extern VOS_UINT_PTR                         g_ulPamBbpCommBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpCommFeBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpCommOnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpComm1BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm0BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm0OnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm1BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm1OnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm2BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpGsm2OnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpWcdma0BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpWcdma0OnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpWcdma1BaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpWcdma1OnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpCdmaBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpCdmaOnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbp2CdmaOnBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpDbgTrigBaseAddr;

extern VOS_UINT_PTR                         g_ulPamBbpDbgClkBaseAddr;

#define SOC_BBP_COMM0_BASE_ADDR             (g_ulPamBbpCommBaseAddr)

#define SOC_BBP_COMM_FE_BASE_ADDR           (g_ulPamBbpCommFeBaseAddr)

#define SOC_BBP_COMM_ON_BASE_ADDR           (g_ulPamBbpCommOnBaseAddr)

#define SOC_BBP_COMM1_BASE_ADDR             (g_ulPamBbpComm1BaseAddr)

#define SOC_BBP_GSM0_BASE_ADDR              (g_ulPamBbpGsm0BaseAddr)

#define SOC_BBP_GSM0_ON_BASE_ADDR           (g_ulPamBbpGsm0OnBaseAddr)

#define SOC_BBP_GSM1_BASE_ADDR              (g_ulPamBbpGsm1BaseAddr)

#define SOC_BBP_GSM1_ON_BASE_ADDR           (g_ulPamBbpGsm1OnBaseAddr)

#define SOC_BBP_GSM2_BASE_ADDR              (g_ulPamBbpGsm2BaseAddr)

#define SOC_BBP_GSM2_ON_BASE_ADDR           (g_ulPamBbpGsm2OnBaseAddr)

#define SOC_BBP_WCDMA0_BASE_ADDR            (g_ulPamBbpWcdma0BaseAddr)

#define SOC_BBP_WCDMA0_ON_BASE_ADDR         (g_ulPamBbpWcdma0OnBaseAddr)

#define SOC_BBP_WCDMA1_BASE_ADDR            (g_ulPamBbpWcdma1BaseAddr)

#define SOC_BBP_WCDMA1_ON_BASE_ADDR         (g_ulPamBbpWcdma1OnBaseAddr)

#define SOC_BBP_CDMA_BASE_ADDR              (g_ulPamBbpCdmaBaseAddr)

#define SOC_BBP_CDMA_ON_BASE_ADDR           (g_ulPamBbp2CdmaOnBaseAddr)

#define SOC_BBP_DBG_TRIG_BASE_ADDR          (g_ulPamBbpDbgTrigBaseAddr)

#define SOC_BBP_SCTRL_BASE_ADDR             (g_ulPamBbpDbgClkBaseAddr)
#else
extern VOS_UINT_PTR                         g_ulCphySramAddr;

#define SOC_BBP_DBG_TRIG_BASE_ADDR          (0)

#define SOC_BBP_SCTRL_BASE_ADDR             (0)

#define CPHY_SRAM_ADDR                      (g_ulCphySramAddr)
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/


#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of apmprocess.h */
