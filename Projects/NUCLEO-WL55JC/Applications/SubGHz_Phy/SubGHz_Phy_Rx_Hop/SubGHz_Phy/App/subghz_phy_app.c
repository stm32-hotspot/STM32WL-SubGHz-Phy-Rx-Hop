/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */

/* additional includes to accommodate the sequencer, IO and object time server */

#include "utilities_def.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "main.h"
#include "stdio.h"
#include "stm32wlxx_hal_subghz.h"
#include "radio_driver.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
/* STM */
#define RECEIVER //RECEIVER //  TRANSMITTER
#define BUFFER_SIZE 	48      /* this is the set payload length.  Edit this in the subghz_phy_app.h */
#define USECAD          0       /*  when enabled, CAD detect is used */
#define USEPREAMBLE     1        /* when enabled, the preamble detect is used */
#define true            1
extern UART_HandleTypeDef huart2;
extern SUBGHZ_HandleTypeDef hsubghz;
#define FREQOFFSET 450000       /* initial starting frequency offset*/
#define FREQSTEP   150000       /* frequency step each Rx hop interval */
#define NEXTHOPCHANNEL 0        /* starting hop channel number */
#define HOP_MAX_NB_CHANNEL 8    /* total number of channels to hop over */
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
static UTIL_TIMER_Object_t timerTransmit;   /* STM */
static UTIL_TIMER_Object_t timerReceive;    /* STM */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRANSMIT_PERIOD_MS 2500  /* set Tx period of timer to 2 seconds */     /* STM */
#define RECEIVE_PERIOD_MS  17    /* set Rx hop timer period in milli seconds */  /* STM */
/* Extra Definitions */
#define RX_CONTINUOUS_ON  1   /* switch for Rx when using FSK */  /* STM */
#define CMD_BUF_SIZE  12      /* placeholder for command buffer programming approach */ /* STM */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* USER CODE BEGIN PV */
uint16_t BufferSize = BUFFER_SIZE;   /* Buffer size for both Tx and Rx */
uint8_t Buffer[BUFFER_SIZE];         /* Buffer array definition */
uint8_t CmdBuf[CMD_BUF_SIZE];         /* Placeholder for SubGHz Direct Command */
uint32_t freq_step = FREQSTEP;        /* Rx frequency step */
uint32_t freq_off = FREQOFFSET;
uint32_t NextChannel = NEXTHOPCHANNEL;    /* Rx Next Channel Counter */
uint32_t NumChannel = HOP_MAX_NB_CHANNEL;       /* Define the number of hop channels */
uint32_t Receive_Period_ms = RECEIVE_PERIOD_MS;  /* Receive Period Define for Dynamic modification */
uint32_t Channel_Select[8] = {0,6,7,1,4,3,2,5};  /* Channel Order Array for Testing, 8 channels in order shown */


static uint8_t syncword[] = {0xC1, 0x94, 0xC1};  /* Sync Word for FSK Only */
int32_t count = 0;
uint8_t i = 0;
uint8_t CadFlag = 0;
uint8_t Bsize = 10;
uint32_t j=1;
/* Rx Stats */
int8_t rssi;
int8_t last_rssi;
uint8_t Rflag = 0;
uint8_t CRxStatus = 0;
uint8_t CRxRSSI = 0;
uint8_t CRxSNR = 0;
uint8_t RxSigRSSI = 0;
uint8_t PFlag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */

void OnPreambleDetect(void);  /* OnPreambleDetect*/  /* STM */
static void CadDone(bool channelActivityDetected); /* CAD Detect*/ /* STM */

static void Rx_Process(void);           /* Rx Process for sequencer */
static void CaD_Process(void);          /* CAD process for sequencer */
//static void Tx_Process(void);         /* placeholder for future use */
static void TransmitPacket(void *context);    /* Tx packet for Tx timeserver */
static void ReceivePacket(void *context);     /* Rx packet hop timer */  
/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
    /* USER CODE BEGIN SubghzApp_Init_1 */

