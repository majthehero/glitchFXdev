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

/*	ChannelShift.cpp	

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

#include "ChannelShift.h"


//char * MODE_NAMES = "RGB|HSL|YUV";
char * TOPIC_NAMES[3][3] = {
	{ "Red", "Green", "Blue" },
	{ "Hue", "Saturation", "Luma" },
	{ "Luma", "U - chroma", "V - chroma" },
};

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
	
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	AEFX_CLR_STRUCT(def);
	
	// mode dropdown
	def.flags = PF_ParamFlag_SUPERVISE; // set flags before adding params - unnecessary 
	PF_ADD_POPUPX(	
		STR(StrID_Mode_Param_Name), 
		CHANNELSHIFT_MODE_NUM_MODES,
		CHANNELSHIFT_MODE_RGB, 
		STR(StrID_Mode_Names),
		PF_ParamFlag_SUPERVISE,
		MODE_DISK_ID
	);
	AEFX_CLR_STRUCT(def);
	// sliders shift
	for (int i = 0; i < 3; i++) {
		PF_ADD_TOPICX(TOPIC_NAMES[CHANNELSHIFT_MODE_RGB][i], PF_Param_GROUP_START, TOPIC_START_1_ID + i);
		AEFX_CLR_STRUCT(def);
		suites.UtilitySuite6();
		PF_ADD_FLOAT_SLIDERX(
			STR(StrID_R_X_Param_Name),
			CHANNELSHIFT_X_MIN,
			CHANNELSHIFT_X_MAX,
			CHANNELSHIFT_X_LOW,
			CHANNELSHIFT_X_HIGH,
			CHANNELSHIFT_X_DEF,
			PF_Precision_INTEGER,
			NULL,
			NULL,
			A_X_DISK_ID + 2 * i
		);
		AEFX_CLR_STRUCT(def);

		PF_ADD_FLOAT_SLIDERX(
			STR(StrID_R_Y_Param_Name),
			CHANNELSHIFT_Y_MIN,
			CHANNELSHIFT_Y_MAX,
			CHANNELSHIFT_Y_LOW,
			CHANNELSHIFT_Y_HIGH,
			CHANNELSHIFT_Y_DEF,
			PF_Precision_INTEGER,
			NULL,
			NULL,
			A_Y_DISK_ID + 2 * i
		);
		AEFX_CLR_STRUCT(def);

		PF_END_TOPIC(TOPIC_END_1_ID + i);
		AEFX_CLR_STRUCT(def);
	}
	
	out_data->num_params = CHANNELSHIFT_NUM_PARAMS;

	return err;
}


/*
Copies parameter values into a new array
Needed for changing UI and param values
*/
static PF_Err
MakeParamCopy(
	PF_ParamDef *actual[],	/* >> */
	PF_ParamDef copy[])		/* << */
{
	for (A_short iS = 0; iS < CHANNELSHIFT_NUM_PARAMS; ++iS) {
		AEFX_CLR_STRUCT(copy[iS]);
	}
	copy[CHANNELSHIFT_INPUT] = *actual[CHANNELSHIFT_INPUT];
	copy[CHANNELSHIFT_A_X] = *actual[CHANNELSHIFT_A_X];
	copy[CHANNELSHIFT_B_X] = *actual[CHANNELSHIFT_B_X];
	copy[CHANNELSHIFT_C_X] = *actual[CHANNELSHIFT_C_X];
	copy[CHANNELSHIFT_A_Y] = *actual[CHANNELSHIFT_A_Y];
	copy[CHANNELSHIFT_B_Y] = *actual[CHANNELSHIFT_B_Y];
	copy[CHANNELSHIFT_C_Y] = *actual[CHANNELSHIFT_C_Y];
	copy[CHANNELSHIFT_MODE] = *actual[CHANNELSHIFT_MODE];
	copy[CHANNELSHIFT_TOPIC_1] = *actual[CHANNELSHIFT_TOPIC_1];
	copy[CHANNELSHIFT_TOPIC_2] = *actual[CHANNELSHIFT_TOPIC_2];
	copy[CHANNELSHIFT_TOPIC_3] = *actual[CHANNELSHIFT_TOPIC_3];
	
	return PF_Err_NONE;
}

