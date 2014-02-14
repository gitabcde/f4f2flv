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
  int pos_beg;

}F4FTagInfo;




typedef struct tag_F4MINFO
{
  bool livestream;
  uint8_t qualitycount;
  char** quality;
  char** mediaurl;
  bool include_bootstrap;
  char** bootstrap;
  uint32_t Seg_Upper;
  uint32_t Seg_Lowwer;
  uint32_t Frag_Upper;
  uint32_t Frag_Lowwer;
  
}F4MINFO

char FLVHEADER[]={'F','L','V',0x01,0x05,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x00};

class CF4FPaser
{
 
 public:
  CF4FPaser();
  ~CF4FPaser();
  void DownLoadFile(char* fileurl);
  void GetF4MInfoFromFile(char* f4mfile,F4MINFO* pF4mInfo);
  void CreateFlvFile(char* filename);
  void WriteFlvDataFromF4file(char* f4file,char* flvname);
 private:
  int GetTagInfoFromF4file(char* f4file,char* tagname,F4FTagInfo* taginfo);
  void AjustFlvTimeStamp(char* flvname);
  bool IsLiveVideo(char* f4mfile);
  //  void GetVideoSegFragRange(
 private:
  uint32_t prev_video_timestamp,video_timestamp_offset,prev_audio_timestamp,audio_timestamp_offset,prev_script_timestamp,script_timestamp_offset,timestamp;
  uint64_t lastpos;
  std::vector<std::string>  downloader_f4files;
  std::string f4m_baseurl;
};