#ifdef RECEIVER
 RxConfigGeneric_t RxConfig = {0};
  /* RX LEDs*/
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#endif
  
#ifdef TRANSMITTER
  TxConfigGeneric_t TxConfig;
  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_EXTI);
  /* TX LED*/
  BSP_LED_Init(LED_BLUE);
#endif
  
  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
  RadioEvents.OnPreambleDetect = OnPreambleDetect;   /* STM  */
  RadioEvents.CadDone = CadDone;                     /* STM */

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
  
#ifdef TRANSMITTER 
   /* LoRa Modem */
  /* for now, the Tx is set up for 915 MHz Tx duty cycle only */
 #if(USE_MODEM_LORA) 
  APP_LOG(TS_ON, VLEVEL_L, "******TRANSMITTER LORA******\n\r");    
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);  
  
  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);  
  Radio.SetChannel(RF_FREQUENCY);

#endif
 #if( USE_MODEM_FSK)
   APP_LOG(TS_ON, VLEVEL_L, "******TRANSMITTER FSK GENERIC******\n\r");
   /*fsk modulation*/
  TxConfig.fsk.ModulationShaping = RADIO_FSK_MOD_SHAPING_G_BT_05;
  TxConfig.fsk.Bandwidth = FSK_BANDWIDTH;
  TxConfig.fsk.FrequencyDeviation = FSK_FDEV;
  TxConfig.fsk.BitRate = FSK_DATARATE; /*BitRate*/
  TxConfig.fsk.PreambleLen = FSK_PREAMBLE_LENGTH;        /*in Bytes        */
  TxConfig.fsk.SyncWordLength = sizeof(syncword);       /*in Bytes        */
  TxConfig.fsk.SyncWord = syncword;                     /*SyncWord Buffer*/
  TxConfig.fsk.whiteSeed =  0x01FF ;                    /*WhiteningSeed  */
  TxConfig.fsk.HeaderType  = RADIO_FSK_PACKET_VARIABLE_LENGTH; /*legacy: payload length field is 1 byte long*/
  TxConfig.fsk.CrcLength = RADIO_FSK_CRC_OFF;       /* Size of the CRC block in the GFSK packet*/
  TxConfig.fsk.CrcPolynomial = 0x8005;
  TxConfig.fsk.CrcSeed = 0xFFFF;
  TxConfig.fsk.Whitening = RADIO_FSK_DC_FREE_OFF;
  if (0UL != Radio.RadioSetTxGenericConfig(GENERIC_FSK, &TxConfig, TX_OUTPUT_POWER, TX_TIMEOUT_VALUE))
  {
    while (1);
  }
#endif
  Radio.SetChannel(RF_FREQUENCY); 
#endif


#ifdef RECEIVER  
  
/* for the LoRa Rx, note that the parameter "LORA_RX_PREAMBLE_LENGTH" is
* set separately from the Tx side 
*/
#if(USE_MODEM_LORA)  
  APP_LOG(TS_ON, VLEVEL_L, "******RECEIVER LoRA******\n\r");    
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_RX_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, 0, 0, 0, LORA_IQ_INVERSION_ON, true);
 
   Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);  
   Radio.SetChannel(RF_FREQUENCY - 3*freq_step);  /* place at 915 - 3* 150k so that Rx hops through Tx*/
 
    
