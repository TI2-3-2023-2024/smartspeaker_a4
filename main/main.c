#include "http_mp3.h"

void app_main()
{
    radio_player_init("https://www.mp3streams.nl/zender/radio-538/stream/4-mp3-128");
    radio_player_start();
}