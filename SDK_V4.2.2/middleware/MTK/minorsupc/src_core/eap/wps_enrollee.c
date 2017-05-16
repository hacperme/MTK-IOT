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

#ifndef MTK_WIFI_CHIP_USE_MT5931
#pragma import(__use_no_semihosting_swi)
#endif

//#include "kal_release.h"
#include "kal_general_types.h"
#include "kal_public_defs.h"
#include "kal_public_api.h"
//#include "supc_trc.h"
#include "wndrv_supc_types.h"
#include "supc_common.h"
#include "wpa.h"
#include "defs.h"

#include "wpa_supplicant_i.h"

#include "supc_wpa_common.h"
#include "config_ssid.h"
#include "supc_config.h"
#include "eap.h"
#include "wpa_supplicant_i.h"

#include "sha256.h"
#include "wps_i.h"
#include "wps_dev_attr.h"
#include "wpabuf.h"

static int wps_build_mac_addr(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * MAC Address");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_MAC_ADDR);
    wpabuf_put_be16(msg, ATTR_MAC_ADDR);
    wpabuf_put_be16(msg, ETH_ALEN);
    wpabuf_put_data(msg, wps->mac_addr_e, ETH_ALEN);
    return 0;
}


static int wps_build_wps_state(struct wps_data *wps, struct wpabuf *msg)
{
    u8 state;
    if (wps->wps->ap) {
        state = wps->wps->wps_state;
    } else {
        state = WPS_STATE_NOT_CONFIGURED;
    }
    //wpa_printf(MSG_DEBUG, "WPS:  * Wi-Fi Protected Setup State (%d)",
    // state);
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_WPS_STATE,state);
    wpabuf_put_be16(msg, ATTR_WPS_STATE);
    wpabuf_put_be16(msg, 1);
    wpabuf_put_u8(msg, state);
    return 0;
}


static int wps_build_e_hash(struct wps_data *wps, struct wpabuf *msg)
{
    u8 *hash;
    const u8 *addr[4];
    size_t len[4];

    if (hostapd_get_rand(wps->snonce, 2 * WPS_SECRET_NONCE_LEN) < 0) {
        return -1;
    }
    wpa_hexdump(MSG_DEBUG, "WPS: E-S1", (char *)wps->snonce, WPS_SECRET_NONCE_LEN);
    wpa_hexdump(MSG_DEBUG, "WPS: E-S2",
                (char *)(wps->snonce + WPS_SECRET_NONCE_LEN), WPS_SECRET_NONCE_LEN);

    if (wps->dh_pubkey_e == NULL || wps->dh_pubkey_r == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: DH public keys not available for "
        //"E-Hash derivation");
//      kal_trace(DBG_WPS,INFO_WPS_NO_DH_FOR_E_HASH_DERIVE);
        return -1;
    }

    //wpa_printf(MSG_DEBUG, "WPS:  * E-Hash1");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_E_HASH1);
    wpabuf_put_be16(msg, ATTR_E_HASH1);
    wpabuf_put_be16(msg, SHA256_MAC_LEN);
    hash = wpabuf_put(msg, SHA256_MAC_LEN);
    /* E-Hash1 = HMAC_AuthKey(E-S1 || PSK1 || PK_E || PK_R) */
    addr[0] = wps->snonce;
    len[0] = WPS_SECRET_NONCE_LEN;
    addr[1] = wps->psk1;
    len[1] = WPS_PSK_LEN;
    addr[2] = wpabuf_head(wps->dh_pubkey_e);
    len[2] = wpabuf_len(wps->dh_pubkey_e);
    addr[3] = wpabuf_head(wps->dh_pubkey_r);
    len[3] = wpabuf_len(wps->dh_pubkey_r);
    hmac_sha256_vector(wps->authkey, WPS_AUTHKEY_LEN, 4, addr, len, hash);
    wpa_hexdump(MSG_DEBUG, "WPS: E-Hash1", (char *)hash, SHA256_MAC_LEN);

    //wpa_printf(MSG_DEBUG, "WPS:  * E-Hash2");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_E_HASH2);
    wpabuf_put_be16(msg, ATTR_E_HASH2);
    wpabuf_put_be16(msg, SHA256_MAC_LEN);
    hash = wpabuf_put(msg, SHA256_MAC_LEN);
    /* E-Hash2 = HMAC_AuthKey(E-S2 || PSK2 || PK_E || PK_R) */
    addr[0] = wps->snonce + WPS_SECRET_NONCE_LEN;
    addr[1] = wps->psk2;
    hmac_sha256_vector(wps->authkey, WPS_AUTHKEY_LEN, 4, addr, len, hash);
    wpa_hexdump(MSG_DEBUG, "WPS: E-Hash2", (char *)hash, SHA256_MAC_LEN);

    return 0;
}


static int wps_build_e_snonce1(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * E-SNonce1");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_E_SNONCE1);
    wpabuf_put_be16(msg, ATTR_E_SNONCE1);
    wpabuf_put_be16(msg, WPS_SECRET_NONCE_LEN);
    wpabuf_put_data(msg, wps->snonce, WPS_SECRET_NONCE_LEN);
    return 0;
}


static int wps_build_e_snonce2(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * E-SNonce2");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_E_SNONCE2);
    wpabuf_put_be16(msg, ATTR_E_SNONCE2);
    wpabuf_put_be16(msg, WPS_SECRET_NONCE_LEN);
    wpabuf_put_data(msg, wps->snonce + WPS_SECRET_NONCE_LEN,
                    WPS_SECRET_NONCE_LEN);
    return 0;
}


static struct wpabuf *wps_build_m1(struct wps_data *wps)
{
    struct wpabuf *msg;
    u16 methods;

    if (hostapd_get_rand(wps->nonce_e, WPS_NONCE_LEN) < 0) {
        return NULL;
    }
    wpa_hexdump(MSG_DEBUG, "WPS: Enrollee Nonce",
                (char *)wps->nonce_e, WPS_NONCE_LEN);

