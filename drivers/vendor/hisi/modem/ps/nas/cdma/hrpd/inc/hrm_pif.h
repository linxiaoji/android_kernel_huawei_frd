/** ****************************************************************************

                    Huawei Technologies Sweden AB (C), 2001-2016

 ********************************************************************************
 * @author    Automatically generated by DAISY
 * @version
 * @date      2016-02-25 09:40:09
 * @file
 * @brief
 * @copyright Huawei Technologies Sweden AB
 *******************************************************************************/
#ifndef HRM_PIF_H
#define HRM_PIF_H

/*******************************************************************************
 1. Other files included
*******************************************************************************/

#include "vos.h"
#include "cttf_hrpd_sig_public_pif.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */
#pragma pack(4)

/*******************************************************************************
 2. Macro definitions
*******************************************************************************/


/*******************************************************************************
 3. Enumerations declarations
*******************************************************************************/

/** ****************************************************************************
 * Name        : CNAS_HRM_DATA_REQ_TYPE_ENUM_UINT32
 *
 * Description :
 *******************************************************************************/
enum CNAS_HRM_DATA_REQ_TYPE_ENUM
{
    CNAS_HRM_DATA_REQ_UATI_REQUEST                          = 0x00000000,
    CNAS_HRM_DATA_REQ_UATI_CMPL                             = 0x00000001,
    CNAS_HRM_DATA_REQ_HARDWARE_ID                           = 0x00000002,
    CNAS_HRM_DATA_REQ_MO_KEEP_ALIVE                         = 0x00000003,
    CNAS_HRM_DATA_REQ_MT_KEEP_ALIVE                         = 0x00000004,
    CNAS_HRM_DATA_REQ_SESSION_CLOSE                         = 0x00000005,
    CNAS_HRM_DATA_REQ_LOC_NTF                               = 0x00000006,       /* 收到网络请求，发给网侧的LU notification */
    CNAS_HRM_DATA_REQ_HANDOFF_LOC_NTF                       = 0x00000007,       /* 收到1X HANDOFF HRPD消息后，主动发给网侧的LU notification */
    CNAS_HRM_DATA_REQ_LOC_CMPL                              = 0x00000008,
    CNAS_HRM_DATA_REQ_BLOB_NTF                              = 0x00000009,
    CNAS_HRM_DATA_REQ_BLOB_CMPL                             = 0x0000000A,
    CNAS_HRM_DATA_REQ_TYPE_ENUM_BUTT                        = 0x0000000B
};
typedef VOS_UINT32 CNAS_HRM_DATA_REQ_TYPE_ENUM_UINT32;

/*******************************************************************************
 4. Message Header declaration
*******************************************************************************/

/*******************************************************************************
 5. Message declaration
*******************************************************************************/

/*******************************************************************************
 6. STRUCT and UNION declaration
*******************************************************************************/

/** ****************************************************************************
 * Name        : CNAS_HRM_DATA_REQ_COMM_CONTENT_STRU
 *
 * Description :
 *******************************************************************************/
typedef struct
{
    CTTF_HRPD_PROTOCOL_TYPE_ENUM_UINT16                     enProtocolType; /**< protocol type */
    CTTF_HRPD_INSTANCE_TYPE_ENUM_UINT8                      enInstanceType; /**< InConfiguration or InUse instance */
    CTTF_HRPD_ADDRESS_TYPE_ENUM_UINT8                       enAddressType;  /**< addressing modes: broadcast, multicast, unicast */
    CTTF_HRPD_CHANNEL_TYPE_ENUM_UINT8                       enChannelType;  /**< channel type */
    CTTF_HRPD_DELLIVERY_TYPE_ENUM_UINT8                     enDeliveryType; /**< SLP delivery type */
    VOS_UINT8                                               ucPriority;     /**< a number between 0 and 255 */
    CTTF_HRPD_BEST_EFFORT_NEED_RSLT_ENUM_UINT8              enNeedRslt;
    SNPITF_MSG_DATA_STRU                                    stMsgData;      /**< message data struct */
} CNAS_HRM_DATA_REQ_COMM_CONTENT_STRU;

/*******************************************************************************
 7. OTHER declarations
*******************************************************************************/

/*******************************************************************************
 8. Global  declaration
*******************************************************************************/

/*******************************************************************************
 9. Function declarations
*******************************************************************************/

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif
