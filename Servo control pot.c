//El presente programa se diseñó para controlar un servomotor con un PIC18F4550 y un potenciómetro.
//El potenciómetro se conectó al pin RA0 y el pin de control del servomotor al pin RD0 del PIC.
#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pic18f4550.h>
//VARIABLES GLOBALES
unsigned long grados = 0;
unsigned long pot =0;
 int flag=1;
 //FUNCIONES 
void servo_control (void);
//INICIO DEL PROGRAMA
void main(void) {
    TRISA = 0X0F;
    TRISD = 0X00;
    INTCONbits.GIE = 0;
    PIE1bits.ADIE = 0;
    INTCONbits.PEIE_GIEL = 0;
    INTCONbits.TMR0IE = 1;
    ADCON1 = 0B00001110;
    //CONFIGURACIÓN DEL ADC
    //MIN. TAD 0.8uS; frecuencia configurada 48 Mhz
//    Fosc necesario 64 y TAD necesario de 2
    ADCON2 = 0B10001110;
    ADCON0bits.CHS = 0B0000;
    ADCON0bits.ADON = 1;
    ADCON0bits.GO_nDONE = 1;
    //CONFIGURACIÓN DEL TIMER0 CON UN PRESCALER DE 4, EL CUAL EN TOTAL
    //TENEMOS UNA DIVICIÓN ENTRE 16. (RECORDAR QUE LA FREQ DEL TIMER ESTÁ DADA
    //POR 4/FOSC. AL ASIGNAR EL PREESCALER TENEMOS QUE MULTIPLICAR 4 POR 3)
    //ASÍ TENEMOS QUE CADA 16 CICLOS DE RELOJ AUMENTA EN UNO EL CONTEO DEL TIMER0
    T0CON = 0B10000001; 
    //Se activan las interrupciónes y los periféricos
     INTCONbits.GIE = 1;
    INTCONbits.PEIE_GIEL = 1;
    //se resetea el registro del TMR0 y la bandera que indica el desbordamiento del mismo
    TMR0 = 0;
    TMR0IF = 0;
    //se carga el pin LD0 con el valor lógico de 0 o cero volts.
    LATDbits.LD0 = 0;
    while(1)    
    {             
           ADCON0bits.GO_DONE = 1;
          while(ADCON0bits.GO_DONE==1);    
          //Se manda llamar la función para controlar lo que se carga en el registro TMR0
          servo_control();       
    }
}

void __interrupt() my_isr(void)
{
     
    if (TMR0IF == 1)
    {       
        if (flag == 0) flag =2;
        if (flag == 3) flag = 1;      
        LATDbits.LD0 = !LATDbits.LD0 ;   
        TMR0IF = 0;
    }
  
}
void servo_control(void)
{
          //Es necesario poner las siguientes banderas para evitar que el registro TMR0 se sobreescriba a cada momento
          //con esta condición solamente hasta que se ingresa a la función de interrupción se cambia el valor y entonces
          //se actualiza el valor cargado al TMR0
       if (flag == 1){
              pot=ADRES;            
              //al realizar las cuentas necesarias (considerando que se configuró un prescaler de 16/48Mhz), tenemos que
              //para obtener un pulso de 1ms es necesario 3000 pulsos y para 2s el doble, es decir 6000. En este programa se adecuó
              //según el dispositivo físico el cuál no responde finamente a la frecuencia establecidad, por eso se ve en los valores 2000 en lugar de 3000 y 512 en lugar de 1023.
              //Se entiende que el registro ADRES incluye el valor del ADC es decir el ADRESL y ADRESH. Por esa razón se ha quitado el corrimiento de bits.
              //Para tener una frecuencia de 50Hz simplemente deberás cambiar el 2000 que aparece abajo por 3000
              grados =65535 - (1500+((pot*5500)/1023));
              TMR0 = grados;
              //la actualización de las banderas es necesario ponerlas al final, despues de cargar el registro TMR0
              //para evitar una acción en falso.
               flag=0;
          }    
          else if(flag == 2)
          {
              TMR0 = 65535 -(60000- (1500+((pot*5500)/1023)));
               //la actualización de las banderas es necesario ponerlas al final, despues de cargar el registro TMR0
              //para evitar una acción en falso.
              flag=3;
          }
}