    //wpa_printf(MSG_DEBUG, "WPS: Building Message M1");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_MSG_M1);
    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        return NULL;
    }

    methods = (WPS_CONFIG_LABEL |
               WPS_CONFIG_DISPLAY |
               WPS_CONFIG_KEYPAD |
               WPS_CONFIG_PHY_DISP_PIN |
               WPS_CONFIG_PUSHBUTTON |
               WPS_CONFIG_V_PUSHBUTTON);

    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_M1) ||
            wps_build_uuid_e(msg, wps->uuid_e) ||
            wps_build_mac_addr(wps, msg) ||
            wps_build_enrollee_nonce(wps, msg) ||
            wps_build_public_key(wps, msg) ||
            wps_build_auth_type_flags(wps, msg) ||
            wps_build_encr_type_flags(wps, msg) ||
            wps_build_conn_type_flags(wps, msg) ||
            wps_build_config_methods(msg, methods) ||
            wps_build_wps_state(wps, msg) ||
            wps_build_device_attrs(&wps->wps->dev, msg) ||
            wps_build_rf_bands(&wps->wps->dev, msg) ||
            wps_build_assoc_state(wps, msg) ||
            wps_build_dev_password_id(msg, wps->dev_pw_id) ||
            wps_build_config_error(msg, WPS_CFG_NO_ERROR) ||
            wps_build_os_version(&wps->wps->dev, msg) ||
            wps_build_wfa_sub_ver(msg)) {
        wpabuf_free(msg);
        return NULL;
    }

    wps->state = RECV_M2;
    return msg;
}


static struct wpabuf *wps_build_m3(struct wps_data *wps)
{
    struct wpabuf *msg;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message M3");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_MSG_M3);

    if (wps->dev_password == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No Device Password available");
//      kal_trace(DBG_WPS,INFO_WPS_NO_DEV_PW_IN_M3);
        return NULL;
    }
    wps_derive_psk(wps, wps->dev_password, wps->dev_password_len);

    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        return NULL;
    }

    /*add wfa ext, for wps 2.0*/
    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_M3) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_e_hash(wps, msg) ||
            wps_build_wfa_sub_ver(msg) ||
            wps_build_authenticator(wps, msg)) {
        wpabuf_free(msg);
        return NULL;
    }

    wps->state = RECV_M4;
    return msg;
}


static struct wpabuf *wps_build_m5(struct wps_data *wps)
{
    struct wpabuf *msg, *plain;
    //kal_uint32 offset = 0;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message M5");
//    kal_trace(DBG_WPS,INFO_WPS_BUILD_MSG_M5);
    plain = wpabuf_alloc(200);
    if (plain == NULL) {
        return NULL;
    }

    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        wpabuf_free(plain);
        return NULL;
    }
    /*add build wfa ext, for wps 2.0*/
    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_M5) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_e_snonce1(wps, plain) ||
            wps_build_key_wrap_auth(wps, plain) ||
            wps_build_encr_settings(wps, msg, plain) ||
            wps_build_wfa_sub_ver(msg) ||
            wps_build_authenticator(wps, msg)) {
        wpabuf_free(plain);
        wpabuf_free(msg);
        return NULL;
    }
    wpabuf_free(plain);

    wps->state = RECV_M6;
    return msg;
}


static int wps_build_cred_ssid(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * SSID");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_SSID);
    wpabuf_put_be16(msg, ATTR_SSID);
    wpabuf_put_be16(msg, wps->wps->ssid_len);
    wpabuf_put_data(msg, wps->wps->ssid, wps->wps->ssid_len);
    return 0;
}


static int wps_build_cred_auth_type(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * Authentication Type");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_AUTH_TYPE);
    wpabuf_put_be16(msg, ATTR_AUTH_TYPE);
    wpabuf_put_be16(msg, 2);
    wpabuf_put_be16(msg, wps->wps->auth_types);
    return 0;
}


static int wps_build_cred_encr_type(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * Encryption Type");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_ENCR_TYPE);
    wpabuf_put_be16(msg, ATTR_ENCR_TYPE);
    wpabuf_put_be16(msg, 2);
    wpabuf_put_be16(msg, wps->wps->encr_types);
    return 0;
}


static int wps_build_cred_network_key(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * Network Key");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_NETWORK_KEY);
    wpabuf_put_be16(msg, ATTR_NETWORK_KEY);
    wpabuf_put_be16(msg, wps->wps->network_key_len);
    wpabuf_put_data(msg, wps->wps->network_key, wps->wps->network_key_len);
    return 0;
}


static int wps_build_cred_mac_addr(struct wps_data *wps, struct wpabuf *msg)
{
    //wpa_printf(MSG_DEBUG, "WPS:  * MAC Address (AP BSSID)");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_MAC_ADDR);
    wpabuf_put_be16(msg, ATTR_MAC_ADDR);
    wpabuf_put_be16(msg, ETH_ALEN);
    wpabuf_put_data(msg, wps->wps->dev.mac_addr, ETH_ALEN);
    return 0;
}


static int wps_build_ap_settings(struct wps_data *wps, struct wpabuf *plain)
{
    if (wps->wps->ap_settings) {
        //wpa_printf(MSG_DEBUG, "WPS:  * AP Settings (pre-configured)");
//      kal_trace(DBG_WPS,INFO_WPS_BUILD_CRED_AP_SETTING);
        wpabuf_put_data(plain, wps->wps->ap_settings,
                        wps->wps->ap_settings_len);
        return 0;
    }

    return wps_build_cred_ssid(wps, plain) ||
           wps_build_cred_mac_addr(wps, plain) ||
           wps_build_cred_auth_type(wps, plain) ||
           wps_build_cred_encr_type(wps, plain) ||
           wps_build_cred_network_key(wps, plain);
}


static struct wpabuf *wps_build_m7(struct wps_data *wps)
{
    struct wpabuf *msg, *plain;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message M7");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_MSG_M7);

    plain = wpabuf_alloc(500 + wps->wps->ap_settings_len);
    if (plain == NULL) {
        return NULL;
    }

    msg = wpabuf_alloc(1000 + wps->wps->ap_settings_len);
    if (msg == NULL) {
        wpabuf_free(plain);
        return NULL;
    }

