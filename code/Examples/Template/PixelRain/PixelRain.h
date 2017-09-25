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
	PixelRain.h
*/

#pragma once

#ifndef PIXELRAIN_H
#define PIXELRAIN_H

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

#include <iostream>

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

#include "PixelRain_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	2
#define	MINOR_VERSION	1
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1

/* Function macros */
#define SQ( X ) ( X * X )
#define EUC_VAR_ALT( R, G, B) ( 2 * SQ( R ) + 4 * SQ( G ) + 3 * SQ( B ) )

/* Constants */
#define EPSILON 0.001

/* Parameter defaults */
#define	PIXELRAIN_LENGTH_MIN		0
#define	PIXELRAIN_LENGTH_MAX		1000
#define	PIXELRAIN_LENGTH_DFLT		30

#define	PIXELRAIN_DIFF_MIN		0
#define	PIXELRAIN_DIFF_MAX		1
#define	PIXELRAIN_DIFF_DFLT		0.3

enum {
	PIXELRAIN_INPUT = 0,
	PIXELRAIN_LENGTH,
	
	PIXELRAIN_HUE_CB,
	PIXELRAIN_HUE_TOLERANCE,

	PIXELRAIN_LUMA_CB,
	PIXELRAIN_LUMA_TOLERANCE,

	PIXELRAIN_SAT_CB,
	PIXELRAIN_SAT_TOLERANCE,

	PIXELRAIN_COLOR,
	PIXELRAIN_SHOW_MASK,
	PIXELRAIN_NUM_PARAMS
};

enum {
	LENGTH_DISK_ID = 1,

	HUECB_DISK_ID,
	HUETOL_DISK_ID,
	
	LUMACB_DISK_ID,
	LUMATOL_DISK_ID,
	
	SATCB_DISK_ID,
	SATTOL_DISK_ID,
	
	COLOR_DISK_ID,
	MASKCB_DISK_ID
};

typedef struct TrailInfo {
	PF_FpLong	lengthF;
	PF_LayerDef *input; // read-only, unless same as output, see below
	PF_LayerDef *output; // each thread should only modify/write to one column
	PF_LayerDef *pixelMask; // read-only after generated
	PF_InData *in_data; // needed for accessor macros
} TrailInfo;

/*
Luma, hue and sat are double between 0.0 and 1.0
*/
typedef struct ColorHSL {
	double luma;
	double hue;
	double saturation;
} ColorHSL;

// info required to generate pix mask - select pixels to affect
typedef struct PixSelInfo {
	PF_Pixel tgtColor;
	PF_Boolean hueCheck;
	PF_FpLong hueTolerance;
	PF_Boolean lumaCheck;
	PF_FpLong lumaTolerance;
	PF_Boolean satCheck;
	PF_FpLong satTolerance;
	PF_Boolean showMask;
} PixSelInfo;

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

#endif // PIXELRAIN_H