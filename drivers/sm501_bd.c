#include <common.h>

#ifdef CONFIG_SM501
#include <config.h>
#include <common.h>
#include <sm501_bd.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <lcd.h>
#include <lcd_panels.h>
#include <asm/arch/pxa-regs.h>

#define FASTCLOCK1 0x291A0201		//faster pixel clock:     P2S = 1, P2 =  9  (/6)      ( panel source 1, divide by 6)
                                 //                        V2S = 1, V2 = 10 (/12)      ( crt source 1, divide by 12)
                                 //                        M2S = 0, MR =  2  (/4)      (sdram source 1, divide by 4)
                                 //                        M1S = 0, MR =  1  (/2)
                                 // miscTimReg[5:4] == 0 (336 MHz)
                                 //                   / 6 == 56 MHz
                                 //
#define FASTCLOCK2 0x291A0201
#define FASTCLOCK3 0x00080800

#define SLOWCLOCK1 0x0A1A0201        //slow pixel clock:       P2S = 0, P2 = 10 (/12)      ( panel source 0, divide by 12)  
                                     //                        V2S = 1, V2 = 10 (/12)      ( crt source 1, divide by 12)   
                                     //                        M2S = 0, MR =  2  (/4)      (sdram source 1, divide by 4)   
                                     //                        M1S = 0, MR =  1  (/2)                                      
                                     // miscTimReg[5:4] == 0 (288 MHz)                                                     
                                     //                   / 12 == 24 MHz                                                    
                                     //
#define SLOWCLOCK2 0x0A1A0A09
#define SLOWCLOCK3 0x00090900

#if 1
#define SM501_BASE PXA_CS3_PHYS			// Neon, Neon-270 Enc
#else
#define SM501_BASE PXA_CS1_PHYS			// Neon-270
#endif

#define REG_BASE SM501_BASE+0x03E00000

unsigned long const fbStart = SM501_BASE ;
unsigned long const fbMax   = 0x00800000 ;    //

unsigned long const mmioStart  = REG_BASE ;
unsigned long const mmioLength = 0x00200000 ;
unsigned long const lcdPaletteRegs = REG_BASE+0x80400 ;
unsigned long const crtPaletteRegs = REG_BASE+0x80C00 ;

#ifdef CONFIG_LCD
unsigned long paletteRegs = lcdPaletteRegs ;
#endif

const unsigned int sm501_list1[]={
   REG_BASE,
	0x00100000,0x00001402,0x00000000,0x00000000,0x24B927C0,0x05146732,0x40715128,0x00000000,
	0x00000000,0x00180002,0x00000002,0x00000002,0x00000000,0x00000000,0x0000001F,0x291A0201,
	0x0000001F,0x291A0201,0x00000007,0x291A0201,0x00018000,0x00000000,0x00000000,0x00000000,
	0x050100A0,0x00000000,0x00080800};

const unsigned int sm501_list2[]={ REG_BASE+0x80000,
/* 80000 dispctrl  */ 	0x00013100,             // 0f0d0105
/* 80004 pan       */   0x00000000,
/* 80008 colorkey  */   0x00000000,
/* 8000C fbaddr    */   0x00000000,
/* 80010 offsetww  */   0x00000000, // ((LCD_XRES)<<16)+(LCD_XRES),
/* 80014 fbwidth   */   0x00000000, // (LCD_XRES<<16),
/* 80018 fbheight  */   0x00000000, // (LCD_YRES<<16),
/* 8001C tllocate  */   0x00000000,
/* 80020 brlocate  */ 	0x00000000, // ((LCD_YRES-1)<<16)+(LCD_XRES-1),
/* 80024 htotal    */ 	0x00000000, // ((LCD_BEGIN_OF_LINE_WAIT_COUNT+
/*                 */               //   LCD_XRES+
/*                 */               //   LCD_END_OF_LINE_WAIT_COUNT+
/*                 */               //   LCD_HORIZONTAL_SYNC_PULSE_WIDTH-1)<<16)
/*                 */               // +(LCD_XRES-1),
/* 80028 hsync     */ 	0x00000000, // (LCD_HORIZONTAL_SYNC_PULSE_WIDTH<<16)
/*                 */               // +(LCD_XRES+LCD_BEGIN_OF_LINE_WAIT_COUNT-1),
/* 8002C vtotal    */ 	0x00000000, // ((LCD_BEGIN_FRAME_WAIT_COUNT+
/*                 */               //   LCD_YRES+
/*                 */               //   LCD_END_OF_FRAME_WAIT_COUNT+
/*                 */               //   LCD_VERTICAL_SYNC_PULSE_WIDTH-1)<<16)
/*                 */               // +(LCD_YRES-1),
/* 80030 vsync     */ 	0x00000000, // (LCD_VERTICAL_SYNC_PULSE_WIDTH<<16)
/*                 */               // +(LCD_YRES+LCD_BEGIN_FRAME_WAIT_COUNT-1)
};

