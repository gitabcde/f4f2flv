#include <fstream>
#include <iostream>
#include "moyea_base_types.h"

void cvtendian(void* input,int size)
{
  char tmp;
  char* src=(char*)input;
  for(int n=0;n<size/2;n++)
    {
      tmp=src[n];
      src[n]=src[size-n-1];
      src[size-n-1]=tmp;
    }
}
int main(int argc,char** argv)
{
  std::fstream fs;
  fs.open(argv[1],std::fstream::in|std::fstream::out|std::fstream::binary);
  uint64_t pos=0;
  char tag_type;
  uint32_t length_tagdata=0,timestamp=0,prev_video_timestamp=0,prev_audio_timestamp=0,prev_script_timestamp=0,tmp_timestamp=0;
  char* tmp_buffer;
  fs.seekp(std::fstream::beg);
  fs.seekg(std::fstream::beg);
  pos+=13;
  fs.seekp(pos);
  fs.seekg(pos);
  while(!fs.eof())
    {
      usleep(10000);
      length_tagdata=0;
      timestamp=0;
      fs.read(&tag_type,1);
      fs.read((char*)&length_tagdata,3);
      length_tagdata=length_tagdata<<8;
      cvtendian(&length_tagdata,4);
      fs.read((char*)&timestamp,3);
      timestamp=timestamp<<8;
      fs.read((char*)&timestamp,1);
      cvtendian(&timestamp,4);
      std::cout<<std::hex<<std::endl;
      switch(tag_type)
	{
	case 0x08:
	  std::cout<<"tag is audio"<<std::endl;
	  std::cout<<"length is "<<length_tagdata<<std::endl;
	  if(prev_audio_timestamp==0)
	    {
	      prev_audio_timestamp=timestamp;
	      timestamp=0;
	    }
	  else
	    {
	      timestamp=timestamp-prev_audio_timestamp;
	    }
	  std::cout<<"timestamp is "<<timestamp<<std::endl;
	  tmp_timestamp=timestamp;
	  cvtendian(&tmp_timestamp,4);
	  tmp_buffer=(char*)&tmp_timestamp;
	  tmp_buffer++;
	  fs.seekp(pos+4);
	  fs.write(tmp_buffer,3);
	  tmp_buffer--;
	  fs.write(tmp_buffer,1);
	  break;
	case 0x09:
	  std::cout<<"tag is video"<<std::endl;
	  
	  std::cout<<"length is "<<length_tagdata<<std::
	  if(prev_video_timestamp==0)
	    {
	      prev_video_timestamp=timestamp;
	      timestamp=0;
	    }
	  else
	    {
	      timestamp=timestamp-prev_video_timestamp;
	    }
	  std::cout<<"timestamp is "<<timestamp<<std::endl;
	  tmp_timestamp=timestamp;
	  cvtendian(&tmp_timestamp,4);
	  tmp_buffer=(char*)&tmp_timestamp;
	  tmp_buffer++;
	  fs.seekp(pos+4);
	  fs.write(tmp_buffer,3);
	  tmp_buffer--;
	  fs.write(tmp_buffer,1);
     	  break;
	case 0x12:
	  std::cout<<"tag is ScriptData"<<std::endl;
	  std::cout<<"pos is "<<pos<<std::endl;
	  std::cout<<"length is "<<length_tagdata<<std::endl;
	  if(prev_script_timestamp==0)
	    {
	      prev_script_timestamp=timestamp;
	      timestamp=0;
	    }
	  else
	    {
	      timestamp=timestamp-prev_script_timestamp;
	    }
	  std::cout<<"timestamp is "<<timestamp<<std::endl;
	  tmp_timestamp=timestamp;
	  cvtendian(&tmp_timestamp,4);
	  tmp_buffer=(char*)&tmp_timestamp;
	  tmp_buffer++;
	  fs.seekp(pos+4);
	  fs.write(tmp_buffer,3);
	  tmp_buffer--;
	  fs.write(tmp_buffer,1);
	  break;
	default:
	  std::cout<<"error "<<std::endl;
	  break;
	}
      pos+=length_tagdata+15;
      fs.seekp(pos);
      fs.seekg(pos);
    }
  return 0;
}
