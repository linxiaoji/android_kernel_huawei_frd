

/*****************************************************************************
  1 Header File Include
*****************************************************************************/
#include "CnasHsmFsmUatiRequest.h"
#include "nas_cttf_hrpd_rsnp_pif.h"
#include "nas_cttf_hrpd_rmac_ctrl_pif.h"
#include "CnasHsmCtx.h"
#include "CnasHsmEncode.h"
#include "CnasHsmDecode.h"
#include "CnasHsmFsmTbl.h"
#include "CnasHsmSndTtf.h"
#include "cttf_hrpd_rmac_data_status_pif.h"
#include "CnasHsmComFunc.h"
#include "CnasHsmSndInternalMsg.h"
#include "CnasHsmSndHsd.h"
#include "CnasHsmComFunc.h"
#include "CnasMntn.h"
#include "CnasHsmMntn.h"
#include "CnasHsmFsmMain.h"
#include "CnasHsmSndAs.h"
#include "CnasHsmProcNvim.h"
#include "CnasTimerMgmt.h"
#include "Nas_Mem.h"
#include "NasErrLogCcbCtx.h"
#include "CnasHsmSndRrm.h"
#include "CnasHsmSndHrm.h"

#ifdef  __cplusplus
#if  __cplusplus
extern "C"{
#endif
#endif

#define THIS_FILE_ID                    PS_FILE_ID_CNAS_HSM_FSM_UATI_REQUEST_C

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

/*****************************************************************************
  2 The Define Of The Gloabal Variable
*****************************************************************************/

CNAS_HSM_SCP_FAIL_REASON_MAP_UATI_FAIL_REASON_STRU g_astHsmScpFailReasonMapUatiFailReasonTbl[] =
{
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_FAILURE,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_NEG_FAIL,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_AN_EXPIRED,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_AN_EXP,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CONN_OPEN_FAIL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CONN_OPEN_FAIL,
    },

    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CONN_OPEN_DENY_GENERAL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CONN_OPEN_DENY,
    },

    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CONN_OPEN_DENY_NW_BUSY,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CONN_OPEN_DENY,
    },

    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CONN_OPEN_DENY_AUTH_BILL_FAIL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CONN_OPEN_DENY,
    },

    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CONN_OPEN_DENY_PREF_CHAN_NOT_AVAIL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CONN_OPEN_DENY,
    },

    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_PERSONALITY_INVALID,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_PERS_INVALID,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CFG_RSP_EXPIRED,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CFG_RSP_EXP,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_CFG_MSG_FAIL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CFG_MSG_FAIL,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_AN_GAUP_FAIL,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_CFG_AN_GAUP_FAIL,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_IRAT_TO_LTE,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_IRAT_TO_LTE,
    },
    {   CAS_CNAS_HRPD_SCP_NEG_RSLT_NO_RF,
        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SCP_NO_RF,
    },
};


/*****************************************************************************
  3 Function Define
*****************************************************************************/

VOS_UINT32 CNAS_HSM_RcvInterUatiReq_UATIRequest_Init(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_HSM_UATI_REQ_STRU                             *pstUatiReq;
    CNAS_HSM_L1_STA_ENUM_UINT32                             enMainState;
#if (FEATURE_PTM == FEATURE_ON)
    VOS_UINT32                                              ulUatiFailRecordIndex;
#endif

    pstUatiReq  = (CNAS_HSM_HSM_UATI_REQ_STRU *)pstMsg;

#if (FEATURE_PTM == FEATURE_ON)
    /* 先对ccb ctx初始化 */
    NAS_ERRLOG_CCB_InitHrpdUatiFailCtx();

    NAS_ERRLOG_CCB_SetHrpdUatiFailStartTime(VOS_GetSlice());
    NAS_ERRLOG_CCB_SetHrpdUatiFailTriggerCause(pstUatiReq->enTriggerReason);
#endif

    if (CNAS_HSM_UATI_REQ_TRIGGER_BY_AT == pstUatiReq->enTriggerReason)
    {
        if (CNAS_HSM_HRPD_CAS_STATUS_ENUM_CONN == CNAS_HSM_GetHrpdConvertedCasStatus())
        {
            enMainState          = CNAS_HSM_GetCurrMainState();

            if (CNAS_HSM_L1_STA_OPEN != enMainState)
            {
                 CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvInterUatiReq_UATIRequest_Init: main state is not open state when conn exist");
            }

            /* 如果AC信道不存在则直接退出状态机，等待网侧直接指派UATI assignment */
            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                    CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
        }
        else
        {
            /* 能够发起UATI流程肯定是有网络的，所以走到这个分支CAS的状态肯定为IDLE而肯定不会是INIT */
            CNAS_HSM_SndUatiReqMsg_UatiReq();

#if (FEATURE_PTM == FEATURE_ON)
            /* uati分配次数加1 */
            NAS_ERRLOG_CCB_InCreaseHrpdUatiFailRecordCnt();

            ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
            NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordStartTime(ulUatiFailRecordIndex, VOS_GetSlice());
            NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordIndex(ulUatiFailRecordIndex);
#endif

            CNAS_HSM_VoteExitSlottedMode(CNAS_HSM_SLOT_VOTE_TYPE_UATI_REQ_PROCEDURE);

            CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_SNP_DATA_CNF);
            CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF, TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);
        }

    }
    else
    {
        CNAS_HSM_SndUatiCmplMsg();

        CNAS_HSM_StartTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND,
                            TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND_LEN);

        CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_UATI_CMPL_SND_IND);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;

}


VOS_UINT32 CNAS_HSM_RcvHrmDataCnf_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_DATA_CNF_STRU                                  *pstSnpDataCnf;
    VOS_UINT16                                              usUatiReqOpId;
    CNAS_HSM_MNTN_LOG_DISCARD_MSG_REASON_UNION              unDiscardMsgReason;
    CNAS_HSM_L1_STA_ENUM_UINT32                             enMainState;
    CNAS_HSM_HRPD_CAS_STATUS_ENUM_UINT16                    enCovertedCasStatus;
#if (FEATURE_PTM == FEATURE_ON)
    VOS_UINT32                                              ulUatiFailRecordIndex;
#endif

    pstSnpDataCnf   = (HRM_HSM_DATA_CNF_STRU *)pstMsg;
    usUatiReqOpId   = CNAS_HSM_GetUatiReqSnpDataReqOpId();

    unDiscardMsgReason.stDiscardSnpDataCnfReason.enMainState    = CNAS_HSM_GetCurrMainState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.enSubState     = CNAS_HSM_GetCurrSubState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usRcvOpId      = pstSnpDataCnf->usOpId;
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usSndOpId      = usUatiReqOpId;

    /* If OpId not match, this message is not as expected, ignore it */
    if (usUatiReqOpId != pstSnpDataCnf->usOpId)
    {
        CNAS_HSM_LogDiscardMsgInfo(CNAS_HSM_MNTN_LOG_DISCARD_MSG_SCENE_ERROR_SNP_DATA_CNF, &unDiscardMsgReason);

        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCttfSnpDataCnf_UatiReq_WaitUatiReqSnpDataCnf: OpId not as expected");

        return CNAS_HSM_FSM_EVENT_NOT_HANDLED;
    }

    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    if (HRM_HSM_DATA_CNF_RSLT_SUCC == pstSnpDataCnf->enRslt)
    {

        CNAS_HSM_ResetUATIReqFailedCnt();

        CNAS_HSM_StartTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN, CNAS_HSM_GetWaitUatiAssignTimerLen_UatiReq());

        CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_UATI_ASSIGN);

#if (FEATURE_PTM == FEATURE_ON)
        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiRcvSnpDataCnfTimer(ulUatiFailRecordIndex, VOS_GetSlice());
#endif
    }
    /* 如果发送消息结果为资源被抢需要注册资源, 等待资源可用后再发起协商 */
    else if (HRM_HSM_DATA_CNF_RSLT_NO_RF == pstSnpDataCnf->enRslt)
    {
#if (FEATURE_PTM == FEATURE_ON)
        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordEndTime(ulUatiFailRecordIndex, VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordRslt(ulUatiFailRecordIndex, NAS_ERR_LOG_HRPD_UATI_FAIL_HRM_HSM_DATA_CNF_RSLT_NO_RF);
#endif
        CNAS_HSM_SndRrmRegisterInd(RRM_PS_TASK_TYPE_HRPD_UATI, VOS_RATMODE_HRPD);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_NO_RF,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_SNP_DATA_CNF_NO_RF);

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }
    else
    {
#if (FEATURE_PTM == FEATURE_ON)
        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordEndTime(ulUatiFailRecordIndex, VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordRslt(ulUatiFailRecordIndex, NAS_ERR_LOG_HRPD_UATI_FAIL_HRM_HSM_DATA_CNF_RSLT_FAIL);
#endif

        if (CTTF_HRPD_SNP_DATA_FAIL_REASON_AC_CHANNEL_NOT_EXIST == pstSnpDataCnf->enFailReason)
        {
            enMainState          = CNAS_HSM_GetCurrMainState();

            if (CNAS_HSM_L1_STA_OPEN != enMainState)
            {
                 CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCttfSnpDataCnf_UatiReq_WaitUatiReqSnpDataCnf: main state is not open state when conn exist");
            }

            /* 如果AC信道不存在则直接退出状态机，等待网侧直接指派UATI assignment */
            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                    CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        CNAS_HSM_IncreaseUATIReqFailedCnt();

        if (CNAS_HSM_GetUATIReqFailedCnt() >= CNAS_HSM_UAIT_REQ_FAILED_MAX_CNT)
        {
            /* 增加Session deactive原因值 */
            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                    CNAS_HSM_UATI_REQUEST_FAIL_REASON_REACH_MAX_CNT);
        }
        else
        {
            /* 这里增加保护,防止CAS消息时序的影响,如果之前丢网了这里不再重试,等待
               HSD重新驻留后重新进行协商 */

            enCovertedCasStatus = CNAS_HSM_GetHrpdConvertedCasStatus();

            if (CNAS_HSM_HRPD_CAS_STATUS_ENUM_INIT == enCovertedCasStatus)
            {
                CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                        CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);

                return CNAS_HSM_FSM_EVENT_HANDLED;
            }

            if (CNAS_HSM_HRPD_CAS_STATUS_ENUM_CONN == enCovertedCasStatus)
            {
                enMainState          = CNAS_HSM_GetCurrMainState();

                if (CNAS_HSM_L1_STA_OPEN != enMainState)
                {
                     CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvInterUatiReq_UATIRequest_Init: main state is not open state when conn exist");
                }

                /* 如果AC信道不存在则直接退出状态机，等待网侧直接指派UATI assignment */
                CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                        CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

                return CNAS_HSM_FSM_EVENT_HANDLED;
            }

            CNAS_HSM_SndUatiReqMsg_UatiReq();
            CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF, TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);
#if (FEATURE_PTM == FEATURE_ON)
            /* uati发起重分配，统计，uati分配次数加1 */
            NAS_ERRLOG_CCB_InCreaseHrpdUatiFailRecordCnt();

            ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
            NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordStartTime(ulUatiFailRecordIndex, VOS_GetSlice());
            NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordIndex(ulUatiFailRecordIndex);
#endif

        }
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvTiWaitSnpDataCnfTimerExpire_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
#if (FEATURE_PTM == FEATURE_ON)
    VOS_UINT32                          ulUatiFailRecordIndex;

    /*  先保存上报uati分配失败并触发重分配的原因 */
    ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();

    NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordRslt(ulUatiFailRecordIndex, NAS_ERR_LOG_HRPD_UATI_FAIL_WAIT_HRM_SNP_DATA_CNF_TIMER_EXPIRE);
    NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordEndTime(ulUatiFailRecordIndex, VOS_GetSlice());
