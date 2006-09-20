/*
 * M41T81S: 
 *
 * This module defines a single 'rtc' command to read/write or
 * test the ST Micro Real Time Clock attached to the SM-501
 * I2C pins.
 *
 * If called with no parameters, it will display the current time
 * to the console device and set the 'time' environment variable.
 *
 * If called with a single parameter of "test", it will read the 
 * current time, wait a second and read the current time again.
 * It will return success (zero) if the RTC time appears to tick
 * by 1 second.
 *
 * If called with one or two parameters that appear to fit an 
 * ISO 8601 time format (i.e. YYYY-MM-DD HH:MM:SS.00 ), it will
 * set the date and time.
 *
 * Copyright (c) Boundary Devices, 2006
 *
 */
#include "sm501.h" 
#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <command.h>

int  WriteI2C(int bVal);
int  ReadI2C(int ack);
int  I2CStart(int bSlaveAddress);
void I2CStop(void);
void I2CInit(void);

//////////////////////////////////////////////////////////

#define SM501_GPIO_DATA 0x10000
#define SM501_GPIO_DIR  0x10008

#define INPUT  0
#define OUTPUT 1

#define I2C_CLK  46
#define I2C_DATA 47
#define GPBIT_MASK(bitnum) (1<<(bitnum&0x1f))
#define GPBIT_TEST(bitnum) ( (ReadReg(SM501_GPIO_DATA + ((bitnum>>5)<<2))) & (1<<(bitnum&0x1f)) )
#define GPBIT_TESTBIT(c,bitnum) ( c & (1<<(bitnum&0x1f)) )
#define GPBIT_SET(x,bitnum,value) ( (value) ? ((x) | (1<<(bitnum&0x1f))) : ((x) & ~(1<<(bitnum&0x1f))) )

//0.400 Mhz protocol  = 1/.4 uSec = 10/4 uSec = 2.5 uSec
//4 should be safe
#define I2CWait 4
#define LongBusWait 8
#define StartBusWait 12

typedef struct {
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliseconds;
} SYSTEMTIME ;

typedef SYSTEMTIME *LPSYSTEMTIME ;

void I2CInit(void)
{
    int i;

    for(i=0; i<9; i++)
    {
        I2CStop();  
    }
}


#define ReadReg(reg) READ_SM501_REG(reg)
#define WriteReg(reg,data) STUFF_SM501_REG((reg), (data))

void SetVal_ClkData(int clk,int data)
{
#if (I2C_DATA>>5)==(I2C_CLK>>5)
	ulong c = ReadReg( SM501_GPIO_DATA + ((I2C_DATA>>5)<<2) );
	c = GPBIT_SET(c, I2C_DATA, data);
	c = GPBIT_SET(c, I2C_CLK, clk);
	WriteReg(SM501_GPIO_DATA + ((I2C_DATA>>5)<<2), c);
#else
	ulong c = ReadReg( SM501_GPIO_DATA + ((I2C_DATA>>5)<<2) );
	c = GPBIT_SET(c, I2C_DATA, data);
	WriteReg(SM501_GPIO_DATA + ((I2C_DATA>>5)<<2), c);

	c = ReadReg( SM501_GPIO_DATA + ((I2C_CLK>>5)<<2) );
	c = GPBIT_SET(c, I2C_CLK, clk);
	WriteReg(SM501_GPIO_DATA + ((I2C_CLK>>5)<<2), c);
#endif
}

void SetDir_ClkData(int clk,int data)
{
	ulong c,c1;
#if (I2C_DATA>>5)==(I2C_CLK>>5)
	c = ReadReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2) );
	if (clk==OUTPUT) {
		//will be low, and can then change data
		c1 = GPBIT_SET(c, I2C_CLK, OUTPUT);
		if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2), c1);
		c = GPBIT_SET(c1, I2C_DATA, data);
	} else {
		//clk might be low currently, so change data 1st
		c1 = GPBIT_SET(c, I2C_DATA, data);
		if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_DATA>>5)<<2), c1);
		c = GPBIT_SET(c1, I2C_CLK, INPUT);
	}
	if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2), c);
