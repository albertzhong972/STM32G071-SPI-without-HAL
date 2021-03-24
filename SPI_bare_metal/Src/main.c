/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/*
 * Written by Albert Zhong
 * Purpose: Implementing SPI without HAL on a STM32G071RB microcontroller
 * SPI config settings:
 *
 * */

#include <../Inc/stm32g071xx.h>

//25LC1024 EEPROM instructions
const uint8_t EEPROM_READ = 0b00000011;
const uint8_t EEPROM_WRITE = 0b00000010;
const uint8_t EEPROM_WREN = 0b00000110;
const uint8_t EEPROM_WRDI = 0b00000100;
const uint8_t EEPROM_RDSR = 0b00000101;
const uint8_t EEPROM_WRSR = 0b00000001;

void GPIO_Init(void);
void GPIOtest_Init(void);
void SPI1_Config(void);
void SPI1_SendChar (uint8_t c);
void SPI1_SendString (char * s);


/*
 * GPIO_Init: Initialize PA11, PA12, PD8, PD9 for SPI communication
 * PC8 is used as SPI Chip Select
 */
void GPIO_Init(void) {

	RCC->IOPENR = 0x1; //enable Port A
	RCC->IOPENR |= (1 << 3); //enable Port D
	RCC->IOPENR |= (1 << 2); //enable Port C
	GPIOC->MODER &= ~(3 << 26);	//set PC13 as input
	GPIOC->MODER &= ~(1 << 17);	//set PC8 as output

	RCC->APBENR2 |= (1 << 12); //SPI1 clock enable

	GPIOA->MODER &= ~(1 << 22);	//Set PA11 to use alternate function
	GPIOA->MODER &= ~(1 << 24);	//Set PA12 to use alternate function
	GPIOD->MODER &= ~(1 << 16);	//Set PD8 to use alternate function
	GPIOD->MODER &= ~(1 << 18);	//Set PD9 to use alternate function

	GPIOA->AFR[1] |= (0 << 12);	//Use AF0 (SPI1_MISO) on PA11
	GPIOA->AFR[1] |= (0 << 16);	//Use AF0 (SPI1_MOSI) on PA12
	GPIOD->AFR[1] |= (1 << 0);	//Use AF1 (SPI1_SCK) on PD8
	GPIOD->AFR[1] |= (1 << 4);	//Use AF1 (SPI1_NSS) on PD9

}

void SPI1_Config(void) {
	GPIOC->ODR |= (1 << 8);

	SPI1->CR1 |= (1 << 2);	//Master configuration

	SPI1->CR1 |= (1 << 4);	//set baud rate to fclk / 8
	SPI1->CR1 |= (0 << 0); //CPHA = 0
	SPI1->CR1 |= (0 << 1); //CPOL = 0
	SPI1->CR1 |= (0 << 10); //Full-duplex mode
	SPI1->CR1 |= (0 << 7); //MSB transmitted first

	//CRC disabled

	SPI1->CR2 |= (7 << 8); //Data size = 8 bits
	//SPI1->CR2 |= (1 << 12);	//RXNE event generated when fifo level is greater than or equal to 1/4 (8-bit)
	SPI1->CR2 |= (1 << 2); //SS output enabled in master mode
	//NSSP disabled
	//RXNE event is generated if the FIFO level is greater than or equal to 1/2 (16-bit)

	//No CRC
	//No DMA

	SPI1->CR1 |= (1 << 6); //SPI enabled
}

void SPI1_SendChar (uint8_t c) {
	GPIOC->ODR &= ~(1 << 8);
	*((uint8_t*) &SPI1->DR) = c;	//uint8_t pointer cast is needed otherwise there will be extra clock pulses
	while(!(SPI1->SR & (1 << 1)));
	while((SPI1->SR & (1 << 7))); //wait for the BSY flag to clear
	GPIOC->ODR |= (1 << 8);

}

void SPI1_SendString (char * s) {
	GPIOC->ODR &= ~(1 << 8);
	while(*s != 0)
	{
		*((uint8_t*) &SPI1->DR) = *s;
		while(!(SPI1->SR & (1 << 1)));
		s++;
	}
	while((SPI1->SR & (1 << 7))); //wait for the BSY flag to clear
	GPIOC->ODR |= (1 << 8);

}


int main(void)
{
	GPIO_Init();
	SPI1_Config();
	int pressed = 0;

	while(1)
	{
		if(!((GPIOC -> IDR) & 0x2000) && pressed == 0)	//When button at PC13 is pressed, send SPI string
		{
			SPI1_SendString("ALBERT");
			pressed = 1;
		}

	}
}
