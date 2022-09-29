/*
 * Author   :   Icaro Fernando
 * Date     :   28/09/2022 (dd/mm/yy)
 * Github   :   github.com/icarofcb
 * Linkedin :   linkedin.com/in/ícaro-fernando-46139919b/?tab=repositories
 *
 * Description  : This is an code that works on Renesas S7G2 SK Evaluation Board.
 * It sends a string through SCI UART 0 each time you press one of the buttons, SW4 or SW5.
 * The string is sent with the name of the pressed button "SW4" or "SW5".
 *
 * With TX short circuited to RX(using a jumper), it receives the string sent by the button's interrupt function, the it
 * validates the information received and blinks the assigned led, LED GREEN for SW4 and LED RED for SW5
***********************************************************************************************************************/

#include "hal_data.h"
#include "string.h"
#include "stdio.h"

#define LEDVERDE        IOPORT_PORT_06_PIN_00   // Green  LED
#define LEDVERMELHO     IOPORT_PORT_06_PIN_01   // Red    LED
#define LEDAMARELO      IOPORT_PORT_06_PIN_02   // Yellow LED

void clearBuffer();                             // clears the buffer received and store the data on data variable
void ligaLed();                                 // it turns the desired LED on for 3 seconds



volatile uint8_t bufRx[16]      = "";           //buffer just to receive our info
volatile uint8_t data[16]       = "";           //stores the last data received by buffer

volatile uint8_t dataRxFinished = 0;            //flag to show that the last byte '\n' has been received

volatile uint32_t j = 0;                        //a variable to be incremented to fill the buffer

void user_uart_callback(uart_callback_args_t *p_args)
{

    if(!dataRxFinished)
    {
        if(UART_EVENT_RX_CHAR==p_args->event)
        {
            bufRx[j] = (uint8_t) p_args->data;  //stores the char received on the buffer
            j++;                                //increments the buffer address
        }
    }

}

/* This function is a callback for the on board switch 4, it sends a string with it's name */
void sw4_callback(external_irq_callback_args_t *p_args)
{
    SSP_PARAMETER_NOT_USED(p_args);

    uint8_t msg[] = "SW4\n";

    g_uart0.p_api->write(g_uart0.p_ctrl, (uint8_t *)&msg, strlen( (char *)msg ) );

    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
}

/* This function is a callback for the on board switch 5, it sends a string with it's name */
void sw5_callback(external_irq_callback_args_t *p_args)
{
    SSP_PARAMETER_NOT_USED(p_args);

    uint8_t msg[] = "SW5\n";

    g_uart0.p_api->write(g_uart0.p_ctrl, (uint8_t *)&msg, strlen( (char *)msg ) );

    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
}

/* this function saves our buffer into the data variable and clears the buffer for new info */
void clearBuffer()
{
    strcpy((char *)data, (char *)bufRx);
    j = 0;
    memset((char *)bufRx, 0,sizeof(bufRx));
}

/* this function validates the data variable(string received) then turns on the on board LEDs accordingly to the desired logic*/
void ligaLed()
{
    g_uart0.p_api->close(g_uart0.p_ctrl);

    if( !(strcmp((char *)data, "SW4\n" )) )
    {
        g_ioport.p_api->pinWrite(LEDVERDE, IOPORT_LEVEL_LOW);

        R_BSP_SoftwareDelay(3000, BSP_DELAY_UNITS_MILLISECONDS);

        g_ioport.p_api->pinWrite(LEDVERDE, IOPORT_LEVEL_HIGH);


    }else if( !(strcmp((char *)data, "SW5\n" )) )
    {
        g_ioport.p_api->pinWrite(LEDVERMELHO, IOPORT_LEVEL_LOW);

        R_BSP_SoftwareDelay(3000, BSP_DELAY_UNITS_MILLISECONDS);

        g_ioport.p_api->pinWrite(LEDVERMELHO, IOPORT_LEVEL_HIGH);
    }

    memset((char *)data, 0,sizeof(data));

    g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);

    dataRxFinished = 0;
}
/* main function */
void hal_entry(void) {

    ioport_level_t level = IOPORT_LEVEL_HIGH;

    g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);                             //initialize uart 0

    g_external_irq10.p_api->open(g_external_irq10.p_ctrl,g_external_irq10.p_cfg);   //initialize sw4 interrupt
    g_external_irq11.p_api->open(g_external_irq11.p_ctrl,g_external_irq11.p_cfg);   //initialize sw5 interrupt

    uint8_t countDelay = 0;                                                         //variable to work as a non blocking delay

    while(1)
    {
        if(countDelay>5)                                                            //each 500mS toggles Yellow LED
        {
            countDelay = 0;

            if(IOPORT_LEVEL_LOW == level)
            {
                level = IOPORT_LEVEL_HIGH;

            }
            else
            {
                level = IOPORT_LEVEL_LOW;
            }


            g_ioport.p_api->pinWrite(LEDAMARELO, level);


        }else countDelay++;

        ////////////////////////////////

        if(strchr((char *)bufRx, '\n') != NULL)                                     //function to check if info has finished being received
        {
            dataRxFinished = 1;

            clearBuffer();                                                          //clears the buffer and saves the data

            ligaLed();                                                              //turns on LED

        }

        ////////////////////////////////

        R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);                     //small value delay

    }
}
