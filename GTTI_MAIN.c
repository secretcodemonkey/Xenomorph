/****************************************************************************************************/
/*                                                                                                  */
/*  X    X                                                                                          */
/*   X  X                                                                                           */
/*    XX ENOMORPH                                                                                   */
/*    XX ENOMORPH                                                                                   */
/*   X  X                                                                                           */
/*  X    X                    VERSION: GTTI                                                         */
/*                                                                                                  */
/****************************************************************************************************/
/*                                                                                                  */
/* (c) 2011-2012 R.J.Barnes (Codemonkey). All rights reserved                                       */  
/*                                                                                                  */
/* NOTE For upload:                                                                                 */
/* In Silicon Labs IDE, Tools/Upload Memory to file                                                 */
/* upload the CODE segment from address 1000 hex to F9FF hex                                        */
/* Send the output to a suitably named .txt file, and insert letter 'X' in front of the first line  */
/* so  '02'  becomes  'X02'                                                                         */ 
/*                                                                                                  */
/****************************************************************************************************/

#include "compiler_defs.h"
#include "C8051f380_defs.h"
#include "GTTI_MAIN.h"

/* Software Identification **************************************************************************/
#define REVNUMLENGTH		20
#define SIGLENGTH			32

/*                                                    0123456789012345678901234567890123456789*/
const char __at 0xF990    XenoRevNum[REVNUMLENGTH] = "MSII Xenomorph 0.61";
const char __at 0xF9C0    XenoSign[SIGLENGTH]      = "XenomorphGTti 0.61 Codemonkey  ";
/* Software Identification **************************************************************************/



//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void Timer_Init(void);                  
void Adc_Init(void);
void Port_Init(void);
void Suspend_Device(void);
void Initialize(void);
void commshandler(void);
void TASK_10ms (void);						/* 10ms task container 			*/
void IOP_Convert10ms (void); 				/* HAL to IOP Conversion 		*/
void IGN_StallCheckOpMode10ms (void); 		/* Stall check/operating mode 	*/
void FUEL_Fuelling10ms (void);           	/* Fuelling calculations 		*/
void IGN_Ignition10ms (void);               /* Ignition calculations 		*/
void VS_GearRatioDet10ms(void); 			/* Gear ratio detection 		*/
void TURBO_BoostLimit10ms(void); 			/* Handle boost limit/restart   */
void VS_ClutchHandle10ms(void); 			/* Handle clutch controlled func*/
void IOP_RPMLimiter10ms(void);              /* Handler RPM limiter function */
void TURBO_BoostControl10ms(void);			/* Boost closed loop control 	*/
void IOP_Aux10ms(void);                     /* Handle auxilliary functions  */
void IOP_FakeMAP10ms(void);					/* Calculate DAC fake MAP input */
void IOP_UpdatePCData10ms(void);  			/* Byte switch for monitor      */
void TASK_100ms (void);						/* 100ms task container         */
void IOP_Convert100ms (void);				/* HAL to IOP Conversion 		*/
void IOP_Housekeeping100ms (void);			/* 100ms housekeeping functions */
void IOP_BurnCheckBL(void);					/* Background burn check        */
UINT16 switchu16(UINT16 swin);				/* Unsigned 16 switch utility   */
SINT16 switchs16(SINT16 swin);              /* Signed 16 switch utility     */


#pragma nooverlay                           /* No register overlay allowed  */

/***********************************************************************************/
/*                                                                                 */
/* main() - Start of C code function execution                                     */
/*                                                                                 */
/***********************************************************************************/
void main(void)
{


	UINT16 copycnt;
	UINT16 * sourceptr;
	UINT16 * destptr;
    UINT16 calc16;
	UINT8 tableindex;

	PCA0MD &= ~0x40;                       			/* Disable watchdog 	 */

	FLSCL     = 0x90;

	CLKSEL    = 0x03;

	OSCICN    = 0xC3;

	Initialize();									/* Initialse the core    */

	Vars.HAL_Burn = 0; /* Ensure that no burn request is made without new session */

	/* Determine the active ROM set */

	if (*(UINT16 __code *)ROMPAGE1VALID == ROMVALIDMARK)
		{
			ROM1Num = *(UINT16 __code *)ROMPAGE1COUNT;
		}
	else
		{
			ROM1Num = 0;
		}

	if (*(UINT16 __code *)ROMPAGE2VALID == ROMVALIDMARK)
		{
		    ROM2Num = *(UINT16 __code *)ROMPAGE2COUNT;
		}
	else
		{
			ROM2Num = 0;
		}

	if (ROM1Num > ROM2Num)
		{
														/* Copy the active parameter sets before use */
			for (tableindex = 0; tableindex < NO_PARTBLES; tableindex++)
				{
					/* Copy Flash to RAM parameters */
					for (copycnt = 0,sourceptr = Partables1[tableindex].addrFlash,destptr = Partables[tableindex].addrRam;copycnt < (Partables[tableindex].n_bytes / 2);copycnt++)
						{
							*destptr++ = *sourceptr++;
						}
				}

            ROMActivePage = ROM1Num;

			Boost2RAM.PageCounter = ROMActivePage + 1;

		}
	else
		{
														/* Copy the active parameter sets before use */
			for (tableindex = 0; tableindex < NO_PARTBLES; tableindex++)
				{
					/* Copy Flash to RAM parameters */
					for (copycnt = 0,sourceptr = Partables2[tableindex].addrFlash,destptr = Partables[tableindex].addrRam;copycnt < (Partables[tableindex].n_bytes / 2);copycnt++)
						{
							*destptr++ = *sourceptr++;
						}
				}

            ROMActivePage = ROM2Num;

			Boost2RAM.PageCounter = ROMActivePage + 1;

		}

	Vars.HAL_Seconds = 0;							/* Reset the time since start count */

   /* Calculate one time only constants for operation of the code   */
   /* Calculate correction factor to use for re-sizing of injectors */
	calc16 = (((UINT16)GlobalsRAM.FUEL_STDINJSIZE * 100) / GlobalsRAM.FUEL_NEWINJSIZE);

	if (calc16 > 255)
		{
		   Vars.FUEL_CorFactSize = 255;
		}
	else
   		{
		   Vars.FUEL_CorFactSize = (UINT8)calc16;
		}

   /* Calculate RPM constant for engine speed measurement          */

	HAL_kEngineSpeed = FPCA0 * 60 * PPRENGINESPEEDDENOM / PPRENGINESPEEDNUM;

	HAL_EnginePeriod = 0xFFFF;

   /* Calculate RPM constant for vehicle speed measurement          */

//	HAL_kVehicleSpeed = FPCA0 * 60 / PPRVEHICLESPEED;
	HAL_kVehicleSpeed = FPCA0 * 14;   /* From hand calibration based on speedo against motor */

	HAL_VehiclePeriod = 0xFFFF;

	/* Pre-calculation - angle per cylinder */
	IGN_AnglePerCylinder = (UINT8)(720 / GlobalsRAM.IOP_NUMBEROFCYLS); 

    /* Calculate the ratio splits for gear ID */
#ifdef INVERT
	VS_GearRatio[0] = switchu16(GlobalsRAM.VS_RATIO1);
	VS_GearRatio[1] = switchu16(GlobalsRAM.VS_RATIO2);
	VS_GearRatio[2] = switchu16(GlobalsRAM.VS_RATIO3);
	VS_GearRatio[3] = switchu16(GlobalsRAM.VS_RATIO4);
	VS_GearRatio[4] = switchu16(GlobalsRAM.VS_RATIO5);
	VS_GearRatio[5] = switchu16(GlobalsRAM.VS_RATIO6);
	VS_GearRatio[6] = switchu16(GlobalsRAM.VS_RATIO7);
#else
	VS_GearRatio[0] = GlobalsRAM.VS_RATIO1;
	VS_GearRatio[1] = GlobalsRAM.VS_RATIO2;
	VS_GearRatio[2] = GlobalsRAM.VS_RATIO3;
	VS_GearRatio[3] = GlobalsRAM.VS_RATIO4;
	VS_GearRatio[4] = GlobalsRAM.VS_RATIO5;
	VS_GearRatio[5] = GlobalsRAM.VS_RATIO6;
	VS_GearRatio[6] = GlobalsRAM.VS_RATIO7;
#endif
    /* Load splits with average of two sequential ratios */
    for (tableindex = 0; tableindex < 6 ; tableindex++)
		{
		   VS_RatioSplits[tableindex] = VS_GearRatio[tableindex] + VS_GearRatio[tableindex + 1];
           VS_RatioSplits[tableindex] /= 20;
		}

    /* Calculate factor for vehicle speed to kph conversion */
    VS_KPHFact = 107;

	// tyre circumference (mm) = Pi.D = pi x ((rim * 25.4) + 2 x (width * ratio / 100)
	// kph = RPM * 60 * circumference (mm) / 1000 / 1000
	// 
//    rimmm = (GlobalsRAM.VS_TYREDIAMETER * 254) / 100;  /* x10 x 10 = x100 / 100 */
//	tworubber = ((UINT16)GlobalsRAM.VS_TYREWIDTH * GlobalsRAM.VS_TYREPROFILE) / 50;   /* mm x % / 100 x 2 = mm x 1*/
//    diameter = rimm + tworubber;
//	factor = diameter * 188;  // 60 x pi included


/* Initialise the operating mode to mirror mode - no engine reference at this time */
   	Vars.IOP_OpMode = MIRRORMODE;
   	HAL_FuelStart = 0xFFFF;    				/* Clear fuel start counter 		*/
   	HAL_FuelEnd = 0xFFFF;           		/* Clear fuel end counter           */
   	HAL_SparkStart = 0xFFFF;				/* Clear start spark time           */
   	HAL_SparkEnd = 0xFFFF;					/* Clear end spark time             */
   	HAL_IgnSync = 0;						/* Spark out of sync                */
   	Vars.FUEL_BaseTime = 0x0;				/* Zero base fuel time measurement  */
   	Vars.IGN_SparkHigh = 0xFFFF;    		/* Clear spark high time            */
   	Vars.IGN_SparkPeriod = 0xFFFF;  		/* Maximum spark period             */
	Vars.TURBO_MAPITerm = 0;                /* Reset the I Term    */
	Vars.TURBO_ClosedLoop = 0;              /* Indicate no control */
    Vars.TURBO_MAPTarget = 0;               /* Zero the data       */
    Vars.TURBO_MAPError  = 0;
	Vars.TURBO_MAPPTerm = 0;								
	Vars.TURBO_CutCounter = 0;
	Vars.TURBO_OkayCounter = 255;
    Vars.VS_EngLimClutch = 20000;	     	/* Push limit up            		*/
	Vars.HAL_Seconds = 0;
	Vars.IOP_CutBits = 0;
    Vars.DEBUG = 0;
	DIGIO1 = 0;
	Vars.IOP_CoolantTemp = 0;
	HAL_FuelCounter = 0;					/* Clear igns since fuel time       */



   /* Enable the timers and interrupts */

   	EIE1      = 0x98;

   	IP        = 0x08;			/* Highest priority interrupts are Timer 1......*/

   	EIP1      = 0x90;            /* ..Timer 3 and PCA            			    */

   	TR0 = 1;                     /* Timer 0 used for 500000Hz source - PCA0      */

//   	ET2 = 1;                     /* Timer 2 interrupt enable - 1.0ms counter RTI */

   	TR2 = 1;                     /* Timer 2 enable - 1.0ms counter RTI           */

   	IT01CF    = 0x89;			/*  */

   	EIE1      |= 0x10;			/* Enable external interrupt source (PCA0)      */

   	IE        = 0xA0;			/* Enable Timer 2 Only */
								
   	while (1)					/* Background loop (co-operative tasks)         */
	   	{
			/* Look for scheduled start of 10ms task */
			if (HAL_Task10msStart > 0)
				{
					TASK_10ms();	
		   		 	HAL_Task10msStart -= 1;
				}
			/* Look for scheduled start of 100ms task */
			if (HAL_Task100msStart > 0)
				{
					TASK_100ms();
					HAL_Task100msStart -= 1;
				}

		} /* End of main while statement */

}



/***********************************************************************************/
/*                                                                                 */
/* TASK_10ms() - Container for all 10ms routine calls                              */
/*                                                                                 */
/***********************************************************************************/
void TASK_10ms ()
{

     	IOP_Convert10ms (); 				/* HAL to IOP Conversion 		*/
     	IGN_StallCheckOpMode10ms (); 		/* Stall check/operating mode 	*/
		FUEL_Fuelling10ms ();           	/* Fuelling calculations 		*/
		IGN_Ignition10ms ();               	/* Ignition calculations 		*/
		VS_GearRatioDet10ms(); 				/* Gear ratio detection 		*/
		TURBO_BoostLimit10ms(); 			/* Handle boost limit/restart   */
		VS_ClutchHandle10ms(); 				/* Handle clutch controlled func*/
        IOP_RPMLimiter10ms();               /* Call RPM limiter for engine  */
		TURBO_BoostControl10ms();			/* Boost closed loop control 	*/
		IOP_FakeMAP10ms();					/* Calculate DAC fake MAP input */
		IOP_Aux10ms();                      /* Auxilliary functions         */
		IOP_UpdatePCData10ms();  			/* Byte switch for monitor      */

	return;
}



