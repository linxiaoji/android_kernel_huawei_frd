

#include "GmmInc.h"
#include "GmmCasGlobal.h"
#include "GmmCasComm.h"
#include "GmmCasSend.h"
#include "GmmCasGsm.h"
#include "GmmCasSuspend.h"
#include "GmmCasMain.h"
#include "GmmMmInterface.h"
#include "NasMmlCtx.h"
#if (FEATURE_ON == FEATURE_LTE)
#include "NasGmmProcLResult.h"
#endif

#include "NasStkInterface.h"
#include "NasUsimmApi.h"
#include "NasUtranCtrlInterface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767 修改人:luojian 107747;检视人:sunshaohua65952;原因:LOG方案设计需要*/
#define    THIS_FILE_ID        PS_FILE_ID_GMM_RAU_C
/*lint +e767 修改人:luojian 107747;检视人:sunshaohua*/

/*lint -save -e958 */


VOS_VOID Gmm_FillPdpContext(
                        VOS_UINT8 *pAddr                                        /* 填写PDP Context status的首地址           */
                        )
{
    VOS_UINT32                          ulPdpCtxStatus;
    NAS_MML_PS_BEARER_CONTEXT_STRU     *pstPsBearerCtx;

    pstPsBearerCtx = NAS_MML_GetPsBearerCtx();

    pAddr[0] = GMM_IEI_PDP_CONTEXT_STATUS;                                      /* PDP context status IEI                   */
    pAddr[1] = 2;                                                               /* Length of PDP context status contents    */

    NAS_GMM_ConvertPdpCtxStatusToNetworkPdpCtxStatus(pstPsBearerCtx, &ulPdpCtxStatus);
    pAddr[2] = (VOS_UINT8)ulPdpCtxStatus;
    pAddr[3] = (VOS_UINT8)(ulPdpCtxStatus >> NAS_MML_OCTET_MOVE_EIGHT_BITS);

    return;
}


VOS_UINT8  NAS_Gmm_FillRoutingPtmsiIE(
    VOS_UINT8                          *pucAddr
)
{
    VOS_UINT8                          *pucPtmsiAddr = VOS_NULL_PTR;
    VOS_UINT8                           ucNumber;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enRatType;
    VOS_UINT8                           ucUeIdMask;
    VOS_UINT32                          ulLocalPtmsiFlg;

#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_TIN_TYPE_ENUM_UINT8         enTinType;
    VOS_UINT32                          ulGetGutiRst;
    VOS_UINT8                          *pucMappedPtmsiAddr = VOS_NULL_PTR;

    VOS_UINT32                          ulIsSupportLteCapability;

    VOS_UINT8                           ucLDisabledRauUseLInfoFlag;

    ucLDisabledRauUseLInfoFlag = NAS_MML_GetLteDisabledRauUseLteInfoFlag();
#endif

    ucNumber                = GMM_MSG_LEN_TLV_PTMSI;


    /* 后续可选IE,以及非w模式，没有PTMSI IE */
    enRatType    = NAS_MML_GetCurrNetRatType();

    if (NAS_MML_NET_RAT_TYPE_WCDMA != enRatType)
    {
        ucNumber       = 0;
        return ucNumber;
    }

    /* 取得存储PTMSI数据的地址 */
    pucPtmsiAddr = NAS_MML_GetUeIdPtmsi();

    /* GU本地的UE ID MASK*/
    ucUeIdMask         = NAS_GMM_GetUeIdMask();

    /* GU本地的PTMSI是否有效，VOS_TRUE:有效;VOS_FALSE:无效; */
    ulLocalPtmsiFlg    = VOS_FALSE;

    if ( GMM_UEID_P_TMSI == ( ucUeIdMask & GMM_UEID_P_TMSI) )
    {
        ulLocalPtmsiFlg = VOS_TRUE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    /*4.7.5.1.1 Normal and periodic routing area updating procedure initiation
    If the MS supports S1 mode, the MS shall handle the P-TMSI  IE as follows:
    */

    /*If the TIN indicates "GUTI" and the MS holds a valid GUTI,  the MS shall map the GUTI into a P-TMSI,
    When the routing area updating procedure is initiated in Iu mode, the MS shall also include the mapped P-TMSI in the P TMSI IE.
    */

     /*If the TIN indicates "P-TMSI" or "RAT related TMSI" and the MS holds a
    valid P-TMSI and RAI, the MS shall indicate the RAI in the Old routing area
    identification IE. When the routing area updating procedure is initiated in Iu mode,
    the MS shall also include the P-TMSI in the P TMSI IE.
    */

    enTinType               = NAS_MML_GetTinType();
    ulGetGutiRst            = NAS_GMM_MapPtmsiFromGUTI();
    pucMappedPtmsiAddr      = NAS_GMM_GetMappedPtmsiAddr();

    ulIsSupportLteCapability = NAS_MML_IsSupportLteCapability();

    if ((VOS_TRUE == ulIsSupportLteCapability)
     || (VOS_TRUE == ucLDisabledRauUseLInfoFlag))
    {
        if ((NAS_MML_TIN_TYPE_GUTI == enTinType)
         && (VOS_OK == ulGetGutiRst))
        {
            pucAddr[0]      = GMM_IEI_P_TMSI;
            NAS_GMM_Fill_IE_PtmsiFromSrc(&pucAddr[1], pucMappedPtmsiAddr);
        }
        else if ( ((NAS_MML_TIN_TYPE_RAT_RELATED_TMSI == enTinType) || (NAS_MML_TIN_TYPE_PTMSI  == enTinType))
               && (VOS_TRUE == ulLocalPtmsiFlg) )
        {
            pucAddr[0]      = GMM_IEI_P_TMSI;
            NAS_GMM_Fill_IE_PtmsiFromSrc(&pucAddr[1], pucPtmsiAddr);
        }
        else
        {
            ucNumber       = 0;
        }

        return ucNumber;
    }
    else
    {
        if (VOS_TRUE == ulLocalPtmsiFlg)
        {
            pucAddr[0]      = GMM_IEI_P_TMSI;
            NAS_GMM_Fill_IE_PtmsiFromSrc(&pucAddr[1], pucPtmsiAddr);
        }
        else
        {
            ucNumber       = 0;
        }

        return ucNumber;
    }
#else

    if (VOS_TRUE == ulLocalPtmsiFlg)
    {
        pucAddr[0]      = GMM_IEI_P_TMSI;
        NAS_GMM_Fill_IE_PtmsiFromSrc(&pucAddr[1], pucPtmsiAddr);
    }
    else
    {
        ucNumber       = 0;
    }

    return ucNumber;

#endif

}

#if (FEATURE_ON == FEATURE_LTE)


VOS_UINT8 NAS_Gmm_FillRoutingPtmsiTypeIE(
    VOS_UINT8                          *pucAddr
)
{
    VOS_UINT8                           ucNumber;
    VOS_UINT8                           ucUeIdMask;
    VOS_UINT32                          ulLocalPtmsiFlg;
    NAS_MML_TIN_TYPE_ENUM_UINT8         enTinType;
    VOS_UINT32                          ulGetGutiRst;

    VOS_UINT32                          ulIsSupportLteCapability;

    VOS_UINT8                           ucLDisabledRauUseLInfoFlag;

    ucLDisabledRauUseLInfoFlag = NAS_MML_GetLteDisabledRauUseLteInfoFlag();

    ucNumber                = GMM_MSG_LEN_TV_PTMSITYPE;


    /* GU本地的UE ID MASK*/
    ucUeIdMask         = NAS_GMM_GetUeIdMask();

    /* GU本地的PTMSI是否有效，VOS_TRUE:有效;VOS_FALSE:无效; */
    ulLocalPtmsiFlg    = VOS_FALSE;

    if ( GMM_UEID_P_TMSI == ( ucUeIdMask & GMM_UEID_P_TMSI) )
    {
        ulLocalPtmsiFlg = VOS_TRUE;
    }

    /*4.7.5.1.1 Normal and periodic routing area updating procedure initiation
    If the MS supports S1 mode, the MS shall handle the P-TMSI  IE as follows:
    */

    /*If the TIN indicates "GUTI" and the MS holds a valid GUTI,  the MS shall map the GUTI into a P-TMSI,
    When the routing area updating procedure is initiated in Iu mode, the MS shall also include the mapped P-TMSI in the P TMSI IE.
    */

     /*If the TIN indicates "P-TMSI" or "RAT related TMSI" and the MS holds a
    valid P-TMSI and RAI, the MS shall indicate the RAI in the Old routing area
    identification IE. When the routing area updating procedure is initiated in Iu mode,
    the MS shall also include the P-TMSI in the P TMSI IE.
    */

    enTinType               = NAS_MML_GetTinType();
    ulGetGutiRst            = NAS_GMM_MapPtmsiFromGUTI();

    ulIsSupportLteCapability = NAS_MML_IsSupportLteCapability();

    if ((VOS_TRUE == ulIsSupportLteCapability)
     || (VOS_TRUE == ucLDisabledRauUseLInfoFlag))
    {
        if ((NAS_MML_TIN_TYPE_GUTI == enTinType)
         && (VOS_OK == ulGetGutiRst) )
        {
            /* 填写P-TMSI type为 Mapped P-TMSI*/
            pucAddr[0]       = 0x01;

            /* 填写IEI为 E- */
            pucAddr[0]      |= GMM_IEI_P_TMSI_TYPE;
        }
        else if ( ((NAS_MML_TIN_TYPE_RAT_RELATED_TMSI == enTinType) || (NAS_MML_TIN_TYPE_PTMSI  == enTinType))
               && (VOS_TRUE == ulLocalPtmsiFlg) )
        {
            /* 填写P-TMSI type为 Mapped P-TMSI*/
            pucAddr[0]       = 0x00;

            /* 填写IEI为 E0 */
            pucAddr[0]      |= GMM_IEI_P_TMSI_TYPE;
        }
        else
        {
            ucNumber       = 0;
        }
    }
    else
    {
        if (VOS_TRUE == ulLocalPtmsiFlg)
        {
            pucAddr[0]       = 0x00;

            /* 填写IEI为 E0 */
            pucAddr[0]      |= GMM_IEI_P_TMSI_TYPE;
        }
        else
        {
            ucNumber       = 0;
        }
    }

    return ucNumber;

}
#endif


VOS_UINT32  NAS_GMM_GetRoutingAreaUpdateRequestMsgIELength( VOS_VOID )
{
    VOS_UINT32                          ulMsgLen;

    /* Protocol discriminator 和 Skip indicator IE 长度1字节 */
    ulMsgLen = 1;

    /* Routing area update request message identity IE长度1字节 */
    ulMsgLen += 1;

    /* Update type 和GPRS ciphering key sequence number IE长度1字节 */
    ulMsgLen += 1;

    /* Old routing area identification IE长度6字节 */
    ulMsgLen += 6;

    /* 固定IE: MS Radio Access capability IE 6-51字节*/
    ulMsgLen += 51;

    /* Old P-TMSI signature IE */
    ulMsgLen += 4;

    /* Requested READY timer value IE */
    ulMsgLen += 2;

    /* DRX parameter IE */
    /* 这个可选IE ,现在并不是永远存在的，当LTE Attach/TAU 未携带过DRX 参数时，RAU才携带*/
    ulMsgLen += 3;



    /* TMSI status IE */
    ulMsgLen += 1;

    /*R99以前版本无P-TMSI(含)及后续可选IE*/
    /* P-TMSI IE */
    ulMsgLen += 7;

    /* 可选IE: MS Network Capability IE ,4-10字节 */
    ulMsgLen += 10;

    /* 可选IE: PDP Context status IE */
    ulMsgLen += 4;

    /* PS LCS Capability IE 长度3字节 */
    ulMsgLen += 3;

    /* MBMS context status 长度2-18字节 */
    ulMsgLen += 18;

    /* Additional mobile identity IE */
    ulMsgLen += 7;

    /* Additional old routing area identification IE */
    ulMsgLen += 8;

    /* P-TMSI type */
    ulMsgLen += 1;

    /* Device properties IE长度1字节 */
    ulMsgLen += 1;

    /* classmark2  */
    ulMsgLen += 5;

    /* classmark3  */
    ulMsgLen += 34;

    /* supported codec list  */
    ulMsgLen += 10;

    /* UE network capability IE 4-15字节 */
    ulMsgLen += 15;
    /* 说明如果UE支持E-UTRAN能力且支持CS fallback and SMS over SGs，
        则ATTACH和RAU请求消息中需要带Voice domain preference and UE's usage setting IE项 */

    /* 填写Voice domain preference and UE's usage setting IE项,长度为3 */
    ulMsgLen += GMM_MSG_LEN_TLV_VOICE_DOMAIN_PREFERANCE_AND_UE_USAGE_SETTING_LEN;

    /* MS network feature support IE */
    ulMsgLen += 1;

    /* Old location area identification IE 长度7字节*/
    ulMsgLen += 7;

    /* Additional update type 长度1字节 */
    ulMsgLen += 1;

    /* TMSI based NRI container IE 长度4字节 */
    ulMsgLen += 4;

    /* T3324 value IE长度3字节 */
    ulMsgLen += 3;

    /* T3312 extended value IE长度3字节 */
    ulMsgLen += 3;

    return ulMsgLen;

}


NAS_MSG_STRU *Gmm_RoutingAreaUpdateRequestMsgMake(
    VOS_UINT8                           ucRauType                               /* RAU的类型                                */
)
{
    VOS_INT8                            cVersion;
    NAS_MSG_STRU                       *pRauRequest;
    VOS_UINT8                           ucNumber;                                /* 填写IE时用来计数                         */
    VOS_UINT32                          ulMsgLen;                                /* 除 MS Radio Access capability IE外的固定IE 长度 */
#if (FEATURE_ON == FEATURE_LTE)
    NAS_LMM_INFO_STRU                   stLmmInfo;
#endif
    VOS_UINT32                          ulDataAvailFlg;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulIsLteCapabilityDisabled;

#endif

    NAS_MML_PLMN_RAT_PRIO_STRU          stMsRatCapabilityList;


    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = VOS_GetModemIDFromPid(WUEPS_PID_GMM);

    ulDataAvailFlg = CDS_IsPsDataAvail(enModemId);

    ucNumber         = 0;

    /* 取得填充IE所需要的长度 */
    ulMsgLen = NAS_GMM_GetRoutingAreaUpdateRequestMsgIELength();

    cVersion = NAS_Common_Get_Supported_3GPP_Version(MM_COM_SRVDOMAIN_PS);

    if (VOS_TRUE == NAS_GMM_IsCombinedSpecProc())
    {
        NAS_GMM_SetSpecProcInCsTrans(GMM_NULL_PROCEDURE);
    }

    /* 目前的处理网络能力变化后不会进行RAU */
    pRauRequest =
            (NAS_MSG_STRU *)PS_MEM_ALLOC(WUEPS_PID_GMM, (sizeof(NAS_MSG_STRU) + ulMsgLen) - 4);

    if (VOS_NULL_PTR == pRauRequest)
    {
        NAS_ERROR_LOG(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateRequestMsgMake:ERROR: MALLOC FAIL!");
        return pRauRequest;                                                     /* 返回                                     */
    }
    PS_MEM_SET(pRauRequest, 0, (sizeof(NAS_MSG_STRU ) + ulMsgLen) - 4);        /* 内存空间清0                              */

    pRauRequest->ulNasMsgSize = ulMsgLen;
    pRauRequest->aucNasMsg[0] = GMM_PD_GMM;
    pRauRequest->aucNasMsg[1] = GMM_MSG_RAU_REQUEST;
    ucNumber += 2;

    pRauRequest->aucNasMsg[ucNumber] = ucRauType;                               /* update type                              */

    /* 获取上层是否存在缓存数据,在上层存在缓存的数据时,返回需要设置PS_TRUE标志 */
    if (VOS_TRUE == NAS_GMM_IsFollowOnPend())
    {
        pRauRequest->aucNasMsg[ucNumber] |= GMM_FOLLOW_ON_REQ_PENDING;          /* follow on                                */
    }
    else if ((NAS_MML_NET_RAT_TYPE_WCDMA == NAS_MML_GetCurrNetRatType())
         &&  (PS_TRUE  == ulDataAvailFlg))
    {
        pRauRequest->aucNasMsg[ucNumber] |= GMM_FOLLOW_ON_REQ_PENDING;          /* follow on                                */
    }
    else
    {
    }

    if (VOS_TRUE == NAS_MML_IsNeedClearPsCksn_UsimDoneGsmPsAuth())
    {
        NAS_MML_SetSimPsSecurityCksn(MM_CKSN_INVALID);

        NAS_MML_SetUsimDoneGsmPsAuthFlg(VOS_FALSE);        
    }
    
    pRauRequest->aucNasMsg[ucNumber]     |= ((NAS_MML_GetSimPsSecurityCksn()<< 4) & 0xF0);  /* CKSN                                     */
    ucNumber += 1;

    /* 填写 RAI IE */
    ucNumber += NAS_Gmm_FillRaiForSnd(&pRauRequest->aucNasMsg[ucNumber]);       /* 填写RAI                                  */

    /* 无线接入能力填写 */
    NAS_MML_GetMsPsCurrRatCapabilityList(&stMsRatCapabilityList);
    NAS_MML_SetMsNotifyNwPsPrioRatList(&stMsRatCapabilityList);

    if ((GMM_COMBINED_RALA_UPDATING             == ucRauType)
     || (GMM_COMBINED_RALAU_WITH_IMSI_ATTACH    == ucRauType))
    {
        NAS_MML_GetMsCsCurrRatCapabilityList(&stMsRatCapabilityList);
        NAS_MML_SetMsNotifyNwCsPrioRatList(&stMsRatCapabilityList);
    }

    ucNumber += Gmm_Fill_IE_RadioAccessCapability(&pRauRequest->aucNasMsg[ucNumber]);

    /* 填写 P-TMSI Signature IE */
    ucNumber += NAS_Gmm_FillPtmsiSignatureIeForSnd(&pRauRequest->aucNasMsg[ucNumber]);

    GMM_FillReadyTimerParaIe(&pRauRequest->aucNasMsg[ucNumber]);
    ucNumber += 2;

    g_GmmGlobalCtrl.UeInfo.enLatestAttachOrRauContainDrx = NAS_MML_GU_PS_REG_NOT_CONTAIN_DRX_PARA;

    if (NAS_MML_LTE_PS_REG_CONTAIN_DRX_PARA != NAS_MML_GetPsRegContainDrx())
    {

        /* 填写DRX参数 */
        /*lint -e961*/
        pRauRequest->aucNasMsg[ucNumber++] = GMM_IEI_DRX_PARAMETER;             /* [false alarm]: 数组已动态分配*/
        /*lint +e961*/

        Gmm_FillDrxPara(&pRauRequest->aucNasMsg[ucNumber]);
        ucNumber += 2;                                                      /* DRX参数长度为2                           */

        g_GmmGlobalCtrl.UeInfo.enLatestAttachOrRauContainDrx = NAS_MML_GU_PS_REG_CONTAIN_DRX_PARA;
    }

    if (VOS_TRUE == NAS_GMM_IsRegNeedFillTmsiStatusIE())
    {                                                                           /* TMSI status                              */

        /* 填写TMSI status */
        pRauRequest->aucNasMsg[ucNumber] = GMM_IEI_TMSI_STATUS_NO_VALID;        /* [false alarm]: 数组已动态分配 */
        ucNumber += 1;                                                          /* DRX参数长度为3                           */
    }

    /* R99以前版本无P-TMSI(含)及后续可选IE*/
    if(PS_PTL_VER_PRE_R99 != cVersion)
    {
        ucNumber += NAS_Gmm_FillRoutingPtmsiIE(&pRauRequest->aucNasMsg[ucNumber]);

        /*填写网络能力
 */
        pRauRequest->aucNasMsg[ucNumber] = GMM_IEI_MS_NETWORK_CAPABILITY;
        ucNumber += 1;
        ucNumber += Gmm_Fill_IE_NetworkCapability(&pRauRequest->aucNasMsg[ucNumber]);

        Gmm_FillPdpContext(&pRauRequest->aucNasMsg[ucNumber]);                     /* 填写PDP context capability               */


        ucNumber += 4;


#if (FEATURE_ON == FEATURE_LTE)
        if (cVersion >= PS_PTL_VER_R9)
        {
            /* An MS supporting S1 mode shall include this IE, unless the update type indicates "periodic update" */
            if ((VOS_TRUE == NAS_MML_IsNetRatSupported(NAS_MML_NET_RAT_TYPE_LTE))
             && (GMM_PERIODC_UPDATING != ucRauType))
            {
                /* 填写UE network Capacility */
                ulIsLteCapabilityDisabled = NAS_MML_IsLteCapabilityDisabled(NAS_MML_GetLteCapabilityStatus(),
                                                                    NAS_MML_GetDisableLteRoamFlg());

                /* L未被disabled */
                if (VOS_FALSE == ulIsLteCapabilityDisabled)
                {
                    ucNumber += NAS_GMM_Fill_IE_UeNetwrokCapacity(&pRauRequest->aucNasMsg[ucNumber]);
                }


                /* 填写Additional mobile identity */
                ucNumber += NAS_Gmm_Fill_IE_AdditionalMobileIdentity(&pRauRequest->aucNasMsg[ucNumber]);

                /* 填写Additional old routing area identification */
                ucNumber += NAS_Gmm_Fill_IE_AdditionalOldRAI(&pRauRequest->aucNasMsg[ucNumber]);

            }

            if (VOS_TRUE == NAS_MML_GetSupportSrvccFlg())
            {
                /* 填写classmark2 */
                GMM_Fill_IE_ClassMark2(&pRauRequest->aucNasMsg[ucNumber]);
                ucNumber += GMM_MSG_LEN_CLASSMARK2;

                /* 填写classmark3 */
                /* 降圈复杂度 */
                ucNumber += GMM_Fill_IE_ClassMark3(&pRauRequest->aucNasMsg[ucNumber]);            

                /* 填写support code list */
                ucNumber += GMM_CALL_FillIeSupCodecList(&pRauRequest->aucNasMsg[ucNumber]);

            }



            if (VOS_TRUE == NAS_GMM_IsRegNeedFillVoiceDomainPreferenceAndUeUsageSettingIE())
            {
                /* 填写Voice domain preference and UE's usage setting IE项,长度为3 */
                ucNumber += NAS_GMM_Fill_IE_VoiceDomainPreferenceAndUeUsageSetting(&pRauRequest->aucNasMsg[ucNumber]);
            }

            /* 填写P-TMSI type */
            ucNumber += NAS_Gmm_FillRoutingPtmsiTypeIE(&pRauRequest->aucNasMsg[ucNumber]);

            /* 输出可维可测信息 */
            if ( VOS_TRUE == NAS_GMM_GetLteInfo(NAS_LMM_GUTI, &stLmmInfo))
            {
                NAS_GMM_LogGutiInfo(&(stLmmInfo.u.stGuti));
            }
        }

#endif

        /* 填写MS network feature support IE */
        if (PS_PTL_VER_R11 <= NAS_Common_Get_Supported_3GPP_Version(MM_COM_SRVDOMAIN_PS))
        {
            ucNumber += NAS_GMM_FillMsNetworkFeatureSupportIE(&pRauRequest->aucNasMsg[ucNumber]);
        }

#if (FEATURE_ON == FEATURE_LTE)
        /* 填写Old location area identification IE */
        if (VOS_TRUE == NAS_GMM_IsRegNeedFillOldLocationAreaIdentificationIE())
        {
            ucNumber += NAS_GMM_FillOldLocationAreaIdentificationIE(&pRauRequest->aucNasMsg[ucNumber]);
        }
#endif

        /* 填写 TMSI based NRI container IE*/
        if (VOS_TRUE == NAS_GMM_IsRegNeedFillTmsiBasedNRIContainerIE())
        {
            ucNumber += NAS_GMM_FillTmsiBasedNRIContainerIE(&pRauRequest->aucNasMsg[ucNumber]);
        }

    }

    g_GmmGlobalCtrl.UeInfo.enVoiceDomainFromRegReq  = NAS_MML_GetVoiceDomainPreference();

    /* 填写空口消息长度*/
    pRauRequest->ulNasMsgSize = ucNumber;

    /* 发起RAU流程，更新GMM链路释放控制变量 */
    NAS_GMM_UpdateGmmLinkCtrlStru();
    NAS_GMM_SetRetryRauForRelCtrlFlg(VOS_FALSE);

    return (NAS_MSG_STRU *)pRauRequest;
}


VOS_UINT16 NAS_GMM_GetRauComPleteMsgLen()
{
    VOS_UINT16                          usMsgLen;
    VOS_UINT8                           ucInterRatInfoFlag;

#if (FEATURE_ON == FEATURE_LTE)
        VOS_UINT16              ucEUeCapDataLen = 0;
        VOS_UINT8               aucEUeCapData[LTE_NAS_INTER_RAT_UETRAN_CAP_LEN];
        MMC_LMM_PLMN_ID_STRU    stLmmPlmn;
        VOS_UINT8               ucEUtranInterRatInfoFlag = VOS_FALSE;


    PS_MEM_SET(&stLmmPlmn, 0x00, sizeof(MMC_LMM_PLMN_ID_STRU));
#endif

#if (FEATURE_ON == FEATURE_LTE)
        if (VOS_TRUE == g_GmmInterRatInfoCtrl.ucEutranInterRatInfoRequestedFlag)
        {
            NAS_MML_ConvertNasPlmnToLMMFormat(&stLmmPlmn,NAS_MML_GetCurrCampPlmnId());

            ucEUeCapDataLen = LTE_NAS_INTER_RAT_UETRAN_CAP_LEN;
            if (MMC_LMM_SUCC == Nas_GetEutraUeCap(stLmmPlmn, (VOS_UINT16*)&ucEUeCapDataLen, aucEUeCapData))
            {
                if (0 != ucEUeCapDataLen)
                {
                    ucEUtranInterRatInfoFlag = VOS_TRUE;
                }
            }

        }
#endif

    if ((VOS_TRUE == g_GmmInterRatInfoCtrl.ucRauCmpWaitInterRatCnfMsg)
      && (VOS_TRUE == g_GmmInterRatInfoCtrl.ucInterRatCnfMsgRcvdFlg)
      && (0 != g_GmmInterRatInfoCtrl.ucDataLen))
    {
        ucInterRatInfoFlag = VOS_TRUE;
    }
    else
    {
        ucInterRatInfoFlag = VOS_FALSE;
    }

    usMsgLen = sizeof(NAS_MSG_STRU) + g_GmmRauCtrl.ucNpduCnt;

    /* 包含UTRAN INTER RAT INFOMATION */
    if (VOS_TRUE == ucInterRatInfoFlag)
    {
        usMsgLen += 2 + g_GmmInterRatInfoCtrl.ucDataLen;
    }

    /* 包含E-UTRAN INTER RAT INFORMATION */
#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == ucEUtranInterRatInfoFlag)
    {
        usMsgLen += 2 + ucEUeCapDataLen;
    }
#endif

    return  usMsgLen;
}


VOS_VOID NAS_GMM_RoutingAreaUpdateHandle_Handling_REQUESTED_MS_INFORMATION_IE(
    NAS_MSG_FOR_PCLINT_STRU            *pMsg,
    GMM_MSG_RESOLVE_STRU               *pRauAcceptIe,
    VOS_UINT8                          *pucSndCompleteFlg,
    VOS_UINT8                          *pucInterRatInfoFlg
)
{
    NAS_MML_PLMN_RAT_PRIO_STRU         *pstRatList;
    VOS_UINT32                          ulSurpportLteFlag;

    pstRatList        = NAS_MML_GetMsPrioRatList();
    ulSurpportLteFlag = NAS_MML_IsNetRatSupported(NAS_MML_NET_RAT_TYPE_LTE);

    if ( (( VOS_TRUE == NAS_MML_IsNetRatSupported(NAS_MML_NET_RAT_TYPE_WCDMA))
        || ( VOS_TRUE == ulSurpportLteFlag))
      && ( pstRatList->ucRatNum > 1))
    {
        if (GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION_FLG
            != (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION_FLG))
        {
            /* 消息中不包含 IE:Requested MS information直接返回   */
            return;
        }

        g_GmmInterRatInfoCtrl.ucRauCmpWaitInterRatCnfMsg = VOS_FALSE;

        /* 平台支持UTRAN */
        if (VOS_TRUE == NAS_MML_IsPlatformSupportUtran())
        {
            if (0x08 == (0x08 & (pMsg->aucNasMsg
                    [pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION]])))
            {                                                                   /* 消息中指示 Inter RAT information container IE requested */
                *pucSndCompleteFlg = GMM_TRUE;                                  /* 设定需要发送RAU COMPLETE消息             */
                *pucInterRatInfoFlg = GMM_TRUE;
            }
        }

#if   (FEATURE_ON == FEATURE_LTE)
        /* 平台支持LTE */
        if (VOS_TRUE == NAS_MML_IsPlatformSupportLte())
        {
            if (0x04 == (0x04 & (pMsg->aucNasMsg
                    [pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION]])))
            {
                *pucSndCompleteFlg = GMM_TRUE;                                    /* 设定需要发送RAU COMPLETE消息             */
                g_GmmInterRatInfoCtrl.ucEutranInterRatInfoRequestedFlag = VOS_TRUE;
            }
        }
#endif
    }
}



