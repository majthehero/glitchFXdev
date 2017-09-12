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
	ElStAberr.h
*/

#pragma once

#ifndef ElStAberr_H
#define ElStAberr_H

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

#include "ElStAberr_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	ElStAberr_MAGNITUDE_MIN		-1.0
#define	ElStAberr_MAGNITUDE_MAX		1.0
#define	ElStAberr_MAGNITUDE_DFLT	0.02




enum {
	ElStAberr_INPUT = 0,
	ElStAberr_GAIN,
	ElStAberr_FILLIN,
	ElStAberr_CENTER,
	ElStAberr_NUM_PARAMS
};

enum {
	GAIN_DISK_ID = 1,
	FILLIN_CB_ID,
	CENTER_DISK_ID
};

typedef struct GainInfo{
	// in data
	PF_FpLong	magnitudeF;
	PF_InData *inData;
	AEGP_SuiteHandler *suitesP;
	PF_Handle outworldH;

	PF_FpLong center_x;
	PF_FpLong center_y;
	// out data
	PF_LayerDef *outLayerP;
} GainInfo, *GainInfoP, **GainInfoH;

typedef struct AfterAberInfo{
	A_long worldHeight, worldWidth;
	PF_EffectWorld *worldP;
	PF_InData *inData;
	PF_FpLong center_x;
	PF_FpLong center_y;
} AfterAberInfo;

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

#endif // ElStAberr_H