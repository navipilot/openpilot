#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <OMX_Component.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "blocking_queue.h"

class OmxEncoder {
public:
  OmxEncoder(const char* path, int width, int height, int fps, int bitrate);
  ~OmxEncoder();

  int encode_frame_rgba(const uint8_t *ptr, int in_width, int in_height, uint64_t ts);
  void encoder_open(const char* filename);
  void encoder_close();

  static OMX_ERRORTYPE event_handler(OMX_HANDLETYPE component, OMX_PTR app_data, OMX_EVENTTYPE event,
                                     OMX_U32 data1, OMX_U32 data2, OMX_PTR event_data);
  static OMX_ERRORTYPE empty_buffer_done(OMX_HANDLETYPE component, OMX_PTR app_data,
                                         OMX_BUFFERHEADERTYPE *buffer);
  static OMX_ERRORTYPE fill_buffer_done(OMX_HANDLETYPE component, OMX_PTR app_data,
                                        OMX_BUFFERHEADERTYPE *buffer);

  std::atomic<bool> is_open{false};

private:
  void wait_for_state(OMX_STATETYPE state);
  static void handle_out_buf(OmxEncoder *e, OMX_BUFFERHEADERTYPE *out_buf);

  int width, height, fps;
  std::string path;
  std::string vid_path;
  std::string lock_path;

  bool dirty = false;
  int counter = 0;

  FILE *of = nullptr;
  AVFormatContext *ofmt_ctx = nullptr;
  AVStream *out_stream = nullptr;

  std::vector<uint8_t> codec_config;
  bool wrote_codec_config = false;

  std::mutex state_lock;
  std::condition_variable state_cv;
  OMX_STATETYPE state = OMX_StateLoaded;
  OMX_HANDLETYPE handle = nullptr;

  std::vector<OMX_BUFFERHEADERTYPE *> in_buf_headers;
  std::vector<OMX_BUFFERHEADERTYPE *> out_buf_headers;

  uint64_t last_t = 0;

  std::unique_ptr<BlockingQueue<OMX_BUFFERHEADERTYPE *>> free_in;
  std::unique_ptr<BlockingQueue<OMX_BUFFERHEADERTYPE *>> done_out;
};