    /*add wfa ext, fro wps 2.0*/
    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_M7) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_e_snonce2(wps, plain) ||
            (wps->wps->ap && wps_build_ap_settings(wps, plain)) ||
            wps_build_key_wrap_auth(wps, plain) ||
            wps_build_encr_settings(wps, msg, plain) ||
            wps_build_wfa_sub_ver(msg) ||
            wps_build_authenticator(wps, msg)) {
        wpabuf_free(plain);
        wpabuf_free(msg);
        return NULL;
    }
    wpabuf_free(plain);

    wps->state = RECV_M8;
    return msg;
}


static struct wpabuf *wps_build_wsc_done(struct wps_data *wps)
{
    struct wpabuf *msg;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message WSC_Done");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_WSC_DONE);

    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        return NULL;
    }

    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_WSC_DONE) ||
            wps_build_enrollee_nonce(wps, msg) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_wfa_sub_ver(msg)) {
        wpabuf_free(msg);
        return NULL;
    }

    if (wps->wps->ap) {
        wps->state = RECV_ACK;
    } else {
        wps_success_event(wps->wps);
        wps->state = WPS_FINISHED;
    }
    return msg;
}


static struct wpabuf *wps_build_wsc_ack(struct wps_data *wps)
{
    struct wpabuf *msg;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message WSC_ACK");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_WSC_ACK);

    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        return NULL;
    }

    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_WSC_ACK) ||
            wps_build_enrollee_nonce(wps, msg) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_wfa_sub_ver(msg)) {
        wpabuf_free(msg);
        return NULL;
    }

    return msg;
}


static struct wpabuf *wps_build_wsc_nack(struct wps_data *wps)
{
    struct wpabuf *msg;

    //wpa_printf(MSG_DEBUG, "WPS: Building Message WSC_NACK");
//  kal_trace(DBG_WPS,INFO_WPS_BUILD_MSG_NACK);

    msg = wpabuf_alloc(1000);
    if (msg == NULL) {
        return NULL;
    }

    if (wps_build_version(msg) ||
            wps_build_msg_type(msg, WPS_WSC_NACK) ||
            wps_build_enrollee_nonce(wps, msg) ||
            wps_build_registrar_nonce(wps, msg) ||
            wps_build_config_error(msg, wps->config_error) ||
            wps_build_wfa_sub_ver(msg)) {
        wpabuf_free(msg);
        return NULL;
    }

    return msg;
}


struct wpabuf *wps_enrollee_get_msg(struct wps_data *wps,
                                    enum wsc_op_code *op_code) {
    struct wpabuf *msg;

    switch (wps->state)
    {
        case SEND_M1:
            msg = wps_build_m1(wps);
            *op_code = WSC_MSG;
            break;
        case SEND_M3:
            msg = wps_build_m3(wps);
            *op_code = WSC_MSG;
            break;
        case SEND_M5:
            msg = wps_build_m5(wps);
            *op_code = WSC_MSG;
            break;
        case SEND_M7:
            msg = wps_build_m7(wps);
            *op_code = WSC_MSG;
            break;
        case RECEIVED_M2D:
            if (wps->wps->ap) {
                msg = wps_build_wsc_nack(wps);
                *op_code = WSC_NACK;
                break;
            }
            msg = wps_build_wsc_ack(wps);
            *op_code = WSC_ACK;
            if (msg) {
                /* Another M2/M2D may be received */
                wps->state = RECV_M2;
            }
            break;
        case SEND_WSC_NACK:
            msg = wps_build_wsc_nack(wps);
            *op_code = WSC_NACK;
            break;
        case WPS_MSG_DONE:
            msg = wps_build_wsc_done(wps);
            *op_code = WSC_Done;
            break;
        default:
            //wpa_printf(MSG_DEBUG, "WPS: Unsupported state %d for building "
            //"a message", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_STA_FOR_BUILD_MSG, wps->state);
            msg = NULL;
            break;
    }

    if (*op_code == WSC_MSG && msg)
    {
        /* Save a copy of the last message for Authenticator derivation
         */
        wpabuf_free(wps->last_msg);
        wps->last_msg = wpabuf_dup(msg);
    }

    return msg;
}


static int wps_process_registrar_nonce(struct wps_data *wps, const u8 *r_nonce)
{
    if (r_nonce == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No Registrar Nonce received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_REGISTRAR_NONCE_RECEIVED);
        return -1;
    }

    os_memcpy(wps->nonce_r, r_nonce, WPS_NONCE_LEN);
    wpa_hexdump(MSG_DEBUG, "WPS: Registrar Nonce",
                (char *)wps->nonce_r, WPS_NONCE_LEN);

    return 0;
}


static int wps_process_enrollee_nonce(struct wps_data *wps, const u8 *e_nonce)
{
    if (e_nonce == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No Enrollee Nonce received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_ENROLLEE_NONCE_RECEIVED);
        return -1;
    }

    if (os_memcmp(wps->nonce_e, e_nonce, WPS_NONCE_LEN) != 0) {
        //wpa_printf(MSG_DEBUG, "WPS: Invalid Enrollee Nonce received");
        return -1;
    }

    return 0;
}


static int wps_process_uuid_r(struct wps_data *wps, const u8 *uuid_r)
{
    if (uuid_r == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No UUID-R received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_UUID_R_RECEIVED);
        return -1;
    }

    os_memcpy(wps->uuid_r, uuid_r, WPS_UUID_LEN);
    wpa_hexdump(MSG_DEBUG, "WPS: UUID-R", (char *)wps->uuid_r, WPS_UUID_LEN);

    return 0;
}


static int wps_process_pubkey(struct wps_data *wps, const u8 *pk,
                              size_t pk_len)
{
    if (pk == NULL || pk_len == 0) {
        //wpa_printf(MSG_DEBUG, "WPS: No Public Key received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_PUBKEY_RECEIVED);
        return -1;
    }

    wpabuf_free(wps->dh_pubkey_r);
    wps->dh_pubkey_r = wpabuf_alloc_copy(pk, pk_len);
    if (wps->dh_pubkey_r == NULL) {
        return -1;
    }

    if (wps_derive_keys(wps) < 0) {
        return -1;
    }

    return 0;
}


