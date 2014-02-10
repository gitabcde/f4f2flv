#include "moyea_base_types.h"
#include <fstream>

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


char FLVHEADER[]={'F','L','V',0x01,0x05,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x00};

class CF4FPaser
{
 public:
  void CreateFlvFile(char* filename);
  void WriteFlvDataFromF4file(char* f4file);
  int GetTagInfoFromF4file(char* f4file,char* tagname,F4FTagInfo* taginfo);
  ~CF4FPaser();
 private:
  std::ofstream flvfile;


};

