// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __VSE_GEN_ASM_H__
#define __VSE_GEN_ASM_H__

// CodeBase IDs
.CONST $M.VSE_VSE_CAP_ID       	0x00004A;
.CONST $M.VSE_VSE_ALT_CAP_ID_0       	0x406A;
.CONST $M.VSE_VSE_SAMPLE_RATE       	0;
.CONST $M.VSE_VSE_VERSION_MAJOR       	1;

// Constant Values


// Piecewise Disables
.CONST $M.VSE.CONFIG.BYPASS		0x00000001;


// Statistic Block
.CONST $M.VSE.STATUS.CUR_MODE         		0*ADDR_PER_WORD;
.CONST $M.VSE.STATUS.OVR_CONTROL      		1*ADDR_PER_WORD;
.CONST $M.VSE.STATUS.COMPILED_CONFIG  		2*ADDR_PER_WORD;
.CONST $M.VSE.STATUS.OP_STATE         		3*ADDR_PER_WORD;
.CONST $M.VSE.STATUS.OP_INTERNAL_STATE		4*ADDR_PER_WORD;
.CONST $M.VSE.STATUS.BLOCK_SIZE            	5;

// System Mode
.CONST $M.VSE.SYSMODE.STATIC   		0;
.CONST $M.VSE.SYSMODE.MUTE     		1;
.CONST $M.VSE.SYSMODE.FULL     		2;
.CONST $M.VSE.SYSMODE.PASS_THRU		3;
.CONST $M.VSE.SYSMODE.MAX_MODES		4;

// System Control
.CONST $M.VSE.CONTROL.MODE_OVERRIDE		0x2000;

// Parameter Block
.CONST $M.VSE.PARAMETERS.OFFSET_VSE_CONFIG     		0*ADDR_PER_WORD;
.CONST $M.VSE.PARAMETERS.OFFSET_BINAURAL_FLAG  		1*ADDR_PER_WORD;
.CONST $M.VSE.PARAMETERS.OFFSET_SPEAKER_SPACING		2*ADDR_PER_WORD;
.CONST $M.VSE.PARAMETERS.OFFSET_VIRTUAL_ANGLE  		3*ADDR_PER_WORD;
.CONST $M.VSE.PARAMETERS.STRUCT_SIZE          		4;


#endif // __VSE_GEN_ASM_H__