#else
	if (clk==OUTPUT) {
		//will be low, and can then change data
		c = ReadReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2) );
		c1 = GPBIT_SET(c, I2C_CLK, OUTPUT);
		if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2), c1);

		c1 = ReadReg( SM501_GPIO_DIR + ((I2C_DATA>>5)<<2) );
		c = GPBIT_SET(c1, I2C_DATA, data);
		if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_DATA>>5)<<2), c);
	} else {
		//clk might be low currently, so change data 1st
		c = ReadReg( SM501_GPIO_DIR + ((I2C_DATA>>5)<<2) );
		c1 = GPBIT_SET(c, I2C_DATA, data);
		if (c!=c1) WriteReg( SM501_GPIO_DIR + ((I2C_DATA>>5)<<2), c1);

		c1 = ReadReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2) );
		c = GPBIT_SET(c1, I2C_CLK, INPUT);
		if (c!=c1) c1WriteReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2), c);
	}
#endif
}
void SetDir_Clk(int clk)
{
	ulong c = ReadReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2) );
	c = GPBIT_SET(c, I2C_CLK, clk);
	WriteReg( SM501_GPIO_DIR + ((I2C_CLK>>5)<<2), c);
}

void I2CStop()
{
	SetDir_Clk(OUTPUT);		//low clock
	udelay(I2CWait);
	SetDir_ClkData(OUTPUT,OUTPUT);		//low clock, low data
	udelay(I2CWait);

	//Drive Write SCL High
	SetDir_Clk(INPUT);					//high clock
	udelay(I2CWait);

	// Drive Write SDA High
	SetDir_ClkData(INPUT,INPUT);		//transition on data from low to high while clock is high is a stop control signal
	udelay(LongBusWait);

}


//return 0 for success
int WriteI2C(int bVal) 
{

	int mask;
	int i;

	// Enable Write SDA and SCL, and Drv SCL low
	SetDir_Clk(OUTPUT);			//clock low
	udelay(I2CWait);					//hold time

	for (mask=0x80; mask; mask>>=1)
	{
		SetDir_ClkData(OUTPUT, (bVal & mask) ? INPUT : OUTPUT);		// Write data bits to SDA
		udelay(LongBusWait);

		SetDir_Clk(INPUT);		// Drv CLK High
		udelay(I2CWait);

		SetDir_Clk(OUTPUT);		// Drv CLK Low
		udelay(I2CWait);				//hold time after clock goes low
	}


	SetDir_ClkData(OUTPUT,INPUT);	// Disable Write SDA
	udelay(LongBusWait);					//wait for acknowledge to be placed on SDA
	SetDir_Clk(INPUT);				// Drive Clock High

	// Read SDA, until SDA==0
	for (i=0; i<255; i++) {
		udelay(LongBusWait);
		if (!GPBIT_TEST(I2C_DATA)) {
			SetDir_Clk(OUTPUT);			// Drv Clk LOW
			return 0;					//success
		}
	}

	printf( "WriteI2C(%i) failed\n", bVal );
	return -1;
}

int ReadI2C(int ack)
{
	int mask;
	int byRet = 0;

//	SetVal_ClkData(0,0);
	SetDir_Clk(OUTPUT);			//clock low
	udelay(I2CWait);			//hold time

	SetDir_ClkData(OUTPUT,INPUT);		//clock low, data input
	for (mask=0x80; mask; mask>>=1)
	{
		// Disable Write SDA, Drive SCL to LOW
		SetDir_Clk(OUTPUT);				//clock low
		udelay(LongBusWait);

		// Enable Write SCL, Drive SCL to HIGH
		SetDir_Clk(INPUT);				//clock high
		udelay(I2CWait);

		// Read data bits from SDA 
		if (GPBIT_TEST(I2C_DATA)) byRet |= mask;	//sample data bit
	}

	SetDir_Clk(OUTPUT);					//clock low
	udelay(I2CWait);					//let them stop driving data line

	if (ack) {
		SetDir_ClkData(OUTPUT,OUTPUT);		//clock low, data low for ack
	}
	udelay(I2CWait);
	SetDir_Clk(INPUT);					//clock high
	udelay(I2CWait);					//wait ack/noack phase
	return byRet;

}

int I2CStart(int SlaveAddress)
{
	int ret;
	udelay(I2CWait);
	// Enable Write SDA and Write SCL, and drive them high
	SetDir_ClkData(INPUT,INPUT);
	SetVal_ClkData(0,0);			//they float high anyway
	udelay(StartBusWait);

	// Drive Data
	SetDir_ClkData(INPUT,OUTPUT);	//drive data low, (high to low transition on data, while clock high is start signal)
	udelay(StartBusWait);

	ret = WriteI2C(SlaveAddress);
	if (ret) {
	    printf( "I2CStart failed write of device address\n" );
	}
	return ret;
}