#endif
#if( USE_MODEM_FSK)  
 APP_LOG(TS_ON, VLEVEL_L, "******RECEIVER FSK GENERIC******\n\r"); 
  /* RX Continuous */
  RxConfig.fsk.ModulationShaping = RADIO_FSK_MOD_SHAPING_G_BT_05;
  RxConfig.fsk.Bandwidth = FSK_BANDWIDTH;
  RxConfig.fsk.BitRate = FSK_DATARATE;                          /*BitRate*/
  RxConfig.fsk.PreambleLen = FSK_PREAMBLE_LENGTH;               /*in Byte*/
  RxConfig.fsk.SyncWordLength = sizeof(syncword);                /*in Byte*/
  RxConfig.fsk.PreambleMinDetect = RADIO_FSK_PREAMBLE_DETECTOR_16_BITS;  /* FSK has its own preamble detector */
  RxConfig.fsk.SyncWord = syncword;                             /*SyncWord Buffer*/
  RxConfig.fsk.whiteSeed = 0x01FF ;                             /*WhiteningSeed*/
  RxConfig.fsk.LengthMode  = RADIO_FSK_PACKET_VARIABLE_LENGTH; /* legacy: payload length field is 1 byte long*/
  RxConfig.fsk.CrcLength = RADIO_FSK_CRC_OFF;                   /* Size of the CRC block in the GFSK packet*/
  RxConfig.fsk.CrcPolynomial = 0x8005;
  RxConfig.fsk.CrcSeed = 0xFFFF;
  RxConfig.fsk.Whitening = RADIO_FSK_DC_FREE_OFF;
  RxConfig.fsk.MaxPayloadLength = BUFFER_SIZE;
  RxConfig.fsk.StopTimerOnPreambleDetect = 0;
  RxConfig.fsk.AddrComp = RADIO_FSK_ADDRESSCOMP_FILT_OFF;
  if (0UL != Radio.RadioSetRxGenericConfig(GENERIC_FSK, &RxConfig, RX_CONTINUOUS_ON, 0))
  {
    while (1);
  }
#endif
   Radio.SetChannel(RF_FREQUENCY);
#endif
  
   
#ifdef TRANSMITTER    
/* enable a simple buffer for Tx testing */  
   for (int32_t i = 0; i < BufferSize; i++)
    {
      Buffer[i] = i;
    }
   
  /* Add delay between TX and RX
  *time Busy_signal is ON in RX opening window
 */
  HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);
 
 /* use the Time Server to set up a Tx transmit interval */ 
    UTIL_TIMER_Create(&timerTransmit, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TransmitPacket, NULL);
    UTIL_TIMER_SetPeriod(&timerTransmit, TRANSMIT_PERIOD_MS);
    UTIL_TIMER_Start(&timerTransmit);    /* start transmitting packets every TRANSMIT_PERIOD_MS */
 
#endif
  
#ifdef RECEIVER  

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Rx_Process), UTIL_SEQ_RFU, Rx_Process);  
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);  /* start the receive sequencer */
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_CaD_Process), UTIL_SEQ_RFU, CaD_Process);  /* initialize but do not start CAD process here */

  /* set up this time server object specifically for the Receive Hop Clock */
  UTIL_TIMER_Create(&timerReceive, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, ReceivePacket, NULL);
  UTIL_TIMER_SetPeriod(&timerReceive, Receive_Period_ms);
  UTIL_TIMER_Start(&timerReceive);  /* start receiving packets every Rx_Period_mS */
  
#endif 
  
  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */

/* STM */
/* Not here that the Rx_Process is called by the sequencer after the OnRxDone is complete
* both processes for CAD detect and Preamble Detect are managed here
* ensure to select the proper Preamble or CAD switch settings in the defines above 
*/

static void Rx_Process(void)
{
#if(USEPREAMBLE)
  APP_LOG(TS_OFF, VLEVEL_L, "\n\r");    
  APP_LOG(TS_ON, VLEVEL_L, "Radio Rx\n\r");  
 
  /* note it is not necessary to reconfigure the receiver unless parameters have changed */
  
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_RX_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, 0, 0, 0, LORA_IQ_INVERSION_ON, true);
 
  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);  
//  Radio channel was initialized above and controlled by the Rx Hop Timer */
 
 /* Enable Timer Stop on Preamble Detect 
* Note that in FW 1.2, there is a work around fix for this parameter
* Note also, that the work around is not used by this code
* the Rx Hop Timer is used to provide the equivalent of Preamble detect and Rx(TimeOut)
*/
  SUBGRF_SetStopRxTimerOnPreambleDetect( true); 
   
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);  /* use these pins to measure time events */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);  /* use these pins to measure time events */
  /* start the hop clock */
  UTIL_TIMER_Start(&timerReceive);  /* start searching for LoRa packets every Receive_Period_ms */