NAS_MSG_STRU *Gmm_RoutingAreaUpdateCompleteMsgMake(VOS_VOID)
{
    NAS_MSG_STRU    *pRauComplete = VOS_NULL_PTR;
    VOS_UINT16      usMsgLen;
    VOS_UINT16      usIeOffset    = GMM_MSG_LEN_RAU_COMPLETE;
    VOS_BOOL        bInterRatInfoFlg;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT16              usEUeCapDataLen     = 0;
    VOS_UINT8               aucEUeCapData[LTE_NAS_INTER_RAT_UETRAN_CAP_LEN]  = {0};
    MMC_LMM_PLMN_ID_STRU    stLmmPlmn;
#endif

    if ( (VOS_TRUE == g_GmmInterRatInfoCtrl.ucRauCmpWaitInterRatCnfMsg)
      && (VOS_TRUE == g_GmmInterRatInfoCtrl.ucInterRatCnfMsgRcvdFlg)
      && (0 != g_GmmInterRatInfoCtrl.ucDataLen))
    {
        bInterRatInfoFlg = VOS_TRUE;
    }
    else
    {
        bInterRatInfoFlg = VOS_FALSE;
    }

    usMsgLen = NAS_GMM_GetRauComPleteMsgLen();

    pRauComplete = (NAS_MSG_STRU *)Gmm_MemMalloc(usMsgLen);                     /* 申请内存空间                             */
    if (VOS_NULL_PTR == pRauComplete)
    {
        NAS_ERROR_LOG(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateCompleteMsgMake:ERROR: MemMalloc Failed.");
        return pRauComplete;
    }
    Gmm_MemSet(pRauComplete, 0, usMsgLen);                                      /* 内存空间清0                              */

    pRauComplete->ulNasMsgSize = GMM_MSG_LEN_RAU_COMPLETE;                      /* 填写消息大小                             */
    pRauComplete->aucNasMsg[0] = GMM_PD_GMM;                                    /* 填写Protocol discriminator               */
    pRauComplete->aucNasMsg[1] = GMM_MSG_RAU_COMPLETE;                          /* 填写RAU complete message identity        */
    /*lint -e961*/
    if (VOS_TRUE == g_GmmRauCtrl.bNpduPresentInRauAccFlg)
    {                                                                           /* 有N-PDU NUMBER                           */
        pRauComplete->ulNasMsgSize              = 4 + g_GmmRauCtrl.ucNpduCnt;
        pRauComplete->aucNasMsg[usIeOffset++]   = GMM_IEI_LIST_OF_RECEIVE_N_PDU_NUMBERS;
        pRauComplete->aucNasMsg[usIeOffset++]   = g_GmmRauCtrl.ucNpduCnt;
        Gmm_MemCpy(&pRauComplete->aucNasMsg[usIeOffset],
                   &g_GmmRauCtrl.aucNpduNum[0], g_GmmRauCtrl.ucNpduCnt);
        usIeOffset += g_GmmRauCtrl.ucNpduCnt;
    }

    if (VOS_TRUE == bInterRatInfoFlg)
    {
        pRauComplete->ulNasMsgSize             += 2 + g_GmmInterRatInfoCtrl.ucDataLen;
        pRauComplete->aucNasMsg[usIeOffset++]   = GMM_IEI_INTER_RAT_HANDOVER_INFORMATION;  /* [false alarm]: 数组已动态分配 */
        pRauComplete->aucNasMsg[usIeOffset++]   = g_GmmInterRatInfoCtrl.ucDataLen;         /* [false alarm]: 数组已动态分配 */
        Gmm_MemCpy(&(pRauComplete->aucNasMsg[usIeOffset]),
                   g_GmmInterRatInfoCtrl.aucData, g_GmmInterRatInfoCtrl.ucDataLen);
        usIeOffset += g_GmmInterRatInfoCtrl.ucDataLen;

    }
    /*lint +e961*/
#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == g_GmmInterRatInfoCtrl.ucEutranInterRatInfoRequestedFlag)
    {

        PS_MEM_SET(&stLmmPlmn, 0x00, sizeof(stLmmPlmn));

        NAS_MML_ConvertNasPlmnToLMMFormat(&stLmmPlmn,NAS_MML_GetCurrCampPlmnId());

        usEUeCapDataLen = LTE_NAS_INTER_RAT_UETRAN_CAP_LEN;
        if (MMC_LMM_SUCC != Nas_GetEutraUeCap(stLmmPlmn, &usEUeCapDataLen, aucEUeCapData))
        {
            usEUeCapDataLen = 0;
        }
    }
    /*lint -e961*/
    if ((VOS_TRUE == g_GmmInterRatInfoCtrl.ucEutranInterRatInfoRequestedFlag)
     && (0 != usEUeCapDataLen))
    {
        pRauComplete->ulNasMsgSize             += 2 + usEUeCapDataLen;
        pRauComplete->aucNasMsg[usIeOffset++]   = GMM_IEI_E_UTRAN_INTER_RAT_HANDOVER_INFORMATION;
        pRauComplete->aucNasMsg[usIeOffset++]   = (VOS_UINT8)usEUeCapDataLen;
        Gmm_MemCpy(&(pRauComplete->aucNasMsg[usIeOffset]),
                   aucEUeCapData, usEUeCapDataLen);
    }
    /*lint +e961*/
#endif

    /* 全局变量复位 */
    Gmm_MemSet(&g_GmmInterRatInfoCtrl, 0, sizeof(GMM_INTER_RAT_HANDOVER_INFO_CTRL_STRU));

    return pRauComplete;
}
/*******************************************************************************
  Module   : Gmm_RoutingAreaUpdateRejectCause13
  Function : 空口消息RAU REJECT原因值为#13,#15的处理
  Input    : 无
  Output   : 无
  NOTE     : 无
  Return   : 无
  History  :
    1. 张志勇  2003.12.10  新规作成

*******************************************************************************/
VOS_VOID Gmm_RoutingAreaUpdateRejectCause13(VOS_BOOL   bUpdateSimFlg)
{
    if (VOS_TRUE == bUpdateSimFlg)
    {
        NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_ROUTING_AREA_NOT_ALLOWED);      /* 更新状态置为GU3                          */
        if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
        {
            Gmm_SndAgentUsimUpdateFileReq(USIMM_USIM_EFPSLOCI_ID);       /* 更新SIM卡信息                            */
        }
        else
        {
            Gmm_SndAgentUsimUpdateFileReq(USIMM_GSM_EFLOCIGPRS_ID);
        }
    }
    Gmm_ComStaChg(GMM_REGISTERED_LIMITED_SERVICE);                              /* 调用状态的公共处理                       */
    g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_LA_FOR_ROAM;
    g_GmmRauCtrl.ucRauAttmptCnt = 0;                                            /* RAU attempt counter清0                   */

    return;
}

VOS_VOID Gmm_RoutingAreaUpdateRejectCause11(
                                        VOS_UINT32 ulGmmCause,                 /* 失败原因值                               */
                                        VOS_BOOL   bUpdateSimFlg
                                        )
{
    if (NAS_MML_REG_FAIL_CAUSE_LA_NOT_ALLOW == ulGmmCause)
    {
        g_GmmGlobalCtrl.ucLaNotAllowFlg = GMM_TRUE;
    }

    

    g_GmmRauCtrl.ucRauAttmptCnt = 0;

    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateRejectCause11:INFO: PLMN not allowed");

    if (VOS_TRUE == bUpdateSimFlg)
    {
        NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_ROUTING_AREA_NOT_ALLOWED);  /* 更新状态置为GU3                          */
        Gmm_DelPsLocInfoUpdateUsim();                                           /* 调用函数删除PS LOCATION INFO             */
    }

    if ((VOS_FALSE == bUpdateSimFlg)
        && (GMM_STATUS_ATTACHED == g_MmSubLyrShare.GmmShare.ucAttachSta))
    {
        Gmm_ComStaChg(GMM_REGISTERED_LIMITED_SERVICE);                          /* 调用状态的公共处理                       */
    }
    else
    {
        Gmm_ComStaChg(GMM_DEREGISTERED_LIMITED_SERVICE);                        /* 调用状态的公共处理                       */
    }

    if (NAS_MML_REG_FAIL_CAUSE_LA_NOT_ALLOW == ulGmmCause)
    {                                                                           /* 原因值#12                                */
        g_GmmRauCtrl.ucRauAttmptCnt = 0;                                        /* RAU attempt counter清0                   */
        g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_LA_FOR_REG_PRVS_SVS;
    }
    else if (NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW == ulGmmCause)
    {                                                                           /* 原因值#11                                */
        g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_PLMN;

        NAS_GMM_DeleteEPlmnList();
    }
    else
    {
        g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_PLMN_FOR_GPRS;
    }

    return;
}

VOS_VOID Gmm_RoutingAreaUpdateRejectCause9(
    VOS_UINT32                              ulGmmCause,
    VOS_UINT8                              *pucAttachFlg
)
{
    *pucAttachFlg = GMM_TRUE;

    if (NAS_MML_REG_FAIL_CAUSE_MS_ID_NOT_DERIVED == ulGmmCause)
    {                                                                           /* 原因值#9                                 */
        if (GMM_NO_FORBIDDEN == g_GmmGlobalCtrl.SysInfo.ucForbMask)
        {
            NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_NOT_UPDATED);     /* 更新状态置为GU2                          */
            if (GMM_TRUE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
            {
                Gmm_ClearLlcData(LL_GMM_CLEAR_DATA_TYPE_USER);
            }
            Gmm_DelPsLocInfoUpdateUsim();                                           /* 调用函数删除PS LOCATION INFO             */
        }
        else
        {
            /* 如果在FORBINDDEN列表中，不做Attach */
            *pucAttachFlg = GMM_FALSE;
        }
    }
    else
    {
        /* W下rau被拒原因值10情况下也需去指派TLLI */
        if (GMM_TRUE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
        {
            Gmm_ClearLlcData(LL_GMM_CLEAR_DATA_TYPE_ALL);
        }

        if (GMM_NO_FORBIDDEN != g_GmmGlobalCtrl.SysInfo.ucForbMask)
        {
            /* 如果在FORBINDDEN列表中，不做Attach */
            *pucAttachFlg = GMM_FALSE;
        }

#if (FEATURE_ON == FEATURE_LTE)
        if ( NAS_MML_TIN_TYPE_GUTI == NAS_MML_GetTinType())
        {
            NAS_MML_SetSimPsSecurityCksn(NAS_MML_CKSN_INVALID);
            NAS_MML_InitPsSecurityUmtsCkInvalid();
            NAS_MML_InitPsSecurityUmtsIkInvalid();
            Gmm_SndAgentUsimUpdateFileReq(USIMM_USIM_EFKEYSPS_ID);
            Gmm_SndRrmmNasInfoChangeReq(RRC_NAS_MASK_DEL_KEY);
        }

#endif

    }
    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_DETACH_FINISH,
                    VOS_NULL_PTR,
                    NAS_OM_EVENT_NO_PARA);

    if (GMM_TRUE == *pucAttachFlg)
    {
        g_GmmGlobalCtrl.ucNeedChgServiceStatus = VOS_FALSE;
    }
    Gmm_ComStaChg(GMM_DEREGISTERED_NORMAL_SERVICE);
    g_GmmGlobalCtrl.ucNeedChgServiceStatus = VOS_TRUE;

    return;
}

VOS_VOID Gmm_RoutingAreaUpdateRejectCause3(
                                       VOS_UINT32 ulGmmCause                    /* 失败原因值                               */
                                       )
{
    NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_PLMN_NOT_ALLOWED);      /* 更新状态置为GU3                          */
    NAS_MML_SetSimPsRegStatus(VOS_FALSE);
    Gmm_DelPsLocInfoUpdateUsim();                                               /* 调用函数删除PS LOCATION INFO             */
    Gmm_ComStaChg(GMM_DEREGISTERED_NO_IMSI);                                    /* 调用状态的公共处理                       */

    /* 清除相关标志 */
    NAS_GMM_ClearIMSIOfUeID();

    NAS_GMM_DeleteEPlmnList();

    return;
}


VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg_T3302_Handling(
                                       NAS_MSG_FOR_PCLINT_STRU *pMsg
                                       )
{
    NAS_MSG_STRU   *pGmmStatus;

    NAS_MML_PLMN_WITH_RAT_STRU          stPlmnWithRat;

    PS_MEM_SET(&stPlmnWithRat, 0, sizeof(stPlmnWithRat));

    if (GMM_MSG_LEN_RAU_REJECT < pMsg->ulNasMsgSize)
    {                                                                           /* 消息中含有OP项                           */
        /* 24.008: 9.4.17.1
          In Iu mode, the MS shall ignore the contents of this IE if this message is received
          without integrity protection.
          If this IE is not included or if in Iu mode the message is not integrity protected,
          the MS shall use the default value.  */
        if (GMM_IEI_T3302_VALUE == pMsg->aucNasMsg[GMM_MSG_LEN_RAU_REJECT])
        {                                                                       /* 消息中含有T3302 value                    */
            if (((GMM_MSG_LEN_RAU_REJECT + 1) == pMsg->ulNasMsgSize)
                || ((GMM_MSG_LEN_RAU_REJECT + 2) == pMsg->ulNasMsgSize))
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: T3302 of RAU REJECT is omitted");
            }
            else
            {
                if (1 != pMsg->aucNasMsg[GMM_MSG_LEN_RAU_REJECT + 1])
                {
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: RAU REJECT is semantically incorrect(IE:T3302)");
                    pGmmStatus = Gmm_GmmStatusMsgMake(
                                      NAS_MML_REG_FAIL_CAUSE_SEMANTICALLY_INCORRECT_MSG);

                    Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);
                    return;
                }


                /* TDD/FDD需要完整性保护,且没收到SMC，则使用默认值 */
                if ( (VOS_TRUE == NAS_UTRANCTRL_IsUtranPsSmcNeeded())
                  && (NAS_MML_RRC_INTEGRITY_PROTECT_ACTIVE != g_MmSubLyrShare.GmmShare.ucPsIntegrityProtect) )
                {                                                               /* If in Iu mode the message is not integrity protected, the MS shall use the default value */
                    g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
                }
                else
                {
                    Gmm_SaveTimerValue(GMM_TIMER_T3302,
                                   pMsg->aucNasMsg[GMM_MSG_LEN_RAU_REJECT + 2]);/* 存储T3302时长                            */

                    PS_MEM_CPY(&stPlmnWithRat.stPlmnId, NAS_MML_GetCurrCampPlmnId(),sizeof(stPlmnWithRat.stPlmnId));
                    stPlmnWithRat.enRat = NAS_MML_GetCurrNetRatType();

                    NAS_GMM_SetAllocT3302ValuePlmnWithRat(&stPlmnWithRat);
                }
            }
        }

        else
        {
            g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
        }
    }

    else
    {                                                                           /* If this IE is not included , the MS shall use the default value */
        g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
    }

    return;
}


 VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg_Default_Cause_Handling(
                                                 NAS_MSG_FOR_PCLINT_STRU *pMsg,
                                                 VOS_UINT8       *pucAbnormalCaseFlg)
 {
    /* 3GPP 24.008, 4.7.3.1.5 Abnormal cases in the MS
      d)ATTACH REJECT, other causes than those treated in subclause 4.7.3.1.4
      Upon reception of the cause codes # 95, # 96, # 97, # 99 and # 111 the
      MS should set the GPRS attach attempt counter to 5.

      Gmm Combined RAU has similiar handling. */

    /*  3GPP 24.008 10.5.5.14 GMM cause
        Any other value received by the mobile station shall be treated as 0110 1111,
        "Protocol error, unspecified". Any other value received by the network shall
        be treated as 0110 1111, "Protocol error, unspecified".*/

#if (PS_UE_REL_VER >= PS_PTL_VER_R6)
    if ((NAS_MML_REG_FAIL_CAUSE_SEMANTICALLY_INCORRECT_MSG == pMsg->aucNasMsg[2])
     || (NAS_MML_REG_FAIL_CAUSE_INVALID_MANDATORY_INF == pMsg->aucNasMsg[2])
     || (NAS_MML_REG_FAIL_CAUSE_MSG_NONEXIST_NOTIMPLEMENTE == pMsg->aucNasMsg[2])
     || (NAS_MML_REG_FAIL_CAUSE_IE_NONEXIST_NOTIMPLEMENTED == pMsg->aucNasMsg[2])
    /* 根据24008协议，增加对GMM遗漏的原因值处理 */
     || (NAS_MML_REG_FAIL_CAUSE_IMSI_UNKNOWN_IN_VLR == pMsg->aucNasMsg[2])
     || (NAS_MML_REG_FAIL_CAUSE_CS_DOMAIN_NOT_AVAILABLE == pMsg->aucNasMsg[2])
     || (NAS_MML_REG_FAIL_CAUSE_ESM_FAILURE == pMsg->aucNasMsg[2])
     || ((pMsg->aucNasMsg[2] >= NAS_MML_REG_FAIL_CAUSE_NOT_AUTHORIZED_FOR_THIS_CSG )
      && (pMsg->aucNasMsg[2] <= NAS_MML_REG_FAIL_CAUSE_CS_DOMAIN_TEMP_NOT_AVAILABLE))
     || ((pMsg->aucNasMsg[2] > NAS_MML_REG_FAIL_CAUSE_MSG_NOT_COMPATIBLE )
      && (pMsg->aucNasMsg[2] <= NAS_MML_REG_FAIL_CAUSE_PROTOCOL_ERROR) ))
    {
        g_GmmRauCtrl.ucRauAttmptCnt = 5;
    }
#endif
    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: GMM cause is Abnormal");
    if ((NAS_MML_REG_FAIL_CAUSE_RETRY_UPON_ENTRY_CELL_MIN    <= pMsg->aucNasMsg[2])
        && (NAS_MML_REG_FAIL_CAUSE_RETRY_UPON_ENTRY_CELL_MAX >= pMsg->aucNasMsg[2]))
    {
        g_GmmRauCtrl.ucRetryFlg = GMM_TRUE;                                 /* 置换小区进行RAU标志                      */
    }
    else
    {
        g_GmmRauCtrl.ucRetryFlg = GMM_FALSE;                                /* 清除换小区进行RAU标志                    */
    }

    if (VOS_TRUE == NAS_MML_IsRoamingRejectNoRetryFlgActived(pMsg->aucNasMsg[2]))
    {
        g_GmmRauCtrl.ucRauAttmptCnt = 5;
    }

    Gmm_RoutingAreaUpdateAttemptCounter(pMsg->aucNasMsg[2]);                /* 调用异常处理函数                         */
    *pucAbnormalCaseFlg = GMM_TRUE;

    return;
}


VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg_Cause_Handling(
                                       NAS_MSG_FOR_PCLINT_STRU *pMsg,
                                       VOS_UINT8       *pucAttachFlg,
                                       VOS_UINT8       *pucAbnormalCaseFlg
                                       )
{
    /* 保存错误码*/
    NAS_GMM_SetAttach2DetachErrCode(NAS_GMM_TransGmmNwCause2GmmSmCause(pMsg->aucNasMsg[2]));

    switch (pMsg->aucNasMsg[2])
    {                                                                           /* GMM CAUSE                                */
    case NAS_MML_REG_FAIL_CAUSE_ILLEGAL_MS:
    case NAS_MML_REG_FAIL_CAUSE_ILLEGAL_ME:
    case NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW:
        Gmm_RoutingAreaUpdateRejectCause3(pMsg->aucNasMsg[2]);                  /* 调用#3,#6,#7,#8处理函数                  */
        if (NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW == pMsg->aucNasMsg[2])
        {                                                                       /* 原因值7                                  */
            if ((GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg)
                && (GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg))
            {                                                                   /* 要触发LU                                 */
                NAS_GMM_SndMmLuInitiation(GMMMM_NO_TIMER_EXPIRED);              /* 通知MM进行LU                             */
            }
        }
        break;

    case NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOW:
        if ((GMM_RAU_NORMAL == g_GmmGlobalCtrl.ucSpecProc)
            || (GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc))
        {                                                                       /* 当前流程为normal RAU                     */
            g_GmmRauCtrl.ucRetryFlg = GMM_FALSE;

            /* 非联合注册周期性RAU,网侧异常8号原因转化为111,避免导致卡无效 */
            Gmm_RoutingAreaUpdateAttemptCounter(NAS_MML_REG_FAIL_CAUSE_PROTOCOL_ERROR);            /* 调用异常处理函数                         */
            *pucAbnormalCaseFlg = GMM_TRUE;
        }
        else
        {                                                                       /* 当前流程为combined RAU                   */
            Gmm_RoutingAreaUpdateRejectCause3(pMsg->aucNasMsg[2]);              /* 调用#3,#6,#7,#8处理函数                  */
        }
        break;

    case NAS_MML_REG_FAIL_CAUSE_MS_ID_NOT_DERIVED:
    case NAS_MML_REG_FAIL_CAUSE_IMPLICIT_DETACHED:
        Gmm_RoutingAreaUpdateRejectCause9(pMsg->aucNasMsg[2], pucAttachFlg);                  /* 调用#9,#10 处理函数                      */

        NAS_GMM_DeleteEPlmnList();
        break;

    case NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW:
    case NAS_MML_REG_FAIL_CAUSE_LA_NOT_ALLOW:
    case NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN:
        Gmm_RoutingAreaUpdateRejectCause11(pMsg->aucNasMsg[2],VOS_TRUE);        /* 调用#11,#12,#14 处理函数                 */
        if (NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN == pMsg->aucNasMsg[2])
        {                                                                       /* 原因值14                                 */
            g_GmmGlobalCtrl.CsInfo.ucCsTransFlg = GMM_TRUE;
            if ((GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg)
                && (GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg))
            {                                                                   /* 要触发LU                                 */
                NAS_GMM_SndMmLuInitiation(GMMMM_NO_TIMER_EXPIRED);              /* 通知MM进行LU                             */
            }
        }
        break;

    case NAS_MML_REG_FAIL_CAUSE_ROAM_NOT_ALLOW:
    case NAS_MML_REG_FAIL_CAUSE_NO_SUITABL_CELL:
        Gmm_RoutingAreaUpdateRejectCause13(VOS_TRUE);                                   /* 调用#13,#15处理函数                      */
        if (NAS_MML_REG_FAIL_CAUSE_ROAM_NOT_ALLOW == pMsg->aucNasMsg[2])
        {
            NAS_GMM_DeleteEPlmnList();
        }
        break;

    default:
        Gmm_RcvRoutingAreaUpdateRejectMsg_Default_Cause_Handling(pMsg, pucAbnormalCaseFlg);
        break;
    }

    NAS_GMM_CheckCauseToStartT3340(pMsg->aucNasMsg[2]);

    return;
}


VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg_AttachFlg_Handling(VOS_VOID)
{
    VOS_UINT8                           ucSimCsRegStatus;
    VOS_UINT8                           ucCsAttachAllow;

    ucSimCsRegStatus    = NAS_MML_GetSimCsRegStatus();
    ucCsAttachAllow     = NAS_MML_GetCsAttachAllowFlg();

    if (((VOS_TRUE != NAS_MML_GetCsAttachAllowFlg())
      || (GMM_NET_MODE_I != g_GmmGlobalCtrl.ucNetMod)))
    {                                                                           /* 不是用户模式A且网络模式I                 */
        g_GmmGlobalCtrl.ucSpecProc = GMM_ATTACH_NORMAL;                         /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RcvRoutingAreaUpdateRejectMsg_AttachFlg_Handling:INFO: normal attach procedure start");
    }
    else if ((VOS_TRUE == ucSimCsRegStatus)
        && (VOS_TRUE == ucCsAttachAllow)
        && (GMM_FALSE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg))
    {
                                                                    /* CS域ATTACH没被禁止,且没有CS业务                       */
        g_GmmGlobalCtrl.ucSpecProc = GMM_ATTACH_COMBINED;           /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RcvRoutingAreaUpdateRejectMsg_AttachFlg_Handling:INFO: combined attach procedure started");
    }
    else
    {                                                                           /* SIM无效，或者存在CS业务                                  */
        g_GmmGlobalCtrl.ucSpecProc = GMM_ATTACH_NORMAL;                         /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RcvRoutingAreaUpdateRejectMsg_AttachFlg_Handling:INFO: normal attach procedure start");
    }
    NAS_GMM_SndAttachReq();

    return;
}


VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg_Not_AbnormalCaseFlg_Handling(NAS_MSG_FOR_PCLINT_STRU *pMsg)
{
    if ((GMM_RAU_NORMAL == g_GmmGlobalCtrl.ucSpecProc)
        || (GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc))
    {                                                                       /* NORMAL RAU                               */
        NAS_GMM_SndMmGmmActionResultInd(GMMMM_ACTION_FAILURE,
                                        pMsg->aucNasMsg[2]);                         /* 发送MMCGMM_GMM_ACTION_RESULT_IND给MMC    */
    }
    else
    {
        NAS_GMM_SndMmCombinedRauRejected(pMsg->aucNasMsg[2]);                  /* 发送MMCGMM_COMBINED_RAU_REJECTED给MMC    */
    }


    /* 判断等待标志是否存在，如果存在则发送MMCGMM_ATTACH CNF */
    if (GMM_WAIT_PS_ATTACH == (g_GmmGlobalCtrl.stAttachInfo.enAttachType
                             & GMM_WAIT_PS_ATTACH))
    {
        NAS_GMM_SndMmcAttachCnf();
    }
    g_GmmGlobalCtrl.stAttachInfo.enAttachType = GMM_WAIT_NULL_ATTACH;


    /* 向MMC发送PS注册结果 */
    NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                 GMM_MMC_ACTION_RESULT_FAILURE,
                                 pMsg->aucNasMsg[2]);

    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_RAU_FAIL,
                    (VOS_VOID *)&pMsg->aucNasMsg[2],
                    NAS_OM_EVENT_RAU_FAIL_LEN);


    return;
}




NAS_STK_UPDATE_TYPE_ENUM_UINT8 NAS_GMM_ConvertGmmRauTypeToStkRauType (
   NAS_GMM_SPEC_PROC_TYPE_ENUM_UINT8    enGmmRauType
)
{
    switch (enGmmRauType)
    {
        case GMM_RAU_COMBINED:
             return NAS_STK_RAU_COMBINED;

        case GMM_RAU_WITH_IMSI_ATTACH:
             return NAS_STK_RAU_WITH_IMSI_ATTACH;

        case GMM_RAU_NORMAL:
             return NAS_STK_RAU_NORMAL;

        case GMM_RAU_NORMAL_CS_TRANS:
             return NAS_STK_RAU_NORMAL_CS_TRANS;

        case GMM_RAU_NORMAL_CS_UPDATED:
             return NAS_STK_RAU_NORMAL_CS_UPDATED;

        default:
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "NAS_GMM_ConvertGmmRauTypeToStkRauType:WARNING: The input in invalid");
            return NAS_STK_UPDATE_TYPE_ENUM_BUTT;
    }
}



TAF_MMA_RAT_TYPE_ENUM_UINT8 NAS_GMM_ConvertGmmRatTypeToStkRatType (
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enGmmRatType
)
{
    switch (enGmmRatType)
    {
        case NAS_MML_NET_RAT_TYPE_GSM:
             return TAF_MMA_RAT_GSM;

        case NAS_MML_NET_RAT_TYPE_WCDMA:
             return TAF_MMA_RAT_WCDMA;

        case NAS_MML_NET_RAT_TYPE_LTE:
             return TAF_MMA_RAT_LTE;

        default:
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "NAS_GMM_ConvertGmmRatTypeToStkRatType:WARNING: The input in invalid");
            return TAF_MMA_RAT_BUTT;
    }
}



VOS_VOID NAS_GMM_SndStkRauRej(VOS_UINT8 ucCause)
{
    NAS_STK_NETWORK_REJECTION_EVENT_STRU                   *pstMsg = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    NAS_MML_LAI_STRU                                       *pstRAI = VOS_NULL_PTR;

    /* 申请内存 */
    ulLength = sizeof(NAS_STK_NETWORK_REJECTION_EVENT_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (NAS_STK_NETWORK_REJECTION_EVENT_STRU *)PS_ALLOC_MSG(WUEPS_PID_GMM, ulLength);
    if (VOS_NULL_PTR == pstMsg)
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL_PTR, LOG_LEVEL_ERROR, "NAS_GMM_ReportStkRauRej: Alloc Msg");
        return;
    }

    PS_MEM_SET(&(pstMsg->stNetworkRejectionEvent), 0, sizeof(NAS_STK_NETWORK_REJECTION_EVENT_INFO_STRU));

    /* 填写新消息内容 */
    pstMsg->ulReceiverPid = NAS_GetOamReceivePid(MAPS_STK_PID);

    pstRAI    =  NAS_MML_GetCurrCampLai();
    pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mcc = pstRAI->stPlmnId.ulMcc;
    pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mnc = pstRAI->stPlmnId.ulMnc;
    pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.usLac           = (pstRAI->aucLac[0] << 8) | (pstRAI->aucLac[1]);
    pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.ucRac           = NAS_MML_GetCurrCampRac();
    pstMsg->stNetworkRejectionEvent.enRejType                 = NAS_STK_RAU_REJECT;

    pstMsg->stNetworkRejectionEvent.enRat                     = NAS_GMM_ConvertGmmRatTypeToStkRatType(pstRAI->enCampPlmnNetRat);

    pstMsg->stNetworkRejectionEvent.ucCauseCode               = ucCause;

    pstMsg->stNetworkRejectionEvent.enUpdateAttachType        = NAS_GMM_ConvertGmmRauTypeToStkRauType(NAS_GMM_GetSpecProc());

    if (VOS_TRUE == NAS_GMM_GetPeriodicRauFlag())
    {
        pstMsg->stNetworkRejectionEvent.enUpdateAttachType    = NAS_STK_RAU_PERIODIC_UPDATING;
    }

    pstMsg->ulMsgName                                         = ID_NAS_STK_NETWORK_REJECTION_EVENT;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_GMM, pstMsg))
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL_PTR, LOG_LEVEL_WARNING, "NAS_GMM_ReportStkRauRej: PS_SEND_MSG ERROR");
    }

    return;
}



VOS_VOID Gmm_RcvRoutingAreaUpdateRejectMsg(
    NAS_MSG_FOR_PCLINT_STRU            *pMsg
)
{
    VOS_UINT8                       ucAttachFlg;                                /* 触发ATTACH标志                           */
    VOS_UINT8                       ucAbnormalCaseFlg;                          /* 原因值是否异常标志                       */
    NAS_MSG_STRU                   *pGmmStatus = VOS_NULL_PTR;
    VOS_UINT8                       ucForceToStandby;
    VOS_UINT32                      ulNewTlli;
    VOS_UINT8                       aucPtmsi[NAS_MML_MAX_PTMSI_LEN];
    VOS_UINT32                      ulPtmsiValidFlag;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulT3423StatusChgNeedRegFlg;

    ulT3423StatusChgNeedRegFlg = NAS_GMM_IsT3423StatusChangeNeedRegist(NAS_MML_GetLteCsServiceCfg(),
                                                 NAS_MML_GetTinType(), NAS_MML_GetT3423Status());
#endif
    ucAttachFlg        = GMM_FALSE;
    ucAbnormalCaseFlg  = GMM_FALSE;
    ulNewTlli          = 0;

    g_GmmRauCtrl.ucT3312ExpiredFlg = GMM_FALSE;

    if (GMM_ROUTING_AREA_UPDATING_INITIATED != g_GmmGlobalCtrl.ucState)
    {
        /* GMM状态不是GMM_ROUTING_AREA_UPDATING_INITIATED，丢弃此消息 */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: RAU REJECT is unexpected");

        return;
    }

    /* 消息过短 */
    if (GMM_MSG_LEN_RAU_REJECT > pMsg->ulNasMsgSize)
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: The length of RAU REJECT is invalid");
        pGmmStatus = Gmm_GmmStatusMsgMake(NAS_MML_REG_FAIL_CAUSE_INVALID_MANDATORY_INF);

        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);

        return;
    }

#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == ulT3423StatusChgNeedRegFlg)
    {
        /* 需要清除T3423定时器状态，否则再次收到系统消息后会反复做联合RAU */
        NAS_MML_SetT3423Status(NAS_MML_TIMER_STOP);
        NAS_MML_SetT3412Status(NAS_MML_TIMER_STOP);
    }
#endif

    /*==>A32D11635*/
    g_GmmGlobalCtrl.ucGprsResumeFlg = GMM_FALSE;
    /*<==A32D11635*/

    ucForceToStandby = (VOS_UINT8)(pMsg->aucNasMsg[3] & 0x0F);

    if (1 == ucForceToStandby)
    {
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_TRUE;
    }
    else if (0 == ucForceToStandby)
    {
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_FALSE;
    }
    else
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateRejectMsg:WARNING: RAU REJECT is semantically incorrect (IE:Force to Standby)");
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_FALSE;
    }

    NAS_GMM_ChangeRegRejCauseAvoidInvalidSim(&(pMsg->aucNasMsg[2]));

    NAS_GMM_SndStkRauRej(pMsg->aucNasMsg[2]);

    Gmm_RcvRoutingAreaUpdateRejectMsg_T3302_Handling(pMsg);

    Gmm_TimerStop(GMM_TIMER_T3330);                                             /* 停止Timer3330                            */
    Gmm_TimerStop(GMM_TIMER_T3318);                                             /* 停止T3318                                */
    Gmm_TimerStop(GMM_TIMER_T3320);                                             /* 停止T3320                                */

    if (GMM_RAU_FOR_NORMAL != gstGmmSuspendCtrl.ucRauCause)
    {
        g_GmmGlobalCtrl.ucEventCause = pMsg->aucNasMsg[2];
        GMM_RauFailureInterSys();
        g_GmmGlobalCtrl.ucEventCause = GMM_EVENT_CAUSE_INVALID;
    }
    else
    {
        GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_NORMAL, GMM_RABM_RAU_FAILURE);
    }

    Gmm_RcvRoutingAreaUpdateRejectMsg_Cause_Handling(pMsg, &ucAttachFlg, &ucAbnormalCaseFlg);

    if (GMM_FALSE == ucAbnormalCaseFlg)
    {                                                                           /* 不是异常原因值                           */
        Gmm_RcvRoutingAreaUpdateRejectMsg_Not_AbnormalCaseFlg_Handling(pMsg);
    }

    /* 如果缓存的有PS DETACH消息,在RAU拒绝的情况下清除允许PS注册的标志 */
    if ((GMM_WAIT_PS_DETACH == g_GmmGlobalCtrl.stDetachInfo.enDetachType)
     || (GMM_WAIT_CS_PS_DETACH == g_GmmGlobalCtrl.stDetachInfo.enDetachType))
    {
        NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);

        ucAttachFlg = GMM_FALSE;

        /* 向MMC发送PS注册结果 */
        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_ATTACH,
                                     GMM_MMC_ACTION_RESULT_FAILURE,
                                     NAS_MML_REG_FAIL_CAUSE_MS_CFG_DOMAIN_NOT_SUPPORT);
    }

    Gmm_ComCnfHandle();
    g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg = GMM_FALSE;                         /* 清除标志                                 */
    g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg = GMM_FALSE;                        /* 清除标志                                 */
    g_GmmRauCtrl.ucPeriodicRauFlg  = GMM_FALSE;                                 /* 清除当前RAU是否是周期RAU标志             */
    g_GmmGlobalCtrl.ucSpecProc     = GMM_NULL_PROCEDURE;                        /* 清除正在进行的specific过程标志           */
    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RcvRoutingAreaUpdateRejectMsg:INFO: specific procedure ended");


    /* 初始化 */
    PS_MEM_SET(aucPtmsi, 0, sizeof(aucPtmsi));

    ulPtmsiValidFlag   = NAS_GMM_GetPTmsiForTLLI(aucPtmsi);

    if ((GMM_TRUE == GMM_IsCasGsmMode())
      &&(VOS_TRUE == ulPtmsiValidFlag))
    {
        GMM_CharToUlong(&ulNewTlli, aucPtmsi);                                  /* 新的PTMSI */
        GMM_CreateNewTlli(&ulNewTlli, GMM_LOCAL_TLLI);                          /* 生成新TLLI */

        if (GMM_FALSE == GMM_AssignNewTlli(ulNewTlli, GMM_LOCAL_TLLI))
        {
            GMM_LOG_WARN("Gmm_RcvRoutingAreaUpdateRejectMsg():Error: new TLLI assigned unsuccessfully!");
        }
    }


    if (GMM_TRUE == ucAttachFlg)
    {
#if (FEATURE_ON == FEATURE_LTE)
        if ( NAS_MML_REG_FAIL_CAUSE_MS_ID_NOT_DERIVED == pMsg->aucNasMsg[2] )
        {
            NAS_GMM_SetLteGutiValid(VOS_FALSE);
        }
#endif

        if ((NAS_MML_MS_MODE_CS_ONLY == NAS_MML_GetMsMode())
         && (VOS_FALSE == g_GmmGlobalCtrl.ucUserPsAttachFlag))
        {
            NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
        }
        else
        {
        /* 两次ATTACH使用了原来的TBF引起ATTACH失败，所以在此处等到TBF释放后再发起ATTACH */
            if(GMM_TRUE == GMM_IsCasGsmMode())
            {
                g_GmmGlobalCtrl.ulGmmAttachFlg = GMM_TRUE;
            }
            else
            {
                Gmm_RcvRoutingAreaUpdateRejectMsg_AttachFlg_Handling();
            }
        }
#if (FEATURE_ON == FEATURE_LTE)
        NAS_GMM_SetLteGutiValid(VOS_TRUE);
#endif
    }

    if (GMM_TRUE == GMM_IsCasGsmMode())
    {
        if (GMM_TRUE == gstGmmCasGlobalCtrl.ucFtsFlg)
        {
            gstGmmCasGlobalCtrl.ucFtsFlg = GMM_FALSE;

            if (0xffffffff != gstGmmCasGlobalCtrl.ulReadyTimerValue)
            {
                gstGmmCasGlobalCtrl.GmmSrvState = GMM_AGB_GPRS_STANDBY;
                Gmm_TimerStop(GMM_TIMER_T3314);
                if ((0x10 != (g_GmmGlobalCtrl.ucState & 0xF0))
                 && (GMM_REGISTERED_INITIATED != g_GmmGlobalCtrl.ucState))
                {
#if (FEATURE_LTE == FEATURE_ON)
                    if (GMM_TIMER_T3312_FLG != (GMM_TIMER_T3312_FLG & g_GmmTimerMng.ulTimerRunMask))
                    {
                        NAS_GMM_SndLmmTimerInfoNotify(GMM_TIMER_T3312, GMM_LMM_TIMER_START);
                    }
#endif
                    Gmm_TimerStart(GMM_TIMER_T3312);
                }

                NAS_GMM_SndGasInfoChangeReq(NAS_GSM_MASK_GSM_GMM_STATE);
            }
        }
    }

    /* 当ucRauCause为GMM_RAU_FOR_INTERSYSTEM时,通知RABM的RAU类型不同(RAU类型为系统间切换),
       而另外两种原因值发送给RABM的RAU类型相同(正常的RAU),RABM会根据RAU的类型进行状态的迁移,
       所以如果RAU原因为GMM_RAU_FOR_INTERSYSTEM时, 不能清除ucRauCause */
    if (GMM_RAU_FOR_WAITSERVICE == gstGmmSuspendCtrl.ucRauCause)
    {
        /* 此标志记录的是系统间重选或者切换时,RAI相同时,当后续有上行数据时需要发送RAU.
           因为调用此函数时RAU已经完成,所以需要清除此标志 */
        gstGmmSuspendCtrl.ucRauCause = GMM_RAU_FOR_NORMAL;
    }

    return;
}

VOS_UINT8 Gmm_Com_CmpRai(
    GMM_RAI_STRU                       *pstNewRai,
    GMM_RAI_STRU                       *pstOldRai,
    VOS_UINT8                           ucNetMod
)
{
    VOS_UINT8                           ucRet;

    VOS_UINT32                          ulIsPlmnInSupportDamPlmnInfo;
    VOS_UINT32                          ulT3302Status;

    ulIsPlmnInSupportDamPlmnInfo = NAS_MML_IsPlmnSupportDam(NAS_MML_GetCurrCampPlmnId());
    ulT3302Status                = NAS_GMM_QryTimerStatus(GMM_TIMER_T3302);

    ucRet         = GMM_FALSE;

    if (GMM_TRUE == Gmm_Compare_Lai(&pstNewRai->Lai, &pstOldRai->Lai))
    {                                                                           /* LAI一致                                  */
        if (pstNewRai->ucRac == pstOldRai->ucRac)
        {                                                                       /* RAC一致                                  */
            ucRet = GMM_TRUE;                                                   /* 赋返回值                                 */
        }
    }


#if (FEATURE_ON == FEATURE_LTE)
    if (NAS_MML_NET_RAT_TYPE_LTE == gstGmmSuspendCtrl.ucPreRat)
    {
        /* 3GPP 23401 4.3.5.6   Idle mode signalling reduction function:
           The Idle mode Signalling Reduction (ISR) function provides a
           mechanism to limit signalling during inter-RAT cell-reselection
           in idle mode (ECM-IDLE, PMM-IDLE, GPRS STANDBY states).
        */
        /* AT&T LTE-BTR-1-1824、LTE-BTR-1-1826和LTE-BTR-1-1828用例要求
           GMM在GU下attach无响应或被拒达最大次数启动T3302定时器，后续异系统
           或搜网到L下后无需停止，L下注册失败满足disable lte条件到GU下搜网
           回原失败小区时，等T3302超时后再做ps attach */
        if (NAS_MML_TIN_TYPE_RAT_RELATED_TMSI != NAS_MML_GetTinType()
          && ((VOS_FALSE == ulIsPlmnInSupportDamPlmnInfo)
           || (VOS_FALSE == ulT3302Status)))
        {
            ucRet = GMM_FALSE;
        }

        /* 如果网络模式I,从L异系统到GU,需要判断CS 的LAI是否改变，如果改变需要发起联合rau */
        if (GMM_TRUE == NAS_GMM_IsCsLaiChangeNeedRegister(ucNetMod))
        {
            ucRet = GMM_FALSE;
        }
    }
#endif

    return ucRet;
}

VOS_VOID Gmm_RoutingAreaUpdateAttemptCounter(
                                         VOS_UINT32 ulGmmCause                  /* 失败原因                                 */
                                         )
{


    NAS_MML_RAI_STRU                           *pstLastSuccRai;
    GMM_RAI_STRU                                stGmmRai;
    NAS_MML_ROUTING_UPDATE_STATUS_ENUM_UINT8    enPsUpdateStatus;
    NAS_MML_TIN_TYPE_ENUM_UINT8                 enTinType;

    enTinType       = NAS_MML_GetTinType();

    pstLastSuccRai    = NAS_MML_GetPsLastSuccRai();
    NAS_GMM_ConvertPlmnIdToGmmFormat(&(pstLastSuccRai->stLai.stPlmnId), &stGmmRai.Lai.PlmnId);

    stGmmRai.ucRac          = pstLastSuccRai->ucRac;
    stGmmRai.Lai.aucLac[0]  = pstLastSuccRai->stLai.aucLac[0];
    stGmmRai.Lai.aucLac[1]  = pstLastSuccRai->stLai.aucLac[1];



    /* 已经达到最大尝试次数，则没有继续加1的必要，只要达到或超过5次处理都是一样的*/
    if (GMM_ATTACH_RAU_ATTEMPT_MAX_CNT > g_GmmRauCtrl.ucRauAttmptCnt)
    {
        /* NO RF引起的RAU失败不增加尝试次数 */
        if (RR_REL_CAUSE_RR_CONNECTION_FAILURE_NO_RF != g_GmmGlobalCtrl.ucRelCause)
        {
            g_GmmRauCtrl.ucRauAttmptCnt++;                                          /* RAU Attempt Counter加1                   */
            NAS_NORMAL_LOG1(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateAttemptCounter: Not NO RF Scene, increase ucRauAttmptCnt", g_GmmRauCtrl.ucRauAttmptCnt);
        }
    }

    if (5 > g_GmmRauCtrl.ucRauAttmptCnt)
    {                                                                           /* ucRauAttmptCnt小于5                      */
        enPsUpdateStatus = NAS_MML_GetPsUpdateStatus();

        if ((GMM_TRUE == Gmm_Com_CmpRai(&stGmmRai, &g_GmmGlobalCtrl.SysInfo.Rai, g_GmmGlobalCtrl.ucNetMod))
         && (NAS_MML_ROUTING_UPDATE_STATUS_UPDATED == enPsUpdateStatus)
         && (NAS_MML_TIN_TYPE_GUTI                 != enTinType))
        {                                                                       /* RAI相等且更新状态是GU1                   */
            Gmm_ComStaChg(GMM_REGISTERED_NORMAL_SERVICE);                       /* 调用状态的公共处理                       */


            /*只有前面进行过RAU suspend，此处resume才有意义*/
            if (GMM_SUSPEND_LLC_FOR_RAU == (gstGmmCasGlobalCtrl.ucSuspendLlcCause & GMM_SUSPEND_LLC_FOR_RAU))
            {
                gstGmmCasGlobalCtrl.ucSuspendLlcCause &= ~GMM_SUSPEND_LLC_FOR_RAU;

                if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
                {
                    GMM_SndLlcResumeReq(LL_GMM_RESUME_TYPE_ALL);
                }
            }

        }
        else
        {                                                                       /* RAI不相等或者更新状态不是GU1             */
            NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_NOT_UPDATED); /* 置更新状态为GU2                          */
            if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
            {
                Gmm_SndAgentUsimUpdateFileReq(USIMM_USIM_EFPSLOCI_ID);       /* 更新SIM卡信息                            */
            }
            else
            {
                Gmm_SndAgentUsimUpdateFileReq(USIMM_GSM_EFLOCIGPRS_ID);
            }
            Gmm_ComStaChg(GMM_REGISTERED_ATTEMPTING_TO_UPDATE);                 /* 调用状态的公共处理                       */
        }
        Gmm_TimerStart(GMM_TIMER_T3311);                                        /* 启动Timer3311                            */

    }
    else
    {                                                                           /* ucRauAttmptCnt大于等于5                  */



        NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_NOT_UPDATED);     /* 置更新状态为GU2                          */

        /* 在U2状态，迁到attempting to attach状态保存当前的RAI信息 */
        NAS_GMM_SetAttemptUpdateRaiInfo(NAS_MML_GetCurrCampPlmnInfo());

        g_GmmRauCtrl.ucT3312ExpiredFlg = GMM_FALSE;                             /* 流程结束，清除3312溢出标志               */
        /*==>A32D11635*/
        g_GmmGlobalCtrl.ucGprsResumeFlg = GMM_FALSE;
        /*<==A32D11635*/
        if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
        {
            Gmm_SndAgentUsimUpdateFileReq(USIMM_USIM_EFPSLOCI_ID);       /* 更新SIM卡信息                            */
        }
        else
        {
            Gmm_SndAgentUsimUpdateFileReq(USIMM_GSM_EFLOCIGPRS_ID);
        }
        Gmm_ComStaChg(GMM_REGISTERED_ATTEMPTING_TO_UPDATE);                     /* 调用状态的公共处理                       */

        /* 24008_CR1904R1_(Rel-11)_C1-113602 Handling of timer T3302 24008 4.7.2.x章节描述如下:
        The MS shall apply this value in the routing area registered by the MS, until a new value is received.
        The default value of this timer is used in the following cases:
        -   ATTACH ACCEPT message, ROUTING AREA UPDATE ACCEPT message, ATTACH REJECT message, or ROUTING AREA UPDATE REJECT message is received without a value specified;
        -   In Iu mode, if the network provides a value in a non-integrity protected Iu mode GMM message and the MS is not attaching for emergency services or not attached for emergency services;
        -   the MS does not have a stored value for this timer; or
        -   a new PLMN which is not in the list of equivalent PLMNs has been entered, the routing area updating fails and the routing area updating attempt counter is equal to 5.
        注册失败达5次，且上次下发t3302定时器时长的网络同当前驻留网络不同，则t3302使用默认值。*/
        if (VOS_TRUE == NAS_GMM_IsNeedUseDefaultT3302ValueRauAttempCntMax())
        {
            g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
        }

        Gmm_TimerStart(GMM_TIMER_T3302);                                        /* 启动Timer3302                            */

        NAS_GMM_DeleteEPlmnList();
    }

    if ((GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc)
        || (GMM_RAU_NORMAL       == g_GmmGlobalCtrl.ucSpecProc))
    {                                                                           /* NORMAL RAU                               */
        NAS_GMM_SndMmGmmActionResultInd(GMMMM_ACTION_FAILURE,
                                        ulGmmCause);                             /* 发送MMCGMM_GMM_ACTION_RESULT_IND给MMC    */

    }
    else if (GMM_NULL_PROCEDURE != g_GmmGlobalCtrl.ucSpecProc)
    {
        NAS_GMM_SndMmCombinedRauRejected(ulGmmCause);                              /* 发送MMCGMM_COMBINED_RAU_REJECTED给MMC    */
    }
    else
    {
    }

    /* 向MMC发送PS注册结果 */
    NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                 GMM_MMC_ACTION_RESULT_FAILURE,
                                 (NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16)ulGmmCause);
    /* 事件上报 */
    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_RAU_FAIL,
                    (VOS_VOID*)&ulGmmCause,
                    NAS_OM_EVENT_RAU_FAIL_LEN);

    if (GMM_TRUE == g_GmmGlobalCtrl.ucFollowOnFlg)
    {
        if (GMM_DETACH_COMBINED == g_GmmGlobalCtrl.ucSpecProcHold)
        {
            NAS_GMM_SndMmGprsDetachInitiation();                                   /* 向MMC发送MMCGMM_GPRS_DETACH_INITIATION   */

            if (GMM_WAIT_NULL_DETACH != g_GmmGlobalCtrl.stDetachInfo.enDetachType)
            {
                /* 本地detach后,需要迁移到对应的状态 */
                if (GMM_WAIT_PS_DETACH == (g_GmmGlobalCtrl.stDetachInfo.enDetachType & GMM_WAIT_PS_DETACH))
                {
                    Gmm_ComStaChg(GMM_DEREGISTERED_NORMAL_SERVICE);
                    Gmm_TimerStop(GMM_TIMER_T3302);
                    Gmm_TimerStop(GMM_TIMER_T3311);
                    g_GmmRauCtrl.ucRauAttmptCnt = 0;
                    NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
                }
                NAS_GMM_SndMmcMmDetachInfo();
            }

            g_GmmGlobalCtrl.ucFollowOnFlg = GMM_FALSE;                          /* 清除followon标志                         */
            NAS_MML_SetPsServiceBufferStatusFlg(VOS_FALSE);

            g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;                /* 清除正在进行的specific过程标志           */
        }
        else if (GMM_DETACH_NORMAL == g_GmmGlobalCtrl.ucSpecProcHold)
        {
            g_GmmGlobalCtrl.ucFollowOnFlg = GMM_FALSE;                          /* 清除followon标志                         */
            NAS_MML_SetPsServiceBufferStatusFlg(VOS_FALSE);

            g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;                /* 清除正在进行的specific过程标志           */
            if (GMM_WAIT_NULL_DETACH != g_GmmGlobalCtrl.stDetachInfo.enDetachType)
            {
                /* 本地detach后,需要迁移到对应的状态 */
                if (GMM_WAIT_PS_DETACH == (g_GmmGlobalCtrl.stDetachInfo.enDetachType & GMM_WAIT_PS_DETACH))
                {
                    Gmm_ComStaChg(GMM_DEREGISTERED_NORMAL_SERVICE);
                    Gmm_TimerStop(GMM_TIMER_T3302);
                    Gmm_TimerStop(GMM_TIMER_T3311);
                    g_GmmRauCtrl.ucRauAttmptCnt = 0;
                    NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
                }
                NAS_GMM_SndMmcMmDetachInfo();
            }

            if ( VOS_FALSE == NAS_MML_GetPsAttachAllowFlg() )
            {
                /* 收到网侧的去注册成功后需要通知MM，否则MM不启动T3212定时器 */
                NAS_GMM_SndMmGprsDetachComplete();
            }

        }
        else if (GMM_DETACH_WITH_IMSI == g_GmmGlobalCtrl.ucSpecProcHold)
        {
            NAS_GMM_SndMmImsiDetachInitiation();                                   /* 向MMC发送MMCGMM_IMSI_DETACH_INITIATION   */

            if (GMM_WAIT_NULL_DETACH != g_GmmGlobalCtrl.stDetachInfo.enDetachType)
            {
                NAS_GMM_SndMmcMmDetachInfo();
            }
            g_GmmGlobalCtrl.ucFollowOnFlg = GMM_FALSE;                          /* 清除followon标志                         */
            NAS_MML_SetPsServiceBufferStatusFlg(VOS_FALSE);

            g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;                /* 清除正在进行的specific过程标志           */
        }
        else
        {
        }
    }
    Gmm_ComCnfHandle();

    if (NAS_MML_REG_FAIL_CAUSE_TIMER_TIMEOUT == ulGmmCause)
    {                                                                           /* T3330 time-out 5次                       */
        Gmm_SndRrmmRelReq(RRC_CELL_UNBARRED);                                   /* 释放PS域信令连接                         */
    }

    g_GmmGlobalCtrl.ucSpecProc = GMM_NULL_PROCEDURE;                            /* 清除当前进行的specific流程               */
    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateAttemptCounter:INFO: specific procedure ended");

    return;
}

