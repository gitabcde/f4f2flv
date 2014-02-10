#include "f4fpaser.h"
#include <fstream>
#include <string.h>
#include <iostream>





bool IsBigEndian()
{
  char data[]={0x12,0x34,0x56,0x78};
  uint32_t tmp_int=(*(uint32_t*)data);
  if(tmp_int==0x12345678)
    return true;
  else
    return false;
}

void CvtEndian(void* data,int size)
{
  char* input=(char*)data;
  if(size % 2 !=0)
    return;
  char tmp;
  for(int n=0;n<size/2;n++)
    {
      tmp=input[n];
      input[n]=input[size-n-1];
      input[size-n-1]=tmp;
    }

}



void CF4FPaser::CreateFlvFile(char* filename)
{
  flvfile.open(filename,std::ofstream::app|std::ofstream::binary);
  flvfile.write(FLVHEADER,sizeof(FLVHEADER));
  flvfile.flush();
  std::cout<<"create flv file"<<std::endl;
}

void CF4FPaser::WriteFlvDataFromF4file(char* f4file)
{
  std::ifstream f4f_file(f4file,std::ifstream::in|std::ifstream::binary);
  F4FTagInfo myinfo;
  GetTagInfoFromF4file(f4file,"mdat",&myinfo);
  char buffer[F4F_BUFFER_SIZE];
  int count,left;
  std::cout<<"tagname is "<<std::string(myinfo.name)<<" size is "<<myinfo.size.size_64<<std::endl;
  if(myinfo.largesize)
    {
      f4f_file.seekg(myinfo.pos_beg+16);
      std::cout<<"largesize is true,it's  "<<myinfo.size.size_64<<std::endl;
      count=(myinfo.size.size_64-16)/F4F_BUFFER_SIZE;
      left=(myinfo.size.size_64-16)%F4F_BUFFER_SIZE;
    }
  else
    {
      std::cout<<"largesize is false,it's "<<myinfo.size.size_32<<std::endl;
      f4f_file.seekg(myinfo.pos_beg+8);
      count=(myinfo.size.size_32-8)/F4F_BUFFER_SIZE;
      left=(myinfo.size.size_32-8)/F4F_BUFFER_SIZE;
    }

  for(int n=0;n<count;n++)
    {
      f4f_file.read(buffer,F4F_BUFFER_SIZE);
      flvfile.write(buffer,F4F_BUFFER_SIZE);
    }
  f4f_file.read(buffer,left);
  flvfile.write(buffer,left);
  f4f_file.close();
}

int CF4FPaser::GetTagInfoFromF4file(char* filepath,char* tagname,F4FTagInfo* taginfo)
{
  int ret=0;
  std::ifstream fs(filepath,std::ifstream::in|std::ifstream::binary);
  uint32_t src_tagname,dst_tagname;
  fs.seekg(std::ifstream::beg);
  while(fs.tellg()!=std::ifstream::end)
    {
      taginfo->pos_beg=fs.tellg();
      fs.read((char*)&taginfo->size.size_32,4);
      if(!IsBigEndian())
	CvtEndian(&taginfo->size.size_32,4);
      fs.read((char*)taginfo->name,4);
      if(taginfo->size.size_32==1)
	{
	  taginfo->largesize=true;
	  fs.read((char*)&taginfo->size.size_64,8);
	  if(!IsBigEndian())
	    CvtEndian(&taginfo->size.size_64,8);
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_64);
	}
      else
	{
	  taginfo->largesize=false;
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_32);
	}
      if((*((uint32_t*)taginfo->name))==(*((uint32_t*)tagname)))
	  return 0;
    }
  fs.close();
  flvfile.flush();
  return -1;
}
CF4FPaser::~CF4FPaser()
{
  flvfile.close();
}
int main()
{
  if(IsBigEndian())
    printf("hello bigendian\n");
  else
    printf("hello littlendian\n");
  CF4FPaser myf4f;
  F4FTagInfo myinfo;
  myf4f.CreateFlvFile("mycf4f.flv");
  myf4f.GetTagInfoFromF4file("82.flv","mdat",&myinfo);
  if(myinfo.largesize)
    {
      printf("largesize is true\n");
      printf("size is %lld\n",myinfo.size.size_64);
    }
  else
    {
      printf("largesize is false\n");
      printf("size is %d\n",myinfo.size.size_32);
    }
  myf4f.WriteFlvDataFromF4file("82.flv");
  myf4f.WriteFlvDataFromF4file("83.f4f");
  return 0;

}
