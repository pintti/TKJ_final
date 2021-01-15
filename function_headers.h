#ifndef __FUNC_HEAD_H
#define __FUNC_HEAD_H

#include <stdbool.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include "Board.h"
#include "buzzer.h"
#include "wireless/address.h"
#include "wireless/comm_lib.h"
#include "sensors/mpu9250.h"
#include <stdio.h>
#include <inttypes.h>

void moveX(float x);
void moveY(float y);
void sound_effectTask();
void clk_task(UArg arg0);
void sendMessage();
void nuoli_ylos();
void nuoli_alas();
void nuoli_vasen();
void nuoli_oikea();
void tapahtumaTask();
void sound_effect();
void sound_effect_event();
void ledTaskFnkt(UArg arg0, UArg arg1);
void buttonFxn(PIN_Handle handle, PIN_Id pinId);
void menuTask(UArg arg0, UArg arg1);

#endif
