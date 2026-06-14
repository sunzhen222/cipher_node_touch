#include "mqtt_chat.h"
#include "ctype.h"
#include "cJSON.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ui_msg.h"
#include "user_memory.h"
#include "mqtt.h"

#define MQTT_SUB_EVENT_PREFIX       "+EVENT:MQTT_SUB,"
#define MQTT_CHAT_JSON_KEY_SENDER   "senderId"
#define MQTT_CHAT_JSON_KEY_NAME     "name"
#define MQTT_CHAT_JSON_KEY_COLOR    "avatarColor"
#define MQTT_CHAT_JSON_KEY_MSG      "msg"
#define MQTT_SENDER_ID_LENGTH       8


static MqttChatItem_t *g_mqttChatListHead = NULL;
static MqttChatItem_t *g_mqttNextGetNode = NULL;
static uint32_t g_mqttChatItemCount = 0;

static bool ParseMqttChatPayload(const char *payload, uint32_t payloadLen);
static bool ParseAvatarColor(const char *colorString, uint32_t *color);
static void RemoveOldestMqttChatItem(void);


void MqttChatInit(void)
{
    g_mqttChatListHead = NULL;
    g_mqttNextGetNode = NULL;
    g_mqttChatItemCount = 0;
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
    g_mqttChatItemCount = 0;
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

    if (g_mqttChatItemCount >= MQTT_CHAT_MAX_ITEMS) {
        RemoveOldestMqttChatItem();
    }

    if (g_mqttChatListHead == NULL) {
        g_mqttChatListHead = item;
        g_mqttChatItemCount++;
        return item;
    }

    MqttChatItem_t *tail = g_mqttChatListHead;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = item;
    g_mqttChatItemCount++;
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

bool ProcessMqttChatAtCommand(const char *received)
{
    const char *topicStart;
    const char *topicEnd;
    const char *lengthStart;
    char *lengthEnd;
    const char *payloadStart;
    const char *payloadEnd;
    uint32_t payloadLen;

    if (received == NULL) {
        return false;
    }

    if (strncmp(received, MQTT_SUB_EVENT_PREFIX, strlen(MQTT_SUB_EVENT_PREFIX)) != 0) {
        return false;
    }

    topicStart = received + strlen(MQTT_SUB_EVENT_PREFIX);
    topicEnd = strchr(topicStart, ',');
    if (topicEnd == NULL || topicEnd == topicStart) {
        printf("mqtt chat: invalid sub topic\n");
        return false;
    }

    lengthStart = topicEnd + 1;
    payloadLen = strtoul(lengthStart, &lengthEnd, 10);
    if (lengthEnd == lengthStart || lengthEnd[0] != ',') {
        printf("mqtt chat: invalid sub length\n");
        return false;
    }

    payloadStart = lengthEnd + 1;
    payloadEnd = payloadStart;
    while (payloadEnd[0] != '\0' && payloadEnd[0] != '\r' && payloadEnd[0] != '\n') {
        payloadEnd++;
    }

    if ((uint32_t)(payloadEnd - payloadStart) < payloadLen) {
        printf("mqtt chat: payload length short, expect=%lu actual=%lu\n",
               (unsigned long)payloadLen,
               (unsigned long)(payloadEnd - payloadStart));
        return false;
    }

    return ParseMqttChatPayload(payloadStart, payloadLen);
}


void TestMqttChat(void)
{
    ClearMqttChatItems();

    AddMqttChatItem("Me", "MQTT local list ready", true, 0x2FB35A);
    AddMqttChatItem("NodeA", "Subscribed and waiting for payload", false, 0x6155F5);
    AddMqttChatItem("NodeB", "Bootstrap message", false, 0xA84CF0);
}

static bool ParseMqttChatPayload(const char *payload, uint32_t payloadLen)
{
    bool ret = false;
    char *payloadString = NULL;
    cJSON *rootJson = NULL;
    cJSON *senderJson;
    cJSON *nameJson;
    cJSON *colorJson;
    cJSON *msgJson;
    char localSenderId[MQTT_SENDER_ID_LENGTH + 1];
    uint32_t avatarColor;
    MqttChatItem_t *item;

    if (payload == NULL || payloadLen == 0) {
        return false;
    }

    payloadString = SRAM_MALLOC(payloadLen + 1);
    memcpy(payloadString, payload, payloadLen);
    payloadString[payloadLen] = '\0';

    do {
        rootJson = cJSON_Parse(payloadString);
        if (rootJson == NULL) {
            printf("mqtt chat: json parse failed\n");
            break;
        }

        senderJson = cJSON_GetObjectItem(rootJson, MQTT_CHAT_JSON_KEY_SENDER);
        if (cJSON_IsString(senderJson)) {
            GetMqttSenderId(localSenderId, sizeof(localSenderId));
            if (strcmp(cJSON_GetStringValue(senderJson), localSenderId) == 0) {
                ret = true;
                break;
            }
        }

        nameJson = cJSON_GetObjectItem(rootJson, MQTT_CHAT_JSON_KEY_NAME);
        colorJson = cJSON_GetObjectItem(rootJson, MQTT_CHAT_JSON_KEY_COLOR);
        msgJson = cJSON_GetObjectItem(rootJson, MQTT_CHAT_JSON_KEY_MSG);
        if (!cJSON_IsString(nameJson) || !cJSON_IsString(colorJson) || !cJSON_IsString(msgJson)) {
            printf("mqtt chat: invalid json fields\n");
            break;
        }

        if (!ParseAvatarColor(cJSON_GetStringValue(colorJson), &avatarColor)) {
            printf("mqtt chat: invalid avatar color\n");
            break;
        }

        item = AddMqttChatItem(cJSON_GetStringValue(nameJson), cJSON_GetStringValue(msgJson), false, avatarColor);
        SendUiMsg(UI_MSG_CODE_MQTT_CHAT_ITEM, &item, sizeof(item));
        ret = true;
    } while (0);

    if (rootJson != NULL) {
        cJSON_Delete(rootJson);
    }
    SRAM_FREE(payloadString);

    return ret;
}

static bool ParseAvatarColor(const char *colorString, uint32_t *color)
{
    char *end;
    uint32_t value;

    if (colorString == NULL || color == NULL || strlen(colorString) != 6) {
        return false;
    }

    for (uint32_t i = 0; i < 6; i++) {
        if (!isxdigit((unsigned char)colorString[i])) {
            return false;
        }
    }

    value = strtoul(colorString, &end, 16);
    if (end == colorString || end[0] != '\0' || value > 0xFFFFFF) {
        return false;
    }

    *color = value;
    return true;
}

static void RemoveOldestMqttChatItem(void)
{
    MqttChatItem_t *oldest = g_mqttChatListHead;

    if (oldest == NULL) {
        g_mqttChatItemCount = 0;
        return;
    }

    g_mqttChatListHead = oldest->next;
    if (g_mqttNextGetNode == oldest) {
        g_mqttNextGetNode = oldest->next;
    }

    SRAM_FREE(oldest->text);
    SRAM_FREE(oldest);

    if (g_mqttChatItemCount > 0) {
        g_mqttChatItemCount--;
    }
}
