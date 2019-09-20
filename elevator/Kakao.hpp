//
//  Kakao.hpp
//  elevator
//
//  Created by 조기현 on 19/09/2019.
//  Copyright © 2019 none. All rights reserved.
//

#ifndef Kakao_hpp
#define Kakao_hpp
#include <curl/curl.h>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
const std::string _url="http://localhost:8000/";

namespace Kakao{
    std::string start(std::string user,std::string problem,int cnt);
    void oncalls(std::string& token, rapidjson::Document& d);
    void action(std::string& token,const char* json);
};

#endif /* Kakao_hpp */