VOS_VOID Gmm_RoutingAreaUpdataAcceptRaOnly(
    GMM_MSG_RESOLVE_STRU                *pRauAcceptIe,
    NAS_MSG_FOR_PCLINT_STRU             *pMsg
)
{

    NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16  enCause;

    enCause = NAS_MML_REG_FAIL_CAUSE_NULL;

    if (GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG))
    {
        enCause = pMsg->aucNasMsg[pRauAcceptIe->
                aucIeOffset[GMM_RAU_ACCEPT_IE_GMM_CAUSE] + 1 ];                 /* 得到GMM CAUSE                            */
    }


    if ((GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc)
        || (GMM_RAU_NORMAL       == g_GmmGlobalCtrl.ucSpecProc))
    {                                                                           /* 当前的specific过程是NORMAL RAU           */
        Gmm_ComStaChg(GMM_REGISTERED_NORMAL_SERVICE);                           /* 调用状态的公共处理                       */
        g_GmmRauCtrl.ucRauAttmptCnt = 0;                                        /* routing area updating attempt counter清0 */

        NAS_GMM_SndMmGmmActionResultInd(GMMMM_ACTION_SUCCESS,
                                        NAS_MML_REG_FAIL_CAUSE_NULL);                         /* 发送MMCGMM_GMM_ACTION_RESULT_IND给MMC    */



        /* 向MMC发送PS注册结果 */
        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                     GMM_MMC_ACTION_RESULT_SUCCESS,
                                     NAS_MML_REG_FAIL_CAUSE_NULL);

        NAS_EventReport(WUEPS_PID_GMM,
                        NAS_OM_EVENT_RAU_SUCC,
                        VOS_NULL_PTR,
                        NAS_OM_EVENT_NO_PARA);
    }
    else
    {                                                                           /* combined流程                             */
        if ((GMM_TRUE == g_GmmRauCtrl.ucPeriodicRauFlg)
            && (NAS_MML_REG_FAIL_CAUSE_NULL == enCause))
        {
            Gmm_ComStaChg(GMM_REGISTERED_NORMAL_SERVICE);                       /* 调用状态的公共处理                       */
            g_GmmRauCtrl.ucRauAttmptCnt = 0;                                    /* routing area updating attempt counter清0 */
            NAS_GMM_SndMmCombinedRauAccept(GMMMM_RAU_RESULT_COMBINED,
                                           enCause,
                                           pMsg,
                                           pRauAcceptIe);                          /* 发送MMCGMM_COMBINED_RAU_ACCEPTED给MMC    */

            /* 向MMC发送PS注册结果 */
            NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                         GMM_MMC_ACTION_RESULT_SUCCESS,
                                         NAS_MML_REG_FAIL_CAUSE_NULL);
            /*更新RAU记数器 */
            if(5<=g_GmmRauCtrl.ucRauAttmptCnt)
            {
                g_GmmRauCtrl.ucRauAttmptCnt = 0;
            }

            /*  事件上报 */
            NAS_EventReport(WUEPS_PID_GMM,
                            NAS_OM_EVENT_RAU_SUCC,
                            VOS_NULL_PTR,
                            NAS_OM_EVENT_NO_PARA);

            g_GmmGlobalCtrl.ucSpecProc    = GMM_NULL_PROCEDURE;                 /* 清除当前进行的specific 流程标志          */
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdataAcceptRaOnly:INFO: specific procedure ended");
            g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;                          /* 清除当前RAU是否是周期RAU标志             */
            return;
        }


        /* R11升级:   24008 4.7.3.2.3.2    Other GMM causevalues and the case that no GMM cause IE
           was received are considered as abnormal cases. The combined attach procedure shall be
           considered as failed for non-GPRS services */
        if ((GMM_RAU_WITH_IMSI_ATTACH == g_GmmGlobalCtrl.ucSpecProc)
         || (GMM_RAU_COMBINED         == g_GmmGlobalCtrl.ucSpecProc))
        {                                                                           /* 当前发起的联合注册 */
            /* 若失败原因值不为#2,#16,#17,#22,直接将原因值替换为#16 */
            if ((NAS_MML_REG_FAIL_CAUSE_MSC_UNREACHABLE      != enCause)
             && (NAS_MML_REG_FAIL_CAUSE_NETWORK_FAILURE      != enCause)
             && (NAS_MML_REG_FAIL_CAUSE_PROCEDURE_CONGESTION != enCause)
             && (NAS_MML_REG_FAIL_CAUSE_IMSI_UNKNOWN_IN_HLR  != enCause))
            {
                enCause = NAS_MML_REG_FAIL_CAUSE_MSC_UNREACHABLE;
            }
        }


        if (NAS_MML_REG_FAIL_CAUSE_IMSI_UNKNOWN_IN_HLR == enCause)
        {                                                                       /* 原因值为2                                */
            
            g_GmmRauCtrl.ucRauAttmptCnt = 0;
            Gmm_TimerStop(GMM_TIMER_T3330);
            Gmm_ComStaChg(GMM_REGISTERED_NORMAL_SERVICE);
        }
        else if ((NAS_MML_REG_FAIL_CAUSE_MSC_UNREACHABLE         == enCause)
                 || (NAS_MML_REG_FAIL_CAUSE_NETWORK_FAILURE      == enCause)
                 || (NAS_MML_REG_FAIL_CAUSE_PROCEDURE_CONGESTION == enCause))
        {                                                                       /* 原因值为16,17,22                         */
            Gmm_AttachRauAcceptCause16(enCause);                                       /* 调用原因值16，17，22处理函数             */
        }
        else
        {

        }
        if((NAS_MML_NET_RAT_TYPE_WCDMA == NAS_MML_GetCurrNetRatType())
            ||(5 > g_GmmRauCtrl.ucRauAttmptCnt))
        {
            NAS_GMM_SndMmCombinedRauAccept(GMMMM_RAU_RESULT_PS_ONLY,
                                           enCause,
                                           pMsg,
                                           pRauAcceptIe);                          /* 发送MMCGMM_COMBINED_RAU_ACCEPTED给MMC    */


            /* 向MMC发送PS注册结果 */

            NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                         GMM_MMC_ACTION_RESULT_SUCCESS,
                                         enCause);


            /* 事件上报 */
            NAS_EventReport(WUEPS_PID_GMM,
                            NAS_OM_EVENT_RAU_SUCC,
                            VOS_NULL_PTR,
                            NAS_OM_EVENT_NO_PARA
                            );
        }
        g_GmmGlobalCtrl.ucSpecProc    = GMM_NULL_PROCEDURE;                     /* 清除当前进行的specific 流程标志          */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdataAcceptRaOnly:INFO: specific procedure ended");
        g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;                              /* 清除当前RAU是否是周期RAU标志             */
    }

    return;
}

VOS_VOID Gmm_RoutingAreaUpdataAcceptCombined(
                                     GMM_MSG_RESOLVE_STRU    *pRauAcceptIe,     /* 指向GMM_MSG_RESOLVE_STRU结构的指针       */
                                     NAS_MSG_FOR_PCLINT_STRU *pMsg
                                     )
{
    g_GmmRauCtrl.ucRauAttmptCnt = 0;                                            /* routing area updating attempt counter清0 */
    Gmm_ComStaChg(GMM_REGISTERED_NORMAL_SERVICE);                               /* 调用状态的公共处理                       */

    if ((GMM_RAU_NORMAL             == g_GmmGlobalCtrl.ucSpecProc)
        || (GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc))
    {                                                                           /* 当前的specific过程是NORMAL RAU           */
        NAS_GMM_SndMmGmmActionResultInd(GMMMM_ACTION_SUCCESS,
                                        NAS_MML_REG_FAIL_CAUSE_NULL);     /* 发送MMCGMM_GMM_ACTION_RESULT_IND给MMC    */


        /* 向MMC发送PS注册结果 */
        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                     GMM_MMC_ACTION_RESULT_SUCCESS,
                                     NAS_MML_REG_FAIL_CAUSE_NULL);


    }
    else
    {                                                                           /* combined流程                             */
        NAS_GMM_SndMmCombinedRauAccept(GMMMM_RAU_RESULT_COMBINED,
                                       NAS_MML_REG_FAIL_CAUSE_NULL,
                                       pMsg,
                                       pRauAcceptIe);                              /* 发送MMCGMM_COMBINED_RAU_ACCEPTED给MMC    */


        /* 向MMC发送PS注册结果 */
        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                     GMM_MMC_ACTION_RESULT_SUCCESS,
                                     NAS_MML_REG_FAIL_CAUSE_NULL);


        g_GmmGlobalCtrl.ucSpecProc    = GMM_NULL_PROCEDURE;                     /* 清除当前进行的specific 流程标志          */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdataAcceptCombined:INFO: specific procedure ended");
        g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;                              /* 清除当前RAU是否是周期RAU标志             */

        if ((GMM_NET_MODE_II == g_GmmGlobalCtrl.ucNetMod)
            && (GMM_NET_MODE_I == g_GmmGlobalCtrl.SysInfo.ucNetMod))
        {                                                                       /* 网络模式于实际不一致                     */
            g_GmmGlobalCtrl.ucNetMod = GMM_NET_MODE_I;                          /* 改网络模式为I                            */
        }

    }


    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_RAU_SUCC,
                    VOS_NULL_PTR,
                    NAS_OM_EVENT_NO_PARA);

    return;
}

VOS_UINT32 NAS_GMM_CheckNpduValid(
    VOS_UINT8                           ucNpduLen,
    VOS_UINT8                          *pucNpdu
)
{
    VOS_UINT8                           ucLoop;
    VOS_UINT8                           ucNsapi;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucNpduNumber;

    if (ucNpduLen > GMM_NPDU_MAX_LENGTH)
    {
        return VOS_FALSE;
    }

    /*获得N-PDU的个数:*/
    ucNpduNumber = ( VOS_UINT8 )((ucNpduLen * 8) / 12);

    /*遍历N-PDU:*/
    for ( ucLoop = 1; ucLoop <= ucNpduNumber; ucLoop++ )
    {
        if ( 1 == (ucLoop % 2) )
        {
            /*得到N-PDU value在数组中的索引(序号为奇数):*/
            ucIndex  = ( VOS_UINT8 )(( ( ucLoop - 1 ) * 12 ) / 8);

            /*获取NSAPI号:*/
            ucNsapi = (pucNpdu[ucIndex] >> 4) & 0x0f;
        }
        else
        {
            /*得到N-PDU value在数组中的索引(序号为偶数):*/
            ucIndex  = ( VOS_UINT8 )(( ucLoop * 12 ) / 8);

            /*获取NSAPI号:*/
            ucNsapi = pucNpdu[ucIndex] & 0x0f;
        }

        if ((ucNsapi > GMM_NSAPI_MAX_VALUE) || (ucNsapi < GMM_NSAPI_MIN_VALUE))
        {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}


VOS_UINT32 NAS_GMM_DecodeT3312ExtendedValueIE_RcvRauAccept(
    GMM_MSG_RESOLVE_STRU               *pRauAcceptIe,
    NAS_MSG_FOR_PCLINT_STRU            *pMsg,
    VOS_UINT8                           ucIeOffset
)
{
    if ((ucIeOffset + NAS_GMM_IE_T3312_EXTENDED_VALUE_LEN) > (VOS_UINT8)pMsg->ulNasMsgSize)
    {
        return VOS_FALSE;
    }

    if (NAS_GMM_RAU_ACCEPT_IE_T3312_EXTENDED_VALUE_FLG == (pRauAcceptIe->ulOptionalIeMask & NAS_GMM_RAU_ACCEPT_IE_T3312_EXTENDED_VALUE_FLG))
    {
        return VOS_TRUE;
    }

    if (PS_PTL_VER_R11 <= NAS_Common_Get_Supported_3GPP_Version(MM_COM_SRVDOMAIN_PS))
    {
        NAS_GMM_SaveGprsTimer3Value(GMM_TIMER_T3312, pMsg->aucNasMsg[ucIeOffset + 2]);
    }

    pRauAcceptIe->ulOptionalIeMask |= NAS_GMM_RAU_ACCEPT_IE_T3312_EXTENDED_VALUE_FLG;
    pRauAcceptIe->aucIeOffset[NAS_GMM_RAU_ACCEPT_IE_T3312_EXTENDED_VALUE] = ucIeOffset;

    return VOS_TRUE;
}



VOS_VOID Gmm_RoutingAreaUpdateResolveIe(
                                    GMM_MSG_RESOLVE_STRU    *pRauAcceptIe,      /* 指向GMM_MSG_RESOLVE_STRU结构的指针       */
                                    NAS_MSG_FOR_PCLINT_STRU *pMsg               /* 指向NAS_MSG_FOR_PCLINT_STRU结构          */
                                    )
{
    VOS_UINT8   ucEndFlg    = GMM_FALSE;
    VOS_UINT8   ucIeOffset  = GMM_MSG_LEN_RAU_ACCEPT;                           /* 定义临时变量存储存储IE的偏移量           */

    VOS_UINT16                          usIeOffsetAdapt;

    usIeOffsetAdapt         = 0;

    if (NAS_MML_NET_RAT_TYPE_WCDMA == NAS_MML_GetCurrNetRatType())
    {
        if (0 == ((pMsg->aucNasMsg[2] >> 4) & 0x08))
        {
            g_GmmGlobalCtrl.ucFopFlg = GMM_TRUE;
        }
        else
        {
            g_GmmGlobalCtrl.ucFopFlg = GMM_FALSE;
        }
    }

    for (; (ucIeOffset<pMsg->ulNasMsgSize) && (GMM_FALSE == ucEndFlg); )
    {                                                                           /* 存储IE的偏移量小于空口消息的长度         */
        switch (pMsg->aucNasMsg[ucIeOffset])
        {                                                                       /* IEI                                      */
        case GMM_IEI_P_TMSI_SIGNATURE:                                          /* P-TMSI signature                         */
            if ((ucIeOffset + 4) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"P-TMSI signature\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE_FLG))
            {
                ucIeOffset += 4;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE]
                = ucIeOffset;
            ucIeOffset += 4;
            break;
        case GMM_IEI_ALLOCATED_PTMSI:                                           /* Allocated P-TMSI                         */
            if ((ucIeOffset + 7) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"P-TMSI\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_PTMSI_FLG ==
                (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_PTMSI_FLG))
            {
                ucIeOffset += 7;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_PTMSI_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_PTMSI]
                = ucIeOffset;
            ucIeOffset += 7;
            break;
        case GMM_IEI_MS_IDENTITY:                                               /* MS identity                              */
            if (GMM_MOBILE_ID_IMSI == (pMsg->aucNasMsg[ucIeOffset + 2] & 0x07))
            {                                                                   /* IE中是IMSI                               */
                if ((ucIeOffset + 10) > (VOS_UINT8)pMsg->ulNasMsgSize)
                {
                    /* IMSI存在时,即使长度不对,也需要处理 */
                    pRauAcceptIe->ulOptionalIeMask
                        |= GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG;
                    pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_MS_IDENTITY]
                        = ucIeOffset;
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"MS ID(IMSI)\" in RAU ACCEPT is invalid");
                    return;
                }
                if (GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG ==
                    (pRauAcceptIe->ulOptionalIeMask
                    & GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG))
                {
                    ucIeOffset += 10;
                    break;
                }
            }
            else
            {
                if ((ucIeOffset + 7) > (VOS_UINT8)pMsg->ulNasMsgSize)
                {
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"MS ID(TMSI)\" in RAU ACCEPT is invalid");
                    return;
                }
                if (GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG ==
                    (pRauAcceptIe->ulOptionalIeMask
                    & GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG))
                {
                    ucIeOffset += 7;
                    break;
                }
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_MS_IDENTITY]
                = ucIeOffset;
            if (GMM_MOBILE_ID_IMSI == (pMsg->aucNasMsg[ucIeOffset + 2] & 0x07))
            {                                                                   /* IE中是IMSI                               */
                ucIeOffset += 10;
            }
            else
            {                                                                   /* IE中是TMSI                               */
                ucIeOffset += 7;
            }
            break;
        case GMM_IEI_LIST_OF_RECEIVE_N_PDU_NUMBERS:                             /* Allocated P-TMSI                         */
            if ((ucIeOffset + (2 + pMsg->aucNasMsg[ucIeOffset + 1]))
                 > (VOS_UINT8)pMsg->ulNasMsgSize )
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"N-PDU\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_NPDU_NUMBER_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_NPDU_NUMBER_FLG))
            {
                ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
                break;
            }

            if (VOS_TRUE == NAS_GMM_CheckNpduValid(pMsg->aucNasMsg[ucIeOffset + 1], &(pMsg->aucNasMsg[ucIeOffset + 2])))
            {
                pRauAcceptIe->ulOptionalIeMask
                    |= GMM_RAU_ACCEPT_IE_NPDU_NUMBER_FLG;
            }
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_NPDU_NUMBER]
                = ucIeOffset;
            ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
            break;
        case GMM_IEI_NEGOTIATED_READY_TIMER_VALUE:                              /* Negotiated READY timervalue              */
            if ((ucIeOffset + 2) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"READY TIMER value\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_READY_TIMER_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_READY_TIMER_FLG))
            {
                ucIeOffset += 2;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_READY_TIMER_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_READY_TIMER]
                = ucIeOffset;
            ucIeOffset += 2;
            break;
        case GMM_IEI_GMM_CAUSE:                                                 /* GMM cause                                */
            if ((ucIeOffset + 2) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"GMM cause\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG))
            {
                ucIeOffset += 2;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_GMM_CAUSE]
                = ucIeOffset;
            ucIeOffset += 2;
            break;
        case GMM_IEI_T3302_VALUE:                                               /* T3302 value                              */
            if ((ucIeOffset + 3) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"T3302 value\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_T3302_VALUE_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_T3302_VALUE_FLG))
            {
                ucIeOffset += 3;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_T3302_VALUE_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_T3302_VALUE]
                = ucIeOffset;
            ucIeOffset += 3;
            break;
        case GMM_IEI_CELL_NOTIFICATION:                                         /* Cell Notification                        */
            if ((ucIeOffset + 1) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"Cell notification\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION_FLG))
            {
                ucIeOffset += 1;
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION]
                = ucIeOffset;
            ucIeOffset += 1;
            break;
        case GMM_IEI_EQUIVALENT_PLMNS:                                          /* Equivalent PLMNs                         */
            if ((ucIeOffset + (2 + pMsg->aucNasMsg[ucIeOffset + 1]))
                 > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"Equivalent PLMNs\" in RAU ACCEPT is invalid");
                return;
            }
            if (GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN_FLG ==
                (pRauAcceptIe->ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN_FLG))
            {
                ucIeOffset += 2 + pMsg->aucNasMsg[ucIeOffset + 1];
                break;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN]
                = ucIeOffset;
            ucIeOffset += 2 + pMsg->aucNasMsg[ucIeOffset + 1];
            break;
        case GMM_IEI_PDP_CONTEXT_STATUS:
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_PDP_CONTEXT_STATUS_FLG;
            pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_PDP_CONTEXT_STATUS]
                = ucIeOffset;
            ucIeOffset += 4;
            break;
        case GMM_IEI_EMERGENCY_NUMBER_LIST:                                     /* Emergency Number List                    */
            if ( (ucIeOffset + (2 + pMsg->aucNasMsg[ucIeOffset + 1]))
                 > (VOS_UINT8)pMsg->ulNasMsgSize )
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"emergency number\" length error in ATTACH ACCEPT");
                return;
            }
            pRauAcceptIe->ulOptionalIeMask
                |= GMM_RAU_ACCEPT_IE_EMERGENCY_NUMBER_LIST_FLG;

            usIeOffsetAdapt = ucIeOffset;
            (VOS_VOID)NAS_MML_DecodeEmergencyNumList(&usIeOffsetAdapt, pMsg->aucNasMsg, (VOS_UINT16)pMsg->ulNasMsgSize);
            ucIeOffset      = (VOS_UINT8)usIeOffsetAdapt;


            break;

        case NAS_GMM_IEI_T3323_VALUE:
            if ((ucIeOffset + NAS_GMM_IE_T3323_VALUE_LEN) > (VOS_UINT8)pMsg->ulNasMsgSize)
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateResolveIe:WARNING: IE\"T3302 value\" in RAU ACCEPT is invalid");
                return;
            }

            if (GMM_RAU_ACCEPT_IE_T3323_VALUE_FLG == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_T3323_VALUE_FLG))
            {
                ucIeOffset += NAS_GMM_IE_T3323_VALUE_LEN;
                break;
            }

            Gmm_SaveTimerValue(GMM_TIMER_T3323, pMsg->aucNasMsg[ucIeOffset + 2]);

            pRauAcceptIe->ulOptionalIeMask |= GMM_RAU_ACCEPT_IE_T3323_VALUE_FLG;
            pRauAcceptIe->aucIeOffset[NAS_GMM_RAU_ACCEPT_IE_T3323_VALUE] = ucIeOffset;
            ucIeOffset += NAS_GMM_IE_T3323_VALUE_LEN;
            break;

        case NAS_GMM_IEI_T3312_EXTENDED_VALUE:
            if (VOS_FALSE == NAS_GMM_DecodeT3312ExtendedValueIE_RcvRauAccept(pRauAcceptIe, pMsg, ucIeOffset))
            {
                return;
            }

            ucIeOffset += NAS_GMM_IE_T3312_EXTENDED_VALUE_LEN;
            break;

        default:
            if(GMM_IEI_NETWORK_FEATURE_SUPPORT == (pMsg->aucNasMsg[ucIeOffset] & 0xF0))
            {/* Network feature support                  */
                ucIeOffset += 1;
            }
            else if (GMM_IEI_REQUESTED_MS_INFORMATION == (pMsg->aucNasMsg[ucIeOffset] & 0xF0))
            {/* Requested MS information                 */
                pRauAcceptIe->ulOptionalIeMask
                    |= GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION_FLG;
                pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_REQUESTED_MS_INFORMATION]
                    = ucIeOffset;
                ucIeOffset += 1;
            }
            /* TV类型的IE, reference 27.007 11.2.4 */
            else if(GMM_IEI_FORMAT_T_TYPE == (pMsg->aucNasMsg[ucIeOffset] & GMM_IEI_FORMAT_TYPE_INFO))
            {
                ucIeOffset += 1;
            }
            /* TLV类型的IE */
            /*lint -e961*/
            else if (ucIeOffset + 2 + pMsg->aucNasMsg[ucIeOffset + 1] < pMsg->ulNasMsgSize)
            /*lint +e961*/
            {
                ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
            }
            else
            {
                ucEndFlg = GMM_TRUE;
            }
            break;
        }
    }
    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_Under_Gsm(
                                 VOS_UINT8               ucInterRatInfoFlg,
                                 VOS_UINT8               ucReadychangFLG,
                                 VOS_BOOL                bTlliUpdateFlag,
                                 VOS_UINT8               ucSndCompleteFlg
                                 )
{
    NAS_MSG_STRU               *pSendNasMsg      = VOS_NULL_PTR;                /* 定义临时变量                             */

    /* 通知RABM获取的N-PDU Number */
    Gmm_TimerStart(GMM_TIMER_RAU_RSP);                                          /* 消息接受保护定时器 */

    if (VOS_FALSE == bTlliUpdateFlag)
    {
        if (GMM_TRUE == gstGmmCasGlobalCtrl.ucRauCmpFlg)
        {
            g_GmmRauCtrl.ucNpduCnt = g_GmmDownLinkNpduBake.ucNpduCnt;
            Gmm_MemCpy(g_GmmRauCtrl.aucNpduNum, g_GmmDownLinkNpduBake.aucNpduNum, g_GmmRauCtrl.ucNpduCnt);
            gstGmmCasGlobalCtrl.ucRauCmpFlg = GMM_FALSE;
            pSendNasMsg = Gmm_RoutingAreaUpdateCompleteMsgMake();

            Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH, pSendNasMsg);

            /*只有前面进行过RAU suspend，此处resume才有意义*/
            if (GMM_SUSPEND_LLC_FOR_RAU == (gstGmmCasGlobalCtrl.ucSuspendLlcCause & GMM_SUSPEND_LLC_FOR_RAU))
            {
                gstGmmCasGlobalCtrl.ucSuspendLlcCause &= ~GMM_SUSPEND_LLC_FOR_RAU;

                if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
                {
                    GMM_SndLlcResumeReq(LL_GMM_RESUME_TYPE_ALL);                               /* 恢复LLC数据传输 */
                }
            }

            GMM_InitCellUpdate(ucReadychangFLG);                                               /* 小区更新，生效新的READY TIMER */

            g_GmmGlobalCtrl.ucSpecProc = GMM_NULL_PROCEDURE;
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandle_Under_Gsm:INFO: specific procedure ended");
            g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;
        }
        GMM_LOG_WARN("Gmm_RoutingAreaUpdateHandle():WARNING: receive rau accept again!");
    }
    else if (GMM_RAU_FOR_INTERSYSTEM == gstGmmSuspendCtrl.ucRauCause)
    {
        GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_HANDOVER, GMM_RABM_RAU_SUCCESS);
    }
    else
    {
        GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_NORMAL,GMM_RABM_RAU_SUCCESS);
    }

    if (GMM_TRUE == ucReadychangFLG)
    {
        NAS_GMM_SndGasInfoChangeReq(NAS_GSM_MASK_READY_TIMER);
    }

    /* 只有在不需要发送RAU COMPLETE消息时才需要通知LLC发送cell update，
       因为在重复收到网络的RAU ACCEPT消息时，在前面发送RAU COMPLETE后，
       已经把ucRauCmpFlg清除，所以此时需要使用其它标志来判断是否需要发送
       RAU COMPLETE
    */
    if (GMM_TRUE == ucSndCompleteFlg)
    {
        gstGmmCasGlobalCtrl.ucCellUpdateFlg = GMM_FALSE;
        GMM_InitCellUpdate(ucReadychangFLG);                                               /* only restart ready timer */

    }
    else
    {
        /*只有前面进行过RAU suspend，此处resume才有意义*/
        if (GMM_SUSPEND_LLC_FOR_RAU == (gstGmmCasGlobalCtrl.ucSuspendLlcCause & GMM_SUSPEND_LLC_FOR_RAU))
        {
            gstGmmCasGlobalCtrl.ucSuspendLlcCause &= ~GMM_SUSPEND_LLC_FOR_RAU;

            if(GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
            {
                GMM_SndLlcResumeReq(LL_GMM_RESUME_TYPE_ALL);                            /* 恢复LLC数据传输 */
            }
        }
        if (0 != (g_GmmTimerMng.ulTimerValMask & GMM_TIMER_T3314_FLG))      /* 3314 value change */
        {
            gstGmmCasGlobalCtrl.ucCellUpdateFlg = GMM_TRUE;

            /* 根据24.008 4.7.2.1.1 If a new READY timer value is negotiated,
            the MS shall upon the reception of the ATTACH ACCEPT or ROUTING
            AREA UPDATE ACCEPT message perform a initial cell update
            (either by transmitting a LLC frame or, if required, a ATTACH
            COMPLETE or ROUTING AREA UPDATE COMPLETE message), in order to
            apply the new READY timer value immediately. If both the network
            and the MS supports the Cell Notification, the initial cell
            update shall useany LLC frame except the LLC NULL frame. */
            GMM_InitCellUpdate(ucReadychangFLG);                                           /* CELL update&restart ready timer */
        }

        g_GmmGlobalCtrl.ucSpecProc    = GMM_NULL_PROCEDURE;
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandle:INFO: specific procedure ended");
        g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;
        if ( (0 != g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk)
           &&(GMM_TRUE != g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg))
        {                                                                   /* 有保留的消息并且当前没有缓存的RAU过程                             */
            Gmm_DealWithBuffAfterProc();
        }
    }

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_Under_W(VOS_VOID)
{
    if (VOS_TRUE == NAS_MML_IsPsBearerExist())
    {

        if (GMM_RAU_FOR_INTERSYSTEM == gstGmmSuspendCtrl.ucRauCause)
        {
            GMM_RauSuccessInterSys();
        }
        else
        {
            GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_NORMAL, GMM_RABM_RAU_SUCCESS);
        }
    }
    else
    {
        if (GMM_RAU_FOR_INTERSYSTEM != gstGmmSuspendCtrl.ucRauCause)
        {
            GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_NORMAL, GMM_RABM_RAU_SUCCESS);
        }
    }

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_T3302_Handling(GMM_MSG_RESOLVE_STRU           *pRauAcceptIe,
                                                    NAS_MSG_FOR_PCLINT_STRU        *pMsg)
{
    VOS_UINT8                           ucIsUtranSmcNeeded;

    NAS_MML_PLMN_WITH_RAT_STRU          stPlmnWithRat;

    PS_MEM_SET(&stPlmnWithRat, 0, sizeof(stPlmnWithRat));

    ucIsUtranSmcNeeded = NAS_UTRANCTRL_IsUtranPsSmcNeeded();

    if (GMM_RAU_ACCEPT_IE_T3302_VALUE_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_T3302_VALUE_FLG))
    {                                                                           /* 有T3302IE                                */
        /* 24.008: 9.4.15.7
          In Iu mode, if this message is received without integrity protection
          the MS shall ignore the contents of this IE and use the last received value if available.
          If there is no last received value, the MS shall use the default value. */

        if ((VOS_TRUE                             == ucIsUtranSmcNeeded)
         && (NAS_MML_RRC_INTEGRITY_PROTECT_ACTIVE != g_MmSubLyrShare.GmmShare.ucPsIntegrityProtect))
        {
            if (0 == g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal)
            {
                g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
            }
        }
        else
        {
            Gmm_SaveTimerValue(GMM_TIMER_T3302,
                               pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                               [GMM_RAU_ACCEPT_IE_T3302_VALUE] + 2]);               /* 存储T3302时长                            */

            PS_MEM_CPY(&stPlmnWithRat.stPlmnId, NAS_MML_GetCurrCampPlmnId(),sizeof(stPlmnWithRat.stPlmnId));
            stPlmnWithRat.enRat = NAS_MML_GetCurrNetRatType();

            NAS_GMM_SetAllocT3302ValuePlmnWithRat(&stPlmnWithRat);
        }
    }
    else
    {
        /* 24.008: 9.4.15.7
          If this IE is not included in the message in A/Gb mode or
          if in Iu mode this IE is not included in an integrity protected message,
          the MS shall use the default value. */

        if ( (VOS_FALSE == ucIsUtranSmcNeeded)
          || ( (VOS_TRUE == ucIsUtranSmcNeeded)
            && (NAS_MML_RRC_INTEGRITY_PROTECT_ACTIVE == g_MmSubLyrShare.GmmShare.ucPsIntegrityProtect) ) )
        {
            g_GmmTimerMng.aTimerInf[GMM_TIMER_T3302].ulTimerVal = GMM_TIMER_T3302_VALUE;
        }
    }

    return;
}


