#include "mbed.h"
#include "stdint.h" //This allow the use of integers of a known width
#define LM75_REG_TEMP (0x00) // Temperature Register
#define LM75_REG_CONF (0x01) // Configuration Register
#define LM75_ADDR     (0x90) // LM75 address

#define LM75_REG_TOS (0x03) // TOS Register
#define LM75_REG_THYST (0x02) // THYST Register



I2C i2c(I2C_SDA, I2C_SCL);

DigitalOut myled(LED1);
DigitalOut blue(LED2);
DigitalOut red(LED3);

InterruptIn lm75_int(D7); // Make sure you have the OS line connected to D7

Serial pc(SERIAL_TX, SERIAL_RX);

float last_min[60] = {0};
int i = 0;
int i_f;
volatile bool alarm_triggered = false;
bool first_iteration = true;


int16_t i16; // This variable needs to be 16 bits wide for the TOS and THYST conversion to work - can you see why?


void alarm()
{
        alarm_triggered = true;
}

int main()
{
        char data_write[3];
        char data_read[3];

        /* Configure the Temperature sensor device STLM75:
           - Thermostat mode Interrupt
           - Fault tolerance: 0
           - Interrupt mode means that the line will trigger when you exceed TOS and stay triggered until a register is read - see data sheet
        */
        data_write[0] = LM75_REG_CONF;
        data_write[1] = 0x02;
        int status = i2c.write(LM75_ADDR, data_write, 2, 0);
        if (status != 0)
        { // Error
                while (1)
                {
                        myled = !myled;
                        wait(0.2);
                }
        }

        float tos=28; // TOS temperature
        float thyst= -55; // THYST temperature set to the lowest value that the sensor can detect

        // This section of code sets the TOS register
        data_write[0]=LM75_REG_TOS;
        i16 = (int16_t)(tos*256) & 0xFF80;
        data_write[1]=(i16 >> 8) & 0xff;
        data_write[2]=i16 & 0xff;
        i2c.write(LM75_ADDR, data_write, 3, 0);

        //This section of codes set the THYST register
        data_write[0]=LM75_REG_THYST;
        i16 = (int16_t)(thyst*256) & 0xFF80;
        data_write[1]=(i16 >> 8) & 0xff;
        data_write[2]=i16 & 0xff;
        i2c.write(LM75_ADDR, data_write, 3, 0);

        // This line attaches the interrupt.
        // The interrupt line is active low so we trigger on a falling edge
        lm75_int.fall(&alarm);



        while (alarm_triggered == false)
        {
                // Read temperature register
                data_write[0] = LM75_REG_TEMP;
                i2c.write(LM75_ADDR, data_write, 1, 1); // no stop
                i2c.read(LM75_ADDR, data_read, 2, 0);

                // Calculate temperature value in Celcius
                int16_t i16 = (data_read[0] << 8) | data_read[1];
                // Read data as twos complement integer so sign is correct
                float temp = i16 / 256.0;

                // Display result
                pc.printf("Temp=%.3f  INT=%d  alarm=%d\r\n", temp, lm75_int.read(), alarm_triggered);
                myled = !myled;

                //store value in array
                last_min[i] = temp;
                i_f = i; // store most recent value of i in i_f
                i = (i + 1) % 60;
                wait(1.0);
        }

        while(1)
        {
            if(first_iteration == true) { 
                red = 1; 
                for (int k = 0; k < 60; k++) {
                    int idx = (i_f + 1 + k) % 60;
                    pc.printf("Temperature = %.3f\r\n", last_min[idx]);
                wait(0.02);
}
                first_iteration = false; 
            }
                blue = !blue;
                wait(1.0);
        }    

        }
