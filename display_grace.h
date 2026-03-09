/*
 * display_grace.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#ifndef DISPLAY_GRACE_H_
#define DISPLAY_GRACE_H_

/**
 * @file grace.h
 * @brief Driver HAL para display rodando baseado no TM1629A.
 * @author Vinicius Andrade
 * @date Dezembro, 2025
 * @version 1.0
 *
 * Este arquivo define a abstrção HAL e mapeamento
 * do display físico em relação ao controlador.
 */

#include "tm1629a.h"
#include <stdbool.h>

#ifndef  __GRACE_H__
#define  __GRACE_H__
// todo: remover include guard duplicado e manter apenas DISPLAY_GRACE_H_.

#include "stdint.h"

#ifdef  __cplusplus
#define GRACE_BEGIN  extern "C" {
#define GRACE_END    }
#else
#define GRACE_BEGIN
#define GRACE_END
#endif

GRACE_BEGIN

typedef enum {
    GRACE_ICON_WIFI,
    GRACE_ICON_PUMP,
    GRACE_ICON_CELSIUS,
    GRACE_ICON_CONFIG_MODE,
    GRACE_ICON_MANUAL_MODE,
    GRACE_ICON_AUTO_MODE,
    GRACE_ICON_ALERT,
    GRACE_ICON_COUNT
} grace_icon_id_t;

typedef enum {
    GRACE_WEEK_DOM,
    GRACE_WEEK_SEG,
    GRACE_WEEK_TER,
    GRACE_WEEK_QUA,
    GRACE_WEEK_QUI,
    GRACE_WEEK_SEX,
    GRACE_WEEK_SAB,
    GRACE_WEEK_COUNT
} grace_week_id_t;

typedef enum {
    GRACE_DOT0_THERMOMETER,
    GRACE_DOT1_THERMOMETER,
    GRACE_DOT0_CLOCK,
    GRACE_DOT1_CLOCK,
    GRACE_DOT_COUNT
} grace_dot_id_t;

typedef enum {
    GRACE_DIGIT0_THERMOMETER,
    GRACE_DIGIT1_THERMOMETER,
    GRACE_DIGIT2_THERMOMETER,
    GRACE_DIGIT0_CLOCK,
    GRACE_DIGIT1_CLOCK,
    GRACE_DIGIT2_CLOCK,
    GRACE_DIGIT3_CLOCK,
    GRACE_DIGIT_COUNT
} grace_digit_id_t;

/**
 * @brief Controle individual de ícones no display.
 *
 * @param icon Ícone a ser controlado.
 * @param on true para ligar o ícone, false para desligar.
 *
 * @return 0 em sucesso, -1 se o valor for inválido.
 */
int grace_icon_set(grace_icon_id_t icon, bool state);

/**
 * @brief Controle individual de dias da semana no display.
 *
 * @param week Dia da semana a ser controlado.
 * @param on true para ligar o dia, false para desligar.
 *
 * @return 0 em sucesso, -1 se o valor for inválido.
 */
int grace_week_set(grace_week_id_t week, bool state);

/**
 * @brief Controle individual dos pontos decimais no display.
 *
 * @param dot Ponto decimal a ser controlado.
 * @param on true para ligar o ponto, false para desligar.
 *
 * @return 0 em sucesso, -1 se o valor for inválido.
 */
int grace_dot_set(grace_dot_id_t dot, bool state);

/**
 * @brief Define o valor de um dígito no display.
 *
 * @param digit Dígito a ser definido.
 * @param value Valor (0-F) a ser exibido no dígito.
 *
 * @return 0 em sucesso, -1 se o valor for inválido.
 */
int grace_digit_set(grace_digit_id_t digit, uint8_t value);

/**
 * @brief Liga todos os segmentos do display.
 *
 * @return 0 em sucesso, -1 se ocorrer erro.
 */
int grace_all_on(void);

/**
 * @brief Desliga todos os segmentos do display.
 *
 * @return 0 em sucesso, -1 se ocorrer erro.
 */
int grace_all_off(void);

GRACE_END

#endif


#endif /* DISPLAY_GRACE_H_ */
