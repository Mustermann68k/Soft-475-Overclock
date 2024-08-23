/****************************************************************/
/* Purpose..: This is V0.4 of a PROVE OF CONCEPT for programming*/
/*            Gazelle clock generator in Macintosh LC475.      	*/
/*																*/
/*			  ABSOLUTE NO WARANTY								*/
/*																*/
/*			  This program will run your computer out of		*/
/*			  specifications and may damage your computer.		*/
/*			  This program may change the CPU clock freqency	*/
/*			  and may crash or overheat of some components		*/
/*			  in your computer which may cause permanent damage.*/
/*																*/
/*			  The program may or MAY NOT WORK as expected.		*/
/*																*/
/*			  NO LIABILITY for any harm at your computer, you,  */
/*			  or the rest of the world.							*/
/*																*/
/* Input....: void                                              */
/* Returns..: void                                              */
/*																*/
/* V0.1		7/20/2024 First working version includes 20-42Mhz	*/
/*						-- this mode done by Phipli             */
/*																*/
/* V0.2		7/20/2024 GestaltID added							*/
/*			7/22/2024 Settings for registers below based on		*/
/*					  tables from LC475 ROM:					*/
/*					  	MEMCjrConfiguration						*/
/*					  	MEMCjrRefreshInterval					*/
/*					  	BIOSTimeout								*/
/*					  	BIOSConfig								*/
/*					  Settings for register below based on		*/
/*					  MEMCjr Datasheet:							*/
/*					  	FrameBufferConfiguration				*/
/*						-- this mode done by Phipli             */
/*						                                        */
/* V0.3		Converted to 40MHz Extension    */
/*						-- zigzagjoe            */
/*																*/
/* V0.4		17th Aug 2024 Extended functionality of Extension    */
/*               Gets current speed settings from resources     */
/*               Shows status using loadtime Icon               */
/*               Checks if PRAM is set to 24bit                 */
/*               Does not run if PRAM is set to 24 bit          */
/*               Reimplemented and extended MachineType check   */
/*						-- Phipli                               */
/*																*/
/****************************************************************/

#include "A4Stuff.h"
#include "SetupA4.h"
#include "ShowInitIcon.h"
#include <Gestalt.h>
#include "xpram.h"

#define MEMCjrAddr                 	0x50F0E000
#define MEMCjrConfiguration			0x50F0E030
#define MEMCjrRefreshInterval		0x50F0E034
#define MEMCjrHoldingRegister		0x50F0E07C
#define FrameBufferConfiguration	0xF9800010
#define ClockChipInterface			0xF98003C0
#define BIOSTimeout					0x50F18600
#define BIOSConfig					0x50F18000

const int kIconBkg = 129;
const int kIcon20 = 130;
const int kIcon25 = 131;
const int kIcon33 = 132;
const int kIcon40 = 133;
const int kIcon45 = 134;
const int kIcon50 = 135;
const int kIconBad = 200;
const int kIconOff = 201;
const int kIconErr = 202;
const int kIconPro = 203;
const int kIcon24b = 204;


// weAre32Bit reads the 32-bit addressing status out of the xPRAM of the machine
// and returns true if we're currently set up for 32-bit addressing or false if not.
int weAre32Bit() {
	char membyte;
	read_xpram(&membyte, 0x8A, 1);
	
	if ((membyte & 5) != 5) {
		return false;
	}

	return true;
}

typedef struct SpdStruct{
	int Icon;
	int SpeedTenths;
	unsigned int isSetShort;
	char rMin;
	char rMax;
	unsigned int valN;
	unsigned int valM;
	unsigned int valP;
} SPDS;
typedef SPDS* SPDSPtr;
typedef SPDSPtr* SPDSHandle;

