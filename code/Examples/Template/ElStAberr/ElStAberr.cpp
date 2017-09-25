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

/*	ElStAberr.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision history: 

	1.0 (seemed like a good idea at the time)	bbb		6/1/2002

	1.0 Okay, I'm leaving the version at 1.0,	bbb		2/15/2006
		for obvious reasons; you're going to 
		copy these files directly! This is the
		first XCode version, though.

	1.0	Let's simplify this barebones sample	zal		11/11/2010

*/

#include "ElStAberr.h"

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

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;	// just 16bpc, not 32bpc
	
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
		STR(StrID_Gain_Param_Name), 
		ElStAberr_MAGNITUDE_MIN,
		ElStAberr_MAGNITUDE_MAX, 
		ElStAberr_MAGNITUDE_MIN, 
		ElStAberr_MAGNITUDE_MAX, 
		ElStAberr_MAGNITUDE_DFLT,
		PF_Precision_TEN_THOUSANDTHS,
		0,
		0,
		GAIN_DISK_ID);
	AEFX_CLR_STRUCT(def);
	
	PF_ADD_CHECKBOX("FILL IN",
		"Fill In",
		false,
		NULL,
		FILLIN_CB_ID);
	AEFX_CLR_STRUCT(def);

	int centerX = 0;
	int centerY = 0;
	if (output) {
		centerX = output->width / 2;
		centerY = output->height / 2;
	}
	def.flags = PF_ParamFlag_SUPERVISE;
	PF_ADD_POINT(
		STR(StrID_Center_Param_Name),
		centerX,
		centerY,
		NULL,
		CENTER_DISK_ID);
	AEFX_CLR_STRUCT(def);

	out_data->num_params = ElStAberr_NUM_PARAMS;

	return err;
}


/* tested & done
HSL FROM COLOR - source "http://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/" 14. aug 2017 12:59
param inP: pixel in rgb format
returns ColorHSL
*/
static double//ColorHSL*
lumaFromColor(
	PF_Pixel8 *inP) {
	/* 1. char to float */
	double rF = inP->red / 255.0;
	double gF = inP->green / 255.0;
	double bF = inP->blue / 255.0;
	/* 2. find max and min */
	double minF = (rF < gF ? rF : gF) < bF ? (rF < gF ? rF : gF) : bF;
	double maxF = (rF > gF ? rF : gF) > bF ? (rF > gF ? rF : gF) : bF;
	/* 3. luminance */
	double lumF = (minF + maxF) / 2.0;
	
	return lumF;
}

/* tested & done
color pixels RGBA(xFF,x00,xFF,x00)
for easily recognisable blank state
*/
static PF_Err
ColorBlankPreAberGapsPixFunc8(
	void		*refcon, // null
	A_long		xL,
	A_long		yL,
	PF_Pixel8	*inP, // null
	PF_Pixel8	*outP)
{
	outP->alpha = 0;
	outP->red = 255;
	outP->green = 0;
	outP->blue = 255;
	return PF_Err_NONE;
}


static PF_Err
FillAfterAberGapsPixFunc8 (
	void		*refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel8	*inP,
	PF_Pixel8	*outP)
{
	PF_Err err = PF_Err_NONE;
	AfterAberInfo *aaiP = (AfterAberInfo *)refcon;
	PF_InData *in_data = aaiP->inData;
	// check for easily recognisable blank state
	A_long nearY = yL;
	A_long nearX = xL;
	// center is where center is
	int centerX = (int)aaiP->center_x;
	int centerY = (int)aaiP->center_y;
	if (inP->alpha == 0) {
		bool notDone = true;
		int count = 0;
		while (notDone) {
			// !TODO rewrite for better direction to center
			PF_Pixel8 *nearPixP = NULL;
			// find nearest written coords
			if (nearY <= centerY) {
				nearY += 1;
			}
			else {
				nearY -= 1;
			}
			if (nearX <= centerX) {
				nearX += 1;
			}
			else {
				nearX -= 1;
			}
			PF_GET_PIXEL_DATA8(aaiP->worldP, NULL, &nearPixP);
			nearPixP += nearY * aaiP->worldP->rowbytes / sizeof(PF_Pixel8) + nearX;
			if (nearPixP->alpha > 0) {
				notDone = false;
				outP->alpha = nearPixP->alpha;
				outP->red = nearPixP->red;
				outP->green = nearPixP->green;
				outP->blue = nearPixP->blue;
			}
			count++;
			if (count > aaiP->worldWidth) {
				notDone = false;
			}
		}
	}
	return err;
}


