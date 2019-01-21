/*

    ptp-mii-tool: monitor and control the MII for a network interface with PTP registers

    Usage:

	ptp-mii-tool [-Vvpedst] 

    This program is based on David A. Hinds's "mii-tool" program, customized 
    to read/write extended PHY registers from a DP83640 PHYTER with PTP
    functions.  "mii-tool" is based on Donald Becker's "mii-diag" program. 
    Also uses (ported) code snippets and #defines from DP83640 kernel 
    driver by OMICRON electronics GmbH. 

    Copyright (C) 2018 XIA LLC -- sales@xia.com
      
    mii-tool is written/copyright 2000 by David A. Hinds -- dhinds@pcmcia.sourceforge.org
    mii-diag is written/copyright 1997-2000 by Donald Becker  <becker@scyld.com>
    DP83640.c is written/copyright 2010 by OMICRON electronics GmbH

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


*/

static char Version[] = "$Id: ptp-mii-tool.c,v 1.00 2018-01-10  ecki Exp $\n(Author: W.Hennig based on mii-tool, mii-diag, and DP83640 kernel driver)";

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>

#ifndef __GLIBC__
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#include "mii.h"
//#include "version.h"
#include "dp83640_reg.h"

#define MAX_ETH		8		/* Maximum # of interfaces */
#define TWOTO32   4294967296

/* Table of known MII's */
static const struct {
    u_short	id1, id2;
    char	*name;
} mii_id[] = {
    { 0x2000, 0x5ce0, "TI National DP83640" },
};
#define NMII (sizeof(mii_id)/sizeof(mii_id[0]))

/*--------------------------------------------------------------------*/

struct option longopts[] = {
 /* { name  has_arg  *flag  val } */
 //   {"advertise",	1, 0, 'A'},	 /* Change capabilities advertised. */
 //   {"force",		1, 0, 'F'},	 /* Change capabilities advertised. */
 //   {"phy",		1, 0, 'p'},	    /* Set PHY (MII address) to report. */
 //   {"log",		0, 0, 'l'},	    /* Set PHY (MII address) to report. */
 //   {"restart",		0, 0, 'r'},	 /* Restart link negotiation */
 //   {"reset",		0, 0, 'R'},	 /* Reset the transceiver. */
      {"syncE", 	   0, 0, 's'},	 /* Toggle syncE.  */
      {"pps", 	      0, 0, 'p'},	 /* Toggle PPS.  */
      {"enable", 	   1, 0, 'e'},	 /* Set Enable at time enablestart.  */
      {"duration", 	1, 0, 'd'},	 /* Clear Enable after duration.  */
      {"time", 	   0, 0, 't'},	 /* Report time only.  */
      {"verbose", 	0, 0, 'v'},	 /* Report each action taken.  */
      {"version", 	0, 0, 'V'},	 /* Emit version information.  */  
 //   {"watch", 		0, 0, 'w'},	 /* Constantly monitor the port.  */
      {"help", 		0, 0, '?'},	 /* Give help */
      { 0, 0, 0, 0 }
};

static unsigned int
    verbose = 0,
    opt_version = 0,
    opt_pps = 0,
    duration = 0,
    opt_time = 0,
    opt_syncE = 0;
//    opt_restart = 0,
//    opt_reset = 0,
//    opt_log = 0,
//    opt_watch = 0;
//static int nway_advertise = 0;
//static int fixed_speed = 0;
//static int override_phy = -1;
static unsigned long long int    enabletime = 0;

static int skfd = -1;		/* AF_INET socket for ioctl() calls. */
static struct ifreq ifr;

/*--------------------------------------------------------------------*/

static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
	fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
		strerror(errno));
	return -1;
    }
    return mii->val_out;
}

static void mdio_write(int skfd, int location, int value)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    mii->val_in = value;
    if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
	fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
		strerror(errno));
    }
}

/*--------------------------------------------------------------------*/

