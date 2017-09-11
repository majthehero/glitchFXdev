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

	1.0 Started from Skeleton template.			maj		03/07/2017
	1.1 Correctly generates pixel mask.			maj		07/07/2017
	2.0 Works. Possible mem leaks.				maj		16/08/2017
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

/*
UI SET UP
AE callback
*/
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
	// hue cb
	PF_ADD_CHECKBOX(	STR(StrID_HueCB_Param_Name),
						STR(StrID_HueCB_Param_Name),
						true,
						NULL,
						HUECB_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// hue slider
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_HueSlider_Param_Name),
							0,
							1,
							0,
							1,
							0.3,
							PF_Precision_THOUSANDTHS,
							0,
							0,
							HUETOL_DISK_ID);
	// luma cb
	PF_ADD_CHECKBOX(	STR(StrID_LumaCB_Param_Name),
						STR(StrID_LumaCB_Param_Name),
						false,
						NULL,
						LUMACB_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// luma slider
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LumaSlider_Param_Name),
							0,
							1,
							0,
							1,
							0.3,
							PF_Precision_THOUSANDTHS,
							0,
							0,
							LUMATOL_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// sat cb
	PF_ADD_CHECKBOX(	STR(StrID_SatCB_Param_Name),
						STR(StrID_SatCB_Param_Name),
						false,
						NULL,
						SATCB_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// sat slider
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_SatSlider_Param_Name),
							0,
							1,
							0,
							1,
							0.3,
							PF_Precision_THOUSANDTHS,
							0,
							0,
							SATTOL_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// color picker
	PF_ADD_COLOR(	STR(StrID_Color_Param_Name), 
					PF_HALF_CHAN8,
					PF_MAX_CHAN8,
					PF_MAX_CHAN8,
					COLOR_DISK_ID);
	AEFX_CLR_STRUCT(def);
	// mask only cb
	PF_ADD_CHECKBOX(	STR(StrID_MaskCB_Param_Name),
						STR(StrID_MaskCB_Param_Name),
						false,
						NULL,
						MASKCB_DISK_ID);
	AEFX_CLR_STRUCT(def);

	out_data->num_params = PIXELRAIN_NUM_PARAMS;

	return err;
}


/* tested & done
HSL FROM COLOR - source "http://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/" 14. aug 2017 12:59
param inP: pixel in rgb format
returns ColorHSL
*/
static ColorHSL*
hslFromColor(
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
	/* 4. saturation */
	double satF = 0.0;
	double hueF = 0.0;
	if (abs(minF - maxF) > EPSILON) { // saturation is not 0
		if (lumF < 0.5) { // dark
			satF = (maxF - minF) / (maxF + minF);
		}
		else { // light
			satF = (maxF - minF) / (2.0 - maxF - minF);
		}
	}
	/* 5. hue */
	if (maxF == rF) {
		hueF = (gF - bF) / (maxF - minF);
	}		  
	else if (maxF == gF) {
		hueF = 2.0 + (bF - rF) / (maxF - minF);
	}		  
	else if (maxF == bF) {
		hueF = 4.0 + (rF - gF) / (maxF - minF);
	}
	// hue must be positive
	if (hueF < 0.0) hueF += 6;
	hueF /= 6.0; // from sixths to percentage

	/* 6. return ColorHSL */
	//ColorHSL hsl;
	//AEFX_CLR_STRUCT(hsl);
	//ColorHSL *hslP = &hsl;
	ColorHSL *hslP = new ColorHSL();
	hslP->hue = hueF;
	hslP->luma = lumF;
	hslP->saturation = satF;
	return hslP;
}