VOS_VOID NAS_GMM_HandleRoutingAreaUpdateEPlmn(
    GMM_MSG_RESOLVE_STRU               *pstRauAcceptIe,
    NAS_MSG_FOR_PCLINT_STRU            *pstMsg
)
{
    NAS_MML_EQUPLMN_INFO_STRU          *pstEPlmnList    = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU               *pstCurrCampPlmn = VOS_NULL_PTR;
    NAS_MML_EQUPLMN_INFO_STRU           stTmpEPlmnList;


    PS_MEM_SET(&stTmpEPlmnList, 0x00, sizeof(stTmpEPlmnList));

    pstEPlmnList    = NAS_MML_GetEquPlmnList();
    pstCurrCampPlmn = NAS_MML_GetCurrCampPlmnId();

    /* 如果空口消息中没有EPLMN,保存当前网络为EPLMN */
    if (GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN_FLG
        != (pstRauAcceptIe->ulOptionalIeMask
        & GMM_RAU_ACCEPT_IE_EQUIVALENT_PLMN_FLG))
    {
        stTmpEPlmnList.ucEquPlmnNum = 0x1;
        stTmpEPlmnList.astEquPlmnAddr[0] = *pstCurrCampPlmn;
    }
    else
    {
        /* 解析消息中EPLMN */
        NAS_GMM_DecodeEquPlmnInfoIE(GMM_MMC_ACTION_RAU,
                                pstRauAcceptIe,
                                pstMsg,
                                &stTmpEPlmnList);
    }

    /* 空口消息中EPLMN与保存的不同,将EPLMN保存到MML全局变量和更新NV */

    /* 将EPLMN保存到MML全局变量 */
    PS_MEM_CPY(pstEPlmnList, &stTmpEPlmnList, sizeof(NAS_MML_EQUPLMN_INFO_STRU));

    NAS_MML_SetEplmnValidFlg(VOS_TRUE);

    /* 将EPLMN更新到NV中 */
    NAS_GMM_WriteEplmnNvim(pstEPlmnList);

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_PTMSI_SIGNATURE_Handling(GMM_MSG_RESOLVE_STRU   *pRauAcceptIe,
                                                    NAS_MSG_FOR_PCLINT_STRU          *pMsg)
{
    if (GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE_FLG
        == (pRauAcceptIe->ulOptionalIeMask
        & GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE_FLG))
    {                                                                           /* 空口消息中有P-TMSI signature             */
        NAS_MML_SetUeIdPtmsiSignature(&(pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                   [GMM_RAU_ACCEPT_IE_PTMSI_SIGNATURE] + 1]));                                                          /* 存储P-TMSI signature                     */

        g_GmmGlobalCtrl.UeInfo.UeId.ucUeIdMask |= GMM_UEID_P_TMSI_SIGNATURE;    /* 设置UE ID存在标志                        */

    }
    else
    {
        NAS_MML_InitUeIdPtmsiSignatureInvalid();

        g_GmmGlobalCtrl.UeInfo.UeId.ucUeIdMask &= ~GMM_UEID_P_TMSI_SIGNATURE;   /* 清UE ID存在标志                          */

    }

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_READY_TIMER_Handling(GMM_MSG_RESOLVE_STRU   *pRauAcceptIe,
                                                    NAS_MSG_FOR_PCLINT_STRU      *pMsg,
                                                    VOS_UINT8                    *pucReadychangFLG)
{
    if (GMM_RAU_ACCEPT_IE_READY_TIMER_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_READY_TIMER_FLG))
    {
        *pucReadychangFLG = GMM_TRUE;
       GMM_SaveReadyTimerValue(pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset\
                [GMM_RAU_ACCEPT_IE_READY_TIMER] + 1]);
    }
    else
    {
        if (GMM_TRUE == gstGmmCasGlobalCtrl.ucReadyTimerNgtFlg)
        {
            *pucReadychangFLG = GMM_TRUE;
            GMM_SaveReadyTimerValue(GMM_REQUESTED_READY_TIMER_VALUE);
            gstGmmCasGlobalCtrl.ucReadyTimerNgtFlg = GMM_FALSE;
        }
    }

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_Handling_Local_TLLI(VOS_BOOL                bTlliUpdateFlag)
{
    VOS_UINT32                  ulNewTlli        = 0;

    if (VOS_TRUE == bTlliUpdateFlag)
    {
        GMM_CharToUlong(&ulNewTlli,
                        NAS_MML_GetUeIdPtmsi());                                /* 新的PTMSI */
        GMM_CreateNewTlli(&ulNewTlli, GMM_LOCAL_TLLI);                          /* 生成新TLLI */
        if ( GMM_FALSE == GMM_AssignNewTlli(ulNewTlli, GMM_LOCAL_TLLI) )
        {
            GMM_LOG_WARN("Gmm_RoutingAreaUpdateHandle():Error: new TLLI assigned unsuccessfully!");
        }
        else
        {
            Gmm_TimerStart(GMM_TIMER_PROTECT_OLD_TLLI);
        }
    }
    else
    {
        Gmm_TimerStop(GMM_TIMER_PROTECT_OLD_TLLI);
        Gmm_TimerStart(GMM_TIMER_PROTECT_OLD_TLLI);
    }

    return;
}


VOS_VOID Gmm_RoutingAreaUpdateHandle_Handling_RAU_RESULT_IE(
                                 GMM_MSG_RESOLVE_STRU           *pRauAcceptIe,          /* 指向GMM_MSG_RESOLVE_STRU结构的指针       */
                                 NAS_MSG_FOR_PCLINT_STRU        *pMsg,
                                 VOS_UINT8                      *pucSndCompleteFlg,
                                 VOS_BOOL                        bTlliUpdateFlag
                                 )
{
    VOS_UINT8                           ucUpdateResultValue;

    ucUpdateResultValue = (pMsg->aucNasMsg[2] >> NAS_MML_OCTET_MOVE_FOUR_BITS) & NAS_MML_OCTET_LOW_THREE_BITS;

    if ((GMM_COMBINED_RALA_UPDATED == ucUpdateResultValue)
     || (GMM_COMBINED_RALA_UPDATED_ISR_ACTIVE == ucUpdateResultValue))
    {                                                                           /* RAU结果是combined RA/LA updated          */
        if (VOS_TRUE == NAS_MML_GetCsAttachAllowFlg())
        {
            g_MmSubLyrShare.GmmShare.ucGsAssociationFlg = VOS_TRUE;
        }
        else
        {
            g_MmSubLyrShare.GmmShare.ucGsAssociationFlg = VOS_FALSE;
        }
        Gmm_RoutingAreaUpdataAcceptCombined(pRauAcceptIe, pMsg);                /* 调用函数处理combined RA/LA updated       */

        if (GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG
            == (pRauAcceptIe->ulOptionalIeMask
            & GMM_RAU_ACCEPT_IE_MS_IDENTITY_FLG))
        {                                                                       /* 该IE存在                                 */
            if (GMM_MOBILE_ID_TMSI_PTMSI
                == (GMM_MOBILE_ID_TMSI_PTMSI &
                pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                [GMM_RAU_ACCEPT_IE_MS_IDENTITY] + 2]))
            {                                                                   /* IE_MS_IDENTITY中携带TMSI                 */
                *pucSndCompleteFlg = GMM_TRUE;                                  /* 设定需要发送RAU COMPLETE消息             */
            }
        }
    }
    else
    {
        /* 周期性RAU时认为联合注册成功,不需要清除GS口 */
        if (VOS_FALSE == g_GmmRauCtrl.ucPeriodicRauFlg)
        {
            g_MmSubLyrShare.GmmShare.ucGsAssociationFlg = VOS_FALSE;
        }

        /* 如果bTlliUpdateFlag是FALSE，说明是RAU ACCEPT重复收到，
           此时不需要再次通知MM/MMC RAU结果,因为此时如果通知，
           RAU的类型是错误的
        */
        if(VOS_TRUE == bTlliUpdateFlag)
        {
            Gmm_RoutingAreaUpdataAcceptRaOnly(pRauAcceptIe, pMsg);              /* 调用函数处理RA only updated              */
        }
    }

    return;
}

/***********************************************************************
 *  MODULE   : Gmm_RoutingAreaUpdateHandle_Handling_SndComplete_Flag
 *  FUNCTION : RAU流程是否发送RAU COMPLETE消息的处理
 *  INPUT    : VOS_VOID
 *  OUTPUT   : VOS_VOID
 *  RETURN   : VOS_UINT8 ucEventID
 *  NOTE     :
 *  HISTORY  :
     1.  欧阳飞   2009.06.11  新版作成
 ************************************************************************/
VOS_VOID Gmm_RoutingAreaUpdateHandle_Handling_SndComplete_Flag(
    VOS_UINT8                   ucSndCompleteFlg,
    VOS_UINT8                   ucInterRatInfoFlg
)
{
    NAS_MSG_STRU                *pSendNasMsg = VOS_NULL_PTR;
    VOS_UINT32                  ulRst;

    if (VOS_TRUE == ucSndCompleteFlg)
    {
        /* 需要向WRR获取InterRatInfo信息 */
        if (VOS_TRUE == ucInterRatInfoFlg)
        {
            ulRst = NAS_GMM_SndRrmmInterRatHandoverInfoReq();

            if (VOS_OK == ulRst)
            {
                gstGmmCasGlobalCtrl.ucRauCmpFlg = VOS_TRUE;

                /* 需要等待WRR回复后，发送RAU COMPLETE 消息  */
                g_GmmInterRatInfoCtrl.ucRauCmpWaitInterRatCnfMsg = VOS_TRUE;
                g_GmmInterRatInfoCtrl.ucInterRatCnfMsgRcvdFlg = VOS_FALSE;
                g_GmmInterRatInfoCtrl.ucDataLen = 0;
            }
        }

        /* G下，需要等待RABM回复后，再发送RAU COMPLETE 消息  */
        if (VOS_TRUE == GMM_IsCasGsmMode())
        {
            gstGmmCasGlobalCtrl.ucRauCmpFlg = VOS_TRUE;

            /* G下，需等待RABM的回复 */
            gstGmmCasGlobalCtrl.ucWaitRabmRauRspFlg = VOS_TRUE;
            gstGmmCasGlobalCtrl.ucRabmRauRspRcvdFlg = VOS_FALSE;
        }

        /* 不需要等待WRR或RABM回复，直接发送RAU COMPLETE消息 */
        if (VOS_FALSE == gstGmmCasGlobalCtrl.ucRauCmpFlg)
        {
            g_GmmInterRatInfoCtrl.ucRauCmpWaitInterRatCnfMsg = VOS_FALSE;
            gstGmmCasGlobalCtrl.ucWaitRabmRauRspFlg = VOS_FALSE;

            pSendNasMsg = Gmm_RoutingAreaUpdateCompleteMsgMake();
            Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH, pSendNasMsg);

        }
    }
    else
    {
        gstGmmCasGlobalCtrl.ucRauCmpFlg = GMM_FALSE;
    }
}


