#ifndef wificlientbearssl_h
#define < WiFiClientSecure.h >
#endif
#ifndef TICKER_H
#define < Ticker.h >
#endif

class TelegramBot
{
private:
    const char *cmd_start = "/start";
    const char *cmd_restart = "/restart";
    const char *cmd_chatid = "/chatid";
    const char *cmd_info_device = "/info";
    const char *cmd_info_voucher = "/voucher";
    const char *cmd_info_sta = "/station";
    const char *cmd_info_ap = "/access_point";
    const char *cmd_set_voucher = "/device_voucher";          // username_password_type
    const char *cmd_set_ap_password = "/device_appassword";   // wifi password
    const char *cmd_set_landing_page = "/device_landingpage"; // url landing page
    const char *cmd_set_thumbprint = "/device_thumbprint";    // thumbprint wifi.id

    const char *host = "api.telegram.org";
    WiFiClientSecure client;
    void restartSystem();
    const char *findArr(String s, byte index);
    Ticker ticker;
    String res;

public:
    bool _debug;
    char *token = BOT_TOKEN_ADMIN;
    char *chatid_admin = BOT_CHATID_ADMIN;
    int chatID = 0;
    String chatText = "";
    int chatUpdateID = 0;
    int chatMessageID = 0;
    void clear();
    void TELEGRAM_HANDLE_MESSAGE();
    TelegramBot(WiFiClientSecure &client);
    bool TELEGRAM_UPDATES(int offset);
    bool TELEGRAM_SEND_MESSAGE(int _chatId, String _text, int _reply_to_message_id, const char *parse_mode);
    int CHECK_VOUCHER_VALIDITY();
    bool TELEGRAM_CONNECT();
    void Disconnect();
};

int TelegramBot::CHECK_VOUCHER_VALIDITY()
{
    String sDate = String(val_note);
    String __year = sDate.substring(0, sDate.indexOf("-"));
    PRINTV("year" + __year);
    sDate = sDate.substring(sDate.indexOf("-") + 1);
    String __month = sDate.substring(0, sDate.indexOf("-"));
    PRINTV("month" + __month);
    sDate = sDate.substring(sDate.indexOf("-") + 1);
    String __date = sDate;
    PRINTV("date " + __date);

    time_t nextMakeTime;
    tmElements_t tm;
    tm.Second = 0;
    tm.Hour = 0;
    tm.Minute = 0;
    tm.Day = __date.toInt();
    tm.Month = __month.toInt();
    tm.Year = __year.toInt() - 1970; // offset from 1970;
    nextMakeTime = makeTime(tm);     // convert time elements into time_t
    int elapse = nextMakeTime - timeClient.getEpochTime();
    int _day = day(nextMakeTime - timeClient.getEpochTime());
    PRINTV("Current : " + myClass.getFormattedDateTime(timeClient.getEpochTime()));
    PRINTV("Expired : " + myClass.getFormattedDateTime(nextMakeTime));
    PRINTV("Elapsed : " + String(day(nextMakeTime - timeClient.getEpochTime())));

    return _day;
}

void TelegramBot::Disconnect()
{
    if (client.connected())
    {
        client.flush();
        client.stop();
        client.stopAll();
    }
}

const char *TelegramBot::findArr(String s, byte index)
{
    byte i = 0;
    String arr[10];
    while (s.indexOf("_") >= 0)
    {
        s = s.substring(s.indexOf("_") + 1);
        // PRINTV(s);
        // PRINTV(s.substring(0, s.indexOf("_")));
        arr[i] = s.substring(0, s.indexOf("_"));
        PRINTV("arr[" + String(i) + "] = " + arr[i]);
        i++;
    }
    return arr[index].c_str();
}

void TelegramBot::clear()
{
    chatID = 0;
    chatText = "";
    chatUpdateID = 0;
    chatMessageID = 0;
}

void TelegramBot::restartSystem()
{
    ESP.restart();
}

TelegramBot::TelegramBot(WiFiClientSecure &client)
{
    client = client;
}

bool TelegramBot::TELEGRAM_CONNECT()
{
    if (!client.connected())
    {
        PRINTV(("CONNECTING API TELEGRAM.. " + String(host)));
        client.setFingerprint("");
        client.setInsecure();
        if (client.connect(host, 443))
        {
            PRINTV(F("CONNECTING API TELEGRAM.. CONNECTED"));
        }
        else
        {
            PRINTV(F("CONNECTING API TELEGRAM.. FAILED"));
        }
    }
    return client.connected();
}

