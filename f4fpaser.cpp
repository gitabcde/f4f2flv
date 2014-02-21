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

int WriteToString(void* buff,size_t size,size_t count,void* userdata)
{
  size_t datalen=size*count;
  std::string *str=(std::string*)userdata;
  str->append((char*)buff,datalen);
  return datalen;
}

int WriteToFile(void* buff,size_t size,size_t count,void* userdata)
{
  int* n=(int*)userdata;
  size_t datalen=size*count;
  char f4fname[30];
  memset(f4fname,0,30);
  sprintf(f4fname,"video%d.f4f",*n);
  std::fstream fs(f4fname,std::fstream::out|std::fstream::binary|std::fstream::app);
  fs.write((char*)buff,datalen);
  fs.close();
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
  current_frag=0;
  current_seg=0;  
  hd_curl=curl_easy_init();
  prev_type_video=true;
  have_set_zero_audio=false;
  have_set_zero_video=false;
}

CF4FPaser::~CF4FPaser()
{
  curl_easy_cleanup(hd_curl);
}


void CF4FPaser::SetF4m(char* f4murl)
{
  WriteToStringFromUrl(f4murl,f4m_str);
  f4m_baseurl=f4murl;
  f4m_baseurl=f4m_baseurl.substr(0,f4m_baseurl.rfind("/")+1);
}


void CF4FPaser::GetF4MInfo(F4MINFO* pF4mInfo)
{
  std::size_t key_begin=0,key_end=0;
  key_begin=f4m_str.find("<streamType>")+12;
  key_end=f4m_str.find("</streamType>");
  if(f4m_str.substr(key_begin,key_end-key_begin).find("live")!=std::string::npos)
    pF4mInfo->livestream=true;
  else
    pF4mInfo->livestream=false;
  uint8_t count=0;
  while(1)
    {
      key_begin=f4m_str.find("<media",key_begin);
      key_end=f4m_str.find("</media>",key_begin);
      if(key_begin==std::string::npos || key_end==std::string::npos)
	break;
      count++;
      key_begin=key_end;
    }
  pF4mInfo->qualitycount=count;
  pF4mInfo->quality=new char*[count];
  pF4mInfo->mediaurl=new char*[count];
  pF4mInfo->bootstrap=new std::string[count];
  pF4mInfo->first_seg=new uint32_t[count];
  pF4mInfo->last_seg=new uint32_t[count];
  pF4mInfo->first_seg_fragcount=new uint32_t[count];
  pF4mInfo->last_seg_fragcount=new uint32_t[count];
  pF4mInfo->first_frag_duration=new uint32_t[count];
  pF4mInfo->last_frag_duration=new uint32_t[count];
  std::size_t quality_begin=0,quality_end=0,mediaurl_begin=0,mediaurl_end=0,bootstrap_begin=0,bootstrap_end=0;
  uint8_t tmp=0;
  while(tmp<count)
    {
      quality_begin=f4m_str.find("bitrate=\"",quality_begin)+9;
      quality_end=f4m_str.find("\"",quality_begin);
      pF4mInfo->quality[tmp]=new char[quality_end-quality_begin+1];
      memset(pF4mInfo->quality[tmp],0,quality_end-quality_begin+1);
      memcpy(pF4mInfo->quality[tmp],f4m_str.substr(quality_begin,quality_end-quality_begin).c_str(),quality_end-quality_begin);
      quality_begin=quality_end;

      mediaurl_begin=f4m_str.find("<media",mediaurl_begin);
      mediaurl_end=f4m_str.find("</media>",mediaurl_begin);
      mediaurl_begin=f4m_str.find("url=\"",mediaurl_begin)+5;
      mediaurl_end=f4m_str.find("\"",mediaurl_begin);
      pF4mInfo->mediaurl[tmp]=new char[mediaurl_end-mediaurl_begin+1];
      memset(pF4mInfo->mediaurl[tmp],0,mediaurl_end-mediaurl_begin+1);
      memcpy(pF4mInfo->mediaurl[tmp],f4m_str.substr(mediaurl_begin,mediaurl_end-mediaurl_begin).c_str(),mediaurl_end-mediaurl_begin);
      mediaurl_begin=mediaurl_end;
      bootstrap_begin=f4m_str.find("<bootstrapInfo",bootstrap_begin);
      bootstrap_end=f4m_str.find("</bootstrapInfo>",bootstrap_begin);
      if(f4m_str.find("url=",bootstrap_begin)==std::string::npos || f4m_str.find("url=",bootstrap_begin)>bootstrap_end)
	{
	  bootstrap_begin=f4m_str.find(">",bootstrap_begin)+1;
	  pF4mInfo->include_bootstrap=true;
	}
      else
	{
	  pF4mInfo->include_bootstrap=false;
	  bootstrap_begin=f4m_str.find("url=\"",bootstrap_begin)+5;
	  bootstrap_end=f4m_str.find("\"",bootstrap_begin);
	}
      /*
      if(pF4mInfo->include_bootstrap)
	std::cout<<"include_bootstrap is true"<<std::endl;
      else
	std::cout<<"include_bootstrap is false"<<std::endl;
      */
      std::cout<<"bst's begin is "<<bootstrap_begin<<" ,bst's end is "<<bootstrap_end<<std::endl;
      pF4mInfo->bootstrap[tmp].append(f4m_str.substr(bootstrap_begin,bootstrap_end-bootstrap_begin).c_str(),bootstrap_end-bootstrap_begin);
      bootstrap_begin=bootstrap_end;

      GetBootstrap(pF4mInfo,tmp);
      GetSegFragInfo(pF4mInfo,tmp);
      tmp++;
    }
}