static int wps_process_r_hash1(struct wps_data *wps, const u8 *r_hash1)
{
    if (r_hash1 == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No R-Hash1 received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_R1_HASH1_RECEIVED);
        return -1;
    }

    os_memcpy(wps->peer_hash1, r_hash1, WPS_HASH_LEN);
    wpa_hexdump(MSG_DEBUG, "WPS: R-Hash1", (char *)wps->peer_hash1, WPS_HASH_LEN);

    return 0;
}


static int wps_process_r_hash2(struct wps_data *wps, const u8 *r_hash2)
{
    if (r_hash2 == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No R-Hash2 received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_R1_HASH2_RECEIVED);
        return -1;
    }

    os_memcpy(wps->peer_hash2, r_hash2, WPS_HASH_LEN);
    wpa_hexdump(MSG_DEBUG, "WPS: R-Hash2", (char *)wps->peer_hash2, WPS_HASH_LEN);

    return 0;
}


static int wps_process_r_snonce1(struct wps_data *wps, const u8 *r_snonce1)
{
    u8 hash[SHA256_MAC_LEN];
    const u8 *addr[4];
    size_t len[4];

    if (r_snonce1 == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No R-SNonce1 received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_R_SNONCE1_RECEIVED);
        return -1;
    }

    wpa_hexdump_key(MSG_DEBUG, "WPS: R-SNonce1", (char *)r_snonce1,
                    WPS_SECRET_NONCE_LEN);

    /* R-Hash1 = HMAC_AuthKey(R-S1 || PSK1 || PK_E || PK_R) */
    addr[0] = r_snonce1;
    len[0] = WPS_SECRET_NONCE_LEN;
    addr[1] = wps->psk1;
    len[1] = WPS_PSK_LEN;
    addr[2] = wpabuf_head(wps->dh_pubkey_e);
    len[2] = wpabuf_len(wps->dh_pubkey_e);
    addr[3] = wpabuf_head(wps->dh_pubkey_r);
    len[3] = wpabuf_len(wps->dh_pubkey_r);
    hmac_sha256_vector(wps->authkey, WPS_AUTHKEY_LEN, 4, addr, len, hash);

    if (os_memcmp(wps->peer_hash1, hash, WPS_HASH_LEN) != 0) {
        //wpa_printf(MSG_DEBUG, "WPS: R-Hash1 derived from R-S1 does "
        //"not match with the pre-committed value");
//      kal_trace(DBG_WPS,INFO_WPS_R_HASH1_NOT_MATCH);
        wps->config_error = WPS_CFG_DEV_PASSWORD_AUTH_FAILURE;
        wps_pwd_auth_fail_event(wps->wps, 1, 1);
        return -1;
    }

    //wpa_printf(MSG_DEBUG, "WPS: Registrar proved knowledge of the first "
    //"half of the device password");
//kal_trace(DBG_WPS,INFO_WPS_PROVED_HALF_DEV_PASSWORD);

    return 0;
}


static int wps_process_r_snonce2(struct wps_data *wps, const u8 *r_snonce2)
{
    u8 hash[SHA256_MAC_LEN];
    const u8 *addr[4];
    size_t len[4];

    if (r_snonce2 == NULL) {
        //wpa_printf(MSG_DEBUG, "WPS: No R-SNonce2 received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_R_SNONCE2_RECEIVED);
        return -1;
    }

    wpa_hexdump_key(MSG_DEBUG, "WPS: R-SNonce2", (char *)r_snonce2,
                    WPS_SECRET_NONCE_LEN);

    /* R-Hash2 = HMAC_AuthKey(R-S2 || PSK2 || PK_E || PK_R) */
    addr[0] = r_snonce2;
    len[0] = WPS_SECRET_NONCE_LEN;
    addr[1] = wps->psk2;
    len[1] = WPS_PSK_LEN;
    addr[2] = wpabuf_head(wps->dh_pubkey_e);
    len[2] = wpabuf_len(wps->dh_pubkey_e);
    addr[3] = wpabuf_head(wps->dh_pubkey_r);
    len[3] = wpabuf_len(wps->dh_pubkey_r);
    hmac_sha256_vector(wps->authkey, WPS_AUTHKEY_LEN, 4, addr, len, hash);

    if (os_memcmp(wps->peer_hash2, hash, WPS_HASH_LEN) != 0) {
        //wpa_printf(MSG_DEBUG, "WPS: R-Hash2 derived from R-S2 does "
        //"not match with the pre-committed value");
//      kal_trace(DBG_WPS,INFO_WPS_R_HASH2_NOT_MATCH);
        wps->config_error = WPS_CFG_DEV_PASSWORD_AUTH_FAILURE;
        wps_pwd_auth_fail_event(wps->wps, 1, 2);
        return -1;
    }

    //wpa_printf(MSG_DEBUG, "WPS: Registrar proved knowledge of the second "
    //"half of the device password");
//  kal_trace(DBG_WPS,INFO_WPS_PROVED_SENCOND_HALF_DEV_PASSWORD);

    return 0;
}


