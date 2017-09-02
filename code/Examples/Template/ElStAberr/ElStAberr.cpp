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

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Gain_Param_Name), 
							-1.0, 
							1.0, 
							-1.0, 
							1.0, 
							0.002,
							PF_Precision_TEN_THOUSANDTHS,
							0,
							0,
							GAIN_DISK_ID);

	AEFX_CLR_STRUCT(def);

	PF_ADD_COLOR(	STR(StrID_Color_Param_Name), 
					PF_HALF_CHAN8,
					PF_MAX_CHAN8,
					PF_MAX_CHAN8,
					COLOR_DISK_ID);
	
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


static PF_Err
MySimpleGainFunc8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	GainInfo *giP = reinterpret_cast<GainInfo*>(refcon);
	PF_InData *in_data = giP->inData;
	AEGP_SuiteHandler *suitesP = giP->suitesP;
	PF_Handle outworldH = giP->outworldH;
	double luma = lumaFromColor(inP); // luma [0.0, 1.0]
	// remember state
	int oldX, oldY;
	// recenter image
	int centerX, centerY; // !TODO parameter and that
	centerX = giP->outLayerP->width/2;
	centerY = giP->outLayerP->height/2;
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
	// debug - to compare
	oldX += centerX;
	oldY += centerY;

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
	/* Set up per frame memory */
	GainInfo			gi;
	AEFX_CLR_STRUCT(gi);
	A_long				linesL	= 0;
	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	gi.magnitudeF 	= params[ElStAberr_GAIN]->u.fs_d.value;
	PF_Handle outworldH = suites.HandleSuite1()->host_new_handle(sizeof(PF_EffectWorld));
	PF_EffectWorld *outworldP = (PF_EffectWorld*)*outworldH;
	A_long outWidth = output->width;
	A_long outHeight = output->height;
	suites.WorldSuite1()->new_world(
		NULL, 
		outWidth, 
		outHeight, 
		NULL, 
		outworldP);
	gi.outLayerP = reinterpret_cast<PF_LayerDef*>(outworldP);
	gi.inData = in_data;
	gi.suitesP = &suites;
	gi.outworldH = outworldH;

	/* Do render */
	if (PF_WORLD_IS_DEEP(output)){
		return PF_Err_INTERNAL_STRUCT_DAMAGED; // not supported - should drop deep color awarness.
	} else {
		ERR(suites.Iterate8Suite1()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												&params[ElStAberr_INPUT]->u.ld,	// src 
												NULL,							// area - null for all pixels
												(void*)&gi,					// refcon - your custom data pointer
												MySimpleGainFunc8,				// pixel function pointer
												output));	
	}
	/* Copy out data out */
	suites.WorldTransformSuite1()->copy(NULL,
		gi.outLayerP,
		output,
		NULL,
		NULL);

	/* Free memory */
	suites.WorldSuite1()->dispose_world(NULL, outworldP);
	suites.HandleSuite1()->host_dispose_handle(outworldH);

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

