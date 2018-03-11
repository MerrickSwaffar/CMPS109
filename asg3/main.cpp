// $Id: main.cpp,v 1.8 2015-04-28 19:23:13-07 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iomanip>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
                       << endl;
            break;
      }
   }
}

//remove leading and trailing whitespace from a string
string trim(string s){
    int start = s.find_first_not_of(" \t\n\r");
    int end = s.find_last_not_of(" \t\n\r");
    if (start == -1 || end == -1) return "";
    s = s.substr(start, end-start + 1);
    return s;
}

//find the key in a line of input. return the empty string if no key
string parse_key(string line){
    return trim(line.substr(0, line.find_first_of('=')));
}

//find the value in a line. return the empty string if no value
string parse_value(string line){
    return trim(line.substr(line.find_first_of('=') + 1));
}

//perform the operation for a line of input.
void do_operation(str_str_map* map, string line){
           string key = parse_key(line);
           string value = parse_value(line);
           if (line.find('=') == line.npos){
               auto i = map->find(key);
               if (i == map->end()){ 
                   cerr << key << ": key not found"<< endl;
                   sys_info::set_exit_status(1);
               }
               else 
                   cout << i->first << " = "<< i->second << endl;
           }
           else if (key == "" && value == ""){ 
               for (auto i = map->begin(); i != map->end(); ++i)
                   cout << i->first << " = " << i->second << endl;
           }
           else if (key == ""){ 
               for (auto i = map->begin(); i != map->end(); ++i)
                   if (i->second == value)
                       cout << i->first <<" = "<< i->second << endl;
           }
           else if (value == ""){
               auto i = map->find(key);
               if (i != map->end())
                   map->erase(i);
           }
           else{
               auto i = map->find(key);
               if (i == map->end()){
                   str_str_pair pair (key, value);
                   map->insert(pair);
               } else i->second = value;
               cout << key << " = " << value << endl;    
           }
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   int count = 0;
   string filename;
   str_str_map* map = new str_str_map();
   for (int i = 1; i < argc; ++i){
       filename = argv[i];
       ifstream file (filename);
       string line;
       while (getline(file, line)){
           ++count;
           cout << filename << ": " << count << ": "<< line << endl;
           if (line.empty()) continue;
           if (line.at(0) == '#' || trim(line) == "") continue;
           do_operation(map, line);
       }
   }
   if (argc == 1 || filename == "-"){
       for(;;){
           string line;
           getline(cin, line);
           if (cin.eof()) break;
           ++count;
           cout << "-: " << count << ": "<< line << endl;
           if (line.empty()) continue;
           if (line.at(0) == '#' || trim(line) == "") continue;
           do_operation(map, line);
       }
   }
   delete(map);
   return sys_info::get_exit_status ();
}