static PF_Err
LumaAberatePixFunc8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP) // output unused
{
	PF_Err		err = PF_Err_NONE;

	AberInfo *giP = reinterpret_cast<AberInfo*>(refcon);
	PF_InData *in_data = giP->inData;
	AEGP_SuiteHandler *suitesP = giP->suitesP;
	double luma = lumaFromColor(inP); // luma [0.0, 1.0]
	// remember state
	int oldX, oldY;
	// recenter image
	int centerX, centerY;
	centerX = (int)giP->center_x;
	centerY = (int)giP->center_y;

	oldX = xL - centerX;
	oldY = yL - centerY;
	// calculate scaling factor
	int newX, newY;
	double multiplier = giP->magnitudeF * 2.0; // [-2, 2]
	double modifier = giP->magnitudeF * -1.0 + 1.0; // [+2, 0]
	double scaleDelta = multiplier * luma + modifier;
	// scale image
	newX = (int)(oldX * scaleDelta);
	newY = (int)(oldY * scaleDelta);

	// recenter image
	newX += centerX;
	newY += centerY;

	// check for out of bounds pixels
	A_long outWidth = giP->outLayerP->width;
	A_long outHeight = giP->outLayerP->height;
	if (newX < 0 || newY < 0 || newX >= outWidth || newY >= outHeight) {
		return PF_Err_NONE;
	}

	// prepare pointers
	PF_Pixel8 *outPixP = NULL;
	PF_GET_PIXEL_DATA8(reinterpret_cast<PF_EffectWorld*>(giP->outLayerP), NULL, &outPixP);
	outPixP += (giP->outLayerP->rowbytes / sizeof(PF_Pixel8) * newY) + newX;
	
	// write pixel
	outPixP->alpha = inP->alpha;
	outPixP->red = inP->red;
	outPixP->green = inP->green;
	outPixP->blue = inP->blue;
		
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
	
	// refcon -> LumaAberate
	AberInfo aberInfo;
	AEFX_CLR_STRUCT(aberInfo);

	// input data
	A_long				linesL	= 0;
	linesL 		= output->extent_hint.bottom - output->extent_hint.top;

	/* Do render */
	// 1. Color world easily recognisible blank state
	ERR(suites.Iterate8Suite1()->iterate(
		in_data,
		0,								// progress base
		linesL,							// progress final
		NULL,							// src 
		NULL,							// area - null for all pixels
		NULL,							// refcon - your custom data pointer
		ColorBlankPreAberGapsPixFunc8,	// pixel function pointer
		output));
	// 2. Luma (Electro statically) aberate
	aberInfo.magnitudeF = params[ElStAberr_GAIN]->u.fs_d.value;
	// new output world
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

	// refcon -> LumaAberatePixFunc8
	aberInfo.outLayerP = reinterpret_cast<PF_LayerDef*>(outworldP);
	aberInfo.inData = in_data;
	aberInfo.suitesP = &suites;
	aberInfo.outworldH = outworldH;
	aberInfo.center_x = (PF_FpLong)params[ElStAberr_CENTER]->u.td.x_value;
	aberInfo.center_y = (PF_FpLong)params[ElStAberr_CENTER]->u.td.y_value;

	ERR(suites.Iterate8Suite1()->iterate(	
		in_data,
		0,								
		linesL,							
		&params[ElStAberr_INPUT]->u.ld,	
		NULL,				
		(void*)&aberInfo,			
		LumaAberatePixFunc8,
		output));	// dest EffectWorld not used in LumaAberPixFunc

	// 3. Fill emptynesses
	// new world
	PF_Handle fillinWorldH = suites.HandleSuite1()->host_new_handle(sizeof(PF_EffectWorld));
	PF_EffectWorld *fillInWorldP = (PF_EffectWorld*)*fillinWorldH;
	A_long fiwWidth = outworldP->width; // only fill in what users care about
	A_long fiwHeight = outworldP->height;
	suites.WorldSuite1()->new_world(
		in_data->effect_ref,
		fiwWidth,
		fiwHeight,
		PF_PixelFormat_ARGB32,
		fillInWorldP);
	// refcon setup
	AfterAberInfo aai;
	AEFX_CLR_STRUCT(aai);
	aai.worldHeight = output->height;
	aai.worldWidth = output->width;
	aai.worldP = outworldP;
	aai.inData = in_data;
	aai.center_x = aberInfo.center_x;
	aai.center_y = aberInfo.center_y;

	ERR(suites.Iterate8Suite1()->iterate(
		in_data,
		0,								// progress base
		linesL,							// progress final
		outworldP,	
		NULL,							// area - null for all pixels
		(void*)&aai,					// refcon - your custom data pointer
		FillAfterAberGapsPixFunc8,		// pixel function pointer
		fillInWorldP));
	// transfer mode for fill in
	PF_Handle simpleBehindH = (PF_Handle)suites.HandleSuite1()->host_new_handle(sizeof(PF_CompositeMode));
	PF_CompositeMode *simpleBehind = (PF_CompositeMode*)*simpleBehindH;
	simpleBehind->opacity = 100;
	simpleBehind->xfer = PF_Xfer_BEHIND;
	// blend fill in under output
	suites.WorldTransformSuite1()->composite_rect(
		in_data->effect_ref,
		NULL,
		100, // A_long opacitity -- 0 to ??
		fillInWorldP,
		0, 0, 
		PF_Field_FRAME,
		PF_Xfer_BEHIND,
		outworldP
	);
	/*suites.WorldTransformSuite1()->transform_world(
		in_data->effect_ref,
		PF_Quality_LO,
		NULL,
		NULL,
		fillInWorldP,
		simpleBehind,
		NULL,
		NULL,
		0,
		NULL,
		NULL,
		output
	);*/
	// put output out
	suites.WorldTransformSuite1()->copy(
		NULL,
		outworldP,
		output,
		NULL,
		NULL
	);
	/* Free memory */
	suites.WorldSuite1()->dispose_world(in_data->effect_ref, outworldP);
	suites.HandleSuite1()->host_dispose_handle(outworldH);
	suites.WorldSuite1()->dispose_world(in_data->effect_ref, fillInWorldP);
	suites.HandleSuite1()->host_dispose_handle(fillinWorldH);
	suites.HandleSuite1()->host_dispose_handle(simpleBehindH);
	return err;
}