#define M41T81S_SlaveAddrWrite   0xd0
#define M41T81S_SlaveAddrRead    0xd1

#define M_HUNDREDTHS	0		//00-99	BCD
#define M_SECONDS		1		//00-59	BCD, bit 7 ST (oscillator is stopped bit)
#define M_MINUTES		2		//00-59 BCD
#define M_HOURS			3		//high 2 bits are CENTURY, low 6 BCD 00-23
#define M_WEEKDAY		4		//01-07
#define M_DAY			5		//01-31 BCD
#define M_MONTH			6		//01-12 BCD
#define M_YEAR			7		//00-99	BCD
#define M_CALIBRATION	8
#define M_WATCHDOG		9
#define M_ALARM_MOTH	0x0a	//01-12 BCD, high 3 bits enable
#define M_ALARM_DAY		0x0b	//01-31 BCD. high 2 bits RPT4, RPT5
#define M_ALARM_HOUR	0x0c	//00-23 BCD, high 2 bits RPT3, HT
#define M_ALARM_MINUTES 0x0d	//00-59 BCD, high bit RPT2
#define M_ALARM_SECONDS 0x0e	//00-59 BCD, high bit RPT1
#define M_HALT 0x40

#define M_FLAGS			0x0f	//bit 7(WDF), 6(AF), 4(BL), 2(OF)
#define M_SQW			0x13	//bit 7(RS3), 6(RS2), 5(RS1), 4(RS0)

//M_FLAGS bit mask definitions
#define MF_WDF			0x80	//Watchdog flag
#define MF_AF			0x40	//Alarm flag
#define MF_BL			0x10	//Battery low
#define MF_OF			0x04	//Oscillator fail
static int bcd(int val,int low, int high)
{
	int tens = val>>4;
	int ones = val & 0xf;
	if ((tens > 9) || (ones > 9)) return -1;
	val = (tens*10) + ones;
	if ((val < low) || (val > high)) return -1;
	return val;
}
static unsigned char toBcd(int val)
{
	int tens = val/10;
	int rem = val - (tens*10);
	if (tens >= 10) tens = tens % 10;
	return (tens<<4) | rem;
}

static int GetTime(LPSYSTEMTIME lpst)
{
	unsigned char b[M_YEAR+1];
	unsigned char * p = b;
	unsigned char flags;
	int i;
	I2CInit();		//send a bunch of stops

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_ALARM_HOUR)) return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;

	b[0] = (unsigned char)ReadI2C(0);	//read Halt bit
	I2CStop();

	if (b[0] & M_HALT) {
      printf( "M41T81S_GetTime: was halted\n" );
		//halted, restart it
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_ALARM_HOUR)) return -1;
		if (WriteI2C(b[0] & ~M_HALT)) return -1;	//clear Halt bit
		I2CStop();
	}

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_HUNDREDTHS))	return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;
	for (i=0; i<M_YEAR; i++) *p++ = (unsigned char)ReadI2C(1);
	*p++ = (unsigned char)ReadI2C(0);	//read year
	I2CStop();

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_FLAGS))	return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;
	flags = (unsigned char)ReadI2C(0);	//read flags
	I2CStop();

	if (flags & MF_BL) {
		printf( "M41T81S_GetTime: Battery low\n" );
	}
	if (flags & MF_OF) {
		printf( "M41T81S_GetTime: Oscillator failed\n" );

		//stop oscillator
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_SECONDS)) return -1;
		if (WriteI2C(b[M_SECONDS] | 0x80)) return -1;			//set stop bit
		I2CStop();

		//start oscillator
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_SECONDS)) return -1;
		if (WriteI2C(b[M_SECONDS] & ~0x80)) return -1;			//clear stop bit
		I2CStop();
	}


	lpst->wYear			= ( (b[M_YEAR] > 0x99) || ((b[M_YEAR]&0xf) > 0x9) ) ? 0 :
								(bcd(b[M_YEAR],0,99) + ( (b[M_HOURS]&0x40) ? 2100 : 2000));
	lpst->wMonth		= bcd(b[M_MONTH],1,12);
	lpst->wDayOfWeek	= bcd(b[M_WEEKDAY],1,7);
	lpst->wDay			= bcd(b[M_DAY],1,31);
	lpst->wHour			= bcd(b[M_HOURS]&0x3f,0,23);
	lpst->wMinute		= bcd(b[M_MINUTES],0,59);
	lpst->wSecond		= bcd(b[M_SECONDS]&0x7f,0,59);
	lpst->wMilliseconds = bcd(b[M_HUNDREDTHS],0,99)*10;

	if (b[M_SECONDS] & 0x80) {
		//oscillator is stopped, restart it
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_SECONDS)) return -1;
		if (WriteI2C(b[M_SECONDS] & ~0x80)) return -1;			//clear stop bit
		I2CStop();
	}

