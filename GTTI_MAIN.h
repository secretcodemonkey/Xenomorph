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


/* Bootloader requirements **************************************************************************/
const UINT32 __at 0xF9F0	BLSoftwareVersion = 0x00000110;
const UINT32 __at 0xF9F4	BLParameterVersion= 0x00000100;
const UINT16 __at 0xF9F8	BLChecksum        = 0x0000;
const UINT16 __at 0xF9FA	BLValidator       = 0x1513;
UINT8 __at 0xff	BLSemaphore;
/* Bootloader requirements **************************************************************************/

//#define REVENG	1

#define BLSEMDOWNLOAD				0x5A
#define DISABLE_WDT()      			PCA0MD &= ~0x40;
#define FL_PAGE_SIZE       			512
#define LOCK_PAGE          			0xFA00
#define FLASH_SAFE_ADDR    			0xFFFF
#define ROMPAGELEN         			0x1000
#define ROMPAGE1START      			0xC000
#define ROMPAGE2START      			0xD000
#define ROMPAGE1COUNT 	   			0xCDFE
#define ROMPAGE2COUNT 	   			0xDDFE
#define ROMPAGE1VALID 	   			0xCDFC
#define ROMPAGE2VALID 	   			0xDDFC
#define ROMVALIDMARK       			0xF9CE 
#define LASTBYTEOFAPP	   			0xF9FF					
#define LASTPAGEOFAPP	   			0xF800
#define HAL_MAXBATTERYFORBURN		50
#define HAL_MAXSENSORFORBURN		30
SEGMENT_VARIABLE(flashKey1, UINT8, SEG_DATA) = 0x00;
SEGMENT_VARIABLE(flashKey2, UINT8, SEG_DATA) = 0x00;
SEGMENT_VARIABLE(flashAddress, UINT16, SEG_DATA) = FLASH_SAFE_ADDR;
#define RSTSRC_VAL      			0x02        // Set to enable desired reset sources
#define FUEL_MAP_PTS 				16
#define SPARK_MAP_PTS 				16
#define FUEL_RPM_PTS 				16
#define SPARK_RPM_PTS 				16
#define HAL_O2_PTS					7
#define TURBO_DUTY_RPM_PTS			8
#define TURBO_DUTY_TPS_PTS			8
#define TURBO_TARG_RPM_PTS			8
#define TURBO_TARG_TPS_PTS			8
#define FLASH_FOUR_PAGE_ADD			0xF800
#define FLASH_BLOCK_WRITE			0x01
#define FLASH_BLOCK_ERASE			0x03
#define AMXP1_6						0x06
#define AMXP1_7						0x07
#define AMXP2_0						0x08
#define AMXP2_1						0x09
#define AMXP2_3						0x0B
#define AMXP2_4						0x0C
#define AMXP2_5						0x0D
#define AMXP2_7						0x0F
#define AMXTEMP						0x1E
#define AMXVDD						0x1F
#define HAL_ADCNUMCHAN				8
#define FPCA0						500000L
#define FTIMER1						500000L
#define FTIMER3						2000000L
#define FSYS                        48000000L
#define FBAUD                       115200L
#define HAL_MINIMUMTIMERLATENCY		20
#define NULL						(UINT16 *)0
#define PPRENGINESPEEDDENOM			2
#define PPRENGINESPEEDNUM			3
#define PPRVEHICLESPEED				60
#define IOP_OPTIONLIMITERSPARK            0x01
#define IOP_OPTIONLIMITERFUEL             0x02
#define CUTRPMFUEL					0x01
#define CUTRPMSPARK					0x02
#define CUTRPMCLEAR					0x03
#define CUTOBFUEL					0x04
#define CUTOBSPARK					0x08
#define CUTOBCLEAR					0x0C
#define CUTCLTFUEL					0x10
#define CUTCLTSPARK					0x20
#define CUTCLTCLEAR					0x30
#define CUTSPR2FUEL					0x40
#define CUTSPR2SPARK				0x80
#define CUTSPR2CLEAR				0xC0
#define CUTFUELEITHER				0x55
#define CUTSPARKEITHER              0xAA
#define INJECTOR_ON					 if (Vars.IOP_CutBits & CUTFUELEITHER){}else {if (HAL_InjAtInterrupt == 0x20)	{HAL_InjAtInterrupt = 0x00;} P1 |= 0x08;}
#define INJECTOR_OFF				P1 &= 0xF7
#define INJECTOR_DELAY_ON(delayon)	 if (Vars.IOP_CutBits & CUTFUELEITHER){}else{  HAL_InjAtInterrupt = 0x10; HAL_Tmr1Reload = 65535-delayon;  TL1   = (UINT8)(HAL_Tmr1Reload & 0xFF);TH1 = (UINT8)(HAL_Tmr1Reload / 256); TF1 = 0;  TR1 = 0x1;IE |= 0x08;}
#define	INJECTOR_DELAY_OFF(delayoff)	HAL_InjAtInterrupt = 0x20; HAL_Tmr1Reload = 65535-delayoff; TL1   = (UINT8)(HAL_Tmr1Reload & 0xFF); TH1 = (UINT8)(HAL_Tmr1Reload / 256); TF1 = 0; TR1 = 0x1;IE |= 0x08;
#define INJECTORFACTOR				2
#define SPARK_ON                	if (Vars.IOP_CutBits & CUTSPARKEITHER){}else {P1 &= 0xEF;}
#define SPARK_OFF               	P1 |= 0x10
#define SPARK_DELAY_ON(delayon)	    if (Vars.IOP_CutBits & CUTSPARKEITHER){}else {HAL_SpkAtInterrupt = 0x10; HAL_Tmr3Reload = 65535-delayon;  TMR3L   = (UINT8)((HAL_Tmr3Reload & 0x00FF));TMR3H = (UINT8)((HAL_Tmr3Reload & 0xFF00) / 256); TMR3CN    = 0x04;EIE1 |= 0x80;}
#define SPARK_DELAY_OFF(delayoff)   HAL_SpkAtInterrupt = 0x20; HAL_Tmr3Reload = 65535-delayoff; TMR3L   = (UINT8)((HAL_Tmr3Reload & 0x00FF));TMR3H = (UINT8)((HAL_Tmr3Reload & 0xFF00) / 256); TMR3CN    = 0x04;EIE1 |= 0x80;
#define SPARK_ON_OFF(delayon)	    HAL_SpkAtInterrupt = 0x30; HAL_Tmr3Reload = 65535-delayon;  TMR3L   = (UINT8)((HAL_Tmr3Reload & 0x00FF));TMR3H = (UINT8)((HAL_Tmr3Reload & 0xFF00) / 256); TMR3CN    = 0x04;EIE1 |= 0x80;
#define SPARKFACTOR					8
#define SPARKMAXBEFOREFACT			(65536 / SPARKFACTOR) - 1
#define FAKEVSON					PCA0CPL3 = 0x00; PCA0CPH3 = 0x00; PCA0CPM3 |= 0x40; 
#define FAKEVSOFF					PCA0CPL3 = 0xFF; PCA0CPH3 = 0xFF; PCA0CPM3 &= 0xBF; 
#define HAL_BOOSTMINHIGH			160			/* 1% duty drive - minimum 				   */
#define HAL_BOOSTPERIOD				16000       /* 500,000Hz/16000Cnts = 31.25Hz           */
#define HAL_BOOSTMAXLOW				HAL_BOOSTPERIOD - HAL_BOOSTMINHIGH
#define HAL_BOOSTMINLOW				HAL_BOOSTMINHIGH
#define HAL_BOOSTMINDUTY			1
#define HAL_BOOSTMAXDUTY			99
#define HAL_BOOSTONEPERCENT 		HAL_BOOSTMINHIGH
#define VS_MINVEHSPEEDGEARDET   	5
#define TURBODUTYSET(d)				HAL_BoostLowCount = (HAL_BOOSTPERIOD - d); HAL_BoostHighCount = d;