const struct {
    char	*name;
    u_short	value[2];
} media[] = {
    /* The order through 100baseT4 matches bits in the BMSR */
    { "10baseT-HD",	{MII_AN_10BASET_HD} },
    { "10baseT-FD",	{MII_AN_10BASET_FD} },
    { "100baseTx-HD",	{MII_AN_100BASETX_HD} },
    { "100baseTx-FD",	{MII_AN_100BASETX_FD} },
    { "100baseT4",	{MII_AN_100BASET4} },
    { "100baseTx",	{MII_AN_100BASETX_FD | MII_AN_100BASETX_HD} },
    { "10baseT",	{MII_AN_10BASET_FD | MII_AN_10BASET_HD} },

    { "1000baseT-HD",	{0, MII_BMCR2_1000HALF} },
    { "1000baseT-FD",	{0, MII_BMCR2_1000FULL} },
    { "1000baseT",	{0, MII_BMCR2_1000HALF|MII_BMCR2_1000FULL} },
};
#define NMEDIA (sizeof(media)/sizeof(media[0]))
	
/* Parse an argument list of media types */
static int parse_media(char *arg, unsigned *bmcr2)
{
    int mask, i;
    char *s;
    mask = strtoul(arg, &s, 16);
    if ((*arg != '\0') && (*s == '\0')) {
	if ((mask & MII_AN_ABILITY_MASK) &&
	    !(mask & ~MII_AN_ABILITY_MASK)) {
		*bmcr2 = 0;
		return mask;
	}
	goto failed;
    }
    mask = 0;
    *bmcr2 = 0;
    s = strtok(arg, ", ");
    do {
	    for (i = 0; i < NMEDIA; i++)
		if (s && strcasecmp(media[i].name, s) == 0) break;
	    if (i == NMEDIA) goto failed;
	    mask |= media[i].value[0];
	    *bmcr2 |= media[i].value[1];
    } while ((s = strtok(NULL, ", ")) != NULL);

    return mask;
failed:
    fprintf(stderr, "Invalid media specification '%s'.\n", arg);
    return -1;
}

/*--------------------------------------------------------------------*/

static const char *media_list(unsigned mask, unsigned mask2, int best)
{
    static char buf[100];
    int i;
    *buf = '\0';

    if (mask & MII_BMCR_SPEED1000) {
	if (mask2 & MII_BMCR2_1000HALF) {
	    strcat(buf, " ");
	    strcat(buf, "1000baseT-HD");
	    if (best) goto out;
	}
	if (mask2 & MII_BMCR2_1000FULL) {
	    strcat(buf, " ");
	    strcat(buf, "1000baseT-FD");
	    if (best) goto out;
	}
    }
    mask >>= 5;
    for (i = 4; i >= 0; i--) {
	if (mask & (1<<i)) {
	    strcat(buf, " ");
	    strcat(buf, media[i].name);
	    if (best) break;
	}
    }
 out:
    if (mask & (1<<5))
	strcat(buf, " flow-control");
    return buf;
}