/* place the radio into Rx(0).  Important to use this and not Rx(TimeOut) for this process */
 
  Radio.Rx(0);
#endif  
 
#if(USECAD)
  
  APP_LOG(TS_OFF, VLEVEL_L, "\n\r");    
  APP_LOG(TS_ON, VLEVEL_L, "Radio Cad Rx\n\r"); 
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH, /* note here that the preamble lengths are requried to match in Rx and Tx */
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
 
    Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE); 
  /* set up Cad Params according to Ref Manual Page 187, Table 36 */
  SUBGRF_SetCadParams( LORA_CAD_04_SYMBOL, 0x28, 0x0A, LORA_CAD_RX, 0xFFFFFF);  /* set CaD Parameters for SF = 7-9 */
  /* Note that on the first Rx search, the CAD is set up separately as the Rx is configured during initialization.
* On subsequent CAD searches, it is necessary to reconfigure the Rx completely before launching a new CAD search
*/
  
  if(CadFlag == 0)
  {
    Radio.StartCad();   /* first time through */
  }
  else if(CadFlag == 1)
  {
   
     APP_LOG(TS_OFF, VLEVEL_L, "\n\r");    
     APP_LOG(TS_ON, VLEVEL_L, "Starting new CAD Process\n\r");     
        
     Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH, /* note here that the preamble lengths are requried to match in Rx and Tx */
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
 
    Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);  
    Radio.SetChannel(RF_FREQUENCY);
   //  Radio.SetChannel(RF_FREQUENCY - 3*freq_step); /* note that the CAD can be used in hop mode, provided the hop rate is properly set according to Nyquist */
   //  UTIL_TIMER_Start(&timerReceive);           /* enable hopping if required Receive_Period mS */
    SUBGRF_ClearIrqStatus( IRQ_CAD_CLEAR );       /* clear the IRQ prior to starting */
    SUBGRF_SetCadParams( LORA_CAD_04_SYMBOL, 0x20, 0x0A, LORA_CAD_RX, 0xFFFFFF);  /* set CaD Parameters for proper SF */
    Radio.StartCad();
  }
#endif
  
}

/* on every CAD search, a CAD IRQ is generated, regardless of whether a LoRa signal was detected or not 
* It is therefore necessary to check for the Presence of a LoRa signal by configuring the Rx(TimeOut) state
*/

static void CaD_Process(void)
{
      Radio.Rx(RX_TIMEOUT_VALUE);
}

//static void Tx_Process(void)
//{
//  APP_LOG(TS_ON, VLEVEL_L, "Radio Send\n\r");      
//  Radio.Send(Buffer, BufferSize);
//}

static void TransmitPacket(void *context)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
/* Transmti the packets according to requried settings in
* subghz_phy_app.h and defines above
*/
 
  for (int32_t i = 0; i < BufferSize; i++)
    {
      Buffer[i] = count;
      APP_LOG(TS_OFF,VLEVEL_L, "%x",Buffer[i]);
    }
  count++;
  
  APP_LOG(TS_ON, VLEVEL_L,"Tx_Freq%d\r\n",RF_FREQUENCY);
 
  Radio.Send(Buffer, BufferSize);
}

