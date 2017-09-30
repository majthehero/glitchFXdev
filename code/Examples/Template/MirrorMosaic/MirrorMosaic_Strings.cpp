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

#include "MirrorMosaic.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"MirrorMosaic",
	StrID_Description,				"Mirrors a part of video over a grid with a time delay.",
	StrID_GridX_Param_Name,			"Grid X resolution",
	StrID_GridY_Param_Name,			"Grid Y resolution",
	//StrID_MirrorHor_Param_Name,		"Mirror horizontally",
	//StrID_MirrorVert_Param_Name,		"Mirror vertically",
	StrID_MirrorDir_Param_Name,		"Mirroring direction",
	StrID_DeltaT_Param_Name,		"Time delay in frames",
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	