#endif

    CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvTiWaitSnpDataCnfTimerExpire_UatiReq_WaitUatiReqSnpDataCnf: timer out");

    CNAS_HSM_IncreaseUATIReqFailedCnt();

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    if (CNAS_HSM_GetUATIReqFailedCnt() >= CNAS_HSM_UAIT_REQ_FAILED_MAX_CNT)
    {
        /* 增加Session deactive原因值 */
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_REACH_MAX_CNT);
    }
    else
    {
        /* 这里增加保护,防止CAS消息时序的影响,如果之前丢网了这里不再重试,等待
           HSD重新驻留后重新进行协商 */
        if (CNAS_HSM_HRPD_CAS_STATUS_ENUM_INIT == CNAS_HSM_GetHrpdConvertedCasStatus())
        {
            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);

            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        CNAS_HSM_SndUatiReqMsg_UatiReq();
        CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF, TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);

#if (FEATURE_PTM == FEATURE_ON)
        /* uati发起重分配，统计，uati分配次数加1 */
        NAS_ERRLOG_CCB_InCreaseHrpdUatiFailRecordCnt();

        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordStartTime(ulUatiFailRecordIndex, VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordIndex(ulUatiFailRecordIndex);
#endif
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvHsdPowerOffReq_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_OFF);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHrmDataInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_DATA_IND_STRU              *pstSnpDataInd;
    VOS_UINT32                          ulRslt;

    pstSnpDataInd = (HRM_HSM_DATA_IND_STRU *)pstMsg;

    ulRslt = CNAS_HSM_FSM_EVENT_NOT_HANDLED;

    if (CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL == pstSnpDataInd->enProtocolType)
    {
        ulRslt = CNAS_HSM_ProcAmpMsg_UATIRequest_WaitUATIAssign(pstSnpDataInd);
    }
    else
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCttfSnpDataInd_UatiReq_WaitUatiAssign: enProtocolType not as expected");
    }

    return ulRslt;
}


VOS_UINT32 CNAS_HSM_RcvTiWaitUATIAssignTimerExpire_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_L1_STA_ENUM_UINT32                             enMainState;
    VOS_UINT8                                               ucUatiReqMaxTimes;
    VOS_UINT8                                               ucUatiReqRetryTimesWhenUatiAssignTimerExpireInAmpOpen;
#if (FEATURE_PTM == FEATURE_ON)
    VOS_UINT32                          ulUatiFailRecordIndex;

    /*  先保存上报uati分配失败并触发重分配的原因 */
    ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();

    NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordRslt(ulUatiFailRecordIndex, NAS_ERR_LOG_HRPD_UATI_FAIL_WAIT_UATI_ASSIGN_TIMER_EXPIRE);
    NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordEndTime(ulUatiFailRecordIndex, VOS_GetSlice());
#endif

    CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvTiWaitUATIAssignTimerExpire_UatiReq_WaitUatiAssign: timer out");

    enMainState          = CNAS_HSM_GetCurrMainState();
    if  (CNAS_HSM_L1_STA_OPEN == enMainState)
    {
        ucUatiReqRetryTimesWhenUatiAssignTimerExpireInAmpOpen = CNAS_HSM_GetUatiReqRetryTimesWhenUatiAssignTimerExpireInAmpOpen();

        /* uati req最大发送次数应包含第一次发送 */
        ucUatiReqMaxTimes = ucUatiReqRetryTimesWhenUatiAssignTimerExpireInAmpOpen + 1;
    }
    else
    {
        ucUatiReqMaxTimes = CNAS_HSM_UATI_ASSIGN_TIMEOUT_MAX_CNT;
    }

    CNAS_HSM_IncreaseUATIAssignTimerExpiredCnt();

    if (CNAS_HSM_GetUATIAssignTimerExpiredCnt() >= ucUatiReqMaxTimes)
    {
        /* 增加Session deactive原因值 */
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_REACH_MAX_CNT);
    }
    else
    {

        /* 这里增加保护,防止CAS消息时序的影响,如果之前丢网了这里不再重试,等待
            HSD重新驻留后重新进行协商 */
        if (CNAS_HSM_HRPD_CAS_STATUS_ENUM_INIT == CNAS_HSM_GetHrpdConvertedCasStatus())
        {
            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);

            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        CNAS_HSM_ResetUATIReqFailedCnt();

        CNAS_HSM_SndUatiReqMsg_UatiReq();

        CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF, TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);

        CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_SNP_DATA_CNF);

#if (FEATURE_PTM == FEATURE_ON)
        /* uati发起重分配，统计，uati分配次数加1 */
        NAS_ERRLOG_CCB_InCreaseHrpdUatiFailRecordCnt();

        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordStartTime(ulUatiFailRecordIndex, VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordIndex(ulUatiFailRecordIndex);
#endif
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvHsdPowerOffReq_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_OFF);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}

#if 0

VOS_UINT32 CNAS_HSM_RcvCttfSnpDataCnf_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_HRPD_SNP_DATA_CNF_STRU                       *pstCttfSnpDataCnf;
    VOS_UINT16                                              usUatiCmplOpId;
    CNAS_HSM_MNTN_LOG_DISCARD_MSG_REASON_UNION              unDiscardMsgReason;

    pstCttfSnpDataCnf   = (CTTF_CNAS_HRPD_SNP_DATA_CNF_STRU *)pstMsg;
    usUatiCmplOpId      = CNAS_HSM_GetUatiCmplSnpDataReqOpId();

    unDiscardMsgReason.stDiscardSnpDataCnfReason.enMainState    = CNAS_HSM_GetCurrMainState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.enSubState     = CNAS_HSM_GetCurrSubState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usRcvOpId      = pstCttfSnpDataCnf->usOpId;
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usSndOpId      = usUatiCmplOpId;

    if (usUatiCmplOpId == pstCttfSnpDataCnf->usOpId)
    {
        CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
        CNAS_HSM_LogDiscardMsgInfo(CNAS_HSM_MNTN_LOG_DISCARD_MSG_SCENE_ERROR_SNP_DATA_CNF, &unDiscardMsgReason);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvTiWaitSnpDataCnfTimerExpire_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvTiWaitSnpDataCnfTimerExpire_UatiReq_WaitUatiCmplSnpDataCnf: timer out");

    /* UATIComlete信令的SNP_DATA_CNF保护定时器超时，按照成功处理 */
    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerOffReq_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    /* UATI分配等待complete，SETUP下，按照失败处理，
        OPEN下，按照成功处理，此时退出后不需要发起deactive流程 */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_OFF);
    }
    else
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}
#endif


VOS_UINT32 CNAS_HSM_RcvHsdSuspendInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    if (CNAS_HSM_L1_STA_OPEN == CNAS_HSM_GetCurrMainState())
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SUSPEND);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerSaveReq_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_SAVE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdSuspendInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    if (CNAS_HSM_L1_STA_OPEN == CNAS_HSM_GetCurrMainState())
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SUSPEND);

    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerSaveReq_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_SAVE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}

#if 0

VOS_UINT32 CNAS_HSM_RcvHsdSuspendInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    /* Because Setup State have no valid session, and the UATI is cannot be used after quit Slave State,
       So we consider this procedure is fail though we have rcv valid and fresh UATIAssignment */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SUSPEND);
    }
    else
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerSaveReq_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    /* Because Setup State have no valid session, and the UATI is cannot be used after quit Slave State,
       So we consider this procedure is fail though we have rcv valid and fresh UATIAssignment */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_SAVE);
    }
    else
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}
#endif