/***********************************************************************************/
/*                                                                                 */
/* TASK_100ms() - Container for all 100ms routine calls                            */
/*                                                                                 */
/***********************************************************************************/
void TASK_100ms (void)
{

		IOP_Convert100ms ();				/* HAL to IOP Conversion 		*/
		IOP_Housekeeping100ms ();			/* 100ms housekeeping functions */
		IOP_BurnCheckBL();					/* Background burn check        */

	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_Aux10ms() - Update auxilliary outputs/functions                             */
/*                                                                                 */
/***********************************************************************************/
void IOP_Aux10ms (void)		                       
	{

	/* Is temperature related drive required? */
	if ((GlobalsRAM.IOP_TEMPDRIVEON <= -128) || (GlobalsRAM.IOP_TEMPDRIVEOFF <= -128))
		{
			/* No, update with throttle */
			if (Vars.IOP_ThrottlePos >= GlobalsRAM.IOP_TPSDRIVEON)
				{
					Vars.IOP_Auxilliary |= IOP_DIGIO1;
				}
			else if (Vars.IOP_ThrottlePos <= GlobalsRAM.IOP_TPSDRIVEOFF)
				{
					Vars.IOP_Auxilliary &= ~IOP_DIGIO1;
				}
		}
	else /* Update with temperature */
		{
			if (Vars.IOP_CoolantTemp >= GlobalsRAM.IOP_TEMPDRIVEON)
				{
					Vars.IOP_Auxilliary |= IOP_DIGIO1;
				}
			else if (Vars.IOP_CoolantTemp <= GlobalsRAM.IOP_TEMPDRIVEOFF)
				{
					Vars.IOP_Auxilliary &= ~IOP_DIGIO1;
				}
		}

	if (Vars.IOP_Auxilliary & IOP_DIGIO1)
		{
			DIGIO1 = 1;
		}
	else
		{
			DIGIO1 = 0;
		}
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_Convert10ms() - Convert HAL to IOP level variables                          */
/*                                                                                 */
/***********************************************************************************/
void IOP_Convert10ms (void)		/* HAL to IOP Conversion 		*/
	{


		UINT16 local16;
		UINT8 dindex;

								/* Convert manifold pressure */
		  if (Vars.HAL_ManPressRaw <= GlobalsRAM.HAL_MAPMIN)
				{
					Vars.IOP_ManifoldPress = 0;
				}
		  else if (Vars.HAL_ManPressRaw >= GlobalsRAM.HAL_MAPMAX)
				{
					Vars.IOP_ManifoldPress = ((UINT16)GlobalsRAM.IOP_MAXMAPPRESS * 12);
				}
    	  else
				{
					Vars.IOP_ManifoldPress = (UINT16)(((UINT32)GlobalsRAM.IOP_MAXMAPPRESS * (Vars.HAL_ManPressRaw - GlobalsRAM.HAL_MAPMIN) * 12) / (GlobalsRAM.HAL_MAPMAX - GlobalsRAM.HAL_MAPMIN));
				}

								/* Get rate of change of MAP */
		  Vars.IOP_dManifold = (SINT16)Vars.IOP_ManifoldPress - IOP_MAPHist[0];

		  dindex = 0;
		  do
		  {
			IOP_MAPHist[dindex] = IOP_MAPHist[dindex + 1];
		  }
		  while (++dindex < (MAPHISTLEN - 1));

		  IOP_MAPHist[GlobalsRAM.TURBO_DMAPDELAY] = Vars.IOP_ManifoldPress;

								/* Convert throttle position */
		  if (Vars.HAL_ThrottleRaw <= GlobalsRAM.HAL_THROTTLEMIN)
				{
					Vars.IOP_ThrottlePos = 0;
				}
		  else if (Vars.HAL_ThrottleRaw >= GlobalsRAM.HAL_THROTTLEMAX)
				{
					Vars.IOP_ThrottlePos = 100;
				}
    	  else
				{
					Vars.IOP_ThrottlePos = ((UINT16)(Vars.HAL_ThrottleRaw - GlobalsRAM.HAL_THROTTLEMIN) * 100) / (GlobalsRAM.HAL_THROTTLEMAX - GlobalsRAM.HAL_THROTTLEMIN);
				}

								/* Convert 02 sensor 		*/
		  Vars.IOP_AirFuelRatio = HAL_O2Lookup(Vars.HAL_O2SensorRaw);


								/* Calculate engine speed	*/
		  if ((HAL_EnginePeriod != 0xFFFF) && (HAL_EnginePeriod != 0))
		  		{
				  	Vars.IOP_EngineSpeed = HAL_kEngineSpeed / HAL_EnginePeriod;

					local16 = HAL_EnginePeriod * GlobalsRAM.IOP_NUMBEROFCYLS;

					HAL_EnginePeriod720 = local16;
                }
		  else
		  		{
					Vars.IOP_EngineSpeed = 0;
					HAL_EnginePeriod720 = 0xFFFF;
				}

			/* Check for garbage values on FTT */
		  if (Vars.IOP_VehicleSpeed > HAL_VEHICLEMAXSPEED)
		  		{
					Vars.IOP_VehicleSpeed = 0;
				}
			
								/* Calculate vehicle speed	*/
		  if ((HAL_VehiclePeriod != 0xFFFF) && (HAL_VehiclePeriod > HAL_VEHICLEMINPERIOD) && (HAL_VehiclePeriod != 0))
		  		{
				  	IOP_Vehicle = HAL_kVehicleSpeed / HAL_VehiclePeriod;
                }
		  else
		  		{
					IOP_Vehicle = 0;
				}
								/* Calculate change in speed in the last 10ms */
		  IOP_VehicleDelta = (SINT16)IOP_Vehicle - Vars.IOP_VehicleSpeed;

		  if (ClutchType == CLUTCHLCLIMIT)
		  		{
								/* Rate limit output of vehicle speed */
				  if(IOP_VehicleDelta > GlobalsRAM.VS_VEHICLEMAXRISELC)
				  		{
							IOP_VehicleDelta = GlobalsRAM.VS_VEHICLEMAXRISELC;
						}
				  else if (IOP_VehicleDelta < GlobalsRAM.VS_VEHICLEMAXFALL)
						{
							IOP_VehicleDelta = GlobalsRAM.VS_VEHICLEMAXFALL;
						}
				}
		  else
		  		{
								/* Rate limit output of vehicle speed */
				  if(IOP_VehicleDelta > GlobalsRAM.VS_VEHICLEMAXRISE)
				  		{
							IOP_VehicleDelta = GlobalsRAM.VS_VEHICLEMAXRISE;
						}
				  else if (IOP_VehicleDelta < GlobalsRAM.VS_VEHICLEMAXFALL)
						{
							IOP_VehicleDelta = GlobalsRAM.VS_VEHICLEMAXFALL;
						}
				}
								/* Find vehicle speed after rate limit */
		  Vars.IOP_VehicleSpeed += IOP_VehicleDelta;
								/* Limit to a maximum                  */
		  if (Vars.IOP_VehicleSpeed >= HAL_VEHICLEMAXSPEED)
		  		{
					Vars.IOP_VehicleSpeed = HAL_VEHICLEMAXSPEED;
				}

								/* Read clutch switch input (close to clutch top) */
		  if (Clutch)
				{
					if (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHINVERTSW)
						{
							Vars.IOP_ClutchSwitch = 0;					
						}
					else
						{
							Vars.IOP_ClutchSwitch = 1;					
						}
				}
		  else
				{
					if (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHINVERTSW)
						{
							Vars.IOP_ClutchSwitch = 1;					
						}
					else
						{
							Vars.IOP_ClutchSwitch = 0;					
						}
				}
	return;
}


/***********************************************************************************/
/*                                                                                 */
/* IGN_StallCheckOpMode10ms() - Stall check/operating mode                         */
/*                                                                                 */
/***********************************************************************************/
 void IGN_StallCheckOpMode10ms (void) 		/* Stall check/operating mode 	*/
	{

	UINT8	EngLow;


		if (Vars.IOP_EngineSpeed < 16320)
			{
				EngLow = (UINT8)(Vars.IOP_EngineSpeed / 64);
			}
		else
			{
				EngLow = 0xFF;
			}

/* Engine monitoring/sync calculations */
		if (EngLow < GlobalsRAM.IOP_ENGINESTALLSPEED)
				{
					Vars.IOP_OpMode = ENGINESTOP; 	/* Prevent interventions    		*/
					HAL_FuelStart = 0xFFFF;    		/* Clear fuel start counter 		*/
					HAL_FuelEnd = 0xFFFF;           /* Clear fuel end counter           */
					HAL_SparkStart = 0xFFFF;		/* Clear start spark time           */
					HAL_SparkEnd = 0xFFFF;			/* Clear end spark time             */
					HAL_IgnSync = 0;				/* Spark out of sync                */
					Vars.FUEL_BaseTime = 0x0;		/* Zero base fuel time measurement  */
					Vars.IGN_SparkHigh = 0xFFFF;    /* Clear spark high time            */
					Vars.IGN_SparkPeriod = 0xFFFF;  /* Maximum spark period             */
					HAL_FuelCounter = 0;			/* Clear igns since fuel time       */
					Vars.TURBO_CutCounter = 0;
					Vars.TURBO_OkayCounter = 255;
				    Vars.VS_EngLimClutch = 20000;	/* Push limit up            		*/
					Vars.IOP_CutBits = 0;
				}
		else if (EngLow > GlobalsRAM.IOP_ENGINESYNCSPEED)
				{
					if (Vars.IOP_OpMode != GlobalsRAM.IOP_OPERATINGMODE)
						{
							if(Vars.FUEL_BaseTime != 0x0000)
								{
									Vars.IOP_OpMode = GlobalsRAM.IOP_OPERATINGMODE;
													/* Running with fuel - go to mode   */
								}		
							else
								{
									Vars.IOP_OpMode = MIRRORMODE; 	
													/* Prevent interventions    		*/
								}
						}
				}
	return;
}


/***********************************************************************************/
/*                                                                                 */
/* FUEL_Fuelling10ms() - Fuelling calculations                                     */
/*                                                                                 */
/***********************************************************************************/
void FUEL_Fuelling10ms (void)           	/* Fuelling calculations 		*/
	{

    UINT16 FUEL_CorFactMap;
	UINT16 FUEL_OverallTemp;
	UINT16 HAL_BaseScaled;
	UINT16 BaseLocal;
	UINT8 IncFact;

/* Fuelling calculations */

		  BaseLocal = Vars.FUEL_BaseTime;

							    /* Read the fuel correction percentage to make from the map */
		  FUEL_CorFactMap = (UINT16)FUEL_TableLookup((Vars.IOP_EngineSpeed / 64), (Vars.IOP_ManifoldPress / 12), HAL_FuelMapSel);

		  /* Rate limit the fuel factor to prevent fast changes */			
		  if ((Vars.FUEL_CorFactMap == 0) || (Vars.FUEL_CorFactMap < GlobalsRAM.FUEL_MAXGRADIENT) || (Vars.FUEL_CorFactMap > (2550 - GlobalsRAM.FUEL_MAXGRADIENT)))
		  	{
				Vars.FUEL_CorFactMap = FUEL_CorFactMap;
			}
		  else if (FUEL_CorFactMap > (Vars.FUEL_CorFactMap + GlobalsRAM.FUEL_MAXGRADIENT))
		  	{
				Vars.FUEL_CorFactMap = Vars.FUEL_CorFactMap + GlobalsRAM.FUEL_MAXGRADIENT;
			}
		  else if (FUEL_CorFactMap < (Vars.FUEL_CorFactMap - GlobalsRAM.FUEL_MAXGRADIENT))
		  	{
				Vars.FUEL_CorFactMap = Vars.FUEL_CorFactMap - GlobalsRAM.FUEL_MAXGRADIENT;
			}
		  else
		  	{
				Vars.FUEL_CorFactMap = FUEL_CorFactMap;
			}


			/* Find overall correction factor as product of size and map data */
		  FUEL_OverallTemp = (((UINT16)Vars.FUEL_CorFactSize * Vars.FUEL_CorFactMap) / 100);

			/* Limit to 255% of original */
		  if (FUEL_OverallTemp > 255)
		  	{
				FUEL_OverallTemp = 255;
			}
		  else
		  	{
			}

			/* Calculate increase duration for > 100% fuel charge */
          if (FUEL_OverallTemp > 100)
		  	{
				IncFact = (UINT8)FUEL_OverallTemp - 100;
			}
		  else
		  	{
				IncFact = 0;
			}

             /* Copy data in shortest possible time */
 		  FUEL_IncreaseFact = IncFact;

			/* Copy overall factor in AFTER increase factor  (ISR access order!) */
  		  FUEL_OverallFact = (UINT8)FUEL_OverallTemp;


		  HAL_FuelUnscaled = (UINT16)(((UINT32)BaseLocal * FUEL_OverallFact) / 100);
		  						/* Calculate correction factor to use for fuelling offset map    */

		  HAL_BaseScaled = BaseLocal * INJECTORFACTOR;


		  Vars.FUEL_CorrectTime = HAL_FuelUnscaled * INJECTORFACTOR;          
			  						/* Generate a corrected loading time for the injectors           */

									/* Calculate ISR extend time - not on low increases! */
		  if (Vars.FUEL_CorrectTime > (HAL_BaseScaled + 10))
		  	{
				HAL_FuelExtend = (Vars.FUEL_CorrectTime - HAL_BaseScaled);
			}
		  else
		  	{
				HAL_FuelExtend = 0;
			}

	return;
}


/***********************************************************************************/
/*                                                                                 */
/* IGN_Ignition10ms() - Ignition calculations                                      */
/*                                                                                 */
/***********************************************************************************/
void IGN_Ignition10ms (void)               	/* Ignition calculations 		*/
	{

    SINT8 IGN_CorAngleMap;
    UINT32 IGN_SparkTrig32;
	SINT16 IGN_RetardSum;
	UINT16 SparkLocal;
	
/* Ignition calculations */


		  SparkLocal = Vars.IGN_SparkPeriod;

		  /* Read the ignition angle correction to make from the map */
	      IGN_CorAngleMap = IGN_TableLookup((Vars.IOP_EngineSpeed / 64), (Vars.IOP_ManifoldPress / 12),HAL_IgnMapSel);

		  /* Rate limit the fuel factor to prevent fast changes */			
		  if ((Vars.IGN_CorAngleMap < (-128 + GlobalsRAM.IGN_MAXGRADIENT)) || (Vars.IGN_CorAngleMap > (127 - GlobalsRAM.IGN_MAXGRADIENT)))
		  	{
				Vars.IGN_CorAngleMap = IGN_CorAngleMap;
			}
		  else if (IGN_CorAngleMap > (Vars.IGN_CorAngleMap + GlobalsRAM.IGN_MAXGRADIENT))
		  	{
				Vars.IGN_CorAngleMap = Vars.IGN_CorAngleMap + GlobalsRAM.IGN_MAXGRADIENT;
			}
		  else if (IGN_CorAngleMap < (Vars.IGN_CorAngleMap - GlobalsRAM.IGN_MAXGRADIENT))
		  	{
				Vars.IGN_CorAngleMap = Vars.IGN_CorAngleMap - GlobalsRAM.IGN_MAXGRADIENT;
			}
		  else
		  	{
				Vars.IGN_CorAngleMap = IGN_CorAngleMap;
			}

			/* Sum the map offset with the clutch based (fast) offset, and limit */
          IGN_RetardSum = (SINT16)Vars.IGN_CorAngleMap + Vars.IGN_ClutchRetard;
		
		  if (IGN_RetardSum < -128)
		  	{
				Vars.IGN_FinalAngle = -128;
			}
		  else if (IGN_RetardSum > 127)
		  	{
				Vars.IGN_FinalAngle = 127;
			}
		  else
		  	{
				Vars.IGN_FinalAngle = (SINT8) IGN_RetardSum;
			}


		  /* Calculate the number of counts to be applied as lead/lag on the ignition signal */

/* Calculate the timer offset value for the PCA ISR to use when generating spark timer  */
		  if (Vars.IGN_FinalAngle <= 0) 			/* Positive number - advance         */
				{
					IGN_SparkTrig32 = (((UINT32)SparkLocal * (UINT16)(0 - Vars.IGN_FinalAngle * SPARKFACTOR)) / (IGN_AnglePerCylinder));

					if (IGN_SparkTrig32 > 0xFFF0)
						{
							Vars.IGN_SparkTrigger = 0xFFF0;
						}
					else
						{
							Vars.IGN_SparkTrigger = (UINT16)IGN_SparkTrig32;
						}
				}    /* Calculate only retard delays here */
		  else
		  		{
					Vars.IGN_SparkTrigger = 0;/* Allow only retard for now! */
				
				}
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* VS_GearRatioDet10ms() - Gear ratio detection                                    */
/*                                                                                 */
/***********************************************************************************/
void VS_GearRatioDet10ms(void) 				/* Gear ratio detection 		*/
	{

	UINT8 VS_TableIndex;

/* Gear ratio detection */
		  /* Calculate ratio between engine and output speed */
		  if (Vars.IOP_VehicleSpeed > VS_MINVEHSPEEDGEARDET)
			{
                Vars.VS_Ratio = (Vars.IOP_EngineSpeed * 5);
				Vars.VS_Ratio /= (Vars.IOP_VehicleSpeed / 2);

                VS_TableIndex = 0;
				  do
				  	{				
					} while ((Vars.VS_Ratio < VS_RatioSplits[VS_TableIndex]) && (++VS_TableIndex < 7));

				Vars.VS_Gear = VS_TableIndex + 1;			 
			 }
		  else
		 	 {
			    Vars.VS_Ratio = 0x250; /* Report 25:1 ratio */
				Vars.VS_Gear = 1;	   /* Report gear 1     */
			 } 		
	return;
}


/***********************************************************************************/
/*                                                                                 */
/* TURBO_BoostLimit10ms() - Handle boost limit/restart                             */
/*                                                                                 */
/***********************************************************************************/
void TURBO_BoostLimit10ms(void) 			/* Handle boost limit/restart   */
	{

		  /* Handle boost limit/restart */
		  if (Vars.IOP_ManifoldPress >= ((UINT16)GlobalsRAM.TURBO_BOOSTCUT * 12))
		  											/* Has maximum turbo pressure been exceeded */
		  		{
					Vars.TURBO_OkayCounter = 0;
					if (Vars.TURBO_CutCounter < GlobalsRAM.TURBO_CUTTIME)
						{
							Vars.TURBO_CutCounter++;
						}
					else
						{
							if (GlobalsRAM.VS_LIMITEROPTIONS & IOP_OPTIONLIMITERSPARK)
								{
									Vars.IOP_CutBits |= CUTOBSPARK;
								}  
							if (GlobalsRAM.VS_LIMITEROPTIONS & IOP_OPTIONLIMITERFUEL)
								{
									Vars.IOP_CutBits |= CUTOBFUEL;
								}  
						}
				}  	
          else if (Vars.IOP_ManifoldPress <= ((UINT16)GlobalsRAM.TURBO_BOOSTOKAY * 12))
		  											 /* Less than restart boost level?     */
		  		{
					Vars.TURBO_CutCounter = 0;
					if (Vars.TURBO_OkayCounter < GlobalsRAM.TURBO_OKAYTIME)
						{
							Vars.TURBO_OkayCounter++;
						}
					else
						{
							Vars.IOP_CutBits &= ~CUTOBCLEAR;
						}
				}
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* VS_ClutchHandle10ms() - Handle clutch controlled func                           */
/*                                                                                 */
/***********************************************************************************/
void  VS_ClutchHandle10ms(void) 				/* Handle clutch controlled func*/
	{

static UINT8 IOP_ClutchSwitchPrevious = 0;


		  /* Handle clutch controlled functions */
		  if (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHENABLE)
		  	{				/* Clutch switch enabled? */
		  		if (Vars.IOP_ClutchSwitch)
					{		/* Full throttle shift */
						if ((GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHFTENABLE) && \
						(Vars.IOP_ThrottlePos >= GlobalsRAM.VS_FTMINTHROTTLE) && \
						(Vars.IOP_VehicleSpeed >= ((UINT16)GlobalsRAM.VS_LAUNCHMAXVEHICLE * 64)) && \
						(Vars.IOP_EngineSpeed > 1000) && \
						(IOP_ClutchSwitchPrevious == 0) && \
						(Vars.VS_ClutchTimer == 0))
							{		/* Just triggered? */
								Vars.VS_EngLimClutch = Vars.IOP_EngineSpeed + ((UINT16)GlobalsRAM.VS_FTSHIFTMAXRISE * 64);
									/* Snapshot engine speed + offset */
								VS_FTDropToRPM = (UINT16)(((UINT32)Vars.VS_EngLimClutch * VS_GearRatio[Vars.VS_Gear]) / VS_GearRatio[Vars.VS_Gear - 1]);

								VS_FTDropToRPM += ((UINT16)GlobalsRAM.VS_FTSHIFTTARGABOVE * 64);
									/* Calculate drop to RPM */
								if (VS_FTDropToRPM > (Vars.VS_EngLimClutch - 200))
									{
										VS_FTDropToRPM = (Vars.VS_EngLimClutch - 200);
									}

								ClutchType = CLUTCHFTLIMIT;
									/* Reset clutch timer */
							}
							/* Launch Control */
						if ((GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHLCENABLE) && \
						(Vars.IOP_ThrottlePos >= GlobalsRAM.VS_LCACTTHROTTLE) && \
						(Vars.IOP_VehicleSpeed < ((UINT16)GlobalsRAM.VS_LAUNCHMAXVEHICLE* 64))&& \
						(Vars.VS_ClutchTimer == 0))
							{		/* Just triggered? */
								Vars.VS_EngLimClutch = ((UINT16)GlobalsRAM.VS_LAUNCHRPM * 64);
									/* Set engine speed limiter */
								ClutchType = CLUTCHLCLIMIT;
									/* Reset clutch timer */
								Vars.VS_ClutchTimer = 0;
							}
							/* Running full throttle shift? */

						if (ClutchType == CLUTCHFTLIMIT)
							{
								if (Vars.VS_ClutchTimer < 65535)
									{
										Vars.VS_ClutchTimer += 1;      
														/* Increment clutch timer */
                                        Vars.VS_EngLimClutch -= GlobalsRAM.VS_FTFALLSPEEDGRAD;
										                /* Generate falling RPM target */
									}
													    /* Check for shift timeout/complete */
								if ((Vars.VS_ClutchTimer >  ((UINT16)GlobalsRAM.VS_FTMAXTIME * 10)) || (Vars.IOP_EngineSpeed <=VS_FTDropToRPM))
									{
										Vars.VS_EngLimClutch = 20000;
														/* Push limit up            */
			 						  	ClutchType = 0;
										/* Timer != 0 prevents re-entry */
									}
							}
							/* Running launch control       */
						if (ClutchType == CLUTCHLCLIMIT)
							{
								if (Vars.VS_ClutchTimer < 65535)
									{
										Vars.VS_ClutchTimer += 1;      
														/* Increment clutch timer */
									}
								if ((Vars.IOP_VehicleSpeed >= ((UINT16)GlobalsRAM.VS_LAUNCHMAXVEHICLE * 64)) || \
								    (Vars.VS_ClutchTimer > ((UINT16)GlobalsRAM.VS_LCMAXTIME * 10))    || \
									(Vars.IOP_ThrottlePos < GlobalsRAM.VS_LCDISTHROTTLE))
									{

										Vars.VS_EngLimClutch = 20000;
														/* Push limit up            */
			 						  	ClutchType = 0;
										/* Timer != 0 prevents re-entry */
									}
							}
					}
				  else
				  	{
						if ((ClutchType == CLUTCHFTLIMIT) && (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHFTLINGER))
							{
								if (Vars.VS_ClutchTimer < 65535)
									{
										Vars.VS_ClutchTimer += 1;      
														/* Increment clutch timer */
                                        Vars.VS_EngLimClutch -= GlobalsRAM.VS_FTFALLSPEEDGRAD;
										                /* Generate falling RPM target */

									}
													    /* Check for shift timeout/complete */
								if ((Vars.VS_ClutchTimer > ((UINT16)GlobalsRAM.VS_FTMAXTIME * 10)) || (Vars.IOP_EngineSpeed <=VS_FTDropToRPM))
									{
										Vars.VS_EngLimClutch = 20000;
														/* Push limit up            */
			 						  	ClutchType = 0;
								  	
									    Vars.VS_ClutchTimer = 0;
										
									}
							}
						else
							{
								  Vars.VS_EngLimClutch = 20000;
								  /* Push limit up            */
 						  		  ClutchType = 0;
						  		  Vars.VS_ClutchTimer = 0;
							}
					}  
			}
		  else				/* Clutch switch not enabled */
		   	{
				  Vars.VS_EngLimClutch = 20000;
							/* Push limit up            */
				  ClutchType = 0;
				  Vars.VS_ClutchTimer = 0;
			}

		  if (Vars.VS_EngLimClutch < 900)
		  	{
				Vars.VS_EngLimClutch = 900;
			}

				/* Store previous clutch switch state */
		  IOP_ClutchSwitchPrevious = Vars.IOP_ClutchSwitch;

	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_RPMLimiter10ms() - Handle engine speed limiter functions                    */
/*                                                                                 */
/***********************************************************************************/


void IOP_RPMLimiter10ms()
{	

	UINT16 limitlocal;
	UINT8 limitcause;
    SINT16 CLRetard;
	UINT8 RPMError;

    limitlocal = GlobalsRAM.IOP_MAXENGINESPEED * 64;/* Local limit = main      */

	limitcause = LIMCAUSEMAIN;						/* Set cause to 1 (main)   */
	

	if (Vars.VS_EngLimClutch < limitlocal) 			/* Main engine limit lower?*/
		{
			limitlocal = Vars.VS_EngLimClutch;
													/* Apply lower limiter     */
 
			limitcause = LIMCAUSECLUTCH;			/* Set cause to 2 (clutch) */
		}                                           

    CLRetard = 0;

	/* Handle the main RPM limiter */
	if (limitcause == LIMCAUSEMAIN)
		{
			if (Vars.IOP_EngineSpeed > limitlocal)
				{

				if (GlobalsRAM.VS_LIMITEROPTIONS & IOP_OPTIONLIMITERFUEL)
					{
						Vars.IOP_CutBits |= CUTRPMFUEL;	
					}
				else
					{
						Vars.IOP_CutBits |= CUTRPMSPARK;
					}
				}
			else if (Vars.IOP_EngineSpeed < (limitlocal - GlobalsRAM.VS_LIMITERHYST))
				{
					Vars.IOP_CutBits &= ~CUTRPMCLEAR;
					Vars.IOP_CutBits &= ~CUTCLTCLEAR;
				}
			Vars.IGN_ClutchRetard = 0;
		}
	/* Handle the clutch RPM limiter */
	else
		{

			/* Check the limiter type, and apply retard/cut as required */
			if (ClutchType == CLUTCHFTLIMIT)
				{			
					/* Limiting required? */
					if (Vars.IOP_EngineSpeed > limitlocal)
						{
							/* Yes, calculate error */
							if (Vars.IOP_EngineSpeed >= limitlocal + 255)
								{
									RPMError = 255;
								}
							else
								{
									RPMError = (UINT8)(Vars.IOP_EngineSpeed - limitlocal);
								}

							CLRetard = (SINT16)(((UINT16)RPMError * GlobalsRAM.VS_FTRPMRETARDGAIN) / 256);

							if (CLRetard > 127)
								{
									CLRetard = 127;
								}

							Vars.IGN_ClutchRetard = (SINT8)(0 - CLRetard);

							if (Vars.IGN_ClutchRetard <= GlobalsRAM.VS_FTRETARDLIMIT)
								{
									if (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHFTSPARK)
										{
											Vars.IOP_CutBits |= CUTCLTSPARK;
										}
									else
										{
											Vars.IOP_CutBits |= CUTCLTFUEL;	
										}			
									Vars.IGN_ClutchRetard = GlobalsRAM.VS_FTRETARDLIMIT;
								}
							else
								{
									Vars.IOP_CutBits &= ~CUTRPMCLEAR;
									Vars.IOP_CutBits &= ~CUTCLTCLEAR;
								}
						}
					else
						{
							/* Under limiter RPM, no cut, no retard */
							Vars.IOP_CutBits &= ~CUTRPMCLEAR;
							Vars.IOP_CutBits &= ~CUTCLTCLEAR;
							Vars.IGN_ClutchRetard = 0;
						}
				}
			else			/* CLUTCHLCLIMIT */
				{
					/* Limiting required? */
					if (Vars.IOP_EngineSpeed > limitlocal)
						{
							/* Yes, calculate error */
							if (Vars.IOP_EngineSpeed >= limitlocal + 255)
								{
									RPMError = 255;
								}
							else
								{
									RPMError = (UINT8)(Vars.IOP_EngineSpeed - limitlocal);
								}

							CLRetard = (SINT16)(((UINT16)RPMError * GlobalsRAM.VS_LCRPMRETARDGAIN) / 256);

							if (CLRetard > 127)
								{
									CLRetard = 127;
								}

							Vars.IGN_ClutchRetard = (SINT8)(0 - CLRetard);

							if (Vars.IGN_ClutchRetard <= GlobalsRAM.VS_LCRETARDLIMIT)
								{
									if (GlobalsRAM.VS_CLUTCHOPTIONS & VS_CLUTCHLCSPARK)
										{
											Vars.IOP_CutBits |= CUTCLTSPARK;
										}
									else
										{
											Vars.IOP_CutBits |= CUTCLTFUEL;	
										}			
									Vars.IGN_ClutchRetard = GlobalsRAM.VS_LCRETARDLIMIT;
								}
							else
								{
									Vars.IOP_CutBits &= ~CUTRPMCLEAR;
									Vars.IOP_CutBits &= ~CUTCLTCLEAR;
								}
						}
					else
						{
							/* Under limiter RPM, no cut, no retard */
							Vars.IOP_CutBits &= ~CUTRPMCLEAR;
							Vars.IOP_CutBits &= ~CUTCLTCLEAR;
							Vars.IGN_ClutchRetard = 0;
						}
				}
		}

	Vars.VS_OveralLimiter = limitlocal;

	Vars.IOP_CutBits &= ~CUTSPR2CLEAR;

	return;
}

/***********************************************************************************/
/*                                                                                 */
/* TURBO_BoostControl10ms() - Boost closed loop control                            */
/*                                                                                 */
/***********************************************************************************/

void TURBO_BoostControl10ms()				  /* Boost closed loop control 	*/
	{
          
/* Boost closed loop control */
		  if (++HAL_BoostSchedule >= 3)       /* Every 30ms, run the boost control 	*/
				{
				  HAL_BoostSchedule = 0;      /* Reset scheduler 					*/

				  if ((Vars.IOP_ManifoldPress < ((UINT16)GlobalsRAM.TURBO_MINMAPCL * 12)) || (Vars.IOP_ThrottlePos < GlobalsRAM.TURBO_MINTHROTTLE) || (Vars.IOP_OpMode == ENGINESTOP))
					{										/* Conditions not good for boost control - so don't */
						
						Vars.TURBO_ValveDuty = HAL_BOOSTMINDUTY;
												           /* Valve off           */
				  		Vars.TURBO_MAPITerm = 0;           /* Reset the I Term    */
						Vars.TURBO_ClosedLoop = 0;         /* Indicate no control */
                        Vars.TURBO_MAPTarget = 0;          /* Zero the data       */
                        Vars.TURBO_MAPError  = 0;
						Vars.TURBO_MAPPTerm = 0;								
						Vars.TURBO_MAPDTerm = 0;								
					}
				  else
				  	{


			          	Vars.TURBO_Feedforward = TURBO_DutyTableLookup((Vars.IOP_EngineSpeed / 64), (Vars.IOP_ThrottlePos),HAL_TurboMapSel);
															/* Get FF term            	*/	
				       	Vars.TURBO_MAPTarget = ((UINT16)TURBO_TargetTableLookup((Vars.IOP_EngineSpeed / 64), (Vars.IOP_ThrottlePos),HAL_TurboMapSel) * 12);
															/* Get MAP target 			*/
                        TURBO_PtrMaxBoostGr = &GlobalsRAM.TURBO_GR1MAXBOOST;
															/* Look up into maximum boost vs gear */
						if ((Vars.VS_Gear >= 1) && (Vars.VS_Gear <= 8))
							{
								TURBO_LimitGear = (UINT16)TURBO_PtrMaxBoostGr[Vars.VS_Gear - 1] * 12;
							}
						else
							{
								TURBO_LimitGear = 0xFFFF;
							}
															/* Calculate limit based on gear */
						if (Vars.TURBO_MAPTarget > TURBO_LimitGear)
							{
								Vars.TURBO_MAPTarget = TURBO_LimitGear;
							}
			          	Vars.TURBO_MAPError = (SINT16)Vars.IOP_ManifoldPress - Vars.TURBO_MAPTarget;
															/* Calculate error 			*/
						if (Vars.TURBO_ClosedLoop == 0)
							{								/* Just restarted? 			*/
								Vars.TURBO_ClosedLoop = 1;
															/* Flag closed loop again   */
								Vars.TURBO_MAPPTerm = 0;								
															/* Zero P Term (FTT)		*/			
						  		Vars.TURBO_MAPITerm = 0;
															/* Zero I Term (FTT)        */
							}
						else
							{
							  	Vars.TURBO_MAPPTerm = (SINT16)((((SINT32)Vars.TURBO_MAPError) * GlobalsRAM.TURBO_PGAIN) / 256);
							}								/* P Term 					*/
						
						if (GlobalsRAM.TURBO_IGAIN == 0)
				  			{								/* Only calculate I part if gain non zero */
						  		Vars.TURBO_MAPITerm = 0;
							}
				  		else
				  			{

								if (Vars.TURBO_MAPError > ((SINT16)GlobalsRAM.TURBO_MAXIERROR * 12))
									{
										HAL_IErrorTemp = ((SINT16)GlobalsRAM.TURBO_MAXIERROR * 12);
									}
								else if (Vars.TURBO_MAPError < ((SINT16)GlobalsRAM.TURBO_MINIERROR * 12))
									{
										HAL_IErrorTemp = ((SINT16)GlobalsRAM.TURBO_MINIERROR * 12);
									}
								else
									{
										HAL_IErrorTemp = Vars.TURBO_MAPError;
									}
					
						  		Vars.TURBO_MAPITerm += (SINT16)(((SINT32)HAL_IErrorTemp * GlobalsRAM.TURBO_IGAIN) / 16);
							}								/* I Term 					*/
		
					  	if (Vars.TURBO_MAPITerm > (GlobalsRAM.TURBO_ITERMMAX * 256))
				  			{									/* Limit the I part 		*/
								Vars.TURBO_MAPITerm = (GlobalsRAM.TURBO_ITERMMAX * 256);
							}
				  		else if (Vars.TURBO_MAPITerm < (GlobalsRAM.TURBO_ITERMMIN * 256))
				  			{
								Vars.TURBO_MAPITerm = (GlobalsRAM.TURBO_ITERMMIN * 256);
							}

						if (GlobalsRAM.TURBO_DGAIN == 0)
				  			{								/* Only calculate D part if gain non zero */
						  		Vars.TURBO_MAPDTerm = 0;
							}
				  		else
				  			{
								Vars.TURBO_MAPDTerm = (SINT16)((((SINT32)Vars.IOP_dManifold) * GlobalsRAM.TURBO_DGAIN) / 128);
							}
				  
				  		HAL_TurboDuty = (SINT16)Vars.TURBO_Feedforward + (Vars.TURBO_MAPITerm / 256) + Vars.TURBO_MAPPTerm + Vars.TURBO_MAPDTerm;

				  		if (HAL_TurboDuty > GlobalsRAM.TURBO_MAXDUTY)
				  			{
								Vars.TURBO_ValveDuty = GlobalsRAM.TURBO_MAXDUTY;
							}
				  		else if (HAL_TurboDuty < GlobalsRAM.TURBO_MINDUTY)
				  			{
								Vars.TURBO_ValveDuty = GlobalsRAM.TURBO_MINDUTY;
							}
				 	 	else
				  			{
								Vars.TURBO_ValveDuty = (UINT8)HAL_TurboDuty;
							}
						}	
					/* Write the duty request to the high speed output channel */

					if (Vars.TURBO_ValveDuty <= HAL_BOOSTMINDUTY)
						{
							TURBODUTYSET(HAL_BOOSTMINHIGH);
						}
					else if (Vars.TURBO_ValveDuty >= HAL_BOOSTMAXDUTY)
						{
							TURBODUTYSET(HAL_BOOSTMINLOW);
						}
					else
						{
							TURBODUTYSET(((UINT16)(100 - Vars.TURBO_ValveDuty) * HAL_BOOSTONEPERCENT));
						}
				}
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_FakeMAP10ms() - Calculate DAC fake MAP input                                */
/*                                                                                 */
/***********************************************************************************/
void IOP_FakeMAP10ms(void)					/* Calculate DAC fake MAP input */
	{
		UINT16 IOP_DACdata;

		  /* Calculate DAC fake signal for MAP input */
	      if (Vars.IOP_ManifoldPress >= ((UINT16)GlobalsRAM.DAC_FAKEMAXMAPPRESS * 12))
		  	{
				Vars.DAC_FakeCountsUL = GlobalsRAM.DAC_FAKEMAPMAX;
			}
          else
		  	{
				Vars.DAC_FakeCountsUL = GlobalsRAM.DAC_FAKEMAPMIN + (UINT8)(((UINT32)Vars.IOP_ManifoldPress * (GlobalsRAM.DAC_FAKEMAPMAX - GlobalsRAM.DAC_FAKEMAPMIN)) / ((UINT16)GlobalsRAM.DAC_FAKEMAXMAPPRESS * 12));
			}

          	/* Find limited version of DAC for fake signal */
		  if (Vars.DAC_FakeCountsUL > GlobalsRAM.DAC_FAKEFCDMAX)
		   	{
				Vars.DAC_FakeCounts = GlobalsRAM.DAC_FAKEFCDMAX;
			}
		  else
		   	{
				Vars.DAC_FakeCounts = Vars.DAC_FakeCountsUL;
			}

		   /* Convert the 0-5V = 0-255 counts output to a 0 - 4.096V = 0 - 4096 counts DAC SPI message */
		  IOP_DACdata = (((UINT16)Vars.DAC_FakeCounts * 5) * 4);

		  /* Ensure that the 4.096V limit isn't exceeded */
		  if (IOP_DACdata > 4095)
		  	{
				IOP_DACdata = 4095;
			}

          /* Add the data transfer mask b15 = 0 (listen to it), b14 = x (don't care), b13 = /GA (gain select, 0 = x2 gain = 4.096V), b12 = /SHDN (1 = run) */
		  DACConv = (IOP_DACdata | 0x1000);
	return;
}


/***********************************************************************************/
/*                                                                                 */
/* IOP_UpdatePCData10ms() - Byte switch for monitor                                */
/*                                                                                 */
/***********************************************************************************/
void IOP_UpdatePCData10ms(void)  			/* Byte switch for monitor      */
	{

/* Byte switch for monitor variables */
			/* Copy the 16 bit variables over */
#ifdef INVERT
			Vars.IOP_EngineSpeedd 		= switchu16(Vars.IOP_EngineSpeed);
			BOBEngineSpeedd = Vars.IOP_EngineSpeedd;
			Vars.IOP_ManifoldPressd 	= switchu16(Vars.IOP_ManifoldPress);
		    Vars.FUEL_CorFactSized 		= switchu16(((UINT16)Vars.FUEL_CorFactSize * 10));
			Vars.FUEL_CorFactMapd 		= switchu16(((UINT16)Vars.FUEL_CorFactMap * 10));
			Vars.FUEL_BaseTimed   		= switchu16(Vars.FUEL_BaseTime);
			if (Vars.IOP_CutBits & CUTFUELEITHER)
				{
					Vars.FUEL_CorrectTimed  	= 0;
				}
			else
				{
					Vars.FUEL_CorrectTimed  	= switchu16(HAL_FuelUnscaled);
				}
			Vars.IGN_SparkPeriodd  		= switchu16(Vars.IGN_SparkPeriod);
			Vars.IGN_CorAngleMapd  		= switchs16(((SINT16)Vars.IGN_CorAngleMap * 10));
			Vars.IGN_SparkHighd  		= switchu16(Vars.IGN_SparkHigh);
			Vars.IGN_SparkTriggerd  	= switchu16(Vars.IGN_SparkTrigger);
			Vars.TURBO_MAPTargetd		= switchu16(Vars.TURBO_MAPTarget);
			Vars.TURBO_MAPErrord		= switchs16(Vars.TURBO_MAPError);
			Vars.TURBO_MAPPTermd		= switchs16(Vars.TURBO_MAPPTerm);
			Vars.TURBO_MAPITermd		= switchs16(Vars.TURBO_MAPITerm);
            Vars.VS_VehicleSpeedd  		= switchu16(Vars.IOP_VehicleSpeed);
            Vars.VS_Ratiod  		    = switchu16(Vars.VS_Ratio);
			Vars.VS_EngLimClutchd   	= switchu16(Vars.VS_OveralLimiter);
#else
			Vars.IOP_EngineSpeedd 		= Vars.IOP_EngineSpeed;
			BOBEngineSpeedd = Vars.IOP_EngineSpeedd;
			Vars.IOP_ManifoldPressd 	= Vars.IOP_ManifoldPress;
		    Vars.FUEL_CorFactSized 		= ((UINT16)Vars.FUEL_CorFactSize * 10);
			Vars.FUEL_CorFactMapd 		= ((UINT16)Vars.FUEL_CorFactMap * 10);
			Vars.FUEL_BaseTimed   		= (Vars.FUEL_BaseTime);
			if (Vars.IOP_CutBits & CUTFUELEITHER)
				{
					Vars.FUEL_CorrectTimed  	= 0;
				}
			else
				{
					Vars.FUEL_CorrectTimed  	= HAL_FuelUnscaled;
				}
#ifdef REVENG
			Vars.IGN_SparkPeriodd  		= HAL_IgnitionDelay;
#else
			Vars.IGN_SparkPeriodd  		= Vars.IGN_SparkPeriod;
#endif
			Vars.IGN_CorAngleMapd  		= ((SINT16)Vars.IGN_CorAngleMap * 10);
			Vars.IGN_SparkHighd  		= Vars.IGN_SparkHigh;
			Vars.IGN_SparkTriggerd  	= Vars.IGN_SparkTrigger;
			Vars.TURBO_MAPTargetd		= Vars.TURBO_MAPTarget;
			Vars.TURBO_MAPErrord		= Vars.TURBO_MAPError;
			Vars.TURBO_MAPPTermd		= Vars.TURBO_MAPPTerm;
			Vars.TURBO_MAPITermd		= Vars.TURBO_MAPITerm;
            Vars.VS_VehicleSpeedd  		= Vars.IOP_VehicleSpeed;
            Vars.VS_Ratiod  		    = Vars.VS_Ratio;
			Vars.VS_EngLimClutchd   	= Vars.VS_OveralLimiter;

#endif

	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_Convert100ms() - HAL to IOP Conversion                                      */
/*                                                                                 */
/***********************************************************************************/
void IOP_Convert100ms ()				/* HAL to IOP Conversion 		*/
	{


		/* Perform conversion on 100ms variables */
		/* Convert Coolant Temperature */
		   Vars.IOP_CoolantTemp = IOP_COOLTMPMAP[Vars.HAL_CoolTempRaw];
		/* Convert Inlet Air Temp  */
		   Vars.IOP_AirInletTemp = IOP_COOLTMPMAP[Vars.HAL_AirInletRaw];
		/* Convert Battery Voltage */
		   Vars.IOP_BatteryVolts = (UINT8)(((UINT16)Vars.HAL_VBattRaw * 72) / 100) + 8;
		/* Convert Sensor Voltage */
		   Vars.IOP_SensorVolts = (UINT8)(((UINT16)Vars.HAL_VSenseRaw * 41) / 100);
        /* Convert vehicle speed to kph */
		   Vars.IOP_VehicleSpeedKph = (UINT16)(((UINT32)Vars.IOP_VehicleSpeed * VS_KPHFact) / 100);
      	/* And bit switch for display */
#ifdef INVERT
		   Vars.VS_Vehiclekphd  		= switchu16(Vars.IOP_VehicleSpeedKph);
#else
		   Vars.VS_Vehiclekphd  		= Vars.IOP_VehicleSpeedKph; 
#endif
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_Housekeeping100ms() - housekeeping functions                                */
/*                                                                                 */
/***********************************************************************************/
void IOP_Housekeeping100ms (void)			/* 100ms housekeeping functions */
	{


		/* Check status of global map select switch, and action */

		if (GlobalsRAM.IOP_ANIN1FUNCTION == IOP_MAPSELECTSWITCH)
			{	/* Use ANIN1 as map selection */
				if (Vars.HAL_ANA1 > HAL_HALFRAIL)
					{
						HAL_TurboMapSel = 1;
				
						HAL_IgnMapSel = 1;
						
						HAL_FuelMapSel = 1;
					}
				else
					{
						HAL_TurboMapSel = 0;
				
						HAL_IgnMapSel = 0;
						
						HAL_FuelMapSel = 0;
					}

			}
		else
			{	/* Use standard parameters for map selections */

				HAL_TurboMapSel = GlobalsRAM.IOP_TURBOMAPSELECT;
		
				HAL_IgnMapSel = GlobalsRAM.IOP_IGNMAPSELECT;
				
				HAL_FuelMapSel = GlobalsRAM.IOP_FUELMAPSELECT;
			}

				/* Concatenate bits to make a display variable */
		Vars.IOP_MapSwitches = (HAL_TurboMapSel * 4) + (HAL_IgnMapSel * 2) + HAL_FuelMapSel;

		if (++HAL_BackgroundCnt >= 5)
			{
				HAL_BackgroundCnt = 0;
			}
		else					
			{
			}
	return;
}



/***********************************************************************************/
/*                                                                                 */
/* IOP_BurnCheckBL() - Background burn check                                       */
/*                                                                                 */
/***********************************************************************************/

void IOP_BurnCheckBL(void)					/* Background burn check        */
	{

	    __bit EA_SAVE;
    	SEGMENT_VARIABLE_SEGMENT_POINTER(pwrite, UINT8, SEG_XDATA, SEG_DATA);
    	SINT8 write_erase;
		UINT8 * copyptr;
		UINT16 pagecnt;

	/* Check for request to burn from PC session - check the rules, and burn if required */
	if ((Vars.HAL_Burn == 1) && (Vars.IOP_BatteryVolts < HAL_MAXBATTERYFORBURN) && \
	(Vars.IOP_SensorVolts < HAL_MAXSENSORFORBURN) && (Vars.IOP_EngineSpeed == 0))

		{
			/* Erase pages to be written for new set */

			   EA_SAVE = EA;                   		// preserve EA
						
			   EA = 0;                             // disable interrupts
			
			   VDM0CN = 0x80;

			   RSTSRC = (RSTSRC_VAL | 0x02);

               HAL_Debug = FLKEY;

	           if (ROMActivePage & 0x01)
			   		{
			   			flashAddress = ROMPAGE2START;       // copy passed address
					}
			   else
			   		{
			   			flashAddress = ROMPAGE1START;       // copy passed address
					}


			   for (pagecnt = 0; pagecnt < 8; pagecnt++, flashAddress += 0x200)
			   		{
				       pwrite = (UINT8 SEG_XDATA *) flashAddress;
					   FLKEY  = 0xA5; 
					   FLKEY  = 0xF1;
				       PSCTL |= 0x03;              // set up PSEE, PSWE
				       *pwrite = 0x55;             // write the byte
				       PSCTL &= ~0x03;             // clear PSEE and PSWE			       
					}

	           if (ROMActivePage & 0x01)
			   		{
			   			flashAddress = ROMPAGE2START;       // copy passed address
					}
			   else
			   		{
			   			flashAddress = ROMPAGE1START;       // copy passed address
					}

			   write_erase = 0x01;                 // set to perform segment erase
				
               copyptr = (UINT8 *)Partables[0].addrRam;        
                                                   // Point to start of RAM copy
		       pwrite = (UINT8 SEG_XDATA *) flashAddress;

			   for (pagecnt = 0; pagecnt < 0x100; pagecnt++)
			   		{
					   FLKEY  = 0xA5; 
					   FLKEY  = 0xF1;
				       PSCTL |= 0x01;		       // set up PSEE, PSWE
				       *pwrite++ = *copyptr++;     // write the byte
				       PSCTL &= ~0x03;             // clear PSEE and PSWE			       
					}

               copyptr = (UINT8 *)Partables[1].addrRam;        
                                                   // Point to start of RAM copy
               flashAddress += 0x200;

		       pwrite = (UINT8 SEG_XDATA *) flashAddress;

			   for (pagecnt = 0; pagecnt < 0xC00 ; pagecnt++)
			   		{
					   FLKEY  = 0xA5; 
					   FLKEY  = 0xF1;
				       PSCTL |= 0x01;		       // set up PSEE, PSWE
				       *pwrite++ = *copyptr++;     // write the byte
				       PSCTL &= ~0x03;             // clear PSEE and PSWE			       
					}


			   flashAddress = FLASH_SAFE_ADDR;   // reset to safe address

 	           ROMActivePage = Boost2RAM.PageCounter;
 	
			   Boost2RAM.PageCounter = ROMActivePage + 1;

			   Vars.HAL_Burn = 0;

			   EA = EA_SAVE;                       // restore interrupts
			
		}
	return;
}


/***********************************************************************************/
/*                                                                                 */
/* switchu16() - Routine switches byte order of passed 16 bit unsigned             */
/*                                                                                 */
/***********************************************************************************/
UINT16 switchu16(UINT16 swin)
	{
		UINT16 swout;

			swout = (swin & 0x00FF) * 256;
			swout += (swin / 256);
		return (swout);
	}

/***********************************************************************************/
/*                                                                                 */
/* switchs16() - Routine switches byte order of passed 16 bit signed               */
/*                                                                                 */
/***********************************************************************************/
SINT16 switchs16(SINT16 swin)
	{
		SINT16 swout;

			swout = (swin & 0x00FF) * 256;
			swout += (swin / 256);
		return (swout);
	}


/***********************************************************************************/
/*                                                                                 */
/* UART_Init() - Initialisation routines for UART1                                 */
/*                                                                                 */
/***********************************************************************************/

void UART_Init()
{

	SMOD1     = 0x0C;
	SCON1     = 0x10;
    SBRLL1    = 0x30;			 /* Enable UART 1 8N1 115385 (115,200) */
    SBRLH1    = 0xFF;
    SBCON1   |= 0x03;            /* set prescaler to 1 */
    SCON1    |= 0x02;
    SBCON1   |= 0x40;
}




/***********************************************************************************/
/*                                                                                 */
/* Port_Init() - Initialisation routines for the processor port pins               */
/*                                                                                 */
/***********************************************************************************/

void Port_Init(void)
{
    // P0.0  -  SCK  (SPI0), Open-Drain, Digital
    // P0.1  -  MISO (SPI0), Open-Drain, Digital
    // P0.2  -  MOSI (SPI0), Open-Drain, Digital
    // P0.3  -  NSS, (SPI0), Open-Drain, Digital
    // P0.4  -  CEX0  (PCA), Open-Drain, Digital
    // P0.5  -  CEX1  (PCA), Open-Drain, Digital
    // P0.6  -  CEX2  (PCA), Push-Pull,  Digital
    // P0.7  -  CEX3  (PCA), Push-Pull,  Digital

    // P1.0  -  CEX4  (PCA), Open-Drain, Digital
    // P1.1  -  TX1 (UART1), Push-Pull,  Digital
    // P1.2  -  RX1 (UART1), Open-Drain, Digital
    // P1.3  -  Unassigned,  Push-Pull,  Digital
    // P1.4  -  Unassigned,  Push-Pull,  Digital
    // P1.5  -  Unassigned,  Open-Drain, Digital
    // P1.6  -  Unassigned,  Open-Drain, Digital
    // P1.7  -  Unassigned,  Open-Drain, Digital

    // P2.0  -  Unassigned,  Open-Drain, Digital
    // P2.1  -  Unassigned,  Open-Drain, Digital
    // P2.2  -  Unassigned,  Push-Pull,  Digital
    // P2.3  -  Unassigned,  Open-Drain, Digital
    // P2.4  -  Unassigned,  Open-Drain, Digital
    // P2.5  -  Unassigned,  Open-Drain, Digital
    // P2.6  -  Unassigned,  Open-Drain, Digital
    // P2.7  -  Unassigned,  Open-Drain, Digital

    // P3.0  -  Unassigned,  Push-Pull,  Digital


    P1MDIN    = 0x3F;
    P2MDIN    = 0x44;
    P0MDOUT   = 0xCD;
    P1MDOUT   = 0x1A;
    P2MDOUT   = 0x04;
    P3MDOUT   = 0x01;

    XBR0      = 0x02;
    XBR1      = 0x45;
    XBR2      = 0x01;

}

/***********************************************************************************/
/*                                                                                 */
/* Timer_Init() - Initialisation routines for the timers                           */
/*                                                                                 */
/***********************************************************************************/

void Timer_Init(void)
{

	
	TCON      = 0x50;

    TMOD      = 0x12;

    CKCON     = 0x06;

    TMR2CN    = 0x04;

    TMR3CN    = 0x0C;

    TMR2RLL   = 0x18; // 4khz

    TMR2RLH   = 0xFC;

    TMR3L     = 0xFF;

    TMR3H     = 0xFF;

    TL0       = 0xA0;

    TH0       = 0xA0;

}


/***********************************************************************************/
/*                                                                                 */
/* Adc_Init() - Initialisation routine for the A to D converter                    */
/*                                                                                 */
/***********************************************************************************/

void Adc_Init(void)
{
   REF0CN  = 0x08;                        // Enable voltage reference VREF

   AMX0P = 0x1E;                          // Positive input starts as temp sensor

   AMX0N = 0x1F;                          // Single ended mode(negative input = gnd)

   ADC0CF  = 0xFC;                        // SAR Period 0x1F, Left Justified

   ADC0CN  = 0xC2;                        // Continuous converion on timer 2 overflow
                                          // with low power tracking mode on
   EIE1   |= 0x08;                        // Enable conversion complete interrupt
}





/***********************************************************************************/
/*                                                                                 */
/* PCA_Init() - Initialisation routine for the PCA                                 */
/*                                                                                 */
/***********************************************************************************/

void PCA_Init(void)
{


    PCA0CN    = 0x40;
    PCA0MD    &= ~0x40;
    PCA0MD    = 0x05;		/* Use Timer 0 o/f as source, interrupt on overflow    */

    PCA0CPM0  = 0x31;       /* Capture input, on any transition - Spark            */
    PCA0CPM1  = 0x31;       /* Capture input, on any transition - Fuel             */
    PCA0CPM2  = 0x4D;       /* PWM output - Turbo control - High speed             */
    PCA0CPM3  = 0x43;       /* PWM output - 8 bit no int - VSO                     */
    PCA0CPM4  = 0x21;       /* Capture input, on rising edge - Vehicle speed       */


    PCA0L = 0x00;           /* Reset channel counter to zero                       */
    PCA0H = 0x00;
							/* First transition will take pin from high to low     */
							/* ISR will reload low and high time, but set defaults */
	HAL_BoostHighCount = HAL_BOOSTMINHIGH;
	HAL_BoostLowCount = HAL_BOOSTMAXLOW;
                            /* Set sensible timer values for PWM output            */

   	PCA0CPL2 = HAL_BOOSTPERIOD & 0x00FF; 
   	PCA0CPH2 = (HAL_BOOSTPERIOD & 0xFF00) / 256;
							/* Allow code entire cycle to calculate some data      */

}


/***********************************************************************************/
/*                                                                                 */
/* SPI_Init() - Initialisation routine for the serial peripheral interface         */
/*                                                                                 */
/***********************************************************************************/

void SPI_Init(void)
{

    SPI0CFG   = 0x40;

    SPI0CN    = 0x0D;                   // 4-wire Single Master, SPI enabled

    SPI0CKR   = 0xEF;
}


/***********************************************************************************/
/*                                                                                 */
/* Initialise() - Initialisation routine for micro                                 */
/*                                                                                 */
/***********************************************************************************/

void Initialize(void)
{
   PCA_Init();

   Timer_Init();                          // Initialize timer2

   Port_Init();                           // Initialize crossbar and GPIO

   SPI_Init();

   UART_Init();

   Adc_Init();                            // Initialize ADC
}


/***********************************************************************************/
/*                                                                                 */
/* Timer0() - ISR for the Timer0 input pin                                         */
/*                                                                                 */
/***********************************************************************************/

INTERRUPT(Timer0_ISR, INTERRUPT_TIMER0)
{


    HAL_T0Count++;

	TF0 = 0;
}


/***********************************************************************************/
/*                                                                                 */
/* Timer1ISR() - Controls delayed start/stop of injector channel                   */
/*                                                                                 */
/***********************************************************************************/

INTERRUPT(Timer1_ISR, INTERRUPT_TIMER1)
{
   if (HAL_InjAtInterrupt == 0x10)   /* Injector on at interrupt? */
   		{
			INJECTOR_ON;	
			HAL_InjAtInterrupt = 0x00;
		}
   else if (HAL_InjAtInterrupt == 0x20)  /* Injector off at interrupt? */
   		{
			INJECTOR_OFF;	
			HAL_InjAtInterrupt = 0x00;
		}
   else									 /* Necessary to check for aborted trans req. */
   		{
		}

   HAL_T1Count++;

   TR1 = 0;
   TF1 = 0;

   IE &= 0xF7;         	 /* Disable interrupt to core                              */

   TF1 = 0;

}


/***********************************************************************************/
/*                                                                                 */
/* RTI() - Real time interrupt - 1ms                                               */
/*                                                                                 */
/***********************************************************************************/

INTERRUPT(Timer2_ISR, INTERRUPT_TIMER2)
{


   TF2H = 0;                                  /* Clear Timer2 interrupt flag FIRST */


	if(++HAL_1msScheduler >= 4)
		{

		   lmms++;

			HAL_1msScheduler = 0;
    
			if (HAL_10msScheduler == 0)
				{
				    NSSMD0 = 0;
				}
			else if (HAL_10msScheduler == 1)
				{
					DACTransfer1 = (UINT8)(DACConv / 256);

					DACTransfer2 = (UINT8)(DACConv);
					
					SPI0DAT = DACTransfer1;
				}
			else if (HAL_10msScheduler == 2)
				{
					SPI0DAT = DACTransfer2;
				}
			else
				{
				    NSSMD0 = 1;
				}

			if (++HAL_10msScheduler >= 10)
				{
					HAL_Task10msStart += 1;
					HAL_10msScheduler = 0;
					if (++HAL_100msScheduler >= 10)
						{
							HAL_Task100msStart += 1;
							HAL_100msScheduler = 0;
							if (++HAL_1SScheduler >= 10)
								{
									HAL_1SScheduler = 0;
									Vars.HAL_Seconds++;
								}
							if (HAL_1SScheduler > 4)
								{
									Led1 = 0;
								}
							else
								{
									Led1 = 1;
								}
						}
				}
		}



	commshandler();

}


/***********************************************************************************/
/*                                                                                 */
/* Timer3() - ISR for the Timer3 input pin                                         */
/*                                                                                 */
/***********************************************************************************/


INTERRUPT(Timer3_ISR, INTERRUPT_TIMER3)
{

   if (HAL_SpkAtInterrupt == 0x30)
		{
			SPARK_ON;			/* Set spark output on */
			SPARK_DELAY_OFF(Vars.IGN_SparkHigh);    
								/* Schedule next interrupt on this channel to switch off  */
		}
   else if (HAL_SpkAtInterrupt == 0x10)
   		{
			SPARK_ON;	
			HAL_SpkAtInterrupt = 0x00;
		    EIE1 &= 0x7F;         /* Disable interrupt to core                              */
		}
   else
   		{
			SPARK_OFF;	
			HAL_SpkAtInterrupt = 0x00;
            EIE1 &= 0x7F;         /* Disable interrupt to core                              */
		}

   TMR3CN    &= 0x3F;  	  /* Disable interrupt/clear flags and timer until next use */
   HAL_T3Count++;


}


/***********************************************************************************/
/*                                                                                 */
/* PCA0_ISR - ISR for the PCA0 channels                                            */
/*                                                                                 */
/***********************************************************************************/

INTERRUPT(PCA0_ISR, INTERRUPT_PCA0)
{

	UINT16 TMRTemp;
	UINT16 PerTemp;


      if (CF)											/* Ensure that overflow handled first, regardless */
		{
			CF = 0;
			if (++HAL_EngineOverflow >= 2)
				{
					HAL_EngineOverflow = 2;		
					HAL_EnginePeriod = 0xFFFF;			/* Maximum period - zero speed 			  */
					Vars.DEBUG += 35;
				}
			if (++HAL_VehicleOverflow >= 2)
				{
					HAL_VehicleOverflow = 2;		
					HAL_VehiclePeriod = 0xFFFF;			/* Maximum period - zero speed 			  */
				}
		}


/************** CEX 0 - Ignition (Spark) Input Capture Channel ************************************/

      if (CCF0)											/* Is source channel PCA0 (Spark channel)?*/
		{
			CCF0 = 0;
		
		    TMRTemp = PCA0CP0;
														/* Take a local copy of the timer         */
			if (!(P0 & 0x10))							/* Was it a FALLING edge?                  */
				{

					HAL_SparkStart = TMRTemp;           /* Store start of spark event             */

					Vars.IGN_SparkPeriod = HAL_SparkStart - HAL_SparkStartPrev;

					HAL_SparkStartPrev = HAL_SparkStart;/* Calculate the period of spark          */

					if (++HAL_InjSync >= GlobalsRAM.IOP_NUMBEROFCYLS)
														/* Increment injector sync.               */
						{
							HAL_InjSync = 0;			/* Clear sync counter at 3                */

							HAL_InjThisCycle = 0;       /* Clear number of injections counter     */
						}
					

				    if ((HAL_EngineOverflow == 0) || ((HAL_EngineOverflow == 1) && (TMRTemp < HAL_EnginePrevious)))		
														/* Acceptable overflows?                  */
						{
							HAL_EnginePeriod = TMRTemp - HAL_EnginePrevious;
						}
					else
					    {
							HAL_EnginePeriod = 0xFFFF;  /* Zero period    						  */
							Vars.DEBUG += 25;
						}

			
					HAL_EngineOverflow = 0;				/* Clear overflow counter regardless      */
				
					HAL_EnginePrevious = TMRTemp;		/* Store previous counter                 */

					if (((Vars.IOP_OpMode == SPARKANDFUEL) || (Vars.IOP_OpMode == SPARKONLY)) && (HAL_IgnSync == 1))
						{
						    if (Vars.IGN_SparkTrigger > HAL_MINIMUMTIMERLATENCY)
								{
									SPARK_ON_OFF(Vars.IGN_SparkTrigger);					
														/* Turn spark on/off                      */							
								}
							else
								{						/* Delay too short to use - bypass        */
									SPARK_ON;
									SPARK_DELAY_OFF(Vars.IGN_SparkHigh);
								}
						}
					else
						{
							SPARK_ON;					/* Turn spark on immediately              */							
						}

                    ++HAL_FuelCounter;

					if ( ((GlobalsRAM.TESTFIXES == 2) && (HAL_FuelCounter >= (GlobalsRAM.IOP_NUMBEROFCYLS + 2))) ||
						((GlobalsRAM.TESTFIXES < 2) && (HAL_FuelCounter >= (GlobalsRAM.IOP_NUMBEROFCYLS + 1))) )
														/* Edge on the fuel channel overdue       */
						{
							if (P0 & 0x20)				/* Is fuel state high (still injecting)   */
								{
										
									Vars.DEBUG += 1;

									Vars.FUEL_BaseTime = HAL_EnginePeriod720;
														  /* Calculated 100% duty time            */
									HAL_InjThisCycle = 2; /* Must get full pulse to see real fuel */

									if (((Vars.IOP_OpMode == SPARKANDFUEL) || (Vars.IOP_OpMode == FUELONLY)) && (Vars.FUEL_CorrectTime != 0xFFFF))
										{
											INJECTOR_ON;				/* Immediately turn injector on           */

											INJECTOR_DELAY_OFF(Vars.FUEL_CorrectTime);							
																		/* Turn injector off after delay          */				
										}
									else
										{
											INJECTOR_ON;				/* Immediately turn injector on           */
										}

									HAL_FuelCounter = 1;
														/* Counter 1 - 1 rev of CAM until next test */
								}
							else						/* Fuel not injecting-clear count and wait*/
								{
									Vars.DEBUG += 16;

									HAL_FuelCounter = 0;
														/* Clear fuel counter                     */
									Vars.FUEL_BaseTime = 0;
									                    /* Clear base time                        */
								}
						}
				}
			else										/* It was a RISING edge                   */
				{

#ifdef REVENG
					HAL_IgnitionDelay = PCA0 - HAL_DistribTime;
#endif

					HAL_SparkEnd = TMRTemp;             /* Store end of spark event               */

					Vars.IGN_SparkHigh = (HAL_SparkEnd - HAL_SparkStart);

					if (Vars.IGN_SparkHigh > SPARKMAXBEFOREFACT)
						{
							Vars.IGN_SparkHigh = 0xFFF0;  
						}
					else
						{
							Vars.IGN_SparkHigh *= SPARKFACTOR;
						}

					if ((Vars.IOP_OpMode == SPARKANDFUEL) || (Vars.IOP_OpMode == SPARKONLY))
						{
							if (HAL_IgnSync == 0)
								{
									SPARK_OFF;	        /* Turn spark off immediately             */							
									HAL_IgnSync = 1;    /* 1 complete cycle in correct mode done  */
								}
						}
					else
						{
							SPARK_OFF;	                /* Turn spark off immediately             */							
						}
				}
		}


/************** CEX 1 - Injection (Fuel) Input Capture Channel ************************************/
      if (CCF1)
		{
			CCF1 = 0;									/* Is source channel PCA1 (Fuel Channel)? */

		    TMRTemp = PCA0CP1; 							/* Get edge time                          */

			if (P0 & 0x20)								/* Was it a RISING edge?                  */
				{
					HAL_FuelStart = TMRTemp;            /* Log start of injection time            */

					if ((HAL_InjThisCycle < 2) && (Vars.FUEL_CorrectTime != 0xFFFF) && ((Vars.IOP_OpMode == SPARKANDFUEL) || (Vars.IOP_OpMode == FUELONLY)))
						{
							INJECTOR_ON;				/* Immediately turn injector on           */
																						
							if (FUEL_OverallFact >= 100)/* Increasing width?                  */
								{
									FUEL_IncreaseISR = FUEL_IncreaseFact;

									HAL_FuelExtendISR = HAL_FuelExtend;

														/* IS increase, data to shut off          */											
								}
							else						/* Decrease width                         */
								{
									INJECTOR_DELAY_OFF(Vars.FUEL_CorrectTime);							
														/* Turn injector off after delay          */
									FUEL_IncreaseISR = 0xFF;
														/* Don't handle extend time               */
								}
						}
					else
						{
							INJECTOR_ON;				/* Immediately turn injector on           */
							FUEL_IncreaseISR = 0xFF;
							HAL_FuelExtendISR = 0;
														/* Don't handle extend time               */
						}
					HAL_FuelCounter = 0;				/* Reset the duty limit counter for fuel  */

					if (++HAL_InjThisCycle == 1)        /* Increment the number of inj. starts    */
						{
							HAL_InjSync = 0;            /* Reset injector sync counter            */
						}
				}
			else										/* It was a FALLING edge                  */
				{
					if (GlobalsRAM.TESTFIXES == 1)
						{
							HAL_FuelCounter = 0;		/* Reset the duty limit counter for fuel  */
						}
					HAL_FuelEnd = TMRTemp;				/* Log end of injection time              */

					if (HAL_InjThisCycle <= 1)          /* Calculate base time ONLY on main pulse */
						{
							Vars.FUEL_BaseTime = HAL_FuelEnd - HAL_FuelStart;
														/* In case of 100 - 99 - 100 type pulses  */
														/* watch for over time. limit             */
							if (Vars.FUEL_BaseTime > HAL_EnginePeriod720)
								{
									Vars.FUEL_BaseTime = HAL_EnginePeriod720;
								}
						}
														/* Log injection duration (x 2us)         */						
					if ((Vars.IOP_OpMode != SPARKANDFUEL) && (Vars.IOP_OpMode != FUELONLY))
						{								/* Mirror mode - injection mirror only    */
							INJECTOR_OFF;				/* Immediately turn injector off          */
						}
					else if (FUEL_IncreaseISR != 0xFF)  /* Check for extend type                  */
						{

							  if ((FUEL_IncreaseISR > 0) && (HAL_FuelExtendISR != 0))
							  		{
	 									INJECTOR_DELAY_OFF(HAL_FuelExtendISR);							
										  				/* Set delayed shut off of injector       */
									}
							  else
							  		{
			 							INJECTOR_OFF;	/* Immediately turn injector off          */
									}
						}
					else if (HAL_InjThisCycle >= 2)		/* Mirror the runt pulses                 */					
						{
  							  INJECTOR_OFF;				/* Immediately turn injector off          */
						}
					else								/* Reducing, handled by start of inject   */
						{
						}
					FUEL_IncreaseISR = 0xFF;
				}
		}

/************** CEX 2 - Boost control solenoid PWM update *****************************************/
      if (CCF2)
		{
			CCF2 = 0;									/* Is source channel PCA2 (Boost Channel)? */

	      	TMRTemp = PCA0CP2;

			if (P0 & 0x40)								/* Did the last toggle put pin high (low time) */
				{
		    	  	PCA0CPL2 = (HAL_BoostLowTrans & 0x00FF);
														/* Copy low - high duration transition time in */
	      			PCA0CPH2 = (HAL_BoostLowTrans & 0xFF00) / 256;
														/* Next low to high will calculate timers      */
				}
			else
				{
														/* Just had a low to high transition?          */
					TMRTemp += HAL_BoostHighCount;		/* Calculate high duration                     */

		    	  	PCA0CPL2 = (TMRTemp & 0x00FF);      /* Copy high time into the output              */

	      			PCA0CPH2 = (TMRTemp & 0xFF00) / 256;
					
					HAL_BoostLowTrans = TMRTemp + HAL_BoostLowCount; 
														/* Calculate full data for this PWM cycle now  */
				}

		}

/************** CEX 3 - No function  **************************************************************/
	  if (CCF3)
	  	{
			CCF3 = 0;
		}

/************** CEX 4 - Vehicle Speed Input Capture Channel ***************************************/
      if (CCF4)
		{
			CCF4 = 0;									/* Is source channel PCA4 (Veh. Spd. Channel)? */

#ifdef REVENG
			HAL_DistribTime = PCA0;
#endif
					FAKEVSON;					
				    TMRTemp = PCA0CP4;
														/* Take a local copy of the timer        		*/
				    if ((HAL_VehicleOverflow == 0) || ((HAL_VehicleOverflow == 1) && (TMRTemp < HAL_VehiclePrevious)))		
														/* Acceptable overflows?                        */
						{
						    PerTemp = TMRTemp - HAL_VehiclePrevious;
						}
					else
						{
							PerTemp = 0xFFFF;
						}
														/* Check for glitches, and disregard            */
					if (PerTemp <= HAL_VEHICLEMINPERIOD)
						{	
							/* Assume that last was a glitch, and carry on accumulating                 */
						}
					else
						{
							HAL_VehicleOverflow = 0;	/* Clear overflow counter regardless            */

							HAL_VehiclePeriod = PerTemp;/* Copy checked period in                       */
														
							HAL_VehiclePrevious = TMRTemp;
														/* Store previous counter                       */
						}

				    FAKEVSOFF;
		}
		

    HAL_PCA0Count++;
		
}



/***********************************************************************************/
/*                                                                                 */
/* IGN_TableLookup() - 3D lookup table access for ignition                         */
/*                                                                                 */
/***********************************************************************************/

SINT8 IGN_TableLookup(UINT8 rpm, UINT8 map, UINT8 mapselect)  
	{


	SEG_XDATA UINT8 interp1;
	SEG_XDATA UINT8 interp2;
	SEG_XDATA UINT8 factx1y1,factx2y1,factx1y2,factx2y2;
	SEG_XDATA SINT16 cornerx1y1,cornerx2y1,cornerx1y2,cornerx2y2;
	SEG_XDATA UINT16 interp3;
	SEG_XDATA UINT8 mapindex1,rpmindex1;
	SEG_XDATA UINT8 mapindex2,rpmindex2;
	SEG_XDATA IgnMaps *  mapptr;

	if (mapselect)
		{
			mapptr = &Ign2RAM;
		}
	else
		{
			mapptr = &Ign1RAM;
		}

		if(map >= mapptr->IGN_MAPTABLE[SPARK_MAP_PTS-1])
			{
				map = mapptr->IGN_MAPTABLE[SPARK_MAP_PTS-1];
				mapindex1 = SPARK_MAP_PTS-1;
				mapindex2 = SPARK_MAP_PTS-1;
				interp1 = 0;
			}
		else if(map <= mapptr->IGN_MAPTABLE[0])
			{
				map = mapptr->IGN_MAPTABLE[0];
				mapindex1 = 0;
				mapindex2 = 0;
				interp1 = 0;
			}
		else
			{
				mapindex2 = 0;
				do
					{
						mapindex2++;
					}
				while ((map > mapptr->IGN_MAPTABLE[mapindex2])&&(mapindex2 < SPARK_MAP_PTS-1));

				mapindex1 = mapindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->IGN_MAPTABLE[mapindex2] > mapptr->IGN_MAPTABLE[mapindex1])
					{
						interp1 = mapptr->IGN_MAPTABLE[mapindex2] - mapptr->IGN_MAPTABLE[mapindex1];
						interp3 = (map - mapptr->IGN_MAPTABLE[mapindex1]) * 8; 
						interp1 = interp3 / interp1; 
					}
				else
					{
						interp1 = 0;
					}
			}
								
		if(rpm > mapptr->IGN_RPMTABLE[SPARK_RPM_PTS-1])
			{
				rpm = mapptr->IGN_RPMTABLE[SPARK_RPM_PTS-1];
				rpmindex1 = SPARK_RPM_PTS-1;
				rpmindex2 = SPARK_RPM_PTS-1;
				interp2 = 0;
			}
		else if(rpm < mapptr->IGN_RPMTABLE[0])
			{
				rpm = mapptr->IGN_RPMTABLE[0];
				rpmindex1 = 0;
				rpmindex2 = 0;
				interp2 = 0;
			}
		else
			{
				rpmindex2 = 0;
				do
					{
						rpmindex2++;
					}
				while ((rpm > mapptr->IGN_RPMTABLE[rpmindex2])&&(rpmindex2 < SPARK_RPM_PTS-1));

				rpmindex1 = rpmindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->IGN_RPMTABLE[rpmindex2] > mapptr->IGN_RPMTABLE[rpmindex1])
					{
						interp2 = mapptr->IGN_RPMTABLE[rpmindex2] - mapptr->IGN_RPMTABLE[rpmindex1];
						interp3 = (rpm - mapptr->IGN_RPMTABLE[rpmindex1]) * 8; 
						interp2 = interp3 / interp2; 
					}
				else
					{
						interp2 = 0;				
					}
			}

		factx1y1 = (8 - interp1) * (8 - interp2);
		factx2y1 = (interp1) * (8 - interp2);
		factx1y2 = (8 - interp1) * (interp2);
		factx2y2 = (interp1) * (interp2);
	    cornerx1y1 =  (mapptr->IGN_IGNOFFTABLE[mapindex1][rpmindex1] * factx1y1);
	    cornerx2y1 =  (mapptr->IGN_IGNOFFTABLE[mapindex2][rpmindex1] * factx2y1);
	    cornerx1y2 =  (mapptr->IGN_IGNOFFTABLE[mapindex1][rpmindex2] * factx1y2);
	    cornerx2y2 =  (mapptr->IGN_IGNOFFTABLE[mapindex2][rpmindex2] * factx2y2);

		return((SINT8)((cornerx1y1 + cornerx2y1 + cornerx1y2 + cornerx2y2) / 64));
}


/***********************************************************************************/
/*                                                                                 */
/* FUEL_TableLookup() - 3D lookup table access for fuel                            */
/*                                                                                 */
/***********************************************************************************/



UINT8 FUEL_TableLookup(UINT8 rpm, UINT8 map, UINT8 mapselect)  
	{
		SEG_XDATA UINT8 mapindex1,rpmindex1;
		SEG_XDATA UINT8 mapindex2,rpmindex2;
		SEG_XDATA UINT8 factx1y1,factx2y1,factx1y2,factx2y2;
		SEG_XDATA UINT16 cornerx1y1,cornerx2y1,cornerx1y2,cornerx2y2;
		SEG_XDATA UINT8 interp1;
		SEG_XDATA UINT8 interp2;
		SEG_XDATA UINT16 interp3;
		SEG_XDATA FuelMaps *  mapptr;

		if (mapselect)
			{
				mapptr = &Fuel2RAM;
			}
		else
			{
				mapptr = &Fuel1RAM;
			}

		if(map >= mapptr->FUEL_MAPTABLE[FUEL_MAP_PTS-1])
			{
				map = mapptr->FUEL_MAPTABLE[FUEL_MAP_PTS-1];
				mapindex1 = FUEL_MAP_PTS-1;
				mapindex2 = FUEL_MAP_PTS-1;
				interp1 = 0;
			}
		else if(map <= mapptr->FUEL_MAPTABLE[0])
			{
				map = mapptr->FUEL_MAPTABLE[0];
				mapindex1 = 0;
				mapindex2 = 0;
				interp1 = 0;
			}
		else
			{
				mapindex2 = 0;
				do
					{
						mapindex2++;
					}
				while ((map > mapptr->FUEL_MAPTABLE[mapindex2])&&(mapindex2 < FUEL_MAP_PTS-1));

				mapindex1 = mapindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->FUEL_MAPTABLE[mapindex2] > mapptr->FUEL_MAPTABLE[mapindex1])
					{
						interp1 = mapptr->FUEL_MAPTABLE[mapindex2] - mapptr->FUEL_MAPTABLE[mapindex1];
						interp3 = (map - mapptr->FUEL_MAPTABLE[mapindex1]) * 8; 
						interp1 = interp3 / interp1; 
					}
				else
					{
						interp1 = 0;
					}
			}
								
		if(rpm > mapptr->FUEL_RPMTABLE[FUEL_RPM_PTS-1])
			{
				rpm = mapptr->FUEL_RPMTABLE[FUEL_RPM_PTS-1];
				rpmindex1 = FUEL_RPM_PTS-1;
				rpmindex2 = FUEL_RPM_PTS-1;
				interp2 = 0;
			}
		else if(rpm < mapptr->FUEL_RPMTABLE[0])
			{
				rpm = mapptr->FUEL_RPMTABLE[0];
				rpmindex1 = 0;
				rpmindex2 = 0;
				interp2 = 0;
			}
		else
			{
				rpmindex2 = 0;
				do
					{
						rpmindex2++;
					}
				while ((rpm > mapptr->FUEL_RPMTABLE[rpmindex2])&&(rpmindex2 < FUEL_RPM_PTS-1));

				rpmindex1 = rpmindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->FUEL_RPMTABLE[rpmindex2] > mapptr->FUEL_RPMTABLE[rpmindex1])
					{
						interp2 = mapptr->FUEL_RPMTABLE[rpmindex2] - mapptr->FUEL_RPMTABLE[rpmindex1];
						interp3 = (rpm - mapptr->FUEL_RPMTABLE[rpmindex1]) * 8; 
						interp2 = interp3 / interp2; 
					}
				else
					{
						interp2 = 0;				
					}
			}


		factx1y1 = (8 - interp1) * (8 - interp2);
		factx2y1 = (interp1) * (8 - interp2);
		factx1y2 = (8 - interp1) * (interp2);
		factx2y2 = (interp1) * (interp2);
	    cornerx1y1 =  (mapptr->FUEL_PRCNTCORRTABLE[mapindex1][rpmindex1] * factx1y1);
	    cornerx2y1 =  (mapptr->FUEL_PRCNTCORRTABLE[mapindex2][rpmindex1] * factx2y1);
	    cornerx1y2 =  (mapptr->FUEL_PRCNTCORRTABLE[mapindex1][rpmindex2] * factx1y2);
	    cornerx2y2 =  (mapptr->FUEL_PRCNTCORRTABLE[mapindex2][rpmindex2] * factx2y2);

		return((UINT8)((cornerx1y1 + cornerx2y1 + cornerx1y2 + cornerx2y2) / 64));
                 
}


/***********************************************************************************/
/*                                                                                 */
/* TURBO_TableLookup() - 3D lookup table access for turbo target duty              */
/*                                                                                 */
/***********************************************************************************/

UINT8 TURBO_DutyTableLookup(UINT8 rpm, UINT8 tps, UINT8 mapselect)  
	{
		SEG_XDATA UINT8 tpsindex1,rpmindex1;
		SEG_XDATA UINT8 tpsindex2,rpmindex2;
		SEG_XDATA UINT8 factx1y1,factx2y1,factx1y2,factx2y2;
		SEG_XDATA UINT16 cornerx1y1,cornerx2y1,cornerx1y2,cornerx2y2;
		SEG_XDATA UINT8 interp1;
		SEG_XDATA UINT8 interp2;
		SEG_XDATA UINT16 interp3;
		SEG_XDATA BoostMaps *  mapptr;


		if (mapselect)
			{
				mapptr = &Boost2RAM;
			}
		else
			{
				mapptr = &Boost1RAM;
			}

		if(tps > mapptr->TURBO_DUTYTPSTABLE[TURBO_DUTY_TPS_PTS-1])
			{
				tps = mapptr->TURBO_DUTYTPSTABLE[TURBO_DUTY_TPS_PTS-1];
				tpsindex1 = TURBO_DUTY_TPS_PTS-1;
				tpsindex2 = TURBO_DUTY_TPS_PTS-1;
				interp1 = 0;
			}
		else if(tps < mapptr->TURBO_DUTYTPSTABLE[0])
			{
				tps = mapptr->TURBO_DUTYTPSTABLE[0];
				tpsindex1 = 0;
				tpsindex2 = 0;
				interp1 = 0;
			}
		else
			{
				tpsindex2 = 0;
				do
					{
						tpsindex2++;
					}
				while ((tps > mapptr->TURBO_DUTYTPSTABLE[tpsindex2])&&(tpsindex2 < TURBO_DUTY_TPS_PTS-1));

				tpsindex1 = tpsindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->TURBO_DUTYTPSTABLE[tpsindex2] > mapptr->TURBO_DUTYTPSTABLE[tpsindex1])
					{
						interp1 = mapptr->TURBO_DUTYTPSTABLE[tpsindex2] - mapptr->TURBO_DUTYTPSTABLE[tpsindex1];
						interp3 = (tps - mapptr->TURBO_DUTYTPSTABLE[tpsindex1]) * 8; 
						interp1 = interp3 / interp1; 
					}
				else
					{
						interp1 = 0;				
					}
			}

		if(rpm > mapptr->TURBO_DUTYRPMTABLE[TURBO_DUTY_RPM_PTS-1])
			{
				rpm = mapptr->TURBO_DUTYRPMTABLE[TURBO_DUTY_RPM_PTS-1];
				rpmindex1 = TURBO_DUTY_RPM_PTS-1;
				rpmindex2 = TURBO_DUTY_RPM_PTS-1;
				interp2 = 0;
			}
		else if(rpm <= mapptr->TURBO_DUTYRPMTABLE[0])
			{
				rpm = mapptr->TURBO_DUTYRPMTABLE[0];
				rpmindex1 = 0;
				rpmindex2 = 0;
				interp2 = 0;
			}
		else
			{
				rpmindex2 = 0;
				do
					{
						rpmindex2++;
					}
				while ((rpm > mapptr->TURBO_DUTYRPMTABLE[rpmindex2])&&(rpmindex2 < TURBO_DUTY_RPM_PTS-1));

				rpmindex1 = rpmindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->TURBO_DUTYRPMTABLE[rpmindex2] > mapptr->TURBO_DUTYRPMTABLE[rpmindex1])
					{
						interp2 = mapptr->TURBO_DUTYRPMTABLE[rpmindex2] - mapptr->TURBO_DUTYRPMTABLE[rpmindex1];
						interp3 = (rpm - mapptr->TURBO_DUTYRPMTABLE[rpmindex1]) * 8; 
						interp2 = interp3 / interp2; 
					}
				else
					{
						interp2 = 0;
					}
			}


		factx1y1 = (8 - interp1) * (8 - interp2);
		factx2y1 = (interp1) * (8 - interp2);
		factx1y2 = (8 - interp1) * (interp2);
		factx2y2 = (interp1) * (interp2);

	    cornerx1y1 =  ((UINT16)mapptr->TURBO_DUTYTABLE[tpsindex1][rpmindex1] * factx1y1);
	    cornerx2y1 =  ((UINT16)mapptr->TURBO_DUTYTABLE[tpsindex2][rpmindex1] * factx2y1);
	    cornerx1y2 =  ((UINT16)mapptr->TURBO_DUTYTABLE[tpsindex1][rpmindex2] * factx1y2);
	    cornerx2y2 =  ((UINT16)mapptr->TURBO_DUTYTABLE[tpsindex2][rpmindex2] * factx2y2);

		return((UINT8)((cornerx1y1 + cornerx2y1 + cornerx1y2 + cornerx2y2) / 64));
}



/***********************************************************************************/
/*                                                                                 */
/* TURBO_TableLookup() - 3D lookup table access for turbo target press             */
/*                                                                                 */
/***********************************************************************************/

UINT8 TURBO_TargetTableLookup(UINT8 rpm, UINT8 tps, UINT8 mapselect)  
	{

		SEG_XDATA UINT8 tpsindex1,rpmindex1;
		SEG_XDATA UINT8 tpsindex2,rpmindex2;
		SEG_XDATA UINT8 factx1y1,factx2y1,factx1y2,factx2y2;
		SEG_XDATA UINT16 cornerx1y1,cornerx2y1,cornerx1y2,cornerx2y2;
		SEG_XDATA UINT8 interp1;
		SEG_XDATA UINT8 interp2;
		SEG_XDATA UINT16 interp3;
		SEG_XDATA BoostMaps *  mapptr;


		if (mapselect)
			{
				mapptr = &Boost2RAM;
			}
		else
			{
				mapptr = &Boost1RAM;
			}

		if(tps > mapptr->TURBO_TARGTPSTABLE[TURBO_TARG_TPS_PTS-1])
			{
				tps = mapptr->TURBO_TARGTPSTABLE[TURBO_TARG_TPS_PTS-1];
				tpsindex1 = TURBO_TARG_TPS_PTS-1;
				tpsindex2 = TURBO_TARG_TPS_PTS-1;
				interp1 = 0;
			}
		else if(tps < mapptr->TURBO_TARGTPSTABLE[0])
			{
				tps = mapptr->TURBO_TARGTPSTABLE[0];
				tpsindex1 = 0;
				tpsindex2 = 0;
				interp1 = 0;
			}
		else
			{
				tpsindex2 = 0;
				do
					{
						tpsindex2++;
					}
				while ((tps > mapptr->TURBO_TARGTPSTABLE[tpsindex2])&&(tpsindex2 < TURBO_TARG_TPS_PTS-1));

				tpsindex1 = tpsindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->TURBO_TARGTPSTABLE[tpsindex2] > mapptr->TURBO_TARGTPSTABLE[tpsindex1])
					{
						interp1 = mapptr->TURBO_TARGTPSTABLE[tpsindex2] - mapptr->TURBO_TARGTPSTABLE[tpsindex1];
						interp3 = (tps - mapptr->TURBO_TARGTPSTABLE[tpsindex1]) * 8; 
						interp1 = interp3 / interp1; 
					}
				else
					{
						interp1 = 0;				
					}
			}

		if(rpm > mapptr->TURBO_TARGRPMTABLE[TURBO_TARG_RPM_PTS-1])
			{
				rpm = mapptr->TURBO_TARGRPMTABLE[TURBO_TARG_RPM_PTS-1];
				rpmindex1 = TURBO_TARG_RPM_PTS-1;
				rpmindex2 = TURBO_TARG_RPM_PTS-1;
				interp2 = 0;
			}
		else if(rpm <= mapptr->TURBO_TARGRPMTABLE[0])
			{
				rpm = mapptr->TURBO_TARGRPMTABLE[0];
				rpmindex1 = 0;
				rpmindex2 = 0;
				interp2 = 0;
			}
		else
			{
				rpmindex2 = 0;
				do
					{
						rpmindex2++;
					}
				while ((rpm > mapptr->TURBO_TARGRPMTABLE[rpmindex2])&&(rpmindex2 < TURBO_TARG_RPM_PTS-1));

				rpmindex1 = rpmindex2 - 1;

				/* Allow only positive progression through axis points */
		        if (mapptr->TURBO_TARGRPMTABLE[rpmindex2] > mapptr->TURBO_TARGRPMTABLE[rpmindex1])
					{
						interp2 = mapptr->TURBO_TARGRPMTABLE[rpmindex2] - mapptr->TURBO_TARGRPMTABLE[rpmindex1];
						interp3 = (rpm - mapptr->TURBO_TARGRPMTABLE[rpmindex1]) * 8; 
						interp2 = interp3 / interp2; 
					}
				else
					{
						interp2 = 0;
					}
			}

		factx1y1 = (8 - interp1) * (8 - interp2);
		factx2y1 = (interp1) * (8 - interp2);
		factx1y2 = (8 - interp1) * (interp2);
		factx2y2 = (interp1) * (interp2);

	    cornerx1y1 =  ((UINT16)factx1y1 * mapptr->TURBO_TARGTABLE[tpsindex1][rpmindex1]);
	    cornerx2y1 =  ((UINT16)factx2y1 * mapptr->TURBO_TARGTABLE[tpsindex2][rpmindex1]);
	    cornerx1y2 =  ((UINT16)factx1y2 * mapptr->TURBO_TARGTABLE[tpsindex1][rpmindex2]) ;
	    cornerx2y2 =  ((UINT16)factx2y2 * mapptr->TURBO_TARGTABLE[tpsindex2][rpmindex2]) ;

		return((UINT8)((cornerx1y1 + cornerx2y1 + cornerx1y2 + cornerx2y2) / 64));
}


/***********************************************************************************/
/*                                                                                 */
/* HAL_O2Lookup() - Lookup of O2 sensor (raw to physical)                          */
/*                                                                                 */
/***********************************************************************************/

UINT8 HAL_O2Lookup(UINT8 Raw)
{
	
		SEG_XDATA UINT8 retval;
		SEG_XDATA UINT8 cnt;		
		SEG_XDATA UINT8 xoff;
		SEG_XDATA UINT8 xoffminus;
		SEG_XDATA UINT8 xstep;


		if(Raw >= GlobalsRAM.HAL_O2RAWTABLE[HAL_O2_PTS - 1])
			{				/* Check for constant extrapolation at max top */
				retval = GlobalsRAM.HAL_O2VALTABLE[HAL_O2_PTS - 1];
			}
		else if(Raw <= GlobalsRAM.HAL_O2RAWTABLE[0])
			{			    /* Check for constant extrapolation at min bottom */
				retval = GlobalsRAM.HAL_O2VALTABLE[0];
			}
		else				/* Value in table - interpolate */
			{
				cnt = 1;

				do			/* Search until Raw is no longer greater than x axis[cnt] point */
					{
					}
				while ((Raw > GlobalsRAM.HAL_O2RAWTABLE[cnt]) && (++cnt < HAL_O2_PTS));
				
				if (GlobalsRAM.HAL_O2RAWTABLE[cnt] > GlobalsRAM.HAL_O2RAWTABLE[cnt - 1])
					{		/* Monotony check 												*/
						xoff = Raw - GlobalsRAM.HAL_O2RAWTABLE[cnt - 1];
							/* Get lower point data 										*/
						xstep = GlobalsRAM.HAL_O2RAWTABLE[cnt] - GlobalsRAM.HAL_O2RAWTABLE[cnt - 1];
							/* Get point to point step 										*/
						xoffminus = xstep - xoff;
							/* Get distance to next point 									*/
						retval = (UINT8)(((UINT16)xoff * GlobalsRAM.HAL_O2VALTABLE[cnt]) / xstep);
							/* Calculate upper point contribution                           */
						retval += (UINT8)(((UINT16)xoffminus * GlobalsRAM.HAL_O2VALTABLE[cnt - 1]) / xstep);
							/* Sum with lower point contribution                            */
					}
				else		/* Bad monotony - return limit value */
					{
						retval = GlobalsRAM.HAL_O2VALTABLE[cnt];
					}
			}


	return (retval);
}



/***********************************************************************************/
/*                                                                                 */
/* Adc_ConvComplete_ISR() - Conversion complete interrupt for ADC control          */
/*                                                                                 */
/***********************************************************************************/

INTERRUPT(Adc_ConvComplete_ISR, INTERRUPT_ADC0_EOC)
{

UINT8 * pChannel;

	pChannel = &Vars.HAL_ANA1; 		     /* Point to first raw in group        */

	pChannel[HAL_ADCChanSel] = ADC0H;

    if (++HAL_ADCChanSel >= HAL_ADCNUMCHAN)
		{
			HAL_ADCChanSel = 0;
			HAL_ADCLoops++;
		}

    AMX0P = HAL_ADCChan[HAL_ADCChanSel];   /* Load next channel in queue to read */

    AD0INT = 0;


}



/***********************************************************************************/
/*                                                                                 */
/* SCI Communications                                                              */
/*                                                                                 */
/* Communications is established when the PC communications program sends          */
/* a command character - the particular character sets the mode:                   */
/*                                                                                 */
/* "a" = send all of the realtime display variables (Vars structure) via txport.   */
/* "w"+<offset lsb>+<offset msb>+<nobytes>+<newbytes> =                            */
/*    receive updated data parameter(s) and write into offset location             */
/*    relative to start of data block                                              */
/* "e" = same as "w" above, followed by "r" below to echo back value               */
/* "r"+<offset lsb>+<offset msb>+<nobytes>+<newbytes> = read and                   */
/*    send back value of a data parameter or block in offset location              */
/* "y" = verify Maps1RAM data block = inpflash data block, return no. bytes different*/
/* "b" = jump to flash burner routine and burn a ram data block into a flash       */
/*    data block.                                                                  */
/* "t" = receive new data for clt/mat/ego/maf tables                               */
/* "T" = receive new table data for CAN re-transmission to GPIO                    */
/* "c" = Test communications - echo back Seconds                                   */
/* "Q" = Send over Embedded Code Revision Number                                   */
/* "S" = Send program title.                                                       */
/*                                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/*                                                                                 */
/* UART1_ISR() - ISR for the UART1 interrupt                                       */
/*                                                                                 */
/***********************************************************************************/


void commshandler(void)
{

#define MAINCOMMAND		0
#define SENDRTDATA		1
#define SENDVERIFY		2
#define SENDECHO		3
#define SENDREV			4
#define SENDSIGN		5
#define GETDATAFORRAM1	5
#define GETDATAFORRAM2	6
#define GETDATAFORRAM3	7
#define GETDATAFORRAM4	8
#define GETDATAFORRAM5	9
#define GETBOOTLOADER1	30
#define GETBOOTLOADER2	31
#define GETTABLEID1    	40
#define GETTABLEID2     41
#define SETBURNPARAMS 	99


  static SINT16 rd_wr,xcntr;
  static UINT8 ibuf,sendCANdat=0,next_txmode,cksum,Tcntr;
  static UINT16 txptr,tble_word,ntword;
  static UINT8 shrtbuff[2];
  UINT16 index;
  UINT8 ReceiveLocal;
  UINT8 temptx;
  
if (txgoal == 0)						/* Finished transmitting? */
{
	if(SCON1 & 0x01)
		{
			ReceiveLocal = SBUF1;				/* Take a local copy of the data */
		
			SCON1 &= ~0x01;						/* Clear receive interrupt flag  */
		
			switch(txmode)  
				{
					case MAINCOMMAND:
						{
						  switch(ReceiveLocal)  
					  			{
			 		     			  
								    case 'a':	/* Send fixed DAQ frame to PC */
								    case 'A':	
									  	tble_idx = RT_TABLE_INDX;			
									  	cksum = 0;
						     	 	  	txgoal = DatatableBytes(tble_idx);
									  	txmode = SENDRTDATA;
						     	 	  	xcntr = 0;
						     	 	  	txptr = 0;
										shrtbuff[0] = *DatatableByteRam(tble_idx, 0);
										shrtbuff[1] = *DatatableByteRam(tble_idx, 1);
						     	 	  	TRANSMITBYTE(shrtbuff[0]);
						     	 	  	cksum += ReceiveLocal; 
									  	txcnt = 1;
									  break;
		
								    case 'w':	/* receive new ram input data and write into offset location */
								    case 'W':		  
									  	next_txmode = GETDATAFORRAM1;
									  	txmode = GETTABLEID1;
									  	rd_wr = 1;
									  break;
				
								    case 'e':		  /* same as 'w', but verify by echoing back values. */
								    case 'E':		  
									  	next_txmode = GETDATAFORRAM1;
									  	txmode = GETTABLEID1;
									  	rd_wr = 2;
									  break;
				
								    case 'r':		  /* read and send back ram input data from offset location; */
								    case 'R':		                  
									  	next_txmode = GETDATAFORRAM1;
							  		  	txmode = GETTABLEID1;
							  		  	rd_wr = 0;
							  		  	cksum = 0;
							  		break;
				   			
								    case 'X':        // start receiving reinit/reboot command
								    case 'x':        // start receiving reinit/reboot command
									  	txmode = GETBOOTLOADER1;
									break;
		
						    		case 'c':        // send back seconds to test comms
						    		case 'C':        // send back seconds to test comms
							  		  	txcnt = 0;         
							  		  	txmode = SENDRTDATA;
							  		  	txgoal = 1;        // seconds is 1st 2 bytes of Vars structure
							  		  	TRANSMITBYTE(*(UINT8 *)&Vars);
							  		break;

						    		case 'b':        // burn a block of ram input values into flash;
						    		case 'B':        
							  		  	next_txmode = SETBURNPARAMS;
							  		  	txmode      = GETTABLEID1;
							  		break;
		
						    		case 'q':         // send code rev no.
						    		case 'Q':         // send code rev no.
							  		  	txcnt = 0;
							  		  	txmode = SENDREV;
									  	txgoal = REVNUMLENGTH - 1;
							  		  	TRANSMITBYTE(XenoRevNum[0]); 
							  		break;

						    		case 's':         // send program title
						    		case 'S':         // send program title
							  		  	txcnt = 0;
							  			txmode = SENDSIGN;
							  			txgoal = SIGLENGTH - 1;
							  			TRANSMITBYTE(XenoSign[0]); 
							  		break;
		  
						    		case 'y':      // Verify that a flash data block matches a
						    		case 'Y':      // Verify that a flash data block matches a
							  			next_txmode = SENDVERIFY; //  corresponding ram data block
							  			txmode = GETTABLEID1;
							  		break;

						    		case 'k':         // request checksum.
						    		case 'K':         // request checksum.
							  			txcnt = 0;
							  			txmode = MAINCOMMAND;
							  			txgoal = 1;
							  			TRANSMITBYTE(cksum); 
							  		break;
		
								    default:
										TRANSMITBYTE(cksum); 
							  		break;
				  				}     // End of switch for received command
				  			}
			    		break;

						case GETTABLEID1: 
						   if (ReceiveLocal < 16) /* Correctly formatted pre-fix? */
						   		{
								     txmode = GETTABLEID2;
								}
					   	break;


						case GETTABLEID2: 
			   				tble_idx = ReceiveLocal;

							if(tble_idx >= NO_PARTBLES)  
						   		{
								     txmode = MAINCOMMAND;
								}
					   		else
					   			{
						     		 txmode = next_txmode;
								}
			                next_txmode = MAINCOMMAND;

					   		if (txmode == SETBURNPARAMS) 
					   			{ // Burn command
									if (Vars.HAL_Burn == 0)
										{
											Vars.HAL_Burn = 1;
										}
							     	txmode = MAINCOMMAND;	// handle burning in background
					   			} 
					   		else if(txmode == SENDVERIFY)  
								{
					     			vfy_flg = 1;
					   	 			vfy_fail = 0;
					   			}
						break;

						case GETDATAFORRAM1:
		  					rxoffset = ReceiveLocal;    // byte offset(lsb) from start of Maps1RAM
		  					txmode = GETDATAFORRAM2;
		  				break;
		
						case GETDATAFORRAM2:
		  					rxoffset += ((UINT16)ReceiveLocal * 256); // byte offset(msb) from start of Maps1RAM
		  					txmode = GETDATAFORRAM3;
		  				break;
  	
						case GETDATAFORRAM3:
		  					rxnbytes = ReceiveLocal;		// no. bytes (lsb)
		  					txmode = GETDATAFORRAM4;
		  				break;

						case GETDATAFORRAM4:
		  					rxnbytes += ((UINT16)ReceiveLocal * 256); 	// no. bytes (msb)
		  					rxcnt = 0;
  	  						if((rxoffset + rxnbytes) > Partables[tble_idx].n_bytes)  
								{							/* Ensure that number of bytes isn't too high */
		  	   						txmode = MAINCOMMAND;		
		  	  					}
							else
								{
				  					if(rd_wr == 0)  
										{		    // read & send back input data
		  	    							txcnt = 0;
						  	    			txmode = SENDECHO;
						  	    			txgoal = rxnbytes - 1;
					     	    			TRANSMITBYTE(*PartableByteRam(tble_idx, rxoffset));
						  	    			cksum += ReceiveLocal;
										}
					  				else  
										{                   // write data to input data block
				  	  							// buffer to maintain coherence of inputs while awaiting serial bytes
		
							  	  			if((rxnbytes > 1) && (rxnbytes < HAL_RXBUFFSIZE))
								  				{
									  	    		ibuf = 1;
												}
							  	  			else
								  				{
									  	    		ibuf = 0; 
												}
		   						  			txmode = GETDATAFORRAM5;
				  						}
								}
		  				break;
		
						case GETDATAFORRAM5:

		  	  				if(!ibuf)
								{
									*PartableByteRam(tble_idx, (rxoffset + rxcnt)) = ReceiveLocal;
								}
		  	  				else
								{
				  	    			rxbuf[rxcnt] = ReceiveLocal;
								}

			  				rxcnt++;
	
			  				if(rxcnt >= rxnbytes)  
								{
					  	  			if(ibuf)  
										{
									  	    for(index = 0; index < rxnbytes; index++)  
												{
										  	      *PartableByteRam(tble_idx,rxoffset + index) = rxbuf[index];
					  	    					}
					  	  				}
					  	  			if(rd_wr == 1)
										{
									  	    txmode = MAINCOMMAND;         // done receiving/writing all the data
										}
								  	else  
										{
									  	    txcnt = 0;				// send back data just written
									  	    txmode = SENDECHO;				//	 for verification
									  	    txgoal = rxnbytes;
									  	    TRANSMITBYTE(*PartableByteRam(tble_idx, rxoffset));
									  	}
							  	}       // finished all rxnbytes
		  				break;
		

						case GETBOOTLOADER1:
		  					if((ReceiveLocal == 'b') || (ReceiveLocal == 'B'))
								{
								  	  txmode = GETBOOTLOADER2;
								}
						  	else
								{
								  	  txmode = MAINCOMMAND;
								}
			  			break;		

						case GETBOOTLOADER2:
		  					if((ReceiveLocal == 'l')  || (ReceiveLocal == 'L'))
								{   /* Go to bootloader - XBL or xbl */
		  	  					   if((Vars.IOP_EngineSpeed / 64) < GlobalsRAM.IOP_ENGINESTALLSPEED)  
								   		{	/* Check that not rebooting with engine running */

											/* Stop the interrupts */											
										   	IE        = 0x00;

										   	EIE1      = 0x00;

											/* Flag that reboot required */
											BLSemaphore = BLSEMDOWNLOAD;

											/* Jump to the reset vector */
											__asm 
	
											ljmp 0x0000

											__endasm;
						  	  			}
						  		}
						  	txmode = MAINCOMMAND;
					  	break;
			
						default:			/* Lost sync - reset comms */
						  	txmode = MAINCOMMAND;
							txgoal = 0;
				  			next_txmode = MAINCOMMAND;
		  				break;
		
		  		}     // End of case switch for received data

    		rxcountdown = MAXBYTEWAIT;		/* Reset wait counter on every incoming byte, regardless */
		}	// End of if rx data 

	if(txmode != MAINCOMMAND)
  		{
			if (rxcountdown == 0)
				{
				  	txmode = MAINCOMMAND;	/* Lost comms sync - reset */
		  			next_txmode = MAINCOMMAND;
					rxcountdown = MAXBYTEWAIT;					
				}
			else
				{
					rxcountdown--;
				}
		}
}
else /* txgoal > 0 */ /*if (txgoal > 0)	 txgoal != 0, send bytes */
	{
  	  	if(txcnt < txgoal)  
			{
				if (SCON1 & 0x02)
					{
						SCON1 &= ~0x02;			/* Clear transmit interrupt flag  */
				
						txcnt++;				/* Increment the number of bytes */
												
						switch (txmode)
							{
								case SENDVERIFY:
					  				SBUF1 = *((UINT8 *)&vfy_fail + txcnt);
								break;          /* Not happy with the above! */
								case SENDECHO: 
									temptx = *PartableByteRam(tble_idx, rxoffset + txcnt);
					  				SBUF1 = temptx;
								  	if(rd_wr == 0)
										{
									  	    cksum += temptx;
										}
								break;
								case SENDREV:
								  	SBUF1 = XenoRevNum[txcnt];
								break;
								case SENDSIGN: 
					  				SBUF1 = XenoSign[txcnt];
								break;
								case SENDRTDATA:
								default:
					   				xcntr++;
										if (txcnt & 0x01)
											{
												SBUF1 = shrtbuff[1];
											}
										else
											{   /* Pull back 2 bytes at a time to prevent glitches */
												shrtbuff[0] = *DatatableByteRam(tble_idx, txcnt);
												shrtbuff[1] = *DatatableByteRam(tble_idx, txcnt + 1);
												SBUF1 = shrtbuff[0];
											}
					   				cksum += SBUF1;
					   				if(xcntr >= 3)  
										{	 // transmitted 4 bytes, load 2 new words
					       					xcntr = -1;
					       					txptr += 2;
					   					}
								break;
							}
					}
			} 
		 else  
			{   /* done transmitting */
		   		if(txmode == SENDRTDATA)
					{
			      		Vars.HAL_Checksum = cksum;
					}
		    	txcnt = 0;
		    	txgoal = 0;
		    	txmode = MAINCOMMAND;
//				SCON1 &= ~0x01;						/* Clear receive interrupt flag  */
			} 
	}														 
		
	return;
}

		

/***********************************************************************************/
/*                                                                                 */
/* _sdcc_external_startup() - Correction for SDCC problem                          */
/*                                                                                 */
/***********************************************************************************/

#if defined SDCC
void _sdcc_external_startup (void)
{
   PCA0MD &= ~0x40;                       /* Set watchdog off */
}
#endif