/* tested & done
GENERATE MASK
pixel function, checks whether a pixel's hue, luma and  saturation are similiar enough to target color
*/
static PF_Err
GenerateMaskPixFunc(
	void		*refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel8	*inP,
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;
	PixSelInfo *psiP = (PixSelInfo*)refcon;
	PF_Pixel8 *tgtColorP = &(psiP->tgtColor);

	outP->alpha = inP->alpha;
	outP->red = inP->red;
	outP->green = inP->green;
	outP->blue = inP->blue;

	ColorHSL *tgtHSL = hslFromColor(tgtColorP);
	ColorHSL *inHSL = hslFromColor(inP);

	A_u_char maskAlpha = psiP->showMask ? 255 : 0;

	/* 1. Hue mask */
	if (psiP->hueCheck) {
		double maxHueDiff = psiP->hueTolerance / 2.0; // in UI diff of 1.0 covers all, in code 0.5
		double hueTgt = tgtHSL->hue;
		double hueIn = inHSL->hue;
		double hueDiff = 0.5 - abs(abs(hueTgt - hueIn) - 0.5); // hue wraps around
		if (hueDiff > maxHueDiff) {
			// alpha is mask, color tells which property masked
			outP->alpha = maskAlpha;
			outP->red = 255; // red for hue mask
		}
	}

	/* 2. Luma mask */
	if (psiP->lumaCheck) {
		double maxLumaDiff = psiP->lumaTolerance; // in UI diff of 1.0 covers all
		double lumaTgt = tgtHSL->luma;
		double lumaIn = inHSL->luma;
		double lumaDiff = abs(lumaTgt - lumaIn); // luma and sat don't wrap
		if (lumaDiff > maxLumaDiff) {
			outP->alpha = maskAlpha;
			outP->green = 255; // green for luma mask
		}
	}

	/* 3. Sat mask */
	if (psiP->satCheck) {
		double maxSatDiff = psiP->satTolerance; // in UI diff of 1.0 covers all
		double satTgt = tgtHSL->saturation;
		double satIn = inHSL->saturation;
		double satDiff = abs(satTgt - satIn); // luma and sat don't wrap
		if (satDiff > maxSatDiff) {
			outP->alpha = maskAlpha;
			outP->blue = 255; // blue for sat mask
		}
	}

	/* 4. free memmory */ // !TODO na vse aizhodne toèke daj delete!!!
	delete tgtHSL;
	delete inHSL;

	return err;
}


/* tested & done
GENERATE TRAILS COLUMN FUNCTION
called for each column to drop trail
*/
static PF_Err
GenerateTrailColFunc8(
	void *refcon, // trailInfoP
	A_long thread_indexL, // thread index, for abort and prog report
	A_long i, // OUTER LOOP current iteration
	A_long iterationsL)  // OUTER LOOP num of iterations
{
	PF_Err err = PF_Err_NONE;
	TrailInfo *tilP = (TrailInfo*)refcon;
	PF_InData *in_data = tilP->in_data;
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	A_long columnI = i;
	A_long outWidth = tilP->output->width;
	A_long outHeight = tilP->output->height;
	A_long rowbytes = tilP->output->rowbytes;

	PF_Pixel8 *inP = NULL;
	PF_Pixel8 *maskP = NULL;
	PF_Pixel8 *outP = NULL;
	PF_GET_PIXEL_DATA8(reinterpret_cast<PF_EffectWorld*>(tilP->input), NULL, &inP); 
	PF_GET_PIXEL_DATA8(reinterpret_cast<PF_EffectWorld*>(tilP->pixelMask), NULL, &maskP);
	PF_GET_PIXEL_DATA8(reinterpret_cast<PF_EffectWorld*>(tilP->output), NULL, &outP);
	PF_Pixel8 sampledColor;
	AEFX_CLR_STRUCT(sampledColor);
	bool mustSample = true;

	// jump to chosen column
	inP += columnI;
	maskP += columnI;
	outP += columnI;

	int lastRowNum = 0;

	// iterate through column
	// sample -> drip method
	for (int j = 0; j < outHeight; j++) {
		/* this is test of iterator coorrectness
		if (j % 5 == 0) {
			outP->alpha = 255;
			outP->red = 255;
			outP->green = 0;
			outP->blue = 0;
		}
		else {
			outP->alpha = 255;
			outP->red = 0;
			outP->green = 128;
			outP->blue = 128;
		}
		if (columnI == 0) {
			outP->alpha = 255;
			outP->red = 255;
			outP->green = 255;
			outP->blue = 255;
		}*/
		// sample
		if (maskP->alpha > 128) {
			mustSample = false;
			sampledColor.red = inP->red;
			sampledColor.green = inP->green;
			sampledColor.blue = inP->blue;
			lastRowNum = j;
			// empty out pix - AE doesn't always give us empty frames
			outP->alpha = 0;
		}
		// drip
		else if (j - lastRowNum < tilP->lengthF) {
			if (!mustSample) { // don't drip when there's nothing to drip
				outP->alpha = 255;
				outP->red = sampledColor.red;
				outP->green = sampledColor.green;
				outP->blue = sampledColor.blue;
			}
			else { // rather drip nothing just in case
				outP->alpha = 0;
				outP->red = 255;
				outP->green = 255;
				outP->blue = 255;
			}
		}
		// yes this is the last branch - empty pixels under trails
		else {
			outP->alpha = 0;
		}
		// next row
		inP += tilP->input->rowbytes / sizeof(PF_Pixel8);
		outP += tilP->output->rowbytes / sizeof(PF_Pixel8);
		maskP += tilP->pixelMask->rowbytes / sizeof(PF_Pixel8);
	}
	return err;
}