bool TelegramBot::TELEGRAM_UPDATES(int offset)
{
    PRINTV(F("TELEGRAM_UPDATES"));

    if (!TELEGRAM_CONNECT())
        return 0;

    // Serial.printf("offset is %d\n", offset);
    PRINTD(F("getUpdates... "));
    PRINTV("OFFSET " + String(offset));
    String prm = "GET /bot" + String(token) +
                 "/getUpdates?offset=" + String(offset) +
                 "&limit=1" + " HTTP/1.1\n" +
                 ("Host: " + String(host) + "\n") +
                 "Cache-Control: no-cache\n"
                 "Postman-Token: 1cfddc59-fd42-29a7-b8eb-1fba3a5b4558\n"
                 "\n";
    // PRINTV("HTTP " + prm);
    if (client.availableForWrite())
        client.write(prm.c_str());

    PRINTV(F("WAITING RESPONSE"));
    while (!client.available())
    {
        // PRINTV(".");
        // delay(1000);

        if (!internet)
            return 0;
    }

    PRINTV(F("GETING RESPONSE"));
    res = "";
    while (client.available())
    {
        if (!internet)
            return 0;
        char buf = char(client.read());
        res += buf;
    }
    // Disconnect();
    // PRINTV("RESPONSE\n " + res);
    clear();
    bool success = strcmp(myClass.getJsonFieldFromClientResponse(res, "\"ok\":", ",").c_str(), "true") == 0;
    chatID = atoi(myClass.getJsonFieldFromClientResponse(res, "\"id\":", ",").c_str());
    chatText = myClass.getJsonFieldFromClientResponse(res, "\"text\":\"", "\"");
    chatUpdateID = atoi(myClass.getJsonFieldFromClientResponse(res, "\"update_id\":", ",").c_str());
    chatMessageID = atoi(myClass.getJsonFieldFromClientResponse(res, "\"message_id\":", ",").c_str());
    myClass.getJsonFieldFromClientResponse(res, "\"error_code\":", ",");
    myClass.getJsonFieldFromClientResponse(res, "\"description\":", ",");

    if (success)
        PRINTV(F("TELEGRAM UPDATES OK"));
    else
        PRINTV(F("TELEGRAM UPDATES FAILED"));

    return success;
}

