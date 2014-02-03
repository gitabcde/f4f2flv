
#include <iostream>
#include <fstream>
#include "CXPrimitiveType.h"
#include <string.h>
char FLVHEADER[]={'F','L','V',0x01,0x05,0x00,0x00,0x00,0x09};
struct mybitset
{
  char mybit1:1;
  char mybit2:2;
  char mybit3_4:2;
  char mybit5_8:3;

};
void myalignfun()
{
  int input;
  std::cout<<"input the num aligned to 8"<<std::endl;
  std::cin>>input;
  std::cout<<"the num aligned to 8 is "<<input+((8-(input%8))*((input%8)!=0))<<std::endl;

}
void mybitmember()
{
  mybitset mybitsettest;
  mybitsettest.mybit1=1;
  mybitsettest.mybit2=0;
  mybitsettest.mybit3_4=2;
  mybitsettest.mybit5_8=5;
  char* myst=(char*)&mybitsettest;
  std::cout<<std::hex<<(int)*myst<<std::endl;
  std::cout<<"sizeof mybitset is "<<sizeof(mybitset)<<std::endl;

}

bool myisbigendian()
{
  unsigned short  mytestshort=0x1234;
  char* mytestchar=(char*)&mytestshort;
  if(*mytestchar==0x12)
    return true;
  else
    return false;
}

void mywriteheader()
{
  std::fstream output;
  output.open("./output",std::fstream::app|std::fstream::out|std::fstream::binary);
  char myout[]={0x46,0x4c,0x56};
  output.write(myout,3);
  output.close();
}

void myoutput(double input)
{
  std::cout<<input<<std::endl;
}

void mystringsearch()
{
  char mytest[]="hello world,this is a string with end flag ";
  mytest[8]='\0';
  std::string mystr;
  mystr.append(mytest,25);
  printf("mytest is %s\n",mytest);
  std::cout<<"his's pos is "<<mystr.find("his")<<std::endl;
  std::cout<<"mystr's size is "<<mystr.size()<<std::endl;
  std::cout<<"strstr(mytest,\"his\") is "<<(CX_UINT64)(strstr(mytest,"lo")-mytest)<<std::endl;
}

void myfileread()
{
  std::fstream myfs;
  myfs.open("./test.cpp",std::fstream::in);
  char buff[100];
  memset(buff,0,100);
  myfs.read(buff,100);
  std::cout<<"tellg is "<<myfs.tellg()<<std::endl;
  myfs.close();
}

typedef struct tag_F4F_TagInfo
{
  bool largesize;
  union
  {
    CX_UINT32 size_32;
    CX_UINT64 size_64;
  }size;
  char name[4];
  int pos_beg;
}F4F_TagInfo;

void mycvtbigendian(void* input,int size)
{
  char* ptr=(char*)input;
  if(myisbigendian())
    return;
  char tmp;
  for(int n=0;n<size/2;n++)
    {
      tmp=ptr[n];
      ptr[n]=ptr[size-1-n];
      ptr[size-1-n]=tmp;
    }

}



void mywriteflvheader(char* flvpath)
{
  std::ofstream flv_file;
  flv_file.open(flvpath,std::ofstream::app|std::ifstream::binary);
  flv_file.write(FLVHEADER,sizeof(FLVHEADER));
  flv_file.close();
}

int prev_size=0;
int myFindF4FTagByName(char* tagname,F4F_TagInfo* taginfo,char* filepath)
{
  int ret=0;
  std::fstream fs(filepath,std::fstream::in|std::fstream::binary);
  CX_UINT32 src_tagname,dst_tagname;
  fs.seekg(std::fstream::beg);
  while(fs.tellg()!=std::fstream::end)
    {
      taginfo->pos_beg=fs.tellg();
      fs.read((char*)&taginfo->size.size_32,4);
      mycvtbigendian(&taginfo->size.size_32,4);
      fs.read((char*)taginfo->name,4);
      if(taginfo->size.size_32==1)
	{
	  taginfo->largesize=true;
	  fs.read((char*)&taginfo->size.size_64,8);
	  mycctbigendian(&taginfo->size.size_64,8);
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_64);
	}
      else
	{
	  taginfo->largesize=false;
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_32);
	}
      if((*((CX_UINT32*)taginfo->name))==(*((CX_UINT32*)tagname)))
	  return 0;
    }
  fs.close();
  return -1;
}



void myf4f2flv(char* f4fpath,char* flvpath)
{
  std::ofstream flv_file;
  std::ifstream f4f_file;
  f4f_file.open(f4fpath,std::ifstream::in|std::ofstream::binary);
  flv_file.open(flvpath,std::ofstream::app|std::ifstream::binary);
  flv_file.write((char*)&prev_size,sizeof(prev_size));
  F4F_TagInfo myinfo;
  myFindF4FTagByName("mdat",&myinfo,f4fpath);
  char buffer[10000];
  int count,left;
  std::cout<<"tagname is "<<std::string(myinfo.name)<<" size is "<<myinfo.size.size_64<<std::endl;
  if(myinfo.largesize)
    {
      f4f_file.seekg(myinfo.pos_beg+16);
      std::cout<<"largesize is true,it's  "<<myinfo.size.size_64<<std::endl;
      prev_size=myinfo.size.size_64;
      std::cout<<"prev_size is "<<prev_size<<std::endl;
      count=(myinfo.size.size_64-16)/10000;
      left=(myinfo.size.size_64-16)%10000;
    }
  else
    {
      std::cout<<"largesize is false,it's "<<myinfo.size.size_32<<std::endl;
      prev_size=myinfo.size.size_32;
      std::cout<<"prev_size is "<<prev_size<<std::endl;
      f4f_file.seekg(myinfo.pos_beg+8);
      count=(myinfo.size.size_32-8)/10000;
      left=(myinfo.size.size_32-8)/10000;
    }

  for(int n=0;n<count;n++)
    {
      f4f_file.read(buffer,10000);
      flv_file.write(buffer,10000);
    }
  f4f_file.read(buffer,left);
  flv_file.write(buffer,left);
  mycvtbigendian(&prev_size,sizeof(prev_size));
  f4f_file.close();
  flv_file.close();
}

int main(int argc,char** argv)
{
  mywriteflvheader("test.flv");
  char in_file[100];
  while(in_file[0]!=32)
    {
      memset(in_file,0,100);
      std::cout<<"input the input file"<<std::endl;
      std::cin>>in_file;
      myf4f2flv(in_file,"./test.flv");
    }
  std::cout<<"convertion is over"<<std::endl;
  return 0;

 
}














