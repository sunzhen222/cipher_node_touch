#include "lora_chat.h"
#include "string.h"
#include "user_memory.h"


static ChatItem_t *g_chatListHead = NULL;
static ChatItem_t *g_nextGetNode = NULL;


void LoraChatInit(void)
{
    g_chatListHead = NULL;
    g_nextGetNode = NULL;
}


void ClearChatItems(void)
{
    ChatItem_t *node = g_chatListHead;
    while (node != NULL) {
        ChatItem_t *next = node->next;
        SRAM_FREE(node->text);
        SRAM_FREE(node);
        node = next;
    }

    g_chatListHead = NULL;
    g_nextGetNode = NULL;
}



void AddChatItem(const char *name, const char *text, uint8_t rssi)
{
    ChatItem_t *item = SRAM_MALLOC(sizeof(ChatItem_t));
    const char *safeName = (name != NULL) ? name : "";
    const char *safeText = (text != NULL) ? text : "";
    size_t textLen = strlen(safeText);

    memset(item->name, 0, sizeof(item->name));
    strncpy(item->name, safeName, sizeof(item->name) - 1);

    item->text = SRAM_MALLOC(textLen + 1);
    memcpy(item->text, safeText, textLen + 1);

    item->rssi = rssi;
    item->next = NULL;

    if (g_chatListHead == NULL) {
        g_chatListHead = item;
        return;
    }

    ChatItem_t *tail = g_chatListHead;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = item;
}


void StartGetChatItem(void)
{
    g_nextGetNode = g_chatListHead;
}


ChatItem_t *GetNextChatItem(void)
{
    ChatItem_t *ret = g_nextGetNode;
    if (g_nextGetNode != NULL) {
        g_nextGetNode = g_nextGetNode->next;
    }
    return ret;
}


void TestLoraChat(void)
{
    ClearChatItems();

    AddChatItem("Alice", "Hi, are you there?", 92);
    AddChatItem("Bob", "Yep, I am here.", 88);
    AddChatItem("Alice", "Great, test passed.", 91);
    AddChatItem("Bob", "Nice and clean.", 86);
    AddChatItem("Alice", "See you later.", 90);
    AddChatItem("Bob", "OK.", 86);
    AddChatItem("Bob", "OK.", 86);
    AddChatItem("Bob", "OK.", 86);
    AddChatItem("Bob", "OK.", 86);
}
