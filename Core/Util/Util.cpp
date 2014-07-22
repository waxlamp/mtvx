// Copyright 2011 A.N.M. Imroz Choudhury
//
// Util.cpp

// MTV headers.
#include <Core/Util/Util.h>

// System headers.
#include <sstream>

// std::string MTV::makeTitle(const std::vector<std::string>& titles, unsigned rank){
//   if(rank < titles.size()){
//     return makeTitle(titles[rank], rank);
//   }
//   else{
//     return makeTitle("", rank);
//   }
// }

std::string MTV::makeTitle(const std::string& title, unsigned rank){
  if(title.length() > 0){
    return title;
  }

  std::stringstream ss;
  ss << "Region " << rank;
  return ss.str();
}
