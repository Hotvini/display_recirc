/*
 * tm1629a.h
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */


/**
 * @file tm1629a.h
 * @brief Driver HAL para o controlador de display com TM1629A.
 *
 * Este arquivo define a API pública do driver TM1629A, incluindo:
 * - Tipos de dados públicos
 * - Estrutura de abstração de hardware (HAL)
 * - Funções de controle de display, buffer e brilho
 *
 * O driver é independente de plataforma e depende do usuário para
 * fornecer funções de controle de GPIO via HAL.
 */

#ifndef  __TM1629A_H__
#define  __TM1629A_H__

#include "stdint.h"

#ifdef  __cplusplus
#define TM1629A_BEGIN  extern "C" {
#define TM1629A_END    }
#else
#define TM1629A_BEGIN
#define TM1629A_END
#endif

TM1629A_BEGIN

#define  TM1629A_CONNECT_TYPE_ANODE    1
#define  TM1629A_CONNECT_TYPE_CATHODE  2
#define  TM1629A_CONNECT_TYPE          TM1629A_CONNECT_TYPE_CATHODE
// todo: mover tipo de conexao para configuracao de placa/build para evitar edicao manual deste header.

#define CLK_DELAY_TIME                 20 // tempo de delay do clock em ns
// todo: recalibrar/parametrizar delay de clock por frequencia real de CPU.

typedef enum {
  DIS_CTRL_DUTY_1_16 = 1, // 0
  DIS_CTRL_DUTY_2_16, // 1
  DIS_CTRL_DUTY_4_16, // 2
  DIS_CTRL_DUTY_10_16, // 3
  DIS_CTRL_DUTY_11_16, // 4
  DIS_CTRL_DUTY_12_16, // 5
  DIS_CTRL_DUTY_13_16, // 6
  DIS_CTRL_DUTY_14_16  // 7
} tm1629a_brightness_t;

typedef struct{
void (*clk_rise)(void);
void (*clk_down)(void);
void (*data_set)(void);
void (*data_clr)(void);
void (*stb_set)(void);
void (*stb_clr)(void);
}tm1629a_hal_driver_t;

/**
 * @brief Registra o driver HAL de GPIO.
 *
 * Deve ser chamado antes de qualquer outra função do driver.
 *
 * @param hal_driver Ponteiro para a estrutura HAL preenchida.
 * @return 0 em sucesso, -1 em caso de ponteiro inválido.
 */
int tm1629a_register_hal_driver(tm1629a_hal_driver_t *hal_driver);

/**
 * @brief Inicializa o TM1629A.
 *
 * Configura o modo de endereçamento, liga o display
 * e define brilho máximo como padrão.
 *
 * @return 0 em sucesso, -1 em erro.
 */
int tm1629a_init();

/**
 * @brief Limpa o buffer interno do display.
 *
 * Não atualiza o display físico automaticamente.
 *
 * @return 0 em sucesso.
 */
int tm1629a_buffer_clean();

/**
 * @brief Atualiza o buffer interno do display.
 *
 * Copia @p cnt bytes para o buffer interno a partir do endereço @p addr.
 *
 * @param addr Endereço inicial do display (0–15).
 * @param update Ponteiro para os dados de segmentos.
 * @param cnt Quantidade de bytes a copiar.
 *
 * @return 0 em sucesso, -1 em erro de parâmetro.
 */
int tm1629a_buffer_update(uint8_t addr,uint8_t *update,uint8_t cnt);

/**
 * @brief Atualiza o buffer interno do display com máscara.
 *
 * Liga ou desliga segmentos específicos no byte do buffer.
 * Ideal para setar icones ou indicadores individuais.
 *
 * @param addr Endereço do display (0–15).
 * @param mask Máscara de bits para os segmentos a alterar.
 * @param on true para ligar os segmentos, false para desligar.
 *
 * @return 0 em sucesso, -1 em erro de parâmetro.
 */
int tm1629a_buffer_update_bit_state(uint8_t addr,uint8_t mask,uint8_t state);

/**
 * @brief Atualiza o buffer interno do display com máscara e valor.
 *
 * Altera segmentos específicos no byte do buffer conforme o valor.
 * Ideal para controle dos segmentos de dígitos.
 *
 * @param addr Endereço do display (0–15).
 * @param mask Máscara de bits para os segmentos a alterar.
 * @param value Valor dos segmentos a serem definidos.
 *
 * @return 0 em sucesso, -1 em erro de parâmetro.
 */
int tm1629a_buffer_update_masked(uint8_t addr,uint8_t mask,uint8_t value);

/**
 * @brief Escreve um único dígito diretamente no display.
 *
 * Utiliza modo de endereçamento FIXED.
 * Não utiliza o buffer interno.
 *
 * @param addr Endereço do dígito (0–15).
 * @param data Byte de segmentos a ser escrito.
 *
 * @return 0 em sucesso.
 */
int tm1629a_write_digit(uint8_t addr, uint8_t data);

/**
 * @brief Envia o buffer interno para o display físico.
 *
 * Deve ser chamado após tm1629a_buffer_update().
 *
 * @return 0 em sucesso.
 */
int tm1629a_display_refresh();

/**
 * @brief Ajusta o brilho do display.
 *
 * @param brightness Nível de brilho ou 0 para desligar o display.
 *
 * @return 0 em sucesso, -1 se o valor for inválido.
 */
int tm1629a_brightness(tm1629a_brightness_t brightness );

TM1629A_END

#endif