static int wps_process_cred_e(struct wps_data *wps, const u8 *cred,
                              size_t cred_len)
{
    struct wps_parse_attr attr;
    struct wpabuf msg;

    wpa_printf(TRACE_GROUP_1, "WPS: Received Credential");
//  kal_trace(DBG_WPS,INFO_WPS_RECEIVED_CREDENTIAL);
    os_memset(&wps->cred, 0, sizeof(wps->cred));
    wpabuf_set(&msg, cred, cred_len);
    if (wps_parse_msg(&msg, &attr) < 0 ||
            wps_process_cred(&attr, &wps->cred)) {
        return -1;
    }

    if (os_memcmp(wps->cred.mac_addr, wps->wps->dev.mac_addr, ETH_ALEN) !=
            0) {
        wpa_printf(TRACE_GROUP_1, "WPS: MAC Address in the Credential ("
                   MACSTR ") does not match with own address (" MACSTR
                   ")", MAC2STR(wps->cred.mac_addr),
                   MAC2STR(wps->wps->dev.mac_addr));
//      kal_trace(DBG_WPS,INFO_WPS_MAC_NOT_MATCH_IN_CRED, MAC2STR(wps->cred.mac_addr),
//             MAC2STR(wps->wps->dev.mac_addr));
        /*
         * In theory, this could be consider fatal error, but there are
         * number of deployed implementations using other address here
         * due to unclarity in the specification. For interoperability
         * reasons, allow this to be processed since we do not really
         * use the MAC Address information for anything.
         */
    }

    /*in wps 2.0 test plan, enrolling WLAN which use WEP is forbidden*/
    if (wps->cred.encr_type & WPS_ENCR_WEP) {
        return -1;
    }
    if (wps->wps->cred_cb) {
        wps->cred.cred_attr = cred - 4;
        wps->cred.cred_attr_len = cred_len + 4;
        wps->wps->cred_cb(wps->wps->cb_ctx, &wps->cred);
        wps->cred.cred_attr = NULL;
        wps->cred.cred_attr_len = 0;
    }

    return 0;
}


static int wps_process_creds(struct wps_data *wps, const u8 *cred[],
                             size_t cred_len[], size_t num_cred)
{
    size_t i;

    if (wps->wps->ap) {
        return 0;
    }

    if (num_cred == 0) {
        wpa_printf(TRACE_GROUP_1, "WPS: No Credential attributes "
                   "received");
//      kal_trace(DBG_WPS,INFO_WPS_NO_CREDENTIAL_AT_RECEIVED);
        return -1;
    }

    for (i = 0; i < num_cred; i++) {
        if (wps_process_cred_e(wps, cred[i], cred_len[i])) {
            return -1;
        }
    }

    return 0;
}


static int wps_process_ap_settings_e(struct wps_data *wps,
                                     struct wps_parse_attr *attr,
                                     struct wpabuf *attrs)
{
    struct wps_credential cred;

    if (!wps->wps->ap) {
        return 0;
    }

    if (wps_process_ap_settings(attr, &cred) < 0) {
        return -1;
    }

    //wpa_printf(MSG_INFO, "WPS: Received new AP configuration from "
    //"Registrar");
//  kal_trace(DBG_WPS,INFO_WPS_RECEIVED_NEW_AP_CONFIG);

    if (os_memcmp(cred.mac_addr, wps->wps->dev.mac_addr, ETH_ALEN) !=
            0) {
        //wpa_printf(MSG_DEBUG, "WPS: MAC Address in the AP Settings ("
        //MACSTR ") does not match with own address (" MACSTR
        //")", MAC2STR(cred.mac_addr),
        //MAC2STR(wps->wps->dev.mac_addr));
//kal_trace(DBG_WPS,INFO_WPS_MAC_NOT_MATCH_IN_AP_SETTING, MAC2STR(cred.mac_addr),
//             MAC2STR(wps->wps->dev.mac_addr));
        /*
         * In theory, this could be consider fatal error, but there are
         * number of deployed implementations using other address here
         * due to unclarity in the specification. For interoperability
         * reasons, allow this to be processed since we do not really
         * use the MAC Address information for anything.
         */
    }

    if (wps->wps->cred_cb) {
        cred.cred_attr = wpabuf_head(attrs);
        cred.cred_attr_len = wpabuf_len(attrs);
        wps->wps->cred_cb(wps->wps->cb_ctx, &cred);
    }

    return 0;
}


static enum wps_process_res wps_process_m2(struct wps_data *wps,
        const struct wpabuf *msg,
        struct wps_parse_attr *attr) {
    //wpa_printf(MSG_DEBUG, "WPS: Received M2");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M2);

    if (wps->state != RECV_M2)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unexpected state (%d) for "
        //"receiving M2", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNEXPECTED_STA_FOR_RX_M2, wps->state);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    if (wps_process_registrar_nonce(wps, attr->registrar_nonce) ||
    wps_process_enrollee_nonce(wps, attr->enrollee_nonce) ||
    wps_process_uuid_r(wps, attr->uuid_r) ||
    wps_process_pubkey(wps, attr->public_key, attr->public_key_len) ||
    wps_process_authenticator(wps, attr->authenticator, msg))
    {
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    if (wps->wps->ap && wps->wps->ap_setup_locked)
    {
        //wpa_printf(MSG_DEBUG, "WPS: AP Setup is locked - refuse "
        //"registration of a new Registrar");
//      kal_trace(DBG_WPS,INFO_WPS_AP_SETUP_LOCKED);
        wps->config_error = WPS_CFG_SETUP_LOCKED;
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    wps->state = SEND_M3;
    return WPS_CONTINUE;
}


static enum wps_process_res wps_process_m2d(struct wps_data *wps,
        struct wps_parse_attr *attr) {
    //wpa_printf(MSG_DEBUG, "WPS: Received M2D");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M2D);

    if (wps->state != RECV_M2)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unexpected state (%d) for "
        //"receiving M2D", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNEXPECTED_STA_FOR_RX_M2D, wps->state);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    wpa_hexdump_ascii(MSG_DEBUG, "WPS: Manufacturer",
    (char *)attr->manufacturer, attr->manufacturer_len);
    wpa_hexdump_ascii(MSG_DEBUG, "WPS: Model Name",
    (char *)attr->model_name, attr->model_name_len);
    wpa_hexdump_ascii(MSG_DEBUG, "WPS: Model Number",
    (char *)attr->model_number, attr->model_number_len);
    wpa_hexdump_ascii(MSG_DEBUG, "WPS: Serial Number",
    (char *)attr->serial_number, attr->serial_number_len);
    wpa_hexdump_ascii(MSG_DEBUG, "WPS: Device Name",
    (char *)attr->dev_name, attr->dev_name_len);

    if (wps->wps->event_cb)
    {
        union wps_event_data data;
        struct wps_event_m2d *m2d = &data.m2d;
        os_memset(&data, 0, sizeof(data));
        if (attr->config_methods)
            m2d->config_methods =
            WPA_GET_BE16(attr->config_methods);
        m2d->manufacturer = attr->manufacturer;
        m2d->manufacturer_len = attr->manufacturer_len;
        m2d->model_name = attr->model_name;
        m2d->model_name_len = attr->model_name_len;
        m2d->model_number = attr->model_number;
        m2d->model_number_len = attr->model_number_len;
        m2d->serial_number = attr->serial_number;
        m2d->serial_number_len = attr->serial_number_len;
        m2d->dev_name = attr->dev_name;
        m2d->dev_name_len = attr->dev_name_len;
        m2d->primary_dev_type = attr->primary_dev_type;
        if (attr->config_error)
            m2d->config_error =
            WPA_GET_BE16(attr->config_error);
        if (attr->dev_password_id)
            m2d->dev_password_id =
            WPA_GET_BE16(attr->dev_password_id);
        wps->wps->event_cb(wps->wps->cb_ctx, WPS_EV_M2D, &data);
    }

    wps->state = RECEIVED_M2D;
    return WPS_CONTINUE;
}


