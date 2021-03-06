/*
*********************************************************************************************************
*                                                    eMOD
*                                   the Easy Portable/Player Operation System
*                                            power manager sub-system
*
*                                     (c) Copyright 2008-2009, kevin.z China
*                                              All Rights Reserved
*
* File   : standby_tmr.c
* Version: V1.0
* By     : kevin.z
* Date   : 2009-7-22 18:31
*********************************************************************************************************
*/
#include "standby_i.h"


static __mem_tmr_reg_t  *TmrReg;
static __u32 TmrIntCtl, Tmr0Ctl, Tmr0IntVal, Tmr0CntVal, Tmr1Ctl, Tmr1IntVal, Tmr1CntVal;

/*
*********************************************************************************************************
*                                     TIMER INIT
*
* Description: initialise timer for standby.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 standby_tmr_init(void)
{
    __s32       i;

    /* set timer register base */
    TmrReg = (__mem_tmr_reg_t *)SW_VA_TIMERC_IO_BASE;

    /* backup timer registers */
    TmrIntCtl   = TmrReg->IntCtl;
    Tmr0Ctl     = TmrReg->Tmr0Ctl;
    Tmr0IntVal  = TmrReg->Tmr0IntVal;
    Tmr0CntVal  = TmrReg->Tmr0CntVal;
    Tmr1Ctl     = TmrReg->Tmr1Ctl;
    Tmr1IntVal  = TmrReg->Tmr1IntVal;
    Tmr1CntVal  = TmrReg->Tmr1CntVal;

    /* config timer interrrupt */
    TmrReg->IntCtl   = 0;
    TmrReg->IntSta   = 1;

    /* config timer1 for process udelay */
    TmrReg->Tmr1Ctl  = 0;
    standby_delay(1000);
    TmrReg->Tmr1Ctl  = (1<<7)|(5<<4);
    standby_delay(1000);

    return 0;
}


/*
*********************************************************************************************************
*                                     TIMER EXIT
*
* Description: exit timer for standby.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 standby_tmr_exit(void)
{
    __s32   i;

    /* restore timer0 parameters */
    TmrReg->Tmr0IntVal  = Tmr0IntVal;
    TmrReg->Tmr0CntVal  = Tmr0CntVal;
    TmrReg->Tmr0Ctl     = Tmr0Ctl;
    TmrReg->Tmr1IntVal  = Tmr1IntVal;
    TmrReg->Tmr1CntVal  = Tmr1CntVal;
    TmrReg->Tmr1Ctl     = Tmr1Ctl;
    TmrReg->IntCtl      = TmrIntCtl;

    return 0;
}


/*
*********************************************************************************************************
*                           standby_tmr_set
*
*Description: set timer for wakeup system.
*
*Arguments  : second    time value for wakeup system.
*
*Return     : result, 0 - successed, -1 - failed;
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 standby_tmr_set(__u32 second)
{
    /* config timer interrrupt */
    TmrReg->IntSta     = 1;
    TmrReg->IntCtl     = 1;

    /* config timer0 for standby */
    TmrReg->Tmr0Ctl    = 0;
    TmrReg->Tmr0IntVal = second << 10;
    TmrReg->Tmr0Ctl    = (1<<7) | (5<<4);
    TmrReg->Tmr0Ctl   |= (1<<1);
    TmrReg->Tmr0Ctl   |= (1<<0);

    return 0;
}


/*
*********************************************************************************************************
*                           standby_tmr_enable_watchdog
*
*Description: enable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void standby_tmr_enable_watchdog(void)
{
    /* set watch-dog reset, timeout is 2 seconds */
    TmrReg->DogMode = (2<<3) | (1<<1);
    /* enable watch-dog */
    TmrReg->DogMode |= (1<<0);
}


/*
*********************************************************************************************************
*                           standby_tmr_disable_watchdog
*
*Description: disable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void standby_tmr_disable_watchdog(void)
{
    /* disable watch-dog reset */
    TmrReg->DogMode &= ~(1<<1);
    /* disable watch-dog */
    TmrReg->DogMode &= ~(1<<0);
}


/*
**********************************************************************************************************************
*                                               standby_tmr_query
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
__s32 standby_tmr_query(enum tmr_event_type_e type)
{
    __s32   result;

    switch(type)
    {
        case TMR_EVENT_POWEROFF:
        {
            if(TmrReg->IntSta & 1)
            {
                TmrReg->IntSta = 1;
                return 0;
            }
            return -1;
        }

        case TMR_EVENT_ALARM:
        default:
            return -1;
    }

    return -1;
}


/*
*********************************************************************************************************
*                           standby_tmr_mdlay
*
*Description: delay ms
*
*Arguments  : ms    time for delay;
*
*Return     : none
*
*Notes      :
*
*********************************************************************************************************
*/
void standby_tmr_mdlay(int ms)
{
    int     i;

    if(ms < 30){
        ms = 30;
    }

    TmrReg->Tmr1IntVal = ms;
    TmrReg->Tmr1Ctl   |= (1<<1);
    TmrReg->Tmr1Ctl   |= (1<<0);
    standby_delay(3000);
    while(TmrReg->Tmr1CntVal);

    return;
}