static unsigned const miscCtrl       = 0x00000004 ;
static unsigned const curClockReg    = 0x0000003C ;
static unsigned const pm0ClockReg    = 0x00000044 ;
static unsigned const pm1ClockReg    = 0x0000004C ;
static unsigned const miscTimReg     = 0x00000068 ;

static unsigned const dispctrlReg    = 0x00080000 ;
static unsigned const fbAddrReg      = 0x0008000c ;
static unsigned const offsetReg      = 0x00080010 ;  // ((xres)<<16)+(xres),
static unsigned const fbWidthReg     = 0x00080014 ;  // (xres<<16), 
static unsigned const fbHeightReg    = 0x00080018 ;  // (yres<<16), 
static unsigned const brLocateReg    = 0x00080020 ;  // ((yres-1)<<16)+(xres-1),
static unsigned const hTotalReg      = 0x00080024 ;  // (left_margin+xres+right_margin+hsync_len-1) << 16
                                                     // + xres
static unsigned const hSyncReg       = 0x00080028 ;  //  (hsync_len<<16)  + (xres+left_margin-1)
static unsigned const vTotalReg      = 0x0008002c ;  // (top_margin+yres+lower_margin+vsync_len-1) << 16
                                                     // + yres-1
static unsigned const vSyncReg       = 0x00080030 ;  // (vsync_len<<16) + yres+top_margin-1

/*
 * CRT regs
 */
static unsigned const crtctrlReg     = 0x00080200 ;
static unsigned const crtFbAddrReg   = 0x00080204 ;
static unsigned const crtFbOffsReg   = 0x00080208 ;
static unsigned const crtFbHTotReg   = 0x0008020C ;
static unsigned const crtFbHSynReg   = 0x00080210 ;
static unsigned const crtFbVTotReg   = 0x00080214 ;
static unsigned const crtFbVSynReg   = 0x00080218 ;

#define DISPCRTL_ENABLE 4
#define CRTCRTL_ENABLE 4

#define CLOCK_ACTIVEHIGH 0
#define CLOCK_ACTIVELOW  (1<<14)
#define CLOCK_ACTIVEMASK (1<<14)

#define LCDTYPE_TFT    0
#define LCDTYPE_STN12  (3<<18)
#define LCDTYPE_MASK   (3<<18)
#define LCD_SIGNAL_ENABLE (0xf<<24)

#define READREG( addr ) *( (unsigned long volatile *)((addr)+mmioStart) )
#define STUFFREG( addr, value ) *( (unsigned long volatile *)((addr)+mmioStart) ) = (value)

const unsigned int sm501_list3[]={REG_BASE+0x80040,
	0x00010000,0x0703E360,0x00200400,0x00A81330,0x0385009C,0x02200240,0x00000000,0x00000000,
	0x00EDEDED,0x089C4040,0x0031E3B0};
 
const unsigned int sm501_list4[]={REG_BASE+0x80080,
	0x00010000,0x05121880,0x28800C00,0x00108030,0x02090040,0x00840050,0x00000000,0x00000000,
	0x0141A200,0x020A0802,0x0088D109,0x20820040,0x10800000,0x30029200,0x00080821,0x01010400,
	0x44000120,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000};

const unsigned int sm501_list5[]={REG_BASE+0x800f0,
	0x0070F800,0x00780140,0x00000000,0x0000FFFF,0x00010000};

struct itemEntry {
	const int cnt;
	const unsigned int* p;
};

const struct itemEntry lists[] = {
	{sizeof(sm501_list1)>>2,sm501_list1},
	{sizeof(sm501_list2)>>2,sm501_list2},
	{sizeof(sm501_list3)>>2,sm501_list3},
	{sizeof(sm501_list4)>>2,sm501_list4},
	{sizeof(sm501_list5)>>2,sm501_list5}
};

