#include "moyea_base_types.h"
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#define F4F_BUFFER_SIZE 4096


typedef struct tag_F4FTagInfo
{
  bool largesize;
  union
  {
    uint32_t size_32;
    uint64_t size_64;
  }size;
  char name[4];
  uint64_t pos_beg;

}F4FTagInfo;




typedef struct tag_F4MINFO
{
  bool livestream;
  uint8_t qualitycount;
  char** quality;
  char** mediaurl;
  bool include_bootstrap;
  std::string* bootstrap;
  uint32_t* first_seg;
  uint32_t* last_seg;
  uint32_t* first_seg_fragcount;
  uint32_t* last_seg_fragcount;
  uint32_t* first_frag_duration;
  uint32_t* last_frag_duration;
}F4MINFO;

char FLVHEADER[]={'F','L','V',0x01,0x05,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x00};

class CF4FPaser
{
 
 public:
  CF4FPaser();
  ~CF4FPaser();
  void SetF4m(char* f4murl);
  void GetF4MInfo(F4MINFO* pF4mInfo);
  int GetVideoSegUrl(std::string& videourl,F4MINFO* pF4mInfo,int qualitylvl);
  void WriteToStringFromUrl(std::string url,std::string& str); 
  void GetBootstrap(F4MINFO* pF4mInfo,int qualitylvl);
  void GetSegFragInfo(F4MINFO* pF4mInfo,int qualitylvl);
  void CreateFlvFile(char* filename);
  void WriteFlvDataFromF4file(char* f4file,char* flvname);
  void print();
  // private:
  int GetTagInfoFromFile(char* f4file,char* tagname,F4FTagInfo* taginfo);
  void FixTimeStampAndWriteFlvData(char* flvname,char* f4fname,F4FTagInfo* taginfo);
 private:
  uint32_t prev_video_timestamp,video_timestamp_offset,prev_audio_timestamp,audio_timestamp_offset,prev_script_timestamp,script_timestamp_offset,timestamp;
  bool have_set_zero_audio,have_set_zero_video;
  uint64_t lastpos;
  bool prev_type_video;
  CURL* hd_curl;
  std::string f4m_baseurl;
  std::string f4m_str;
 public:
  uint32_t current_frag,current_seg;


};



















