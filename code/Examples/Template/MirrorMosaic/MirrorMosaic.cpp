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

/*	MirrorMosaic.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision history: 

	1.0 (seemed like a good idea at the time)	bbb		6/1/2002

	1.0 Okay, I'm leaving the version at 1.0,	bbb		2/15/2006
		for obvious reasons; you're going to 
		copy these files directly! This is the
		first XCode version, though.

	1.0	Let's simplify this barebones sample	zal		11/11/2010

	2.0 Using template to implement				maj		28/9/2017
		MirrorMosaic

*/

#include "MirrorMosaic.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);
	// using frames from other times, not just current_time
	out_data->out_flags = PF_OutFlag_WIDE_TIME_INPUT; 
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	
		STR(StrID_GridX_Param_Name), 
		MIRROR_GRID_MIN, 
		MIRROR_GRID_MAX, 
		MIRROR_GRID_MIN, 
		MIRROR_GRID_MAX, 
		MIRROR_GRID_DEF,
		PF_Precision_INTEGER,
		0,
		0,
		GRIDX_DISK_ID);
	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(
		STR(StrID_GridY_Param_Name),
		MIRROR_GRID_MIN,
		MIRROR_GRID_MAX,
		MIRROR_GRID_MIN,
		MIRROR_GRID_MAX,
		MIRROR_GRID_DEF,
		PF_Precision_INTEGER,
		0,
		0,
		GRIDY_DISK_ID);
	AEFX_CLR_STRUCT(def);

	PF_ADD_POPUPX(
		STR(StrID_MirrorDir_Param_Name),
		2,
		1,
		"Horizontal|Vertical",
		PF_ParamFlag_NONE,
		MIRRDIR_DISK_ID);
	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(
		STR(StrID_DeltaT_Param_Name),
		MIRROR_DT_MINNN,
		MIRROR_DT_MAXXX,
		MIRROR_DT_MIN,
		MIRROR_DT_MAX,
		MIRROR_DT_DEF,
		PF_Precision_INTEGER,
		0,
		0,
		DELTAT_DISK_ID);
	AEFX_CLR_STRUCT(def);

	PF_ADD_LAYER(
		STR(StrID_Layer_Param_Name),
		PF_LayerDefault_MYSELF,
		LAYER_DISK_ID);

	out_data->num_params = MIRR_NUM_PARAMS;

	return err;
}


static PF_Err
MirrorPixFunc8(
	void		*refcon, // MirrorInfo
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, // unused
	PF_Pixel8	*outP) // only outP is writable
{
	PF_Err		err = PF_Err_NONE;
	MirrorInfo *miP = (MirrorInfo*)refcon;
	PF_InData *in_data = miP->in_data;
	PF_Pixel8 *inpixP = NULL;
	PF_LayerDef *input = (PF_LayerDef*)miP->input;
	int newX = xL;
	int newY = yL;
	
	int rectWidth = miP->rect->right - miP->rect->left;
	int rectNoX = xL / rectWidth; // starting at 0
	int rectHeight = miP->rect->bottom - miP->rect->top;
	int rectNoY = xL / rectHeight; // starting at 0


	if (miP->mirrdir == MIRROR_LEFT && 
		rectNoX % 2 == 1) {
		int relX = xL % rectWidth;
		newX = xL + rectWidth - 2 * relX;
	}
	if (miP->mirrdir == MIRROR_DOWN &&
		rectNoY % 2 == 1) {
		int relY = yL % rectHeight;
		newY = yL + rectHeight - 2 * relY;
	}

	ERR(PF_GET_PIXEL_DATA8(miP->input, NULL, &inpixP));
	if (!err) {
		inpixP += newY * miP->input->rowbytes / sizeof(PF_Pixel8) + newX;
		outP->alpha = inpixP->alpha;
		outP->red = inpixP->red;
		outP->green = inpixP->green;
		outP->blue = inpixP->blue;
	}
	
	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);
	A_long linesL = output->extent_hint.bottom - output->extent_hint.top;
	
	PF_Handle outworldH = suites.HandleSuite1()->host_new_handle(sizeof(PF_EffectWorld));
	PF_EffectWorld *outworldP = (PF_EffectWorld*)*outworldH;
	A_long outWidth = output->width;
	A_long outHeight = output->height;
	suites.WorldSuite1()->new_world(
		in_data->effect_ref,
		outWidth,
		outHeight,
		PF_PixelFormat_ARGB32,
		outworldP);
	
	/* 1. Grid setup */
	int gridX, gridY;
	gridX = (int)params[MIRR_GRIDX]->u.fs_d.value;
	gridY = (int)params[MIRR_GRIDY]->u.fs_d.value;
	int dX = output->width / gridX; // width of rect
	int dY = output->height / gridY; // height of rect
	PF_Rect in_rect = { 0, 0, dX, dY }; // rect coords type: A_long
	PF_Rect dest_rect = { 0, 0, 0, 0 };
	PF_Rect mirr_rect = { 0, 0, 0, 0 };

	/* 2. LOOP: checkout frame, place and mirror */
	PF_ParamDef checkout; // layeer from time+dT goes here
	AEFX_CLR_STRUCT(checkout);

	int dT = (int)params[MIRR_DT]->u.fs_d.value;
	int iX, iY;
	for (iX = 0; iX < gridX; iX++) { // vertical
		for (iY = 0; iY < gridY; iY++) { // horizontal
			int deltaT = dT * iY; // whole coll of rects same delay
			// checkout from time
			AEFX_CLR_STRUCT(checkout);
			ERR(PF_CHECKOUT_PARAM(
				in_data,
				MIRR_LAYER,
				(in_data->current_time + deltaT * in_data->time_step),
				in_data->time_step,
				in_data->time_scale,
				&checkout));
			if (!err) {

				// prepare dest rect
				dest_rect.left = iX * dX;
				dest_rect.right = (iX + 1) * dX;
				dest_rect.top = iY * dY;
				dest_rect.bottom = (iY + 1) * dY;

				mirr_rect.left = dest_rect.right;
				mirr_rect.right = dest_rect.left;
				mirr_rect.top = dest_rect.top;
				mirr_rect.bottom = dest_rect.bottom;

				// place rect if frame exists
				if (checkout.u.ld.data) {
					// first copy, then mirror
					ERR(suites.WorldTransformSuite1()->copy(
						in_data->effect_ref,
						&checkout.u.ld,
						output,
						&in_rect,
						&dest_rect));
					
				}
				// if frame not exist, empty black instead
				else {
					ERR(PF_FILL(NULL, &dest_rect, output)); // NULL = RGBA(0,0,0,0)
					continue;
				}
			}
			// every checkout must be checked-in
			PF_CHECKIN_PARAM(in_data, &checkout);
		}
	}
	
	// mirror
	ERR(suites.WorldTransformSuite1()->copy(
		in_data->effect_ref,
		output,
		outworldP,
		NULL,
		NULL));

	MirrorInfo mi;
	AEFX_CLR_STRUCT(mi);
	mi.input = outworldP;
	mi.rect = &in_rect;
	mi.mirrdir = params[MIRR_MIRRDIR]->u.pd.value;
	mi.in_data = in_data;

	ERR(suites.Iterate8Suite1()->iterate(
		in_data,
		0,
		linesL,
		outworldP,
		NULL,
		&mi,
		MirrorPixFunc8,
		output));

	return err;
}


DllExport	
PF_Err 
EntryPointFunc (
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

