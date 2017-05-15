/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef _STACK_MSGS_H
#define _STACK_MSGS_H

#define MAX_CUSTOM_SAP_ID   256

typedef enum {
    INVALID_SAP = 0,
    WNDRV_WNDRV_SAP,
    FT_WNDRV_SAP,
    WNDRV_SUPC_SAP,
    WNDRV_TCPIP_SAP,
    WNDRV_MMI_SAP,
    WNDRV_BT_SAP,
    STACK_TIMER_SAP,
    SUPC_MMI_SAP,
    SUPC_ABM_SAP,
#if defined(__HOTSPOT_SUPPORT__)
    HOSTAP_WNDRV_SAP,
    HOSTAP_ABM_SAP
#endif /*__HOTSPOT_SUPPORT__*/
} sap_type;



typedef enum {
    MSG_ID_WNDRV_TEST_SET_REQ = 0,
    MSG_ID_WNDRV_TEST_QUERY_REQ,
    MSG_ID_WNDRV_SUPC_DATA_REQ,
    MSG_ID_WNDRV_SUPC_INIT_REQ,
    MSG_ID_WNDRV_SUPC_DEINIT_REQ,
    MSG_ID_WNDRV_SUPC_SITE_SURVEY_REQ,
    MSG_ID_WNDRV_SUPC_JOIN_NETWORK_REQ,
    MSG_ID_WNDRV_SUPC_DISJOIN_NETWORK_REQ,
    MSG_ID_WNDRV_SUPC_COUNTRY_SETTING_REQ,
    MSG_ID_WNDRV_SUPC_REMOVE_KEY_REQ,
    MSG_ID_WNDRV_SUPC_ADD_KEY_REQ,          // 10
    MSG_ID_WNDRV_SUPC_PMKID_UPDATE_REQ,
    MSG_ID_WNDRV_SUPC_AUTH_STATE_UPDATE_REQ,
    MSG_ID_WNDRV_SUPC_BLACKLIST_UPDATE_REQ,
    MSG_ID_WNDRV_SUPC_CHIP_POWERON_REQ,
    MSG_ID_WNDRV_SUPC_CHIP_POWEROFF_REQ,
    MSG_ID_WNDRV_SUPC_QUERY_CAPABILITY_REQ,
    MSG_ID_TCPIP_WNDRV_DATA_REQ,            // 17
    MSG_ID_TCPIP_WNDRV_SETBEARER_REQ,

    MSG_ID_WNDRV_TEST_SET_CNF,
    MSG_ID_WNDRV_TEST_QUERY_CNF,            // 20
    MSG_ID_WNDRV_SUPC_STATUS_IND,
    MSG_ID_WNDRV_SUPC_PMKID_CANDIDATE_IND,
    MSG_ID_WNDRV_SUPC_INIT_CNF,
    MSG_ID_WNDRV_SUPC_DEINIT_CNF,
    MSG_ID_WNDRV_SUPC_NETWORK_CONNECT_IND,
    MSG_ID_WNDRV_SUPC_NETWORK_CONNECT_FAIL_IND,
    MSG_ID_WNDRV_SUPC_NETWORK_DISCONNECT_IND,
    MSG_ID_WNDRV_SUPC_SITE_SURVEY_IND,
    MSG_ID_WNDRV_SUPC_NETWORK_STATISTICS_IND,
    MSG_ID_WNDRV_SUPC_QUERY_CAPABILITY_IND, // 30
    MSG_ID_WNDRV_SUPC_DATA_IND,             // 31
    MSG_ID_TCPIP_WNDRV_DATA_IND,            // 32
    MSG_ID_FLC_WNDRV_UL_SSPDU_RESUME,
    MSG_ID_NVRAM_READ_REQ,

    MSG_ID_WNDRV_INTERRUPT_IND,             // 35
    MSG_ID_TIMER_EXPIRY,                    // 36
    MSG_ID_TST_INJECT_STRING,
    MSG_ID_WNDRV_SCAN_INDICATION,
    MSG_ID_WNDRV_STATISTIC_INDICATION,
    MSG_ID_SOC_WNDRV_DL_SRPDU_RESUME,       // 40
    MSG_ID_WNDRV_MMI_POWEROFF_REQ,
    MSG_ID_NVRAM_READ_CNF,
    MSG_ID_WNDRV_MMI_POWEROFF_RSP,
    MSG_ID_BT_WIFI_SET_CHNL_REQ,

    MSG_ID_WMT_WNDRV_SET_BWCS_REQ,
    MSG_ID_WMT_WNDRV_QUERY_RSSI_REQ,
    MSG_ID_WMT_WNDRV_SET_ANT_REQ,
    MSG_ID_WMT_WNDRV_SET_FLOW_CTRL_REQ,
    MSG_ID_WMT_WNDRV_SET_FIXED_RX_GAIN_REQ,
    MSG_ID_WMT_WNDRV_SET_OMIT_LOW_RATE_REQ, // 50
    MSG_ID_WNDRV_WMT_QUERY_RSSI_IND,
    MSG_ID_WNDRV_WMT_CONN_STATUS_UPDATE_IND,

    MSG_ID_ABM_WNDRV_IPNETWORK_STATUS_IND,
    MSG_ID_WNDRV_ABM_SET_PS_MODE_REQ,
    MSG_ID_WNDRV_ABM_SET_PS_MODE_CNF,
    MSG_ID_WNDRV_ABM_NET_STATUS_IND,
    MSG_ID_WNDRV_ABM_SET_TX_PWR_REQ,
    MSG_ID_WNDRV_ABM_SET_TX_PWR_CNF,
    MSG_ID_WNDRV_ABM_SET_SCAN_DESIRED_SSID_LIST_REQ,
    MSG_ID_WNDRV_ABM_SET_SCAN_DESIRED_SSID_LIST_CNF,    //60
    MSG_ID_WNDRV_ABM_SET_ROAMING_PARA_REQ,
    MSG_ID_WNDRV_ABM_SET_ROAMING_PARA_CNF,
    MSG_ID_WNDRV_ABM_SET_SCAN_DWELL_TIME_REQ,
    MSG_ID_WNDRV_ABM_SET_SCAN_DWELL_TIME_CNF,
    MSG_ID_WNDRV_ABM_SET_SNIFFER_MODE_REQ,
    MSG_ID_WNDRV_ABM_SET_SNIFFER_MODE_CNF,
    MSG_ID_WNDRV_ABM_IOT_LISTENED_AP_INFO_IND,

    /****************************************************************************
     *  Request Messages: HOSTAP -> WNDRV
     ****************************************************************************/
    /* Control message */
    MSG_ID_HOSTAP_WNDRV_INIT_REQ,
    MSG_ID_HOSTAP_WNDRV_DEINIT_REQ,
    MSG_ID_HOSTAP_WNDRV_ADD_STA_REQ,        // 70
    MSG_ID_HOSTAP_WNDRV_REMOVE_STA_REQ,
    MSG_ID_HOSTAP_WNDRV_GET_STA_INFO_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_STA_FLAGS_REQ,
    MSG_ID_HOSTAP_WNDRV_FLUSH_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_GENERIC_ELEM_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_ESSID_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_8021X_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_HOST_ENCRYPT_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_HOST_DECRYPT_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_ENCRYPTION_REQ, // 80
    MSG_ID_HOSTAP_WNDRV_GET_ENCRYPTION_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_PRIVACY_REQ,
    MSG_ID_HOSTAP_WNDRV_SET_CHANNEL_REQ,
    /* Management frame/DATA */
    MSG_ID_HOSTAP_WNDRV_FRAME_REQ,

    /****************************************************************************
     *  Indication Messages: WNDRV -> HOSTAP
     ****************************************************************************/
    /* Control message */
    MSG_ID_HOSTAP_WNDRV_INIT_CNF,
    MSG_ID_HOSTAP_WNDRV_EVENT_CODE_BEGIN = MSG_ID_HOSTAP_WNDRV_INIT_CNF,
    MSG_ID_HOSTAP_WNDRV_DEINIT_CNF,
    MSG_ID_HOSTAP_WNDRV_ADD_STA_CNF,
    MSG_ID_HOSTAP_WNDRV_REMOVE_STA_CNF,
    MSG_ID_HOSTAP_WNDRV_GET_STA_INFO_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_STA_FLAGS_CNF,  //90
    MSG_ID_HOSTAP_WNDRV_FLUSH_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_GENERIC_ELEM_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_ESSID_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_8021X_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_HOST_ENCRYPT_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_HOST_DECRYPT_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_ENCRYPTION_CNF,
    MSG_ID_HOSTAP_WNDRV_GET_ENCRYPTION_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_PRIVACY_CNF,
    MSG_ID_HOSTAP_WNDRV_SET_CHANNEL_CNF,    // 100
    /* Management frame */
    MSG_ID_HOSTAP_WNDRV_FRAME_IND,
    MSG_ID_HOSTAP_WNDRV_EVENT_CODE_END = MSG_ID_HOSTAP_WNDRV_FRAME_IND,

    /****************************************************************************
    *   Indication Messages: HOSTAP -> ABM
    ****************************************************************************/
    MSG_ID_HOSTAP_ABM_INIT_REQ,
    MSG_ID_HOSTAP_ABM_DEINIT_REQ,
    MSG_ID_HOSTAP_ABM_STA_LIST_REQ,
    MSG_ID_HOSTAP_ABM_PRE_INIT_REQ,
    MSG_ID_HOSTAP_ABM_TIMER_CREATE_TEST,
    MSG_ID_HOSTAP_ABM_TIMER_CANCEL_TEST,
    MSG_ID_HOSTAP_ABM_GET_ADDRESS_REQ,
    MSG_ID_HOSTAP_ABM_INIT_CNF,
    MSG_ID_HOSTAP_ABM_DEINIT_CNF,           // 110
    MSG_ID_HOSTAP_ABM_STA_LIST_CNF,
    MSG_ID_HOSTAP_ABM_PREINIT_CNF,
    MSG_ID_HOSTAP_ABM_GETMAC_CNF,
    MSG_ID_HOSTAP_ABM_UPDATE_STA_LIST_IND,

    /****************************************************************************
    *   Indication Messages: SUPC <-> ABM
    ****************************************************************************/
    MSG_ID_SUPC_ABM_WIFI_INIT_REQ,
    MSG_ID_SUPC_ABM_WIFI_DEINIT_REQ,
    MSG_ID_SUPC_ABM_WIFI_CONNECT_REQ,
    MSG_ID_SUPC_ABM_WIFI_DISCONNECT_REQ,
    MSG_ID_SUPC_ABM_WIFI_AP_LIST_REQ,
    MSG_ID_SUPC_ABM_WIFI_DISCONNECT_CNF,    // 120
    MSG_ID_SUPC_ABM_WIFI_DISCONNECT_IND,
    MSG_ID_SUPC_ABM_WIFI_CONNECT_IND,
    MSG_ID_SUPC_ABM_WIFI_CONNECT_CNF,
    MSG_ID_SUPC_ABM_WIFI_AP_LIST_CNF,
    MSG_ID_SUPC_ABM_WIFI_INIT_CNF,
    MSG_ID_SUPC_ABM_WIFI_DEINIT_CNF,
    MSG_ID_SUPC_ABM_WIFI_WPS_INFO_IND,
    MSG_ID_SUPC_ABM_WIFI_GET_CONNECTION_INFO_REQ,
    MSG_ID_SUPC_ABM_WIFI_GET_CONNECTION_INFO_CNF,
    MSG_ID_SUPC_ABM_WIFI_WPS_GET_CREDENTIAL_REQ,
    MSG_ID_SUPC_ABM_WIFI_WPS_GET_CREDENTIAL_CNF


} msg_type;
#define MSG_ID_HOSTAP_WNDRV_PRE_INIT_CNF MSG_ID_HOSTAP_WNDRV_INIT_CNF




typedef enum {
    MOD_WNDRV = 0,
    MOD_SUPC,
    MOD_HOSTAP,
    MOD_ABM,
    MOD_WNDRV_HISR,
    MOD_TCPIP,
    MOD_BWCS,
    MOD_TIMER,
    MOD_NIL,
    MOD_FT,
    MOD_MMI,
    MOD_BT,
    MOD_NVRAM,
    MOD_TIMER_HOSTAP,
    MOD_TOTAL_NUM
} module_type;


#endif /* _STACK_MSGS_H */

