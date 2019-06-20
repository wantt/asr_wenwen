// Copyright 2016 Mobvoi Inc. All Rights Reserved.

#include <assert.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>
#include<cstring>
#include "speech_sdk.h"


//   http://ai.chumenwenwen.com/pages/document/get-started
static const std::string kAppKey = "2844F2D8DF07F35DB998BDDA9D130708";
//volatile mobvoi_recognizer_type type = MOBVOI_RECOGNIZER_ONLINE_ONEBOX;
volatile mobvoi_recognizer_type type = MOBVOI_RECOGNIZER_OFFLINE ;

void show_usage() {
  std::cout << "Usage: recognizer_with_file ASR/SEMANTIC/ONEBOX audio_file_path"
            << std::endl;
}

void on_remote_silence_detected() {
	mobvoi_recognizer_start(type);
  std::cout << "--------> dummy on_remote_silence_detected" << std::endl;
}

void on_partial_transcription(const char* result) {
//  std::cout << "--------> dummy on_partial_transcription: " << result
//            << std::endl;
}

void on_final_transcription(const char* result) {
  std::cout << "--------> dummy on_final_transcription: " << result
            << std::endl;
}

void on_result(const char* result) {
  std::cout << "--------> dummy on_result: " << result << std::endl;
}

void on_error(int error_code) {
  std::cout << "--------> dummy on_error with error code: " << error_code
            << std::endl;
}

void on_local_silence_detected() {
  std::cout << "--------> dummy on_local_silence_detected" << std::endl;
  mobvoi_recognizer_stop();
}

void on_volume(float spl) {
  // The sound press level is here, do whatever you want
  // std::cout << "--------> dummy on_speech_spl_generated: spl = "
  //           << std::fixed << std::setprecision(6) << spl
  //           << std::endl;
}

void* send_audio_thread(void* arg) {
  std::ifstream& file = *(std::ifstream*) arg;
  const int kBatchSize = 320;
  int pos = 0;
  file.seekg(0, file.end);
  int length = file.tellg() / 2;
  file.seekg(0, file.beg);
  short in_shorts[kBatchSize];

  usleep(200 * 1000);
  if (file.is_open()) {
    while (pos < length) {
      int stride =
          (pos + kBatchSize < length) ? kBatchSize : (length - pos);
      file.read((char*) &in_shorts, stride * 2);
      mobvoi_send_speech_frame((char*) &in_shorts, stride * 2);
      pos += stride;
  		usleep(2 * 1000);
    }
  } else {
    std::cout << "File could not be opened!" << std::endl;
  }
  file.close();
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    show_usage();
    return 1;
  }

  std::string online_type(argv[1]);
  if (online_type == "ASR") {
    type = MOBVOI_RECOGNIZER_ONLINE_ASR;
  } else if (online_type == "SEMANTIC") {
    type = MOBVOI_RECOGNIZER_ONLINE_SEMANTIC;
  } else {
    //type = MOBVOI_RECOGNIZER_ONLINE_ONEBOX;
    type = MOBVOI_RECOGNIZER_ONLINE_ASR;
  }

  std::ifstream test_file;
  test_file.open(argv[1]);
  // Read the audio file specified by the command line argument
  if (!test_file.is_open()) {
    std::cout << "Failed to open file " << argv[1] << std::endl;
    return 2;
  }

  // SDK initilalization, including callback functions

  mobvoi_sdk_init(kAppKey.c_str());

  mobvoi_recognizer_handler_vtable* speech_handlers =
      new mobvoi_recognizer_handler_vtable;
  assert(speech_handlers != NULL);
  memset(speech_handlers, 0, sizeof(mobvoi_recognizer_handler_vtable));
  speech_handlers->on_error = &on_error;
  speech_handlers->on_partial_transcription = &on_partial_transcription;
  speech_handlers->on_final_transcription = &on_final_transcription;
  speech_handlers->on_local_silence_detected = &on_local_silence_detected;
  speech_handlers->on_remote_silence_detected = &on_remote_silence_detected;
  speech_handlers->on_result = &on_result;
  speech_handlers->on_volume = &on_volume;
  mobvoi_recognizer_set_handler(speech_handlers);

  mobvoi_recognizer_start(type);
	send_audio_thread(&test_file);
  // SDK clean up
  std::cout << "start sdk cleanup" << std::endl;
  mobvoi_sdk_cleanup();
  std::cout << "end sdk cleanup" << std::endl;
  delete speech_handlers;
  std::cout << "end dummy sender" << std::endl;

  return 0;
}
