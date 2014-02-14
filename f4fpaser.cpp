#include "f4fpaser.h"
#include <fstream>
#include <string.h>
#include <iostream>



static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

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

static void decodeblock( unsigned char in[4], unsigned char out[3] )
{
  out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

void base64_decode(const unsigned char* pIn, int len, std::string& OUT out)
{
  int m = len / 4;
  int i, j;
  unsigned char in[4];
  unsigned char v;
  unsigned char* pOut =(unsigned char*) new char[4*m];
  
  for(i=0; i<m; ++i) {
    for (j=0; j<4; ++j) {
      v = pIn[i*4+j];
      v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
      if( v ) {
	v = (unsigned char) ((v == '$') ? 0 : v - 61);
      }
      if( v ) {
	in[ j ] = (unsigned char) (v - 1);
      }
    }
    decodeblock(in, pOut+3*i);
  }
  out.clear();
  out.append((char*)pOut, 3*m);
}

int callback_writefunction(void* buff,size_t size,size_t count,void* userdata)
{
  size_t datalen=size*count;
  std::fstream f4mfile((char*)userdata,std::fstream::app|std::fstream::out);
  std::cout<<"writing file..."<<std::endl;
  f4mfile.write((char*)buff,datalen);
  f4mfile.close();
  return datalen;
}

CF4FPaser::CF4FPaser()
{
  prev_video_timestamp=0;
  prev_audio_timestamp=0;
  prev_script_timestamp=0;
  video_timestamp_offset=0;
  audio_timestamp_offset=0;
  script_timestamp_offset=0;
  timestamp=0;
  lastpos=0;
}

CF4FPaser::~CF4FPaser()
{
  
}


void CF4FPaser::DownLoadFile(char* fileurl,char filename)
{
  CURL* hd_curl=curl_easy_init();
  curl_easy_setopt(hd_curl,CURLOPT_URL,f4murl);
  curl_easy_setopt(hd_curl,CURLOPT_WRITEFUNCTION,&callback_writefunction);
  curl_easy_setopt(hd_curl,CURLOPT_WRITEDATA,filename);
  CURLcode ret_code=curl_easy_perform(hd_curl);
  if(ret_code!=CURLE_OK)
    std::cout<<"cannot download the file"<<std::endl;
  curl_easy_cleanup(hd_curl);
}


void CF4FPaser::GetF4MInfoFromFile(char* f4mfile,F4MINFO* pF4mInfo)
{
  
  std::fstream in_file(f4mfile,std::fstream::in|std::fstream::binary);
  in_file.seekg(0,std::fstream::end);
  length+=in_file.tellg();
  in_file.seekg(0,std::fstream::beg);
  char* buffer=new char[length];
  in_file.read(buffer,length);
  std::string f4m_str(buffer);
  std::size_t key_begin,key_end;
  key_begin=f4m_str.find("<streamType>")+12;
  key_end=f4m_str.find("</streamType>");
  if(f4m_str.substr(key_begin,key_end-key_begin)=="live")
    pF4mInfo->livestream=true;
  else
    pF4mInfo->livestream=false;
  
  uint8_t count=0;
  while(1)
    {
      key_begin=f4m_str.find("<meida");
      key_end=f4m_str.find("</media>",key_begin);
      if(key_begin==std::string::npos || key_end==std::string::npos)
	break;
      count++;
      key_begin=key_end+8;
    }
  quality=new char*[count];
  mediaurl=new char*[count];
  bootstrap=new char*[count];
  std::size_t quality_begin,quality_end,mediaurl_begin,mediaurl_end,bootstrap_begin,bootstrap_end;
  uint8_t tmp=0;
  while(tmp<count)
    {
      quality_begin=f4m_str.find("<bitrate=\"")+10;
      quality_end=f4m_str.find("\"",quality_begin);
      quality[tmp]=new char[quality_end-quality_begin+1];
      memset(quality[tmp],0,quality_end-quality_begin+1);
      memcpy(quality[tmp],f4m_str.substr(quality_begin,quality_end-quality_begin).c_str(),quality_end-quality_begin);
      quality_begin=quality_end;

      mediaurl_begin=f4m_str.find("<media");
      mediaurl_end=f4m_str.find("</media>",mediaurl_begin);
      mediaurl_begin=f4m_str.find("url=\"")+5;
      mediaurl_end=f4m_str.find("\"",mediaurl_begin);
      mediaurl[tmp]=new char[mediaurl_end-mediaurl_begin+1];
      memset(mediaurl[tmp],0,mediaurl_end-mediaurl_begin+1);
      memcpy(mediaurl[tmp],f4m_str.substr(mediaurl_begin,mediaurl_end-mediaurl_begin).c_str(),mediaurl_end-mediaurl_begin);
      mediaurl_begin=mediaurl_end;

      bootstrap_begin=f4m_str.find("<bootstrapInfo");
      bootstrap_end=f4m_str.find("</bootstrapInfo>",bootstrap_begin);
      if(f4m_str.find("url=",bootstrap_begin)==std::string::npos || f4m_str.find("url=",bootstrap_begin)>bootstrap_end)
	{
	  bootstrap_begin=f4m_str.find(">",bootstrap_begin)+1;
	  pF4mInfo->include_bootstrap=true;
	}
      else
	{
	  
	  bootstrap_begin=f4m_str.find("url=\"",bootstrap_begin)+5;
	  bootstrap_end=f4m_str.find("\"",bootstrap_begin);
	}
      bootstrap[tmp]=new char[bootstrap_end-bootstrap_begin+1];
      memset(bootstrap[tmp],0,bootstrap_end-bootstrap_begin+1);
      memcpy(bootstrap[tmp],f4m_str.substr(bootstrap_begin,bootstrap_end-bootstrap_begin).c_str(),bootstrap_end-bootstrap_begin);
      bootstrap_begin=bootstrap_end;
    }
}

void CF4FPaser::CreateFlvFile(char* flvname)
{
  std::fstream flvfile;
  flvfile.open(flvname,std::fstream::out|std::fstream::app|std::fstream::binary);
  flvfile.close();
  flvfile.open(flvname,std::fstream::in|std::fstream::out|std::fstream::binary);
  flvfile.write(FLVHEADER,sizeof(FLVHEADER));
  flvfile.close();
  std::cout<<"create flv file"<<std::endl;
}

void CF4FPaser::WriteFlvDataFromF4file(char* f4file,char* flvname)
{

  std::ifstream f4f_file(f4file,std::ifstream::in|std::ifstream::binary);
  std::fstream flvfile(flvname,std::fstream::out|std::fstream::in|std::fstream::binary);
  F4FTagInfo myinfo;
  flvfile.seekp(0,std::fstream::end);
  lastpos=flvfile.tellp();
  if(GetTagInfoFromF4file(f4file,"mdat",&myinfo)!=0)
    {
      flvfile.close();
      f4f_file.close();
      std::cout<<"wrong filecontent"<<std::endl;
      return;
    }
  char buffer[F4F_BUFFER_SIZE];
  int count,left;
  std::cout<<"filename is "<<f4file<<std::endl;
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
  flvfile.close();
  std::cout<<"prepare to ajusting timestamp"<<std::endl;
  AjustFlvTimeStamp(flvname);
}

int CF4FPaser::GetTagInfoFromF4file(char* filepath,char* tagname,F4FTagInfo* taginfo)
{
  int ret=0;
  std::ifstream fs(filepath,std::ifstream::in|std::ifstream::binary);
  uint32_t src_tagname,dst_tagname;
  fs.seekg(0,std::ifstream::beg);
  while(!fs.eof())
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
	{
	  fs.close();
	  return 0;
	}
    }
  fs.close();
  return -1;
}

