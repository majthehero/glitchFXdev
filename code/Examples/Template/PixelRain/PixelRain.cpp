/*	Skeleton.cpp	

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

/*  PixelRain.cpp

	This is a customizable pixel rain visual effect. Pixels that
	are similar enough to target color leave trail.

	Revision history:

	1.0 Started from Skeleton template.			maj		3/7/2017
	1.1 Correctly generates pixel mask.			maj		7/7/2017
*/

#include "PixelRain.h"

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
	// length slider
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Length_Param_Name), 
							PIXELRAIN_LENGTH_MIN, 
							PIXELRAIN_LENGTH_MAX, 
							PIXELRAIN_LENGTH_MIN, 
							PIXELRAIN_LENGTH_MAX, 
							PIXELRAIN_LENGTH_DFLT,
							PF_Precision_INTEGER,
							0,
							0,
							LENGTH_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// color diff slider
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Diff_Param_Name),
		PIXELRAIN_DIFF_MIN,
		PIXELRAIN_DIFF_MAX,
		PIXELRAIN_DIFF_MIN,
		PIXELRAIN_DIFF_MAX,
		PIXELRAIN_DIFF_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		DIFF_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// color picker
	PF_ADD_COLOR(	STR(StrID_Color_Param_Name), 
					PF_HALF_CHAN8,
					PF_MAX_CHAN8,
					PF_MAX_CHAN8,
					COLOR_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// advanced alpha blending checkbox - TODO not implemented
	PF_ADD_CHECKBOX(STR(StrID_AdvAlpha_Param_Name),
		"",
		false,
		0,
		ADVALPHA_DISK_ID);
	AEFX_CLR_STRUCT(def);

	out_data->num_params = PIXELRAIN_NUM_PARAMS;

	return err;
}

static PF_Err
CheckColorPixFunc16 ( // TODO not implemented
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;
	outP->alpha		=	255;
	outP->red		=	128;
	outP->green		=	64;
	outP->blue		=	64;
	return err;
}

static PF_Err
CheckColorPixFunc (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;
	PixSelInfo *psiP = (PixSelInfo*)refcon;
	PF_Pixel8 tgtColor = psiP->tgtColor;
	// using euclidean variance, altered for human perception
	float deltaR, deltaG, deltaB;
	deltaR = (float)(inP->red - tgtColor.red);
	deltaG = (float)(inP->green - tgtColor.green);
	deltaB = (float)(inP->blue - tgtColor.blue);
	float eucVarAlt = EUC_VAR_ALT(deltaR, deltaG, deltaB);
	float maxVar = EUC_VAR_ALT(255, 255, 255);
	// advanced alpha blending	
	outP->alpha = (A_u_char) (eucVarAlt / maxVar * 255);
	// basic boolean blending
	if (!psiP->advAlpha) {
		if (outP->alpha > (A_u_char)(psiP->diff*255))
			outP->alpha = 255;
		else
			outP->alpha = 0;
	}
	else {
		PF_FpLong tempAlpha = outP->alpha * (5*psiP->diff);
		if (tempAlpha > 255) tempAlpha = 255;
		outP->alpha = (A_u_char)tempAlpha;
	}
#ifdef TEST1
	outP->red = 255;
	outP->green = 0;
	outP->blue = 255;
#endif
	return err;
}

