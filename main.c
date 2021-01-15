//Tekijät
//Aleksi Tuovinen 2623318
//Samuli Nikkilä 2559180


#include <function_headers.h>

#define STACKSIZE 2048
#define axy_up 0.5
#define axy_lo -0.5
#define az_up -0.95
#define az_lo -1.05
#define gz_up 5
#define gz_lo -5
#define axy_stable_up 0.05
#define axy_stable_lo -0.05

Char MpuStack[1024];
Char sfxStack[1024];
Char GameStack[1024];
Char commTaskStack[2048];
Char displayStack[2048];
Char tapahtumaStack[1024];
Char menuTaskStack[1024];
Char ledTaskStack[512];

static PIN_Handle buzzerPin;
static PIN_State buzzerPinState;
static PIN_Config buzzerConfig[] = {
		Board_BUZZER | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL, PIN_TERMINATE
};

static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
		Board_MPU_POWER | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
		PIN_TERMINATE
};

static PIN_Handle buttonHandle1;
static PIN_State buttonState1;

static PIN_Handle ledHandle1;
static PIN_State ledState1;

static PIN_Handle ledHandle2;
static PIN_State ledState2;

static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

PIN_Config buttonConfig1[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE, PIN_TERMINATE
};
PIN_Config ledConfig1[] = {
   Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, PIN_TERMINATE
};
PIN_Config buttonConfig2[] = {
   Board_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE, PIN_TERMINATE
};
PIN_Config ledConfig2[] = {
   Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, PIN_TERMINATE
};

// Values
enum state {MENU=1, GAME, MSG, READ, WIN, LOSE};
enum state gameState = MENU;
enum move {UP=1, DOWN, LEFT, RIGHT};
enum move gameMove = UP;
int movesSpent = 0;
float ax=0, ay=0, az=-1.0, gx=0, gy=0, gz=0;
int firstMenu = 1;
int steps = 0;

//sensor read function
Void readTask(UArg arg0, UArg arg1){
	I2C_Handle i2cMPU;
	I2C_Params i2cMPUParams;
	I2C_Params_init(&i2cMPUParams);
	i2cMPUParams.bitRate = I2C_400kHz;
	i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;

	i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
	if (i2cMPU == NULL){
		System_abort("Error initializing MPU\n");
	}

	PIN_setOutputValue(hMpuPin, Board_MPU_POWER, Board_MPU_POWER_ON);

	mpu9250_setup(&i2cMPU);

	I2C_close(i2cMPU);

	i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
	while(1){
		if (gameState == READ){
			System_printf("READ\n");
			System_flush();
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);

			if(ax > axy_stable_up | ax < axy_stable_lo | ay > axy_stable_up | ay < axy_stable_lo){
				gameState = GAME;
			}
			/*else{
				gameState = MENU;
			}*/
		}
		Task_sleep(500000 / Clock_tickPeriod);
	}
}

//gameloop
Void game_Task(UArg arg0, UArg arg1){
	while(1){
		if (gameState == GAME){
			//Movement commands
			gameState = READ;
			if (ax > axy_up | ax < axy_lo){
				sound_effect();
				moveX(ax);
			}
			else if (ay > axy_up | ay < axy_lo){
				sound_effect();
				moveY(ay);
			}
		}
		Task_sleep(500000/Clock_tickPeriod);
	}
}

//communicates with parent
void sendMessage(){
	char payload[16];
	System_printf("MSG\n");
	System_flush();
	if(gameMove == UP){
		sprintf(payload, "event:UP");
	}
	else if(gameMove == DOWN){
		sprintf(payload, "event:DOWN");
	}
	else if(gameMove == LEFT){
		sprintf(payload, "event:LEFT");
	}
	else{
		sprintf(payload, "event:RIGHT");
	}
	gameState = READ;
	Send6LoWPAN(IEEE80154_SERVER_ADDR, payload, strlen(payload));
	StartReceive6LoWPAN();
}


//Movement functions
void moveX(float x){
	steps++;
	System_printf("UP DOWN\n");
	System_flush();
	if (x < axy_lo){
		gameMove = DOWN;
	}
	else{
		gameMove = UP;
	}
	gameState = MSG;
	movesSpent++;
	sendMessage();
}

void moveY(float y){
	steps++;
	System_printf("LEFT RIGHT\n");
	System_flush();
	if (y < axy_lo){
		gameMove = LEFT;
	}
	else{
		gameMove = RIGHT;
	}
	gameState = MSG;
	movesSpent++;
	sendMessage();
}


