#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_swm.h"
#include "pin_mux.h"


/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M0P */
void BOARD_InitPins(void)
{
    /* Enables clock for IOCON.: enable */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Enables clock for switch matrix.: enable */
    CLOCK_EnableClock(kCLOCK_Swm);
    /* Enables the clock for the GPIO0 module */
    CLOCK_EnableClock(kCLOCK_Gpio0);

    const uint32_t IOCON_INDEX_PIO0_config = (/* No addition pin function */
                                                     IOCON_PIO_MODE_INACT |
                                                     /* Enable hysteresis */
                                                     IOCON_PIO_HYS_EN |
                                                     /* Input not invert */
                                                     IOCON_PIO_INV_DI |
                                                     /* Disables Open-drain function */
                                                     IOCON_PIO_OD_DI);

	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_17, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_13, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_12, IOCON_INDEX_PIO0_config);
	//IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_5, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_4, IOCON_INDEX_PIO0_config);
	//IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_3, IOCON_INDEX_PIO0_config);
	//IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_2, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_11, IOCON_INDEX_PIO0_config);

	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_10, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_21, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_29, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_28, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_27, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_26, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_20, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_15, IOCON_INDEX_PIO0_config);

	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_1, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_9, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_8, IOCON_INDEX_PIO0_config);
	//vdd
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_30, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_7, IOCON_INDEX_PIO0_config);
	//vrefp
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_0, IOCON_INDEX_PIO0_config);

	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_14, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_19, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_25, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_24, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_23, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_22, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_18, IOCON_INDEX_PIO0_config);
	IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_16, IOCON_INDEX_PIO0_config);

    /* USART0_TXD connect to P0_13 */
    //SWM_SetMovablePinSelect(SWM0, kSWM_USART0_TXD, kSWM_PortPin_P0_13);
    /* I2C0_SDA connect to P0_13 */
    SWM_SetMovablePinSelect(SWM0, kSWM_I2C0_SDA, kSWM_PortPin_P0_13);

    /* USART0_RXD connect to P0_17 */
    //SWM_SetMovablePinSelect(SWM0, kSWM_USART0_RXD, kSWM_PortPin_P0_17);
    /* I2C0_SCL connect to P0_17 */
    SWM_SetMovablePinSelect(SWM0, kSWM_I2C0_SCL, kSWM_PortPin_P0_17);

    /* TimerMatchChannel0 connect to P0_8 */
    SWM_SetMovablePinSelect(SWM0, kSWM_T0_MAT_CHN0, kSWM_PortPin_P0_8);

    /* CAPT_X0 connect to P0_20 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_X0, kSWM_PortPin_P0_20);

    /* CAPT_X1 connect to P0_26 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_X1, kSWM_PortPin_P0_26);

    /* CAPT_X2 connect to P0_21 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_X2, kSWM_PortPin_P0_21);

    /* CAPT_X3 connect to P0_10 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_X3, kSWM_PortPin_P0_10);

    /* CAPT_X4 connect to P0_30 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_X4, kSWM_PortPin_P0_30);

    /* CAPT_YL connect to P0_15 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_YL, kSWM_PortPin_P0_15);

    /* CAPT_YH connect to P0_9 */
    SWM_SetMovablePinSelect(SWM0, kSWM_CAPT_YH, kSWM_PortPin_P0_9);

    /* ACMP_INPUT2 connect to P0_1 */
    SWM_SetFixedPinSelect(SWM0, kSWM_ACMP_INPUT2, true);

    gpio_pin_config_t STB_config = {
            .pinDirection = kGPIO_DigitalOutput,
            .outputLogic = 1U,
        };
    GPIO_PinInit(BOARD_INITPINS_STB_GPIO, BOARD_INITPINS_STB_PORT, BOARD_INITPINS_STB_PIN, &STB_config);

    gpio_pin_config_t LED_S1_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U,
    };
    GPIO_PinInit(BOARD_INITPINS_LED_S1_GPIO, BOARD_INITPINS_LED_S1_PORT, BOARD_INITPINS_LED_S1_PIN, &LED_S1_config);

    gpio_pin_config_t DIO_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U,
    };
    GPIO_PinInit(BOARD_INITPINS_DIO_GPIO, BOARD_INITPINS_DIO_PORT, BOARD_INITPINS_DIO_PIN, &DIO_config);

    gpio_pin_config_t LED_S2_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U,
    };
    GPIO_PinInit(BOARD_INITPINS_LED_S2_GPIO, BOARD_INITPINS_LED_S2_PORT, BOARD_INITPINS_LED_S2_PIN, &LED_S2_config);

    gpio_pin_config_t LED_S3_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U,
    };
    GPIO_PinInit(BOARD_INITPINS_LED_S3_GPIO, BOARD_INITPINS_LED_S3_PORT, BOARD_INITPINS_LED_S3_PIN, &LED_S3_config);

    gpio_pin_config_t LED_S4_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U,
    };
    GPIO_PinInit(BOARD_INITPINS_LED_S4_GPIO, BOARD_INITPINS_LED_S4_PORT, BOARD_INITPINS_LED_S4_PIN, &LED_S4_config);

    gpio_pin_config_t CLK_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U,
    };
    GPIO_PinInit(BOARD_INITPINS_CLK_GPIO, BOARD_INITPINS_CLK_PORT, BOARD_INITPINS_CLK_PIN, &CLK_config);

    /* Disable clock for switch matrix. */
    CLOCK_DisableClock(kCLOCK_Swm);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
