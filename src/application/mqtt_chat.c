#include "mqtt_chat.h"
#include "string.h"
#include "user_memory.h"


static MqttChatItem_t *g_mqttChatListHead = NULL;
static MqttChatItem_t *g_mqttNextGetNode = NULL;


void MqttChatInit(void)
{
    g_mqttChatListHead = NULL;
    g_mqttNextGetNode = NULL;
}


void ClearMqttChatItems(void)
{
    MqttChatItem_t *node = g_mqttChatListHead;
    while (node != NULL) {
        MqttChatItem_t *next = node->next;
        SRAM_FREE(node->text);
        SRAM_FREE(node);
        node = next;
    }

    g_mqttChatListHead = NULL;
    g_mqttNextGetNode = NULL;
}


MqttChatItem_t *AddMqttChatItem(const char *name,
                                const char *text,
                                bool self,
                                uint32_t headColor)
{
    MqttChatItem_t *item = SRAM_MALLOC(sizeof(MqttChatItem_t));
    const char *safeName = (name != NULL) ? name : "";
    const char *safeText = (text != NULL) ? text : "";
    size_t textLen = strlen(safeText);

    memset(item->name, 0, sizeof(item->name));
    strncpy(item->name, safeName, sizeof(item->name) - 1);

    item->text = SRAM_MALLOC(textLen + 1);
    memcpy(item->text, safeText, textLen + 1);

    item->self = self;
    item->headColor = headColor;
    item->next = NULL;

    if (g_mqttChatListHead == NULL) {
        g_mqttChatListHead = item;
        return item;
    }

    MqttChatItem_t *tail = g_mqttChatListHead;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = item;
    return item;
}


void StartGetMqttChatItem(void)
{
    g_mqttNextGetNode = g_mqttChatListHead;
}


MqttChatItem_t *GetNextMqttChatItem(void)
{
    MqttChatItem_t *ret = g_mqttNextGetNode;
    if (g_mqttNextGetNode != NULL) {
        g_mqttNextGetNode = g_mqttNextGetNode->next;
    }
    return ret;
}


void TestMqttChat(void)
{
    ClearMqttChatItems();

    AddMqttChatItem("Me", "MQTT local list ready", true, 0x2FB35A);
    AddMqttChatItem("NodeA", "Subscribed and waiting for payload", false, 0x6155F5);
    AddMqttChatItem("NodeB", "Bootstrap message", false, 0xA84CF0);
}