static enum wps_process_res wps_process_m4(struct wps_data *wps,
        const struct wpabuf *msg,
        struct wps_parse_attr *attr) {
    struct wpabuf *decrypted;
    struct wps_parse_attr eattr;

    //wpa_printf(MSG_DEBUG, "WPS: Received M4");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M4);

    if (wps->state != RECV_M4)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unexpected state (%d) for "
        //"receiving M4", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNEXPECTED_STA_FOR_RX_M4, wps->state);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    if (wps_process_enrollee_nonce(wps, attr->enrollee_nonce) ||
    wps_process_authenticator(wps, attr->authenticator, msg) ||
    wps_process_r_hash1(wps, attr->r_hash1) ||
    wps_process_r_hash2(wps, attr->r_hash2))
    {
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    decrypted = wps_decrypt_encr_settings(wps, attr->encr_settings,
    attr->encr_settings_len);
    if (decrypted == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Failed to decrypted Encrypted "
        //"Settings attribute");
//      kal_trace(DBG_WPS,INFO_WPS_FAIL_DECRYPT_ENCR_SETTING);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    //wpa_printf(MSG_DEBUG, "WPS: Processing decrypted Encrypted Settings "
    //"attribute");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_DECRYPTED_SETTING_AT);
    if (wps_parse_msg(decrypted, &eattr) < 0 ||
    wps_process_key_wrap_auth(wps, decrypted, eattr.key_wrap_auth) ||
    wps_process_r_snonce1(wps, eattr.r_snonce1))
    {
        wpabuf_free(decrypted);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }
    wpabuf_free(decrypted);
    //wpa_printf(MAP_MOD_SUPC, "WPS: process M4 over");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M4_OVER);
    wps->state = SEND_M5;
    return WPS_CONTINUE;
}


static enum wps_process_res wps_process_m6(struct wps_data *wps,
        const struct wpabuf *msg,
        struct wps_parse_attr *attr) {
    struct wpabuf *decrypted;
    struct wps_parse_attr eattr;

    //wpa_printf(MSG_DEBUG, "WPS: Received M6");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M6);

    if (wps->state != RECV_M6)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unexpected state (%d) for "
        //"receiving M6", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNEXPECTED_STA_FOR_RX_M6, wps->state);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    if (wps_process_enrollee_nonce(wps, attr->enrollee_nonce) ||
    wps_process_authenticator(wps, attr->authenticator, msg))
    {
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    decrypted = wps_decrypt_encr_settings(wps, attr->encr_settings,
    attr->encr_settings_len);
    if (decrypted == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Failed to decrypted Encrypted "
        //"Settings attribute");
//      kal_trace(DBG_WPS,INFO_WPS_FAIL_DECRYPT_ENCR_SETTING);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    //wpa_printf(MSG_DEBUG, "WPS: Processing decrypted Encrypted Settings "
    //"attribute");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_DECRYPTED_SETTING_AT);
    if (wps_parse_msg(decrypted, &eattr) < 0 ||
    wps_process_key_wrap_auth(wps, decrypted, eattr.key_wrap_auth) ||
    wps_process_r_snonce2(wps, eattr.r_snonce2))
    {
        wpabuf_free(decrypted);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }
    wpabuf_free(decrypted);

    wps->state = SEND_M7;
    return WPS_CONTINUE;
}

static enum wps_process_res wps_process_m8(struct wps_data *wps,
        const struct wpabuf *msg,
        struct wps_parse_attr *attr) {
    struct wpabuf *decrypted;
    struct wps_parse_attr eattr;

    wpa_printf(TRACE_GROUP_1, "WPS: Received M8");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_M8);

    if (wps->state != RECV_M8)
    {
        wpa_printf(TRACE_GROUP_1, "WPS: Unexpected state (%d) for "
        "receiving M8", wps->state);
//      kal_trace(DBG_WPS,INFO_WPS_UNEXPECTED_STA_FOR_RX_M8, wps->state);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    if (wps_process_enrollee_nonce(wps, attr->enrollee_nonce) ||
    wps_process_authenticator(wps, attr->authenticator, msg))
    {
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    decrypted = wps_decrypt_encr_settings(wps, attr->encr_settings,
    attr->encr_settings_len);
    if (decrypted == NULL)
    {
        wpa_printf(TRACE_GROUP_1, "WPS: Failed to decrypted Encrypted "
        "Settings attribute");
//      kal_trace(DBG_WPS,INFO_WPS_FAIL_DECRYPT_ENCR_SETTING);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }

    wpa_printf(TRACE_GROUP_1, "WPS: Processing decrypted Encrypted Settings "
    "attribute");
//  kal_trace(DBG_WPS,INFO_WPS_PROCESS_DECRYPTED_SETTING_AT);
    if (wps_parse_msg(decrypted, &eattr) < 0 ||
    wps_process_key_wrap_auth(wps, decrypted, eattr.key_wrap_auth) ||
    wps_process_creds(wps, eattr.cred, eattr.cred_len,
    eattr.num_cred) ||
    wps_process_ap_settings_e(wps, &eattr, decrypted))
    {
        wpabuf_free(decrypted);
        wps->state = SEND_WSC_NACK;
        return WPS_CONTINUE;
    }
    wpabuf_free(decrypted);

    wps->state = WPS_MSG_DONE;



#if 0
    // following actions are removed to wpas_wps_eapol_cb
    // it cause the wpa_eapol_send drop the WSC_Done
    /*update wps_s's attr*/
    if (wpa_s->current_ssid)
    {
        wpa_printf(MSG_DEBUG, "after M8, update wpa_s");
        wpa_s->key_mgmt = wpa_s->current_ssid->key_mgmt;
        wpa_s->proto = wpa_s->current_ssid->proto;
        wpa_s->pairwise_cipher =  wpa_s->current_ssid->pairwise_cipher;
        wpa_s->group_cipher =  wpa_s->current_ssid->group_cipher;
        wpa_printf(MSG_DEBUG, "lastest wpa_s key_mgmt=%d,"
        "proto=%d, pair_cipher=%d, gp_cipher=%d",
        wpa_s->key_mgmt, wpa_s->proto,
        wpa_s->pairwise_cipher, wpa_s->group_cipher);
    }
    supc_stop_wps_walk_timer();
#endif

    return WPS_CONTINUE;
}


static enum wps_process_res wps_process_wsc_msg(struct wps_data *wps,
        const struct wpabuf *msg) {
    struct wps_parse_attr attr;
    enum wps_process_res ret = WPS_CONTINUE;

    //wpa_printf(MSG_DEBUG, "WPS: Received WSC_MSG");
//  kal_trace(DBG_WPS,INFO_WPS_RECEIVED_WSC_MSG);

    if (wps_parse_msg(msg, &attr) < 0)
    {
        return WPS_FAILURE;
    }

    if (!wps_version_supported(attr.version))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unsupported message version 0x%x",
        //attr.version ? *attr.version : 0);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_MSG_VERSION,attr.version ? *attr.version : 0);
        return WPS_FAILURE;
    }

    if (attr.enrollee_nonce == NULL ||
    os_memcmp(wps->nonce_e, attr.enrollee_nonce, WPS_NONCE_LEN != 0))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Mismatch in enrollee nonce");
//      kal_trace(DBG_WPS,INFO_WPS_MISMATCH_IN_ENROLLEE_NONCE);
        return WPS_FAILURE;
    }

    if (attr.msg_type == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: No Message Type attribute");
//      kal_trace(DBG_WPS,INFO_WPS_NO_MSG_TYPE_AT);
        return WPS_FAILURE;
    }

    switch (*attr.msg_type)
    {
        case WPS_M2:
            ret = wps_process_m2(wps, msg, &attr);
            break;
        case WPS_M2D:
            ret = wps_process_m2d(wps, &attr);
            break;
        case WPS_M4:
            ret = wps_process_m4(wps, msg, &attr);
            if (ret == WPS_FAILURE || wps->state == SEND_WSC_NACK) {
                wps_fail_event(wps->wps, WPS_M4);
            }
            break;
        case WPS_M6:
            ret = wps_process_m6(wps, msg, &attr);
            if (ret == WPS_FAILURE || wps->state == SEND_WSC_NACK) {
                wps_fail_event(wps->wps, WPS_M6);
            }
            break;
        case WPS_M8:
            ret = wps_process_m8(wps, msg, &attr);
            if (ret == WPS_FAILURE || wps->state == SEND_WSC_NACK) {
                wps_fail_event(wps->wps, WPS_M8);
            }
            break;
        default:
            //wpa_printf(MSG_DEBUG, "WPS: Unsupported Message Type %d",
            //*attr.msg_type);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_MSG_TYPE,
//             *attr.msg_type);
            return WPS_FAILURE;
    }

    /*
     * Save a copy of the last message for Authenticator derivation if we
     * are continuing. However, skip M2D since it is not authenticated and
     * neither is the ACK/NACK response frame. This allows the possibly
     * following M2 to be processed correctly by using the previously sent
     * M1 in Authenticator derivation.
     */
    if (ret == WPS_CONTINUE && *attr.msg_type != WPS_M2D)
    {
        /* Save a copy of the last message for Authenticator derivation
         */
        wpabuf_free(wps->last_msg);
        wps->last_msg = wpabuf_dup(msg);
    }

    return ret;
}


static enum wps_process_res wps_process_wsc_ack(struct wps_data *wps,
        const struct wpabuf *msg) {
    struct wps_parse_attr attr;

    //wpa_printf(MSG_DEBUG, "WPS: Received WSC_ACK");
//  kal_trace(DBG_WPS,INFO_WPS_RECEIVED_WSC_ACK);

    if (wps_parse_msg(msg, &attr) < 0)
    {
        return WPS_FAILURE;
    }

    if (!wps_version_supported(attr.version))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unsupported message version 0x%x",
        // attr.version ? *attr.version : 0);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_MSG_VERSION,attr.version ? *attr.version : 0);
        return WPS_FAILURE;
    }

    if (attr.msg_type == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: No Message Type attribute");
//      kal_trace(DBG_WPS,INFO_WPS_NO_MSG_TYPE_AT);
        return WPS_FAILURE;
    }

    if (*attr.msg_type != WPS_WSC_ACK)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Invalid Message Type %d",
        //*attr.msg_type);
//      kal_trace(DBG_WPS,INFO_WPS_INVALID_MSG_TYPE,*attr.msg_type);
        return WPS_FAILURE;
    }

    if (attr.registrar_nonce == NULL ||
    os_memcmp(wps->nonce_r, attr.registrar_nonce, WPS_NONCE_LEN != 0))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Mismatch in registrar nonce");