/* Defines for operating mode */
#define ENGINESTOP   				1
#define MIRRORMODE   				2
#define SPARKONLY    				4
#define FUELONLY     				8
#define SPARKANDFUEL 				16
#define SETSPICS					P0 &= 0xEF
#define CLEARSPICS      			P0 |= 0x10
#define VS_CLUTCHENABLE				0x01		/* Enable for clutch functions            				*/
#define VS_CLUTCHINVERTSW   		0x02        /* Flag to invert clutch switch operation 				*/
#define VS_CLUTCHLCENABLE   		0x04        /* Flag to enable launch control                        */
#define VS_CLUTCHFTENABLE   		0x08        /* Flag to enable full throttle shifts                  */
#define VS_CLUTCHLCSPARK    		0x10        /* LC method - 0 = fuel, 1 = spark                      */
#define VS_CLUTCHFTSPARK    		0x20        /* FT method - 0 = fuel, 1 = spark                      */
#define VS_CLUTCHFTLINGER           0x40        /* 0 = clutch cancels, 1 = RPM/Timeout cancels only     */
#define VS_CLUTCHFTACTIVE   		0x01
#define VS_CLUTCHLCACTIVE   		0x02
#define NO_TBLES					9
#define NO_PARTBLES					7
#define NO_DATATBLES				1
#define NO_IDTBLES					1
#define RT_TABLE_INDX 				0
#define MAXBYTEWAIT                 2000L    	/* x 0.00025S = 2000 x 250us = 0.5S timeout */
#define HAL_VEHICLEMINPERIOD		2000u
#define HAL_VEHICLEMAXSPEED         3500u
#define HAL_VEHICLEMAXRISE			4
#define HAL_VEHICLEMAXFALL			-4
#define LIMCAUSEMAIN				0x01
#define LIMCAUSECLUTCH				0x02
#define CLUTCHFTLIMIT				0x01
#define CLUTCHLCLIMIT				0x02
#define HAL_HALFRAIL 				128
#define IOP_MAPSELECTSWITCH			1
#define IOP_DIGIO1					0x01
#define IOP_DIGIO2					0x02
#define MAPHISTLEN					10


SBIT(Led1, SFR_P2, 2); 
SBIT(Clutch, SFR_P2, 6);
SBIT(DIGIO1, SFR_P3, 0);


const UINT8 HAL_ADCChan[HAL_ADCNUMCHAN] = { AMXP2_7,   /* ANA1 In        */
											AMXP1_7,   /* MAP Sensor     */
											AMXP1_6,   /* Coolant Temp   */
											AMXP2_0 ,  /* Air Inlet Temp */
											AMXP2_1,   /* Throttle Posn  */
											AMXP2_3,   /* O2 Sensor      */
											AMXP2_4,   /* VBat Sense     */
											AMXP2_5};  /* V Sensor Sense */
                                           

const SINT8 IOP_COOLTMPMAP[256] = {210,210,179,162,151,142,135,130,125,120,117,113,110,107,104,102,100,98,96,94,92,90,88,87,85,84,83,81,80,79,
									78,76,75,74,73,72,71,70,69,68,67,66,66,65,64,63,62,62,61,60,59,59,58,57,56,56,55,54,54,53,52,52,51,51,50,49,
									49,48,48,47,47,46,45,45,44,44,43,43,42,42,41,41,40,40,39,39,38,38,37,37,36,36,35,35,34,34,33,33,33,32,32,31,
									31,30,30,29,29,29,28,28,27,27,26,26,26,25,25,24,24,23,23,23,22,22,21,21,21,20,20,19,19,19,18,18,17,17,16,16,
									16,15,15,14,14,14,13,13,12,12,12,11,11,10,10,9,9,9,8,8,7,7,7,6,6,5,5,4,4,4,3,3,2,2,1,1,1,0,0,-1,-1,-2,-2,-3,
									-3,-4,-4,-5,-5,-5,-6,-6,-7,-7,-8,-8,-9,-9,-10,-10,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,-17,-17,-18,-18,-19,
									-20,-20,-21,-22,-22,-23,-24,-25,-25,-26,-27,-28,-28,-29,-30,-31,-32,-33,-34,-35,-36,-37,-38,-39,-40,-42,-43,-44,
									-46,-47,-49,-51,-52,-55,-57,-59,-62,-65,-69,-73,-78,-86,-86};

const SINT8 IOP_AIRTMPMAP[256]  = {210,210,179,162,151,142,135,130,125,120,117,113,110,107,104,102,100,98,96,94,92,90,88,87,85,84,83,81,80,79,
									78,76,75,74,73,72,71,70,69,68,67,66,66,65,64,63,62,62,61,60,59,59,58,57,56,56,55,54,54,53,52,52,51,51,50,49,
									49,48,48,47,47,46,45,45,44,44,43,43,42,42,41,41,40,40,39,39,38,38,37,37,36,36,35,35,34,34,33,33,33,32,32,31,
									31,30,30,29,29,29,28,28,27,27,26,26,26,25,25,24,24,23,23,23,22,22,21,21,21,20,20,19,19,19,18,18,17,17,16,16,
									16,15,15,14,14,14,13,13,12,12,12,11,11,10,10,9,9,9,8,8,7,7,7,6,6,5,5,4,4,4,3,3,2,2,1,1,1,0,0,-1,-1,-2,-2,-3,
									-3,-4,-4,-5,-5,-5,-6,-6,-7,-7,-8,-8,-9,-9,-10,-10,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,-17,-17,-18,-18,-19,
									-20,-20,-21,-22,-22,-23,-24,-25,-25,-26,-27,-28,-28,-29,-30,-31,-32,-33,-34,-35,-36,-37,-38,-39,-40,-42,-43,-44,
									-46,-47,-49,-51,-52,-55,-57,-59,-62,-65,-69,-73,-78,-86,-86};