VOS_UINT32 CNAS_HSM_RcvHsdOverHeadMsgInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_UATI_INFO_STRU            *pstUatiInfo;
    CNAS_HSM_HRPD_SYS_INFO_STRU        *pstHprdSysInfo;
    VOS_UINT8                           ucQuotient;
    VOS_UINT8                           ucRemainder;
    VOS_UINT8                           ucLoop;
    VOS_UINT8                           ucLeftMask;

    pstHprdSysInfo    = CNAS_HSM_GetHrpdSysInfoAddr();
    pstUatiInfo       = CNAS_HSM_GetUatiInfoAddr();

    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    /* C.S0024-A_v3.0 7.3.7.1.6.1 the access terminal shall disable T_ADMPATResponse if following conditions is true:
        1:the UATISubnetMask is equal to the SubnetMask of the sector in the active set, and
        2:the result of bitwise logical AND of the UATI and its subnet mask specified by
          UATISubnetMask is the same from the result of bitwise logical AND of SectorID and
          its subnet mask specified by SubnetMask(where SectorID and SubnetMask correspond to
          the sector in the active set) */

    /* Condition 1: */
    if (pstHprdSysInfo->ucSubNetMask == pstUatiInfo->ucUATISubnetMask)
    {
        /* Condition 2: */
        ucQuotient  = (pstHprdSysInfo->ucSubNetMask) / CNAS_HSM_BIT_LEN_PER_BYTE;
        ucRemainder = (pstHprdSysInfo->ucSubNetMask) % CNAS_HSM_BIT_LEN_PER_BYTE;

        for (ucLoop = 0; ucLoop < ucQuotient; ucLoop++)
        {
            /* pstUatiInfo->aucCurUATI[ucLoop] & 0xFF is equal to pstUatiInfo->aucCurUATI[ucLoop] */
            if ((pstUatiInfo->aucCurUATI[ucLoop])
             != (pstHprdSysInfo->aucSectorId[ucLoop]))
            {
                return CNAS_HSM_FSM_EVENT_HANDLED;
            }
        }

        if (0 == ucRemainder)
        {
           CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

            /* 回到原小区, 停止uati过程, 取消uati req消息发送, 释放资源 */
            CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

           CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                   CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

           return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        ucLeftMask = (VOS_UINT8)(((VOS_UINT32)1) << ucRemainder);
        ucLeftMask -= 1;
        ucLeftMask = (VOS_UINT8)(((VOS_UINT32)ucLeftMask) << (CNAS_HSM_BIT_LEN_PER_BYTE - ucRemainder));

        if ((pstUatiInfo->aucCurUATI[ucLoop] & ucLeftMask)
         != (pstHprdSysInfo->aucSectorId[ucLoop] & ucLeftMask))
        {
            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

        /* 回到原小区, 停止uati过程, 取消uati req消息发送, 释放资源 */
        CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

        return CNAS_HSM_FSM_EVENT_HANDLED;

    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdOverHeadMsgInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    /********************************************************************************************************************
     修改背景:AT驻留在A小区，IDHO到B小区后，开始UATI更新流程，在B小区成功发送UATI req，但是未等到B小区的UATI assignment，
              此时AT又从B小区切换到了C小区，此时AT需要立即在C小区执行UATI更新流程。
     *********************************************************************************************************************/
    if (VOS_FALSE == CNAS_HSM_IsSectorIdEqual(CNAS_HSM_GetSessionCtrlInfoAddr()->aucSectorIdOfLastUatiReq,
                                               CNAS_HSM_GetHrpdSysInfoAddr()->aucSectorId,
                                               CNAS_HSM_SUBNET_ID_MAX_LENGTH))
    {
        if (VOS_TRUE == CNAS_HSM_IsNeedSndUatiReq())
        {
            CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

            NAS_MEM_CPY_S(CNAS_HSM_GetSessionCtrlInfoAddr()->aucSectorIdOfLastUatiReq,
                          CNAS_HSM_SUBNET_ID_MAX_LENGTH,
                          CNAS_HSM_GetHrpdSysInfoAddr()->aucSectorId,
                          CNAS_HSM_SUBNET_ID_MAX_LENGTH);

            CNAS_HSM_SndUatiReqMsg_UatiReq();

            CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_SNP_DATA_CNF);
            CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF, TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);
        }
        else
        {
            CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

            CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                   CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
        }
    }

   return CNAS_HSM_FSM_EVENT_HANDLED;
}

#if 0

VOS_UINT32 CNAS_HSM_RcvHsdOverHeadMsgInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}
#endif


VOS_VOID CNAS_HSM_SndUatiReqMsg_UatiReq(VOS_VOID)
{
    VOS_UINT8                          *pucUATIReqMsgBuf = VOS_NULL_PTR;
    VOS_UINT16                          usMsgBufBitLen;
    VOS_UINT16                          usMsgBufByteLen;
    VOS_UINT8                           ucTransId;
    CNAS_HSM_DATA_REQ_HEADER_STRU       stDataReqHeader;
    VOS_UINT16                          usOpId;

    CNAS_HSM_ATI_LIST_INFO_STRU        *pstPubRcvATIList;


    usMsgBufByteLen = CNAS_HSM_UATI_REQ_MSG_OCTET_LEN;

    /* first alloc UATI request message buffer */
    pucUATIReqMsgBuf = (VOS_UINT8 *)PS_MEM_ALLOC(UEPS_PID_HSM, usMsgBufByteLen);

    if (VOS_NULL_PTR == pucUATIReqMsgBuf)
    {
        return;
    }

    ucTransId = CNAS_HSM_GetUATITransId();
    CNAS_HSM_IncreaseUATITransId();

    NAS_MEM_SET_S(pucUATIReqMsgBuf, usMsgBufByteLen, 0, usMsgBufByteLen);

    /* Encode UATIRequest message */
    usMsgBufBitLen   = 0;
    CNAS_HSM_EncodeUATIReq((VOS_VOID *)&ucTransId,
                            &usMsgBufBitLen,
                            pucUATIReqMsgBuf);

    if (0 == usMsgBufBitLen)
    {
        CNAS_HSM_LogCodecResultInd(ID_CNAS_HSM_MNTN_LOG_ENCODE_RESULT_IND, CNAS_HSM_ENCODE_FAILURE);

        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_SndUatiReqMsg_UatiReq: After encode usMsgBitLen is 0");

        PS_MEM_FREE(UEPS_PID_HSM, pucUATIReqMsgBuf);

        return;
    }

    /* 由hrm再消息发送后打印 */

    NAS_MEM_SET_S(&stDataReqHeader, sizeof(stDataReqHeader), 0, sizeof(stDataReqHeader));


    /* C.S0024-A_v3: 7.3.7.1.2 UATIAssignment Message Validation
    --------------------------------------------------------------------------------------------
    The access terminal shall initialize a receive pointer for the UATIAssignment message validation,
    V(R), to 255 when it sends a UATIRequest message and there exist an entry in the ReceiveATIList list
    whose ATIType field is equal to "11" (i.e., RATI).
    */

    pstPubRcvATIList    = CNAS_HSM_GetReceivedATIListAddr();

    if (CNAS_HSM_ATI_TYPE_RATI == pstPubRcvATIList->astATIEntry[1].enATIType)
    {
        /* Initialize receive pointer value for sequence number validation */
        CNAS_HSM_SetCurrUATIAssignMsgSeqNum(CNAS_HSM_UATI_ASSIGN_MSG_SEQ_NUM_INIT_VALUE);
    }

    /* fill hrpd snp date request header */
    CNAS_HSM_CFG_DATA_REQ_HEADER(&stDataReqHeader,
                                     CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL,
                                     CTTF_HRPD_ADDRESS_UNICAST,
                                     CTTF_HRPD_CHANNEL_AC,
                                     CTTF_HRPD_DELIVERY_TYPE_BEST_EFFORT,
                                     CNAS_HSM_MSG_PRIORITY_UATI_REQ,
                                     CTTF_HRPD_INSTANCE_INUSE,
                                     CNAS_HRM_DATA_REQ_UATI_REQUEST);

    CNAS_HSM_SndHrmDataReq(&stDataReqHeader,
                              &usOpId,
                              usMsgBufBitLen,
                              pucUATIReqMsgBuf);

    CNAS_HSM_SaveUatiReqSnpDataReqOpId(usOpId);

    PS_MEM_FREE(UEPS_PID_HSM, pucUATIReqMsgBuf);
}

#if 0

VOS_VOID CNAS_HSM_SndUatiCmplMsg_UatiReq(VOS_VOID)
{
    VOS_UINT8                                              *pucUATICmpMsgBuf = VOS_NULL_PTR;
    CNAS_HSM_UATI_INFO_STRU                                *pstGlobalUait    = VOS_NULL_PTR;
    VOS_UINT16                                              usMsgBufBitLen;
    VOS_UINT16                                              usMsgBufByteLen;
    CNAS_HSM_UATI_COMP_MSG_STRU                             stUaitCmpInput;
    CNAS_CTTF_HRPD_SNP_DATA_REQ_HEADER_STRU                 stSnpDataReqHeader;

    VOS_UINT16                                              usOpId;

    pstGlobalUait = &(CNAS_HSM_GetSessionCtrlInfoAddr()->stUATIInfo);

    /* first alloc uati complete message */
    usMsgBufByteLen = CNAS_HSM_GET_UATI_COMPLETE_MSG_OCTET_LEN(pstGlobalUait->ucOldUATILen);

    pucUATICmpMsgBuf = (VOS_UINT8 *)PS_MEM_ALLOC(UEPS_PID_HSM, usMsgBufByteLen);

    if (VOS_NULL_PTR == pucUATICmpMsgBuf)
    {
        return;
    }

    NAS_MEM_SET_S(pucUATICmpMsgBuf, 0, usMsgBufByteLen);

    NAS_MEM_SET_S(&stUaitCmpInput, 0, sizeof(stUaitCmpInput));

    /* Fullfill UATIComplte input paramters, C.S0024 7.3.7.2.3 UAIComplete */
    stUaitCmpInput.ucMsgSeq        = CNAS_HSM_GetCurrUATIAssignMsgSeqNum();
    stUaitCmpInput.ucOldUATILen    = pstGlobalUait->ucOldUATILen;

    NAS_MEM_CPY_S(stUaitCmpInput.aucOldUATI, pstGlobalUait->aucOldUATI, CNAS_HSM_UATI_OCTET_LENGTH);

    /* Encode UATIComplete message */
    usMsgBufBitLen = 0;
    CNAS_HSM_EncodeUATIComplete(&stUaitCmpInput,
                                &usMsgBufBitLen,
                                pucUATICmpMsgBuf);
    CNAS_MNTN_LogOmAirMsg(CNAS_HRPD_OM_AIR_MSG_UP,
                          NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_COMPLETE,
                          UEPS_PID_HSM,
                         (VOS_UINT32)usMsgBufBitLen,
                          pucUATICmpMsgBuf);

    if (0 == usMsgBufBitLen)
    {
        CNAS_HSM_LogCodecResultInd(ID_CNAS_HSM_MNTN_LOG_ENCODE_RESULT_IND, CNAS_HSM_ENCODE_FAILURE);
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_SndUatiCmplMsg_UatiReq: After encode usMsgBitLen is 0");

        PS_MEM_FREE(UEPS_PID_HSM, pucUATICmpMsgBuf);

        return;
    }

    NAS_MEM_SET_S(&stSnpDataReqHeader, 0, sizeof(stSnpDataReqHeader));

    /* fill hrpd snp date request header */
    CNAS_CFG_CTTF_HRPD_SNP_DATA_REQ_HEADER(&stSnpDataReqHeader,
                                           CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL,
                                           CTTF_HRPD_ADDRESS_UNICAST,
                                           CTTF_HRPD_CHANNEL_AC_OR_RTC,
                                           CTTF_HRPD_DELIVERY_TYPE_RELIABLE,
                                           CNAS_HSM_MSG_PRIORITY_UATI_COMP,
                                           CTTF_HRPD_INSTANCE_INUSE);

    CNAS_HSM_SndTtfHrpdSnpDataReq(&stSnpDataReqHeader,
                                  &usOpId,
                                  usMsgBufBitLen,
                                  pucUATICmpMsgBuf);

    CNAS_HSM_SaveUatiCmplSnpDataReqOpId(usOpId);

    PS_MEM_FREE(UEPS_PID_HSM, pucUATICmpMsgBuf);
}
#endif


