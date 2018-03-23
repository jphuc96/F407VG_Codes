#include "mbed.h"
/*Network libraries*/
#include "EthernetInterface.h"
#include "MQTTNetwork.h"
#include "MQTTClient.h"
#include "MQTTmbed.h"
/*ModbusRTU libraries*/
#include "BufferedSerial.h"
#include "ModbusMaster.h"
/*TFT*/
#include <stdio.h>
#include <string.h>
#include "SPI_TFT_ILI9341.h"
#include "Arial12x12.h"
#include "Arial24x23.h"

#define PIN_MOSI        PB_5
#define PIN_MISO        PB_4
#define PIN_SCLK        PA_5
#define PIN_RS_TFT      PB_2
#define PIN_CS_TFT      PA_4
#define PIN_DC_TFT      PB_6
//TFT interface

SPI_TFT_ILI9341 TFT(PIN_MOSI, PIN_MISO, PIN_SCLK, PIN_CS_TFT, PIN_RS_TFT, PIN_DC_TFT,"TFT");

//LED interface
DigitalOut led_red(PD_12);
DigitalOut led_blue(PD_13);
DigitalOut led_green(PD_14);
DigitalOut led_yellow(PD_15);

// Network interface
EthernetInterface net;
MQTTNetwork mqtt_network(&net);
MQTT::Client<MQTTNetwork,Countdown,100,10> mqtt(mqtt_network);
const char* mqtt_broker = "m14.cloudmqtt.com";
int mqtt_port = 16409;
int rc;

Serial pc(PA_9,PA_10,115200);

/*Modbus variables*/
BufferedSerial modbus_serial(PD_5,PA_3);
ModbusMaster slave_1,slave_2;
uint16_t MB_data1;
uint16_t MB_data2;
DigitalOut TXEN(PB_0);
void preTransmission(){TXEN = 0;}
void postTransmission(){TXEN = 1;}

/*Threads declaration*/
Thread Thread_Modbus(osPriorityNormal,OS_STACK_SIZE,NULL,"Modbus");
Thread Thread_MQTT_Receive(osPriorityNormal,OS_STACK_SIZE,NULL,"MQTTReceive");

/*Functions declaration*/
void MQTT_Receive_Routine();
void Net_Init();
void MQTT_Init();
void Modbus_Init();
void TFT_Init();

void mb2mqtt(uint16_t mb_data, const char* mqtt_topic)
{
    char buf[100];
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    sprintf(buf,"%d",mb_data);
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf);
    mqtt.publish(mqtt_topic, message);
}

void mqtt_logging(const char* log_msg)
{
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)log_msg;
    message.payloadlen = strlen(log_msg);
    mqtt.publish("jphuc96/logging", message);
}

void MessageHandle(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    char topic[md.topicName.lenstring.len + 1];
    char payload[message.payloadlen + 1];
    sprintf(topic, "%.*s", md.topicName.lenstring.len, md.topicName.lenstring.data);
    sprintf(payload,"%.*s",message.payloadlen,(char*)message.payload);
    //pc.printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    pc.printf("%s: %s\r\n\n",topic,payload);

    if(strcmp(topic,"jphuc96/slave_3") == 0) led_red = atoi(payload);
    if(strcmp(topic,"jphuc96/slave_4") == 0) led_blue = atoi(payload);
    if(strcmp(topic,"jphuc96/slave_5") == 0) led_green = atoi(payload);
    if(strcmp(topic,"jphuc96/tft/text") == 0) TFT.printf("%s: %s\r\n\n",topic,payload);
    if(strcmp(topic,"jphuc96/tft/clear") == 0) {TFT.cls();TFT.locate(0,0);}

}

int main()
{   
    wait(1);
    TFT_Init();
    TFT.printf("=========================\n");
    TFT.printf("ModbusRTU to MQTT Program\n");
    
    Net_Init();
    MQTT_Init();
    Modbus_Init();

    
    /*Start threads*/
    Thread_MQTT_Receive.start(MQTT_Receive_Routine);

    while(1)
    {
        uint8_t result;
        result = slave_1.readHoldingRegisters(0, 1);

        if (result == slave_1.ku8MBSuccess)
        {
            MB_data1 = slave_1.getResponseBuffer(0);
            mb2mqtt(MB_data1,"jphuc96/slave_1");
        }
        else
        {
            mqtt_logging("Slave 1 error");
        }

        Thread::wait(10);

        result = slave_2.readHoldingRegisters(0, 1);
        if (result == slave_2.ku8MBSuccess)
        {
            MB_data2 = slave_2.getResponseBuffer(0);
            mb2mqtt(MB_data2,"jphuc96/slave_2");
        }
        else{
            mqtt_logging("Slave 2 error");
        }
        led_yellow = !led_yellow;
        Thread::wait(1000);
    }
}

void MQTT_Receive_Routine()
{
    while(true)mqtt.yield(1);
}

void Net_Init()
{
    net.connect();
    const char *ip = net.get_ip_address();
    const char *netmask = net.get_netmask();
    const char *gateway = net.get_gateway();
    TFT.printf("IP address: %s\n", ip ? ip : "None");
    TFT.printf("Netmask: %s\n", netmask ? netmask : "None");
    TFT.printf("Gateway: %s\n", gateway ? gateway : "None");
    TFT.printf("Connecting to %s:%d\r\n", mqtt_broker, mqtt_port);
    rc = mqtt_network.connect(mqtt_broker, mqtt_port);
    TFT.printf("rc from TCP connect is %d\r\n", rc);
}

void MQTT_Init()
{
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbedF407VG";
    data.username.cstring = "jrxanwut";
    data.password.cstring = "uDU6C6XHHsFF";

    rc = mqtt.connect(data);
    TFT.printf("rc from MQTT connect is %d\r\n", rc);
    rc =mqtt.subscribe("jphuc96/logging", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/tft/text", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/tft/clear", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/slave_1", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/slave_2", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/slave_3", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/slave_4", MQTT::QOS0, MessageHandle);
    rc =mqtt.subscribe("jphuc96/slave_5", MQTT::QOS0, MessageHandle);
    TFT.printf("rc from MQTT subscribe is %d\r\n", rc);
}
void Modbus_Init()
{
    modbus_serial.baud(9600);
    modbus_serial.format(8,Serial::None,1);
    slave_1.begin(1,modbus_serial);
    slave_2.begin(2,modbus_serial);
    slave_1.preTransmission(preTransmission);
    slave_1.postTransmission(postTransmission);
    slave_2.preTransmission(preTransmission);
    slave_2.postTransmission(postTransmission);
}   

void TFT_Init()
{
    TFT.claim(stdout);
    TFT.background(Black);    // set background to black
    TFT.foreground(White);    // set chars to white
    TFT.cls();                // clear the screen
    TFT.set_font((unsigned char*) Arial12x12);  // select the font
    TFT.set_orientation(3);
    TFT.locate(0,0);
}
//end