void disable_sm501( void )
{
   STUFFREG(dispctrlReg,READREG(dispctrlReg)& ~(DISPCRTL_ENABLE));
   STUFFREG(crtctrlReg,READREG(crtctrlReg)& ~(CRTCRTL_ENABLE));
}

#ifdef CONFIG_LCD
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;		/* Start of console buffer	*/

short console_col;
short console_row;

void lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
   unsigned long *const palette = (unsigned long *)paletteRegs ;
   unsigned long const rgb = ((unsigned long)red ) << 16
                           | ((unsigned long)green ) << 8 
                           | blue ;
   palette[regno] = rgb ;
}
//each entry is in ARGB format, alpha high byte, blue low byte
void lcd_SetPalette(ulong* palette,int colorCnt)
{
	ulong  *cmap = (ulong *)paletteRegs;
	while (colorCnt--) {
		*cmap++ = *palette++ & 0x00ffffff;
	}
}

void lcd_ctrl_init	(void *lcdbase)
{
	unsigned short *fbMem;
   char *panelName ;

	unsigned long val=0;
	const struct itemEntry* l = lists;
	int count = sizeof(lists)/sizeof(struct itemEntry);
	
	while (count) {
		int cnt = l->cnt-1;
		const unsigned long* p = (unsigned long*)l->p;
		volatile unsigned int* reg = (unsigned int*)(*p++);
//		printf( "set regs: %p, cnt:%x, from %p, l:%p\n", reg, cnt, p,l );
//		while (reg==0) {
//		}
		
		while (cnt) {
			val = *p++;
//			printf( "set reg: %p = %x from %p\n", reg, val, p );
			*reg++ = val;
			cnt--;
		}
		count--;
		l++;
	}
//	printf( "sm501 init middle\n");

   panelName = getenv( "panel" );
//	printf( "after getenv\n");
   if( panelName )
   {
      struct lcd_panel_info_t const *panel ;
      panel = find_lcd_panel( panelName );
      if( panel )
      {
         printf( "panel %s found: %u x %u\n", panelName, panel->xres, panel->yres );
//	printf( "before set_lcd_panel\n");
         set_lcd_panel( panel ); 
//	printf( "after set_lcd_panel\n");
      }
      else
         printf( "panel %s not found\n", panelName );
   }

   fbMem = (unsigned short *)fbStart ;
   lcd_base = fbMem ;

/*
Settings for Hitachi 5.7
		PANEL_HORIZONTAL_TOTAL, 01c00160);
		PANEL_HORIZONTAL_SYNC, 00400161);
		PANEL_VERTICAL_TOTAL, 0x010800f0);
		PANEL_VERTICAL_SYNC, 0x00020104);
      
In bdlogo.bmp - offset 436 is pixel data      
   STUFFREG( hTotalReg,      0x01800140 );
   STUFFREG( hSyncReg,       0x0008014f );
   STUFFREG( vTotalReg,      0x010700F0 );
   STUFFREG( vSyncReg,       0x00020100 );
*/
}

void lcd_enable	(void)
{
}

#endif

#define BIT29 (1<<29)
#define BIT20 (1<<20)

static void setClockReg( unsigned reg, unsigned long value )
{
   unsigned oldValue = READREG( reg );
   if( (oldValue & BIT29) != (value&BIT29) )
   {
      oldValue = (oldValue & (~BIT29))
               | (value & BIT29);
      STUFFREG( reg, oldValue );
      udelay(16000);
   } // LCD clock source

   if( (oldValue & BIT20) != (value&BIT20) )
   {
      oldValue = (oldValue & (~BIT20))
               | (value & BIT20);
      STUFFREG( reg, oldValue );
      udelay(16000);
   } // CRT clock source

   oldValue = READREG( reg );
   if( oldValue != value ){
      STUFFREG( reg, value );
      udelay(16000);
   }
}

vidinfo_t panel_info = {
	vl_col:		320,		//this is corrected in SetPanelInfo
	vl_row:		240,
	vl_bpix:    LCD_BPP,
	vl_lcd_line_length: (320 * NBITS(LCD_BPP) ) >> 3
};

static void SetPanelInfo(struct lcd_panel_info_t const *panel)
{
   panel_info.vl_col = panel->xres;
   panel_info.vl_row = panel->yres;
   panel_info.vl_bpix = LCD_BPP;
   panel_info.vl_lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) >> 3;
   printf("panel: %ix%ix%i\n",panel_info.vl_col,panel_info.vl_row,(1<<panel_info.vl_bpix));
}

