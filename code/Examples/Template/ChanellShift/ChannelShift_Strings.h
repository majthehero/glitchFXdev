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

#pragma once

typedef enum {
	StrID_NONE, 
	StrID_Name,
	StrID_Description,
	// per channel horizontal shift
	StrID_R_X_Param_Name,
	StrID_G_X_Param_Name,
	StrID_B_X_Param_Name,

	StrID_H_X_Param_Name,
	StrID_S_X_Param_Name,
	StrID_L_X_Param_Name,

	StrID_Y_X_Param_Name,
	StrID_U_X_Param_Name,
	StrID_V_X_Param_Name,
	// per channel vertical shift
	StrID_R_Y_Param_Name,
	StrID_G_Y_Param_Name,
	StrID_B_Y_Param_Name,

	StrID_H_Y_Param_Name,
	StrID_S_Y_Param_Name,
	StrID_L_Y_Param_Name,

	StrID_Y_Y_Param_Name,
	StrID_U_Y_Param_Name,
	StrID_V_Y_Param_Name,
	//
	StrID_Mode_Param_Name,
	StrID_NUMTYPES
} StrIDType;
