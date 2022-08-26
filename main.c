#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include "spi.h"
#include "gpio.h"
#include "ad9833_lib.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "I2CWrapper.h"
#include "mpu6050.h"

spi_t *spi;

GForceStruct   Data;
int ExitFlag = 0;
int i2c_handle;
float Gtotal;
float GSumSquare;
float Temperature;
long  Xsum,Ysum,Zsum;
double accX, accY, accZ, gyroX, gyroY, gyroZ;
const int BUS = 1;
int I2C_Current_Slave_Adress = 0x68;

volatile int timer = 0;
uint16_t Wave;
float frequency;

void handler(int sig)
{
        (void)sig;
        ++timer;
        return;
}

void sig_handler(int signo)
{
  if (signo == SIGINT)
    printf("received SIGINT\n");
   ExitFlag=1;
}

int main()
{
    FILE *file_ptr;
    file_ptr = fopen("MPU_Data.txt", "w+a"); //File creation and writing
    if (file_ptr != NULL)
        {
        printf("File MPU_Data.txt created succesfully\n");
        }
        else
        {
        printf(stderr, "Failed to create or open file MPU_Data.txt\n");
        return 1;
        }
    fprintf(file_ptr, "MPU Data Read: \n");

        spi = spi_new();
        AD9833_Init();

    i2c_handle = I2CWrapperOpen(BUS,I2C_Current_Slave_Adress);
	if(i2c_handle <0) return -1;
    if(! MPU6050_Test_I2C(i2c_handle))
       {
         printf("Unable to detect MPU6050\n");
       }
       else
      {
           Setup_MPU6050(i2c_handle);

           while(!ExitFlag)
                {
           signal(SIGALRM, handler);
           alarm(20);
           AD9833_Scan();
           printf("Wave: %x, Frequency: %g Hz\n", Wave, frequency);
           fprintf(file_ptr, "Wave: %x, Frequency: %g Hz\n", Wave, frequency);
           timer = 0;
           while(timer == 0)
                        {
           Get_Accel_Values(i2c_handle,&Data);

           accX = (double)Data.Gx / ACC_FS_SENSITIVITY_3;
	   accY = (double)Data.Gy / ACC_FS_SENSITIVITY_3;
	   accZ = (double)Data.Gz / ACC_FS_SENSITIVITY_3;
	   GSumSquare = ((double) accX) * accX;
           GSumSquare += ((double) accY) * accY;
           GSumSquare += ((double) accZ) * accZ;
           Gtotal = sqrt(GSumSquare);
           Temperature = (float)  Data.Temperature / 340.0 + 36.53;
           gyroX = (double)Data.Gyrox / GYRO_FS_SENSITIVITY_0;
	   gyroY = (double)Data.Gyroy / GYRO_FS_SENSITIVITY_0;
	   gyroZ = (double)Data.Gyroz / GYRO_FS_SENSITIVITY_0;
printf("Accelerometer x=%0.2f y=%0.2f z=%0.2f All=%0.2f Temp=%+0.2f Gyro x=%0.2f y=%0.2f z=%0.2f\n",\
        accX, accY, accZ, Gtotal, Temperature, gyroX, gyroY, gyroZ);
fprintf(file_ptr, "Accelerometer x=%0.2f y=%0.2f z=%0.2f All=%0.2f Temp=%+0.2f Gyro x=%0.2f y=%0.2f z=%0.2f\n",\
        accX, accY, accZ, Gtotal, Temperature, gyroX, gyroY, gyroZ);
        fflush(stdout);
                        }
                }
      }
   AD9833_Reset();
   spi_close(spi);
   spi_free(spi);
   close(i2c_handle);
return 0;
}