//Buzzer functions
Void sound_effectTask(UArg arg0, UArg arg1){
	buzzerPin = PIN_open(&buzzerPinState, buzzerConfig);
	if (buzzerPin == NULL){
    	System_abort("Buzzer pin open failed!");
	}
	while(1){
		while(gameState == MENU && firstMenu == 1){
		int i;
		for(i=0; i<3; i++){
			buzzerOpen(buzzerPin);
			buzzerSetFrequency(130);
			Task_sleep(150000/Clock_tickPeriod);
			buzzerSetFrequency(330);
			buzzerClose();
			Task_sleep(100000/Clock_tickPeriod);
			buzzerOpen(buzzerPin);
			Task_sleep(75000/Clock_tickPeriod);
			buzzerClose();
			Task_sleep(100000/Clock_tickPeriod);

			buzzerOpen(buzzerPin);
			buzzerSetFrequency(780);
			Task_sleep(100000/Clock_tickPeriod);
			buzzerSetFrequency(880);
			Task_sleep(100000/Clock_tickPeriod);
			buzzerSetFrequency(780);
			Task_sleep(100000/Clock_tickPeriod);

			buzzerClose();
			}
		}
		while (gameState == WIN){
			buzzerOpen(buzzerPin);
			buzzerSetFrequency(523);
			Task_sleep(75000/Clock_tickPeriod);
			buzzerSetFrequency(587);
			Task_sleep(75000/Clock_tickPeriod);
			buzzerSetFrequency(523);
			Task_sleep(75000/Clock_tickPeriod);
			buzzerSetFrequency(587);
			Task_sleep(75000/Clock_tickPeriod);
			buzzerSetFrequency(659);
			Task_sleep(150000/Clock_tickPeriod);
			buzzerClose();
			Task_sleep(300000/Clock_tickPeriod);
			}
		while (gameState == LOSE){
			buzzerOpen(buzzerPin);
			buzzerSetFrequency(659);
			Task_sleep(250000/Clock_tickPeriod);
			buzzerClose();
			Task_sleep(100000/Clock_tickPeriod);
			buzzerOpen(buzzerPin);
			buzzerSetFrequency(587);
			Task_sleep(150000/Clock_tickPeriod);
			buzzerClose();
			Task_sleep(100000/Clock_tickPeriod);
			buzzerOpen(buzzerPin);
			buzzerSetFrequency(523);
			Task_sleep(250000/Clock_tickPeriod);
			buzzerClose();
			Task_sleep(500000/Clock_tickPeriod);
			}
		Task_sleep(500000/Clock_tickPeriod);
	}
}


//Sound effect for moving
void sound_effect(){
	buzzerOpen(buzzerPin);
	buzzerSetFrequency(500);
	Task_sleep(100000/Clock_tickPeriod);
	buzzerClose();
}


Void commTaskFxn(UArg arg0, UArg arg1) {
	char payload[16];
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}
	System_printf("comms on");
	System_flush();
	uint16_t senderAddr;

    while (1) {
    	if (GetRXFlag() == true) {
    		memset(payload, 0, 16);
    		Receive6LoWPAN(&senderAddr, payload, strlen(payload));
    		if (strlen(payload) == 7){
    			gameState = WIN;
    		}
    		else if (strlen(payload) < 10 ){
    			gameState = LOSE;
    		}
    		else{
    			gameState = READ;
    		}
        }
    }
}

//LCD function
Void tapahtumaTask(UArg arg0, UArg arg1){
	Display_Params disParams;
	Display_Params_init(&disParams);
	disParams.lineClearMode = DISPLAY_CLEAR_BOTH;

	Display_Handle disHandle = Display_open(Display_Type_LCD, &disParams);
	tContext *pContext = DisplayExt_getGrlibContext(disHandle);

	int i = 3;
	int k = 3;
	int i_true = 1;
	int k_true = 1;
	char str[12];

	while(1){
		if (gameState == WIN){
			Display_print0(disHandle, 5, 4, "YOU WIN");
			GrCircleDraw(pContext, 46, 44, 30);
			GrFlush(pContext);
			Task_sleep(500000/Clock_tickPeriod);
			Display_clear(disHandle);
			Display_print0(disHandle, 3, 3, "PRESS THE");
			Display_print0(disHandle, 4, 4, "BUTTON");
			Display_print0(disHandle, 5, 2, "TO CONTINUE");
			Task_sleep(500000/Clock_tickPeriod);
			Display_clear(disHandle);
		}
		else if (gameState == LOSE){
			Display_print0(disHandle, 6, 4, "YOU LOSE");
			GrLineDraw(pContext, 24, 24, 72, 72);
			GrLineDraw(pContext, 24, 72, 72, 24);
			GrFlush(pContext);
			Task_sleep(500000/Clock_tickPeriod);
			Display_clear(disHandle);
			Display_print0(disHandle, 3, 3, "PRESS THE");
			Display_print0(disHandle, 4, 4, "BUTTON");
			Display_print0(disHandle, 5, 2, "TO CONTINUE");
			Task_sleep(500000/Clock_tickPeriod);
			Display_clear(disHandle);
		}
		else if (gameState == MENU){
			Display_print0(disHandle, 3, 3, "TRON GAME");
			Display_print0(disHandle, 5, 2, "THE BUTTON");
			if(firstMenu){
				Display_print0(disHandle, 4, 1, "TO START PRESS");
			}
			else{
				Display_print0(disHandle, 4, 1, "TO CONT PRESS");
				sprintf(str, "LIIKKEET: %02d", steps);
				Display_print0(disHandle, 11, 3, str);
			}
			Task_sleep(500000/Clock_tickPeriod);
			Display_clear(disHandle);
		}
		else{
			GrCircleFill(pContext, i, k, 6);
			GrFlush(pContext);
			if (i_true){
				i = i + 5;
			}
			else{
				i = i - 5;
			}
			if (k_true){
				k = k + 7;
			}
			else{
				k = k - 7;
			}
			if(i >= 86){
				i_true = 0;
			}
			else if(i <= 5){
				i_true = 1;
			}
			if(k >= 86){
				k_true = 0;
			}
			else if(k <= 8){
				k_true = 1;
			}
			Task_sleep(250000/Clock_tickPeriod);
			Display_clear(disHandle);
		}
	}
}

