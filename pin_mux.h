#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

#define IOCON_PIO_HYS_EN 0x20u     /*!<@brief Enable hysteresis */
#define IOCON_PIO_INV_DI 0x00u     /*!<@brief Input not invert */
#define IOCON_PIO_MODE_INACT 0x00u /*!<@brief No addition pin function */
#define IOCON_PIO_OD_DI 0x00u      /*!<@brief Disables Open-drain function */

/*! @name PIO0_4 (number 6), STB
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_STB_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_STB_GPIO_PIN_MASK (1U << 4U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_STB_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_STB_PIN 4U                    /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_10 (number 10), DIO
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_DIO_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_DIO_GPIO_PIN_MASK (1U << 10U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_DIO_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_DIO_PIN 10U                   /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_11 (number 9), CLK
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CLK_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CLK_GPIO_PIN_MASK (1U << 11U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CLK_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_CLK_PIN 11U                   /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_0 (number 19), LED_S1
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_LED_S1_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_LED_S1_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_LED_S1_PIN 0U                    /*!<@brief PORT pin number */
                                                        /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void); /* Function assigned for the Cortex-M0P */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