static unsigned long clockRegs[] = {
   SLOWCLOCK1, SLOWCLOCK2,
   FASTCLOCK1, FASTCLOCK2
};

static unsigned const numClockRegs = sizeof(clockRegs)/sizeof(clockRegs[0])/2 ;

/*
 * The following tables were built by screen-scraping and sorting
 * the tables in the SM501 manual:
 *
 * Fields are:
 *    frequency in Hz
 *    clock source (0 == 288MHz, 1 == 366 MHz)
 *    select bits (5 bits for panels, 4 bits for CRTs)
 */ 
#define ENTRIESPERFREQ 3
unsigned long const panelFrequencies[] = {
      450000/2, 0<<29, 0x17<<24, 
      525000/2, 1<<29, 0x17<<24, 
      750000/2, 0<<29, 0x0f<<24, 
      875000/2, 1<<29, 0x0f<<24, 
      900000/2, 0<<29, 0x16<<24, 
     1050000/2, 1<<29, 0x16<<24, 
     1500000/2, 0<<29, 0x0e<<24, 
     1750000/2, 1<<29, 0x0e<<24, 
     1800000/2, 0<<29, 0x15<<24, 
     2100000/2, 1<<29, 0x15<<24, 
     2250000/2, 0<<29, 0x07<<24, 
     2625000/2, 1<<29, 0x07<<24, 
     3000000/2, 0<<29, 0x0d<<24, 
     3500000/2, 1<<29, 0x0d<<24, 
     3600000/2, 0<<29, 0x14<<24, 
     4200000/2, 1<<29, 0x14<<24, 
     4500000/2, 0<<29, 0x06<<24, 
     5250000/2, 1<<29, 0x06<<24, 
     6000000/2, 0<<29, 0x0c<<24, 
     7000000/2, 1<<29, 0x0c<<24, 
     7200000/2, 0<<29, 0x13<<24, 
     8400000/2, 1<<29, 0x13<<24, 
     9000000/2, 0<<29, 0x05<<24, 
    10500000/2, 1<<29, 0x05<<24, 
    12000000/2, 0<<29, 0x0b<<24, 
    14000000/2, 1<<29, 0x0b<<24, 
    14400000/2, 0<<29, 0x12<<24, 
    16800000/2, 1<<29, 0x12<<24, 
    18000000/2, 0<<29, 0x04<<24, 
    21000000/2, 1<<29, 0x04<<24, 
    24000000/2, 0<<29, 0x0a<<24, 
    28000000/2, 1<<29, 0x0a<<24, 
    28800000/2, 0<<29, 0x11<<24, 
    33600000/2, 1<<29, 0x11<<24, 
    36000000/2, 0<<29, 0x03<<24, 
    42000000/2, 1<<29, 0x03<<24, 
    48000000/2, 0<<29, 0x09<<24, 
    56000000/2, 1<<29, 0x09<<24, 
    57600000/2, 0<<29, 0x10<<24, 
    67200000/2, 1<<29, 0x10<<24, 
    72000000/2, 0<<29, 0x02<<24, 
    84000000/2, 1<<29, 0x02<<24, 
    96000000/2, 0<<29, 0x08<<24, 
   112000000/2, 1<<29, 0x08<<24, 
   144000000/2, 0<<29, 0x01<<24, 
   168000000/2, 1<<29, 0x01<<24, 
   288000000/2, 0<<29, 0x00<<24, 
   336000000/2, 1<<29, 0x00<<24 
};
#define numPanelFrequencies (sizeof(panelFrequencies)/sizeof(panelFrequencies[0])/ENTRIESPERFREQ)

