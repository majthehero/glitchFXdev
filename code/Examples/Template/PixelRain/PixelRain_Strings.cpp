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

#include "PixelRain.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"PixelRain",
	StrID_Description,				"An empty (skeletal, if you will) effect sample,\r for your modifying pleasure.\rCopyright 2007 Adobe Systems Incorporated.",
	StrID_Length_Param_Name,			"Length",
	StrID_Diff_Param_Name,			"Color difference",
	StrID_Color_Param_Name,			"Color",
	StrID_AdvAlpha_Param_Name,			"Advanced alpha blending",
	StrID_DBG_OPTYPE_Param_Name,			"1 - pixelMask; 2 - trails; 3 - blended",
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	