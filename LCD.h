#include <msp430.h>
#define true 1
#define false 0
#define BR100K 11


char i2c_test(char addr);
void i2c_write(char dado);
void USCI_B0_config();
void lcd_aux(char dado);
void lcd_inic(void);
void lcd_write_char(char nibble);
void move_cursor(char nibble);
void leds_config();
void delay(long x);

void MensagemOk();
void MensagemPerigo();

char i2c_test(char addr){ //checkar endere�o para escrita

UCB0I2CSA = addr;
UCB0CTL1 |= UCTR; //mestre transmissor
UCB0CTL1 |= UCTXSTT; ;//start
while ((UCB0IFG & UCTXIFG)==0); //trava pra esperar TXIFG ir pra 1 (confirma start)

UCB0CTL1 |= UCTXSTP; //envia o stop e passa a aguardar pelo ack/nack
while((UCB0CTL1 & UCTXSTP)==UCTXSTP); //espera o stop

if ((UCB0IFG & UCNACKIFG)==0)
    return true; //houve ack
else
    return false; //houve nack
}

void i2c_write(char dado){
    UCB0CTL1 |= UCTR; //mestre transmissor
    UCB0CTL1 |= UCTXSTT; ;//start

    while ((UCB0IFG & UCTXIFG)==0);
    UCB0TXBUF = dado; //escreve o dado

    while ((UCB0CTL1 & UCTXSTT)==UCTXSTT); //espera start ir pra 0

    if ((UCB0IFG & UCNACKIFG)==UCNACKIFG){ //pro caso de receber nack (problema)
        P1OUT |= BIT0;
        while(true); //trava execu��o
    }

    UCB0CTL1 |= UCTXSTP; //gerar stop
    while((UCB0CTL1 & UCTXSTP)==UCTXSTP);//verdadeiro enquanto stop for 1


}

void USCI_B0_config(){
    UCB0CTL1 = UCSWRST; //ativa reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0BRW = BR100K; //clock freq
    UCB0CTL1 = UCSSEL_3; //smclk

    P3SEL |= BIT1 | BIT0;
    P3REN |= BIT1 | BIT0;
    P3OUT |= BIT1 | BIT0;
}

// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    //UCB0I2CSA = addr;    //Endere�o Escravo
    UCB0CTL1 |= UCTR    |   //Mestre TX
                UCTXSTT;    //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Sa�da PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
                while(1);

    // Come�ar inicializa��o
    lcd_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(2);     //2

    // Entrou em modo 4 bits
    lcd_aux(2);     lcd_aux(8);     //0x28
    lcd_aux(0);     lcd_aux(8);     //0x08
    lcd_aux(0);     lcd_aux(1);     //0x01
    lcd_aux(0);     lcd_aux(6);     //0x06
    lcd_aux(0);     lcd_aux(0xC);   //0x0C pro cursor n�o ficar piscando

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

// Auxiliar inicializa��o do LCD (RS=RW=0)
// *** S� serve para a inicializa��o ***
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;     //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //E=0;
}

void lcd_write_char(char nibble) {
    //char number;
    //Escrever um nibble na tela
        // Parte alta do nibble (4 bits mais significativos)
        i2c_write((nibble & 0xF0) | 0x09);  // Envia a parte alta com 0x09
        i2c_write((nibble & 0xF0) | 0x0D);  // Envia a parte alta com 0x0D
        i2c_write((nibble & 0xF0) | 0x09);  // Envia a parte alta com 0x09

        // Parte baixa do nibble (4 bits menos significativos)
        i2c_write((nibble << 4) | 0x09);    // Envia a parte baixa com 0x09
        i2c_write((nibble << 4) | 0x0D);    // Envia a parte baixa com 0x0D
        i2c_write((nibble << 4) | 0x09);    // Envia a parte baixa com 0x09
}

void move_cursor(char nibble){
      i2c_write((nibble & 0xF0) | 0x08);  // Envia a parte alta com 0x08
      i2c_write((nibble & 0xF0) | 0x0C);  // Envia a parte alta com 0x0C
      i2c_write((nibble & 0xF0) | 0x08);  // Envia a parte alta com 0x08

      // Parte baixa do nibble (4 bits menos significativos)
      i2c_write((nibble << 4) | 0x08);    // Envia a parte baixa com 0x08
      i2c_write((nibble << 4) | 0x0C);    // Envia a parte baixa com 0x0C
      i2c_write((nibble << 4) | 0x08);    // Envia a parte baixa com 0x08
}

void leds_config(){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}

void delay(long x){
    volatile long y;
    for (y = 0; y<x; y++);
}

void MensagemOk(){
       lcd_write_char('T');
       lcd_write_char('u');
       lcd_write_char('d');
       lcd_write_char('o');
       lcd_write_char(0x20);
       lcd_write_char('c');
       lcd_write_char('e');
       lcd_write_char('r');
       lcd_write_char('t');
       lcd_write_char('o');
       lcd_write_char(0x20);
       lcd_write_char(0x20);
       lcd_write_char(0x20);
       lcd_write_char(0x20);
       lcd_write_char(0x20);

       move_cursor(0x80); //colocar cursor no inicio
}

void MensagemPerigo(){
       lcd_write_char('A');
       lcd_write_char('L');
       lcd_write_char('E');
       lcd_write_char('R');
       lcd_write_char('T');
       lcd_write_char('A');
       lcd_write_char(0x20);
       lcd_write_char('I');
       lcd_write_char('N');
       lcd_write_char('C');
       lcd_write_char('E');
       lcd_write_char('N');
       lcd_write_char('D');
       lcd_write_char('I');
       lcd_write_char('O');

       move_cursor(0x80); //colocar cursor no inicio
}