int setregistermax(int level)
{
	unsigned long config = 0;

	switch(level)
	{
		case 0:
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x0101>>6;
			*((unsigned long*)(MEMCjrConfiguration)) = 0x0101;
			config = 0x0000003f & *((unsigned long*)(FrameBufferConfiguration));
			config = config + ((0x0000003f & *((unsigned long*)(MEMCjrHoldingRegister)))<<6);
			config = config & 0x000000ff;
			*((unsigned long*)(MEMCjrHoldingRegister)) = config>>6;
			*((unsigned long*)(FrameBufferConfiguration)) = config;
			break;
		case 1:
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x001A>>6;
			*((unsigned long*)(MEMCjrConfiguration)) = 0x001A;		
			config = 0x0000003f & *((unsigned long*)(FrameBufferConfiguration));
			config = config + ((0x0000003f & *((unsigned long*)(MEMCjrHoldingRegister)))<<6);
			config = config & 0x000000ff;
			*((unsigned long*)(MEMCjrHoldingRegister)) = config>>6;
			*((unsigned long*)(FrameBufferConfiguration)) = config;
			break;
		case 2:
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x00DC>>6;
			*((unsigned long*)(MEMCjrConfiguration)) = 0x00DC;
			config = 0x0000003f & *((unsigned long*)(FrameBufferConfiguration));
			config = config + ((0x0000003f & *((unsigned long*)(MEMCjrHoldingRegister)))<<6);
			config = config & 0x000000ff;
			config = config | 0x00000200;
			*((unsigned long*)(MEMCjrHoldingRegister)) = config>>6;
			*((unsigned long*)(FrameBufferConfiguration)) = config;

			break;
		case 3:
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x02DC>>6;
			*((unsigned long*)(MEMCjrConfiguration)) = 0x02DC;		
			config = 0x0000003f & *((unsigned long*)(FrameBufferConfiguration));
			config = config + ((0x0000003f & *((unsigned long*)(MEMCjrHoldingRegister)))<<6);
			config = config & 0x000000ff;
			config = config | 0x00000A00;
			*((unsigned long*)(MEMCjrHoldingRegister)) = config>>6;
			*((unsigned long*)(FrameBufferConfiguration)) = config;
			break;
		default:
			return -1;
	}
	return 0;
}

int setregistermin(int level)
{
	switch(level)
	{
		case 0:
			*((unsigned long*)(BIOSTimeout)) = 0x0280;
			*((unsigned long*)(BIOSConfig)) = 0x01;
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x011D>>6;
			*((unsigned long*)(MEMCjrRefreshInterval)) = 0x011D;		
			break;
		case 1:
			*((unsigned long*)(BIOSTimeout)) = 0x01E0;
			*((unsigned long*)(BIOSConfig)) = 0x01;
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x016B>>6;
			*((unsigned long*)(MEMCjrRefreshInterval)) = 0x016B;		
			break;
		case 2:
			*((unsigned long*)(BIOSTimeout)) = 0x00D5;
			*((unsigned long*)(BIOSConfig)) = 0x00;
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x01E7>>6;
			*((unsigned long*)(MEMCjrRefreshInterval)) = 0x01E7;		
			break;
		case 3:
			*((unsigned long*)(BIOSTimeout)) = 0x0000;
			*((unsigned long*)(BIOSConfig)) = 0x00;
			*((unsigned long*)(MEMCjrHoldingRegister)) = 0x0255>>6;
			*((unsigned long*)(MEMCjrRefreshInterval)) = 0x0255;		
			break;
		default:
			return -1;
	}
	return 0;
}

// This does not work with my 33MHz machine with the 161 chip
// It hard crashes, like, really hard. Needs power cycle, not reset. - Phipli
/* Need to be retested Mustermann68k */

int setlong(unsigned int n, unsigned int m, unsigned int p)
/* This function push 21 bit data to Gazelle clock generator and enable a CPU frequency based on numbers in that data */