unsigned long const crtFrequencies[] = {
      750000/2, 0<<20, 0x0f<<16, 
      875000/2, 1<<20, 0x0f<<16, 
     1500000/2, 0<<20, 0x0e<<16, 
     1750000/2, 1<<20, 0x0e<<16, 
     2250000/2, 0<<20, 0x07<<16, 
     2625000/2, 1<<20, 0x07<<16, 
     3000000/2, 0<<20, 0x0d<<16, 
     3500000/2, 1<<20, 0x0d<<16, 
     4500000/2, 0<<20, 0x06<<16, 
     5250000/2, 1<<20, 0x06<<16, 
     6000000/2, 0<<20, 0x0c<<16, 
     7000000/2, 1<<20, 0x0c<<16, 
     9000000/2, 0<<20, 0x05<<16, 
    10500000/2, 1<<20, 0x05<<16, 
    12000000/2, 0<<20, 0x0b<<16, 
    14000000/2, 1<<20, 0x0b<<16, 
    18000000/2, 0<<20, 0x04<<16, 
    21000000/2, 1<<20, 0x04<<16, 
    24000000/2, 0<<20, 0x0a<<16, 
    28000000/2, 1<<20, 0x0a<<16, 
    36000000/2, 0<<20, 0x03<<16, 
    42000000/2, 1<<20, 0x03<<16, 
    48000000/2, 0<<20, 0x09<<16, 
    56000000/2, 1<<20, 0x09<<16, 
    72000000/2, 0<<20, 0x02<<16, 
    84000000/2, 1<<20, 0x02<<16, 
    96000000/2, 0<<20, 0x08<<16, 
   112000000/2, 1<<20, 0x08<<16, 
   144000000/2, 0<<20, 0x01<<16, 
   168000000/2, 1<<20, 0x01<<16, 
   288000000/2, 0<<20, 0x00<<16, 
   336000000/2, 1<<20, 0x00<<16
};
#define numCrtFrequencies (sizeof(crtFrequencies)/sizeof(crtFrequencies[0])/ENTRIESPERFREQ)

unsigned long const * const frequencies[] = {
   panelFrequencies,
   crtFrequencies
};

unsigned const numFrequencies[] = {
   numPanelFrequencies,
   numCrtFrequencies
};

#define PANELCLOCKMASK  (0x3F<<24)
#define CRTCLOCKMASK    (0x1F<<16)

static unsigned long const clockMasks[] = {
   PANELCLOCKMASK,
   CRTCLOCKMASK
};

#define XGA_PIXELS (1024*768)

static void useFastRAM(void)
{
//   STUFFREG( pm0ClockReg, READREG(pm0ClockReg)|0x10 ); // use 336 MHz input
}


#ifdef CONFIG_LCD

static void updateCRT( unsigned long const           *freq,
                       struct lcd_panel_info_t const *panel )
{
   unsigned long reg ;
   unsigned long crtCtrl = 0x00010000 ; // FIFO 3 or more, disable CRT Timing - use lcd panel timings
   if ( !panel->pclk_redg ) crtCtrl |= (3<<14);     // horizontal and vertical phase
   STUFFREG( crtFbAddrReg, 0 );
   STUFFREG( crtFbOffsReg, ((panel->xres)<<16)+(panel->xres) );
   STUFFREG( crtFbHTotReg, (( panel->left_margin
                            +panel->xres
                            +panel->right_margin
                            +panel->hsync_len - 1) << 16 )
                          + panel->xres-1 );
   STUFFREG( crtFbHSynReg, (panel->hsync_len<<16)+ (panel->xres+panel->left_margin-1) );
   STUFFREG( crtFbVTotReg, (( panel->upper_margin
                            +panel->yres
                            +panel->lower_margin
                            +panel->vsync_len-1 ) << 16 )
                          + panel->yres-1 );
   STUFFREG( crtFbVSynReg,(panel->vsync_len<<16) 
                          + panel->yres+panel->upper_margin-1 );
   STUFFREG( crtctrlReg, crtCtrl );    // enable

   reg = READREG( miscCtrl ) & ~0x1000 ;
   STUFFREG( miscCtrl, reg );
}