VOS_VOID CNAS_HSM_QuitFsmUatiReq(
    CNAS_HSM_UATI_REQUEST_RESULT_ENUM_UINT8                 enRslt,
    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8            enFailReason
)
{
    /* 退状态机可以分为两种场景，一种是正常UATI申请成功，一种是UATI流程不成功(包括FAIL、CANCEL)
        成功退出场景: 会发送UATIComplete消息, 退出状态机的时候不需要发送session end notify
        不成功退出场景: UATI请求失败, 需要发送session end notify */
    if (CNAS_HSM_UATI_REQUEST_RESULT_SUCC != enRslt)
    {
        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

#if (FEATURE_PTM == FEATURE_ON)
        /* 记录失败信息 */
        NAS_ERRLOG_CCB_SetHrpdUatiFailEndTime(VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiFailCause(enFailReason);

        if (VOS_TRUE == CNAS_HSM_IsUatiFailReasonNeedReportErrLog(enFailReason))
        {
            /* Uati分配失败CHR信息上报 */
            CNAS_HSM_ErrLogReportUatiFail();
        }
#endif
    }

#if (FEATURE_PTM == FEATURE_ON)
    /* 不管失败还是成功，都需要清空CCB中保存的信息 */
    NAS_ERRLOG_CCB_InitHrpdUatiFailCtx();
#endif

    NAS_MEM_SET_S(CNAS_HSM_GetSessionCtrlInfoAddr()->aucSectorIdOfLastUatiReq,
                  sizeof(CNAS_HSM_GetSessionCtrlInfoAddr()->aucSectorIdOfLastUatiReq),
                  0,
                  CNAS_HSM_SUBNET_ID_MAX_LENGTH);

    CNAS_HSM_VoteEnterSlottedMode(CNAS_HSM_SLOT_VOTE_TYPE_UATI_REQ_PROCEDURE);

    /* reset UATI request context info */
    CNAS_HSM_InitUATIReqFsmCtx(CNAS_HSM_GetUATIRequestFsmCtxAddr());

    /* back to the initial substate */
    CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_VACANT);

    CNAS_HSM_SndInternalUATIRequestResultInd(enRslt, enFailReason);
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdNetworkLostInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    if (CNAS_HSM_L1_STA_OPEN == CNAS_HSM_GetCurrMainState())
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdNetworkLostInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    if (CNAS_HSM_L1_STA_OPEN == CNAS_HSM_GetCurrMainState())
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_CANCEL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
         CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                 CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}

#if 0

VOS_UINT32 CNAS_HSM_RcvCasHrpdNetworkLostInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }
    else
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCttfSnpDataInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CTTF_CNAS_HRPD_SNP_DATA_IND_STRU                       *pstSnpDataInd;
    CTTF_HRPD_PROTOCOL_TYPE_ENUM_UINT16                     enProtocolType;
    VOS_UINT8                                               ucMsgType;

    pstSnpDataInd  = (CTTF_CNAS_HRPD_SNP_DATA_IND_STRU*)pstMsg;

    enProtocolType = pstSnpDataInd->enProtocolType;

    CNAS_HSM_DecodeMsgType(pstSnpDataInd->enProtocolType,
                           pstSnpDataInd->stMsgData.aucMsgData,
                           &ucMsgType);

    if (CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL == enProtocolType)
    {
        if (CNAS_HSM_AMP_MSG_TYPE_UATI_ASSIGN == ucMsgType)
        {
            CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

            /* just cache the message */
            CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);
        }

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }
    else
    {
        return CNAS_HSM_FSM_EVENT_NOT_HANDLED;
    }
}


VOS_UINT32 CNAS_HSM_RcvHsmSessionCloseInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU                    *pstMsgInd;
    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8            enUatiFailReason;

    pstMsgInd = (CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU *)pstMsg;

    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    switch (pstMsgInd->enScene)
    {
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_KEEP_ALIVE_EXP_OPEN:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_KEEP_ALIVE_EXP;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_SETUP:
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_OPEN_NOT_SESSION_ACT:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_AN_CLOSE_SESSION;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_RCV_CAS_NEG_RSLD_IND:

            enUatiFailReason = CNAS_HSM_ConvertScpFailReasonToUatiFailReason(
                                                         pstMsgInd->stSessionNegRsltInfo.enResult);

            if (PS_TRUE == pstMsgInd->stSessionNegRsltInfo.enProtocolTypeVaildFlag)
            {
                CNAS_HSM_SetScpActFailProcType(pstMsgInd->stSessionNegRsltInfo.usProtocolType);
                CNAS_HSM_SetScpActFailProcSubtype(pstMsgInd->stSessionNegRsltInfo.usProtocolSubType);
            }

            break;

        default:

            return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL, enUatiFailReason);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdPageInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvApsDiscReq_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvEhsmDiscReq_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}
#endif


VOS_UINT32 CNAS_HSM_RcvHsmSessionCloseInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU                    *pstMsgInd;
    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8            enUatiFailReason;

    VOS_UINT32                                              ulHsmEventType;
    VOS_UINT8                                               ucIndex;

    /* 如果缓存中有OHM消息，则清除缓存中的OHM消息，因为后续在处理session close ind的时候会重新进行重试流程,
        此处如果不清理OHM消息，则在稳态处理缓存时可能向网侧发送两次session close */
    ulHsmEventType = CNAS_BuildEventType(UEPS_PID_HSD, (VOS_UINT16)ID_HSD_HSM_OVERHEAD_MSG_IND);

    ucIndex = CNAS_HSM_GetCacheIndexByEventType(ulHsmEventType);

    if (CNAS_HSM_INVAILD_CACHE_INDEX != ucIndex)
    {
        CNAS_HSM_ClearCacheMsgByIndex(ucIndex);
    }

    pstMsgInd = (CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU *)pstMsg;

    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    switch (pstMsgInd->enScene)
    {
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_KEEP_ALIVE_EXP_OPEN:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_KEEP_ALIVE_EXP;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_SETUP:
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_OPEN_NOT_SESSION_ACT:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_AN_CLOSE_SESSION;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_RCV_CAS_NEG_RSLD_IND:

            enUatiFailReason = CNAS_HSM_ConvertScpFailReasonToUatiFailReason(
                                                         pstMsgInd->stSessionNegRsltInfo.enResult);

            if (PS_TRUE == pstMsgInd->stSessionNegRsltInfo.enProtocolTypeVaildFlag)
            {
                CNAS_HSM_SetScpActFailProcType(pstMsgInd->stSessionNegRsltInfo.usProtocolType);
                CNAS_HSM_SetScpActFailProcSubtype(pstMsgInd->stSessionNegRsltInfo.usProtocolSubType);
            }

            break;

        default:

            return CNAS_HSM_FSM_EVENT_HANDLED;

    }

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL, enUatiFailReason);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsmSessionCloseInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU                    *pstMsgInd;
    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8            enUatiFailReason;

    VOS_UINT32                                              ulHsmEventType;
    VOS_UINT8                                               ucIndex;

    /* 如果缓存中有OHM消息，则清除缓存中的OHM消息，因为后续在处理session close ind的时候会重新进行UATI,
        此处如果不清理OHM消息，则在稳态处理缓存时可能向网侧发送两次session close */
    ulHsmEventType = CNAS_BuildEventType(UEPS_PID_HSD, (VOS_UINT16)ID_HSD_HSM_OVERHEAD_MSG_IND);

    ucIndex = CNAS_HSM_GetCacheIndexByEventType(ulHsmEventType);

    if (CNAS_HSM_INVAILD_CACHE_INDEX != ucIndex)
    {
        CNAS_HSM_ClearCacheMsgByIndex(ucIndex);
    }

    pstMsgInd = (CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU *)pstMsg;

    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    switch (pstMsgInd->enScene)
    {
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_KEEP_ALIVE_EXP_OPEN:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_KEEP_ALIVE_EXP;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_SETUP:
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_OPEN_NOT_SESSION_ACT:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_AN_CLOSE_SESSION;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_RCV_CAS_NEG_RSLD_IND:

            enUatiFailReason = CNAS_HSM_ConvertScpFailReasonToUatiFailReason(
                                                         pstMsgInd->stSessionNegRsltInfo.enResult);

            if (PS_TRUE == pstMsgInd->stSessionNegRsltInfo.enProtocolTypeVaildFlag)
            {
                CNAS_HSM_SetScpActFailProcType(pstMsgInd->stSessionNegRsltInfo.usProtocolType);
                CNAS_HSM_SetScpActFailProcSubtype(pstMsgInd->stSessionNegRsltInfo.usProtocolSubType);
            }

            break;

        default:

            return CNAS_HSM_FSM_EVENT_HANDLED;

    }

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL, enUatiFailReason);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdPageInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvApsDiscReq_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvEhsmDiscReq_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT8 CNAS_HSM_ProcAmpMsg_UATIRequest_WaitUATIAssign(
    HRM_HSM_DATA_IND_STRU              *pstSnpDataInd
)
{
    VOS_UINT8                                               ucMsgType;
    CNAS_HSM_UATI_ASSIGN_MSG_STRU                           stUatiAssignMsg;
    CNAS_HSM_DECODE_ERROR_TYPE_ENUM_UINT32                  ulDecodeRslt;
    VOS_UINT8                                               ucIsUatiAssignValidAndFresh;
#if (FEATURE_PTM == FEATURE_ON)
    VOS_UINT32                                              ulUatiFailRecordIndex;
#endif

    ucMsgType = CNAS_HSM_AMP_MESSAGE_TYPE_BUTT;

    ulDecodeRslt = CNAS_HSM_DecodeMsgType(pstSnpDataInd->enProtocolType,
                                          pstSnpDataInd->stMsgData.aucMsgData,
                                          &ucMsgType);

    if ((CNAS_HSM_DECODE_SUCCESS           != ulDecodeRslt)
     || (CNAS_HSM_AMP_MSG_TYPE_UATI_ASSIGN != ucMsgType))
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_ProcAmpMsgFromTtf_UATIRequest_WaitUATIAssign: Decode msg type fail or ucMsgType not as expected");

        return CNAS_HSM_FSM_EVENT_NOT_HANDLED;
    }

    NAS_MEM_SET_S(&stUatiAssignMsg, sizeof(stUatiAssignMsg), 0, sizeof(stUatiAssignMsg));

    ulDecodeRslt = CNAS_HSM_DecodeUATIAssignment(pstSnpDataInd->stMsgData.aucMsgData,
                                                 pstSnpDataInd->stMsgData.usMsgBitLen,
                                                 (VOS_VOID *)&stUatiAssignMsg);
    CNAS_MNTN_LogOmAirMsg(CNAS_HRPD_OM_AIR_MSG_DOWN,
                          NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_ASSIGN,
                          UEPS_PID_HSM,
                          (VOS_UINT32)pstSnpDataInd->stMsgData.usMsgBitLen,
                          pstSnpDataInd->stMsgData.aucMsgData);
    if (CNAS_HSM_DECODE_SUCCESS != ulDecodeRslt)
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_ProcAmpMsgFromTtf_UATIRequest_WaitUATIAssign: Decode msg content fail");

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    /* check the uati assign valid&fresh */
    ucIsUatiAssignValidAndFresh = CNAS_HSM_IsUatiAssignValidAndFresh(&stUatiAssignMsg);

    if (VOS_FALSE == ucIsUatiAssignValidAndFresh)
    {
        /* just discard the message and still wait uati assignment */
        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_HandleUatiAssignMsg(&stUatiAssignMsg);

    CNAS_HSM_SndUatiCmplMsg();

    CNAS_HSM_StartTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND,
                        TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND_LEN);

    CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_UATI_CMPL_SND_IND);