/* Parameters n, m, p are used to set output of Gazelle chip to (n / (m * p)) * CrystalFrequncy(31.3344Mhz).  CPU frequency is double of this output */
/* 21 bit are shifted to Gazelle register, starting with DS0(Clock Generator Address bit = 1 for CPU frequency), DS1(Tri-state bit = 0 as an output is needed */
/* DS2(VCO disable bit = 0 as VCO is needed), DS3(VCO Mode bit = 0 to use data shifted into Gazelle instead of internal table) */
/* next p, n, m are shifted into register. */
/* n and m shall be 1<=n,m<=127 , p = 0 for divider 1, p = 1 for divider 2, p = 2 for divider 4, p = 3 for divider 8 */ 

/* Return 0 if parameters are in range, else -1*/
{
	unsigned long clockvector = 0;
	unsigned long tmp = 0;
	int i;
	
	if((n<0) || (n>127) || (m<0) || (m>127) || (p<0)|| (p>3)) return -1;
	
	clockvector = 1; /* b0001 Clock1, No Tri, Enable, External */ 
	
	clockvector = clockvector + ((p & 3)<<4); /* Add P, P is 0,1,2,3 not 1,2,4,8 */ 
	clockvector = clockvector + ((n & 127)<<6); /* Add N */
	clockvector = clockvector + ((m & 127)<<13); /* Add M */
	
	
	*((unsigned long*)(ClockChipInterface)) = 0x00000000; /* Start with a defined value */
	*((unsigned long*)(ClockChipInterface)) = 0x00000008; /* Activate control about DATA and WR */
	*((unsigned long*)(ClockChipInterface)) = 0x0000000C; /* Reset shift register, switch to 20Mhz */
	*((unsigned long*)(ClockChipInterface)) = 0x00000008; /* Reset shift register */
	
	for(i=1;i<21;i++) /* Write 21 bit of data to shift register */
	{
		if(clockvector == (clockvector>>1)*2)
		{
			*((unsigned long*)(ClockChipInterface)) = 0x00000008; /* Set DATA to 0 */
			*((unsigned long*)(ClockChipInterface)) = 0x0000000A; /* Write to shift register */
		}
		else
		{
			*((unsigned long*)(ClockChipInterface)) = 0x00000009; /* Set DATA to 1 */
			*((unsigned long*)(ClockChipInterface)) = 0x0000000B; /* Write to shift register */
		}
		clockvector = clockvector>>1; /* Remove one bit from the lower end of clockvector */
	}
	*((unsigned long*)(ClockChipInterface)) = 0x0000000A; /* Prepare reseet */
	*((unsigned long*)(ClockChipInterface)) = 0x0000000E; /* Reset shift register and switch to variable speed (was 33Mhy) */
	*((unsigned long*)(ClockChipInterface)) = 0x0000000A; /* Reset shift register */
	*((unsigned long*)(ClockChipInterface)) = 0x00000002; /* Deactivate control, mazbe also fine with 0x00000000 */
	return 0;
}

// This works on my 33MHz 475 with the 161 chip - Phipli
int setshort(int level)
{
	unsigned long clockset = 0;
	if(level>-1 && level<4)
	{
		clockset = level;
		*((unsigned long*)(ClockChipInterface)) = 0x00000000 | clockset;
		*((unsigned long*)(ClockChipInterface)) = 0x00000008 | clockset;
		*((unsigned long*)(ClockChipInterface)) = 0x0000000C | clockset; /* Freqency changes at this point */
		*((unsigned long*)(ClockChipInterface)) = 0x00000008 | clockset;
		*((unsigned long*)(ClockChipInterface)) = 0x00000000 | clockset;
	}
	return 0;
}

