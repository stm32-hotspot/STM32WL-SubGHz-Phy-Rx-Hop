/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.h
  * @author  MCD Application Team
  * @brief   Header of application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SUBGHZ_PHY_APP_H__
#define __SUBGHZ_PHY_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* MODEM type: one shall be 1 the other shall be 0 */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   0

#define REGION_US915

#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 /* Hz */
#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 /* Hz */

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 /* Hz */

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 /* Hz */

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 /* Hz */

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 /* Hz */

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 /* Hz */

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 /* Hz */

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 /* Hz */

#else
#error "Please define a frequency band in the compiler options."
#endif /* REGION_XXxxx */

#define TX_OUTPUT_POWER                             0        /* dBm */

#if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR                       9         /* [SF7..SF12] */
#define LORA_CODINGRATE                             1         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        32         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         4         /* Symbols */
#define LORA_RX_PREAMBLE_LENGTH                     4         /* detection preamble symbols for LoRa Rx in hop mode */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true
#define LORA_IQ_INVERSION_ON                        false

#elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

#define FSK_FDEV                                    25000     /* Hz */
#define FSK_DATARATE                                50000     /* bps */
#define FSK_BANDWIDTH                               50000     /* Hz */
#define FSK_PREAMBLE_LENGTH                         8         /* Same for Tx and Rx */
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
#error "Please define a modem in the compiler subghz_phy_app.h."
#endif /* USE_MODEM_LORA | USE_MODEM_FSK */

#define PAYLOAD_LEN                                 64

/* USER CODE BEGIN EC */
#define RX_TIMEOUT_VALUE                            2000   /* STM */
#define TX_TIMEOUT_VALUE                            1000   /* STM */

#define TCXO_WORKAROUND_TIME_MARGIN                 5  /* 50ms margin */  /* STM */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init Subghz Application
  */
void SubghzApp_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__SUBGHZ_PHY_APP_H__*/
