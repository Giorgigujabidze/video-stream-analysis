//
// Created by gio on 11/22/24.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <string>

void programSetup();

void getHelp(const std::string &name);

int startConfigMaker(const std::string &filename);

void filePutContents(const std::string &filename, const std::string &content, bool append);


#endif //HELPERS_HPP
