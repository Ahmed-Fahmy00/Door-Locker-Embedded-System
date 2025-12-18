#include "buzzer.h"
#include "../MCAL/dio.h"
#include <stdbool.h>

/* Buzzer is connected to PA5 (PORTA, PIN5) */
#define BUZZER_PORT PORTA
#define BUZZER_PIN  PIN5

void buzzer_init(void)
{
    /* Initialize PA5 as output for buzzer */
    DIO_Init(BUZZER_PORT, BUZZER_PIN, OUTPUT);
    buzzer_off();
}


void buzzer_on(void)
{
    DIO_WritePin(BUZZER_PORT, BUZZER_PIN, HIGH);
}


void buzzer_off(void)
{
    DIO_WritePin(BUZZER_PORT, BUZZER_PIN, LOW);
}
