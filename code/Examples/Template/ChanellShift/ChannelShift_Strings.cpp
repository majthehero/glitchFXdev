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

#include "ChannelShift.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"ChannelShift",
	StrID_Description,				"Split image into channels and move them around to create digital distortion effects.",
	
	// per channel horizontal shift
	StrID_R_X_Param_Name,			"Red channel horizontal shift",
	StrID_G_X_Param_Name,			"Green channel horizontal shift",
	StrID_B_X_Param_Name,			"Blue channel horizontal shift",
	
	StrID_H_X_Param_Name,			"Hue channel horizontal shift",
	StrID_S_X_Param_Name,			"Saturation channel horizontal shift",
	StrID_L_X_Param_Name,			"Luma channel horizontal shift",

	StrID_Y_X_Param_Name,			"Luma channel horizontal shift",
	StrID_U_X_Param_Name,			"U-chroma channel horizontal shift",
	StrID_V_X_Param_Name,			"V-chroma channel horizontal shift",
	// per channel vertical shift
	StrID_R_Y_Param_Name,			"Red channel vertical shift",
	StrID_G_Y_Param_Name,			"Green channel vertical shift",
	StrID_B_Y_Param_Name,			"Blue channel vertical shift",

	StrID_H_Y_Param_Name,			"Hue channel vertical shift",
	StrID_S_Y_Param_Name,			"Saturation channel vertical shift",
	StrID_L_Y_Param_Name,			"Luma channel vertical shift",

	StrID_Y_Y_Param_Name,			"Luma channel vertical shift",
	StrID_U_Y_Param_Name,			"U-chroma channel vertical shift",
	StrID_V_Y_Param_Name,			"V-chroma channel vertical shift",
	//
	StrID_Mode_Param_Name,			"Colorspace mode",
	// mode names
	StrID_Mode_Names,				"RGB| "
									"HSL| "
									"YUV| ",
	
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	