int CF4FPaser::GetVideoSegUrl(std::string &videourl, F4MINFO *pF4mInfo, int qualitylvl)
{
  if(pF4mInfo->livestream)
    {
      if(current_frag==0)
	{
	  current_seg=pF4mInfo->last_seg[qualitylvl];
	  current_frag=0;
	}
      if(current_frag<=pF4mInfo->last_seg_fragcount[qualitylvl])
	current_frag++;
      else
	{
	  GetBootstrap(pF4mInfo,qualitylvl);
	  GetSegFragInfo(pF4mInfo,qualitylvl);
	  current_seg=pF4mInfo->last_seg[qualitylvl];
	  current_frag=1;
	  if(current_frag>pF4mInfo->last_seg_fragcount[qualitylvl])
	    return -1;
	}
    }
  else
    {
      if(current_seg==0 || current_frag==0)
	{
	  current_seg=pF4mInfo->first_seg[qualitylvl];
	  current_frag=0;
	}
      current_frag++;
      if(current_seg<pF4mInfo->last_seg[qualitylvl])
	{
	  current_frag++;
	  if(current_frag>pF4mInfo->first_seg_fragcount[qualitylvl])
	    {
	      current_frag=1;
	      current_seg++;
	    }
	}
      else
	{
	  if(current_frag>pF4mInfo->last_seg_fragcount[qualitylvl])
	    return -1;
	}
    }

  videourl="";
  std::string tmp_baseurl=f4m_baseurl;
  char* tmp_mediaurl=pF4mInfo->mediaurl[qualitylvl];
  if(strstr(tmp_mediaurl,"../")!=NULL)
    tmp_baseurl=f4m_baseurl.substr(0,f4m_baseurl.rfind("/"));
  while(strstr(tmp_mediaurl,"../")!=NULL)
    {
      tmp_mediaurl=strstr(tmp_mediaurl,"../")+2;
      tmp_baseurl=tmp_baseurl.substr(0,f4m_baseurl.rfind("/"));
      std::cout<<"tmp_baseurl is "<<tmp_baseurl<<std::endl;
    }
  videourl.append(tmp_baseurl);
  videourl.append(tmp_mediaurl);
  char buffer[100];
  memset(buffer,0,100);
  sprintf(buffer,"Seg%d-Frag%d",current_seg,current_frag);
  videourl.append(buffer);
  std::cout<<"videourl in GetVideoSegUrl is "<<videourl<<std::endl;

  return 0;
}