//led function
Void ledTaskFnkt(UArg arg0, UArg arg1){
	while(1){
		if (gameState == WIN || gameState == LOSE){
			PIN_setOutputValue(ledHandle1, Board_LED1, !PIN_getOutputValue(Board_LED1));
			Task_sleep(250000/Clock_tickPeriod);
			PIN_setOutputValue(ledHandle1, Board_LED1, !PIN_getOutputValue(Board_LED1));
			PIN_setOutputValue(ledHandle2, Board_LED0, !PIN_getOutputValue(Board_LED0));
			Task_sleep(250000/Clock_tickPeriod);
			PIN_setOutputValue(ledHandle2, Board_LED0, !PIN_getOutputValue(Board_LED0));
		}
		else{
			Task_sleep(500000/Clock_tickPeriod);
		}
	}
}

//button function
void buttonFxn(PIN_Handle handle, PIN_Id pinId){
	System_printf("PRESS\n");
	System_flush();
	if(gameState == WIN || gameState == LOSE){
		gameState = MENU;
	}
	else if (gameState == MENU){
		gameState = READ;
		firstMenu = 0;
	}
	else if (gameState == READ || gameState == GAME){
		gameState = MENU;
	}
}


Int main(void) {
	Board_initGeneral();
    Board_initI2C();
    Board_initUART();

	Task_Handle mpuTask;
	Task_Params mpuTaskParams;

	Task_Handle gameTask;
	Task_Params gameTaskParams;

    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);

    ledHandle1 = PIN_open(&ledState1, ledConfig1);
    ledHandle2 = PIN_open(&ledState2, ledConfig2);

    buttonHandle1 = PIN_open(&ledState1, buttonConfig1);
    if(!buttonHandle1){
    	System_abort("Error initializing button");
    }
    if (PIN_registerIntCb(buttonHandle1, &buttonFxn) != 0){
    	System_abort("Error registering function");
    }

	Task_Handle sfxTask;
	Task_Params sfxTaskParams;
	Task_Params_init(&sfxTaskParams);
    sfxTaskParams.stackSize = STACKSIZE/2;
    sfxTaskParams.stack = &sfxStack;
	sfxTaskParams.priority=2;
    sfxTask = Task_create(sound_effectTask, &sfxTaskParams, NULL);

    Task_Params_init(&mpuTaskParams);
    mpuTaskParams.stackSize = STACKSIZE/2;
    mpuTaskParams.stack = &MpuStack;
    mpuTaskParams.priority=2;
    mpuTask = Task_create(readTask, &mpuTaskParams, NULL);

    Task_Params_init(&gameTaskParams);
    gameTaskParams.stackSize = STACKSIZE/2;
    gameTaskParams.stack = &GameStack;
    gameTaskParams.priority = 2;
    gameTask = Task_create(game_Task, &gameTaskParams, NULL);

	Task_Handle commTask;
	Task_Params commTaskParams;
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;
    commTask = Task_create(commTaskFxn, &commTaskParams, NULL);

    Task_Handle ledTask;
    Task_Params ledTaskParams;
    Task_Params_init(&ledTaskParams);
    ledTaskParams.stackSize = STACKSIZE/4;
    ledTaskParams.stack = &ledTaskStack;
    ledTaskParams.priority=2;
    ledTask = Task_create(ledTaskFnkt, &ledTaskParams, NULL);

    Task_Params eventTaskParams;
    Task_Handle eventTaskHandle;
    Task_Params_init(&eventTaskParams);
    eventTaskParams.stackSize = STACKSIZE/2;
    eventTaskParams.stack = &tapahtumaStack;
    eventTaskParams.priority = 2;
    eventTaskHandle = Task_create(tapahtumaTask, &eventTaskParams, NULL);

    Task_Handle menuTaskHandle;
    Task_Params menuTaskParams;
    Task_Params_init(&menuTaskParams);
    menuTaskParams.stackSize = STACKSIZE/2;
    menuTaskParams.stack = &menuTaskStack;
    menuTaskParams.priority = 2;
    menuTaskHandle = Task_create(menuTask, &menuTaskParams, NULL);

    Init6LoWPAN();

    System_printf("Hello world!\n");
    System_flush();
    
    BIOS_start();

    return (0);
}