/*
	printf( "M41T81S_GetTime: Year:%u, Month:%u, Day:%u, Hour:%u, Minute:%u, second:%u, milli:%u\n",
	        lpst->wYear, lpst->wMonth,lpst->wDay, lpst->wHour, lpst->wMinute, lpst->wSecond,lpst->wMilliseconds );
*/
	if (flags & MF_OF) {
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_FLAGS)) return -1;
		if (WriteI2C(flags & ~MF_OF)) return -1;			//clear oscillator failed bit
		I2CStop();
	}
	return 0;
}

static int SetTime(LPSYSTEMTIME lpst)
{
	unsigned char b[M_YEAR+1];
	unsigned char flags;
	int i;
	if (lpst->wYear < 2004) {
		lpst->wYear = 2004;	//don't allow it to be set in the far past.
		lpst->wMonth = 1;
		lpst->wDay = 1;
		lpst->wDayOfWeek = 4;	//Thursday Jan. 1, 2004
	}

/*	printf( "M41T81S_SetTime: Year:%u, Month:%u, Day:%u, Hour:%u, Minute:%u, second:%u, milli:%u\n",
	        lpst->wYear, lpst->wMonth,lpst->wDay, lpst->wHour, lpst->wMinute, lpst->wSecond,lpst->wMilliseconds );
*/
	b[M_HUNDREDTHS] = 0;		//toBcd(lpst->wMilliseconds/10);
	b[M_SECONDS]	= toBcd(lpst->wSecond);
	b[M_MINUTES]	= toBcd(lpst->wMinute);
	b[M_HOURS]	= toBcd(lpst->wHour) | ( ((lpst->wYear % 200) >= 100) ? 0xc0 : 0x80);
	b[M_WEEKDAY]	= (unsigned char)lpst->wDayOfWeek;
	b[M_DAY]	= toBcd(lpst->wDay);
	b[M_MONTH]	= toBcd(lpst->wMonth);
	b[M_YEAR]	= toBcd( lpst->wYear % 100); 

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_HUNDREDTHS)) return -1;

	for (i=M_HUNDREDTHS; i<=M_YEAR; i++) {
		if (WriteI2C(b[i])) return -1;
	}
	I2CStop();

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_FLAGS))	return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;
	flags = (unsigned char)ReadI2C(0);	//read flags
	I2CStop();

/*	printf( "M41T81S_SetTime: Year:%u, Month:%u, Day:%u, Weekday:%u, Hour:%u, Minute:%u, second:%u flags:%u\n",
		lpst->wYear, lpst->wMonth, lpst->wDay, lpst->wDayOfWeek, lpst->wHour, lpst->wMinute, lpst->wSecond, flags );
*/
	
	if (flags & MF_OF) {
		if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
		if (WriteI2C(M_FLAGS)) return -1;
		if (WriteI2C(flags & ~MF_OF)) return -1;			//clear oscillator failed bit
		I2CStop();
	}
	return 0;
}

