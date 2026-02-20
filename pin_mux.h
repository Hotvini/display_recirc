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

/*! @name PIO0_27 (number 13), STB
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_STB_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_STB_GPIO_PIN_MASK (1U << 27U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_STB_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_STB_PIN 27U                   /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_29 (number 11), DIO
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_DIO_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_DIO_GPIO_PIN_MASK (1U << 29U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_DIO_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_DIO_PIN 29U                   /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_28 (number 12), CLK
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CLK_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CLK_GPIO_PIN_MASK (1U << 28U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CLK_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_CLK_PIN 28U                   /*!<@brief PORT pin number */
                                                     /* @} */

/*! @name PIO0_16 (number 32), LED_S4
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_LED_S4_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_LED_S4_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_LED_S4_PIN 16U                   /*!<@brief PORT pin number */
                                                        /* @} */

/*! @name PIO0_18 (number 31), LED_S3
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_LED_S3_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_LED_S3_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_LED_S3_PIN 18U                   /*!<@brief PORT pin number */
                                                        /* @} */

/*! @name PIO0_22 (number 30), LED_S2
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_LED_S2_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_LED_S2_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_LED_S2_PIN 22U                   /*!<@brief PORT pin number */
                                                        /* @} */

/*! @name PIO0_23 (number 29), LED_S1
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_LED_S1_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_LED_S1_PORT 0U                   /*!<@brief PORT device index: 0 */
#define BOARD_INITPINS_LED_S1_PIN 23U                   /*!<@brief PORT pin number */
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