bool TelegramBot::TELEGRAM_SEND_MESSAGE(int _chatId = 0, String _text = "", int _reply_to_message_id = 0, const char *parse_mode = "Markdown")
{
    PRINTV(F("TELEGRAM_SEND_MESSAGE"));

    if (_chatId == 0)
    {
        PRINTV("Sending failed, ChatID is " + String(_chatId));
        return 0;
    }

    if (!TELEGRAM_CONNECT())
        return 0;

    String preText = String(ESP.getChipId(), HEX);
    preText.toUpperCase();
    String deviceText = "*" + preText + "*\n";
    _text = deviceText + _text;
    String datas = "{\"chat_id\":\"" + String(_chatId) +
                   "\",\"text\":\"" + _text +
                   "\",\"reply_to_message_id\":\"" + String(_reply_to_message_id) + "\"}";

    String prm =
        ("POST /bot" + String(token) + "/sendMessage?parse_mode=" + parse_mode + " HTTP/1.1\n") +
        ("Host: " + String(host) + "\n") +
        "Content-Type: application/json\n"
        "Cache-Control: no-cache\n"
        "Postman-Token: 207b5d5f-6a78-6ff4-f527-247dfd9f7867\n" +
        ("Content-Length:" + String(datas.length()) + "\n") +
        "\n" +
        (datas + "\n");

    if (client.availableForWrite())
        client.write(prm.c_str());

    // PRINTV("HTTP " + prm);

    PRINTV(F("WAITING RESPONSE"));
    while (!client.available())
    {
        // PRINTV(F("."));
        // delay(1000);

        if (!internet)
            return 0;
    }

    PRINTV(F("GETING RESPONSE"));
    res = "";
    while (client.available())
    {
        if (!internet)
            return 0;
        char buf = char(client.read());
        res += buf;
    }
    // Disconnect();

    // PRINTV("RESPONSE\n " + res);
    bool success = strcmp(myClass.getJsonFieldFromClientResponse(res, "\"ok\":", ",").c_str(), "true") == 0;
    myClass.getJsonFieldFromClientResponse(res, "\"error_code\":", ",");
    myClass.getJsonFieldFromClientResponse(res, "\"description\":", ",");

    if (success)
        PRINTV(F("TELEGRAM SEND OK"));
    else
        PRINTV(F("TELEGRAM SEND FAILED"));

    return success;
}
void TelegramBot::TELEGRAM_HANDLE_MESSAGE()
{
    PRINTV(F("TELEGRAM HANDLE MESSAGE"));

    bool isAdmin = chatID == atoi(BOT_CHATID_ADMIN);
    bool isUser = chatID == atoi(val_chat_id);
    bool isUserRegistered = isAdmin || isUser;

    // COMMON
    if (strcmp(chatText.c_str(), "/start") == 0)
    {
        chatMessageID = 0; // jangan reply
        chatText = "\n*Welcome to Bot DAL WIFI ID*"
                   "\nCommand available" +
                   ("\n[" + String(cmd_start) + "]") +
                   ("\n[" + String(cmd_chatid) + "]") +
                   (isUserRegistered ? "\n[" + String(cmd_info_device) + "]" : "") +
                   (isUserRegistered ? "\n[" + String(cmd_info_voucher) + "]" : "") +
                   (isUserRegistered ? "\n[" + String(cmd_info_sta) + "]" : "") +
                   (isUserRegistered ? "\n[" + String(cmd_info_ap) + "]" : "") +
                   (isUserRegistered ? "\n[" + String(cmd_restart) + "]" : "") +
                   (!isUserRegistered ? "\n_please register *ChatID* to your *Device*_" : "");
    }

    if (strcmp(chatText.c_str(), cmd_chatid) == 0)
    {
        chatText = "Your ChatID is " + String(chatID);
    }

    // ADMIN & USER
    if (isAdmin || isUser)
    {
        if (strcmp(chatText.c_str(), cmd_info_voucher) == 0)
        {
            chatText = "*Voucher Information*" +
                       ("\nUsername " + String(val_username)) +
                       ("\nPassword -not show-") +
                       ("\nVoucher type " + String(val_voucher_type)) +
                       ("\nVoucher expired at " + String(val_note)) +
                       ("\nVoucher expire in " + String(CHECK_VOUCHER_VALIDITY()) + " day");
        }
        if (strcmp(chatText.c_str(), cmd_info_sta) == 0)
        {
            chatText = "*Station Information*"
                       "\nSSID " +
                       WiFi.SSID() +
                       "\nMAC " + WiFi.macAddress() +
                       "\nIP Address " + WiFi.localIP().toString() +
                       "\nGateway " + WiFi.gatewayIP().toString() +
                       "\nSignal " + String(WiFi.RSSI()) + " dB";
        }
        if (strcmp(chatText.c_str(), cmd_info_ap) == 0)
        {
            chatText = "*Access Point Information*"
                       "\nSSID " +
                       WiFi.softAPSSID() +
                       "\nMAC " + WiFi.softAPmacAddress() +
                       "\nIP " + WiFi.softAPIP().toString();
        }
        if (strcmp(chatText.c_str(), cmd_info_device) == 0)
        {
            chatText = "*Device Information*"
                       "\nUptime " +
                       String(myClass.getFormattedUpTime()) +
                       "\nChip ID " +
                       String(ESP.getChipId(), HEX) +
                       "\nFlash chip ID " + String(ESP.getFlashChipId(), HEX) +
                       "\nFree heap " + String(ESP.getFreeHeap()) +
                       "\nReset reason " + ESP.getResetReason();
        }
        if (strcmp(chatText.c_str(), cmd_restart) == 0)
        {
            // chatText = F("Restarting system..");
            chatText = F("please restart device manualy");
            // ESP.restart();
            // ticker.once(10, restartSystem);
        }
    }
    else
    {
    }

    String s = chatText;
    String arr[10];
    // PRINTV("s.indexOf(cmd_set_voucher) | " + String(s.indexOf(cmd_set_voucher)));

    if (s.indexOf(cmd_set_voucher) >= 0)
    {
        sprintf(val_username, findArr(s, 1));
        sprintf(val_password, findArr(s, 2));
        sprintf(val_voucher_type, findArr(s, 3));
        UpdateConfig();
        chatText = F("Voucher has been setup");
    }
    else if (s.indexOf(cmd_set_ap_password) >= 0)
    {
        sprintf(val_pass_wifi, findArr(s, 1));
        UpdateConfig();
        chatText = F("pass ap wifi has been setup");
    }
    else if (s.indexOf(cmd_set_landing_page) >= 0)
    {
        sprintf(val_loginurl, findArr(s, 1));
        UpdateConfig();
        chatText = F("landing page has been setup");
    }
    else if (s.indexOf(cmd_set_thumbprint) >= 0)
    {
        sprintf(val_thumbprint, findArr(s, 1));
        UpdateConfig();
        chatText = F("thumbprint wifi.id has been setup");
    }
}
