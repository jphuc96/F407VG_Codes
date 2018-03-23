#include "mbed.h"
#include "EthernetInterface.h"
#include "MQTTNetwork.h"
#include "MQTTClient.h"
#include "MQTTmbed.h"


// Network interface
EthernetInterface net;

const char* mqtt_broker = "broker.hivemq.com";
int mqtt_port = 1883;
char* mqtt_topic = "jphuc96/mbed";

Serial pc(PA_9,PA_10);
DigitalOut led_red(PD_12);
DigitalOut led_blue(PD_13);
// Socket demo
int rc;
int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    pc.printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    pc.printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

int main() {
    pc.baud(115200);
    pc.printf("=========================\n");
    pc.printf("DISCO_F407VG MQTT Program\n");
    // Bring up the ethernet interface
    net.connect();
    
    // Show the network address
    const char *ip = net.get_ip_address();
    const char *netmask = net.get_netmask();
    const char *gateway = net.get_gateway();
    pc.printf("IP address: %s\n", ip ? ip : "None");
    pc.printf("Netmask: %s\n", netmask ? netmask : "None");
    pc.printf("Gateway: %s\n", gateway ? gateway : "None");
    
    MQTTNetwork mqtt_network(&net);
    MQTT::Client<MQTTNetwork,Countdown> mqtt(mqtt_network);
    pc.printf("Connecting to %s:%d\r\n", mqtt_broker, mqtt_port);
    rc = mqtt_network.connect(mqtt_broker, mqtt_port);
    pc.printf("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbedF407VG";

    rc = mqtt.connect(data);
    pc.printf("rc from MQTT connect is %d\r\n", rc);

    rc = mqtt.subscribe(mqtt_topic, MQTT::QOS2, messageArrived);
    pc.printf("rc from MQTT subscribe is %d\r\n", rc);
    pc.printf("Subscribed to topic \"%s\" \r\n",mqtt_topic);

    MQTT::Message message;

    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", 0.6);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = mqtt.publish(mqtt_topic, message);
    while (arrivedcount < 1)
        mqtt.yield(100);
    pc.printf("Published data sucessfully\r\n");
    
    // Bring down the ethernet interface
    net.disconnect();
    pc.printf("Done\n");
}