static void ReceivePacket(void *context)
{
   HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);  /* use this a time marker on a scope */
   
   /* uncomment this section if a linear sweep/hop is requried */
 /*  
*  NextChannel++;
*  if (NextChannel == HOP_MAX_NB_CHANNEL)
*  {
*    NextChannel = 0;
*  }
 */ 
  /* random hop according to sequence in Channel_Select */ 
   j++;  
  NextChannel = Channel_Select[j % 8];
  
 /* dither the Rx Hop clock to ensure slightly different periods each hop
* This will allow better detection, exit from erroneous frequencies and
* will not allow for Tx and Rx syncing, which can provide erroneous data
*/
  
    if(Rflag == 0)
    {
      Receive_Period_ms++;
      Rflag=1;
    }
   else
     {
         Receive_Period_ms--;
          Rflag = 0;
   }
   
    UTIL_TIMER_Start(&timerReceive);    /* start the time hop */
    Radio.SetChannel((RF_FREQUENCY-freq_off) + NextChannel*freq_step); /* set the next channel */
    rssi = SUBGRF_GetRssiInst();                                       /* get RSSI as this is important for future processing/decision making */
    APP_LOG(TS_ON, VLEVEL_L,"Hop_Rx_Freq%d\r\n",(RF_FREQUENCY-freq_off) + NextChannel*freq_step);
    APP_LOG(TS_ON, VLEVEL_L,"Hop Channel RSSI %d\r\n", rssi);
  
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); /* when reset and viewed on a scope, the hop rate is visible */
   
     
}
/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/

static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
  APP_LOG(TS_ON, VLEVEL_L, "OnTxDone\n\r");  
  
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  UTIL_TIMER_Start(&timerTransmit);  //Transmit packet in 2s  
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
  APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
  APP_LOG(TS_ON, VLEVEL_L,  "RssiValue=%d dBm, SnrValue=%d\n\r", rssi, LoraSnr_FskCfo);
  
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
 
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  
  for (int32_t i = 0; i < BufferSize; i++)
    {
         APP_LOG(TS_OFF,VLEVEL_L, "%x",Buffer[i]);
    }
    PFlag = 0;
 
   
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);  /* restart the Rx process once a packet is received */
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
  APP_LOG(TS_ON, VLEVEL_L, "RxTimeOut\n\r");
    
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */
void OnPreambleDetect(void)
{
 
  UTIL_TIMER_Stop(&timerReceive);  /* stop the hop clock timer as soon as a preamble is detected */
  Radio.SetChannel((RF_FREQUENCY-freq_off) + NextChannel*freq_step);  /* monitor channel we are switching to */
  
  /* Next two statements, clear the Preamble IRQ to prevent multiple trigger */
  SUBGRF_ClearIrqStatus(IRQ_PREAMBLE_DETECTED); 
  SUBGRF_SetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT | IRQ_CRC_ERROR | IRQ_HEADER_ERROR,
                          IRQ_RADIO_NONE,
                          IRQ_RADIO_NONE);
  
 // Radio.SetChannel((RF_FREQUENCY-freq_off) + (NextChannel-1)*freq_step);  /* monitor channel we came from */
//  last_rssi = SUBGRF_GetRssiInst();
//  APP_LOG(TS_ON, VLEVEL_L,"RSSI-1 %d\r\n", rssi);
//  Radio.SetChannel((RF_FREQUENCY-freq_off) + NextChannel*freq_step);  /* monitor channel we are switching to */
  rssi = SUBGRF_GetRssiInst();
  APP_LOG(TS_ON, VLEVEL_L,"RSSI %d\r\n", rssi);
    
  APP_LOG(TS_ON, VLEVEL_L, "On_Preamble_Done\n\r");
  APP_LOG(TS_ON, VLEVEL_L,"Preamble_Detect_Freq%d\r\n",(RF_FREQUENCY-freq_off) + NextChannel*freq_step);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);


PFlag = 1;

}

static void CadDone(bool channelActivityDetected)
{
  UTIL_TIMER_Stop(&timerReceive);
  APP_LOG(TS_ON, VLEVEL_L, "CadDone Check Rx Buffer\n\r");
  CadFlag = 1;
  
   SUBGRF_ClearIrqStatus( IRQ_CAD_CLEAR );      /* clear the IRQ */

   UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_CaD_Process), CFG_SEQ_Prio_0);   /* go CAD_Process() to receive */
  
}

/* USER CODE END PrFD */