UINT16	HAL_Tmr1Reload;
UINT16	HAL_Tmr3Reload;
UINT8 HAL_Debug;
UINT16 HAL_BoostHighCount;				/* High time of boost waveform             */
UINT16 HAL_BoostLowCount;               /* Low time of boost waveform              */
UINT16 HAL_BoostLowTrans;               /* PCA transition time of low cycle        */
UINT8 DACTransfer1;
UINT8 DACTransfer2;
UINT16 DACConv;
UINT8 DACdir;
UINT8 txmode;
UINT8 kill_ser;
UINT8 tble_idx;
UINT8 vfy_flg;
UINT8 vfy_fail;
UINT16 rxcnt;
UINT8 reinit_flag;
UINT8 can_id;
UINT16 txcnt;
UINT16 txgoal;
UINT16 kill_ser_t;
UINT16 rxoffset;
UINT16 rxnbytes;
UINT32 rxcountdown;
UINT32 lmms;
UINT16 ROM1Num;
UINT16 ROM2Num;
UINT16 ROMActivePage;
UINT16 ROMRead;
UINT8 const * ROMBurnPointer;

SINT16 HAL_TurboDuty;
SINT16 HAL_IErrorTemp;
UINT8 HAL_ADCChanSel;
UINT8 HAL_ADCLoops;
UINT8 HAL_1msScheduler;
UINT8 HAL_Task10msStart;
UINT8 HAL_10msScheduler;
UINT8 HAL_Task100msStart;
UINT8 HAL_100msScheduler;
UINT8 HAL_1SScheduler;
UINT8 HAL_BoostSchedule;
UINT32 HAL_kEngineSpeed;
UINT16 HAL_EnginePeriod;
UINT16 HAL_EnginePrevious;
UINT8 HAL_EngineOverflow;
UINT32 HAL_kVehicleSpeed;
UINT16 HAL_VehiclePeriod;
UINT16 HAL_VehiclePrevious;
UINT8 HAL_VehicleOverflow;
UINT8 HAL_InjAtInterrupt;
UINT8 HAL_SpkAtInterrupt;
UINT16 HAL_FuelStart;
UINT16 HAL_FuelEnd;
UINT16 HAL_FuelPulseTime;
UINT16 HAL_FuelFactor;
UINT16 HAL_FuelUnscaled;
UINT8 IGN_AnglePerCylinder;
UINT16 HAL_SparkStartPrev;
UINT16 HAL_SparkStart;
UINT16 HAL_SparkEnd;
UINT16 FUEL_TimeTemp;
UINT16 FUEL_OverallTemp;
UINT8  FUEL_OverallFact;
UINT8  FUEL_IncreaseFact;
UINT8  FUEL_IncreaseISR;
UINT16 HAL_FuelExtend;
UINT16 HAL_FuelExtendISR;
UINT8 HAL_InjThisCycle;
UINT8 HAL_InjSync;
UINT8 HAL_IgnSync;
UINT8 HAL_T0Count;
UINT8 HAL_T1Count;
UINT8 HAL_T2Count;
UINT8 HAL_T3Count;
UINT8 HAL_FuelCounter;
UINT8 HAL_PCA0Count;
UINT8 HAL_BackgroundCnt;
UINT8  VS_KPHFact;
UINT8 * TURBO_PtrMaxBoostGr;
UINT16 TURBO_LimitGear;
UINT16 BOBEngineSpeed;
UINT16 BOBEngineSpeedd;
UINT16 BOBMap;
SINT8 BOBCorAngleMap;
UINT8 BOBCorFactMap;
UINT8 BOBTurboDuty;
UINT16 VS_RatioSplits[7];
UINT16	VS_FTDropToRPM;
UINT16  VS_GearRatio[8];
SINT16 IOP_VehicleDelta;
UINT16 IOP_Vehicle;
UINT8 ClutchType;
UINT8 HAL_TurboMapSel;
UINT8 HAL_IgnMapSel;
UINT8 HAL_FuelMapSel;
#ifdef REVENG
UINT16 HAL_IgnitionDelay;
UINT16 HAL_DistribTime;
#endif
UINT16 HAL_EnginePeriod720;
UINT16 IOP_MAPHist[MAPHISTLEN];

SINT8 IGN_TableLookup(UINT8 rpm, UINT8 map, UINT8 mapselect);
UINT8 FUEL_TableLookup(UINT8 rpm, UINT8 map, UINT8 mapselect);
UINT8 TURBO_DutyTableLookup(UINT8 rpm, UINT8 tps, UINT8 mapselect);
UINT8 TURBO_TargetTableLookup(UINT8 rpm, UINT8 tps, UINT8 mapselect);
UINT8 HAL_O2Lookup(UINT8 Raw);

