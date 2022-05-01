/*
 * PS2.c
 *
 *  Created on: Apr 23, 2022
 *      Author: 27504
 */

#include "PS2.h"

PS2Buttons *PS2_PS2;
TIM_HandleTypeDef *PS2_TIMER;

void delay_us(uint16_t us) {
	__HAL_TIM_SET_COUNTER(PS2_TIMER, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(PS2_TIMER) < us)
		;  // wait for the counter to reach the us input in the parameter
}

uint8_t PS2_SendByte(uint8_t cmd) {
	uint8_t data = 0x00;
	uint16_t buffer = 0x01;
	for (buffer = 0x01; buffer < 0x0100; buffer <<= 1) {
		if (buffer & cmd) {
			HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, GPIO_PIN_RESET);
		}
		HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
		delay_us(8);
		HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
		delay_us(8);
		HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
		if (HAL_GPIO_ReadPin(MISO_GPIO_Port, MISO_Pin)) {
			data |= buffer;
		}
	}
	delay_us(8);
	return data;
}

void PS2_Cmd(uint8_t *transmit, uint8_t *receive, uint8_t length) {
	HAL_GPIO_WritePin(SS_GPIO_Port, SS_Pin, GPIO_PIN_RESET);
	delay_us(8);
	for (uint8_t i = 0; i < length; i++) {
		receive[i] = PS2_SendByte(transmit[i]);
	}
	HAL_GPIO_WritePin(SS_GPIO_Port, SS_Pin, GPIO_PIN_SET);
	delay_us(8);
}

void PS2_Poll() {
	uint8_t transmit[5] = { 0x01, 0x42 };
	uint8_t receive[5] = { 0x00 };
	PS2_Cmd(transmit, receive, 5);
}

void PS2_EnterConfig() {
	uint8_t transmit[5] = { 0x01, 0x43, 0x00, 0x01 };
	uint8_t receive[5] = { 0x00 };
	PS2_Cmd(transmit, receive, 5);
}

void PS2_AnalogMode() {
	uint8_t transmit[9] = { 0x01, 0x44, 0x00, 0x01, 0x03 };
	uint8_t receive[9] = { 0x00 };
	PS2_Cmd(transmit, receive, 9);
}

void PS2_ExitConfig() {
	uint8_t transmit[9] = { 0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A };
	uint8_t receive[9] = { 0x00 };
	PS2_Cmd(transmit, receive, 9);
}

void PS2_Init(TIM_HandleTypeDef *timer, PS2Buttons *ps2) {
	// Set hardware
	PS2_TIMER = timer, PS2_PS2 = ps2;
	// Enable timer
	HAL_TIM_Base_Start(PS2_TIMER);
	// Enable analog mode
	PS2_Poll();
	PS2_EnterConfig();
	PS2_AnalogMode();
	PS2_Poll();
	PS2_ExitConfig();
}

void PS2_Update() {
	uint8_t transmit[9] = { 0x01, 0x42 };
	uint8_t receive[9] = { 0x00 };
	PS2_Cmd(transmit, receive, 9);
	if (receive[1] != 0x41 && receive[1] != 0x73) return;
	uint16_t buttonStatus = receive[3] | (receive[4] << 8);
	PS2_PS2->SELECT = !((buttonStatus >> 0) & 0x01);
	PS2_PS2->L3 = !((buttonStatus >> 1) & 0x01);
	PS2_PS2->R3 = !((buttonStatus >> 2) & 0x01);
	PS2_PS2->START = !((buttonStatus >> 3) & 0x01);
	PS2_PS2->UP = !((buttonStatus >> 4) & 0x01);
	PS2_PS2->RIGHT = !((buttonStatus >> 5) & 0x01);
	PS2_PS2->DOWN = !((buttonStatus >> 6) & 0x01);
	PS2_PS2->LEFT = !((buttonStatus >> 7) & 0x01);
	PS2_PS2->L2 = !((buttonStatus >> 8) & 0x01);
	PS2_PS2->R2 = !((buttonStatus >> 9) & 0x01);
	PS2_PS2->L1 = !((buttonStatus >> 10) & 0x01);
	PS2_PS2->R1 = !((buttonStatus >> 11) & 0x01);
	PS2_PS2->TRIANGLE = !((buttonStatus >> 12) & 0x01);
	PS2_PS2->CIRCLE = !((buttonStatus >> 13) & 0x01);
	PS2_PS2->CROSS = !((buttonStatus >> 14) & 0x01);
	PS2_PS2->SQUARE = !((buttonStatus >> 15) & 0x01);
	if (receive[1] != 0x73) return;
	PS2_PS2->RX = receive[5];
	PS2_PS2->RY = receive[6];
	PS2_PS2->LX = receive[7];
	PS2_PS2->LY = receive[8];
}
