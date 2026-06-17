#include "sdGlobals.h"

AudioFileSourceSD* source = nullptr;
AudioGeneratorMP3* mp3Decoder = nullptr;
AudioGeneratorWAV* wavDecoder = nullptr;

DirContents dirContents = {{}, 0, {}};
SongInfo songInfo = {"", "", "", 0, 0, false, 0, 0};

bool userStopped = false;
TaskHandle_t audioTaskHandle = NULL;
volatile bool stopAudio = false;
volatile bool sdCardRemoved = false;
String currentDir = "/";