typedef struct {

/* Inputs - raw inputs  - Note: ORDER!! */
	UINT16 HAL_Seconds;                             /* Time in seconds                  *//*  0*/
    UINT8 HAL_ANA1;                                 /* ADC counts - Analogue in 1       *//*  2*/
    UINT8 HAL_ManPressRaw;							/* ADC counts - manifold pressure   *//*  3*/
	UINT8 HAL_CoolTempRaw;							/* ADC counts - Coolant temperature *//*  4*/
	UINT8 HAL_AirInletRaw;                          /* ADC counts - Air inlet temp.     *//*  5*/
	UINT8 HAL_ThrottleRaw;							/* ADC counts - Throttle Position   *//*  6*/
	UINT8 HAL_O2SensorRaw;							/* ADC counts - O2 Sensor           *//*  7*/
	UINT8 HAL_VBattRaw;								/* ADC counts - V Battery Supply    *//*  8*/
	UINT8 HAL_VSenseRaw;							/* ADC counts - V Sensor supply		*//*  9*/

/* Inputs - post conversion */
	UINT16 IOP_EngineSpeedd;						/* x 1 Engine rotational speed   	*//* 10*/
    UINT16 IOP_ManifoldPressd;						/* x 10 Manifold Pressure kPa		*//* 12*/
	UINT8  IOP_ThrottlePos;							/* x 1 Throttle Position %          *//* 14*/
	SINT8  IOP_AirInletTemp;						/* x 1 Air inlet temperature C      *//* 15*/
	SINT8  IOP_CoolantTemp;							/* x 1 Coolant temperature C        *//* 16*/
	UINT8  IOP_BatteryVolts;						/* x 10 ECU Supply voltage V        *//* 17*/
	UINT8  IOP_SensorVolts;							/* x 10 Sensor supply voltage V     *//* 18*/
	UINT8  IOP_AirFuelRatio;						/* x 10 Air Fuel Ratio X:1          *//* 19*/

/* Fuel variables */
	UINT16 FUEL_CorFactSized;						/* x 1000 injector resize factor 	*//* 20*/
	UINT16 FUEL_CorFactMapd;                        /* x 1000 injector map factor       *//* 22*/
	UINT16 FUEL_BaseTimed;							/* Base injection time from input   *//* 24*/
	UINT16 FUEL_CorrectTimed;						/* Corrected injection time         *//* 26*/

/* Spark variables */
	UINT16 IGN_SparkPeriodd;						/* Time between spark event         *//* 28*/
    UINT8  IOP_OpMode;                              /* Operating mode of the controller *//* 30*/
	UINT8  IOP_Auxilliary;                          /* Auxilliary bits                  *//* 31*/
	SINT16 IGN_CorAngleMapd;						/* Correction angle from map        *//* 32*/
	UINT16 IGN_SparkHighd;                          /* High time of spark pulse         *//* 34 */
	UINT16 IGN_SparkTriggerd;						/* Delay from rising spark til next *//* 36 */
    UINT8  TURBO_ValveDuty;                         /* Turbo control valve duty demand  *//* 38 */

/* Outputs */
	UINT8  IOP_MapSwitches;                         /* Map select switch data           *//* 39 */
	UINT8  DAC_FakeCounts;							/* Fake signal DAC counts after lim *//* 40 */
	UINT8  HAL_Checksum;                            /* Checksum for sector              *//* 41 */
	UINT8  TURBO_Feedforward;                       /* Turbo control open loop demand   *//* 42 */
	UINT16 TURBO_MAPTargetd;                        /* Turbo control target pressure    *//* 43 */
	SINT16 TURBO_MAPErrord;							/* Turbo control pressure error     *//* 45 */
	SINT16 TURBO_MAPPTermd;							/* Turbo control proportional term  *//* 47 */
	SINT16 TURBO_MAPITermd;							/* Turbo control integral term      *//* 49 */
    UINT8  HAL_Burn;                                /* Flag indicating that burn needed *//* 51 */
	UINT8  TURBO_ClosedLoop;						/* Flag indicating boost control on *//* 52 */
	UINT16 VS_VehicleSpeedd;                        /* Vehicle indicated speed          *//* 53 */          
	UINT8  VS_Gear;                                 /* Vehicle indicated gear           *//* 55 */          
	UINT16 VS_Vehiclekphd;                          /* Vehicle indicated speed kph      *//* 56 */
	UINT16 VS_Ratiod;                               /* Calculated gear ratio            *//* 58 */
    UINT8  VS_ClutchBits;                           /* Status bit of the clutch system  *//* 60 */
    UINT16 VS_EngLimClutchd;                        /* Maximum engine speed for clutch  *//* 61 */
	UINT8  IOP_CutBits;                             /* Cut status bits                  *//* 63 */
	UINT8  IOP_ClutchSwitch;                        /* Clutch switch status             *//* 64 */
	UINT16 VS_ClutchTimer;                          /* Clutch system timer              *//* 65 */
	SINT8  IGN_FinalAngle;                          /* Final spark alteration angle     *//* 67 */
	SINT8  IGN_ClutchRetard;                        /* Spark alteration angle clutch    *//* 68 */
	UINT8  DEBUG;									/* Debug                            *//* 69 */
	SINT16 IOP_dManifold;							/* Rate of change of MAP            *//* 70 */
    SINT16 TURBO_MAPDTerm;                          /* Derivative term of turbo control *//* 72 */
	UINT16 IOP_EngineSpeed;							/* x 1 Engine rotational speed   	*//* 74 */
    UINT16 IOP_ManifoldPress;						/* x 10 Manifold Pressure kPa		*//* 76 */
	UINT8  FUEL_CorFactSize;						/* x 1000 injector resize factor 	*//* 78 */
	UINT8  FUEL_CorFactMap;                         /* x 1000 injector map factor       *//* 79 */
	UINT16 FUEL_BaseTime;							/* Base injection time from input   *//* 80 */
	UINT16 FUEL_CorrectTime;						/* Corrected injection time         *//* 82 */
	UINT16 IGN_SparkPeriod;							/* Time between spark event         *//* 84 */
	SINT8  IGN_CorAngleMap;							/* Correction angle from map        *//* 86 */
	UINT16 IGN_SparkHigh;                           /* High time of spark pulse         *//* 87 */
	UINT16 IGN_SparkTrigger;						/* Delay from rising spark til next *//* 89 */
	UINT16 TURBO_MAPTarget;                         /* Turbo control target pressure    *//* 91 */
	SINT16 TURBO_MAPError;							/* Turbo control pressure error     *//* 93 */
	SINT16 TURBO_MAPPTerm;							/* Turbo control proportional term  *//* 95 */
	SINT16 TURBO_MAPITerm;							/* Turbo control integral term      *//* 97 */
    UINT16 VS_Ratio;                                /* Calculate gear ratio             *//* 99 */
	UINT8  TURBO_CutCounter;                        /* Counter for overboost detect     *//*101 */
	UINT8  TURBO_OkayCounter;                       /* Counter for okayboost detect     *//*102 */
	UINT16 VS_EngLimClutch;                         /* Engine speed limit from clutch   *//*103 */
	UINT16 IOP_VehicleSpeed;						/* Vehicle speed RPM                *//*105 */
	UINT16 IOP_VehicleSpeedKph;                     /* Vehicle speed KPH                *//*107 */
	UINT16 VS_OveralLimiter;                        /* Engine speed limit RPM           *//*109 */
	UINT8  DAC_FakeCountsUL;						/* Fake signal DAC counts before lim*//*111 */
}VarForMonitor;


VarForMonitor Vars;


