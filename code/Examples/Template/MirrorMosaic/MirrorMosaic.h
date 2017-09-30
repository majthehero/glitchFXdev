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
	MirrorMosaic.h
*/

#pragma once

#ifndef MIRRORMOSAIC_H
#define MIRRORMOSAIC_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 0	// make sure we get 16bpc pixels; no need 
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

#include "MirrorMosaic_Strings.h"

/* Versioning information */
	// because of PiPL this will remain 1.0
#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	MIRROR_GRID_MIN 1
#define	MIRROR_GRID_DEF 4
#define	MIRROR_GRID_MAX 32

#define MIRROR_DT_MINNN -3000
#define MIRROR_DT_MIN -30
#define MIRROR_DT_DEF 3
#define MIRROR_DT_MAX 30 // at least one second in usual framerates
#define MIRROR_DT_MAXXX 3000 // whatever ;; big is good


enum {
	MIRROR_LEFT = 1,
	MIRROR_DOWN
};

enum {
	MIRR_INPUT = 0,
	MIRR_GRIDX,
	MIRR_GRIDY,
	MIRR_MIRRDIR,
	MIRR_DT,
	MIRR_LAYER,
	MIRR_NUM_PARAMS
};

enum {
	GRIDX_DISK_ID = 1,
	GRIDY_DISK_ID,
	MIRRDIR_DISK_ID,
	DELTAT_DISK_ID,
	LAYER_DISK_ID
};

typedef struct MirrorInfo{
	int mirrdir;
	PF_Rect *rect;
	PF_EffectWorld *input;
	PF_InData *in_data;
} MirrorInfo;

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

#endif // MIRRORMOSAIC_H