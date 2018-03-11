// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"
#include <iostream>
#include <string>

using namespace std;

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for (unsigned int i = 1; i < words.size(); ++i){
      state.get_cwd()->get_contents()->find_node(words.at(i))->
         get_contents()->print();
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ((words.size() == 1) || (words.at(1) == "/")){
       state.set_cwd(state.get_root());
       state.get_pwd().clear();
       return;
   }
   if (words.at(1) == ".") return;
   state.set_cwd(state.get_cwd()->get_contents()->
      find_node(words.at(1)));
   if (words.at(1) == ".." && !(state.get_pwd().empty())) 
      state.get_pwd().pop_back();
   else state.get_pwd().push_back(words.at(1));
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   try{
      if (words.size() > 1)
         exit_status::set (stoi(words.at(1)));
   }catch (invalid_argument& e) {
       exit_status::set(127);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ((words.size() == 1) || (words.at(1) == ".")){
      fn_pwd(state, words);
      state.get_cwd()->get_contents()->print();
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if ((words.size() == 1) || (words.at(1) == ".")){
      fn_pwd(state, words);
      state.get_cwd()->get_contents()->print();
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() < 2) return;
   wordvec data = wordvec(words.begin()+1, words.end());
   state.get_cwd()->get_contents()->mkfile(words.at(1))->
      get_contents()->writefile(data);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() < 2) return;
   state.get_cwd()->get_contents()->mkdir(words.at(1));
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.set_prompt(words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string pwdstring = "";
   if (state.get_pwd().empty()) pwdstring = "/";
   for(unsigned int i = 0; i < state.get_pwd().size(); ++i) 
      pwdstring += "/" + state.get_pwd().at(i);
   cout << pwdstring << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for (unsigned int i = 1; i < words.size(); ++i)
      state.get_cwd()->get_contents()->remove(words.at(i));
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}