int M41T81S_GetTime(LPSYSTEMTIME lpst)
{
	int ret;
	int reg =  (READ_SM501_REG(SMIR_POWER_MODE_CONTROL) & 1) ? SMIR_PWRM1_GATE : SMIR_PWRM0_GATE;
	int gate = READ_SM501_REG(reg);
	if ((gate & 0x40)==0) STUFF_SM501_REG(reg, gate | 0x40);
	STUFF_SM501_REG(SMIR_GPIO_32_63_CONTROL, 
			READ_SM501_REG(SMIR_GPIO_32_63_CONTROL) 
			& ~(GPBIT_MASK(I2C_CLK)|GPBIT_MASK(I2C_DATA)));	// set as gpio controlled
	udelay(LongBusWait);
	ret = GetTime(lpst);
	udelay(I2CWait);
	SetDir_ClkData(INPUT,INPUT);		//high clock, high data, just for safety, should be input already
	if ((gate & 0x40)==0){
		STUFF_SM501_REG( reg,
                		 READ_SM501_REG( reg )
                                 & ~0x40 );	//disable gpio if was originally
	}

        return ret;
}
int M41T81S_SetTime(LPSYSTEMTIME lpst)
{
	int ret;
	int reg =  (READ_SM501_REG(SMIR_POWER_MODE_CONTROL) & 1) ? SMIR_PWRM1_GATE : SMIR_PWRM0_GATE;
	int gate = READ_SM501_REG(reg);
	if ((gate & 0x40)==0) STUFF_SM501_REG(reg, gate | 0x40);
	STUFF_SM501_REG(SMIR_GPIO_32_63_CONTROL, 
			READ_SM501_REG(SMIR_GPIO_32_63_CONTROL) 
			& ~(GPBIT_MASK(I2C_CLK)|GPBIT_MASK(I2C_DATA)));	// set as gpio controlled
	udelay(LongBusWait);
	ret = SetTime(lpst);
	udelay(I2CWait);
	SetDir_ClkData(INPUT,INPUT);		//high clock, high data, just for safety, should be input already
	if ((gate & 0x40)==0){
		STUFF_SM501_REG( reg,
                		 READ_SM501_REG( reg )
                                 & ~0x40 );	//disable gpio if was originally
	}

	return ret;
}

static int badTime( LPSYSTEMTIME lpst )
{
   return ( 1 > lpst->wMonth)
          ||
          ( 12 < lpst->wMonth)
          ||
          ( 1 > lpst->wDay)
          ||
          ( 12 < lpst->wDay)
          ||
          ( 24 <= lpst->wHour)
          ||
          ( 60 <= lpst->wMinute)
          ||
          ( 60 <= lpst->wSecond)
          ||
          ( 1000 <= lpst->wMilliseconds );
}

static unsigned diffMs( LPSYSTEMTIME lpst1,
                        LPSYSTEMTIME lpst2 )
{
   return ( (long)lpst2->wMilliseconds-(long)lpst1->wMilliseconds )
        + ( (long)lpst2->wSecond-(long)lpst1->wSecond)*1000
        + ( (long)lpst2->wMinute-(long)lpst1->wMinute)*60000
        + ( (long)lpst2->wHour-(long)lpst1->wHour)*3600000 ;
}

static void printTime( LPSYSTEMTIME t )
{
   printf( "%04u-%02u-%02u %02u:%02u:%02u.%02u",
           t->wYear, t->wMonth, t->wDay,
           t->wHour, t->wMinute, t->wSecond, t->wMilliseconds/10 );
}

// returns zero for success
static int parseTime( LPSYSTEMTIME t,
                      char const  *dateString,
                      char const  *timeString )
{
   int rval = 1 ;
   char *nextIn ;
   unsigned long inVal = simple_strtoul( dateString, &nextIn, 10 );
   if( ( 0 == inVal ) || ( 2999 < inVal ) || ( '-' != *nextIn ) )
      goto bail ;

   nextIn++ ;
   t->wYear = inVal ;

   inVal = simple_strtoul( nextIn, &nextIn, 10 );
   if( ( 0 == inVal ) || ( 12 < inVal ) || ( '-' != *nextIn ) )
      goto bail ;

   nextIn++ ;
   t->wMonth = inVal ;

   inVal = simple_strtoul( nextIn, &nextIn, 10 );
   if( ( 0 == inVal ) || ( 31 < inVal ) || ( '\0' != *nextIn ) )
      goto bail ;

   nextIn++ ;
   t->wDay = inVal ;

   t->wHour = 0 ; t->wMinute = 0 ; t->wSecond = 0 ; t->wMilliseconds = 0 ;

   if( 0 != timeString )
   {
      inVal = simple_strtoul( timeString, &nextIn, 10 );
      if( ( 23 < inVal ) || ( ':' != *nextIn ) )
         goto bail ;
   
      nextIn++ ;
      t->wHour = inVal ;
   
      inVal = simple_strtoul( nextIn, &nextIn, 10 );
      if( ( 59 < inVal ) || ( ':' != *nextIn ) )
         goto bail ;
   
      nextIn++ ;
      t->wMinute = inVal ;
      
      inVal = simple_strtoul( nextIn, &nextIn, 10 );
      if( ( 59 < inVal ) || ( '.' != *nextIn ) )
         goto bail ;
   
      nextIn++ ;
      t->wSecond = inVal ;
   
      inVal = simple_strtoul( nextIn, &nextIn, 10 );
      if( ( 99 < inVal ) || ( '\0' != *nextIn ) )
         goto bail ;

      nextIn++ ;
      t->wMilliseconds = inVal*10 ;
      rval = 0 ;
   }
   else
      rval = 0 ;

bail:
   return rval ;
}