void CF4FPaser::GetSegFragInfo(F4MINFO* pF4mInfo,int qualitylvl)
{
  uint32_t tag_length;
  if(pF4mInfo->livestream)
    std::cout<<"livestream in GetSegFragInfo"<<std::endl;
  else
    std::cout<<"recordedstream in GetSegFragInfo"<<std::endl;
  
  tag_length=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pF4mInfo->bootstrap[qualitylvl].find("asrt")-4));
  if(!IsBigEndian())
    CvtEndian(&tag_length,4);
  std::cout<<"tag_length is "<<tag_length<<std::endl;
  int pos=0;
  if(tag_length==1)
    pos=pF4mInfo->bootstrap[qualitylvl].find("asrt")+16;
  else
    pos=pF4mInfo->bootstrap[qualitylvl].find("asrt")+8;
  uint8_t qualityEntryCount;
  qualityEntryCount=(*(uint8_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  std::cout<<"qualityEntryCount is "<<(uint32_t)qualityEntryCount<<std::endl;
  pos++;
  for(;qualityEntryCount>0;qualityEntryCount--)
    {
	  pos=pF4mInfo->bootstrap[qualitylvl].find("\0",pos)+1;
    }
  uint32_t segcount=0;
  
  std::cout<<"now pos is "<<pos<<std::endl;
  segcount=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  if(!IsBigEndian())
    CvtEndian(&segcount,4);
  std::cout<<"segcount is "<<segcount<<std::endl;
  pos+=4;
  pF4mInfo->first_seg[qualitylvl]=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  if(!IsBigEndian())
    CvtEndian(&pF4mInfo->first_seg[qualitylvl],4);
  pF4mInfo->first_seg_fragcount[qualitylvl]=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos+4));
  if(!IsBigEndian())
    CvtEndian(&pF4mInfo->first_seg_fragcount[qualitylvl],4);
  pos+=(segcount-1)*8;
  pF4mInfo->last_seg[qualitylvl]=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  if(!IsBigEndian())
    CvtEndian(&pF4mInfo->last_seg[qualitylvl],4);
  pF4mInfo->last_seg_fragcount[qualitylvl]=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos+4));
  if(!IsBigEndian())
    CvtEndian(&pF4mInfo->last_seg_fragcount[qualitylvl],4);

  tag_length=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pF4mInfo->bootstrap[qualitylvl].find("afrt")-4));
  if(!IsBigEndian())
    CvtEndian(&tag_length,4);
  std::cout<<"tag_length is "<<tag_length<<std::endl;
  if(tag_length==1)
    pos=pF4mInfo->bootstrap[qualitylvl].find("asrt")+12;
  else
    pos=pF4mInfo->bootstrap[qualitylvl].find("asrt")+20;
  qualityEntryCount=(*(uint8_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  pos++;
  for(;qualityEntryCount>0;qualityEntryCount--)
    {
      pos=pF4mInfo->bootstrap[qualitylvl].find("\0",pos)+1;
    }
  uint32_t fragEntryCount;
  fragEntryCount=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  if(!IsBigEndian())
    CvtEndian(&fragEntryCount,4);
  pos+=4;
  pF4mInfo->first_frag_duration[qualitylvl]=(*(uint32_t*)(pF4mInfo->bootstrap[qualitylvl].c_str()+pos));
  if(!IsBigEndian())
    CvtEndian(&pF4mInfo->first_frag_duration[qualitylvl],4);
  
}


void CF4FPaser::WriteToStringFromUrl(std::string url,std::string& str)
{
  curl_easy_setopt(hd_curl,CURLOPT_USERAGENT,"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
  curl_easy_setopt(hd_curl,CURLOPT_URL,url.c_str());
  curl_easy_setopt(hd_curl,CURLOPT_WRITEFUNCTION,&WriteToString);
  curl_easy_setopt(hd_curl,CURLOPT_WRITEDATA,&str);
  CURLcode ret_code=curl_easy_perform(hd_curl);
  if(ret_code!=CURLE_OK)
    std::cout<<"cannot download the file"<<std::endl;

}

void CF4FPaser::GetBootstrap(F4MINFO* pF4mInfo,int qualitylvl)
{
  if(pF4mInfo->livestream)
    {
      std::cout<<"fail to finding bootstrap in f4m file"<<std::endl;
      std::string bootstrap_url;
      char* tmp_bootstrap_url=pF4mInfo->mediaurl[qualitylvl];
      if(strstr(tmp_bootstrap_url,"../")!=NULL)
	bootstrap_url=f4m_baseurl.substr(0,f4m_baseurl.rfind("/"));
      while(strstr(tmp_bootstrap_url,"../")!=NULL)
	{
	  tmp_bootstrap_url=strstr(tmp_bootstrap_url,"../")+2;
	  bootstrap_url=bootstrap_url.substr(0,bootstrap_url.rfind("/"));
	}
      bootstrap_url.append(tmp_bootstrap_url);
      bootstrap_url.append(".bootstrap");
      std::cout<<"bootstrap_url in GetBootStrap is "<<bootstrap_url<<std::endl;
      WriteToStringFromUrl(bootstrap_url,pF4mInfo->bootstrap[qualitylvl]);
      std::cout<<"bootstrap in GetBootStrap is "<<pF4mInfo->bootstrap[qualitylvl]<<std::endl;
    }
  else
    {
      std::cout<<"find bootstrap in f4m filee"<<std::endl;
      int length=pF4mInfo->bootstrap[qualitylvl].size();
      base64_decode((const unsigned char*)pF4mInfo->bootstrap[qualitylvl].c_str(),length,pF4mInfo->bootstrap[qualitylvl]);
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
  //std::cout<<"create flv file"<<std::endl;
}

void CF4FPaser::WriteFlvDataFromF4file(char* f4file,char* flvname)
{
  F4FTagInfo myinfo;
  myinfo.largesize=false;
  myinfo.size.size_32=0;
  myinfo.size.size_64=0;
  memset(myinfo.name,0,4);
  myinfo.pos_beg=0;
  while(GetTagInfoFromFile(f4file,"mdat",&myinfo)==0)
    {
      std::ifstream f4f_file(f4file,std::ifstream::in|std::ifstream::binary);
      std::fstream flvfile(flvname,std::fstream::out|std::fstream::in|std::fstream::binary);
      flvfile.seekp(0,std::fstream::end);
      std::cout<<"pos_beg is "<<myinfo.pos_beg<<std::endl;      
      std::cout<<"begin to WriteFlvDataFromF4file"<<std::endl;
      lastpos=flvfile.tellp();
      //      std::cout<<"lastpos in WriteFlvDataFromF4file is "<<lastpos<<std::endl;
      char buffer[F4F_BUFFER_SIZE];
      int count,left;
      std::cout<<"filename is "<<f4file<<std::endl;
      std::cout<<"tagname is "<<std::string(myinfo.name)<<" size is "<<myinfo.size.size_64<<std::endl;
      //     std::cout<<"pos_beg is "<<myinfo.pos_beg<<std::endl;
      if(myinfo.largesize)
	{
	  f4f_file.seekg(myinfo.pos_beg+16);
	  std::cout<<"largesize is true,it's  "<<myinfo.size.size_64<<std::endl;
	  count=(myinfo.size.size_64-16)/F4F_BUFFER_SIZE;
	  left=(myinfo.size.size_64-16)%F4F_BUFFER_SIZE;
	  myinfo.pos_beg+=myinfo.size.size_64;
	}
      else
	{
	  std::cout<<"largesize is false,it's "<<myinfo.size.size_32<<std::endl;
	  f4f_file.seekg(myinfo.pos_beg+8);
	  count=(myinfo.size.size_32-8)/F4F_BUFFER_SIZE;
	  left=(myinfo.size.size_32-8)/F4F_BUFFER_SIZE;
	  myinfo.pos_beg+=myinfo.size.size_32;
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

  std::cout<<"No valid content left in f4file"<<std::endl;
   
}

int CF4FPaser::GetTagInfoFromFile(char* filepath,char* tagname,F4FTagInfo* taginfo)
{
  std::cout<<"begin GetTagInfoFromFile"<<std::endl;
  int ret=0;
  std::ifstream fs(filepath,std::ifstream::in|std::ifstream::binary);
  uint32_t src_tagname,dst_tagname;
  fs.seekg(taginfo->pos_beg);
  std::cout<<"taginfo->pos_beg is "<<taginfo->pos_beg<<std::endl;
  while(!fs.eof())
    {
      fs.read((char*)&taginfo->size.size_32,4);
      if(!IsBigEndian())
	CvtEndian(&taginfo->size.size_32,4);
      if(taginfo->size.size_32==1)
	{
	  taginfo->largesize=true;
	  fs.read((char*)&taginfo->size.size_64,8);
	  if(!IsBigEndian())
	    CvtEndian(&taginfo->size.size_64,8);
     	}
      else
	{
	  taginfo->largesize=false;
	}
      fs.read((char*)taginfo->name,4);
      std::cout<<"tagname is "<<taginfo->name<<std::endl;
      if((*((uint32_t*)taginfo->name))==(*((uint32_t*)tagname)))
	{
	  fs.close();
	  return 0;
	}
      if(taginfo->size.size_32==1)
	{
	  taginfo->largesize=true;
	  fs.read((char*)&taginfo->size.size_64,8);
	  if(!IsBigEndian())
	    CvtEndian(&taginfo->size.size_64,8);
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_64);
	  taginfo->pos_beg+=taginfo->size.size_64;
	}
      else
	{
	  taginfo->largesize=false;
	  fs.seekg(taginfo->pos_beg+taginfo->size.size_32);
	  taginfo->pos_beg+=taginfo->size.size_32;
	}
      
    }
  fs.close();
  return -1;
}

void CF4FPaser::AjustFlvTimeStamp(char* flvname)
{
  std::cout<<"begin to ajust timestamp"<<std::endl;
  char tag_type;
  uint32_t length_tagdata=0;
  std::fstream flvfile(flvname,std::fstream::out|std::fstream::in|std::fstream::binary);
  while(lastpos!=(size_t)-1)
    {
      std::cout<<"AjustFlvTimeStamp"<<std::endl;
      //std::cout<<"lastpos is "<<std::hex<<lastpos<<std::endl;
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
      //if(length_tagdata==0)
      //continue;
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
	    {
	      if(!have_set_zero_audio)
		{
		  prev_audio_timestamp=timestamp;
		  have_set_zero_audio=true;
		}
	    }

	  if(timestamp-prev_audio_timestamp>1000)
	    {
	      std::cout<<"audio is not continous"<<std::endl;
	      std::cout<<"timestamp is "<<timestamp<<" ,prev_audio_timestamp is "<<prev_audio_timestamp<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	      prev_audio_timestamp=timestamp-40;
	    }
	  else
	    {
	      std::cout<<"normal timestamp in audio"<<std::endl;
	      std::cout<<"timestamp is "<<timestamp<<" ,prev_audio_timestamp is "<<prev_audio_timestamp<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	      
	    }
	  //	  audio_timestamp_offset=(video_timestamp_offset>audio_timestamp_offset?video_timestamp_offset:audio_timestamp_offset);
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
	  prev_type_video=false;
	  break;
	case 0x09:
	  std::cout<<"video tag"<<std::endl;
	  if(prev_video_timestamp==0)
	    {
	      if(!have_set_zero_video)
		{
		  have_set_zero_video=true;
		  prev_video_timestamp=timestamp;
		}
	    }
	  if(timestamp-prev_video_timestamp>1000)
	    {
	      std::cout<<"video is not continous"<<std::endl;
	      std::cout<<"timestamp is "<<timestamp<<" ,prev_video_timestamp is "<<prev_video_timestamp<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	      prev_video_timestamp=timestamp-40;
	    }
	  else
	    {
	      std::cout<<"normal timestamp in video"<<std::endl;
	      std::cout<<"timestamp is "<<timestamp<<" ,prev_video_timestamp is "<<prev_video_timestamp<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	    }
	  
	  //	  video_timestamp_offset=(video_timestamp_offset>audio_timestamp_offset?video_timestamp_offset:audio_timestamp_offset);
	  video_timestamp_offset+=timestamp-prev_video_timestamp;
	  prev_video_timestamp=timestamp;
	  timestamp=video_timestamp_offset;
	  std::cout<<std::hex<<"video's timestamp is "<<timestamp<<std::endl;
	  std::cout<<std::hex<<"length is "<<length_tagdata<<std::endl;
	  if(!IsBigEndian())
	    CvtEndian(&timestamp,4);
	  lastpos+=4;
	  flvfile.seekp(lastpos);
	  flvfile.write(((char*)&timestamp)+1,3);
	  flvfile.write((char*)&timestamp,1);
	  prev_type_video=true;
	  break;
	case 0x12:
	  std::cout<<"script tag"<<std::endl;
	  if(prev_script_timestamp==0)
	    prev_script_timestamp=timestamp;
	  if(timestamp-prev_script_timestamp>1000)
	    {
	      std::cout<<"script's timestamp is not continous"<<std::endl;
	      std::cout<<"timestamp is "<<timestamp<<" ,prev_script_timestamp is "<<prev_script_timestamp<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	      prev_script_timestamp=timestamp-40;

	    }
	  else
	    {
	      std::cout<<"normal script timestamp"<<std::endl;
	      std::cout<<"pos is "<<lastpos<<std::endl;
	      prev_script_timestamp=timestamp;
	    }
	  script_timestamp_offset+=timestamp-prev_script_timestamp;
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
	  std::cout<<"unknown datatype"<<std::endl;
	  if(prev_type_video)
	    timestamp=video_timestamp_offset;
	  else
	    timestamp=audio_timestamp_offset;
	  if(!IsBigEndian())
	    CvtEndian(&timestamp,4);
	  lastpos+=4;
	  flvfile.seekp(lastpos);
	  flvfile.write(((char*)&timestamp)+1,3);
	  flvfile.write((char*)&timestamp,1);
	  break;
	}
      flvfile.seekp(lastpos+length_tagdata+11);
      lastpos=flvfile.tellg();
    }
 
  flvfile.close();
  std::cout<<std::dec<<std::endl;

}
void CF4FPaser::print()
{
  std::cout<<"f4m_baseurl is "<<f4m_baseurl<<std::endl;
  std::cout<<"f4m_str is "<<f4m_str<<std::endl;
}
int main()
{
  std::cout<<std::hex;
  CF4FPaser myf4f;
#ifdef F4F_MAKER
  int first=0,last=0;
  std::string filename_prex;
  std::cout<<"input the common string of all the files"<<std::endl;
  std::cin>>filename_prex;
  std::cout<<"input the first f4ffile's number and the last f4ffile's number to make a flv file.I will makeup all the files between them."<<std::endl;
  std::cout<<"first f4ffile's number:";
  std::cin>>first;
  std::cout<<"last f4ffile's number:";
  std::cin>>last;
  char f4fname[30];
  memset(f4fname,0,30);
  std::cout<<"filename_prex is "<<filename_prex<<std::endl;
  memcpy(f4fname,filename_prex.c_str(),filename_prex.size());
  myf4f.CreateFlvFile("videourl.flv");
  std::cout<<"begin to combine f4files to flvfile"<<std::endl;
  for(int n=first;n<=last;n++)
    {
      sprintf(f4fname+filename_prex.size(),"%d.f4f",n);
      std::cout<<"processing file:"<<f4fname<<std::endl;
      myf4f.WriteFlvDataFromF4file(f4fname,"videourl.flv");
    }
#endif  


#ifdef F4F_DOWNLOAD
  //myf4f.SetF4m("http://cdnl3.1internet.tv-live.hds.adaptive.level3.net/hds-live11/livepkgr/_definst_/1tv-hdbk.f4m?e=1392690176");
  myf4f.SetF4m("http://hdflashmegatv-f.akamaihd.net/z/,content/2012/07/27/k386195_%7BD5743677-B7DD-46C6-927A-5630BD688199%7D_lo.mp4,.csmil/manifest.f4m?hdcore=2.11.3&g=LUICOABJOFQO");
  
  F4MINFO myf4minfo;
  myf4f.GetF4MInfo(&myf4minfo);
  
  if(myf4minfo.livestream)
    std::cout<<"livestream"<<std::endl;
  else 
    std::cout<<"recordstream"<<std::endl;
  std::cout<<"streamnum is "<<(int)myf4minfo.qualitycount<<std::endl;
  for(int n=0;n<myf4minfo.qualitycount;n++)
    {
      std::cout<<"quality["<<n<<"] is "<<myf4minfo.quality[n]<<std::endl;
      std::cout<<"mediaurl["<<n<<"] is "<<myf4minfo.mediaurl[n]<<std::endl;
      std::cout<<"bootstrap["<<n<<"] is "<<myf4minfo.bootstrap[n]<<std::endl;
      std::cout<<"first_seg["<<n<<"] is "<<myf4minfo.first_seg[n]<<std::endl;
      std::cout<<"last_seg["<<n<<"] is "<<myf4minfo.last_seg[n]<<std::endl;
      std::cout<<"first_seg_fragcount["<<n<<"] is "<<myf4minfo.first_seg_fragcount[n]<<std::endl;
      std::cout<<"last_seg_fragcount["<<n<<"] is "<<myf4minfo.last_seg_fragcount[n]<<std::endl;
    }
  myf4f.print();
  std::string mytmpstr;
  int n=0;
  CURL* hd_curl=curl_easy_init();
  while(myf4f.GetVideoSegUrl(mytmpstr,&myf4minfo,0)==0)
    {
      curl_easy_setopt(hd_curl,CURLOPT_USERAGENT,"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
      curl_easy_setopt(hd_curl,CURLOPT_URL,mytmpstr.c_str());
      curl_easy_setopt(hd_curl,CURLOPT_WRITEFUNCTION,&WriteToFile);
      curl_easy_setopt(hd_curl,CURLOPT_WRITEDATA,&n);
      CURLcode ret_code=curl_easy_perform(hd_curl);
      if(ret_code!=CURLE_OK)
	{
	  std::cout<<"ret code is "<<ret_code<<std::endl;
	  std::cout<<"cannot download video"<<std::endl;
	}
      n++;
    }
  curl_easy_cleanup(hd_curl);
  std::cout<<"videourl is "<<mytmpstr<<std::endl;
#endif

#ifdef F4F_TEST
  myf4f.CreateFlvFile("mytestvideo.flv");
  myf4f.WriteFlvDataFromF4file("video0.f4f","mytestvideo.flv");
  myf4f.WriteFlvDataFromF4file("video1.f4f","mytestvideo.flv");

#endif

#ifdef F4F_INFO
  F4FTagInfo mytaginfo;
  myf4f.GetTagInfoFromFile("video1.f4f","mdat",&mytaginfo);
  std::cout<<"video1.f4f's size is "<<mytaginfo.size.size_32<<std::endl;
  myf4f.GetTagInfoFromFile("video2.f4f","mdat",&mytaginfo);
  std::cout<<"video2.f4f's size is "<<mytaginfo.size.size_32<<std::endl;
#endif
  return 0;

}