//      kal_trace(DBG_WPS,INFO_WPS_MISMATCH_IN_REGISTRAR_NONCE);
        return WPS_FAILURE;
    }

    if (attr.enrollee_nonce == NULL ||
    os_memcmp(wps->nonce_e, attr.enrollee_nonce, WPS_NONCE_LEN != 0))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Mismatch in enrollee nonce");
//      kal_trace(DBG_WPS,INFO_WPS_MISMATCH_IN_ENROLLEE_NONCE);
        return WPS_FAILURE;
    }

    if (wps->state == RECV_ACK && wps->wps->ap)
    {
        //wpa_printf(MSG_DEBUG, "WPS: External Registrar registration "
        //"completed successfully");
//      kal_trace(DBG_WPS,INFO_WPS_EXTERNAL_REGISTRATION_DONE);
        wps_success_event(wps->wps);
        wps->state = WPS_FINISHED;
        return WPS_DONE;
    }

    return WPS_FAILURE;
}


static enum wps_process_res wps_process_wsc_nack(struct wps_data *wps,
        const struct wpabuf *msg) {
    struct wps_parse_attr attr;

    //wpa_printf(MSG_DEBUG, "WPS: Received WSC_NACK");
//  kal_trace(DBG_WPS,INFO_WPS_RECEIVED_WSC_NACK);

    if (wps_parse_msg(msg, &attr) < 0)
    {
        return WPS_FAILURE;
    }

    if (!wps_version_supported(attr.version))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Unsupported message version 0x%x",
        //attr.version ? *attr.version : 0);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_MSG_VERSION,attr.version ? *attr.version : 0);
        return WPS_FAILURE;
    }

    if (attr.msg_type == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: No Message Type attribute");
//      kal_trace(DBG_WPS,INFO_WPS_NO_MSG_TYPE_AT);
        return WPS_FAILURE;
    }

    if (*attr.msg_type != WPS_WSC_NACK)
    {
        //wpa_printf(MSG_DEBUG, "WPS: Invalid Message Type %d",
        //*attr.msg_type);
//      kal_trace(DBG_WPS,INFO_WPS_INVALID_MSG_TYPE,*attr.msg_type);
        return WPS_FAILURE;
    }

    if (attr.registrar_nonce == NULL ||
    os_memcmp(wps->nonce_r, attr.registrar_nonce, WPS_NONCE_LEN != 0))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Mismatch in registrar nonce");
