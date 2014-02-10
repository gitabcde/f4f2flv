#include <fstream>
#include <iostream>

int main()
{
  std::fstream fs;
  fs.open("testfile",std::fstream::in|std::fstream::out);
  int mytest;
  fs>>mytest;
  std::cout<<"mytest is "<<mytest<<std::endl;
  fs.seekg(8);
  int myinput=0;
  fs.read((char*)&myinput,sizeof(myinput));
  std::cout<<"before modify,myinput is "<<myinput<<std::endl;
  fs.seekp(8);
  myinput+=10;
  fs.write((char*)&myinput,sizeof(myinput));
  fs.seekg(8);
  fs.read((char*)&myinput,sizeof(myinput));
  std::cout<<"after modify,myinput is "<<myinput<<std::endl;
  fs.close();
  return 0;
}
