/*****************************************************************************
 * Gopal_power_consumption_test.c
 *
 * Author: Gopal Vishwakarma
 * email: gopalvishwakarma19@yahoo.com
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Gopal_power_consumption_test.h"
#include <drivers/pwr/adi_pwr.h>
#include <drivers/gpio/adi_gpio.h>
#include <drivers/adc/adi_adc.h>
#include <common.h>


ADI_ADC_HANDLE hADC = NULL;
uint8_t ADCMemory[ADI_ADC_MEMORY_SIZE];

#define ADC0_PIN 3
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];

// need to configure right pin mux register values for adc pin
void pinmux(){
  *((volatile uint32_t *)REG_GPIO2_CFG) = ((uint16_t) ((uint16_t) 1<<(ADC0_PIN * 2)));
}

// adc interrupt handler
void adc_int_handler(void *pCBParam, uint32_t Event, void *pArg){
  switch(Event){
  case  ADI_ADC_EVENT_HIGH_LIMIT_CROSSED:                       // LED4 turns off, LED5 turns on
    adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_10);         //*   Red LED on GPIO42 (DS4) ---RED LED ONN HERE*/
    adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_2);  			 /* Green LED on GPIO34 (DS3)----GREEN LED OFF */
    break;
  case  ADI_ADC_EVENT_LOW_LIMIT_CROSSED:                        // LED4 turns on, LED5 turns off
    adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_2);  			 /* Green LED on GPIO34 (DS3)----GREEN LED ON */
    adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_10);         //*   Red LED on GPIO42 (DS4) ---RED LED OFF*/
    break;
  default:
    break;
  }
}

int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	

	/* Begin adding your custom code here */
	bool bADCReady = false, bCalibrationDone = false;

	// setup for system and gpio
	  SystemInit();
	  adi_gpio_Init((void*)gpioMemory, ADI_GPIO_MEMORY_SIZE);
	  adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_10, true);
	  adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_2, true);
	  adi_gpio_OutputEnable(ADI_GPIO_PORT0, ADI_GPIO_PIN_13, true);

	 // setup for ADC component
		adi_adc_Open(0, ADCMemory, sizeof(ADCMemory), &hADC);
		adi_adc_PowerUp (hADC, true);
		adi_adc_SetVrefSource (hADC, ADI_ADC_VREF_SRC_INT_2_50_V);
		adi_adc_EnableADCSubSystem (hADC, true);

	bADCReady = false;
	  while (bADCReady == false) {
		adi_adc_IsReady (hADC, &bADCReady);
	  }

	  adi_adc_StartCalibration (hADC);

	    bCalibrationDone = false;
	    while (bCalibrationDone == false) {
	      adi_adc_IsCalibrationDone (hADC, &bCalibrationDone);
	    }

	    // set the adc digital comparator
	     adi_adc_SetLowLimit(hADC, ADI_ADC_CHANNEL_0, true, 0x999);    // 0x666 = 1.0v  // 999 = 1.5v
	     adi_adc_SetHighLimit(hADC, ADI_ADC_CHANNEL_0, true, 0xae0);   // 0xccd = 2.0v   // ae0= 1.7v
	     adi_adc_SetNumMonitorCycles(hADC, ADI_ADC_CHANNEL_0, 4);

	     adi_adc_RegisterCallback(hADC, adc_int_handler, NULL);        // register interrupt handler
	     adi_adc_EnableDigitalComparator(hADC,true);               // enable the comparator

	     adi_gpio_SetHigh(ADI_GPIO_PORT0, ADI_GPIO_PIN_13);            // if LED3 is on, there's a problem ( system hang )

	     while(1){
	     }

	return 0;
}