void main(void)
{	
	long		machineType;
	long		oldA4;	
	Handle 		theHandle = nil;
	SPDSHandle 	customSpeedHandle = nil;
	SPDS		customData;
	int 		targetClockSpeed = 20;
	int* 		argh;
	int 		x = 0;
	int 		weAreElsie = false;
	
	oldA4 = SetCurrentA4();
	RememberA4();
	SetUpA4();
	
	// Load the background of the Icon - the jigsaw and some of the text.
	ShowInitIcon(kIconBkg, false);
	
	// Icon list
	/*
	const int kIconBkg = 129;
	const int kIcon20 = 130;
	const int kIcon25 = 131;
	const int kIcon33 = 132;
	const int kIcon40 = 133;
	const int kIcon45 = 134;
	const int kIcon50 = 135;
	const int kIconBad = 200;
	const int kIconCst = 201;
	const int kIconOff = 201;
	const int kIconErr = 201;
	const int kIconPro = 201;
	const int kIcon24b = 201;
	*/
	
	// get the speed from the Resource "SPD!"
	theHandle = Get1Resource( 'SPD!', 128 );
	if( theHandle != nil ){
		argh = (int *)*theHandle; // argh is an int*
		targetClockSpeed = *argh;
		ReleaseResource(theHandle);
	}else{ // draw an Error icon if I can't read the resource
		targetClockSpeed = -5;
	}
	
	// if targetClockSpeed is < 0, it is an error
	// if targetClockSpeed is 0, the extension is disabled
	// if targetClockSpeed is >0 and <=127, it might be a raw speed in MHz
	// if targetClockSpeed is >127 it is probably a resource id for a 'SPDS' resource
	
	/* lets read in the 'SPDS' resource if needed
	' Icon -       DWRD - 2 bytes
    ' Speed -    DWRD - 2 bytes
    ' Is short (boolean) - 1 bytes
    ' RegMin -  DBYT - 1 bytes
    ' RegMax - DBYT - 1 bytes
    ' Value N - DWRD - 2 bytes
    ' Value M - DWRD - 2 bytes
    ' Value P - DWRD - 2 bytes*/
	if( targetClockSpeed>127 ){
		customSpeedHandle = (SPDSHandle)Get1Resource( 'SPDS', targetClockSpeed );
		if( customSpeedHandle != nil ){
			customData = **customSpeedHandle;
			ReleaseResource(theHandle);
		}else{
			targetClockSpeed = -5;
		}
	}
	
	//
	// If there is an error, we override the speed with a number below 0
	//
	
	if( !weAre32Bit() ){ // if we are not 32Bit, show the 24b Error icon
		targetClockSpeed = -1;
	}
	
	// I want to make it check if the machine id is one of the following...
	Gestalt('mach', &machineType );
	
	switch( machineType )
	{
		case 86: // 20MHz LC / Performa 475 or 476 (Primus)
			weAreElsie = true;
			break;
		case 89: // 25MHz LC / Performa 475 or 476
			weAreElsie = true;
			break;
		case 90: // 33MHz LC / Performa 475 or 476
			weAreElsie = true;
			break;
		case 104: // PPC LC / Performa 475 or 476
			weAreElsie = true;
			break;
		case 93: // 20MHz Q605 (Aladdin)
			weAreElsie = true;
			break;
		case 94: // 25MHz Q605
			weAreElsie = true;
			break;
		case 95: // 33MHz Q605
			weAreElsie = true;
			break;
		case 87: // 20MHz LC / Performa 575 577 or 578 (Optimus)
			weAreElsie = true;
			break;
		case 91: // 25MHz LC / Performa 575 577 or 578
			weAreElsie = true;
			break;
		case 92: // 33MHz LC / Performa 575 577 or 578
			weAreElsie = true;
			break;
		case 105: // PPC LC / Performa 575 577 or 578ý
			weAreElsie = true;
			break;
		case 98: // Various Speeds LC / Performa / Quadra 630 and family - For testing only
			weAreElsie = true;
			break;
		case 106: // Various Speeds LC / Performa / Quadra 630 and family - For testing only
			weAreElsie = true;
			break;
		case 99: // Various Speeds LC / Performa 580 and family - For testing only
			weAreElsie = true;
			break;
		case 107: // Various Speeds LC / Performa 580 and family - For testing only
			weAreElsie = true;
			break;
		case 48: // IIvx - For testing only
			weAreElsie = true;
			break;
		case 27: // LC III - For testing only
			weAreElsie = true;
			break;
		case 77: // Apple Interactive Television Box / STB (perhaps 78 too? it was spare in '93)
			weAreElsie = true;
			break;
		default: // in all other cases
			weAreElsie = false;
	}
	
	if( !weAreElsie ){
		targetClockSpeed = -6;
	}
	
	/*********************/
	/*  setlong(n, m, p) */
	/*********************/
	
	if(targetClockSpeed > 128){ // don't worry, if the resource was missing, this should be -5
		//Here we deal with the special cases
		/* Struct Reminder
		int Icon;
		int SpeedTenths;
		unsigned int isSetShort;
		char rMin;
		char rMax;
		int valN;
		int valM;
		int valP; */
		ShowInitIcon(customData.Icon, true);
		setregistermin(0);
		setregistermax(3);
		if( customData.isSetShort == false ){
			setlong(customData.valN,customData.valM,customData.valP);
		}else{
			setshort(customData.valN); // Just re-using N for "Level"
		}
		setregistermin(customData.rMin);
		setregistermax(customData.rMax);
	}else{
		switch(targetClockSpeed)
		{
			case 0: // used when the extension is disabled by setting to zero mhz
				ShowInitIcon(kIconOff, true);
				break;
			case -1: // used when the computer is in 24b mode
				ShowInitIcon(kIcon24b, true);
				break;
			case -5: // used when I can't read the resources
				ShowInitIcon(kIconErr, true);
				break;
			case -6: // used for Not a supported computer, 
				ShowInitIcon(kIconBad, true);
				break;
			case 20:
				ShowInitIcon(kIcon20, true);
				setregistermin(0);
				setregistermax(3);
				setshort(0);
				setregistermin(0);
				setregistermax(0);
				break;
			case 21:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(97,36,3);
				setregistermin(0);
				setregistermax(1);
				break;
			case 22:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(79,28,3);
				setregistermin(0);
				setregistermax(1);
				break;
			case 23:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(59,20,3);
				setregistermin(0);
				setregistermax(1);
				break;
			case 24:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(40,13,3);
				setregistermin(0);
				setregistermax(1);
				break;
			case 25:
				ShowInitIcon(kIcon25, true);
				setregistermin(0);
				setregistermax(3);
				setshort(1); 
				setregistermin(1);
				setregistermax(1);
				break;
			case 26:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(5,3,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 27:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(64,37,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 28:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(20,45,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 29:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(13,7,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 30:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(73,38,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 31:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(99,50,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 32:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(84,41,2);
				setregistermin(1);
				setregistermax(2);
				break;
			case 33:
				ShowInitIcon(kIcon33, true);
				setregistermin(0);
				setregistermax(3);
				setshort(2);
				setregistermin(2);
				setregistermax(2);
				break;
			case 34:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(37,17,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 35:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(56,25,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 36:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(53,23,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 37:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(45,19,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 38:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(90,37,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 39:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(5,2,2);
				setregistermin(2);
				setregistermax(3);
				break;
			case 40:
				ShowInitIcon(kIcon40, true);
				setregistermin(0);
				setregistermax(3);
				setshort(3); 
				setregistermin(3);
				setregistermax(3);
				break;
			case 41:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(21,8,2);
				setregistermin(3);
				setregistermax(3);
				break;
			case 42:
				ShowInitIcon(kIconPro, true);
				setregistermin(0);
				setregistermax(3);
				setlong(43,16,2);
				setregistermin(3);
				setregistermax(3);
				break;
			case 45:
				ShowInitIcon(kIcon45, true);
				setregistermin(0);
				setregistermax(3);
				setlong(112,39,2);
				setregistermin(3);
				setregistermax(3);
				break;
			case 50:
				ShowInitIcon(kIcon50, true);
				setregistermin(0);
				setregistermax(3);
				setlong(75,47,1);
				setregistermin(3);
				setregistermax(3);
				break;
			default:
				ShowInitIcon(kIconPro, true);
		}
	}
	
	/* restore the a4 world */
	SetA4(oldA4);

	return;	

}	

