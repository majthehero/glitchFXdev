/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*
	ChannelShift.h
*/

#pragma once

#ifndef CHANNELSHIFT_H
#define CHANNELSHIFT_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "ChannelShift_Strings.h"

/* Versioning information */
#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1
	
/* Parameter defaults */
#define CHANNELSHIFT_X_MIN -2000
#define CHANNELSHIFT_Y_MIN -4000					 

#define CHANNELSHIFT_X_LOW -500
#define CHANNELSHIFT_Y_LOW -500					 

#define CHANNELSHIFT_X_DEF 0
#define CHANNELSHIFT_Y_DEF 0

#define CHANNELSHIFT_X_HIGH 500
#define CHANNELSHIFT_Y_HIGH 500					 

#define CHANNELSHIFT_X_MAX 2000
#define CHANNELSHIFT_Y_MAX 4000					 
// mode
enum {
	CHANNELSHIFT_MODE_RGB = 0,
	CHANNELSHIFT_MODE_HSL,
	CHANNELSHIFT_MODE_YUV,
	CHANNELSHIFT_MODE_NUM_MODES
};

// parameter names
enum {
	CHANNELSHIFT_INPUT = 0,
	CHANNELSHIFT_A_X,
	CHANNELSHIFT_A_Y,
	CHANNELSHIFT_B_X,
	CHANNELSHIFT_B_Y,
	CHANNELSHIFT_C_X,
	CHANNELSHIFT_C_Y,
	CHANNELSHIFT_MODE,
	CHANNELSHIFT_TOPIC_1,
	CHANNELSHIFT_TOPIC_2,
	CHANNELSHIFT_TOPIC_3,
	CHANNELSHIFT_NUM_PARAMS
};

// UI elements
enum {
	GAIN_DISK_ID = 1,
	A_X_DISK_ID,
	A_Y_DISK_ID,
	B_X_DISK_ID,
	B_Y_DISK_ID,
	C_X_DISK_ID,
	C_Y_DISK_ID,
	MODE_DISK_ID,
	
	TOPIC_START_1_ID,
	TOPIC_START_2_ID,
	TOPIC_START_3_ID,

	TOPIC_END_1_ID,
	TOPIC_END_2_ID,
	TOPIC_END_3_ID
	

};

// refcons
typedef struct GainInfo{
	PF_FpLong	gainF;
} GainInfo, *GainInfoP, **GainInfoH;


//
#ifdef __cplusplus
	extern "C" {
#endif
	
DllExport	PF_Err 
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra) ;

#ifdef __cplusplus
}
#endif

#endif // CHANNELSHIFT_H