typedef struct {

/* IOP Input Parameters */
	UINT8  HAL_THROTTLEMIN;             /* 0   */  /* Raw counts for 0% throttle      	x1  	*/
	UINT8  HAL_THROTTLEMAX;             /* 1   */  /* Raw counts for 100% throttle    	x1  	*/
	UINT8  HAL_MAPMIN;                  /* 2   */  /* Raw counts for minimum MAP      	x1  	*/
	UINT8  HAL_MAPMAX;                  /* 3   */  /* Raw counts for maximum MAP      	x1  	*/
    UINT8  IOP_MAXMAPPRESS;			    /* 4   */  /* Maximum MAP pressure kPa        	x12  	*/	
/* DAC sensor parameters  */
	UINT8  DAC_FAKEMAPMIN;			    /* 5   */  /* Raw counts for min fake MAP     	x1  	*/
	UINT8  DAC_FAKEMAPMAX;              /* 6   */  /* Raw counts for max fake MAP     	x1  	*/
	UINT8  DAC_FAKEMAXMAPPRESS;		    /* 7   */  /* Maximum MAP pressure kPa fake   	x12  	*/
	UINT8  DAC_FAKEFCDMAX;			    /* 8   */  /* Fuel cut defender limit on Fake 	x1  	*/	
/* General parameters */
	UINT8  IOP_ENGINESTALLSPEED;	    /* 9   */  /* Stalling speed of engine RPM    	x64  	*/
	UINT8  IOP_ENGINESYNCSPEED;		    /*10   */  /* Sync speed of engine RPM        	x64  	*/
	UINT8  IOP_NUMBEROFCYLS;            /*11   */  /* Engine number of cyls (3-8)     	x1  	*/
    UINT8  IOP_OPERATINGMODE;           /*12   */  /* Requested operating mode        	Enum  	*/
/* FUEL Fuel parameters */
	UINT8  FUEL_STDINJSIZE;			    /*13   */  /* Standard injector size (cc)     	x2  	*/
	UINT8  FUEL_NEWINJSIZE;			    /*14   */  /* New injector size (cc)          	x2	 	*/
	UINT8  IOP_FUELMAPSELECT;           /*15   */  /* Fuel map selection              	bool  	*/  
	UINT8  IOP_IGNMAPSELECT;            /*16   */  /* Ignition map selection          	bool  	*/  
	UINT8  IOP_TURBOMAPSELECT;          /*17   */  /* Turbo map selection             	bool  	*/  
	UINT8  TURBO_PGAIN;					/*18   */  /* Proportional gain of boost controller     */
	UINT8  TURBO_IGAIN;					/*19   */  /* Integral gain of the boost controller     */
	SINT8  TURBO_ITERMMIN;				/*20   */  /* Minimum output of boost controller        */
	SINT8  TURBO_ITERMMAX;				/*21   */  /* Maximum output of boost controller        */
	UINT8  TURBO_MINMAPCL;				/*22   */  /* Minimum map for closed loop       x12     */
    UINT8  TURBO_MINTHROTTLE;           /*23   */  /* Minimum throttle for closed loop          */
	UINT8  TURBO_OFFDUTY;               /*24   */  /* Valve duty for valve off condition        */
	UINT8  TURBO_MAXDUTY;               /*25   */  /* Valve duty for maximum bleed              */
	UINT8  TURBO_MINDUTY;               /*26   */  /* Valve duty for minimum bleed when not off */
	SINT8  TURBO_MINIERROR;             /*27   */  /* Minimum I part error going into I calc    */
	SINT8  TURBO_MAXIERROR;             /*28   */  /* Maximum I part error going into I calc    */
	UINT8  FUEL_MAXGRADIENT;			/*29   */  /* Maximum gradient of fuel factor /10ms     */
	UINT8  IGN_MAXGRADIENT;             /*30   */  /* Maximum gradient of ign advance /10ms     */
    UINT8  VS_MAXFAKEGEAR;              /*31   */  /* Maximum gear for fake output speed   x1   */
	UINT16 VS_RATIO1;					/*32   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO2;					/*34   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO3;					/*36   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO4;					/*38   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO5;					/*40   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO6;					/*42   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIO7;					/*44   */  /* Transmission overall ratio          x100  */
	UINT16 VS_RATIOR;					/*46   */  /* Transmission overall ratio          x100  */
    UINT8  TURBO_GR1MAXBOOST;           /*48   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR2MAXBOOST;           /*49   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR3MAXBOOST;           /*50   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR4MAXBOOST;           /*51   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR5MAXBOOST;           /*52   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR6MAXBOOST;           /*53   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GR7MAXBOOST;           /*54   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_GRRMAXBOOST;           /*55   */  /* Maximum boost against gear          x12   */      
    UINT8  TURBO_BOOSTCUT;              /*56   */  /* Maximum boost limit - cut           x12   */
    UINT8  TURBO_BOOSTOKAY;             /*57   */  /* Maximum boost limit - restart       x12   */
    UINT8  TURBO_CUTTIME;               /*58   */  /* Time at cut limit before cut        x100  */   
    UINT8  TURBO_OKAYTIME;              /*59   */  /* Time at okay before restart         x100  */
	UINT8  VS_TYREWIDTH;                /*60   */  /* Tyre width (mm)                     x1    */ 
	UINT8  VS_TYREPROFILE;              /*61   */  /* Tyre profile (%)                    x1    */
	UINT8  VS_TYREDIAMETER;             /*62   */  /* Tyre diameter (inch)                x10   */
    UINT8  VS_LAUNCHRPM;                /*63   */  /* Launch control stall RPM            x64   */
	UINT8  VS_LAUNCHMAXVEHICLE;         /*64   */  /* Maximum vehicle RPM for launch      x64   */
	UINT8  VS_LIMITERHYST;              /*65   */  /* Engine limiter hysteresis           x64   */
	UINT8  VS_FTSHIFTMAXRISE;           /*66   */  /* Full throttle shift RPM max rise    x64   */
	UINT8  VS_FTMINTHROTTLE;            /*67   */  /* Min throttle for full throttle shiftx1    */
	UINT8  VS_FTMAXTIME;                /*68   */  /* Timeout of FT gear shifts           x10   */
	UINT8  VS_CLUTCHOPTIONS;            /*69   */  /* Clutch option bits                        */
	UINT8  VS_LIMITEROPTIONS;           /*70   */  /* Limiter option bits                       */
	UINT8  IOP_MAXENGINESPEED;          /*71   */  /* Engine speed artificial limiter     x64   */
	SINT8  VS_LCRETARDLIMIT;            /*72   */  /* Limit of spark retard for launch use x1   */
    UINT8  VS_LCRPMRETARDGAIN;          /*73   */  /* Prop gain of RPM controller         x1/256*/
    UINT8  VS_FTDROPTIME;               /*74   */  /* Time until RPM drop for full throt. x10   */
    UINT8  VS_LCMAXTIME;                /*75   */  /* Timeout for launch stall            x10   */
	UINT8  VS_FTSHIFTTARGABOVE;         /*76   */  /* Full throttle margin above coming   x64   */
	UINT8  VS_LCACTTHROTTLE;            /*77   */  /* Min throttle for launch             x1    */
	UINT8  VS_LCDISTHROTTLE;            /*78   */  /* Throttle for launch cancel          x1    */
	UINT8  VS_FTFALLSPEEDGRAD;          /*79   */  /* Falling speed gradient for WOT shftx1/10ms*/
	UINT8  VS_FTRPMRETARDGAIN;          /*80   */  /* Prop gain of RPM controller         x1/256*/
    SINT8  VS_VEHICLEMAXRISE;			/*81   */  /* Rising speed gradient for veh. spd.x1/10ms*/
    SINT8  VS_VEHICLEMAXFALL;			/*82   */  /* Falling speed gradient for veh.spd.x1/10ms*/
    SINT8  VS_VEHICLEMAXRISELC;			/*83   */  /* LC    speed gradient for veh. spd.x1/10ms */
	SINT8  VS_FTRETARDLIMIT;            /*84   */  /* Limit of spark retard for shift use  x1   */
	UINT8  HAL_O2RAWTABLE[HAL_O2_PTS];	/*85   */  /* Raw data axis points of O2 sensor x 1     */
	UINT8  HAL_O2VALTABLE[HAL_O2_PTS];  /*92   */  /* Actual value (AFR) points         x 10    */
	UINT8  IOP_ANIN1FUNCTION;			/*99   */  /* Function of ANIN1 input           x 1     */
    SINT8  IOP_TEMPDRIVEON;             /*100  */  /* Temperature for drive on          x 1     */
    SINT8  IOP_TEMPDRIVEOFF;            /*101  */  /* Temperature for drive off         x 1     */
	UINT8  IOP_TPSDRIVEON;              /*102  */  /* Throttle for drive on             x 1     */
	UINT8  IOP_TPSDRIVEOFF;             /*103  */  /* Throttle for drive off            x 1     */
	UINT8  TESTFIXES;                   /*104  */  /* For debug purposes                        */
	UINT8  TURBO_DMAPDELAY;             /*105  */  /* Delay time of delta MAP           x 10ms  */
	SINT8  TURBO_DGAIN;					/*106  */  /* Derivative gain of turbo control          */
	UINT8  pad2;                        /*107  */
	UINT32 Paddingpg[37];               /*108  */

} ParGlobals;   /* Page size 256 */