void set_lcd_panel( struct lcd_panel_info_t const *panel )
{
   unsigned long dispctrl = READREG( dispctrlReg );
   dispctrl &= ~(CLOCK_ACTIVEMASK|LCDTYPE_MASK|LCD_SIGNAL_ENABLE|3);
   if( !panel->pclk_redg ) dispctrl |= CLOCK_ACTIVELOW ;

   if( !panel->active ) dispctrl |= LCDTYPE_STN12 ;
   if (!panel->crt) dispctrl |= LCD_SIGNAL_ENABLE;
   dispctrl |= 4;

   STUFFREG( offsetReg,   ((panel->xres)<<16)+(panel->xres) );
   STUFFREG( fbWidthReg,  (panel->xres<<16) );
   STUFFREG( fbHeightReg, (panel->yres<<16) ); 
   STUFFREG( brLocateReg, ((panel->yres-1)<<16)+(panel->xres-1) );
   STUFFREG( hTotalReg,   (( panel->left_margin
                            +panel->xres
                            +panel->right_margin
                            +panel->hsync_len - 1) << 16 )
                          + panel->xres-1 );
   STUFFREG( hSyncReg,    (panel->hsync_len<<16)+ (panel->xres+panel->left_margin-1) );
   STUFFREG( vTotalReg,   (( panel->upper_margin
                            +panel->yres
                            +panel->lower_margin
                            +panel->vsync_len-1 ) << 16 )
                          + panel->yres-1 );
   STUFFREG( vSyncReg,    (panel->vsync_len<<16) 
                          + panel->yres+panel->upper_margin-1 );

   if( panel->pixclock < numClockRegs )
   {
      unsigned long const *clk = clockRegs+(panel->pixclock*2);
      setClockReg( curClockReg, *clk );
      setClockReg( pm0ClockReg, *clk++ );
      setClockReg( pm1ClockReg, *clk );
   }
   else
   {
      int const isCRT = 0 ; // (0 != panel->crt);
      unsigned long reg ;
      unsigned long const *freq = frequencies[isCRT];
      unsigned const count = numFrequencies[isCRT];

      unsigned long f, diffl, diffh ;
      int i ;
      unsigned long low, high ;

      //
      // linear scan for closest frequency
      //
      for( i = 0 ; i < count ; i++, freq += ENTRIESPERFREQ )
      {
         if( *freq > panel->pixclock )
            break;
      }
      
      low  = (i > 0) 
             ? freq[0-ENTRIESPERFREQ]
             : 0 ;
      diffl = panel->pixclock - low ;

      high = (i < count ) 
             ? *freq
             : 0xFFFFFFFF ;
      diffh = high - panel->pixclock ;

      if( diffh < diffl )
      {
         f = high ;
      }
      else
      {
         f = low ;
         freq -= ENTRIESPERFREQ ;
      }

      reg = READREG( curClockReg );
      
      reg &= ~(clockMasks[isCRT]);

      // Clock source
      printf( "source 0x%08X, divisor 0x%08X\n", freq[1], freq[2] );
      reg |= freq[1];
      reg |= freq[2];

      setClockReg( curClockReg, reg );
      setClockReg( pm0ClockReg, reg );
      setClockReg( pm1ClockReg, reg );
      
      if (panel->crt) updateCRT( freq, panel );
         paletteRegs = lcdPaletteRegs ;
   }

   if(panel->xres*panel->yres > XGA_PIXELS)
      useFastRAM();

   STUFFREG( dispctrlReg, dispctrl );
   cur_lcd_panel = panel ;
   SetPanelInfo(panel);
}

void disable_lcd_panel( void )
{
   unsigned long dispctrl = READREG( dispctrlReg );
   unsigned long crtctrl = READREG( crtctrlReg );

   dispctrl &= ~(DISPCRTL_ENABLE);
   STUFFREG( dispctrlReg, dispctrl );

   crtctrl  &= ~(DISPCRTL_ENABLE);
   STUFFREG( crtctrlReg, crtctrl );
}

#endif

#ifdef CONFIG_LCD_MULTI
static int initialized = 0 ;

inline unsigned crtWidth( void )
{
   return READREG( crtFbOffsReg ) >> 16 ;
}

inline unsigned crtHeight( void )
{
   return ( READREG( crtFbVTotReg ) & 0x7ff ) + 1 ;
}

static void lcd_ctrl_init(void)
{
	unsigned long val=0;
	const struct itemEntry* l = lists;
	int count = sizeof(lists)/sizeof(struct itemEntry);
	
	while (count) {
		int cnt = l->cnt-1;
		const unsigned long* p = (unsigned long*)l->p;
		volatile unsigned int* reg = (unsigned int*)(*p++);
//		printf( "set regs: %p, cnt:%x, from %p, l:%p\n", reg, cnt, p,l );
//		while (reg==0) {
//		}
		
		while (cnt) {
			val = *p++;
//			printf( "set reg: %p = %x from %p\n", reg, val, p );
			*reg++ = val;
			cnt--;
		}
		count--;
		l++;
	}
   initialized = 1 ; 
}

static void sm501_lcd_set_palette(unsigned long *colors, unsigned colorCount)
{
   if( colorCount > 256 )
      colorCount = 256 ;
   memcpy((void *)lcdPaletteRegs,colors,colorCount*sizeof(colors[0]));
}

static unsigned long sm501_lcd_get_palette_color(unsigned char idx)
{
   return ((unsigned long volatile *)lcdPaletteRegs)[idx];
}

