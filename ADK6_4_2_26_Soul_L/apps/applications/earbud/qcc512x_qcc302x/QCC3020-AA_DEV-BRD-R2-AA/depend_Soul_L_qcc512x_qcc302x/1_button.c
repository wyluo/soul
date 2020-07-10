#include "input_event_manager.h"

#include "1_button.h"

const InputEventConfig_t InputEventConfig  = 
{
	/* Table to convert from PIO to input event ID*/
	{
		 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	},

	/* Masks for each PIO bank to configure as inputs */
	{ 0x00000001UL, 0x00000000UL, 0x00000000UL },
	/* PIO debounce settings */
	4, 5
};

const InputActionMessage_t InputEventActions[] = 
{
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		5500,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_5_SECOND,                /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		10000,                                  /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_10_SECOND,               /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		3000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_3_SECOND,                /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		15000,                                  /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_15_SECOND,               /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		8000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_8_SECOND,                /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD,                                   /* Action */
		1000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_1_SECOND,                /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		10000,                                  /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_10_SECOND_REL,           /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		1000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_1_SECOND_REL,            /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		8000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_8_SECOND_REL,            /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		5500,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_5_SECOND_REL,            /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		15000,                                  /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_15_SECOND_REL,           /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		HELD_RELEASE,                           /* Action */
		3000,                                   /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_3_SECOND_REL,            /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		DOUBLE,                                 /* Action */
		400,                                    /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_ANC_TOGGLE_ON_OFF,       /* Message */
	},
	{
		MFB_BUTTON,                             /* Input event bits */
		MFB_BUTTON,                             /* Input event mask */
		RELEASE,                                /* Action */
		0,                                      /* Timeout */
		0,                                      /* Repeat */
		APP_MFB_BUTTON_PRESS,                   /* Message */
	},
};