typedef struct {
/* FUEL Fuelling parameters */
	UINT8  FUEL_RPMTABLE[FUEL_RPM_PTS];/*   0*/  /* Engine speed axis data - Fuel   */
	UINT8  FUEL_MAPTABLE[FUEL_MAP_PTS];/*   16*/  /* MAP sensor axis data - Fuel     */
	UINT8  FUEL_PRCNTCORRTABLE[FUEL_MAP_PTS][FUEL_RPM_PTS];  /*   32*/  /* Fuel table                      */
															 /*  288*/
	UINT32 Paddingfm[56]; 	
} FuelMaps;                             /* Page size 512 */
					
typedef struct {
/* IGN ignition parameters */
	UINT8  IGN_RPMTABLE[SPARK_RPM_PTS];/*    0*/   /* Engine speed axis data - Ign    */
	UINT8  IGN_MAPTABLE[SPARK_MAP_PTS];/*   16*/   /* MAP sensor axis data - Ign      */
	SINT8  IGN_IGNOFFTABLE[SPARK_MAP_PTS][SPARK_RPM_PTS]; /*  32*/	/* Ignition offset table           */
															 /*  288*/
	UINT32 Paddingim[56]; 		
} IgnMaps;                             /* Page size 512 */

																				
typedef struct {
/* TURBO turbo control parameters */
	UINT8  TURBO_DUTYRPMTABLE[TURBO_DUTY_RPM_PTS];  /*   0*/ /* Engine speed axis data - boost */
										
	UINT8  TURBO_DUTYTPSTABLE[TURBO_DUTY_TPS_PTS];  /*   8*//* Throttle pos axis data - boost */
										             
	UINT8  TURBO_DUTYTABLE[TURBO_DUTY_TPS_PTS][TURBO_DUTY_RPM_PTS];	/*  16*/ /* Duty request data table        */

	UINT8  TURBO_TARGRPMTABLE[TURBO_TARG_RPM_PTS]; /*   80*/ /* Engine speed axis data - boost */

	UINT8  TURBO_TARGTPSTABLE[TURBO_TARG_TPS_PTS]; /*   88*/ /* Throttle pos axis data - boost */

	UINT8  TURBO_TARGTABLE[TURBO_TARG_TPS_PTS][TURBO_TARG_RPM_PTS]; /*  96*/ /* MAP request data table        */
													/*  160*/	 
	UINT32 Paddingbm[87]; 		
	UINT16 ValidateLong;
	UINT16 PageCounter;
} BoostMaps;                            /* Page size 512 */



// ram copy of inputs
SEG_XDATA ParGlobals 	 	GlobalsRAM;
SEG_XDATA FuelMaps	 		Fuel1RAM;
SEG_XDATA IgnMaps		 	Ign1RAM;
SEG_XDATA BoostMaps	 		Boost1RAM;
SEG_XDATA FuelMaps	 		Fuel2RAM;
SEG_XDATA IgnMaps		 	Ign2RAM;
SEG_XDATA BoostMaps	 		Boost2RAM;
// Duplicate pages starting at 0xD000


#define HAL_RXBUFFSIZE			256
UINT8 rxbuf[HAL_RXBUFFSIZE];


// flash copy of inputs - initialized
const ParGlobals __at(0xC000) GlobalsROM  = {

/* IOP Input Parameters */
    18,  /* HAL_THROTTLEMIN    */
	144, /* HAL_THROTTLEMAX    */
    0,   /* HAL_MAPMIN         */
	255, /* HAL_MAPMAX         */
    177, /* IOP_MAXMAPPRESS    */
	0,	 /*  DAC_FAKEMAPMIN Raw counts for min fake MAP          */
	255, /*  DAC_FAKEMAPMAX Raw counts for max fake MAP          */
	177, /*  DAC_FAKEMAXMAPPRESS Maximum MAP pressure kPa fake   */
	200, /*  DAC_FAKEFCDMAX Fuel cut defender limit on Fake     */	
/* General parameters */
	6, 	 /*	IOP_ENGINESTALLSPEED RPM for engine stall point      */
	8, 	 /* IOP_ENGINESYNCSPEED RPM for engine sync enable       */
    3,   /* IOP_NUMBEROFCYLS   	*/
    MIRRORMODE,   /* IOP_OPERATINGMODE  	*/
	/* DAC sensor parameters  	*/
	/* FUEL Fuel parameters 	*/
    173, /* FUEL_STDINJSIZE - cc*/ /* x 2 formula */
	173, /* FUEL_NEWINJSIZE - cc*/ /* x 2 formula */
	0, 	 /* IOP_FUELMAPSELECT   */
	0,   /* IOP_IGNMAPSELECT    */
	0,   /* IOP_TURBOMAPSELECT  */
	15,  /* TURBO_PGAIN         */
	30,  /* TURBO_IGAIN         */
	-40, /* TURBO_ITERMMIN      */
	60,  /* TURBO_ITERMMAX      */
	0,   /* TURBO_MINMAPCL      */
    62,	 /* TURBO_MINTHROTTLE	*/
	0,	 /* TURBO_OFFDUTY		*/
	80,	 /* TURBO_MAXDUTY		*/
	20,	 /* TURBO_MINDUTY		*/
	-17, /* TURBO_MINIERROR		*/
	17,	 /* TURBO_MAXIERROR		*/
	2,	 /* FUEL_MAXGRADIENT 	*/
	1,	 /* IGN_MAXGRADIENT 	*/
    3,   /* VS_MAXGEAR          */    
	0x059A,/* VS_RATIO1	 14.34  */
	0x032C,/* VS_RATIO2	  8.12  */
	0x023B,/* VS_RATIO3	  5.71  */
	0x01A9,/* VS_RATIO4	  4.25  */
	0x015C,/* VS_RATIO5	  3.48  */
	0x0064,/* VS_RATIO6	  1.00  */
	0x0064,/* VS_RATIO7	  1.00  */
	0x05B3,/* VS_RATIOR	 14.59  */
    167, /* TURBO_GR1MAXBOOST   */
    167, /* TURBO_GR2MAXBOOST   */
    167, /* TURBO_GR3MAXBOOST   */
    167, /* TURBO_GR4MAXBOOST   */
    167, /* TURBO_GR5MAXBOOST   */
    167, /* TURBO_GR6MAXBOOST   */
    167, /* TURBO_GR7MAXBOOST   */
    167, /* TURBO_GRRMAXBOOST   */
    253, /* TURBO_BOOSTCUT      */
    253, /* TURBO_BOOSTOKAY     */
    200, /* TURBO_CUTTIME       */
    100, /* TURBO_OKAYTIME      */
	175, /* VS_TYREWIDTH        */
	60,  /* VS_TYREPROFILE      */
	140, /* VS_TYREDIAMETER     */
    47,  /* VS_LAUNCHRPM        */
	2,   /* VS_LAUNCHMAXVEHICLE */
	1,   /* VS_LIMITERHYST      */
	2,   /* VS_FTSHIFTMAXRISE   */
	85,  /* VS_FTMINTHROTTLE    */
	10,  /* VS_FTMAXTIME        */
	2,   /* VS_CLUTCHOPTIONS    */
	0x0c,/* VS_LIMITEROPTIONS   */
	255, /* IOP_MAXENGINESPEED  */
	0,   /* VS_LCRETARDLIMIT    */
    100, /* VS_LCRPMRETARDGAIN  */
    3,   /* VS_FTDROPTIME       */
	200, /* VS_LCMAXTIME        */
	3,   /* VS_FTSHIFTTARGABOVE */
	75,  /* VS_LCACTTHROTTLE    */
	15,  /* VS_LCDISTHROTTLE    */
	45,  /* VS_FTFALLSPEEDGRAD  */
	100, /* VS_FTRPMRETARDGAIN  */
    4,   /* VS_VEHICLEMAXRISE   */
    -4,  /* VS_VEHICLEMAXFALL   */
    3,   /* VS_VEHICLEMAXRISELC */
	0,   /* VS_FTRETARDLIMIT    */
	{0,42,84,128,170,212,254},  /* HAL_O2RAWTABLE[HAL_O2_PTS]  */
	{0,42,84,128,170,212,254},  /* HAL_O2VALTABLE[HAL_O2_PTS]  */
	0,	 /* IOP_ANIN1FUNCTION	*/
    98,  /* IOP_TEMPDRIVEON;    */
	92,  /* IOP_TEMPDRIVEOFF;   */
    255, /* IOP_TPSDRIVEON;     */
    255, /* IOP_TPSDRIVEOFF;    */
	0,	 /* TESTFIXES           */
	4,   /* TURBO_DMAPDELAY     */
	0,   /* TURBO_DGAIN         */
	0,   /* Padding             */
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xCAFEFACE,0xCAFEFACE}
	}; 