//static PF_Err
//UserChangedParam(
//	PF_InData						*in_data,
//	PF_OutData						*out_data,
//	PF_ParamDef						*params[],
//	PF_LayerDef						*outputP,
//	const PF_UserChangedParamExtra	*which_hitP)
//{
//	PF_Err				err = PF_Err_NONE;
//
//
//	return err;
//}

static PF_Err
UpdateParameterUI(
	PF_InData			*in_data,
	PF_OutData			*out_data,
	PF_ParamDef			*params[],
	PF_LayerDef			*outputP)
{
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	PF_ParamDef def;
	AEFX_CLR_STRUCT(def);

	if (outputP) { // !TODO unsure if needed now that in params setup this is ..set up
		def = *params[ElStAberr_CENTER];
		def.u.td.x_dephault = outputP->width / 2;
		def.u.td.y_dephault = outputP->height / 2;
		def.u.td.x_value = outputP->width / 2;
		def.u.td.y_value = outputP->height / 2;
		suites.ParamUtilsSuite3()->PF_UpdateParamUI(
			in_data->effect_ref,
			ElStAberr_CENTER,
			&def);
	}

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
				err = About(
					in_data,
					out_data,
					params,
					output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(	
					in_data,
					out_data,
					params,
					output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(	
					in_data,
					out_data,
					params,
					output);
				break;
				
			case PF_Cmd_RENDER:
				err = Render(
					in_data,
					out_data,
					params,
					output);
				break;

				// user changed param not really needed after initial correct  of center coords
			/*case PF_Cmd_USER_CHANGED_PARAM:
				err = UserChangedParam(
					in_data,
					out_data,
					params,
					output,
					reinterpret_cast<const PF_UserChangedParamExtra *>(extra));*/

				case PF_Cmd_UPDATE_PARAMS_UI:
				err = UpdateParameterUI(	
					in_data,
					out_data,
					params,
					output);
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