#if (FEATURE_PTM == FEATURE_ON)
        ulUatiFailRecordIndex = NAS_ERRLOG_CCB_CalcHrpdUatiAssignFailRecordIndex();
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordEndTime(ulUatiFailRecordIndex, VOS_GetSlice());
        NAS_ERRLOG_CCB_SetHrpdUatiAssignRecordRslt(ulUatiFailRecordIndex, NAS_ERR_LOG_HRPD_UATI_FAIL_HRM_HSM_DATA_CNF_RSLT_SUCC);
#endif

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvEhsmEhrpdNotAvailableInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetEhrpdAvailFlg(VOS_FALSE);

    /* just cache the message */
    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_EHRPD_NOT_AVAILABLE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasPaRatModeNtf_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU                     *pstPaRatModeNtf;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enLastSessionType;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enSessionType;
    CNAS_HSM_HSM_HRPD_PA_RAT_MODE_NTF_STRU                  stHsmHrpdPaRatModeNtf;
    VOS_UINT32                                              ulHsmEventType;

    enLastSessionType = CNAS_HSM_GetNegoSessionType();

    pstPaRatModeNtf = (CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU *)pstMsg;

    /* convert the session type */
    enSessionType = CNAS_HSM_ConvertPaModeToHsmSessionType(pstPaRatModeNtf->enPaMode);
    if (CNAS_HSM_SESSION_TYPE_BUTT == enSessionType)
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCasPaRatModeNtf_UATIRequest_WaitSnpDataCnf,receive the invalid session type--BUTT");

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    /* Last session type is eHRPD and new negotiated session type is HRPD
       need fallback
     */
    if ((CNAS_HSM_SESSION_TYPE_EHRPD == enLastSessionType)
     && (CNAS_HSM_SESSION_TYPE_HRPD == enSessionType))
    {

        /* 该状态收到ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF之后，进入该分支后，会执行deactive流程，
           后续进入inactive后，需要重新进行uati，session激活流程，因而需要缓存一条消息在inactive
           态处理。但是，ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF在各个状态下都可能收到，而在inactive态收
           到该消息，hsm应该丢弃该消息，所以需要将其先转换为一条内部消息，再进行缓存 */
        CNAS_HSM_BuildHsmHrpdPaRatModeNtfMsg(pstPaRatModeNtf, &stHsmHrpdPaRatModeNtf);

        ulHsmEventType = CNAS_BuildEventType(stHsmHrpdPaRatModeNtf.ulSenderPid, (VOS_UINT16)stHsmHrpdPaRatModeNtf.enMsgId);

        /* just cache the message */
        CNAS_HSM_SaveCacheMsg(ulHsmEventType, &stHsmHrpdPaRatModeNtf);

        CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

        CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);


        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_PA_MODE_NTF);

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

     /* store the negotiated session type */
    CNAS_HSM_SetNegoSessionType(enSessionType);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}




VOS_UINT32 CNAS_HSM_RcvEhsmEhrpdNotAvailableInd_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetEhrpdAvailFlg(VOS_FALSE);

    /* just cache the message */
    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_EHRPD_NOT_AVAILABLE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasPaRatModeNtf_UatiReq_WaitUatiAssign(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU                     *pstPaRatModeNtf;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enLastSessionType;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enSessionType;
    CNAS_HSM_HSM_HRPD_PA_RAT_MODE_NTF_STRU                  stHsmHrpdPaRatModeNtf;
    VOS_UINT32                                              ulHsmEventType;

    enLastSessionType = CNAS_HSM_GetNegoSessionType();

    pstPaRatModeNtf = (CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU *)pstMsg;

    /* convert the session type */
    enSessionType = CNAS_HSM_ConvertPaModeToHsmSessionType(pstPaRatModeNtf->enPaMode);
    if (CNAS_HSM_SESSION_TYPE_BUTT == enSessionType)
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCasPaRatModeNtf_UATIRequest_WaitUATIAssign,receive the invalid session type--BUTT");

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    /* Last session type is eHRPD and new negotiated session type is HRPD
       need fallback
     */
    if ((CNAS_HSM_SESSION_TYPE_EHRPD == enLastSessionType)
     && (CNAS_HSM_SESSION_TYPE_HRPD  == enSessionType))
    {

        /* 该状态收到ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF之后，进入该分支后，会执行deactive流程，
           后续进入inactive后，需要重新进行uati，session激活流程，因而需要缓存一条消息在inactive
           态处理。但是，ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF在各个状态下都可能收到，而在inactive态收
           到该消息，hsm应该丢弃该消息，所以需要将其先转换为一条内部消息，再进行缓存 */
        CNAS_HSM_BuildHsmHrpdPaRatModeNtfMsg(pstPaRatModeNtf, &stHsmHrpdPaRatModeNtf);

        ulHsmEventType = CNAS_BuildEventType(stHsmHrpdPaRatModeNtf.ulSenderPid, (VOS_UINT16)stHsmHrpdPaRatModeNtf.enMsgId);

        /* just cache the message */
        CNAS_HSM_SaveCacheMsg(ulHsmEventType, &stHsmHrpdPaRatModeNtf);

        CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_ASSIGN);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_PA_MODE_NTF);

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

     /* store the negotiated session type */
    CNAS_HSM_SetNegoSessionType(enSessionType);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}

#if 0

VOS_UINT32 CNAS_HSM_RcvEhsmEhrpdNotAvailableInd_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* just cache the message */
    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_EHRPD_NOT_AVAILABLE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasPaRatModeNtf_UatiReq_WaitUatiCmplSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU                     *pstPaRatModeNtf;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enLastSessionType;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enSessionType;
    CNAS_HSM_HSM_HRPD_PA_RAT_MODE_NTF_STRU                  stHsmHrpdPaRatModeNtf;
    VOS_UINT32                                              ulHsmEventType;

    enLastSessionType = CNAS_HSM_GetNegoSessionType();

    pstPaRatModeNtf = (CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU *)pstMsg;

    /* convert the session type */
    enSessionType = CNAS_HSM_ConvertPaModeToHsmSessionType(pstPaRatModeNtf->enPaMode);
    if (CNAS_HSM_SESSION_TYPE_BUTT == enSessionType)
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCasPaRatModeNtf_UATICmpl_WaitSnpDataCnf,receive the invalid session type--BUTT");

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    /* Last session type is eHRPD and new negotiated session type is HRPD
       need fallback
     */
    if ((CNAS_HSM_SESSION_TYPE_EHRPD == enLastSessionType)
     && (CNAS_HSM_SESSION_TYPE_HRPD == enSessionType))
    {

        /* 该状态收到ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF之后，进入该分支后，会执行deactive流程，
           后续进入inactive后，需要重新进行uati，session激活流程，因而需要缓存一条消息在inactive
           态处理。但是，ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF在各个状态下都可能收到，而在inactive态收
           到该消息，hsm应该丢弃该消息，所以需要将其先转换为一条内部消息，再进行缓存 */
        CNAS_HSM_BuildHsmHrpdPaRatModeNtfMsg(pstPaRatModeNtf, &stHsmHrpdPaRatModeNtf);

        ulHsmEventType = CNAS_BuildEventType(stHsmHrpdPaRatModeNtf.ulSenderPid, (VOS_UINT16)stHsmHrpdPaRatModeNtf.enMsgId);

        /* just cache the message */
        CNAS_HSM_SaveCacheMsg(ulHsmEventType, &stHsmHrpdPaRatModeNtf);

        CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_EHRPD_NOT_AVAILABLE);

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

     /* store the negotiated session type */
    CNAS_HSM_SetNegoSessionType(enSessionType);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}
#endif





