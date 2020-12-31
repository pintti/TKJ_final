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
static PIN_State buttonState2;

static PIN_Handle ledHandle1;
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
enum state {MENU=1, GAME, MSG, READ};
enum state gameState = MENU;
enum move {UP=1, DOWN, LEFT, RIGHT};
enum move gameMove = UP;
int movesSpent = 0;
float ax=0, ay=0, az=-1.0, gx=0, gy=0, gz=0;


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
			else{
				gameState = MENU;
			}
		}
		Task_sleep(30000 / Clock_tickPeriod);
	}
}

Void game_Task(UArg arg0, UArg arg1){
	while(1){
		if(gameState == MENU){
			gameState = READ;
		}
		if (gameState == GAME){

			//Movement commands
			gameState = READ;
			if (ax > axy_up | ax < axy_lo){
				moveX(ax);
				sound_effect();
			}
			else if (ay > axy_up | ay < axy_lo){
				moveY(ay);
				sound_effect();
			}
		}
		Task_sleep(100000/Clock_tickPeriod);
	}
}


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
	while(gameState == MENU){
	int i;
	for(i=0; i<5; i++){
		buzzerOpen(buzzerPin);
		buzzerSetFrequency(130);
		Task_sleep(100000/Clock_tickPeriod);
		buzzerSetFrequency(330);
		buzzerClose();
		Task_sleep(50000/Clock_tickPeriod);
		buzzerOpen(buzzerPin);
		Task_sleep(75000/Clock_tickPeriod);
		buzzerClose();
		Task_sleep(50000/Clock_tickPeriod);

		buzzerOpen(buzzerPin);
		buzzerSetFrequency(780);
		Task_sleep(50000/Clock_tickPeriod);
		buzzerSetFrequency(880);
		Task_sleep(50000/Clock_tickPeriod);
		buzzerSetFrequency(780);
		Task_sleep(50000/Clock_tickPeriod);

		buzzerClose();
	}
}
}


//Sound effect for moving
void sound_effect(){
	buzzerOpen(buzzerPin);
	buzzerSetFrequency(500);
	Task_sleep(100000/Clock_tickPeriod);
	buzzerClose();
}




/* Communication Task */

Void commTaskFxn(UArg arg0, UArg arg1) {
	char payload[16];
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}
	System_printf("comms on");
	System_flush();

    while (1) {
    	if (GetRXFlag() == true) {
    		memset(payload, 0, 16);
    		Receive6LoWPAN(IEEE80154_MY_ADDR, payload, strlen(payload));
    		System_printf(payload);
    		System_flush();
        }
    }
}


Int main(void) {

	// Initialize board
	Board_initGeneral();
    Board_initI2C();
    Board_initUART();

    // Task variables
	Task_Handle mpuTask;
	Task_Params mpuTaskParams;

	Task_Handle gameTask;
	Task_Params gameTaskParams;

    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL){
    	System_abort("Pin open failed!");
    }

	Task_Handle sfxTask;
	Task_Params sfxTaskParams;

	Task_Params_init(&sfxTaskParams);
    sfxTaskParams.stackSize = STACKSIZE;
    sfxTaskParams.stack = &sfxStack;
	sfxTaskParams.priority=2;

    sfxTask = Task_create(sound_effectTask, &sfxTaskParams, NULL);
    if (sfxTask == NULL) {
      	System_abort("SFX task create failed!");
    }

    Task_Params_init(&mpuTaskParams);
    mpuTaskParams.stackSize = STACKSIZE;
    mpuTaskParams.stack = &MpuStack;
    mpuTaskParams.priority=2;

    mpuTask = Task_create(readTask, &mpuTaskParams, NULL);
    if (mpuTask == NULL) {
    	System_abort("MPU task create failed!");
    }

    Task_Params_init(&gameTaskParams);
    gameTaskParams.stackSize = STACKSIZE;
    gameTaskParams.stack = &GameStack;
    gameTaskParams.priority = 2;

    gameTask = Task_create(game_Task, &gameTaskParams, NULL);
    if (gameTask == NULL){
    	System_abort("Game task create failed!");
    }

	Task_Handle commTask;
	Task_Params commTaskParams;

    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;

    commTask = Task_create(commTaskFxn, &commTaskParams, NULL);
    if (commTask == NULL) {
    	System_abort("Task create failed!");
    }

    Init6LoWPAN();

    System_printf("Hello world!\n");
    System_flush();
    
    BIOS_start();

    return (0);
}
