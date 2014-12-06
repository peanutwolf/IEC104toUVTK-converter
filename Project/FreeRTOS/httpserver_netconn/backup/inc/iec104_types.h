/*
 * Copyright (C) 2005 by Grigoriy A. Sitkarev                            
 * sitkarev@komi.tgk-9.ru                                                  
 *                                                                         
 * This program is free software; you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation; either version 2 of the License, or     
 * (at your option) any later version.                                   
 *                                                                        
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                         
 * You should have received a copy of the GNU General Public License     
 * along with this program; if not, write to the                         
 * Free Software Foundation, Inc.,                                       
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             
 */

#ifndef __IEC104_TYPES_H
#define __IEC104_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stm32f4xx.h"
#include "IEC104_server.h"
 
#define M_SP_NA_1	1
#define M_BO_NA_1	7
#define M_ME_NA_1	9
#define M_ME_NB_1	11
#define M_ME_NC_1	13
#define M_SP_TB_1	30
#define M_BO_TB_1	33
#define M_ME_TD_1	34
#define M_ME_TE_1	35
#define M_ME_TF_1	36
#define C_IC_NA_1 100
#define C_CS_NA_1 103
	
#define ACT_COT 6
#define ACTCON_COT 7
#define INROGEN_COT 20	

/* 32-bit string state and change data unit */
struct iec_stcd {
	u16_t		st[0];
	u8_t		st1	:1;
	u8_t		st2	:1;
	u8_t		st3	:1;
	u8_t		st4	:1;
	u8_t		st5	:1;
	u8_t		st6	:1;
	u8_t		st7	:1;
	u8_t		st8	:1;
	u8_t		st9	:1;
	u8_t		st10	:1;
	u8_t		st11	:1;
	u8_t		st12	:1;
	u8_t		st13	:1;
	u8_t		st14	:1;
	u8_t		st15	:1;
	u8_t		st16	:1;
	u16_t		cd[0];
	u8_t		cd1	:1;
	u8_t		cd2	:1;
	u8_t		cd3	:1;
	u8_t		cd4	:1;
	u8_t		cd5	:1;
	u8_t		cd6	:1;
	u8_t		cd7	:1;
	u8_t		cd8	:1;
	u8_t		cd9	:1;
	u8_t		cd10	:1;
	u8_t		cd11	:1;
	u8_t		cd12	:1;
	u8_t		cd13	:1;
	u8_t		cd14	:1;
	u8_t		cd15	:1;
	u8_t		cd16	:1;
}__attribute__((__packed__));

/* CP56Time2a timestamp */
typedef struct cp56time2a {
	u16_t		msec;
	u8_t		min	:6;
	u8_t		res1	:1;
	u8_t		iv	:1;
	u8_t		hour	:5;
	u8_t		res2	:2;
	u8_t		su	:1;
	u8_t		mday	:5;
	u8_t		wday	:3;
	u8_t		month	:4;
	u8_t		res3	:4;
	u8_t		year	:7;
	u8_t		res4	:1;	
} cp56time2a;

/* M_SP_NA_1 - single point information with quality description */
struct iec_type1 {
	u8_t		sp	:1; /* single point information */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
} __attribute__((__packed__));

/* M_BO_NA_1 - state and change information bit string */
struct iec_type7 {
	struct iec_stcd	stcd;
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* M_ME_NA_1 - normalized measured value */
struct iec_type9 {
	u16_t		mv;	/* normalized value */
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* M_ME_NB_1 - scaled measured value */
struct iec_type11 {
	u16_t		mv;	/* scaled value */
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* M_ME_NC_1 - short floating point measured value */
struct iec_type13 {
	float		mv;
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* M_SP_TB_1 - single point information with quality description and time tag */
struct iec_type30 {
	u8_t		sp	:1; /* single point information */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
	cp56time2a	time;
} __attribute__((__packed__));

/* M_BO_TB_1 - state and change information bit string and time tag  */
struct iec_type33 {
	struct iec_stcd	stcd;
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
	cp56time2a	time;
}__attribute__((__packed__));

/* M_ME_TD_1 - scaled measured value with time tag */
struct iec_type34 {
	u16_t		mv;	/* scaled value */
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
	cp56time2a	time;
}__attribute__((__packed__));

/* M_ME_TE_1 - scaled measured value with time tag*/
struct iec_type35 {
	u16_t		mv;	/* scaled value */
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
	cp56time2a	time;
}__attribute__((__packed__));

/* M_ME_TF_1 - short floating point measurement value and time tag */
struct iec_type36 {
	float		mv;
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
	cp56time2a	time;
}__attribute__((__packed__));

/* Quality description byte */
struct iec_qds {
	u8_t		ov	:1; /* overflow/no overflow */
	u8_t		res	:3;
	u8_t		bl	:1; /* blocked/not blocked */
	u8_t		sb	:1; /* substituted/not substituted */
	u8_t		nt	:1; /* not topical/topical */
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* Structure with normalized value invalid/valid flag */
struct iec_mv {
	u16_t mv;
	u8_t		iv	:1; /* valid/invalid */
}__attribute__((__packed__));

/* Data unit identifier block - ASDU header */
struct iec_unit_id {
	u8_t		type;	/* type identification */
	u8_t		num	:7; /* number of information objects */
	u8_t		sq	:1; /* sequenced/not sequenced address */
	u8_t		cause	:6; /* cause of transmission */
	u8_t		t	:1; /* test */
	u8_t		pn	:1; /* positive/negative app. confirmation */
	u8_t    oa; /* Originator adress */
	u16_t		ca;	/* common address of ASDU */
}__attribute__((__packed__));

/* Information object */
struct iec_object {
	u16_t		ioa;	/* information object address */
	union {
		struct iec_type1	type1;
		struct iec_type7	type7;
		struct iec_type9	type9;
		struct iec_type11	type11;
		struct iec_type13	type13;
		struct iec_type30	type30;
		struct iec_type33	type33;
		struct iec_type34	type34;
		struct iec_type35	type35;
		struct iec_type36	type36;
	} o;	
};

#ifdef __cplusplus
}
#endif

#endif	/* __IEC104_TYPES_H */
