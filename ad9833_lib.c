#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ad9833_lib.h"
#include "spi.h"
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>

spi_t *spi;

uint16_t MSB = 0;            //FREQ REG MSB
uint16_t LSB = 0;            //FREQ REG LSB
uint32_t phaseVal= 0;        //Phase Value
uint16_t last_phase_val = 0; //Phase value in degree
uint32_t freq = 0;           // Value Frequency Register
uint16_t Wave;
float frequency;
float Phase;

static void AD9833_Write(uint16_t val)
{
        uint16_t tx = (val >> 8) | (val << 8);
        uint16_t rx;
        spi_transfer(spi, &tx, &rx, sizeof(val));
}
void AD9833_Scan(void)
{
//const char *signal_form [3] = {"SIN", "TRI", "SQR"};
printf("Signal form:\n");
scanf("%x", &Wave);
//scanf("%s", &signal_form);
printf("Frequency:\n");
scanf("%f", &frequency);
/*if (signal_form == "SIN")
  AD9833_Wave(SIN, frequency, 0.0);
  else if (signal_form == "TRI")
   AD9833_Wave(TRI, frequency, 0.0);
   else if (signal_form == "SQR")
     AD9833_Wave(SQR, frequency, 0.0);
    else
    AD9833_Wave(SIN, frequency, 0.0);*/
AD9833_Wave(Wave, frequency, 0.0);
}
void AD9833_Wave(uint16_t Wave,float frequency,float Phase)
{
last_phase_val += Phase;
        if (last_phase_val > 360) last_phase_val -= 360;
phaseVal = (uint32_t)(last_phase_val * 4096/360);
phaseVal |= 0xc000;

freq = ((frequency*pow(2,28))/FMCLK); // Tuning Word
MSB = ((freq & 0xFFFC000)>>14);
LSB = ((freq & 0x3FFF));
LSB |= 0x4000;
MSB |= 0x4000;

AD9833_Write(0x0100);
AD9833_Write(0x1000);
AD9833_Write(MSB);
AD9833_Write(0x0000);
AD9833_Write(LSB);
AD9833_Write(phaseVal);

switch(Wave)
{
       case 0:
         Wave = SIN;//SIN
         break;
       case 1:
         Wave = SQR;//SQR
         break;
       case 2:
         Wave = TRI;//TRI
         break;
       default:
         break;
}
        printf("shifted in Wave: 0x%02x\n", Wave);
        printf("shifted in FRQ: 0x%02x\n", freq);
        printf("shifted in Phase: 0x%02x\n", phaseVal);

AD9833_Write(Wave);
}

void AD9833_Reset(void)
{
     AD9833_Write(0x0100);
}

void AD9833_Init()
{
        unsigned int mode = SPI_MODE_3;
        spi_bit_order_t bit_order = MSB_FIRST;
        uint8_t bits_per_word = 16;
        uint32_t max_speed = 500000;

        spi_set_mode(spi, mode);
        spi_set_max_speed(spi, max_speed);
        spi_set_bit_order(spi, bit_order);
        spi_set_bits_per_word(spi, bits_per_word);

        printf("mode: %d\n", mode);
        printf("max speed: %d\n", max_speed);
        printf("bit_order: %d\n", bit_order);
        printf("bits per word: %d\n", bits_per_word);

        spi_open(spi, "/dev/spidev0.0", mode, max_speed);

        if (spi_open(spi, "/dev/spidev0.0", mode, max_speed) < 0) {
        printf("spi_open(): %s\n", spi_errmsg(spi));
        exit(1);
    }
    else
    {
    printf("Signal generator: AD9833 SPI...OK\n");
    }
    usleep(10);
}