VOS_VOID Gmm_RoutingAreaUpdateHandle(
    GMM_MSG_RESOLVE_STRU               *pRauAcceptIe,          /* 指向GMM_MSG_RESOLVE_STRU结构的指针       */
    NAS_MSG_FOR_PCLINT_STRU            *pMsg,
    VOS_BOOL                            bTlliUpdateFlag
)
{
    VOS_UINT8                           ucSndCompleteFlg = GMM_FALSE;                   /* 是否发送RAU COMPLETE消息                 */
    VOS_UINT32                          ulPDPStatus;
    VOS_UINT8                           ucReadychangFLG  = GMM_FALSE;
    VOS_UINT8                           ucOffSet;
    VOS_UINT8                           ucInterRatInfoFlg = GMM_FALSE;

    VOS_UINT8                           ucUeWDrxlength;
    VOS_UINT8                           ucWSysinfoDrxlength;


    GMMSM_DATA_REQ_STRU                 *pstSmDataReqMsg = VOS_NULL_PTR;


    NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_UPDATED);             /* 置更新状态为GU1                          */
    gstGmmCasGlobalCtrl.ucRauCmpFlg = GMM_FALSE;                                /* 默认RAU CMP发送不等待 */

    NAS_GMM_RoutingAreaUpdateHandle_Handling_REQUESTED_MS_INFORMATION_IE(pMsg, pRauAcceptIe, &ucSndCompleteFlg, &ucInterRatInfoFlg);

    Gmm_RoutingAreaUpdateHandle_Handling_RAU_RESULT_IE(pRauAcceptIe, pMsg, &ucSndCompleteFlg, bTlliUpdateFlag);


    Gmm_RoutingAreaUpdateHandle_T3302_Handling(pRauAcceptIe, pMsg);

    Gmm_RoutingAreaUpdateHandle_PTMSI_SIGNATURE_Handling(pRauAcceptIe, pMsg);

    NAS_GMM_HandleRoutingAreaUpdateEPlmn(pRauAcceptIe, pMsg);

    if (GMM_RAU_ACCEPT_IE_NPDU_NUMBER_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_NPDU_NUMBER_FLG))
    {                                                                           /* 空口消息中有N-PDU Numbers                */
        g_GmmRauCtrl.ucNpduCnt = pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                                 [GMM_RAU_ACCEPT_IE_NPDU_NUMBER] + 1];          /* 存储N-PDU Numbers的个数                  */
        Gmm_MemCpy(&g_GmmRauCtrl.aucNpduNum[0],
                   &pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                   [GMM_RAU_ACCEPT_IE_NPDU_NUMBER] + 2],
                   g_GmmRauCtrl.ucNpduCnt);                                     /* 存储N-PDU Numbers                        */

        ucSndCompleteFlg = GMM_TRUE;
        g_GmmRauCtrl.bNpduPresentInRauAccFlg = VOS_TRUE;
    }
    else
    {
        g_GmmRauCtrl.ucNpduCnt = 0;                                             /* 清除保存的 N-PDU NUMBER                  */
        g_GmmRauCtrl.bNpduPresentInRauAccFlg = VOS_FALSE;
    }

    Gmm_RoutingAreaUpdateHandle_READY_TIMER_Handling(pRauAcceptIe, pMsg, &ucReadychangFLG);

    if (GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION_FLG
        == (pRauAcceptIe->ulOptionalIeMask
            & GMM_RAU_ACCEPT_IE_CELL_NOTIFICATION_FLG))
    {
        gstGmmCasGlobalCtrl.ucCellNotifyFlg = GMM_TRUE;                         /* cell notificaion有效标志 */
    }
    else
    {
        gstGmmCasGlobalCtrl.ucCellNotifyFlg = GMM_FALSE;                        /* cell notificaion无效标志 */
    }

    if (GMM_RAU_ACCEPT_IE_PTMSI_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_PTMSI_FLG))
    {                                                                           /* 空口消息中有P-TMSI                       */
        Gmm_MemCpy(NAS_MML_GetUeIdPtmsi(),
                   &pMsg->aucNasMsg[pRauAcceptIe->aucIeOffset
                   [GMM_RAU_ACCEPT_IE_PTMSI] + 3],
                   NAS_MML_MAX_PTMSI_LEN);                                                          /* 存储P-TMSI                               */
        ucSndCompleteFlg = GMM_TRUE;                                            /* 置回COMPLETE消息标志                     */

        g_GmmGlobalCtrl.UeInfo.UeId.ucUeIdMask |= GMM_UEID_P_TMSI;              /* 设置UE ID存在标志                        */
    }

    if (GMM_RAU_ACCEPT_IE_PDP_CONTEXT_STATUS_FLG
        == (pRauAcceptIe->ulOptionalIeMask & GMM_RAU_ACCEPT_IE_PDP_CONTEXT_STATUS_FLG))
    {
        ucOffSet = pRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_PDP_CONTEXT_STATUS];
        ulPDPStatus = (pMsg->aucNasMsg[(ucOffSet+2)]
                    | (pMsg->aucNasMsg[(ucOffSet+3)]<<8))& 0x0000FFFF;

        /*2G下，如果收到了RAU ACCEPT,此时是在Normal Service下，那么实际不做任何操作*/
        if (bTlliUpdateFlag != VOS_FALSE)
        {
            Gmm_SndSmPdpStatusInd(ulPDPStatus, GMM_SM_CAUSE_RAU_ACCEPT);

            if (0x0 == ulPDPStatus)
            {
                if (GMM_MSG_HOLD_FOR_SERVICE
                    == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SERVICE))
                {
                    g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk &= ~GMM_MSG_HOLD_FOR_SERVICE;      /* 清除缓存标志                             */
                    Gmm_MemFree(g_GmmGlobalCtrl.MsgHold.ulMsgAddrForService);
                    Gmm_SndRabmReestablishCnf(GMM_RABM_SERVICEREQ_FAILURE);                 /* 发送GMMRABM_REESTABLISH_CNF(fail)        */
                }
                if ((GMM_TRUE == g_GmmGlobalCtrl.ucFollowOnFlg)
                    && ((GMM_SERVICE_REQUEST_DATA_IDLE == g_GmmGlobalCtrl.ucSpecProcHold)
                    || (GMM_SERVICE_REQUEST_DATA_CONN == g_GmmGlobalCtrl.ucSpecProcHold)))
                {
                    g_GmmGlobalCtrl.ucFollowOnFlg = GMM_FALSE;
                    NAS_MML_SetPsServiceBufferStatusFlg(VOS_FALSE);

                    g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;
                    Gmm_SndRabmReestablishCnf(GMM_RABM_SERVICEREQ_FAILURE);                 /* 发送GMMRABM_REESTABLISH_CNF(fail)        */
                }

                /* 如果之前有PDP激活,此时PDP被去激活了,需要清除缓存的PDP去激活请求,避免继续发去激活请求给SM */
                if (GMM_MSG_HOLD_FOR_SM == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SM))
                {
                    /* 得到缓存的消息 */
                    pstSmDataReqMsg = (GMMSM_DATA_REQ_STRU *)g_GmmGlobalCtrl.MsgHold.ulMsgAddrForSm;

                    if (SM_ACT_PDP_CONTEXT_REQ != NAS_GMM_GetGmmSmDataReqMsgType(&(pstSmDataReqMsg->SmMsg)))
                    {
                        NAS_NORMAL_LOG(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateHandle(): Clear cached SM data Req");

                        Gmm_MemFree(g_GmmGlobalCtrl.MsgHold.ulMsgAddrForSm);
                        g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk &= ~GMM_MSG_HOLD_FOR_SM;
                    }
                }
            }
        }
    }

    Gmm_SndRrmmNasInfoChangeReq(RRC_NAS_MASK_PTMSI_RAI);                        /* 通知RRC NAS层信息变化                    */

    /* 2G模式计算新的local TLLI值 */
    if ((GMM_TRUE==GMM_IsCasGsmMode())
      &&(GMM_UEID_P_TMSI==(g_GmmGlobalCtrl.UeInfo.UeId.ucUeIdMask
         & GMM_UEID_P_TMSI))
       )
    {
        Gmm_RoutingAreaUpdateHandle_Handling_Local_TLLI(bTlliUpdateFlag);
    }

    Gmm_RoutingAreaUpdateHandle_Handling_SndComplete_Flag(ucSndCompleteFlg, ucInterRatInfoFlg);

    if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
    {
        Gmm_SndAgentUsimUpdateFileReq(USIMM_USIM_EFPSLOCI_ID);       /* 更新SIM卡信息                            */
    }
    else
    {
        Gmm_SndAgentUsimUpdateFileReq(USIMM_GSM_EFLOCIGPRS_ID);
    }

    ucUeWDrxlength      = NAS_MML_GetUeUtranPsDrxLen();
    ucWSysinfoDrxlength = NAS_MML_GetWSysInfoDrxLen();
    if (ucWSysinfoDrxlength != ucUeWDrxlength)
    {                                                                           /* DRX参数发生变化                          */
        Gmm_SndRrmmNasInfoChangeReq(RRC_NAS_MASK_DRX);                          /* 通知RRC协商后的PS域DRX参数               */
    }

    if (GMM_TRUE == GMM_IsCasGsmMode())
    {
        Gmm_RoutingAreaUpdateHandle_Under_Gsm(ucInterRatInfoFlg, ucReadychangFLG, bTlliUpdateFlag, ucSndCompleteFlg);
    }
    else
    {
        Gmm_RoutingAreaUpdateHandle_Under_W();
    }


    /* 当ucRauCause为GMM_RAU_FOR_INTERSYSTEM时,通知RABM的RAU类型不同(RAU类型为系统间切换),
       而另外两种原因值发送给RABM的RAU类型相同(正常的RAU),RABM会根据RAU的类型进行状态的迁移,
       所以如果RAU原因为GMM_RAU_FOR_INTERSYSTEM时, 不能清除ucRauCause */
    if (GMM_RAU_FOR_WAITSERVICE == gstGmmSuspendCtrl.ucRauCause)
    {
        /* 此标志记录的是系统间重选或者切换时,RAI相同时,当后续有上行数据时需要发送RAU.
           因为调用此函数时RAU已经完成,所以需要清除此标志 */
        gstGmmSuspendCtrl.ucRauCause = GMM_RAU_FOR_NORMAL;
    }

    if ((VOS_TRUE == NAS_MML_GetCsAttachAllowFlg())
     && (GMM_RAU_NORMAL_CS_TRANS == g_GmmGlobalCtrl.ucSpecProc)
     && (GMM_NET_MODE_I == g_GmmGlobalCtrl.ucNetMod))
    {                                                                           /* CS通信中的NORMAL RAU                     */
        if (GMM_FALSE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
        {                                                                       /* CS不在通信中                             */
            g_GmmGlobalCtrl.ucSpecProc = GMM_NULL_PROCEDURE;                    /* 清除当前进行的specific 流程标志          */
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandle:INFO: specific procedure ended");
            g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;                          /* 清除当前RAU是否是周期RAU标志             */

            g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg = GMM_FALSE;                 /* RAU已经触发，清除缓存的RAU标志*/
            g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg = GMM_FALSE;                /*  GMM进行RAU，清除触发MM的LU过程的标志   */

            if (VOS_TRUE == g_MmSubLyrShare.GmmShare.ucGsAssociationFlg)
            {
                Gmm_RoutingAreaUpdateInitiate(GMM_COMBINED_RALA_UPDATING);          /* 调用RAU流程                              */
            }
            else
            {
                Gmm_RoutingAreaUpdateInitiate(GMM_COMBINED_RALAU_WITH_IMSI_ATTACH);
            }
        }
        else
        {                                                                       /* CS在通信中                               */
            g_GmmGlobalCtrl.ucSpecProc    = GMM_NULL_PROCEDURE;                 /* 清除当前进行的specific 流程标志          */
            PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandle:INFO: specific procedure ended");
            g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_RAU_NORMAL;               /* 记录CS通信中的流程                       */
            g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;                          /* 清除当前RAU是否是周期RAU标志             */
        }
    }
    else
    {                                                                           /* GMM_RAU_NORMAL                           */
        g_GmmGlobalCtrl.ucSpecProc          = GMM_NULL_PROCEDURE;                     /* 清除当前进行的specific 流程标志          */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandle:INFO: specific procedure ended");
        g_GmmRauCtrl.ucPeriodicRauFlg       = GMM_FALSE;                              /* 清除当前RAU是否是周期RAU标志             */
        g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;
    }

    return;
}

VOS_VOID Gmm_RoutingAreaUpdateHandleFollowOn(VOS_VOID)
{
    NAS_MSG_STRU          *pSendNasMsg;                                         /* 用来存储制作的service request消息        */
    GMMSM_DATA_REQ_STRU   *pSmDataReqMsg;

    NAS_NORMAL_LOG2(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateHandleFollowOn: ucSpecProcHol, ulMsgHoldMsk",g_GmmGlobalCtrl.ucSpecProcHold,g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk);

    switch (g_GmmGlobalCtrl.ucSpecProcHold)
    {                                                                           /* 保留的specific流程                       */

    case GMM_SERVICE_REQUEST_DATA_IDLE:
    case GMM_SERVICE_REQUEST_DATA_CONN:
        /* RABM触发的 Service Request 流程被 Rau 流程中断，设置了 Follow On
           标志，如果在 Rau 过程中又收到 RABM 重建消息，那么在 Rau 结束之后
           处理 Follow On 标志的时候，需要清除缓存，否则会造成连续发送两次
           Service Request 请求，即 Service Request 流程结束之后会去处理缓存
           的消息。*/
        if (GMM_MSG_HOLD_FOR_SERVICE ==
            (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SERVICE))
        {
            g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk &= ~GMM_MSG_HOLD_FOR_SERVICE;  /* 清除缓存标志                             */
            Gmm_MemFree(g_GmmGlobalCtrl.MsgHold.ulMsgAddrForService);
            g_GmmGlobalCtrl.MsgHold.ulMsgAddrForService = (VOS_UINT32)VOS_NULL;
        }

        if (GMM_TRUE == GMM_IsCasGsmMode())
        {                                                                       /* 2G下不需要处理 RAB 重建                  */
            g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;                /* 清除保留的specific流程标志               */
            return;
        }

        Gmm_SndRabmReestablishCnf(GMM_RABM_SERVICEREQ_DELAYED);

        break;

    case GMM_SERVICE_REQUEST_SIGNALLING:
        if (GMM_TRUE == g_GmmServiceCtrl.ucSmsFlg)
        {
            if (GMM_TRUE == g_GmmGlobalCtrl.ucSigConFlg)
            {
                Gmm_SndSmsEstCnf();
            }
            else
            {
                g_GmmGlobalCtrl.ucServPreSta = g_GmmGlobalCtrl.ucState;

                GMM_CasFsmStateChangeTo(GMM_SERVICE_REQUEST_INITIATED);

                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_NORMAL, "Gmm_RoutingAreaUpdateHandleFollowOn:NORMAL: STATUS IS GMM_SERVICE_REQUEST_INITIATED");

                g_MmSubLyrShare.GmmShare.ucDeatchEnableFlg = GMM_DETACH_ABLE;

                g_GmmGlobalCtrl.ucSpecProc = GMM_SERVICE_REQUEST_SIGNALLING;    /* 存储当前的流程                           */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandleFollowOn:INFO: SR(signaling) procedure started");

                pSendNasMsg = Gmm_ServiceRequestMsgMake();                      /* 调用service request制作函数              */

                Gmm_SndRrmmEstReq(g_GmmGlobalCtrl.ulEstCause,
                                  GMM_RRC_IDNNS_UNKNOWN,
                                  pSendNasMsg);                                 /* 进行service request(signalling)          */

                Gmm_TimerStart(GMM_TIMER_PROTECT_FOR_SIGNALING);

                return;
            }

        }
        if (GMM_MSG_HOLD_FOR_SM
            ==(g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SM))
        {                                                                       /* 如果有保留的SmDataReq原语                */
            pSmDataReqMsg =
                (GMMSM_DATA_REQ_STRU*)g_GmmGlobalCtrl.MsgHold.ulMsgAddrForSm;   /* 得到保留的消息                           */

            if (GMM_TRUE == g_GmmGlobalCtrl.ucSigConFlg)
            {
                NAS_NORMAL_LOG(WUEPS_PID_GMM, "Gmm_RoutingAreaUpdateHandleFollowOn(): Process cached SM data Req");

                Gmm_SndRrmmDataReqForCm(RRC_NAS_MSG_PRIORTY_HIGH,
                    &pSmDataReqMsg->SmMsg);                                     /* 发送RRMM_DATA_REQ(CM子层的消息首地址)    */
                g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk &= ~GMM_MSG_HOLD_FOR_SM;   /* 清除保留标志                             */
                Gmm_MemFree(pSmDataReqMsg);                                     /* 释放保存的SM消息                         */

                NAS_GMM_SndSmSigConnInd();
            }
            else
            {
                /* 将SM的建链原因值转换为RRC的建链原因值 */
                NAS_GMM_TransferSmEstCause2RrcEstCause(((GMMSM_DATA_REQ_STRU *)pSmDataReqMsg)->ulEstCause,
                                                        &g_GmmGlobalCtrl.ulEstCause);

                g_GmmGlobalCtrl.ucServPreSta = g_GmmGlobalCtrl.ucState;

                GMM_CasFsmStateChangeTo(GMM_SERVICE_REQUEST_INITIATED);

                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_NORMAL, "Gmm_RoutingAreaUpdateHandleFollowOn:NORMAL: STATUS IS GMM_SERVICE_REQUEST_INITIATED");

                g_GmmServiceCtrl.ucSmFlg = GMM_TRUE;
                g_MmSubLyrShare.GmmShare.ucDeatchEnableFlg = GMM_DETACH_ABLE;
                g_GmmGlobalCtrl.ucSpecProc = GMM_SERVICE_REQUEST_SIGNALLING;    /* 存储当前的流程                           */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandleFollowOn:INFO: SR(signaling) procedure started");

                pSendNasMsg = Gmm_ServiceRequestMsgMake();                      /* 调用service request制作函数              */

                Gmm_SndRrmmEstReq(g_GmmGlobalCtrl.ulEstCause, GMM_RRC_IDNNS_UNKNOWN, pSendNasMsg);          /* 进行service request(signalling)          */
                Gmm_TimerStart(GMM_TIMER_PROTECT_FOR_SIGNALING);
            }
        }
        break;

    case GMM_DETACH_COMBINED:
        g_GmmDetachCtrl.ucT3321OutCnt = 0;                                      /* T3321溢出次数清0                         */
        if (GMM_NET_MODE_I == g_GmmGlobalCtrl.ucNetMod)
        {
            Gmm_MsInitNormalDetach(MMC_GMM_PS_CS_DETACH);
        }
        else
        {
            Gmm_MsInitNormalDetach(MMC_GMM_PS_DETACH);
        }
        break;

    case GMM_DETACH_NORMAL:
    case GMM_DETACH_NORMAL_NETMODE_CHANGE:
        g_GmmDetachCtrl.ucT3321OutCnt = 0;                                      /* T3321溢出次数清0                         */
        Gmm_MsInitNormalDetach(MMC_GMM_PS_DETACH);
        g_GmmGlobalCtrl.ucSpecProc = g_GmmGlobalCtrl.ucSpecProcHold;
        break;

    case GMM_DETACH_WITH_IMSI:
        g_GmmDetachCtrl.ucT3321OutCnt = 0;                                      /* T3321溢出次数清0                         */
        Gmm_MsInitNormalDetach(MMC_GMM_CS_DETACH);
        break;

    default:
        break;
    }
    g_GmmGlobalCtrl.ucSpecProcHold = GMM_NULL_PROCEDURE;                        /* 清除保留的specific流程标志               */
    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateHandleFollowOn:INFO: specific procedure ended");
}


VOS_VOID Gmm_RcvRoutingAreaUpdateAcceptMsg(
                                       NAS_MSG_FOR_PCLINT_STRU *pMsg
                                       )
{
    GMM_MSG_RESOLVE_STRU            RauAcceptIe;
    NAS_MSG_STRU                   *pGmmStatus;
    VOS_BOOL                        bTlliUpdateFlag = VOS_TRUE;
    VOS_UINT8                       i,j;
    NAS_MML_IGNORE_AUTH_REJ_INFO_STRU      *pstAuthRejInfo;

    NAS_MML_RAI_STRU                   *pstLastSuccRai = VOS_NULL_PTR;

    VOS_UINT8                           ucUpdateResultValue;

    NAS_MML_EMERGENCY_NUM_LIST_STRU    *pstEmergencyNumList;

    NAS_MML_PLMN_ID_STRU               stMmcPlmnId;

#if   (FEATURE_ON == FEATURE_LTE)
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulT3423StatusChgNeedRegFlg;

    ulT3423StatusChgNeedRegFlg = NAS_GMM_IsT3423StatusChangeNeedRegist(NAS_MML_GetLteCsServiceCfg(),
                                                 NAS_MML_GetTinType(), NAS_MML_GetT3423Status());
#endif

    PS_MEM_SET(&stMmcPlmnId, 0, sizeof(stMmcPlmnId));

    pstEmergencyNumList     = NAS_MML_GetEmergencyNumList();

    g_GmmRauCtrl.ucT3312ExpiredFlg = GMM_FALSE;

    Gmm_MemSet(&RauAcceptIe, 0, sizeof(GMM_MSG_RESOLVE_STRU));

    if ((GMM_TRUE == GMM_IsCasGsmMode())
        && (GMM_REGISTERED_NORMAL_SERVICE == g_GmmGlobalCtrl.ucState))
    {
        bTlliUpdateFlag = VOS_FALSE;
    }
    else if (GMM_ROUTING_AREA_UPDATING_INITIATED != g_GmmGlobalCtrl.ucState)
    {                                                                           /* GMM状态不是GMM_RAU_INITIATED             */

        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateAcceptMsg:WARNING: RAU ACCEPT is unexpected");
        pGmmStatus = Gmm_GmmStatusMsgMake(NAS_MML_REG_FAIL_CAUSE_MSG_TYPE_NOT_COMPATIBLE);

        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);

        return;
    }
    else if (0x20 != (g_GmmGlobalCtrl.ucSpecProc & 0xF0))
    {                                                                           /* 当前的specific过程不是RAU过程            */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateAcceptMsg:WARNING: Current Specific Process is not RAU process");
        return;
    }
    else
    {
    }

    g_GmmGlobalCtrl.ucGprsResumeFlg = GMM_FALSE;

    if (GMM_MSG_LEN_RAU_ACCEPT > pMsg->ulNasMsgSize)
    {                                                                           /* 消息过短                                 */

        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateAcceptMsg:WARNING: RAU ACCEPT is too short");
        pGmmStatus = Gmm_GmmStatusMsgMake(NAS_MML_REG_FAIL_CAUSE_INVALID_MANDATORY_INF);

        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);

        return;
    }

    if (GMM_FAILURE == Gmm_IECheck_RauAccept(pMsg))                             /* 调用函数判断RAU ACCEPT消息的正确性       */
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateAcceptMsg:WARNING: RAU ACCEPT is semantically incorrect");
        pGmmStatus =
            Gmm_GmmStatusMsgMake(NAS_MML_REG_FAIL_CAUSE_SEMANTICALLY_INCORRECT_MSG);

        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);
        return;
    }
    if (GMM_FALSE == Gmm_AttachRauAcceptJudgeRai(pMsg))
    {                                                                           /* RAU ACCEPT中的RAI与系统信息的RAI不一致   */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RcvRoutingAreaUpdateAcceptMsg:WARNING: RAI in RAU ACCEPT is different with current RAI");
        pGmmStatus =
            Gmm_GmmStatusMsgMake(NAS_MML_REG_FAIL_CAUSE_SEMANTICALLY_INCORRECT_MSG);

        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH , pGmmStatus);
        return;
    }

    NAS_GMM_ResetRejCauseChangedCounter();

    /* 先更新mandatory Periodic RA update timer IE t3312的值，
      Gmm_RoutingAreaUpdateResolveIe会根据 T3312 extended value IE再更新t3312的值 */
    Gmm_SaveTimerValue(GMM_TIMER_T3312, pMsg->aucNasMsg[3]);                    /* 存储T3312时长                            */

    Gmm_RoutingAreaUpdateResolveIe(&RauAcceptIe, pMsg);                         /* 调用函数解析RAU ACCEPT消息的IE           */
    NAS_Gmm_SaveRauAcceptGmmCause(pMsg, &RauAcceptIe);
    /*RAU 成功，更新Drx参数携带状态*/
    NAS_MML_SetPsRegContainDrx(g_GmmGlobalCtrl.UeInfo.enLatestAttachOrRauContainDrx);

    /* 可维可测增加消息打印 */
    NAS_GMM_LogPsRegContainDrxInfo(g_GmmGlobalCtrl.UeInfo.enLatestAttachOrRauContainDrx);


    if (GMM_RAU_ACCEPT_IE_EMERGENCY_NUMBER_LIST_FLG
        == (RauAcceptIe.ulOptionalIeMask
                & GMM_RAU_ACCEPT_IE_EMERGENCY_NUMBER_LIST_FLG))
    {                                                                           /* 消息中包含紧急呼列表                     */

        pstEmergencyNumList->ulMcc =
            g_GmmGlobalCtrl.SysInfo.Rai.Lai.PlmnId.aucMccDigit[0]
            | (g_GmmGlobalCtrl.SysInfo.Rai.Lai.PlmnId.aucMccDigit[1] << 8)
            | (g_GmmGlobalCtrl.SysInfo.Rai.Lai.PlmnId.aucMccDigit[2] << 16);

        Gmm_SndCcEmergencyNumberList(pstEmergencyNumList);
    }
    else if (0 != pstEmergencyNumList->ucEmergencyNumber)
    {
        /* 消息中包不含紧急呼列表但之前存储了紧急呼列表,如果不是同一个国家的mcc则，删除之前存储的紧急呼叫列表
        24008_CR2107R1_(Rel-11)_C1-121605 24008 4.4.4.6、4.7.3.1.3、4.7.5.1.3描述如下:
        The emergency number(s) received in the Emergency Number List IE are valid only
        in networks with in.the same MCCcountry as in  the cell on which this IE
        is received.*/
        NAS_GMM_ConvertPlmnIdToMmcFormat(&g_GmmGlobalCtrl.SysInfo.Rai.Lai.PlmnId, &stMmcPlmnId);

        if (VOS_FALSE == NAS_MML_ComparePlmnMcc(pstEmergencyNumList->ulMcc, stMmcPlmnId.ulMcc))
        {
            pstEmergencyNumList->ulMcc = 0xFFFFFFFF;

            for (i = 0; i < pstEmergencyNumList->ucEmergencyNumber; i++)
            {
                pstEmergencyNumList->aucEmergencyList[i].ucCategory = 0xFF;

                for (j=0; j < NAS_MML_EMERGENCY_NUM_MAX_LEN; j++)
                {
                    pstEmergencyNumList->aucEmergencyList[i].aucEmcNum[j] = 0xFF;
                }
            }

            pstEmergencyNumList->ucEmergencyNumber = 0;
        }

        Gmm_SndCcEmergencyNumberList(pstEmergencyNumList);
    }
    else
    {
    }

    /* report network feature information in UMTS. According to VoLTE SRS document,
     * In utran network, the related capability information reported is not supported
     * now.
     */
    NAS_GMM_SndMmcNetworkCapabilityInfoInd(GMM_MMC_NW_EMC_BS_NOT_SUPPORTED,
                                           NAS_MML_NW_IMS_VOICE_NOT_SUPPORTED,
                                           GMM_MMC_LTE_CS_CAPBILITY_NOT_SUPPORTED);


    Gmm_TimerStop(GMM_TIMER_T3330);                                             /* 停止Timer3330                            */
    Gmm_TimerStop(GMM_TIMER_T3311);                                             /* 停止Timer3311                            */
    Gmm_TimerStop(GMM_TIMER_T3318);                                             /* 停止T3318                                */
    Gmm_TimerStop(GMM_TIMER_T3320);                                             /* 停止T3320                                */
    Gmm_TimerStop(GMM_TIMER_T3316);                                             /* 停止T3316                                */

    /* 4.7.2.1.2 Handling of READY timer in the MS in Iu mode and S1 mode
       The READY timer is not applicable for Iu mode and S1 mode.
       Upon completion of a successful GPRS attach or routing area updating
       procedure in Iu mode, the MS may stop the READY timer, if running.
       Upon completion of a successful EPS attach or tracking area updating
       procedure, the MS may stop the READY timer, if running. */
    if (NAS_MML_NET_RAT_TYPE_WCDMA == NAS_MML_GetCurrNetRatType())
    {
        Gmm_TimerStop(GMM_TIMER_T3314);
        gstGmmCasGlobalCtrl.GmmSrvState = GMM_AGB_GPRS_STANDBY;
    }

    /* 清除鉴权相关全局变量 */
    g_GmmAuthenCtrl.ucResStoredFlg  = GMM_FALSE;                                /* 将"RES存在标志 "置为0                    */
    g_GmmAuthenCtrl.ucRandStoredFlg = GMM_FALSE;                                /* 将"RAND存在标志 "置为0                   */

    ucUpdateResultValue = (pMsg->aucNasMsg[2] >> NAS_MML_OCTET_MOVE_FOUR_BITS) & NAS_MML_OCTET_LOW_THREE_BITS;


#if   (FEATURE_ON == FEATURE_LTE)
    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    if ( NAS_MML_TIN_TYPE_PTMSI != pstRplmnCfgInfo->enTinType)
    {
        NAS_GMM_UpdateTinType_RauAccept(pstRplmnCfgInfo->enTinType,
                            pstRplmnCfgInfo->aucLastImsi, ucUpdateResultValue);
    }
