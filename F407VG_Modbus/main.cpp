#include "mbed.h"
#include "BufferedSerial.h"
#include "ModbusMaster.h"
DigitalOut led2(PD_13);
DigitalOut led3(PD_14);

Thread thread_1;
Thread thread_2;

void Modbus_Thread();
void Led_Thread();

Serial pc(PA_9,PA_10,115200);
BufferedSerial modbus_serial(PB_10,PD_9);
ModbusMaster node;
int main(){
    pc.printf("===================\r\n");
    pc.printf("Modbus RTU Testcase\r\n");

    thread_1.start(Modbus_Thread);
    thread_2.start(Led_Thread);
}

void Led_Thread(){
    while(1){
        led3 = 1;
        Thread::wait(500);
        led3 = 0;
        Thread::wait(500);
    }
}

void Modbus_Thread()
{   
    modbus_serial.baud(9600);
    modbus_serial.format(8,Serial::None,1);
    node.begin(1, modbus_serial);
    while(1){
        
        static uint32_t i;
        uint8_t j, result;
        uint16_t data[1];
        
        i++;
        
        // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
        node.setTransmitBuffer(0, lowWord(i));
        
        // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
        node.setTransmitBuffer(1, highWord(i));
        
        // slave: write TX buffer to (2) 16-bit registers starting at register 0
        //result = node.writeMultipleRegisters(0, 2);
        
        // slave: read (6) 16-bit registers starting at register 2 to RX buffer
        result = node.readHoldingRegisters(2, sizeof(data));
        
        // do something with data if read is successful
        if (result == node.ku8MBSuccess)
        {
            for (j = 0; j < sizeof(data); j++)
            {
            data[j] = node.getResponseBuffer(j);
            pc.printf("Reg %d: %d\r\n",j,data[j]);
            }
            pc.printf("\r\n");
        }
        Thread::wait(100);

    }
    
}
