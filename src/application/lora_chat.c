#include "lora_chat.h"
#include "string.h"
#include "user_memory.h"


static LoraChatItem_t *g_chatListHead = NULL;
static LoraChatItem_t *g_nextGetNode = NULL;
static uint32_t g_chatItemCount = 0;

static void RemoveOldestChatItem(void);


void LoraChatInit(void)
{
    g_chatListHead = NULL;
    g_nextGetNode = NULL;
    g_chatItemCount = 0;
}


void ClearChatItems(void)
{
    LoraChatItem_t *node = g_chatListHead;
    while (node != NULL) {
        LoraChatItem_t *next = node->next;
        SRAM_FREE(node->text);
        SRAM_FREE(node);
        node = next;
    }

    g_chatListHead = NULL;
    g_nextGetNode = NULL;
    g_chatItemCount = 0;
}


LoraChatItem_t *AddChatItem(const char *name, const char *text, int8_t rssi, bool self, uint32_t headColor)
{
    LoraChatItem_t *item = SRAM_MALLOC(sizeof(LoraChatItem_t));
    const char *safeName = (name != NULL) ? name : "";
    const char *safeText = (text != NULL) ? text : "";
    size_t textLen = strlen(safeText);

    memset(item->name, 0, sizeof(item->name));
    strncpy(item->name, safeName, sizeof(item->name) - 1);

    item->text = SRAM_MALLOC(textLen + 1);
    memcpy(item->text, safeText, textLen + 1);

    item->rssi = rssi;
    item->self = self;
    item->headColor = headColor;
    item->next = NULL;

    if (g_chatItemCount >= LORA_CHAT_MAX_ITEMS) {
        RemoveOldestChatItem();
    }

    if (g_chatListHead == NULL) {
        g_chatListHead = item;
        g_chatItemCount++;
        return item;
    }

    LoraChatItem_t *tail = g_chatListHead;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = item;
    g_chatItemCount++;
    return item;
}


void StartGetChatItem(void)
{
    g_nextGetNode = g_chatListHead;
}


LoraChatItem_t *GetNextChatItem(void)
{
    LoraChatItem_t *ret = g_nextGetNode;
    if (g_nextGetNode != NULL) {
        g_nextGetNode = g_nextGetNode->next;
    }
    return ret;
}


void TestLoraChat(void)
{
    ClearChatItems();

    AddChatItem("Alice", "Hi, are you there?\nhaha", -72, true, 0xFF2C2C);
    AddChatItem("Bob", "Yep, I am here.1222222223333333333311111111111111111111", -75, false, 0x6155F5);
    AddChatItem("Alice", "Great, test passed.", -71, true, 0xFF2C2C);
    AddChatItem("Bob", "Nice and clean.", -78, false, 0x6155F5);
    AddChatItem("Alice", "See you later.", -73, true, 0xFF2C2C);
    AddChatItem("Bob", "OK.", -80, false, 0x6155F5);
    AddChatItem("Bob", "OK.", -80, false, 0x6155F5);
    AddChatItem("Bob", "OK.", -80, false, 0x6155F5);
    AddChatItem("Bob", "OK.", -80, false, 0x6155F5);
}

static void RemoveOldestChatItem(void)
{
    LoraChatItem_t *oldest = g_chatListHead;

    if (oldest == NULL) {
        g_chatItemCount = 0;
        return;
    }

    g_chatListHead = oldest->next;
    if (g_nextGetNode == oldest) {
        g_nextGetNode = oldest->next;
    }

    SRAM_FREE(oldest->text);
    SRAM_FREE(oldest);

    if (g_chatItemCount > 0) {
        g_chatItemCount--;
    }
}