static void sm501_lcd_disable(void)
{
   unsigned long dispctrl = READREG( dispctrlReg );
   dispctrl &= ~(DISPCRTL_ENABLE); 
   STUFFREG( dispctrlReg, dispctrl );
}

/*
 * Return best frequency table entry
 */
static unsigned long const *findFrequency
   ( unsigned long        frequency,
     unsigned long const *table,
     unsigned             count )
{
   unsigned long f, diffl, diffh ;
   unsigned i ;
   unsigned long low, high ;
   
   //
   // linear scan for closest tableuency
   //
   for( i = 0 ; i < count ; i++, table += ENTRIESPERFREQ )
   {
      if( *table > frequency )
         break;
   }
   
   low  = (i > 0) 
          ? table[0-ENTRIESPERFREQ]
          : 0 ;
   diffl = frequency - low ;
   
   high = (i < count ) ? *table : 0xFFFFFFFF ;
   diffh = high - frequency ;
   
   if( diffh < diffl )
   {
      f = high ;
   }
   else
   {
      f = low ;
      table -= ENTRIESPERFREQ ;
   }

   printf( "frequency %lu -> %u, source 0x%08X, divisor 0x%08X\n", f, table[1], table[2] );
   return table ;
}

static unsigned long const bwPalette[] = {
   0xffffff,
   0
};

void lcd_init_fb( struct lcd_t *lcd )
{
   lcd->set_palette(bwPalette,sizeof(bwPalette)/sizeof(bwPalette[0]));
   memset(lcd->fbAddr,0,lcd->fbMemSize);
   lcd->fg = 1 ;
   lcd->bg = 0 ;
}

void init_sm501_lcd( struct lcd_t *lcd )
{
   struct lcd_panel_info_t *panel = &lcd->info ;
   unsigned long reg_value ;
    
   if( !initialized )
      lcd_ctrl_init();

   reg_value = READREG( dispctrlReg );
   reg_value &= ~(CLOCK_ACTIVEMASK|LCDTYPE_MASK|LCD_SIGNAL_ENABLE|3);
   if( !panel->pclk_redg ) reg_value |= CLOCK_ACTIVELOW ;
   if( !panel->active ) reg_value |= LCDTYPE_STN12 ;
   if (!panel->crt) reg_value |= LCD_SIGNAL_ENABLE;
   reg_value |= 4;

   STUFFREG( offsetReg,   ((panel->xres)<<16)+(panel->xres) );
   STUFFREG( fbWidthReg,  (panel->xres<<16) );
   STUFFREG( fbHeightReg, (panel->yres<<16) ); 
   STUFFREG( brLocateReg, ((panel->yres-1)<<16)+(panel->xres-1) );
   STUFFREG( hTotalReg,   (( panel->left_margin
                            +panel->xres
                            +panel->right_margin
                            +panel->hsync_len - 1) << 16 )
                          + panel->xres-1 );
   STUFFREG( hSyncReg,    (panel->hsync_len<<16)+ (panel->xres+panel->left_margin-1) );
   STUFFREG( vTotalReg,   (( panel->upper_margin
                            +panel->yres
                            +panel->lower_margin
                            +panel->vsync_len-1 ) << 16 )
                          + panel->yres-1 );
   STUFFREG( vSyncReg,    (panel->vsync_len<<16) 
                          + panel->yres+panel->upper_margin-1 );

   if( panel->pixclock < numClockRegs )
   {
      unsigned long const *clk = clockRegs+(panel->pixclock*2);
      setClockReg( curClockReg, *clk );
      setClockReg( pm0ClockReg, *clk++ );
      setClockReg( pm1ClockReg, *clk );
   }
   else
   {
      unsigned long reg ;
      unsigned long const *tableEntry = findFrequency( panel->pixclock, panelFrequencies, numPanelFrequencies );

      reg = READREG( curClockReg ) & ~(PANELCLOCKMASK);

      reg |= tableEntry[1];
      reg |= tableEntry[2];

      setClockReg( curClockReg, reg );
      setClockReg( pm0ClockReg, reg );
      setClockReg( pm1ClockReg, reg );
   }

   if(panel->xres*panel->yres > XGA_PIXELS)
      useFastRAM();

   STUFFREG( dispctrlReg, reg_value );

   /* Is CRT already enabled? */
   reg_value = READREG(crtctrlReg);
   if( 0 == ( reg_value & CRTCRTL_ENABLE ) ){
      lcd->fbAddr = (void *)fbStart ;
   } else {
      unsigned const w = crtWidth(); 
      unsigned const h = crtHeight();
      lcd->fbAddr = (void *)(fbStart + w*h );
   }
   printf( "SM-501 LCD at %p\n", lcd->fbAddr );
   STUFFREG(fbAddrReg, (unsigned long)lcd->fbAddr - fbStart );
   lcd->fbMemSize = panel->xres*panel->yres ;

   lcd->set_palette = sm501_lcd_set_palette ;
   lcd->get_palette_color = sm501_lcd_get_palette_color ;
   lcd->disable = sm501_lcd_disable ;
   lcd_init_fb(lcd);
}