int show_basic_mii(int sock, int phy_id)
{
    char buf[100];
    int i, k, mii_val[320];
    unsigned bmcr, bmsr, advert, lkpar, bmcr2, lpa2;
    unsigned int val[4];

    // PTP DP83640 specific variables 
    // trigger input variables (user readable)
    int gpio_pps = 9;           // GPIO pin for pps (Pixie-Net external)
    int gpio_ena = 8;           // GPIO pin for enable (Pixie-Net internal)
    int trigger_pps = 0;        // trigger number
    int trigger_ena_on  = 2;        // trigger number
    int trigger_ena_off = 3;        // trigger number
    int starttime_s;            // trigger start time (pick a sec value in the future)
    int starttime_ns = 0;       // trigger start time (full s boundary for now)
    int pwidth  = 999999800;    // trigger pulse width and periodicity, here use only ns (period is Pulsewidth + Pulsewidth2).
    int pwidth2 = 200;

    // trigger control variables (for mii)
    unsigned int ptp_trig_value;     //contains control word for PTP_TRIG register
    unsigned int ptp_ctl_value;     //contains control word for PTP_CTL register    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);


    for (i = 0; i < 320; i++)       /* init local copy to 0xFFFF */
	     mii_val[i] = 0xFFFF;

    /* Some bits in the BMSR are latched, but we can't rely on being
       the only reader, so only the current values are meaningful */
    mdio_read(sock, MII_BMSR);
    for (i = 0; i < ((verbose > 1) ? 32 : MII_BASIC_MAX); i++)
	  mii_val[i] = mdio_read(sock, i);

   /* PTP: read more pages */
   if(verbose > 1) {
      for(k=1;k<=6;k++) {
          mdio_write(sock, 0x13, k);   /* write page number */
          for (i = 0x13; i <=  0x1F; i++)
	        mii_val[i+32*k] = mdio_read(sock, i);    /* read page registers into new 32 block */
      } /* for pages */

   } /* if */

/* PTP: read current time */

    mdio_write(sock, PAGEREG, PAGE4);        /* write page number */
    mdio_write(sock, PTP_CTL, PTP_RD_CLK);   /* write "time read" bit to PTP_CTL register */

    val[0] = mdio_read(sock, PTP_TDR); /* ns[15:0]  // read 4 times to get time info*/
    val[1] = mdio_read(sock, PTP_TDR); /* ns[31:16] */
    val[2] = mdio_read(sock, PTP_TDR); /* sec[15:0] */
    val[3] = mdio_read(sock, PTP_TDR); /* sec[31:16] */

   if(opt_time>0) {
      printf("PTP local time (s): %012llu, System Time %d-%d-%d %d:%d:%d\n", val[3]*TWOTO32+val[2], tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
   } else {
      printf("PTP local time: %llu s, %llu ns\n",val[3]*TWOTO32+val[2], val[1]*TWOTO32+val[0]);
   }

/* PTP: END read current time */





    if (mii_val[MII_BMCR] == 0xffff  || mii_val[MII_BMSR] == 0x0000) {
	fprintf(stderr, "  No MII transceiver present!.\n");
	return -1;
    }

    /* Descriptive rename. */
    bmcr = mii_val[MII_BMCR]; bmsr = mii_val[MII_BMSR];
    advert = mii_val[MII_ANAR]; lkpar = mii_val[MII_ANLPAR];
    bmcr2 = mii_val[MII_CTRL1000]; lpa2 = mii_val[MII_STAT1000];

    sprintf(buf, "%s: ", ifr.ifr_name);
    if (bmcr & MII_BMCR_AN_ENA) {
	if (bmsr & MII_BMSR_AN_COMPLETE) {
	    if (advert & lkpar) {
		strcat(buf, (lkpar & MII_AN_ACK) ?
		       "negotiated" : "no autonegotiation,");
		strcat(buf, media_list(advert & lkpar, bmcr2 & lpa2>>2, 1));
		strcat(buf, ", ");
	    } else {
		strcat(buf, "autonegotiation failed, ");
	    }
	} else if (bmcr & MII_BMCR_RESTART) {
	    strcat(buf, "autonegotiation restarted, ");
	}
    } else {
	sprintf(buf+strlen(buf), "%s Mbit, %s duplex, ",
		((bmcr2 & (MII_BMCR2_1000HALF | MII_BMCR2_1000FULL)) & lpa2 >> 2)
		? "1000"
		: (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
		(bmcr & MII_BMCR_DUPLEX) ? "full" : "half");
    }
    strcat(buf, (bmsr & MII_BMSR_LINK_VALID) ? "link ok" : "no link");

/*    if (opt_watch) {
   	if (opt_log) {
   	    syslog(LOG_INFO, buf);
   	} else {
   	    char s[20];
   	    time_t t = time(NULL);
   	    strftime(s, sizeof(s), "%T", localtime(&t));
   	    printf("%s %s\n", s, buf);
   	}
    } else {
   	printf("%s\n", buf);
    }
 */
  if (verbose)  printf("%s\n", buf);      // suppress MII info print unless verbose option 

    if (verbose > 1) {
   	printf("  registers for MII PHY %d: ", phy_id);
   	for (i = 0; i < 32; i++)
   	    printf("%s %4.4x", ((i % 8) ? "" : "\n   "), mii_val[i]);
   	printf("\n");

    /* PTP: print more pages */
      for(k=1;k<=6;k++) {
          printf("  registers for MII PHY %d: ", k);
          for (i = 16; i <  32; i++)
	           printf("%s %4.4x", ((i % 8) ? "" : "\n   "), mii_val[i+32*k]);
          	printf("\n");
      } /* for pages */
    }

    if (verbose) {
   	printf("  product info: ");
   	for (i = 0; i < NMII; i++)
   	    if ((mii_id[i].id1 == mii_val[2]) &&
   		(mii_id[i].id2 == (mii_val[3] & 0xfff0)))
   		break;
   	if (i < NMII)
   	    printf("%s rev %d\n", mii_id[i].name, mii_val[3]&0x0f);
   	else
   	    printf("vendor %02x:%02x:%02x, model %d rev %d\n",
   		   mii_val[2]>>10, (mii_val[2]>>2)&0xff,
   		   ((mii_val[2]<<6)|(mii_val[3]>>10))&0xff,
   		   (mii_val[3]>>4)&0x3f, mii_val[3]&0x0f);
   	printf("  basic mode:   ");
   	if (bmcr & MII_BMCR_RESET)
   	    printf("software reset, ");
   	if (bmcr & MII_BMCR_LOOPBACK)
   	    printf("loopback, ");
   	if (bmcr & MII_BMCR_ISOLATE)
   	    printf("isolate, ");
   	if (bmcr & MII_BMCR_COLTEST)
   	    printf("collision test, ");
   	if (bmcr & MII_BMCR_AN_ENA) {
   	    printf("autonegotiation enabled\n");
   	} else {
   	    printf("%s Mbit, %s duplex\n",
   		   (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
   		   (bmcr & MII_BMCR_DUPLEX) ? "full" : "half");
   	}
   	printf("  basic status: ");
   	if (bmsr & MII_BMSR_AN_COMPLETE)
   	    printf("autonegotiation complete, ");
   	else if (bmcr & MII_BMCR_RESTART)
   	    printf("autonegotiation restarted, ");
   	if (bmsr & MII_BMSR_REMOTE_FAULT)
   	    printf("remote fault, ");
   	printf((bmsr & MII_BMSR_LINK_VALID) ? "link ok" : "no link");
   	printf("\n  capabilities:%s", media_list(bmsr >> 6, bmcr2, 0));
   	printf("\n  advertising: %s", media_list(advert, lpa2 >> 2, 0));
   	if (lkpar & MII_AN_ABILITY_MASK)
   	    printf("\n  link partner:%s", media_list(lkpar, bmcr2, 0));
   	printf("\n");
    }


    /* PTP: set up sync-E */
    if(opt_syncE > 0) {
       mdio_write(sock, PAGEREG, PAGE0);        /* write page number */
   
       val[0] =   mdio_read(sock, PHYCR2);      /* read PHYCR2 register */
       printf("PHYCR2 before change: 0x%04x ",val[0]);
       if((val[0] & 0x2000) >0)
       {
          printf(" -- sync-E is ON\n");
       }
       else
       {
           printf(" -- sync-E is OFF\n");
       }
       
       val[0] =val[0] ^ 0x2000;                 /* toggle the register */
       mdio_write(sock, PHYCR2, val[0]);        /* write reg with sync-E ON */
       val[0] =   mdio_read(sock, PHYCR2);      /* read PHYCR2 register */
       printf("PHYCR2 after  change: 0x%04x ",val[0]);
       if((val[0] & 0x2000) >0)
       {
          printf(" -- sync-E is ON\n");
       }
       else
       {
           printf(" -- sync-E is OFF\n");
       }
    }

    /* PTP: set up PPS output */
   if(opt_pps > 0) {
       ptp_trig_value = TRIG_WR |                                     // write trigger configuration 
   		   (trigger_pps  & TRIG_CSEL_MASK) << TRIG_CSEL_SHIFT |      // choose trigger number
            (gpio_pps     & TRIG_GPIO_MASK) << TRIG_GPIO_SHIFT |      // choose GPIO pin
   		   TRIG_PER |                                                // periodic pulse
   		   TRIG_PULSE;                                               // create pulse (not edge)
   
       mdio_write(sock, PAGEREG, PAGE5);                              // write page number 
       mdio_write(sock, PTP_TRIG, ptp_trig_value);                    // write to PTP_TRIG register 
   
       ptp_ctl_value = (trigger_pps & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_LOAD;                                          // begin trigger load process
   
       starttime_s = val[3]*TWOTO32+val[2] +3;     // start 3s from now
   
   
       mdio_write(sock, PAGEREG, PAGE4);                    /* write page number */
       mdio_write(sock, PTP_CTL, ptp_ctl_value);            /* write to PTP_TRIG register */
       mdio_write(sock, PTP_TDR, starttime_ns & 0xffff);    /* write start time to PTP_TDR register */  /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, starttime_ns >> 16);       /* write start time to PTP_TDR register */  /* ns[31:16] */ 
       mdio_write(sock, PTP_TDR, starttime_s  & 0xffff);    /* write start time to PTP_TDR register */  /* sec[15:0] */ 
       mdio_write(sock, PTP_TDR, starttime_s  >> 16);       /* write start time to PTP_TDR register */  /* sec[31:16] */
       mdio_write(sock, PTP_TDR, pwidth       & 0xffff);    /* write pulsewidth to PTP_TDR register */   /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, pwidth       >> 16);       /* write pulsewidth to PTP_TDR register */   /* ns[31:16] */ 
    	if (trigger_pps < 2) {                                    /* Triggers 0 and 1 has programmable pulsewidth2 */
         mdio_write(sock, PTP_TDR, pwidth2    & 0xffff);    /* write pulsewidth2 to PTP_TDR register */   /* ns[15:0] */  
         mdio_write(sock, PTP_TDR, pwidth2    >> 16);       /* write pulsewidth2 to PTP_TDR register */   /* ns[31:16] */ 
   	 }
   
       ptp_ctl_value = (trigger_pps & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_EN;                                        // enable trigger
   
       mdio_write(sock, PTP_CTL, ptp_ctl_value);   /* write to PTP_TRIG register */

        printf("PPS turning on at PTP local time %d s\n", starttime_s);
    }
    // TDOD: toggle PPS off
    /* PTP: END set up PPS output */

    /* PTP: set up enable output */
   if(enabletime > 0 && duration > 0) {
  //    printf("enabletime %llu, duration %d\n",enabletime, duration);

      // first, check if requested time is later than current time
      mdio_write(sock, PAGEREG, PAGE4);        /* write page number */
      mdio_write(sock, PTP_CTL, PTP_RD_CLK);   /* write "time read" bit to PTP_CTL register */
      
      val[0] = mdio_read(sock, PTP_TDR); /* ns[15:0]  // read 4 times to get time info*/
      val[1] = mdio_read(sock, PTP_TDR); /* ns[31:16] */
      val[2] = mdio_read(sock, PTP_TDR); /* sec[15:0] */
      val[3] = mdio_read(sock, PTP_TDR); /* sec[31:16] */
      
      if(enabletime <  val[3]*TWOTO32+val[2])  {
         printf("PTP local time (%llu s) later than requested Enable start time, no change made\n",val[3]*TWOTO32+val[2]);
         return(-1);
      }
      printf("Current PTP local time %llu s\n",val[3]*TWOTO32+val[2]);

      // second, sepcify turn-on time
      ptp_trig_value = TRIG_WR |                                     // write trigger configuration 
   		   (trigger_ena_on  & TRIG_CSEL_MASK) << TRIG_CSEL_SHIFT |      // choose trigger number            
      //    TRIG_TOGGLE |                                                  // ignore init value
   	//	   TRIG_PER |                                                // periodic pulse
   		   TRIG_PULSE |                                               // create pulse (not edge)
            (gpio_ena       & TRIG_GPIO_MASK) << TRIG_GPIO_SHIFT;  //;     // choose GPIO pin
   
       mdio_write(sock, PAGEREG, PAGE5);                              // write page number 
       mdio_write(sock, PTP_TRIG, ptp_trig_value);                    // write to PTP_TRIG register 
   
       ptp_ctl_value = (trigger_ena_on & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_LOAD;                                          // begin trigger load process
   
       starttime_s = enabletime; // val[3]*TWOTO32+val[2] +2;                                      // start at specified time (s)
       starttime_ns = 0<<31;                                          // start at zero ns, set bit 31 to indicate initial state. 0 means rise to high
       pwidth = 600;

       mdio_write(sock, PAGEREG, PAGE4);                    /* write page number */
       mdio_write(sock, PTP_CTL, ptp_ctl_value);            /* write to PTP_TRIG register */
       mdio_write(sock, PTP_TDR, starttime_ns & 0xffff);    /* write start time to PTP_TDR register */  /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, starttime_ns >> 16);       /* write start time to PTP_TDR register */  /* init, roll, ns[29:16] */ 
       mdio_write(sock, PTP_TDR, starttime_s  & 0xffff);    /* write start time to PTP_TDR register */  /* sec[15:0] */ 
       mdio_write(sock, PTP_TDR, starttime_s  >> 16);       /* write start time to PTP_TDR register */  /* sec[31:16] */
       mdio_write(sock, PTP_TDR, pwidth       & 0xffff);    /* write pulsewidth to PTP_TDR register */  /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, pwidth       >> 16);       /* write pulsewidth to PTP_TDR register */  /* ns[31:16] */ 
    
       ptp_ctl_value = (trigger_ena_on & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_EN;                                        // enable trigger
   
       mdio_write(sock, PTP_CTL, ptp_ctl_value);   /* write to PTP_TRIG register */

       printf("Enable turning on  at PTP local time %d s\n", starttime_s);
 if(1) {
       // third, sepcify turn-off time
      ptp_trig_value = TRIG_WR |                                     // write trigger configuration 
   		   (trigger_ena_off  & TRIG_CSEL_MASK) << TRIG_CSEL_SHIFT |      // choose trigger number
      //      TRIG_TOGGLE |                                              // ignore init value
   	//	   TRIG_PER |                                                 // periodic pulse
   		   TRIG_PULSE |                                               // create pulse (not edge)
            (gpio_ena        & TRIG_GPIO_MASK) << TRIG_GPIO_SHIFT;      // choose GPIO pin
   
       mdio_write(sock, PAGEREG, PAGE5);                              // write page number 
       mdio_write(sock, PTP_TRIG, ptp_trig_value);                    // write to PTP_TRIG register 
   
       ptp_ctl_value = (trigger_ena_off & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_LOAD;                                          // begin trigger load process
   
       starttime_s = enabletime + duration ; //val[3]*TWOTO32+val[2] +2 +duration;                                      // start at specified time (s)
       starttime_ns = 0<<31;                                          // start at zero ns, set bit 31 to indicate initial state. 0 means rise to high
       pwidth = 600;

       mdio_write(sock, PAGEREG, PAGE4);                    /* write page number */
       mdio_write(sock, PTP_CTL, ptp_ctl_value);            /* write to PTP_TRIG register */
       mdio_write(sock, PTP_TDR, starttime_ns & 0xffff);    /* write start time to PTP_TDR register */  /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, starttime_ns >> 16);       /* write start time to PTP_TDR register */  /* init, roll, ns[29:16] */ 
       mdio_write(sock, PTP_TDR, starttime_s  & 0xffff);    /* write start time to PTP_TDR register */  /* sec[15:0] */ 
       mdio_write(sock, PTP_TDR, starttime_s  >> 16);       /* write start time to PTP_TDR register */  /* sec[31:16] */
       mdio_write(sock, PTP_TDR, pwidth       & 0xffff);    /* write pulsewidth to PTP_TDR register */  /* ns[15:0] */  
       mdio_write(sock, PTP_TDR, pwidth       >> 16);       /* write pulsewidth to PTP_TDR register */  /* ns[31:16] */ 
    
       ptp_ctl_value = (trigger_ena_off & TRIG_SEL_MASK) << TRIG_SEL_SHIFT |   // choose trigger number
                       TRIG_EN;                                        // enable trigger
   
       mdio_write(sock, PTP_CTL, ptp_ctl_value);   /* write to PTP_TRIG register */

       printf("Enable turning off at PTP local time %d s\n", starttime_s);
    }
   } else {
         if( (enabletime > 0 && duration <= 0)|| (enabletime <= 0 && duration > 0) )
         {
            printf("Invalid arguments for enable, check if both start time and duration are valid\n");
            return(-1);
         }
   }
   /* PTP: END set up enable output */



/* PTP: END set up sync-E */
    fflush(stdout);
    return 0;
}

/*--------------------------------------------------------------------*/

static int do_one_xcvr(int skfd, char *ifname, int maybe)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
   	if (!maybe || (errno != ENODEV))
   	    fprintf(stderr, "SIOCGMIIPHY on '%s' failed: %s\n",
   		    ifname, strerror(errno));
   	return 1;
    }

/*    if (override_phy >= 0) {
	printf("using the specified MII index %d.\n", override_phy);
	mii->phy_id = override_phy;
    }

    if (opt_reset) {
	printf("resetting the transceiver...\n");
	mdio_write(skfd, MII_BMCR, MII_BMCR_RESET);
    }
    if (nway_advertise > 0) {
	mdio_write(skfd, MII_ANAR, nway_advertise | 1);
	opt_restart = 1;
    }
    if (opt_restart) {
	printf("restarting autonegotiation...\n");
	mdio_write(skfd, MII_BMCR, 0x0000);
	mdio_write(skfd, MII_BMCR, MII_BMCR_AN_ENA|MII_BMCR_RESTART);
    }
    if (fixed_speed) {
	int bmcr = 0;
	if (fixed_speed & (MII_AN_100BASETX_FD|MII_AN_100BASETX_HD))
	    bmcr |= MII_BMCR_100MBIT;
	if (fixed_speed & (MII_AN_100BASETX_FD|MII_AN_10BASET_FD))
	    bmcr |= MII_BMCR_DUPLEX;
	mdio_write(skfd, MII_BMCR, bmcr);
    }

    if (!opt_restart && !opt_reset && !fixed_speed && !nway_advertise)   */
	show_basic_mii(skfd, mii->phy_id);

    return 0;
}

/*--------------------------------------------------------------------*/

/*
static void watch_one_xcvr(int skfd, char *ifname, int index)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    // static int status[MAX_ETH] = { 0, / *...* /  };      // restore comments!
    int now;

    // Get the vitals from the interface. 
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
	if (errno != ENODEV)
	    fprintf(stderr, "SIOCGMIIPHY on '%s' failed: %s\n",
		    ifname, strerror(errno));
	return;
    }
    now = (mdio_read(skfd, MII_BMCR) |
	   (mdio_read(skfd, MII_BMSR) << 16));
    if (status[index] && (status[index] != now))
	show_basic_mii(skfd, mii->phy_id);
    status[index] = now;
}

*/
/*--------------------------------------------------------------------*/

const char *usage =
//"usage: %s [-VvpsRrwl] [-A media,... | -F media] [interface ...]\n"
"usage: %s [-Vvps] \n"
"  -V, --version                  display version information\n"
"  -v, --verbose                  more verbose output (2x even more)\n"
"  -e, --enable=start_time_in_s   set Enable signal on GPIO8 high at PTP local time = start_time_in_s (int) \n"
"  -d, --duration=time_in_s       set Enable signal on GPIO8 low after time_in_s seconds (int) \n"
"  -p, --pps                      toggle pulse per second signal on GPIO9 3s from now\n"
"  -s, --syncE                    toggle SyncE option\n"
"  -t, --time                     report just the PTP local time in s\n";
//"       -R, --reset                 reset MII to poweron state\n"
//"       -r, --restart               restart autonegotiation\n"
//"       -w, --watch                 monitor for link status changes\n"
//"       -l, --log                   with -w, write events to syslog\n"
//"       -A, --advertise=media,...   advertise only specified media\n"
//"       -F, --force=media           force specified media technology\n"
//"media: 1000baseTx-HD, 1000baseTx-FD,\n"
//"       100baseT4, 100baseTx-FD, 100baseTx-HD,\n"
//"       10baseT-FD, 10baseT-HD,\n"
//"       (to advertise both HD and FD) 1000baseTx, 100baseTx, 10baseT\n";


static void version(void)
{
    fprintf(stderr, "%s\n", Version);
    //fprintf(stderr, "%s\n%s\n", Version, RELEASE);
    exit(5); /* E_VERSION */
}


int main(int argc, char **argv)
{
    int i, c, ret, errflag = 0;
    char s[6];
    unsigned ctrl1000 = 0;
    
    while ((c = getopt_long(argc, argv, "vVspdet?", longopts, 0)) != EOF)
	switch (c) {
//	case 'A': nway_advertise = parse_media(optarg, &ctrl1000); break;
//	case 'F': fixed_speed = parse_media(optarg, &ctrl1000); break;
//	case 'p': override_phy = atoi(optarg); break;
//	case 'r': opt_restart++;	break;
//	case 'R': opt_reset++;		break;
   case 's': opt_syncE++;		    break;
   case 'p': opt_pps++;		       break;
   case 'e': enabletime = atoi(optarg); break;
   case 'd': duration = atoi(optarg);   break;
	case 'v': verbose++;		       break;
	case 'V': opt_version++;	    break;
   case 't': opt_time++;          break;
//	case 'w': opt_watch++;		break;
//	case 'l': opt_log++;		break;
	case '?': errflag++;
	}
    /* Check for a few inappropriate option combinations */
//    if (opt_watch) verbose = 0;

//    if ((nway_advertise < 0) || (fixed_speed < 0))
//    	return 2;

    if (errflag) {
    //|| (fixed_speed & (fixed_speed-1)) || (fixed_speed && (opt_restart || nway_advertise))) {
	  fprintf(stderr, usage, argv[0]);
	  return 2;
    }

    if (opt_version)
	version();

    /* Open a basic socket. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
	perror("socket");
	exit(-1);
    }

    if (verbose > 1)
    	printf("Using XXX SIOCGMIIPHY=0x%x\n", SIOCGMIIPHY);	

    /* No remaining args means show all interfaces. */
    if (optind == argc) {
   	ret = 1;
   	for (i = 0; i < MAX_ETH; i++) {
   	    sprintf(s, "eth%d", i);
   	    ret &= do_one_xcvr(skfd, s, 1);
	   }
   	if (ret)
   	    fprintf(stderr, "no MII interfaces found\n");
    } else {
   	ret = 0;
   	for (i = optind; i < argc; i++) {
   	    ret |= do_one_xcvr(skfd, argv[i], 0);
   	}
    }

  /*  if (opt_watch && (ret == 0)) {
   	while (1) {
   	    sleep(1);
   	    if (optind == argc) {
   		for (i = 0; i < MAX_ETH; i++) {
   		    sprintf(s, "eth%d", i);
   		    watch_one_xcvr(skfd, s, i);
   		}
   	    } else {
   		for (i = optind; i < argc; i++)
   		    watch_one_xcvr(skfd, argv[i], i-optind);
   	    }
   	}
    }   */

    close(skfd);
    return ret;
}