int do_rtc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   int rval = 1 ;

   if( 1 == argc )
   {
      SYSTEMTIME t ;
      int rval = M41T81S_GetTime( &t );
      if( 0 == rval )
      {
         printTime(&t);
      }
   }
   else if( ( 2 == argc ) && ( 0 == strcmp( "test", argv[1] ) ) )
   {
      SYSTEMTIME t ;
      int rval = M41T81S_GetTime( &t );
      if( 0 == rval ){
         if( badTime(&t) ){
            printf( "time not initialized...initializing\n" );
            t.wYear = 2006 ;
            t.wMonth = 6 ;
            t.wDay   = 3 ;
            memset( &t, 0, sizeof(t) );
            rval = M41T81S_SetTime(&t);
         }
      }
      
      if( 0 == rval )
      {
         SYSTEMTIME t2 ;
         udelay( 1000000 ); // wait a sec
         rval = M41T81S_GetTime( &t2 );
         if( 0 == rval )
         {
            unsigned diff = diffMs(&t,&t2);

            if( ( diff < 800 ) || ( diff > 1200 ) )
            {
               rval = 1 ; // clock not moving or moving too fast (bad oscillator?)
               if( 0 == diff )
                  printf( "check RTC oscillator\n" );
#ifdef DEBUG                  
               printf( "rtc test failed!\n"
                       "difftime: %u\n", diff );
               printf( "t1: " ); printTime( &t ); printf( "\n" );
               printf( "t2: " ); printTime( &t2 ); printf( "\n" );
#endif
            }
         }
      }
   }
   else if( ( 2 == argc ) || ( 3 == argc ) )
   {
      SYSTEMTIME t ;
      if( 0 == parseTime( &t, argv[1], ( 3 == argc ) ? argv[2] : 0 ) )
      {
         rval = M41T81S_SetTime(&t);
      }
      else
         printf( "Invalid time format: use YYYY-MM-DD HH:MM:SS\n" );
   }
   
   return rval ;
}

U_BOOT_CMD(
	rtc,	127,	0,	do_rtc,
	"rtc      - get/set/test RTC\n",
	NULL
);


static int do_battery (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   unsigned char flags, fault ;

	int reg =  (READ_SM501_REG(SMIR_POWER_MODE_CONTROL) & 1) ? SMIR_PWRM1_GATE : SMIR_PWRM0_GATE;
	int gate = READ_SM501_REG(reg);
	if ((gate & 0x40)==0) STUFF_SM501_REG(reg, gate | 0x40);
	STUFF_SM501_REG(SMIR_GPIO_32_63_CONTROL, 
			READ_SM501_REG(SMIR_GPIO_32_63_CONTROL) 
			& ~(GPBIT_MASK(I2C_CLK)|GPBIT_MASK(I2C_DATA)));	// set as gpio controlled
	udelay(LongBusWait);

   I2CInit();		//send a bunch of stops

	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_ALARM_HOUR)) return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;

	fault = (unsigned char)ReadI2C(0);	//read Halt bit
	I2CStop();

   if( fault & M_HALT )
      printf( "rtc halted = %02x\n", fault );

   if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
	if (WriteI2C(M_FLAGS))	return -1;
	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;
	flags = (unsigned char)ReadI2C(0);	//read flags
	I2CStop();

   printf( "rtc flags = %02x\n", flags );
   if( flags & MF_BL )
      printf( "---> battery low\n" );
   if( flags & MF_OF )
      printf( "---> oscillator failure\n" );
   
   if( 2 <= argc ){
      char *nextIn ;
      unsigned long reg = simple_strtoul( argv[1], &nextIn, 16 );
      if( M_SQW >= reg ){
         unsigned char regVal ;
      	if (I2CStart(M41T81S_SlaveAddrWrite)) return -1;
      	if (WriteI2C(reg)) return -1;
      	if (I2CStart(M41T81S_SlaveAddrRead)) return -1;
      	I2CStop();
      	regVal = (unsigned char)ReadI2C(0);	//read flags
         printf( "battery register 0x%02x == 0x%02x\n", reg, regVal );
      }
   }

   return (0 != (fault & M_HALT)) || (0 != (flags & (MF_BL|MF_OF) ));

}


U_BOOT_CMD(
	battery,	127,	0,	do_battery,
	"battery - Check battery low pin\n",
	NULL
);