static void sm501_crt_set_palette(unsigned long *colors, unsigned colorCount)
{
   if( colorCount > 256 )
      colorCount = 256 ;
   memcpy((void *)crtPaletteRegs,colors,colorCount*sizeof(colors[0]));
}

static unsigned long sm501_crt_get_palette_color(unsigned char idx)
{
   return ((unsigned long volatile *)crtPaletteRegs)[idx];
}

static void sm501_crt_disable(void)
{
   unsigned long reg = READREG( crtctrlReg );
   reg &= ~(CRTCRTL_ENABLE); 
   STUFFREG( crtctrlReg, reg );
}

void init_sm501_crt( struct lcd_t *lcd )
{
   struct lcd_panel_info_t *panel = &lcd->info ;
   unsigned long reg ;
   unsigned long crtCtrl = 0x00010304 ; // FIFO 3 or more, enable CRT timing and data, 8-bit indexed
   unsigned long const *tableEntry ;

   if( !initialized )
      lcd_ctrl_init();

   printf( "Initialize SM-501 CRT to %ux%u here\n", lcd->info.xres, lcd->info.yres );
   if ( !panel->pclk_redg ) crtCtrl |= (3<<14);     // horizontal and vertical phase
   STUFFREG( crtFbAddrReg, 0 );
   STUFFREG( crtFbOffsReg, ((panel->xres)<<16)+(panel->xres) );
   STUFFREG( crtFbHTotReg, (( panel->left_margin
                            +panel->xres
                            +panel->right_margin
                            +panel->hsync_len - 1) << 16 )
                          + panel->xres-1 );
   STUFFREG( crtFbHSynReg, (panel->hsync_len<<16)+ (panel->xres+panel->left_margin-1) );
   STUFFREG( crtFbVTotReg, (( panel->upper_margin
                            +panel->yres
                            +panel->lower_margin
                            +panel->vsync_len-1 ) << 16 )
                          + panel->yres-1 );
   STUFFREG( crtFbVSynReg,(panel->vsync_len<<16) 
                          + panel->yres+panel->upper_margin-1 );
   STUFFREG( crtctrlReg, crtCtrl );    // enable

   reg = READREG( miscCtrl ) & ~0x1000 ;
   STUFFREG( miscCtrl, reg );

   /* Is panel already enabled? */
   reg = READREG(dispctrlReg);
   if( 0 == ( reg & DISPCRTL_ENABLE ) ){
      lcd->fbAddr = (void *)fbStart ;
   } else {
      unsigned const w = READREG( fbWidthReg ) >> 16 ;
      unsigned const h = READREG( fbHeightReg ) >> 16 ;
      lcd->fbAddr = (void *)(fbStart + w*h );
   }
   printf( "SM-501 CRT at %p\n", lcd->fbAddr );
   STUFFREG(crtFbAddrReg, (unsigned long)lcd->fbAddr - fbStart );

   reg = READREG( curClockReg ) & ~(CRTCLOCKMASK);
   tableEntry = findFrequency( panel->pixclock, crtFrequencies, numCrtFrequencies );

   reg |= tableEntry[1];
   reg |= tableEntry[2];

   setClockReg( curClockReg, reg );
   setClockReg( pm0ClockReg, reg );
   setClockReg( pm1ClockReg, reg );

   if(panel->xres*panel->yres > XGA_PIXELS)
      useFastRAM();
   
   lcd->fbMemSize = panel->xres*panel->yres ;

   lcd->set_palette = sm501_crt_set_palette ;
   lcd->get_palette_color = sm501_crt_get_palette_color ;
   lcd->disable = sm501_crt_disable ;
   lcd_init_fb(lcd);
}
#endif

#endif /* CONFIG_SM501 */