VOS_UINT32 CNAS_HSM_RcvHrmHrpdSnpDataInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_DATA_IND_STRU                                  *pstSnpDataInd;
    VOS_UINT32                                              ulRslt;
    VOS_UINT8                                               ucMsgType;
    CNAS_HSM_UATI_ASSIGN_MSG_STRU                           stUatiAssignMsg;
    CNAS_HSM_DECODE_ERROR_TYPE_ENUM_UINT32                  ulDecodeRslt;
    VOS_UINT8                                               ucIsUatiAssignValidAndFresh;

    pstSnpDataInd = (HRM_HSM_DATA_IND_STRU *)pstMsg;

    ulRslt = CNAS_HSM_FSM_EVENT_NOT_HANDLED;

    if (CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL == pstSnpDataInd->enProtocolType)
    {
        ucMsgType = CNAS_HSM_AMP_MESSAGE_TYPE_BUTT;

        ulDecodeRslt = CNAS_HSM_DecodeMsgType(pstSnpDataInd->enProtocolType,
                                              pstSnpDataInd->stMsgData.aucMsgData,
                                              &ucMsgType);

        if ((CNAS_HSM_DECODE_SUCCESS           != ulDecodeRslt)
         || (CNAS_HSM_AMP_MSG_TYPE_UATI_ASSIGN != ucMsgType))
        {
            CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_ProcAmpMsgFromTtf_UATIRequest_WaitUATIAssign: Decode msg type fail or ucMsgType not as expected");

            return CNAS_HSM_FSM_EVENT_NOT_HANDLED;
        }

        NAS_MEM_SET_S(&stUatiAssignMsg, sizeof(stUatiAssignMsg), 0, sizeof(stUatiAssignMsg));

        ulDecodeRslt = CNAS_HSM_DecodeUATIAssignment(pstSnpDataInd->stMsgData.aucMsgData,
                                                     pstSnpDataInd->stMsgData.usMsgBitLen,
                                                     (VOS_VOID *)&stUatiAssignMsg);

        CNAS_MNTN_LogOmAirMsg(CNAS_HRPD_OM_AIR_MSG_DOWN,
                              NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_ASSIGN,
                              UEPS_PID_HSM,
                              (VOS_UINT32)pstSnpDataInd->stMsgData.usMsgBitLen,
                              pstSnpDataInd->stMsgData.aucMsgData);

        if (CNAS_HSM_DECODE_SUCCESS != ulDecodeRslt)
        {
            CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_ProcAmpMsgFromTtf_UATIRequest_WaitUATIAssign: Decode msg content fail");

            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        /* check the uati assign valid&fresh */
        ucIsUatiAssignValidAndFresh = CNAS_HSM_IsUatiAssignValidAndFresh(&stUatiAssignMsg);

        if (VOS_FALSE == ucIsUatiAssignValidAndFresh)
        {
            /* just discard the message and still wait uati assignment */
            return CNAS_HSM_FSM_EVENT_HANDLED;
        }

        /* 在等cttf的snpDataCnf时收到UATI assingment同样认为有效，退出UATI申请流程 */
        CNAS_HSM_StopTimer(TI_CNAS_HSM_UATI_REQUEST_WAIT_SNP_DATA_CNF);

        /* 开始发送uati cmpl, 取消uati req发送 */
        CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_REQUEST);

        CNAS_HSM_HandleUatiAssignMsg(&stUatiAssignMsg);

        CNAS_HSM_SndUatiCmplMsg();

        /* DSDS2.0 setup态需要等到HRM的UATI Cmpl Snd Ind 再退出状态机 */
        CNAS_HSM_StartTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND,
                            TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND_LEN);

        CNAS_HSM_SetCurrSubState(CNAS_HSM_SS_UATI_REQUEST_WAIT_UATI_CMPL_SND_IND);

    }

    return ulRslt;
}



VOS_UINT32 CNAS_HSM_RcvCasHrpdPageInd_UatiReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvApsDiscReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvEhsmDiscReq_WaitUatiReqSnpDataCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_VOID CNAS_HSM_HandleUatiAssignMsg_SetupState(
     CNAS_HSM_UATI_ASSIGN_MSG_STRU                         *pstUatiAssignMsg
)
{
    CNAS_HSM_ATI_RECORD_STRU                               *pstPubTransATI;
    CNAS_HSM_ATI_LIST_INFO_STRU                            *pstPubRcvATIList;
    CNAS_HSM_UATI_INFO_STRU                                *pstUatiInfo;
    CNAS_HSM_HRPD_SYS_INFO_STRU                            *pstHrpdSysInfo;

    /* Update TransmitATI and send to MAC*/
    /* C.S0024 7.3.7.1.5.1: ATIType = '10', ATI = (UATIColorCode | UATI[23:0]) */
    pstPubTransATI                 = CNAS_HSM_GetTransmitATIAddr();
    pstUatiInfo                    = CNAS_HSM_GetUatiInfoAddr();
    pstPubTransATI->enATIType      = CNAS_HSM_ATI_TYPE_UATI;

    pstUatiInfo->ucUATIColorCode   = pstUatiAssignMsg->ucUATIColorCode;
    pstUatiInfo->ucOldUATILen      = pstUatiAssignMsg->ucOldUATILen;

    if (1 == pstUatiAssignMsg->ucSubnetIncluded)
    {/* uati assignment msg include UATI104 and UATI subnetmask field */
        pstUatiInfo->ucUATISubnetMask = pstUatiAssignMsg->ucUATISubnetMask;

        /* UATI = UATI104 | UATI024 */
        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI,
                      sizeof(pstUatiInfo->aucCurUATI),
                      pstUatiAssignMsg->aucUati104,
                      CNAS_HSM_UATI104_OCTET_LENGTH);

        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI + CNAS_HSM_UATI104_OCTET_LENGTH,
                      (CNAS_HSM_UATI_OCTET_LENGTH - CNAS_HSM_UATI104_OCTET_LENGTH),
                      pstUatiAssignMsg->aucUati024,
                      CNAS_HSM_UATI024_OCTET_LENGTH);
    }
    else
    {
        pstHrpdSysInfo = CNAS_HSM_GetHrpdSysInfoAddr();

        pstUatiInfo->ucUATISubnetMask = pstHrpdSysInfo->ucSubNetMask;

        /* UATI = sectorID[127:24] | UATI024 */
        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI,
                      sizeof(pstUatiInfo->aucCurUATI),
                      pstHrpdSysInfo->aucSectorId,
                      CNAS_HSM_UATI104_OCTET_LENGTH);

        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI + CNAS_HSM_UATI104_OCTET_LENGTH,
                      (CNAS_HSM_UATI_OCTET_LENGTH - CNAS_HSM_UATI104_OCTET_LENGTH),
                      pstUatiAssignMsg->aucUati024,
                      CNAS_HSM_UATI024_OCTET_LENGTH);

    }

    pstPubTransATI->aucATIValue[0] = pstUatiAssignMsg->ucUATIColorCode;
    NAS_MEM_CPY_S(&(pstPubTransATI->aucATIValue[1]),
                  (CNAS_HSM_ATI_VALUE_LENGTH - 1),
                  &(pstUatiAssignMsg->aucUati024[0]),
                  CNAS_HSM_UATI024_OCTET_LENGTH);

    CNAS_HSM_SndMacTransmitAtiNotify(pstPubTransATI);

    /* Update ReciveATIList and send to MAC */
    /* C.S0024 7.3.7.1.5.1 Access Terminal Requirements
      Delete any entry in the ReceiveATIList list whose ATIType is equal to "11" (i.e., RATI).
      Add the following entry to the ReceiveATIList list
      <ATIType = '10', ATI = ATI = (UATIColorCode | UATI[23:0])>
    */

    /* In iteration 8, the second record is RATI, so just overwrite the second record with UATI,
       then RATI is deleted, also UATI is added
    */
    pstPubRcvATIList    = CNAS_HSM_GetReceivedATIListAddr();

    pstPubRcvATIList->ulATIRecordNum            = 2;
    pstPubRcvATIList->astATIEntry[1].enATIType  = CNAS_HSM_ATI_TYPE_UATI;

    pstPubRcvATIList->astATIEntry[1].aucATIValue[0] = pstUatiAssignMsg->ucUATIColorCode;
    NAS_MEM_CPY_S(&(pstPubRcvATIList->astATIEntry[1].aucATIValue[1]),
                  (CNAS_HSM_ATI_VALUE_LENGTH - 1),
                  &(pstUatiAssignMsg->aucUati024[0]),
                  CNAS_HSM_UATI024_OCTET_LENGTH);

    CNAS_HSM_SndMacReceiveAtiNotify(pstPubRcvATIList);

}


