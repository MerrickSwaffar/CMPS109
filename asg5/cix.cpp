// $Id: cix.cpp,v 1.2 2015-05-12 18:59:40-07 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"get" , CIX_GET },
   {"put" , CIX_PUT },
   {"rm"  , CIX_RM  },
};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = CIX_LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != CIX_LSOUT) {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

void cix_get (client_socket& server, const string& filename) {
   cix_header header;
   header.command = CIX_GET;
   header.nbytes = 0;
   strcpy(header.filename, filename.c_str()); 
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   log << "received " << header.nbytes << " bytes" << endl;
   if (header.command != CIX_FILE) {
      log << "sent CIX_GET, server did not return CIX_FILE" << endl;
      log << "server returned " << header << endl;
   } else {
      ofstream file(filename);
      char buf[header.nbytes + 1];
      recv_packet (server, buf, header.nbytes);
      buf[header.nbytes] = '\0';
      file.write(buf, header.nbytes);
      file.close();
   }
}

void cix_put (client_socket& server, const string& filename) {
   cix_header header;
   header.command = CIX_PUT;
   strcpy(header.filename, filename.c_str()); 
   stringstream s;
   ifstream file(filename);
   s << file.rdbuf();
   string payload = s.str();
   header.nbytes = payload.size();
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header); 
   send_packet (server, payload.c_str(), payload.size());
   log << "sent " << header.nbytes << " bytes" << endl;
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != CIX_ACK) {
      log << "sent CIX_PUT, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
   }
}

void cix_rm (client_socket& server, const string& filename) {
   cix_header header;
   header.command = CIX_RM;
   header.nbytes = 0;
   strcpy(header.filename, filename.c_str()); 
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   log << "received " << header.nbytes << " bytes" << endl;
   if (header.command != CIX_ACK) {
       log << "sent CIX_RM, server did not return CIX_ACK" << endl;
       log << "server returned " << header << endl;
   }
}

void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

string trim(const string& line) {
   int begin = line.find_first_not_of(" \t");
   if (begin == -1) return "";
   int end = line.find_last_not_of(" \t");
   return line.substr(begin, end-begin+1);
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host;
   in_port_t port;
   if (args.size() == 1) {
      host = get_cix_server_host(args, args.size()+1);
      port = get_cix_server_port(args, 0);
   } else {
      host = get_cix_server_host(args, 0);
      port = get_cix_server_port(args, 1);
   }
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command " << line << endl;
         string command = trim(line);
         string filename;
         int split = command.find(" ");
         if (split != -1){
            filename = command.substr(split+1);
            command = command.substr(0, split);
         }
         const auto& itor = command_map.find (command);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
         switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_GET:
               cix_get(server, filename);
               break;
            case CIX_PUT:
               cix_put(server, filename);
               break;
            case CIX_RM:
               cix_rm(server, filename);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

