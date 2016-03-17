/*******************************************************************************
** File: vc0706_mux.h
**
** Purpose:
**   This file is main hdr file for the VC0706 application.
**
**
*******************************************************************************/

#ifndef	_vc0706_mux_h_
#define _vc0706_mux_h_


typedef struct { // Not sure how much should be implimented in this struct
	int mux_select_pin; // Expects GPIO for mux select line
	int mux_state; 		// 0 or 1 
} mux_t;


void mux_init(mux_t *mux, int select_pin);

int mux_select(mux_t *mux, int select); // sanatized selection.


#endif /* _vc0706_mux_h_ */