VOS_VOID CNAS_HSM_HandleUatiAssignMsg_OpenState(
     CNAS_HSM_UATI_ASSIGN_MSG_STRU                         *pstUatiAssignMsg
)
{
    CNAS_HSM_ATI_RECORD_STRU                               *pstPubTransATI;
    CNAS_HSM_ATI_LIST_INFO_STRU                            *pstPubRcvATIList;
    CNAS_HSM_UATI_INFO_STRU                                *pstUatiInfo;
    CNAS_HSM_HRPD_SYS_INFO_STRU                            *pstHrpdSysInfo;
    VOS_UINT32                                              ulAddrTimerLeft;
    VOS_UINT8                                               ucLoop;
    VOS_UINT32                                              ulTimerRunLen;

    /* Update TransmitATI and send to MAC*/
    /* C.S0024 7.3.7.1.5.1: ATIType = '10', ATI = (UATIColorCode | UATI[23:0]) */
    pstPubTransATI                 = CNAS_HSM_GetTransmitATIAddr();
    pstUatiInfo                    = CNAS_HSM_GetUatiInfoAddr();
    pstPubTransATI->enATIType      = CNAS_HSM_ATI_TYPE_UATI;

    pstUatiInfo->ucUATIColorCode   = pstUatiAssignMsg->ucUATIColorCode;
    pstUatiInfo->ucOldUATILen      = pstUatiAssignMsg->ucOldUATILen;

    if (1 == pstUatiAssignMsg->ucSubnetIncluded)/* uati assignment msg include UATI104 and UATI subnetmask field */
    {
        pstUatiInfo->ucUATISubnetMask = pstUatiAssignMsg->ucUATISubnetMask;

        /* UATI = UATI104 | UATI024 */
        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI,
                      sizeof(pstUatiInfo->aucCurUATI),
                      pstUatiAssignMsg->aucUati104,
                      CNAS_HSM_UATI104_OCTET_LENGTH);

        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI + CNAS_HSM_UATI104_OCTET_LENGTH,
                      (CNAS_HSM_UATI_OCTET_LENGTH - CNAS_HSM_UATI104_OCTET_LENGTH),
                      pstUatiAssignMsg->aucUati024,
                      CNAS_HSM_UATI024_OCTET_LENGTH);
    }
    else
    {
        pstHrpdSysInfo = CNAS_HSM_GetHrpdSysInfoAddr();

        pstUatiInfo->ucUATISubnetMask = pstHrpdSysInfo->ucSubNetMask;

        /* UATI = sectorID[127:24] | UATI024 */
        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI,
                      sizeof(pstUatiInfo->aucCurUATI),
                      pstHrpdSysInfo->aucSectorId,
                      CNAS_HSM_UATI104_OCTET_LENGTH);

        NAS_MEM_CPY_S(pstUatiInfo->aucCurUATI + CNAS_HSM_UATI104_OCTET_LENGTH,
                      (CNAS_HSM_UATI_OCTET_LENGTH - CNAS_HSM_UATI104_OCTET_LENGTH),
                      pstUatiAssignMsg->aucUati024,
                      CNAS_HSM_UATI024_OCTET_LENGTH);

    }

    pstPubTransATI->aucATIValue[0] = pstUatiAssignMsg->ucUATIColorCode;
    NAS_MEM_CPY_S(&(pstPubTransATI->aucATIValue[1]),
                  (CNAS_HSM_ATI_VALUE_LENGTH - 1),
                  &(pstUatiAssignMsg->aucUati024[0]),
                  CNAS_HSM_UATI024_OCTET_LENGTH);

    CNAS_HSM_SndMacTransmitAtiNotify(pstPubTransATI);

    /* Update ReciveATIList and send to MAC */
    /* C.S0024 7.3.7.1.5.1 Access Terminal Requirements
      Delete any entry in the ReceiveATIList list whose ATIType is equal to "11" (i.e., RATI).
      Add the following entry to the ReceiveATIList list
      <ATIType = '10', ATI = ATI = (UATIColorCode | UATI[23:0])>
    */

    pstPubRcvATIList    = CNAS_HSM_GetReceivedATIListAddr();

    if (CNAS_HSM_MAX_UATI_REC_NUM == pstPubRcvATIList->ulATIRecordNum)
    {
        ulAddrTimerLeft = CNAS_GetSpecifiedTimerRemainLen(UEPS_PID_HSM, TI_CNAS_HSM_ADDRESS_TIMER, 0);

        CNAS_HSM_StopTimer(TI_CNAS_HSM_ADDRESS_TIMER);

        NAS_NORMAL_LOG2(UEPS_PID_HSM, "Enter max uati rec num loop branch, address timer left length and address timer length",
                        ulAddrTimerLeft,
                        pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen);

        if (ulAddrTimerLeft < pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen)
        {
            ulTimerRunLen = pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen - ulAddrTimerLeft;
        }
        else
        {
            ulTimerRunLen = 0;
        }


        /* The receive ATI list is full, so we should remove the older ATI form ReceiveATI list */
        for (ucLoop = 1 ; ucLoop < pstPubRcvATIList->ulATIRecordNum - 1; ucLoop++)
        {
            NAS_MEM_CPY_S(&(pstPubRcvATIList->astATIEntry[ucLoop]),
                          sizeof(CNAS_HSM_ATI_RECORD_STRU),
                          &(pstPubRcvATIList->astATIEntry[ucLoop + 1]),
                          sizeof(CNAS_HSM_ATI_RECORD_STRU));

            /* update addr timer */
            pstPubRcvATIList->astATIEntry[ucLoop].ulAddrTimerLen -= ulTimerRunLen;
        }

        /* reset the timer */
        pstPubRcvATIList->astATIEntry[1].ulAddrTimerLen = 0;

        /* restart the address timer */
        CNAS_HSM_StartTimer(TI_CNAS_HSM_ADDRESS_TIMER, pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen);
    }
    else
    {
        if (CNAS_TIMER_STATUS_RUNNING == CNAS_GetSpecifiedTimerStatus(UEPS_PID_HSM, TI_CNAS_HSM_ADDRESS_TIMER, 0))
        {
            ulAddrTimerLeft = CNAS_GetSpecifiedTimerRemainLen(UEPS_PID_HSM, TI_CNAS_HSM_ADDRESS_TIMER, 0);

            NAS_NORMAL_LOG2(UEPS_PID_HSM, "Enter non max uati rec num loop branch, address timer left length and address timer length",
                            ulAddrTimerLeft,
                            pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen);

            if (ulAddrTimerLeft < pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen)
            {
                ulTimerRunLen = pstPubRcvATIList->astATIEntry[2].ulAddrTimerLen - ulAddrTimerLeft;
            }
            else
            {
                ulTimerRunLen = 0;
            }

            pstPubRcvATIList->astATIEntry[pstPubRcvATIList->ulATIRecordNum].ulAddrTimerLen = TI_CNAS_HSM_ADDRESS_TIMER_LEN + ulTimerRunLen;
        }
        else
        {
            pstPubRcvATIList->astATIEntry[pstPubRcvATIList->ulATIRecordNum].ulAddrTimerLen = TI_CNAS_HSM_ADDRESS_TIMER_LEN;
            CNAS_HSM_StartTimer(TI_CNAS_HSM_ADDRESS_TIMER, TI_CNAS_HSM_ADDRESS_TIMER_LEN);
        }
    }

    if (CNAS_HSM_MAX_UATI_REC_NUM != pstPubRcvATIList->ulATIRecordNum)
    {
        pstPubRcvATIList->astATIEntry[pstPubRcvATIList->ulATIRecordNum].enATIType  = CNAS_HSM_ATI_TYPE_UATI;

        pstPubRcvATIList->astATIEntry[pstPubRcvATIList->ulATIRecordNum].aucATIValue[0] = pstUatiAssignMsg->ucUATIColorCode;

        NAS_MEM_CPY_S(&(pstPubRcvATIList->astATIEntry[pstPubRcvATIList->ulATIRecordNum].aucATIValue[1]),
                      (CNAS_HSM_ATI_VALUE_LENGTH - 1),
                      &(pstUatiAssignMsg->aucUati024[0]),
                      CNAS_HSM_UATI024_OCTET_LENGTH);

        pstPubRcvATIList->ulATIRecordNum++;
    }
    else
    {
        pstPubRcvATIList->astATIEntry[CNAS_HSM_MAX_UATI_REC_NUM - 1].enATIType  = CNAS_HSM_ATI_TYPE_UATI;

        pstPubRcvATIList->astATIEntry[CNAS_HSM_MAX_UATI_REC_NUM - 1].aucATIValue[0] = pstUatiAssignMsg->ucUATIColorCode;

        NAS_MEM_CPY_S(&(pstPubRcvATIList->astATIEntry[CNAS_HSM_MAX_UATI_REC_NUM - 1].aucATIValue[1]),
                      (CNAS_HSM_ATI_VALUE_LENGTH - 1),
                      &(pstUatiAssignMsg->aucUati024[0]),
                      CNAS_HSM_UATI024_OCTET_LENGTH);

        /* refresh the addr timer */
        pstPubRcvATIList->astATIEntry[CNAS_HSM_MAX_UATI_REC_NUM - 1].ulAddrTimerLen = TI_CNAS_HSM_ADDRESS_TIMER_LEN;
    }

    CNAS_HSM_SndMacReceiveAtiNotify(pstPubRcvATIList);
}




CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8
CNAS_HSM_ConvertScpFailReasonToUatiFailReason(
    CAS_CNAS_HRPD_SCP_NEG_RSLT_ENUM_UINT8                   enScpFailReason
)
{
    VOS_UINT32                                              ulReasonCnt;
    VOS_UINT32                                              ulLoop;

    ulReasonCnt = sizeof(g_astHsmScpFailReasonMapUatiFailReasonTbl) /
                            sizeof(CNAS_HSM_SCP_FAIL_REASON_MAP_UATI_FAIL_REASON_STRU);

    for (ulLoop = 0; ulLoop < ulReasonCnt; ulLoop++)
    {
        if (g_astHsmScpFailReasonMapUatiFailReasonTbl[ulLoop].enScpFailReason
                                                                         == enScpFailReason)
        {
            return g_astHsmScpFailReasonMapUatiFailReasonTbl[ulLoop].enUatiFailReason;
        }
    }

    return CNAS_HSM_SESSION_ACT_FAIL_REASON_BUTT;
}


VOS_UINT32 CNAS_HSM_GetWaitUatiAssignTimerLen_UatiReq(VOS_VOID)
{
    CNAS_HSM_L1_STA_ENUM_UINT32                             enMainState;
    CNAS_HSM_WAIT_UATI_ASSIGN_TIMER_LEN_INFO_STRU          *pstWaitUatiAssignTimerLenInfoAddr;
    VOS_UINT32                                              ulWaitUatiAssignTimerLen;

    enMainState          = CNAS_HSM_GetCurrMainState();

    pstWaitUatiAssignTimerLenInfoAddr = CNAS_HSM_GetWaitUatiAssignTimerLenInfoAddr();

    if (CNAS_HSM_L1_STA_SETUP == enMainState)
    {
        ulWaitUatiAssignTimerLen =  pstWaitUatiAssignTimerLenInfoAddr->ucWaitUatiAssignTimerLenInAmpSetup;
    }
    else
    {
        ulWaitUatiAssignTimerLen =  pstWaitUatiAssignTimerLenInfoAddr->ucWaitUatiAssignTimerLenInAmpOpen;
    }

    return (ulWaitUatiAssignTimerLen * CNAS_HSM_SEC_TO_MILLISEC);

}


