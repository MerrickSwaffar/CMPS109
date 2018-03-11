// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   root = shared_ptr<inode> (new inode(file_type::DIRECTORY_TYPE));
   root->set_parent(root);
   root->get_contents()->insert(".", root);
   root->get_contents()->insert("..", root);
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

void inode_state::set_prompt(const wordvec& words){
    string prompt = "";
    for(unsigned int i = 1; i < words.size(); ++i) 
       prompt += words[i] + " ";
    this->prompt_ = prompt;
}

const inode_ptr& inode_state::get_cwd(){
    return cwd;
}

const inode_ptr& inode_state::get_root(){
    return root;
}

vector<string>& inode_state::get_pwd(){
    return pwd;
}

void inode_state::set_cwd(inode_ptr dir){
    cwd = dir; 
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
void inode::set_parent(inode_ptr p) {
    parent = p;
}

const base_file_ptr& inode::get_contents(){
    return contents;
} 


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   if (data.size() == 1) return size;
   string datastring = "";
   for (unsigned int i = 1; i < data.size(); ++i) 
      datastring += (data.at(i) + " ");
   size = datastring.size()-1;
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::print() {
    for(unsigned int i = 1; i < data.size(); ++i) 
       cout << data.at(i) << " ";
    cout << endl;
}

void plain_file::insert(string name, inode_ptr p) {
    DEBUGF ('i', name);
    DEBUGF ('i', p);
    throw file_error ("is a plain file");
}

inode_ptr plain_file::find_node(const string& name) {
    DEBUGF ('i', name);
    throw file_error ("is a plain file");
}

bool plain_file::is_directory(){
    return false;
}

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr newdir = 
      shared_ptr<inode> (new inode(file_type::DIRECTORY_TYPE));
   this->insert(dirname, newdir);
   newdir->get_contents()->insert(".", newdir);
   newdir->get_contents()->insert("..", this->find_node("."));
   return newdir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr newfile = 
      shared_ptr<inode> (new inode(file_type::PLAIN_TYPE));
   this->insert(filename, newfile);
   return newfile;
}

void directory::print() {
    for(auto i = dirents.cbegin(); i != dirents.cend(); ++i) {
        cout << "   " << i->second->get_inode_nr() << "   ";
        cout << i->second->get_contents()->size() << " " << i->first;
        if (i->second->get_contents()->is_directory() && 
           !((i->first == ".")||(i->first == ".."))) cout << "/";
        cout << endl;
    }
}

void directory::insert(string name, inode_ptr p) {
    dirents.emplace(name, p);
}

inode_ptr directory::find_node(const string& name) {
    return dirents.at(name);
}

bool directory::is_directory(){
    return true;
}

