/*******************************************************************************
** File: vc0706_mux.c
**
** Purpose:
**   This file is main hdr file for the VC0706 application.
**
**
*******************************************************************************/


void mux_init(mux_t *mux, int select_pin) {
	mux->mux_select_pin = select_pin; 
	pinMode(select_pin, OUTPUT); 
	mux_select(mux, 0); // initialize low 
}

int mux_select(mux_t *mux, int select) { 
	if(select == 1)
	{
		digitalWrite(mux->mux_select_pin, HIGH);
		mux->mu_state = 1;
		return 0;
	}
	else if(select == 0)
	{
		digitalWrite(mux->mux_select_pin, LOW);
		mux->mu_state = 0;
		return 0;
	}
	else
	{
		return -1;
	}
}