#include <common.h>

#ifdef CONFIG_SM501
#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <lcd.h>
#include <lcd_panels.h>

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

unsigned long const fbStart = 0x0C000000 ;
unsigned long const fbMax   = 0x00800000 ;    //

unsigned long const mmioStart  = 0xFE00000 ;
unsigned long const mmioLength = 0x00200000 ;
unsigned long const lcdPaletteRegs = 0xFE80400 ;
unsigned long const crtPaletteRegs = 0xFE80C00 ;
unsigned long paletteRegs = 0xFE80400 ;

const unsigned int sm501_list1[]={
   0x0FE00000,
	0x00100000,0x00001002,0x00000000,0x00000000,0x07F127C0,0x05146732,0x40715128,0x00000000,
	0x00000000,0x00180002,0x00000002,0x00000002,0x00000000,0x00000000,0x0000001F,0x291A0201,
	0x0000001F,0x291A0201,0x00000007,0x291A0201,0x00018000,0x00000000,0x00000000,0x00000000,
	0x050100A0,0x00000000,0x00080800};

const unsigned int sm501_list2[]={ 0x0fe80000,
/* 80000 dispctrl  */ 	0x0F013100,             // 0f0d0105
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

#define CLOCK_ACTIVEHIGH 0
#define CLOCK_ACTIVELOW  (1<<14)
#define CLOCK_ACTIVEMASK (1<<14)

#define LCDTYPE_TFT    0
#define LCDTYPE_STN12  (3<<18)
#define LCDTYPE_MASK   (3<<18)

#define READREG( addr ) *( (unsigned long volatile *)((addr)+mmioStart) )
#define STUFFREG( addr, value ) *( (unsigned long volatile *)((addr)+mmioStart) ) = (value)

const unsigned int sm501_list3[]={0x0fe80040,
	0x00010000,0x0703E360,0x00200400,0x00A81330,0x0385009C,0x02200240,0x00000000,0x00000000,
	0x00EDEDED,0x089C4040,0x0031E3B0};
 
const unsigned int sm501_list4[]={0x0fe80080,
	0x00010000,0x05121880,0x28800C00,0x00108030,0x02090040,0x00840050,0x00000000,0x00000000,
	0x0141A200,0x020A0802,0x0088D109,0x20820040,0x10800000,0x30029200,0x00080821,0x01010400,
	0x44000120,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000};

const unsigned int sm501_list5[]={0x0fe800f0,
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

int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;		/* Start of console buffer	*/

short console_col;
short console_row;


ulong calc_fbsize (void)
{
   if( cur_lcd_panel )
   {
   	int line_length = (cur_lcd_panel->xres * NBITS (LCD_BPP)) / 8;
      return ( cur_lcd_panel->yres * line_length ) + PAGE_SIZE ;
   }
   else
      return 0 ;
}

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
	printf( "sm501 init start\n");
	
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
	printf( "lcd_ctrl_init exit\n");
}

void lcd_enable	(void)
{
}

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

static unsigned long const clockMasks[] = {
   0x3F<<24,
   0x1F<<16
};

#define XGA_PIXELS (1024*768)

static void updateCRT( unsigned long const           *freq,
                       struct lcd_panel_info_t const *panel )
{
   unsigned long reg ;
   unsigned long crtCtrl = 0x00010304 ; // FIFO 3 or more, CRT Timing, CRT data, enable 8-bit
   if( panel->pclk_redg )
      crtCtrl |= (3<<14);     // horizontal and vertical phase
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

static void useFastRAM(void)
{
   STUFFREG( pm0ClockReg, READREG(pm0ClockReg)|0x10 ); // use 336 MHz input
}

void set_lcd_panel( struct lcd_panel_info_t const *panel )
{
   dcache_disable();
   unsigned long dispctrl = READREG( dispctrlReg );
   dispctrl &= ~(CLOCK_ACTIVEMASK|LCDTYPE_MASK);
   if( !panel->pclk_redg )
      dispctrl |= CLOCK_ACTIVELOW ;

   if( !panel->active )
      dispctrl |= LCDTYPE_STN12 ;

   if (panel->crt==0) dispctrl |= 4;

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
      
      if( (0 != panel->crt) )
      {
         paletteRegs = crtPaletteRegs ;
         updateCRT( freq, panel );
      }
      else
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
#endif /* CONFIG_SM501 */
