
/* !TODO add luma and sat masking -- best refactor rewrite nicely
GENERATE HUE MASK
pixel function, checks whether a pixel's hue is similiar enough to target color
*/
static PF_Err
GenerateMaskPixFunc (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;
	PixSelInfo *psiP = (PixSelInfo*)refcon;
	PF_Pixel8 *tgtColorP = &(psiP->tgtColor);
	
	double maxHueDiff = psiP->hueTolerance / 2.0;

	/* USING DISTANCE IN ALTERED RGB SPACE NOT GOOD *
	// using euclidean variance, altered for human perception
	/*float deltaR, deltaG, deltaB;
	deltaR = (float)(inP->red - tgtColor.red);
	deltaG = (float)(inP->green - tgtColor.green);
	deltaB = (float)(inP->blue - tgtColor.blue);
	float eucVarAlt = EUC_VAR_ALT(deltaR, deltaG, deltaB);
	float maxVar = EUC_VAR_ALT(255, 255, 255);*/
	
	/* mask based on hue similiarity */
	ColorHSL *tgtHSL = hslFromColor(tgtColorP);
	ColorHSL *inHSL = hslFromColor(inP);
	double hueTgt = tgtHSL->hue;
	double hueIn = inHSL->hue;
	//double hueDiff = hueTgt - hueIn;
	double hueDiff = 0.5 - abs(abs(hueTgt - hueIn) - 0.5);
	/* 1bit mask */
	if (hueDiff > maxHueDiff) {
		outP->alpha = 0;
		outP->red = 255;
		outP->green = 0;
		outP->blue = 255;
	}
	else {
		outP->alpha = 255;
		outP->red = inP->red;
		outP->green = inP->green;
		outP->blue = inP->blue;
	}
	//// basic boolean blending
	//if ((float)outP->alpha / 255.0 > (psiP->diff)) {
	//	outP->alpha = 255;
	//}
	//else {
	//	outP->alpha = 0;
	//}

	return err;
}