//      kal_trace(DBG_WPS,INFO_WPS_MISMATCH_IN_REGISTRAR_NONCE);
        wpa_hexdump(MSG_DEBUG, "WPS: Received Registrar Nonce",
        (char *)attr.registrar_nonce, WPS_NONCE_LEN);
        wpa_hexdump(MSG_DEBUG, "WPS: Expected Registrar Nonce",
        (char *)wps->nonce_r, WPS_NONCE_LEN);
        return WPS_FAILURE;
    }

    if (attr.enrollee_nonce == NULL ||
    os_memcmp(wps->nonce_e, attr.enrollee_nonce, WPS_NONCE_LEN != 0))
    {
        //wpa_printf(MSG_DEBUG, "WPS: Mismatch in enrollee nonce");
//      kal_trace(DBG_WPS,INFO_WPS_MISMATCH_IN_ENROLLEE_NONCE);
        wpa_hexdump(MSG_DEBUG, "WPS: Received Enrollee Nonce",
        (char *)attr.enrollee_nonce, WPS_NONCE_LEN);
        wpa_hexdump(MSG_DEBUG, "WPS: Expected Enrollee Nonce",
        (char *)wps->nonce_e, WPS_NONCE_LEN);
        return WPS_FAILURE;
    }

    if (attr.config_error == NULL)
    {
        //wpa_printf(MSG_DEBUG, "WPS: No Configuration Error attribute "
        //"in WSC_NACK");
//      kal_trace(DBG_WPS,INFO_WPS_NO_CONFIG_ERROR_AT_IN_WSC_NACK);
        return WPS_FAILURE;
    }

    //wpa_printf(MSG_DEBUG, "WPS: Registrar terminated negotiation with "
    //"Configuration Error %d", WPA_GET_BE16(attr.config_error));
//  kal_trace(DBG_WPS,INFO_WPS_REGISTRAR_TERMINATE_REGISTRATION, WPA_GET_BE16(attr.config_error));

    switch (wps->state)
    {
        case RECV_M4:
            wps_fail_event(wps->wps, WPS_M3);
            break;
        case RECV_M6:
            wps_fail_event(wps->wps, WPS_M5);
            break;
        case RECV_M8:
            wps_fail_event(wps->wps, WPS_M7);
            break;
        default:
            break;
    }

    /* Followed by NACK if Enrollee is Supplicant or EAP-Failure if
     * Enrollee is Authenticator */
    wps->state = SEND_WSC_NACK;

    return WPS_FAILURE;
}


enum wps_process_res wps_enrollee_process_msg(struct wps_data *wps,
        enum wsc_op_code op_code,
        const struct wpabuf *msg) {

    //wpa_printf(MSG_DEBUG, "WPS: Processing received message (len=%lu "
    //"op_code=%d)",
    //(unsigned long) wpabuf_len(msg), op_code);
//kal_trace(DBG_WPS,INFO_WPS_PROCESS_RECEIVED_MSG,
//         (unsigned long) wpabuf_len(msg), op_code);

    if (op_code == WSC_UPnP)
    {
        /* Determine the OpCode based on message type attribute */
        struct wps_parse_attr attr;
        if (wps_parse_msg(msg, &attr) == 0 && attr.msg_type) {
            if (*attr.msg_type == WPS_WSC_ACK) {
                op_code = WSC_ACK;
            } else if (*attr.msg_type == WPS_WSC_NACK) {
                op_code = WSC_NACK;
            }
        }
    }

    switch (op_code)
    {
        case WSC_MSG:
        case WSC_UPnP:
            return wps_process_wsc_msg(wps, msg);
        case WSC_ACK:
            return wps_process_wsc_ack(wps, msg);
        case WSC_NACK:
            return wps_process_wsc_nack(wps, msg);
        default:
            //wpa_printf(MSG_DEBUG, "WPS: Unsupported op_code %d", op_code);
//      kal_trace(DBG_WPS,INFO_WPS_UNSUPPORTED_OP_CODE, op_code);
            return WPS_FAILURE;
    }
}