VOS_UINT32 CNAS_HSM_RcvHrmUatiCmplSndInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_UATI_CMPL_SND_IND_STRU                         *pstHrmSnpDataReqSndInd;
    VOS_UINT16                                              usUatiCmplOpId;

    pstHrmSnpDataReqSndInd  = (HRM_HSM_UATI_CMPL_SND_IND_STRU *)pstMsg;
    usUatiCmplOpId          = CNAS_HSM_GetUatiCmplSnpDataReqOpId();

    if (usUatiCmplOpId == pstHrmSnpDataReqSndInd->usOpId)
    {
        CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

        CNAS_HSM_StartTimer(TI_CNAS_HSM_UATI_COMPLETE_WAIT_SNP_DATA_CNF,
                            TI_CNAS_HSM_WAIT_SNP_DATA_CNF_LEN);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvHrmSnpDataReqSndInd_UatiReq_WaitUatiCmplSndInd: error OpId!");
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHrmDataCnf_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_DATA_CNF_STRU                                  *pstSnpDataCnf;
    VOS_UINT16                                              usUatiCmplOpId;
    CNAS_HSM_MNTN_LOG_DISCARD_MSG_REASON_UNION              unDiscardMsgReason;

    pstSnpDataCnf   = (HRM_HSM_DATA_CNF_STRU *)pstMsg;
    usUatiCmplOpId  = CNAS_HSM_GetUatiCmplSnpDataReqOpId();

    unDiscardMsgReason.stDiscardSnpDataCnfReason.enMainState    = CNAS_HSM_GetCurrMainState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.enSubState     = CNAS_HSM_GetCurrSubState();
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usRcvOpId      = pstSnpDataCnf->usOpId;
    unDiscardMsgReason.stDiscardSnpDataCnfReason.usSndOpId      = usUatiCmplOpId;

    /* Hrm在收到uati cmpl data req 后, 首先进行资源申请, 如果资源申请成功, hrm再发送snp data req到cttf,
       如果资源申请失败, hrm会直接回复hsm data cnf, 发送结果为no rf */
    if (usUatiCmplOpId == pstSnpDataCnf->usOpId)
    {
        CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }
    else
    {
        CNAS_HSM_LogDiscardMsgInfo(CNAS_HSM_MNTN_LOG_DISCARD_MSG_SCENE_ERROR_SNP_DATA_CNF, &unDiscardMsgReason);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvTiWaitUatiCmplSndIndTimerExpire_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvTiWaitUatiCmplSndIndTimerExpire_UatiReq_WaitUatiCmplSndInd: timer out");

    /* 申请资源保护定时器超时, 取消发送 */
    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

    /* UATIComlete信令的SNP_DATA_REQ_SND_IND保护定时器超时简单处理，按照成功处理 */
    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerOffReq_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    /* UATI分配等待complete，SETUP下，按照失败处理，
        OPEN下，按照成功处理，此时退出后不需要发起deactive流程 */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_OFF);
    }
    else
    {
        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdSuspendInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    /* Because Setup State have no valid session, and the UATI is cannot be used after quit Slave State,
       So we consider this procedure is fail though we have rcv valid and fresh UATIAssignment */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_SUSPEND);
    }
    else
    {
        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdPowerSaveReq_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    /* Because Setup State have no valid session, and the UATI is cannot be used after quit Slave State,
       So we consider this procedure is fail though we have rcv valid and fresh UATIAssignment */
    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_POWER_SAVE);
    }
    else
    {
        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_BUTT);
    }

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdNetworkLostInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    if (CNAS_HSM_L1_STA_SETUP == CNAS_HSM_GetCurrMainState())
    {
        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }
    else
    {
        CNAS_HSM_SndHrmSessionEndNtf(HSM_HRM_SESSION_TYPE_UATI);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_SUCC,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_NETWORK_LOST);
    }
    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHrmDataInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    HRM_HSM_DATA_IND_STRU                                  *pstSnpDataInd;
    CTTF_HRPD_PROTOCOL_TYPE_ENUM_UINT16                     enProtocolType;
    VOS_UINT8                                               ucMsgType;

    pstSnpDataInd  = (HRM_HSM_DATA_IND_STRU*)pstMsg;

    enProtocolType = pstSnpDataInd->enProtocolType;

    CNAS_HSM_DecodeMsgType(pstSnpDataInd->enProtocolType,
                           pstSnpDataInd->stMsgData.aucMsgData,
                           &ucMsgType);

    if (CTTF_HRPD_SNP_ADDRESS_MANAGEMENT_PROTOCOL == enProtocolType)
    {
        /* 暂时与原有代码的逻辑保持一致 */
        if (CNAS_HSM_AMP_MSG_TYPE_UATI_ASSIGN == ucMsgType)
        {
            CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

            /* just cache the message */
            CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);
        }

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }
    else
    {
        return CNAS_HSM_FSM_EVENT_NOT_HANDLED;
    }
}


VOS_UINT32 CNAS_HSM_RcvHsmSessionCloseInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU                    *pstMsgInd;
    CNAS_HSM_UATI_REQUEST_FAIL_REASON_ENUM_UINT8            enUatiFailReason;
    VOS_UINT32                                              ulHsmEventType;
    VOS_UINT8                                               ucIndex;

    /* 如果缓存中有OHM消息，则清除缓存中的OHM消息，因为后续在处理session close ind的时候会重新进行重试流程,
       此处如果不清理OHM消息，则在稳态处理缓存时可能向网侧发送两次session close */
    ulHsmEventType = CNAS_BuildEventType(UEPS_PID_HSD, (VOS_UINT16)ID_HSD_HSM_OVERHEAD_MSG_IND);

    ucIndex = CNAS_HSM_GetCacheIndexByEventType(ulHsmEventType);

    if (CNAS_HSM_INVAILD_CACHE_INDEX != ucIndex)
    {
        CNAS_HSM_ClearCacheMsgByIndex(ucIndex);
    }

    pstMsgInd = (CNAS_HSM_HSM_SESSION_CLOSE_IND_STRU *)pstMsg;


    switch (pstMsgInd->enScene)
    {
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_KEEP_ALIVE_EXP_OPEN:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_KEEP_ALIVE_EXP;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_SETUP:
        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_AN_CLOSE_IN_OPEN_NOT_SESSION_ACT:

            enUatiFailReason = CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_AN_CLOSE_SESSION;

            break;

        case CNAS_HSM_INTERNAL_SESSION_CLOSE_IND_SCENE_RCV_CAS_NEG_RSLD_IND:

            enUatiFailReason = CNAS_HSM_ConvertScpFailReasonToUatiFailReason(
                                                         pstMsgInd->stSessionNegRsltInfo.enResult);

            if (PS_TRUE == pstMsgInd->stSessionNegRsltInfo.enProtocolTypeVaildFlag)
            {
                CNAS_HSM_SetScpActFailProcType(pstMsgInd->stSessionNegRsltInfo.usProtocolType);
                CNAS_HSM_SetScpActFailProcSubtype(pstMsgInd->stSessionNegRsltInfo.usProtocolSubType);
            }

            break;

        default:

            return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL, enUatiFailReason);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasHrpdPageInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvApsDiscReq_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}



VOS_UINT32 CNAS_HSM_RcvEhsmDiscReq_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvEhsmEhrpdNotAvailableInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* just cache the message */
    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

    CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

    CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                            CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_EHRPD_NOT_AVAILABLE);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvCasPaRatModeNtf_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU                     *pstPaRatModeNtf;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enLastSessionType;
    CNAS_HSM_SESSION_TYPE_ENUM_UINT8                        enSessionType;
    CNAS_HSM_HSM_HRPD_PA_RAT_MODE_NTF_STRU                  stHsmHrpdPaRatModeNtf;
    VOS_UINT32                                              ulHsmEventType;

    enLastSessionType = CNAS_HSM_GetNegoSessionType();

    pstPaRatModeNtf = (CAS_CNAS_HRPD_PA_RAT_MODE_NTF_STRU *)pstMsg;

    /* convert the session type */
    enSessionType = CNAS_HSM_ConvertPaModeToHsmSessionType(pstPaRatModeNtf->enPaMode);
    if (CNAS_HSM_SESSION_TYPE_BUTT == enSessionType)
    {
        CNAS_WARNING_LOG(UEPS_PID_HSM, "CNAS_HSM_RcvCasPaRatModeNtf_UatiReq_WaitUatiCmplSndInd:receive the invalid session type--BUTT");

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

    /* Last session type is eHRPD and new negotiated session type is HRPD
       need fallback
     */
    if ((CNAS_HSM_SESSION_TYPE_EHRPD == enLastSessionType)
     && (CNAS_HSM_SESSION_TYPE_HRPD == enSessionType))
    {

        /* 该状态收到ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF之后，进入该分支后，会执行deactive流程，
           后续进入inactive后，需要重新进行uati，session激活流程，因而需要缓存一条消息在inactive
           态处理。但是，ID_CAS_CNAS_HRPD_PA_RAT_MODE_NTF在各个状态下都可能收到，而在inactive态收
           到该消息，hsm应该丢弃该消息，所以需要将其先转换为一条内部消息，再进行缓存 */
        CNAS_HSM_BuildHsmHrpdPaRatModeNtfMsg(pstPaRatModeNtf, &stHsmHrpdPaRatModeNtf);

        ulHsmEventType = CNAS_BuildEventType(stHsmHrpdPaRatModeNtf.ulSenderPid, (VOS_UINT16)stHsmHrpdPaRatModeNtf.enMsgId);

        /* just cache the message */
        CNAS_HSM_SaveCacheMsg(ulHsmEventType, &stHsmHrpdPaRatModeNtf);

        CNAS_HSM_StopTimer(TI_CNAS_HSM_WAIT_UATI_CMPL_SND_IND);

        CNAS_HSM_SndHrmDataCancelNtf(CNAS_HRM_DATA_REQ_UATI_CMPL);

        CNAS_HSM_QuitFsmUatiReq(CNAS_HSM_UATI_REQUEST_RESULT_FAIL,
                                CNAS_HSM_UATI_REQUEST_FAIL_REASON_ABORT_BY_PA_MODE_NTF);

        return CNAS_HSM_FSM_EVENT_HANDLED;
    }

     /* store the negotiated session type */
    CNAS_HSM_SetNegoSessionType(enSessionType);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}


VOS_UINT32 CNAS_HSM_RcvHsdOverHeadMsgInd_UatiReq_WaitUatiCmplSndInd(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 走到wait uati cmpl snd ind已经启用了网络分配的uati, 不能再回退到使用原来的uati了,
       所以不能走回退原小区流程 */
    CNAS_HSM_SetAbortFlag_UatiReq(VOS_TRUE);

    CNAS_HSM_SaveCacheMsg(ulEventType, pstMsg);

    return CNAS_HSM_FSM_EVENT_HANDLED;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