const FuelMaps __at(0xC200) Fuel1ROM  = {
/* FUEL_RPMTABLE[FUEL_RPM_PTS] , used in fuel adjust table */
//	{ 500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000}, 
	{   8,  16,  23,  31,  39,  47,  55,  63,  70,  78,  86,  94, 102, 109, 117, 125},     // X64 FORMULA

/* FUEL_MAPTABLE[FUEL_MAP_PTS] , used in fuel adjust table */ 
//	{   0, 190, 390, 590, 800,1000,1200,1410,1610,1810,2020,2220,2430,2630,2840,3040},
	{   0,  16,  32,  48,  66,  83, 100, 118, 134, 151, 168, 185, 203, 219, 237, 253},     // X12 FORMULA

/* 		500  1K   1K5  2K   2K5  3K   3K5  4K   4K5  5K   5K5  6K   6K5  7K   7K5  8K   FUEL_PRCNTCORRTABLE[FUEL_MAP_PTS][FUEL_RPM_PTS] */
	{{100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -1.00   0kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.81  19kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.61  39kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.41  59kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.20  80kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.00 100kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.20 120kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.41 141kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.61 161kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.81 181kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.02 202kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.22 222kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.43 243kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.63 263kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.84 284kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 }}, /*  2.04 304kPa */

     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


const IgnMaps __at(0xC400) Ign1ROM  = {
/* IGN ignition parameters */

/* IGN_RPMTABLE[SPARK_RPM_PTS] , use in spark advance table */
//	{ 500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000}, 
	{   8,  16,  23,  31,  39,  47,  55,  63,  70,  78,  86,  94, 102, 109, 117, 125},     // X64 FORMULA

/* IGN_MAPTABLE[SPARK_MAP_PTS] kPa x 10 , use for spk adv */
//	{   0, 190, 390, 590, 800,1000,1200,1410,1610,1810,2020,2220,2430,2630,2840,3040},
	{   0,  16,  32,  48,  66,  83, 100, 118, 134, 151, 168, 185, 203, 219, 237, 253},     // X12 FORMULA

/* 		500  1K   1K5  2K   2K5  3K   3K5  4K   4K5  5K   5K5  6K   6K5  7K   7K5  8K   IGN_IGNOFFTABLE[SPARK_MAP_PTS][SPARK_RPM_PTS] */
	{{  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -1.00   0kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.81  19kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.61  39kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.41  59kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.20  80kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.00 100kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.20 120kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.41 141kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.61 161kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  0.81 181kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.02 202kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.22 222kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.43 243kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.63 263kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.84 284kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 }}, /*  2.04 304kPa */

     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


const BoostMaps __at(0xC600) Boost1ROM  = {
/*	UINT16 TURBO_DUTYRPMTABLE[TURBO_DUTY_RPM_PTS];   Engine speed axis data - boost */
//	{ 1000,2000,3000,4000,5000,6000,7000,8000},										
	{   16,  31,  47,  63,  78,  94, 109, 125},     // X64 FORMULA
/*	UINT8  TURBO_DUTYTPSTABLE[TURBO_DUTY_TPS_PTS];   Throttle pos axis data - boost */
	{    0,  20,  40,  60,  70,  80,  90, 100},										
/*	UINT8  TURBO_DUTYTABLE[TURBO_DUTY_TPS_PTS][TURBO_DUTY_RPM_PTS];   Duty request data table        */
	{{  30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   }},
/*	UINT16 TURBO_TARGRPMTABLE[TURBO_TARG_RPM_PTS];   Engine speed axis data - boost */
//	{ 1000,2000,3000,4000,5000,6000,7000,8000},										
	{   16,  31,  47,  63,  78,  94, 109, 125},     // X64 FORMULA
/*	UINT8  TURBO_TARGTPSTABLE[TURBO_TARG_TPS_PTS];   Throttle pos axis data - boost */
	{    0,  20,  40,  60,  70,  80,  90, 100},										
/*	UINT16 TURBO_TARGTABLE[TURBO_TARG_TPS_PTS][TURBO_TARG_RPM_PTS];                 */
	{{ 146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   }},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	0x0,
    0x0 
};


const FuelMaps __at(0xC800)  Fuel2ROM  = {
/* FUEL_RPMTABLE[FUEL_RPM_PTS] , used in fuel adjust table */
//	{ 500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000}, 
	{   8,  16,  23,  31,  39,  47,  55,  63,  70,  78,  86,  94, 102, 109, 117, 125},     // X64 FORMULA

/* FUEL_MAPTABLE[FUEL_MAP_PTS] , used in fuel adjust table */ 
//	{   0, 190, 390, 590, 800,1000,1200,1410,1610,1810,2020,2220,2430,2630,2840,3040},
	{   0,  16,  32,  48,  66,  83, 100, 118, 134, 151, 168, 185, 203, 219, 237, 253},     // X12 FORMULA

/* 		500  1K   1K5  2K   2K5  3K   3K5  4K   4K5  5K   5K5  6K   6K5  7K   7K5  8K   FUEL_PRCNTCORRTABLE[FUEL_MAP_PTS][FUEL_RPM_PTS] */
	{{100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -1.00   0kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.81  19kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.61  39kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.41  59kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /* -0.20  80kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.00 100kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.20 120kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.41 141kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.61 161kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  0.81 181kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.02 202kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.22 222kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.43 243kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.63 263kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 },  /*  1.84 284kPa */
	 {100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 ,100 }}, /*  2.04 304kPa */

     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


const IgnMaps __at(0xCA00) Ign2ROM  = {

/* IGN ignition parameters */

/* IGN_RPMTABLE[SPARK_RPM_PTS] , use in spark advance table */
//	{ 500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500,6000,6500,7000,7500,8000}, 
	{   8,  16,  23,  31,  39,  47,  55,  63,  70,  78,  86,  94, 102, 109, 117, 125},     // X64 FORMULA

/* IGN_MAPTABLE[SPARK_MAP_PTS] kPa x 10 , use for spk adv */
//	{   0, 190, 390, 590, 800,1000,1200,1410,1610,1810,2020,2220,2430,2630,2840,3040},
	{   0,  16,  32,  48,  66,  83, 100, 118, 134, 151, 168, 185, 203, 219, 237, 253},     // X12 FORMULA

/* 		500  1K   1K5  2K   2K5  3K   3K5  4K   4K5  5K   5K5  6K   6K5  7K   7K5  8K   IGN_IGNOFFTABLE[SPARK_MAP_PTS][SPARK_RPM_PTS] */
	{{  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -1.00   0kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.81  19kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.61  39kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.41  59kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /* -0.20  80kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.00 100kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.20 120kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.41 141kPa */
	 {  0 ,  0 ,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0 },  /*  0.61 161kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  0.81 181kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.02 202kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.22 222kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.43 243kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.63 263kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 },  /*  1.84 284kPa */
	 {  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 }}, /*  2.04 304kPa */

     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


const BoostMaps __at(0xCC00) Boost2ROM  = {

/*	UINT16 TURBO_DUTYRPMTABLE[TURBO_DUTY_RPM_PTS];   Engine speed axis data - boost */
//	{ 1000,2000,3000,4000,5000,6000,7000,8000},										
	{   16,  31,  47,  63,  78,  94, 109, 125},     // X64 FORMULA
/*	UINT8  TURBO_DUTYTPSTABLE[TURBO_DUTY_TPS_PTS];   Throttle pos axis data - boost */
	{    0,  20,  40,  60,  70,  80,  90, 100},										
/*	UINT8  TURBO_DUTYTABLE[TURBO_DUTY_TPS_PTS][TURBO_DUTY_RPM_PTS];   Duty request data table        */
	{{  30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   },
	{   30,  30,  30,  30,  30,  30,  30,  30   }},
/*	UINT16 TURBO_TARGRPMTABLE[TURBO_TARG_RPM_PTS];   Engine speed axis data - boost */
//	{ 1000,2000,3000,4000,5000,6000,7000,8000},										
	{   16,  31,  47,  63,  78,  94, 109, 125},     // X64 FORMULA
/*	UINT8  TURBO_TARGTPSTABLE[TURBO_TARG_TPS_PTS];   Throttle pos axis data - boost */
	{    0,  20,  40,  60,  70,  80,  90, 100},										
/*	UINT16 TURBO_TARGTABLE[TURBO_TARG_TPS_PTS][TURBO_TARG_RPM_PTS];                 */
	{{ 146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   },
	{  146, 146, 146, 146, 146, 146, 146, 146   }},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	ROMVALIDMARK,  //Validate word
	0x0001   //Counter for page
};


VarForMonitor Vars;


typedef struct {
   SEG_XDATA UINT16 * addrRam;
   UINT16  n_bytes;
} ParDescriptorRAM;

typedef struct {
   UINT16 * addrFlash;
   UINT16  n_bytes;
} ParDescriptorROM;

typedef struct {
   SEG_XDATA UINT8 * addrRam;
   UINT16  n_bytes;
} DataDescriptorRAM;

typedef struct {
   const SEG_CODE UINT16 * addrFlash;
   UINT16  n_bytes;
} IDDescriptorROM;

const ParDescriptorRAM Partables[NO_PARTBLES] = {
  { (UINT16 *)&GlobalsRAM, 	sizeof(ParGlobals)      }, 
  { (UINT16 *)&Fuel1RAM, 	sizeof(FuelMaps)        }, 
  { (UINT16 *)&Ign1RAM, 	sizeof(IgnMaps)    	    }, 
  { (UINT16 *)&Boost1RAM, 	sizeof(BoostMaps)       }, 
  { (UINT16 *)&Fuel2RAM, 	sizeof(FuelMaps)        }, 
  { (UINT16 *)&Ign2RAM, 	sizeof(IgnMaps)    	    }, 
  { (UINT16 *)&Boost2RAM, 	sizeof(BoostMaps)       }};

const ParDescriptorROM Partables1[NO_PARTBLES] =  { 
  { (UINT16 __code *)0xC000,sizeof(ParGlobals)      }, 
  { (UINT16 __code *)0xC200,sizeof(FuelMaps)        }, 
  { (UINT16 __code *)0xC400,sizeof(IgnMaps)    	    }, 
  { (UINT16 __code *)0xC600,sizeof(BoostMaps)       }, 
  { (UINT16 __code *)0xC800,sizeof(FuelMaps)        }, 
  { (UINT16 __code *)0xCA00,sizeof(IgnMaps)    	    }, 
  { (UINT16 __code *)0xCC00,sizeof(BoostMaps)       }};

const ParDescriptorROM Partables2[NO_PARTBLES] =  { 
  { (UINT16 __code *)0xD000,sizeof(ParGlobals)      }, 
  { (UINT16 __code *)0xD200,sizeof(FuelMaps)        }, 
  { (UINT16 __code *)0xD400,sizeof(IgnMaps)    	    }, 
  { (UINT16 __code *)0xD600,sizeof(BoostMaps)       }, 
  { (UINT16 __code *)0xD800,sizeof(FuelMaps)        }, 
  { (UINT16 __code *)0xDA00,sizeof(IgnMaps)    	    }, 
  { (UINT16 __code *)0xDC00,sizeof(BoostMaps)       }};


const DataDescriptorRAM Datatables[NO_DATATBLES] = {
  { (UINT8 *)&Vars,   		74                      }};

const IDDescriptorROM IDtables[NO_IDTBLES] = {
  { (const SEG_CODE UINT16 *)&XenoRevNum,   sizeof(XenoRevNum)     }};


#define PartableByteRam(iTable, iByte)   	((unsigned char *)Partables[iTable].addrRam + iByte)
#define DatatableByteRam(iTable, iByte)   	((unsigned char *)Datatables[iTable].addrRam + iByte)
#define IDtableByteRam(iTable, iByte)   	((unsigned char *)IDtables[iTable].addrRam + iByte)
#define PartableBytes(iTable)            	(Partables[iTable].n_bytes)
#define DatatableBytes(iTable)            	(Datatables[iTable].n_bytes)
#define TRANSMITBYTE(b) 		  		while (!(SCON1 & 0x02));  SCON1 &= ~0x02;   SBUF1 = b;
