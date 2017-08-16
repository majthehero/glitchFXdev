/* !TODO test and check
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
	
	outP->alpha = inP->alpha;
	outP->red 	= inP->red;
	outP->green = inP->green;
	outP->blue 	= inP->blue;
		
	ColorHSL *tgtHSL = hslFromColor(tgtColorP);
	ColorHSL *inHSL = hslFromColor(inP);
	
	/* 1. Hue mask */
	if (psiP->hueCheck) {
		double maxHueDiff = psiP->hueTolerance / 2.0; // in UI diff of 1.0 covers all
		double hueTgt = tgtHSL->hue;
		double hueIn = inHSL->hue;
		double hueDiff = 0.5 - abs(abs(hueTgt - hueIn) - 0.5); // hue wraps around
		// shrink mask
		if (hueDiff > maxHueDiff) {
			// alpha is mask, color tells which property masked
			outP->alpha = 0;
			outP->red = 255; // red for hue mask
		}
	}
	
	/* 2. Luma mask */
	if (psiP->lumaCheck) {
		double maxLumaDiff = psiP->lumaTolerance / 2.0; // in UI diff of 1.0 covers all
		double lumaTgt = tgtHSL->luma;
		double lumaIn = inHSL->luma;
		double lumaDiff = abs(lumaTgt - lumaIn); // luma and sat don't wrap
		// shrink mask
		if (lumaDiff > maxLumaDiff) {
			outP->alpha = 0;
			outP->green = 255; // green for luma mask
		}
	}
	
	/* 3. Sat mask */
	if (psiP->satCheck) {
		double maxSatDiff = psiP->satTolerance / 2.0; // in UI diff of 1.0 covers all
		double satTgt = tgtHSL->saturation;
		double satIn = inHSL->saturation;
		double satDiff = abs(lumaTgt - lumaIn); // luma and sat don't wrap
		// shrink mask
		if (satDiff > maxSatDiff) {
			outP->alpha = 0;
			outP->blue = 255; // blue for sat mask
		}
	}
	
	return err;
}