/* TODO
Called when ECP (UI) needs to be updated
change param UI here - update sliders based on mode selection 
*/
static PF_Err
//UpdateParamUI(
UserChangedParam(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	const PF_UserChangedParamExtra	*which_hitP)
{
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	PF_ParamDef param_copy[CHANNELSHIFT_NUM_PARAMS];
	ERR(MakeParamCopy(params, param_copy));
	
	// mode changed - change names of sliders
	if (CHANNELSHIFT_MODE_RGB == params[CHANNELSHIFT_MODE]->u.pd.value) {
		// change sliders
		strcpy_s(param_copy[CHANNELSHIFT_A_X].name, STR(StrID_R_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_A_Y].name, STR(StrID_R_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_X].name, STR(StrID_G_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_Y].name, STR(StrID_G_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_X].name, STR(StrID_B_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_Y].name, STR(StrID_B_Y_Param_Name));
	}
	else if (CHANNELSHIFT_MODE_HSL == params[CHANNELSHIFT_MODE]->u.pd.value) {
		// change sliders
		strcpy_s(param_copy[CHANNELSHIFT_A_X].name, STR(StrID_H_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_A_Y].name, STR(StrID_H_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_X].name, STR(StrID_S_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_Y].name, STR(StrID_S_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_X].name, STR(StrID_L_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_Y].name, STR(StrID_L_Y_Param_Name));
	}
	else if (CHANNELSHIFT_MODE_YUV == params[CHANNELSHIFT_MODE]->u.pd.value) {
		// change sliders
		strcpy_s(param_copy[CHANNELSHIFT_A_X].name, STR(StrID_Y_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_A_Y].name, STR(StrID_Y_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_X].name, STR(StrID_U_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_B_Y].name, STR(StrID_U_Y_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_X].name, STR(StrID_V_X_Param_Name));
		strcpy_s(param_copy[CHANNELSHIFT_C_Y].name, STR(StrID_V_Y_Param_Name));
	}
	// change topics
	strcpy_s(param_copy[CHANNELSHIFT_TOPIC_1].name, TOPIC_NAMES[params[CHANNELSHIFT_MODE]->u.pd.value][1]);
	strcpy_s(param_copy[CHANNELSHIFT_TOPIC_2].name, TOPIC_NAMES[params[CHANNELSHIFT_MODE]->u.pd.value][2]);
	strcpy_s(param_copy[CHANNELSHIFT_TOPIC_3].name, TOPIC_NAMES[params[CHANNELSHIFT_MODE]->u.pd.value][3]);
	
	// call to actually update UI
	PF_ProgPtr effect_ref = in_data->effect_ref;
	// x sliders
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_A_X,
		&param_copy[CHANNELSHIFT_A_X]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_B_X,
		&param_copy[CHANNELSHIFT_B_X]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_C_X,
		&param_copy[CHANNELSHIFT_C_X]));
	// y sliders
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_A_Y,
		&param_copy[CHANNELSHIFT_A_Y]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_B_Y,
		&param_copy[CHANNELSHIFT_B_Y]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_C_Y,
		&param_copy[CHANNELSHIFT_C_Y]));
	// topics
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_TOPIC_1,
		&param_copy[CHANNELSHIFT_TOPIC_1]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_TOPIC_2,
		&param_copy[CHANNELSHIFT_TOPIC_2]));
	ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(
		effect_ref,
		CHANNELSHIFT_TOPIC_3,
		&param_copy[CHANNELSHIFT_TOPIC_3]));

	return err;
}


/* TODO
Called when parameter values change
Other param values can be changed
which_hitP is changed param - source of call
*
static PF_Err
UserChangedParam(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	const PF_UserChangedParamExtra	*which_hitP)
{
	PF_Err		err = PF_Err_NONE;
	
	if (which_hitP->param_index == CHANNELSHIFT_MODE) {
		params[CHANNELSHIFT_A_X]->u.fs_d.value = 0;
		params[CHANNELSHIFT_B_X]->u.fs_d.value = 0;
		params[CHANNELSHIFT_C_X]->u.fs_d.value = 0;
		params[CHANNELSHIFT_A_Y]->u.fs_d.value = 0;
		params[CHANNELSHIFT_B_Y]->u.fs_d.value = 0;
		params[CHANNELSHIFT_C_Y]->u.fs_d.value = 0;
	}

	return err;
}
/**/

static PF_Err
MySimpleGainFunc16 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;

	GainInfo	*giP	= reinterpret_cast<GainInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (giP){
		tempF = giP->gainF * PF_MAX_CHAN16 / 100.0;
		if (tempF > PF_MAX_CHAN16){
			tempF = PF_MAX_CHAN16;
		};

		outP->alpha		=	inP->alpha;
		outP->red		=	MIN((inP->red	+ (A_u_char) tempF), PF_MAX_CHAN16);
		outP->green		=	MIN((inP->green	+ (A_u_char) tempF), PF_MAX_CHAN16);
		outP->blue		=	MIN((inP->blue	+ (A_u_char) tempF), PF_MAX_CHAN16);
	}

	return err;
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

	GainInfo	*giP	= reinterpret_cast<GainInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (giP){
		tempF = giP->gainF * PF_MAX_CHAN8 / 100.0;
		if (tempF > PF_MAX_CHAN8){
			tempF = PF_MAX_CHAN8;
		};

		outP->alpha		=	inP->alpha;
		outP->red		=	MIN((inP->red	+ (A_u_char) tempF), PF_MAX_CHAN8);
		outP->green		=	MIN((inP->green	+ (A_u_char) tempF), PF_MAX_CHAN8);
		outP->blue		=	MIN((inP->blue	+ (A_u_char) tempF), PF_MAX_CHAN8);
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

	/*	Put interesting code here. */
	GainInfo			giP;
	AEFX_CLR_STRUCT(giP);
	A_long				linesL	= 0;

	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	//giP.gainF 	= params[CHANNELSHIFT_GAIN]->u.fs_d.value;
	
	if (PF_WORLD_IS_DEEP(output)){
		ERR(suites.Iterate16Suite1()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												&params[CHANNELSHIFT_INPUT]->u.ld,	// src 
												NULL,							// area - null for all pixels
												(void*)&giP,					// refcon - your custom data pointer
												MySimpleGainFunc16,				// pixel function pointer
												output));
	} else {
		ERR(suites.Iterate8Suite1()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												&params[CHANNELSHIFT_INPUT]->u.ld,	// src 
												NULL,							// area - null for all pixels
												(void*)&giP,					// refcon - your custom data pointer
												MySimpleGainFunc8,				// pixel function pointer
												output));	
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
					output
				);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(	
					in_data,
					out_data,
					params,
					output
				);
				break;
			
			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(	
					in_data,
					out_data,
					params,
					output
				);
				break;
				
			case PF_Cmd_RENDER:
				err = Render(	
					in_data,
					out_data,
					params,
					output
				);
				break;

			/*case PF_Cmd_UPDATE_PARAMS_UI:
				err = UpdateParamUI(
					in_data,
					out_data,
					params,
					output
				);
				break;*/

			case PF_Cmd_USER_CHANGED_PARAM:
				err = UserChangedParam( 
					in_data,
					out_data,
					params,
					output,
					reinterpret_cast<const PF_UserChangedParamExtra *>(extra)
				);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