#endif

    Gmm_RoutingAreaUpdateHandle(&RauAcceptIe, pMsg, bTlliUpdateFlag);                            /* 解析RAU ACCEPT中的IE                     */


    /* 取得当前注册成功的RAI信息，注意:此信息的获取要放在解析完RAI信息之后 */
    pstLastSuccRai    = NAS_MML_GetPsLastSuccRai();

    /* 若GU模的RPlmn信息发生了改变，则保存RPlmn信息到全局变量中，并更新到NVIM中 */
    if ( VOS_TRUE == NAS_GMM_IsGURplmnChanged( &(pstLastSuccRai->stLai.stPlmnId),
                                               NAS_MML_GetCurrNetRatType()) )
    {
        /* 保存RPlmn信息到全局变量中 */
        NAS_MML_UpdateGURplmn( &(pstLastSuccRai->stLai.stPlmnId),
                               NAS_MML_GetCurrNetRatType() );

        /* 将全局变量中的RPlmn信息更新到NVIM中 */
        NAS_GMM_WriteRplmnWithRatNvim( NAS_MML_GetRplmnCfg() );
    }

    if (GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg)
    {                                                                           /* 等待触发RAU                              */
        if (GMM_TRUE == g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg)
        {                                                                       /* 需要触发MM进行LU                         */
            if ((GMM_NET_MODE_II == g_GmmGlobalCtrl.ucNetMod)
                && (GMM_NET_MODE_I == g_GmmGlobalCtrl.SysInfo.ucNetMod))
            {                                                                   /* 网络模式改变                             */
                NAS_GMM_SndMmLuInitiation(GMMMM_NO_TIMER_EXPIRED);          /* 通知MM进行LU                             */
                g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg = GMM_FALSE;
                g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg = GMM_FALSE;
                return;
            }
        }

        NAS_NORMAL_LOG(WUEPS_PID_GMM, "Gmm_RcvRoutingAreaUpdateAcceptMsg: ucHandleRauFlg true trigger RAU");
        
        Gmm_RoutingAreaUpdateInitiate(GMM_UPDATING_TYPE_INVALID);               /* 触发RAU                                  */
        g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg = GMM_FALSE;
        g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg = GMM_FALSE;
        return;
    }

#if   (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == ulT3423StatusChgNeedRegFlg)
    {
        /* 需要清除T3423定时器状态，否则再次收到系统消息后会反复做联合RAU */
        NAS_MML_SetT3423Status(NAS_MML_TIMER_STOP);
        NAS_MML_SetT3412Status(NAS_MML_TIMER_STOP);
    }

#endif

    /*检查是否需要启动T3340定时器监控RRC链路的释放 */
    NAS_GMM_CheckIfNeedToStartTimerT3340();

    /*处理follow on 或其他缓存的事件 */
    NAS_GMM_HandleDelayedEvent();

    pstAuthRejInfo = NAS_MML_GetAuthRejInfo();
    pstAuthRejInfo->ucHplmnPsAuthRejCounter = 0;
    return;
}

VOS_VOID Gmm_SndRoutingAreaUpdateReq(
                                 VOS_UINT8 ucUpdataType                         /* 发送RAU REQUEST的类型                    */
                                 )
{
    NAS_MSG_STRU                               *pSendNasMsg = VOS_NULL_PTR;                                /* 定义NAS_MSG_STRU类型的指针变量           */
    NAS_MML_RAI_STRU                           *pstLastSuccRai;
    GMM_RAI_STRU                                stGmmRai;
    NAS_MML_ROUTING_UPDATE_STATUS_ENUM_UINT8    enPsUpdateStatus;


#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                                  ulPtmsiMappedFlag;
#endif

    pstLastSuccRai    = NAS_MML_GetPsLastSuccRai();
    NAS_GMM_ConvertPlmnIdToGmmFormat(&(pstLastSuccRai->stLai.stPlmnId), &stGmmRai.Lai.PlmnId);

    stGmmRai.ucRac          = pstLastSuccRai->ucRac;
    stGmmRai.Lai.aucLac[0]  = pstLastSuccRai->stLai.aucLac[0];
    stGmmRai.Lai.aucLac[1]  = pstLastSuccRai->stLai.aucLac[1];

#if (FEATURE_ON == FEATURE_LTE)
    ulPtmsiMappedFlag = NAS_Gmm_IsPtmsiMappedFromGuti();
#endif


#if (FEATURE_ON == FEATURE_LTE)
    if ( VOS_TRUE == NAS_MML_GetCsSigConnStatusFlg() )
    {
        /* CS业务结束后，网络模式I,若PS注册成功，也需要触发COMBINED RAU */
        if ( GMM_NET_MODE_I == g_GmmGlobalCtrl.ucNetMod )
        {
            NAS_GMM_SetSpecProcInCsTrans(GMM_RAU_NORMAL);

            g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;
        }
        else
        {
            g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;
        }


        /* 存在CSFB业务时，若进行联合的RAU,则更改为只进行RAU */
        if ( (GMM_COMBINED_RALA_UPDATING == ucUpdataType)
          || (GMM_COMBINED_RALAU_WITH_IMSI_ATTACH == ucUpdataType) )
        {
            ucUpdataType = GMM_RA_UPDATING;
        }

    }
#endif

    if (VOS_TRUE == NAS_MML_GetCsRestrictRegisterFlg())
    {
        if ((GMM_COMBINED_RALA_UPDATING == ucUpdataType)
            || (GMM_COMBINED_RALAU_WITH_IMSI_ATTACH == ucUpdataType))
        {
            ucUpdataType = GMM_RA_UPDATING;
        }
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;
    }

    if (GMM_PERIODC_UPDATING != ucUpdataType)
    {
        g_GmmRauCtrl.ucPeriodicRauFlg = GMM_FALSE;
    }



    GMM_CasFsmStateChangeTo(GMM_ROUTING_AREA_UPDATING_INITIATED);

    /*24008_CR1906R2_(Rel-11)_C1-113744 Stopping of timer T3311 and T3302 24008
    4.7.5.1.1章节描述如下:
    To initiate the normal routing area updating procedure, the MS sends the message
    ROUTING AREA UPDATE REQUEST to the network, starts timer T3330 and changes to state
    GMM-ROUTING-AREA-UPDATING-INITIATED. If timer T3302 is currently running, the MS
    shall stop timer T3302. If timer T3311 is currently running, the MS shall stop timer T3311.
    4.7.5.2.1章节描述如下:
    If timer T3302 is currently running, the MS shall stop timer T3302. If timer T3311 is
    currently running, the MS shall stop timer T3311.*/
    Gmm_TimerStop(GMM_TIMER_T3311);
    Gmm_TimerStop(GMM_TIMER_T3302);

    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_NORMAL, "Gmm_SndRoutingAreaUpdateReq:NORMAL: STATUS IS GMM_ROUTING_AREA_UPDATING_INITIATED");

    g_MmSubLyrShare.GmmShare.ucDeatchEnableFlg = GMM_DETACH_ABLE;

    /* 发起 Rau 时，判断是否有上层业务，如果有，则设置 Follow-On 标志 */

    if ((GMM_MSG_HOLD_FOR_SERVICE
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SERVICE))
        ||(GMM_MSG_HOLD_FOR_SM
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SM))
        || (GMM_MSG_HOLD_FOR_SMS
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SMS)))
    {
        g_GmmGlobalCtrl.ucFollowOnFlg = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);

    }

    /* 检查是否需要在RAU后发起DETACH流程*/
    if (VOS_TRUE == g_GmmDetachCtrl.ucRetryDetachForRelCtrlFlg)
    {
        g_GmmGlobalCtrl.ucFollowOnFlg  = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);

        g_GmmGlobalCtrl.ucSpecProcHold = g_GmmDetachCtrl.ucDetachProc;
    }

    /* 检查是否需要在RAU后发起Service Request流程*/
    if (VOS_TRUE == g_GmmServiceCtrl.ucRetrySrForRelCtrlFlg)
    {
        g_GmmGlobalCtrl.ucFollowOnFlg  = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);

        g_GmmGlobalCtrl.ucSpecProcHold = g_GmmServiceCtrl.ucServiceRequestProc;
    }

    pSendNasMsg = Gmm_RoutingAreaUpdateRequestMsgMake(ucUpdataType);            /* 制作RAU REQUEST消息                      */

    gstGmmCasGlobalCtrl.ucSysRauFlg = GMM_FALSE;                                /* 系统信息要求的RAU过程标志清除 */

    if (GMM_TRUE == GMM_IsCasGsmMode())
    {
        enPsUpdateStatus = NAS_MML_GetPsUpdateStatus();

        if ( (GMM_FALSE == Gmm_Compare_Rai(&stGmmRai, &g_GmmGlobalCtrl.SysInfo.Rai))
          || (NAS_MML_ROUTING_UPDATE_STATUS_UPDATED != enPsUpdateStatus)
#if (FEATURE_ON == FEATURE_LTE)
          || (VOS_TRUE == ulPtmsiMappedFlag)
#endif
          )
        {
            GMM_UpdateSysTlli(GMM_FOREIGN_TLLI);                                    /* 获得foreign TLLI */

            if (GMM_TRUE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
            {
                if (GMM_FALSE == GMM_AssignModifyTlli(gstGmmCasGlobalCtrl.ulTLLI,
                                                      gstGmmCasGlobalCtrl.ucflgTLLI))
                {
                    GMM_LOG_WARN("Gmm_SndRoutingAreaUpdateReq():Error: new TLLI assigned unsuccessfully!");
                }
            }
            else
            {
                GMM_AssignSysTlli();
            }
        }
        else
        {
            if (GMM_FALSE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
            {
                GMM_UpdateSysTlli(GMM_LOCAL_TLLI);
                GMM_AssignSysTlli();
            }
            else if (gstGmmCasGlobalCtrl.ucLastDataSender !=
                            NAS_MML_GetCurrNetRatType())
            {
                GMM_UpdateSysTlli(GMM_LOCAL_TLLI);
                GMM_FreeOldTlli();
                gstGmmCasGlobalCtrl.ulOldTLLI = 0xffffffff;
            }
            else
            {
            }
        }

        if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
        {
            GMM_SndLlcSuspendReq();                                             /* 停止LLC数据传输 */
        }

        gstGmmCasGlobalCtrl.ucSuspendLlcCause |= GMM_SUSPEND_LLC_FOR_RAU;

        if (VOS_FALSE == g_GmmGlobalCtrl.ucDetachAcceptedFlg )
        {
            /* send LLC Abort Req to delete the saved data */
            Gmm_SndLlcAbortReq(LL_GMM_CLEAR_DATA_TYPE_SIGNALLING);
        }

        /* 在发起RAU时，要求L2可以发送GMM的信令消息，所以需要通知L2恢复信令面 */
        GMM_SndLlcResumeReq(LL_GMM_RESUME_TYPE_SIGNAL);
    }

    /* 向接入层发送的建链原因值按照协议，填写为注册*/
    if (GMM_TRUE == g_GmmGlobalCtrl.ucSigConFlg)
    {                                                                           /* 有PS信令连接                             */
        Gmm_SndRrmmDataReq(RRC_NAS_MSG_PRIORTY_HIGH, pSendNasMsg);              /* 用RRMM_DATA_REQ发送RAU REQUEST消息       */
        Gmm_TimerStart(GMM_TIMER_T3330);                                        /* 启动Timer3330                            */
    }
    else
    {
        Gmm_SndRrmmEstReq(RRC_EST_CAUSE_REGISTRATION,
                          GMM_RRC_IDNNS_UNKNOWN,
                          pSendNasMsg);                                         /* 用RRMM_EST_REQ发送RAU REQUEST消息        */

        Gmm_TimerStart(GMM_TIMER_PROTECT_FOR_SIGNALING);
    }

    if ((GMM_RAU_NORMAL             != g_GmmGlobalCtrl.ucSpecProc)
        && (GMM_RAU_NORMAL_CS_TRANS != g_GmmGlobalCtrl.ucSpecProc))
    {                                                                           /* RAU类型不是normal NORMAL                 */
        NAS_GMM_SndMmCombinedRauInitiation();                                      /* 发送MMCGMM_COMBINED_RAU_INITIATION原 语
                                                                                 * 给MMC                                    */
    }
    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_RAU_REQ,
                    (VOS_VOID *)&ucUpdataType,
                    NAS_OM_EVENT_RAU_REQ_LEN);

    /* 清除释放原因值,如果不清除(释放原因值为#5)，U->L失败回退后，有可能多做RAU*/
    g_GmmGlobalCtrl.ucRelCause = RR_REL_CAUSE_NORMAL_EVENT;

    return;
}



VOS_VOID NAS_GMM_SndRoutingAreaUpdateReq(
    VOS_UINT8                           ucUpdataType
)
{
#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ucIsSupportLteCapaFlg;

    VOS_UINT8                           ucLDisabledRauUseLSecuInfoFlag;

    ucLDisabledRauUseLSecuInfoFlag = NAS_MML_GetLteDisabledRauUseLteInfoFlag();

    ucIsSupportLteCapaFlg  = NAS_MML_IsSupportLteCapability();

    /* 1、L disable状态或syscfg设置设置不支持L，但全局变量记录GAS接入层发送的MS Radio Access capability IE是支持LTE的，
       2、L enalbe状态或syscfg设置支持L,但全局变量记录GAS接入层发送的MS Radio Access capability IE是不支持LTE的，
       需要启动定时器等待GAS接入层上报最新的MS Radio Access capability IE信息*/
    if ( ucIsSupportLteCapaFlg != g_GmmGlobalCtrl.UeInfo.ucMsRadioCapSupportLteFromAs)
    {
        /* 启动定时器 */
        Gmm_TimerStart(GMM_TIMER_WAIT_AS_MS_RADIO_CAPA_INFO);
        NAS_GMM_SetRauUpdateType(ucUpdataType);
        Gmm_ComStaChg(GMM_ROUTING_AREA_UPDATING_INITIATED);

        if (NAS_MML_NET_RAT_TYPE_GSM == NAS_MML_GetCurrNetRatType())
        {
            if (GMM_TRUE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
            {
                if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
                {
                    GMM_SndLlcSuspendReq();                                             /* 停止LLC数据传输 */
                }
                gstGmmCasGlobalCtrl.ucSuspendLlcCause |= GMM_SUSPEND_LLC_FOR_RAU;
            }
        }

        return;
    }

    if ((NAS_MML_TIN_TYPE_GUTI == NAS_MML_GetTinType())
     && (VOS_FALSE == gstGmmSuspendCtrl.ucGetLteSecContext)
     && ((VOS_TRUE == ucIsSupportLteCapaFlg)
      || (VOS_TRUE == ucLDisabledRauUseLSecuInfoFlag)))
    {
        if ( VOS_TRUE == NAS_GMM_GetLmmSecInfoFlg() )
        {
            return;
        }

        NAS_GMM_SetSpecProcNeedSecInfo(NAS_GMM_SPEC_PROC_RAU);
        NAS_GMM_SetRauUpdateType(ucUpdataType);

        NAS_GMM_SetLmmSecInfoFlg(VOS_TRUE);

        Gmm_ComStaChg(GMM_ROUTING_AREA_UPDATING_INITIATED);

        /* 给L模发送GMM_LMM_RESEL_SECU_INFO_REQ */
        NAS_GMM_SndLmmReselSecuInfoReq();

        /* 启动定时器 */
        Gmm_TimerStart(GMM_TIMER_WAIT_GET_RESEL_SECU_INFO_CNF);

        if (NAS_MML_NET_RAT_TYPE_GSM == NAS_MML_GetCurrNetRatType())
        {
            if (GMM_TRUE == gstGmmCasGlobalCtrl.ucTlliAssignFlg)
            {
                if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
                {
                    GMM_SndLlcSuspendReq();                                             /* 停止LLC数据传输 */
                }
                gstGmmCasGlobalCtrl.ucSuspendLlcCause |= GMM_SUSPEND_LLC_FOR_RAU;
            }
        }
    }
    else
#endif
    {
        Gmm_SndRoutingAreaUpdateReq(ucUpdataType);
    }

}


VOS_VOID Gmm_PeriodRoutingAreaUpdateType(VOS_VOID)
{
    if (VOS_TRUE == NAS_MML_GetSimCsRegStatus())
    {                                                                           /* CS域SIM有效                              */

        if (MM_STATUS_ATTACHED == g_MmSubLyrShare.MmShare.ucCsAttachState)

        {                                                                       /* CS域已经注册                             */
            if (GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
            {                                                                   /* CS域在通信中                             */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;           /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_PeriodRoutingAreaUpdateType:INFO: normal RAU procedure during CS transaction started");
                g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;       /* 清除CS通信中的流程                       */
            }
            else
            {
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_COMBINED;                  /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_PeriodRoutingAreaUpdateType:INFO: combined RAU procedure started");
            }
        }
        else
        {                                                                       /* CS域没注册                               */
            if (VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
            {                                                                   /* CS域ATTACH被禁止                         */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                    /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_PeriodRoutingAreaUpdateType:INFO: normal RAU procedure started");
            }
            else
            {
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;          /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_PeriodRoutingAreaUpdateType:INFO: combined RAU with IMSI attach procedure started");
            }
        }
    }
    else
    {                                                                           /* SIM无效                                  */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_PeriodRoutingAreaUpdateType:INFO: normal RAU procedure started");
    }
    return;
}

VOS_VOID Gmm_RoutingAreaUpdateInitiate(
                                   VOS_UINT8 ucUpdataType                       /* RAU的类型                                */
                                   )
{
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurRat;
    VOS_UINT8                           ucCsRestrictRegister;

    ucCsRestrictRegister = NAS_MML_GetCsRestrictRegisterFlg();
    enCurRat = NAS_MML_GetCurrNetRatType();

    if ((GMM_REGISTERED_INITIATED == g_GmmGlobalCtrl.ucState)
        || (0x10 == (g_GmmGlobalCtrl.ucState & 0xF0))
        || (GMM_NULL == g_GmmGlobalCtrl.ucState))
    {                                                                           /* PS域没有注册                             */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateInitiate:WARNING: PS domain hasn't register");
        return;
    }
    /*RAU发起后，清除3312定时器超时标志*/
    gstGmmSuspendCtrl.ucT3312State = GMM_T3312_NOT_TIMEOUT;

    g_GmmRauCtrl.ucT3330outCnt = 0;

    /* RAU发起后，清除3311定时器超时标志 */
    g_GmmRauCtrl.ucT3311ExpiredFlg  = GMM_FALSE;

    NAS_GMM_SetMmT3211ExpiredDelayRauFlag(VOS_FALSE);
    NAS_GMM_InitRaiInfo(NAS_GMM_GetAttemptUpdateRaiInfo());

    if (GMM_UPDATING_TYPE_INVALID != ucUpdataType)
    {                                                                           /* 输入参数有效                             */
        if(GMM_PERIODC_UPDATING == ucUpdataType)
        {                                                                       /* 周期性的RAU                              */
            g_GmmRauCtrl.ucPeriodicRauFlg = GMM_TRUE;                           /* 标记触发了周期RAU                        */

            if ((VOS_TRUE != NAS_MML_GetCsAttachAllowFlg())
             || (GMM_NET_MODE_I != g_GmmGlobalCtrl.ucNetMod))
            {                                                                   /* 不是用户模式A且网络模式I                 */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                    /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure started");
            }
            else
            {
                Gmm_PeriodRoutingAreaUpdateType();                              /* 调用函数填写周期性的RAU的类型            */
            }
        }
        else
        {
            switch (ucUpdataType)
            {                                                                   /* 根据RAU类型存储当前进行的specific流程    */
            case GMM_RA_UPDATING:
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure started");
                break;
            case GMM_COMBINED_RALA_UPDATING:
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_COMBINED;
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: combined RAU procedure started");
                break;
            case GMM_COMBINED_RALAU_WITH_IMSI_ATTACH:
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: combined RAU with IMSI attach procedure started");
                break;
            default:
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateInitiate:WARNING: Update Type is Abnormal");
                break;
            }
        }
        NAS_GMM_SndRoutingAreaUpdateReq(ucUpdataType);                              /* 调用函数发送RAU REQUEST                  */
        return;
    }

    /* NMO1下,不通过手机模式判断是否进行联合注册 */
    if ((VOS_TRUE != NAS_MML_GetCsAttachAllowFlg())
     || (GMM_NET_MODE_I != g_GmmGlobalCtrl.ucNetMod))
    {                                                                           /* 不是用户模式A且网络模式I                 */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure started");
        NAS_GMM_SndRoutingAreaUpdateReq(GMM_RA_UPDATING);                           /* 调用函数发送ATTACH REQUEST               */
        return;
    }

    if (VOS_TRUE == NAS_MML_GetSimCsRegStatus())
    {                                                                           /* CS域SIM有效                              */

        if (MM_STATUS_ATTACHED == g_MmSubLyrShare.MmShare.ucCsAttachState)

        {                                                                       /* CS域已经注册                             */
            if ((VOS_TRUE == ucCsRestrictRegister)
             || ((GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
              && (NAS_MML_NET_RAT_TYPE_WCDMA == enCurRat)))
            {                                                                   /* CS域在通信中                             */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;           /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure during CS transaction started");
                g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;       /* 清除CS通信中的流程                       */
                NAS_GMM_SndRoutingAreaUpdateReq(GMM_RA_UPDATING);                   /* 调用函数发送RAU REQUEST                  */
            }
            else
            {
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_COMBINED;                  /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: combined RAU procedure started");

                if (VOS_TRUE == g_MmSubLyrShare.GmmShare.ucGsAssociationFlg)
                {
                    NAS_GMM_SndRoutingAreaUpdateReq(GMM_COMBINED_RALA_UPDATING);    /* 调用函数发送RAU REQUEST                  */
                }
                else
                {
                    /* 未置g_GmmGlobalCtrl.ucSpecProc为GMM_RAU_WITH_IMSI_ATTACH,
                      T3330超时发起的联合rau类型不为GMM_COMBINED_RALAU_WITH_IMSI_ATTACH*/
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;

                    NAS_GMM_SndRoutingAreaUpdateReq(
                                     GMM_COMBINED_RALAU_WITH_IMSI_ATTACH);
                }
            }
        }
        else
        {                                                                       /* CS域没注册                               */
            if ((VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
             || (VOS_TRUE == ucCsRestrictRegister))
            {                                                                   /* CS域ATTACH被禁止                         */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                    /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure started");
                NAS_GMM_SndRoutingAreaUpdateReq(GMM_RA_UPDATING);                   /* 调用函数发送RAU REQUEST                  */
            }
            else
            {                                                                   /* CS域ATTACH没被禁止                       */
                if (GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
                {                                                               /* CS域在通信中                             */
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;       /* 存储当前进行的specific流程               */
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure during CS transaction started");
                    g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;   /* 清除CS通信中的流程                       */
                    NAS_GMM_SndRoutingAreaUpdateReq(GMM_RA_UPDATING);               /* 调用函数发送RAU REQUEST                  */
                }
                else
                {
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;      /* 存储当前进行的specific流程               */
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: combined RAU with IMSI attach procedure started");
                    NAS_GMM_SndRoutingAreaUpdateReq(
                                     GMM_COMBINED_RALAU_WITH_IMSI_ATTACH);      /* 调用函数发送RAU REQUEST                  */
                }
            }
        }
    }
    else
    {                                                                           /* SIM无效                                  */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiate:INFO: normal RAU procedure started");
        NAS_GMM_SndRoutingAreaUpdateReq(GMM_RA_UPDATING);                           /* 调用函数发送RAU REQUEST                  */
    }
    return;
}


VOS_UINT8 Gmm_RauAccept_Mandatory_Ie_Check(NAS_MSG_FOR_PCLINT_STRU *pMsg,
                                                  VOS_UINT8    *pNeedReturn)
{
    VOS_UINT8                           ucForceToStandby = 0;

    VOS_UINT8                           ucUpdateResultValue;

    if ((VOS_NULL == pMsg) || (VOS_NULL == pNeedReturn))
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RauAccept_Mandatory_Ie_Check:ERROR: Null Pointer");
        return GMM_FAILURE;
    }

    ucForceToStandby = (VOS_UINT8)(pMsg->aucNasMsg[2] & 0x07);

    if (1 == ucForceToStandby)
    {
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_TRUE;
    }

    else if (0 == ucForceToStandby)
    {
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_FALSE;
    }

    else
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RauAccept_Mandatory_Ie_Check:WARNING: IE\"Force to standby\" is invalid in RAU ACCEPT");
        gstGmmCasGlobalCtrl.ucFtsFlg = GMM_FALSE;
    }

    ucUpdateResultValue = (pMsg->aucNasMsg[2] >> NAS_MML_OCTET_MOVE_FOUR_BITS) & NAS_MML_OCTET_LOW_THREE_BITS;

    if ((GMM_RA_UPDATED != ucUpdateResultValue)
     && (GMM_COMBINED_RALA_UPDATED != ucUpdateResultValue)
     && (GMM_RA_UPDATED_ISR_ACTIVE != ucUpdateResultValue)
     && (GMM_COMBINED_RALA_UPDATED_ISR_ACTIVE != ucUpdateResultValue))
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RauAccept_Mandatory_Ie_Check:WARNING: IE\"Update result\" is invalid in RAU ACCEPT");
        return GMM_FAILURE;
    }

    *pNeedReturn = GMM_FALSE;

    return GMM_SUCCESS;
}

VOS_UINT8 Gmm_IeCheck_Npdu_Numbers(NAS_MSG_FOR_PCLINT_STRU *pMsg,
                                                  VOS_UINT8    ucIeOffset,
                                                  VOS_UINT8    *pNeedReturn)
{
    if ((VOS_NULL == pMsg) || (VOS_NULL == pNeedReturn))
    {
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_IeCheck_Npdu_Numbers:ERROR: Null Pointer");
        return GMM_FAILURE;
    }

    if ((VOS_UINT8)(pMsg->ulNasMsgSize) < (ucIeOffset + 2))
    {
        return GMM_SUCCESS;
    }


    *pNeedReturn = GMM_FALSE;

    return GMM_SUCCESS;
}

VOS_UINT8 Gmm_IECheck_RauAccept(NAS_MSG_FOR_PCLINT_STRU *pMsg)
{
    VOS_UINT8   ucIeOffset = GMM_MSG_LEN_RAU_ACCEPT;                            /* 定义临时变量存储存储IE的偏移量           */
    VOS_UINT8   ucEndFlg   = GMM_FALSE;                                         /* 初期化结束标志为FALSE                    */
    VOS_UINT8   ucResult   = GMM_SUCCESS;
    VOS_UINT8   ucNeedReturn = GMM_TRUE;

    ucResult = Gmm_RauAccept_Mandatory_Ie_Check(pMsg, &ucNeedReturn);

    if (GMM_TRUE == ucNeedReturn)
    {
        return ucResult;
    }

    /* 可选IE检查 */
    for (; (ucIeOffset < pMsg->ulNasMsgSize) && (GMM_FALSE == ucEndFlg); )
    {                                                                           /* 存储IE的偏移量小于空口消息的长度         */
        switch (pMsg->aucNasMsg[ucIeOffset])
        {                                                                       /* IEI                                      */
        case GMM_IEI_P_TMSI_SIGNATURE:                                          /* P-TMSI signature，不需要检查             */
            ucIeOffset += 4;
            break;
        case GMM_IEI_ALLOCATED_PTMSI:                                           /* Allocated P-TMSI                         */
            ucNeedReturn = GMM_TRUE;

            ucResult = Gmm_IeCheck_Allocated_Ptmsi(pMsg,ucIeOffset,&ucNeedReturn);

            if (GMM_TRUE == ucNeedReturn)
            {
                return ucResult;
            }
            ucIeOffset += 7;
            break;
        case GMM_IEI_MS_IDENTITY:                                               /* MS identity                              */
            ucNeedReturn = GMM_TRUE;

            ucResult = Gmm_IeCheck_Ms_Identity(pMsg,&ucIeOffset,&ucNeedReturn);

            if (GMM_TRUE == ucNeedReturn)
            {
                return ucResult;
            }
            break;
        case GMM_IEI_LIST_OF_RECEIVE_N_PDU_NUMBERS:                             /* Allocated P-TMSI                         */
            ucNeedReturn = GMM_TRUE;

            ucResult = Gmm_IeCheck_Npdu_Numbers(pMsg,ucIeOffset,&ucNeedReturn);

            if (GMM_TRUE == ucNeedReturn)
            {
                return ucResult;
            }

            ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
            break;
        case GMM_IEI_NEGOTIATED_READY_TIMER_VALUE:                              /* Negotiated READY timervalue，不需要检查  */
            ucIeOffset += 2;
            break;
        case GMM_IEI_GMM_CAUSE:                                                 /* GMM cause，不需要检查                    */
            ucIeOffset += 2;
            break;
        case GMM_IEI_T3302_VALUE:                                               /* T3302 value                              */
            ucNeedReturn = GMM_TRUE;

            ucResult = Gmm_IeCheck_T3302_Value(pMsg, ucIeOffset, &ucNeedReturn);

            if (GMM_TRUE == ucNeedReturn)
            {
                return ucResult;
            }
            ucIeOffset += 3;
            break;
        case GMM_IEI_CELL_NOTIFICATION:                                         /* Cell Notification，不需要检查            */
            ucIeOffset += 1;
            break;
        case GMM_IEI_EQUIVALENT_PLMNS:                                          /* Equivalent PLMNs                         */
            ucNeedReturn = GMM_TRUE;

            ucResult = Gmm_IeCheck_Equivalent_Plmns(pMsg, ucIeOffset, &ucNeedReturn);

            if (GMM_TRUE == ucNeedReturn)
            {
                return ucResult;
            }
            ucIeOffset += 2 + pMsg->aucNasMsg[ucIeOffset + 1];
            break;
        case GMM_IEI_PDP_CONTEXT_STATUS:
            ucIeOffset += 4;
            break;
        case GMM_IEI_EMERGENCY_NUMBER_LIST:                                     /* Emergency Number List                    */
            /*lint -e961*/
            if (ucIeOffset + 2 + pMsg->aucNasMsg[ucIeOffset + 1] <= pMsg->ulNasMsgSize)
            /*lint +e961*/
            {
                ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
            }
            else
            {
                ucEndFlg = GMM_TRUE;
            }
            break;

            case NAS_GMM_IEI_T3323_VALUE:
                ucIeOffset += 3;
                break;

            case NAS_GMM_IEI_T3312_EXTENDED_VALUE:
                ucIeOffset += NAS_GMM_IE_T3312_EXTENDED_VALUE_LEN;
                break;

        default:
            if(GMM_IEI_NETWORK_FEATURE_SUPPORT == (pMsg->aucNasMsg[ucIeOffset] & 0xF0))
            {/* Network feature support                  */
                ucIeOffset += 1;
            }
            /* referenc to 24.008 8.5 */
            else if(GMM_IEI_COMPREHENSION_REQUIRED == (pMsg->aucNasMsg[ucIeOffset] & 0xF0))
            {
                return GMM_FAILURE;
            }
            /* TV类型的IE */
            else if(GMM_IEI_FORMAT_T_TYPE == (pMsg->aucNasMsg[ucIeOffset] & GMM_IEI_FORMAT_TYPE_INFO))
            {
                ucIeOffset += 1;
            }
            /* TLV类型的IE */
            /*lint -e961*/
            else if (ucIeOffset + 2 + pMsg->aucNasMsg[ucIeOffset + 1] < pMsg->ulNasMsgSize)
            /*lint +e961*/
            {
                ucIeOffset += (2 + pMsg->aucNasMsg[ucIeOffset + 1]);
            }
            else
            {
                ucEndFlg = GMM_TRUE;
            }
            break;
        }
    }
    return GMM_SUCCESS;
}



VOS_VOID Gmm_RoutingAreaUpdateInitiateWithReEstRR()
{

    if ((GMM_REGISTERED_INITIATED == g_GmmGlobalCtrl.ucState)
        || (0x10 == (g_GmmGlobalCtrl.ucState & 0xF0))
        || (GMM_NULL == g_GmmGlobalCtrl.ucState))
    {                                                                           /* PS域没有注册                             */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_WARNING, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:WARNING: PS domain hasn't register");
        return;
    }

    g_GmmRauCtrl.ucT3330outCnt = 0;

    g_GmmGlobalCtrl.ucRelCause = RR_REL_CAUSE_NORMAL_EVENT;

#if (FEATURE_ON == FEATURE_LTE)

    /* 存在CS信令连接时，进行RAU */
    if ( VOS_TRUE == NAS_MML_GetCsSigConnStatusFlg() )
    {
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;
        Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);
        return;
    }
#endif

    if ((VOS_TRUE != NAS_MML_GetCsAttachAllowFlg())
     || (GMM_NET_MODE_I != g_GmmGlobalCtrl.ucNetMod))
    {                                                                           /* 不是用户模式A且网络模式I                 */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: normal RAU procedure started");
        Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);                           /* 调用函数发送ATTACH REQUEST               */
        return;
    }

    if (VOS_TRUE == NAS_MML_GetSimCsRegStatus())
    {                                                                           /* CS域SIM有效                              */

        if (MM_STATUS_ATTACHED == g_MmSubLyrShare.MmShare.ucCsAttachState)

        {                                                                       /* CS域已经注册                             */
            if (GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
            {                                                                   /* CS域在通信中                             */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;           /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: normal RAU procedure during CS transaction started");
                g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;       /* 清除CS通信中的流程                       */
                Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);        /* 调用函数发送RAU REQUEST                  */
            }
            else
            {
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: combined RAU procedure started");
                if (VOS_FALSE == g_MmSubLyrShare.GmmShare.ucGsAssociationFlg)
                {
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;      /* 存储当前进行的specific流程               */
                    Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_COMBINED_RALAU_WITH_IMSI_ATTACH);
                }
                else
                {

                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_COMBINED;              /* 存储当前进行的specific流程               */
                    Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_COMBINED_RALA_UPDATING);        /* 调用函数发送RAU REQUEST                  */
                }
            }
        }
        else
        {                                                                       /* CS域没注册                               */
            if (VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
            {                                                                   /* CS域ATTACH被禁止                         */
                g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                    /* 存储当前进行的specific流程               */
                PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: normal RAU procedure started");
                Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);                   /* 调用函数发送RAU REQUEST                  */
            }
            else
            {                                                                   /* CS域ATTACH没被禁止                       */
                if (GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
                {                                                               /* CS域在通信中                             */
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;       /* 存储当前进行的specific流程               */
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: normal RAU procedure during CS transaction started");
                    g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;   /* 清除CS通信中的流程                       */
                    Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);    /* 调用函数发送RAU REQUEST                  */
                }
                else
                {
                    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;      /* 存储当前进行的specific流程               */
                    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: combined RAU with IMSI attach procedure started");
                    Gmm_SndRoutingAreaUpdateReqWithReEstRR(
                                     GMM_COMBINED_RALAU_WITH_IMSI_ATTACH);      /* 调用函数发送RAU REQUEST                  */
                }
            }
        }
    }
    else
    {                                                                           /* SIM无效                                  */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_INFO, "Gmm_RoutingAreaUpdateInitiateWithReEstRR:INFO: normal RAU procedure started");
        Gmm_SndRoutingAreaUpdateReqWithReEstRR(GMM_RA_UPDATING);                /* 调用函数发送RAU REQUEST                  */
    }

    if ((GMM_RAU_NORMAL             != g_GmmGlobalCtrl.ucSpecProc)
        && (GMM_RAU_NORMAL_CS_TRANS != g_GmmGlobalCtrl.ucSpecProc))
    {                                                                           /* RAU类型不是normal NORMAL                 */
        NAS_GMM_SndMmCombinedRauInitiation();
    }

    return;
}


