#include "FatFs_SD.h"
#include "PlaybackWav.h"
#include "AudioCodec.h"

int Total_Musics = 5;
int buttonState = 0;  
int music_num = 0;
char filename[] = "Test_Audio_48khz_16bit_stereo.wav";
char filename1[] = "Music01.wav";
char filename2[] = "Music02.wav";
char filename3[] = "Music03.wav";
char filename4[] = "Music04.wav";
char filename5[] = "Music05.wav";

#define BUFFERSIZE 512
int16_t buffer[BUFFERSIZE] = {0};

char absolute_filename[128];

FatFsSD fs;
PlaybackWav playWav;

// Callback function to feed audio codec with additional data
void writeCBFunc() {
    if(Codec.writeAvaliable()) {
        playWav.readAudioData(buffer, BUFFERSIZE);
        Codec.writeDataPage(buffer, BUFFERSIZE);
    }
}

void setup_wav(int id) {

if (id == 0) {  
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename);
    playWav.openFile(absolute_filename);
} else if (id == 1) {
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename1);
    playWav.openFile(absolute_filename);
} else if (id == 2) {
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename2);
    playWav.openFile(absolute_filename);
} else if (id == 3) {
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename3);
    playWav.openFile(absolute_filename);
} else if (id == 4) {
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename4);
    playWav.openFile(absolute_filename);
} else if (id == 5) {
    sprintf(absolute_filename, "%s%s", fs.getRootPath(), filename5);
    playWav.openFile(absolute_filename);
}
    printf("%s    \r\n", absolute_filename);
    Codec.setSampleRate(playWav.getSampleRate());
    Codec.setChannelCount(playWav.getChannelCount());
    Codec.setBitDepth(playWav.getBitDepth());
    Codec.setWriteCallback(writeCBFunc);
    Codec.begin(FALSE, TRUE);
}

void setup() {
    Serial.begin(115200);
    pinMode(PUSH_BTN, INPUT);
    fs.begin();
    setup_wav(0);
}

void loop() {
  buttonState = digitalRead(PUSH_BTN);
  if (buttonState == HIGH) {
    music_num++;
    if (music_num < (Total_Musics + 1)) {
      printf("Music0%d    \r\n", music_num);
      Codec.end();
      setup_wav(music_num);
    } else {
      music_num = 0;
    }
  }
  
  if(playWav.getPositionMillis() == playWav.getLengthMillis()) {
    music_num++;
    if (music_num < (Total_Musics + 1)) {
      printf("Music0%d    \r\n", music_num);
      Codec.end();
      setup_wav(music_num);
    } else {
      music_num = 0;
    }
  }
  delay(100);
}