static PF_Err
GenerateTrailColFunc(
	void *refcon, // trailInfoP
	A_long thread_indexL, // if 0, answer abort and progress report requests
	A_long i,
	A_long iterationsL) 
{
	PF_Err err = PF_Err_NONE;
	TrailInfo *tilP = (TrailInfo*)refcon;
	PF_InData *in_data = tilP->in_data;

	A_long columnI = i;
	A_long w = tilP->input->width;
	A_long h = tilP->input->height;
	A_long rowbytes = tilP->input->rowbytes;

	PF_Pixel8 *trail = (PF_Pixel8*)calloc((int)tilP->lengthF, sizeof(PF_Pixel8)); // TEST are pixels initialized to black
	int trail_size = 0;
	// TODO not sure if needed. Have to think this algo through a bit more.

	PF_Pixel8 *inP = (PF_Pixel8*)tilP->input->data;
	PF_Pixel8 *maskP = (PF_Pixel8*)tilP->pixelMask->data;
	PF_Pixel8 *outP = (PF_Pixel8*)tilP->output->data;
	PF_GET_PIXEL_DATA8(tilP->input, NULL, &inP);
	PF_GET_PIXEL_DATA8(tilP->pixelMask, NULL, &maskP);
	PF_GET_PIXEL_DATA8(tilP->output, NULL, &outP);
	// iterate through column
	for (int i = 0; i < iterationsL; i++) {
		/* don't want div by zero */
		if (inP->alpha < 0.001 && maskP->alpha < 0.001) {
			/* add pixel to trail */
			char alpha = (char)((inP->alpha / 255.0 * maskP->alpha / 255.0) * 255);
			if (alpha > 0.02) {
				std::cerr << inP->alpha << " * " << maskP->alpha << " = " << alpha << "\n"; // TEST if this work correctly, then remove
				PF_Pixel8 temp = *inP;
				temp.alpha = alpha;
				trail[trail_size] = temp;
				trail_size++;
			}
			/* add trail to output */
			PF_Pixel8 *tempPix = new PF_Pixel8();
			tempPix->alpha = 0;
			tempPix->red = 0;
			tempPix->green = 0;
			tempPix->blue = 0;
			for (int j = trail_size - 1; j >= 0; j--) { // latest pixels should contribute more
				if (255 - tempPix->alpha < trail[j].alpha) {
					break;
				}
				tempPix->alpha += trail[j].alpha;
				tempPix->red += (char)(trail[j].red * trail[j].alpha / 255.0);
				tempPix->green += (char)(trail[j].green * trail[j].alpha / 255.0);
				tempPix->blue += (char)(trail[j].blue * trail[j].alpha / 255.0);
			}
			outP->alpha = tempPix->alpha;
			outP->red = tempPix->red;
			outP->green = tempPix->green;
			outP->blue = tempPix->blue;
		}
		/* change pixel pointers */
		inP += tilP->input->rowbytes;
		outP += tilP->input->rowbytes;
		maskP += tilP->input->rowbytes;
		PF_GET_PIXEL_DATA8(tilP->input, NULL, &inP);
		PF_GET_PIXEL_DATA8(tilP->pixelMask, NULL, &maskP);
		PF_GET_PIXEL_DATA8(tilP->output, NULL, &outP);
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
	/* 0. Data seg */
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	A_long linesL = output->extent_hint.bottom - output->extent_hint.top;
	PixSelInfo psi;
	AEFX_CLR_STRUCT(psi);
	psi.advAlpha = params[PIXELRAIN_ADVALPHA]->u.bd.value;
	psi.diff = params[PIXELRAIN_DIFF]->u.fs_d.value;
	psi.tgtColor = params[PIXELRAIN_COLOR]->u.cd.value;

	/* 1. Generate pixel mask */
	PF_EffectWorld * colorMaskP = new PF_EffectWorld(params[PIXELRAIN_INPUT]->u.ld);
	suites.WorldSuite1()->new_world(NULL,
		params[PIXELRAIN_INPUT]->u.ld.width,
		params[PIXELRAIN_INPUT]->u.ld.height,
		NULL,
		colorMaskP);
	if (PF_WORLD_IS_DEEP(output)) { // for 16bpp colors TODO not implemented
		//ERR(suites.Iterate16Suite1()->iterate(in_data,
		//	0,								// progress base
		//	linesL,							// progress final
		//	&params[PIXELRAIN_INPUT]->u.ld,	// src 
		//	NULL,							// area - null for all pixels
		//	(void*)&psi,					// refcon - your custom data pointer
		//	CheckColorPixFunc16,			// pixel function pointer
		//	colorMaskP));
	} else {
		ERR(suites.Iterate8Suite1()->iterate(in_data,
			0,								// progress base
			linesL,							// progress final
			&params[PIXELRAIN_INPUT]->u.ld,	// src 
			NULL,							// area - null for all pixels
			(void*)&psi,					// refcon - your custom data pointer
			CheckColorPixFunc,				// pixel function pointer
			colorMaskP));
	}
#ifdef TEST1
	suites.WorldTransformSuite1()->copy(NULL,
		colorMaskP,
		output,
		NULL,
		NULL);
	return err;
#endif

	/* 2. Generate trails */
	TrailInfo *tilP = new TrailInfo();
	tilP->lengthF = params[PIXELRAIN_LENGTH]->u.fs_d.value;
	tilP->in_data = in_data;
	tilP->input = &params[PIXELRAIN_INPUT]->u.ld;
	tilP->pixelMask = colorMaskP;
	tilP->output = output;
	ERR(suites.Iterate8Suite1()->iterate_generic(params[PIXELRAIN_INPUT]->u.ld.width,
		tilP,
		GenerateTrailColFunc));
#ifdef TEST3
	suites.WorldTransformSuite1()->copy(NULL,
		colorMaskP,
		output,
		NULL,
		NULL);
	return err;
#endif
	
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