void CF4FPaser::AjustFlvTimeStamp(char* flvname)
{
  std::cout<<"begin to ajust timestamp"<<std::endl;
  std::cout<<std::hex<<std::endl;
  char tag_type;
  uint32_t length_tagdata=0;
  std::fstream flvfile(flvname,std::fstream::out|std::fstream::in|std::fstream::binary);
  while(!flvfile.eof())
    {
      flvfile.seekg(lastpos);
      flvfile.seekp(lastpos);
      length_tagdata=0;
      flvfile.read(&tag_type,1);
      flvfile.read((char*)&length_tagdata,3);
      if(IsBigEndian())
	length_tagdata=length_tagdata>>8;
      else
	{
	  length_tagdata=length_tagdata<<8;
	  CvtEndian(&length_tagdata,4);
	}
      if(length_tagdata==0)
	continue;
      flvfile.read((char*)&timestamp,3);
      if(IsBigEndian())
	timestamp=timestamp>>8;
      else
	timestamp=timestamp<<8;
      flvfile.read((char*)&timestamp,1);
      if(!IsBigEndian())
	CvtEndian(&timestamp,4);
      switch(tag_type)
	{
	case 0x08:
	  std::cout<<"audio tag"<<std::endl;
	  if(prev_audio_timestamp==0)
	    prev_audio_timestamp=timestamp;
	  if(timestamp-prev_audio_timestamp>2000)
	    prev_audio_timestamp=timestamp-audio_timestamp_offset;
	  else
	    audio_timestamp_offset+=timestamp-prev_audio_timestamp;
	  prev_audio_timestamp=timestamp;
	  timestamp=audio_timestamp_offset;
	  std::cout<<"audio's timestamp is "<<timestamp<<std::endl;
	  std::cout<<"length is "<<length_tagdata<<std::endl;
	  if(!IsBigEndian())
	    CvtEndian(&timestamp,4);
	  lastpos+=4;
	  flvfile.seekp(lastpos);
	  flvfile.write(((char*)&timestamp)+1,3);
	  flvfile.write((char*)&timestamp,1);
	  break;
	case 0x09:
	  std::cout<<"video tag"<<std::endl;
	  if(prev_video_timestamp==0)
	    prev_video_timestamp=timestamp;
	  if(timestamp-prev_video_timestamp>2000)
	    prev_video_timestamp=timestamp-video_timestamp_offset;
	  else
	    video_timestamp_offset+=timestamp-prev_video_timestamp;
	  prev_video_timestamp=timestamp;
	  timestamp=video_timestamp_offset;
	  std::cout<<"video's timestamp is "<<timestamp<<std::endl;
	  std::cout<<"length is "<<length_tagdata<<std::endl;
	  if(!IsBigEndian())
	    CvtEndian(&timestamp,4);
	  lastpos+=4;
	  flvfile.seekp(lastpos);
	  flvfile.write(((char*)&timestamp)+1,3);
	  flvfile.write((char*)&timestamp,1);
	  break;
	case 0x12:
	  std::cout<<"script tag"<<std::endl;
	  if(prev_script_timestamp==0)
	    prev_script_timestamp=timestamp;
	  if(timestamp-prev_script_timestamp>2000)
	    prev_script_timestamp=timestamp-script_timestamp_offset;
	  else
	    script_timestamp_offset+=timestamp-prev_script_timestamp;
	  prev_script_timestamp=timestamp;
	  timestamp=script_timestamp_offset;
	  std::cout<<"script's timestamp is "<<timestamp<<std::endl;
	  std::cout<<"length is "<<length_tagdata<<std::endl;
	  if(!IsBigEndian())
	    CvtEndian(&timestamp,4);
	  lastpos+=4;
	  flvfile.seekp(lastpos);
	  flvfile.write(((char*)&timestamp)+1,3);
	  flvfile.write((char*)&timestamp,1);
	  break;
	default:
	  std::cout<<"error"<<std::endl;
	  break;
	}
      lastpos+=length_tagdata+11;
    }
 
  flvfile.close();
  std::cout<<std::dec<<std::endl;

}
int main()
{
  CF4FPaser myf4f;
  /*
  F4FTagInfo myinfo;
  myf4f.CreateFlvFile("mycf4f.flv");
  std::string filename;
  while(1)
    {
      std::cout<<"input f4f filename"<<std::endl;
      std::cin>>filename;
      if(filename=="")
	break;
      myf4f.WriteFlvDataFromF4file((char*)filename.c_str(),"mycf4f.flv");

    }
  
  std::cout<<"input the f4m's url"<<std::endl;
  std::string f4murl;
  std::cin>>f4murl;
  std::cout<<"url is "<<(char*)f4murl.c_str()<<std::endl;
  myf4f.DownLoadF4fFromF4m((char*)f4murl.c_str());
  */
  std::string mydec;
  base64_decode((const unsigned char*)"AAAAi2Fic3QAAAAAAAAAAQAAAAPoAAAAAAABce8AAAAAAAAAAAAAAAAAAQAAABlhc3J0AAAAAAAAAAABAAAAAQAAAA8BAAAARmFmcnQAAAAAAAAD6AAAAAADAAAAAQAAAAAAAAAAAAAXcAAAAA\
8AAAAAAAFIIAAAKc8AAAAAAAAAAAAAAAAAAAAAAA==",190,mydec);
  std::fstream myfs("./mydec",std::fstream::out);
  myfs.write((char*)mydec.c_str(),mydec.size());
  myfs.close();
  return 0;

}