/* tested & done
RENDERS A FRAME
AE callback
*/
static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{

	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	A_long linesL = output->extent_hint.bottom - output->extent_hint.top;
	
	PixSelInfo psi;
	AEFX_CLR_STRUCT(psi);
	psi.hueCheck = params[PIXELRAIN_HUE_CB]->u.bd.value;
	psi.hueTolerance = params[PIXELRAIN_HUE_TOLERANCE]->u.fs_d.value;
	psi.lumaCheck = params[PIXELRAIN_LUMA_CB]->u.bd.value;
	psi.lumaTolerance = params[PIXELRAIN_LUMA_TOLERANCE]->u.fs_d.value;
	psi.satCheck = params[PIXELRAIN_SAT_CB]->u.bd.value;
	psi.satTolerance = params[PIXELRAIN_SAT_TOLERANCE]->u.fs_d.value;
	psi.tgtColor = params[PIXELRAIN_COLOR]->u.cd.value;

	/* 1. Generate pixel mask */
	PF_Handle colorMaskH = suites.HandleSuite1()->host_new_handle(sizeof(PF_EffectWorld));
	PF_EffectWorld * colorMaskP = (PF_EffectWorld*)*colorMaskH;
	//PF_EffectWorld * colorMaskP = new PF_EffectWorld(); 
	ERR(suites.WorldSuite1()->new_world(NULL,
		output->width,
		output->height,
		NULL,
		colorMaskP));
	if (err != PF_Err_NONE) return err;
	
	// deep world not supported - NYI
	if (PF_WORLD_IS_DEEP(output)) { // for 16bpp colors 
		err = PF_Err_INTERNAL_STRUCT_DAMAGED;
		// not supported
		return err;
	}
	else {
		ERR(suites.Iterate8Suite1()->iterate(in_data,
			0,								// progress base
			linesL,							// progress final
			&params[PIXELRAIN_INPUT]->u.ld,	// src 
			NULL,							// area - null for all pixels
			(void*)&psi,					// refcon - your custom data pointer
			GenerateMaskPixFunc,			// pixel function pointer
			colorMaskP));
	}
	if (err != PF_Err_NONE) return err;

	// show mask
	if ((PF_Boolean)params[PIXELRAIN_SHOW_MASK]->u.bd.value) {
		suites.WorldTransformSuite1()->copy(NULL,
			colorMaskP,
			output,
			NULL,
			NULL);
		return err;
	}

	/* 2. Generate trails */
	TrailInfo til;
	AEFX_CLR_STRUCT(til);
	TrailInfo *tilP = &til;
	tilP->lengthF = params[PIXELRAIN_LENGTH]->u.fs_d.value;
	tilP->in_data = in_data;
	tilP->input = &params[PIXELRAIN_INPUT]->u.ld;
	tilP->pixelMask = reinterpret_cast<PF_LayerDef*>(colorMaskP);

	PF_Handle trailsWorldH = suites.HandleSuite1()->host_new_handle(sizeof(PF_EffectWorld));
	PF_EffectWorld * trailsWorldP = (PF_EffectWorld*)*trailsWorldH;
	ERR(suites.WorldSuite1()->new_world(
		NULL,
		output->width,
		output->height,
		NULL,
		trailsWorldP
	));
	if (err != PF_Err_NONE) return err;

	tilP->output = reinterpret_cast<PF_LayerDef*>(trailsWorldP);

	ERR(suites.Iterate8Suite1()->iterate_generic(
		output->width,
		(void*)tilP,
		GenerateTrailColFunc8
	));
	if (err != PF_Err_NONE) return err;
	
	// show trails
	ERR(suites.WorldTransformSuite1()->copy(
		NULL,
		tilP->output,
		output,
		NULL,
		NULL
	));

	/* Free memory */
	suites.HandleSuite1()->host_dispose_handle(colorMaskH);
	suites.HandleSuite1()->host_dispose_handle(trailsWorldH);
	
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

