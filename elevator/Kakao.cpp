//
//  Kakao.cpp
//  elevator
//
//  Created by 조기현 on 19/09/2019.
//  Copyright © 2019 none. All rights reserved.
//

#include "Kakao.hpp"

namespace
{
    size_t callback(const char* in,size_t size,size_t num,std::string* out)
    {
        const size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}

namespace Kakao{
    
    //get post 해더 포함 여부
    std::string start(std::string user,std::string problem,int cnt){
        std::string url = _url+__func__+"/"+user+"/"+problem+"/"+std::to_string(cnt);
        CURL* curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        long httpCode(0);
        std::unique_ptr<std::string> httpData(new std::string());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
        
        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        
        assert(httpCode==200);
        
        rapidjson::Document d;
        d.Parse((*httpData.get()).c_str());
        
        return d["token"].GetString();
    }
    
    void oncalls(std::string& token, rapidjson::Document& d){
        std::string url =_url+__func__;
        std::string header = "X-Auth-Token: "+token;
        
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, header.c_str());
        
        CURL* curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        long httpCode(0);
        std::unique_ptr<std::string> httpData(new std::string());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
        
        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        
        assert(httpCode==200);
        
        d.Parse((*httpData.get()).c_str());
    }
    
    void action(std::string& token,const char* json){
        std::string url = _url+__func__;
        std::string header = "X-Auth-Token: "+token;
        
        CURL* curl = curl_easy_init();
        
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, header.c_str());
        //    chunk = curl_slist_append(chunk, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        //    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        long httpCode(0);
        
        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        
        assert(httpCode==200);
    }
};