VOS_VOID Gmm_SndRoutingAreaUpdateReqWithReEstRR(
                                 VOS_UINT8 ucUpdataType                         /* 发送RAU REQUEST的类型                    */
                                 )
{
    NAS_MSG_STRU    *pSendNasMsg = VOS_NULL_PTR;                                /* 定义NAS_MSG_STRU类型的指针变量           */

    GMM_CasFsmStateChangeTo(GMM_ROUTING_AREA_UPDATING_INITIATED);

    PS_NAS_LOG(WUEPS_PID_GMM, VOS_NULL, PS_LOG_LEVEL_NORMAL, "Gmm_SndRoutingAreaUpdateReqWithReEstRR:NORMAL: STATUS IS GMM_ROUTING_AREA_UPDATING_INITIATED");

    g_MmSubLyrShare.GmmShare.ucDeatchEnableFlg = GMM_DETACH_ABLE;

    /* 发起 Rau 时，判断是否有上层业务，如果有，则设置 Follow-On 标志 */

    if ((GMM_MSG_HOLD_FOR_SERVICE
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SERVICE))
        ||(GMM_MSG_HOLD_FOR_SM
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SM))
        || (GMM_MSG_HOLD_FOR_SMS
        == (g_GmmGlobalCtrl.MsgHold.ulMsgHoldMsk & GMM_MSG_HOLD_FOR_SMS)))
    {
        g_GmmGlobalCtrl.ucFollowOnFlg = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);
    }


    /* 检查是否需要在RAU后发起DETACH流程 */
    if (VOS_TRUE == g_GmmDetachCtrl.ucRetryDetachForRelCtrlFlg)
    {
        g_GmmGlobalCtrl.ucFollowOnFlg  = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);

        g_GmmGlobalCtrl.ucSpecProcHold = g_GmmDetachCtrl.ucDetachProc;
    }

    /* 检查是否需要在RAU后发起Service Request流程 */
    if (VOS_TRUE == g_GmmServiceCtrl.ucRetrySrForRelCtrlFlg)
    {
        g_GmmGlobalCtrl.ucFollowOnFlg  = GMM_TRUE;
        NAS_MML_SetPsServiceBufferStatusFlg(VOS_TRUE);

        g_GmmGlobalCtrl.ucSpecProcHold = g_GmmServiceCtrl.ucServiceRequestProc;
    }

    pSendNasMsg = Gmm_RoutingAreaUpdateRequestMsgMake(ucUpdataType);            /* 制作RAU REQUEST消息                      */

    gstGmmCasGlobalCtrl.ucSysRauFlg = GMM_FALSE;                                /* 系统信息要求的RAU过程标志清除 */

    if (GMM_TRUE == GMM_IsCasGsmMode())
    {
        GMM_UpdateSysTlli(GMM_FOREIGN_TLLI);                                    /* 获得foreign TLLI */

        if ( GMM_FALSE == GMM_AssignModifyTlli(gstGmmCasGlobalCtrl.ulTLLI,
                gstGmmCasGlobalCtrl.ucflgTLLI) )
        {
            GMM_LOG_WARN("Gmm_SndRoutingAreaUpdateReqWithReEstRR():Error: new TLLI assigned unsuccessfully!");
        }
        if (GMM_NOT_SUSPEND_LLC == gstGmmCasGlobalCtrl.ucSuspendLlcCause)
        {
            GMM_SndLlcSuspendReq();                                             /* 停止LLC数据传输 */
        }

        gstGmmCasGlobalCtrl.ucSuspendLlcCause |= GMM_SUSPEND_LLC_FOR_RAU;

        /* 在发起RAU时，要求L2可以发送GMM的信令消息，所以需要通知L2恢复信令面,
        但是因为L2可能保存有缓冲的数据，所以首先要清除L2的信令  */
        Gmm_SndLlcAbortReq(LL_GMM_CLEAR_DATA_TYPE_SIGNALLING);

        GMM_SndLlcResumeReq(LL_GMM_RESUME_TYPE_SIGNAL);
    }

    Gmm_SndRrmmEstReq(RRC_EST_CAUSE_CALL_RE_ESTABLISH,
                      GMM_RRC_IDNNS_UNKNOWN,
                      pSendNasMsg);                                             /* 用RRMM_EST_REQ发送RAU REQUEST消息        */

    Gmm_TimerStart(GMM_TIMER_PROTECT_FOR_SIGNALING);

    NAS_EventReport(WUEPS_PID_GMM,
                    NAS_OM_EVENT_RAU_REQ,
                    (VOS_VOID *)&ucUpdataType,
                    NAS_OM_EVENT_RAU_REQ_LEN);
    return;
}


VOS_VOID GMM_RauFailureInterSys()
{
    GMM_LOG_INFO("GMM_RauFailureInterSys:Gmm rau failure for intersystem change.");

    if (GMM_RAU_FOR_INTERSYSTEM == gstGmmSuspendCtrl.ucRauCause)
    {
        GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_HANDOVER, GMM_RABM_RAU_FAILURE);
    }

    gstGmmSuspendCtrl.ucRauCause = GMM_RAU_FOR_NORMAL;

    if (GMM_T3312_TIMEOUT == gstGmmSuspendCtrl.ucT3312State)
    {
        gstGmmSuspendCtrl.ucT3312State = GMM_T3312_NOT_TIMEOUT;
    }

    GMM_BufferMsgDump();
    Gmm_HoldBufferFree();
    Gmm_ComCnfHandle();

    return;
}


VOS_VOID GMM_RauSuccessInterSys()
{
    GMM_LOG_INFO("GMM_RauSuccessInterSys:Gmm rau success for intersystem change.");

    gstGmmSuspendCtrl.ucRauCause = GMM_RAU_FOR_NORMAL;

    GMM_SndRabmRauInd(GMM_RABM_RAU_UNDER_HANDOVER, GMM_RABM_RAU_SUCCESS);

    GMM_ResumeSuccess();
}


VOS_VOID NAS_GMM_ProcMmLuResult(
    VOS_VOID                           *pstRcvMsg
)
{
    MMGMM_LU_RESULT_IND_STRU           *pstLuResult;
    GMM_STATE_TYPE_UINT8                ucPreState;

    pstLuResult = (MMGMM_LU_RESULT_IND_STRU *)pstRcvMsg;

    if (MMGMM_LU_FAILURE == pstLuResult->enLuResult)
    {
        switch ( pstLuResult->enCause)
        {
            case NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW :
                /* 此时如果在HPLMN上，则GMM不处理MM的注册结果，仍然进行注册，不进入受限服务 */
                if (VOS_FALSE == NAS_MML_ComparePlmnIdWithHplmn(NAS_MML_GetCurrCampPlmnId()))
                {
                    NAS_GMM_SetAttach2DetachErrCode(GMM_SM_CAUSE_PLMN_NOT_ALLOW);

                    ucPreState = g_GmmGlobalCtrl.ucState;

                    Gmm_TimerStop(GMM_TIMER_T3310);                                             /* 停止Timer3310                            */
                    Gmm_TimerStop(GMM_TIMER_T3318);                                             /* 停止T3318                                */
                    Gmm_TimerStop(GMM_TIMER_T3320);                                             /* 停止T3320                                */

                    if (GMM_TRUE == g_GmmAttachCtrl.ucSmCnfFlg)
                    {                                                                           /* ATTACH过程是由SM触发的                   */

                        Gmm_SndSmEstablishCnf(GMM_SM_EST_FAILURE, GMM_SM_CAUSE_PLMN_NOT_ALLOW); /* 通知SM GMM注册失败                       */
                        g_GmmAttachCtrl.ucSmCnfFlg = GMM_FALSE;                                 /* 清ucSmCnfFlg标志                         */
                    }

                    NAS_MML_SetPsUpdateStatus(NAS_MML_ROUTING_UPDATE_STATUS_ROUTING_AREA_NOT_ALLOWED);      /* 更新状态置为GU3                          */

                    Gmm_DelPsLocInfoUpdateUsim();                                               /* 调用函数删除PS LOCATION INFO             */
                    g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_PLMN;

                    if (GMM_STATUS_ATTACHED == g_MmSubLyrShare.GmmShare.ucAttachSta)
                    {
                        /* 向MMC发送PS注册结果 */
                        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_RAU,
                                                     GMM_MMC_ACTION_RESULT_FAILURE,
                                                     NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW);
                    }
                    else
                    {
                        /* 向MMC发送PS注册结果 */
                        NAS_GMM_SndMmcPsRegResultInd(GMM_MMC_ACTION_ATTACH,
                                                     GMM_MMC_ACTION_RESULT_FAILURE,
                                                     NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW);
                    }

                    Gmm_ComStaChg(GMM_DEREGISTERED_LIMITED_SERVICE);                            /* 调用状态公共处理                         */

                    g_GmmAttachCtrl.ucAttachAttmptCnt = 0;                                      /* attach attempt counter清0                */
                    g_GmmRauCtrl.ucRauAttmptCnt       = 0;                                      /* RAU attempt counter清0                   */

                    /* 判断等待标志是否存在，如果存在则发送MMCGMM_ATTACH CNF */
                    if (GMM_WAIT_PS_ATTACH == (g_GmmGlobalCtrl.stAttachInfo.enAttachType
                                             & GMM_WAIT_PS_ATTACH))
                    {
                        NAS_GMM_SndMmcAttachCnf();
                    }
                    g_GmmGlobalCtrl.stAttachInfo.enAttachType = GMM_WAIT_NULL_ATTACH;

                    NAS_GMM_DeleteEPlmnList();

                    if ((NAS_MML_MS_MODE_CS_ONLY == NAS_MML_GetMsMode())
                     && (VOS_FALSE == g_GmmGlobalCtrl.ucUserPsAttachFlag))
                    {
                        NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
                    }

                    g_GmmGlobalCtrl.MsgHold.ucHandleRauFlg  = GMM_FALSE;                         /* 清除标志                                 */
                    g_GmmGlobalCtrl.MsgHold.ucInitiateLuFlg = GMM_FALSE;                        /* 清除标志                                 */
                    g_GmmGlobalCtrl.ucSpecProc              = GMM_NULL_PROCEDURE;                         /* 清除正在进行的specific过程标志           */
                    g_GmmGlobalCtrl.ucFollowOnFlg           = GMM_FALSE;

                    /* 清除fallowon标志                         */
                    NAS_MML_SetPsServiceBufferStatusFlg(VOS_FALSE);

                    if ((GMM_GPRS_SUSPENSION == ucPreState)
                     || (GMM_SUSPENDED_GPRS_SUSPENSION == ucPreState))
                    {
                        gstGmmSuspendCtrl.ucPreState = GMM_DEREGISTERED_LIMITED_SERVICE;

                        Gmm_ComStaChg(ucPreState);

                        gstGmmSuspendCtrl.ulTimerRunMask &= ~(VOS_UINT32)(0x00000001 << GMM_TIMER_T3310);
                        gstGmmSuspendCtrl.ulTimerRunMask &= ~(VOS_UINT32)(0x00000001 << GMM_TIMER_T3318);
                        gstGmmSuspendCtrl.ulTimerRunMask &= ~(VOS_UINT32)(0x00000001 << GMM_TIMER_T3320);
                    }

                    NAS_GMM_CheckCauseToStartT3340(NAS_MML_REG_FAIL_CAUSE_PLMN_NOT_ALLOW);


                }
                break;
            case NAS_MML_REG_FAIL_CAUSE_LA_NOT_ALLOW :
                g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_LA_FOR_REG_PRVS_SVS;
                break;
            case NAS_MML_REG_FAIL_CAUSE_ROAM_NOT_ALLOW :
            case NAS_MML_REG_FAIL_CAUSE_NO_SUITABL_CELL :
                g_GmmGlobalCtrl.SysInfo.ucForbMask |= GMM_FORBIDDEN_LA_FOR_ROAM;
                break;
            default:
                break;
        }
    }
}


VOS_UINT8 NAS_GMM_GetRoutingAreaUpdateType(VOS_VOID)
{
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurRat;
    VOS_UINT8                           ucCsRestrictRegister;
    VOS_UINT8                           ucUpdateType;

    ucCsRestrictRegister = NAS_MML_GetCsRestrictRegisterFlg();
    enCurRat = NAS_MML_GetCurrNetRatType();


    /* NMO1下,不通过手机模式判断是否进行联合注册 */
    if ((VOS_TRUE != NAS_MML_GetCsAttachAllowFlg())
     || (GMM_NET_MODE_I != g_GmmGlobalCtrl.ucNetMod))
    {
        ucUpdateType               = GMM_RA_UPDATING;
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;
        return ucUpdateType;
    }


    /* cs域卡无效 */
    if (VOS_FALSE == NAS_MML_GetSimCsRegStatus())
    {
        /* SIM无效                                  */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                            /* 存储当前进行的specific流程               */
        ucUpdateType = GMM_RA_UPDATING;
        return ucUpdateType;
    }

    /* CS域已经注册 */
    if (MM_STATUS_ATTACHED == g_MmSubLyrShare.MmShare.ucCsAttachState)
    {
        if ((VOS_TRUE == ucCsRestrictRegister)
         || ((GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
          && (NAS_MML_NET_RAT_TYPE_WCDMA == enCurRat)))
        {
            /* CS域在通信中                             */
            g_GmmGlobalCtrl.ucSpecProc          = GMM_RAU_NORMAL_CS_TRANS;  /* 存储当前进行的specific流程               */
            g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;       /* 清除CS通信中的流程                       */
            ucUpdateType                        = GMM_RA_UPDATING;
            return ucUpdateType;
        }

        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_COMBINED;                  /* 存储当前进行的specific流程               */

        if (VOS_TRUE == g_MmSubLyrShare.GmmShare.ucGsAssociationFlg)
        {
            ucUpdateType = GMM_COMBINED_RALA_UPDATING;
            return ucUpdateType;
        }

        /* 未置g_GmmGlobalCtrl.ucSpecProc为GMM_RAU_WITH_IMSI_ATTACH,
          T3330超时发起的联合rau类型不为GMM_COMBINED_RALAU_WITH_IMSI_ATTACH*/
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;
        ucUpdateType               = GMM_COMBINED_RALAU_WITH_IMSI_ATTACH;
        return ucUpdateType;

    }

    /* CS域没注册                               */
    if ((VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
     || (VOS_TRUE == ucCsRestrictRegister))
    {
        /* CS域ATTACH被禁止                         */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL;                    /* 存储当前进行的specific流程               */
        ucUpdateType               = GMM_RA_UPDATING;
        return ucUpdateType;
    }

    /* CS域ATTACH没被禁止                       */
    if (GMM_TRUE == g_GmmGlobalCtrl.CsInfo.ucCsTransFlg)
    {
        /* CS域在通信中                             */
        g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_NORMAL_CS_TRANS;       /* 存储当前进行的specific流程               */
        g_GmmGlobalCtrl.ucSpecProcInCsTrans = GMM_NULL_PROCEDURE;   /* 清除CS通信中的流程                       */
        ucUpdateType                        = GMM_RA_UPDATING;
        return ucUpdateType;
    }

    g_GmmGlobalCtrl.ucSpecProc = GMM_RAU_WITH_IMSI_ATTACH;      /* 存储当前进行的specific流程               */
    ucUpdateType               = GMM_COMBINED_RALAU_WITH_IMSI_ATTACH;
    return ucUpdateType;

}



VOS_VOID NAS_Gmm_SaveRauAcceptGmmCause(
    NAS_MSG_FOR_PCLINT_STRU            *pMsg,
    GMM_MSG_RESOLVE_STRU               *pstRauAcceptIe
)
{
    VOS_UINT32                          ulOptionalIeMask;
    VOS_UINT8                           ucGmmCause;

    ulOptionalIeMask = pstRauAcceptIe->ulOptionalIeMask;
    if ( (GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG == (ulOptionalIeMask & GMM_RAU_ACCEPT_IE_GMM_CAUSE_FLG))
      && (pstRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_GMM_CAUSE] < 254) )
    {
        ucGmmCause = pMsg->aucNasMsg[pstRauAcceptIe->aucIeOffset[GMM_RAU_ACCEPT_IE_GMM_CAUSE] + 1 ];
    }
    else
    {
        ucGmmCause = NAS_MML_REG_FAIL_CAUSE_NULL;
    }
    NAS_MML_SetOriginalRejectCause(ucGmmCause);
    return;
}

/*lint -restore */


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
