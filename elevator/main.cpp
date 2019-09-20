#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "Kakao.hpp"
#include <thread>
#include <chrono>


using namespace std;
using namespace rapidjson;

const int MAX_PERSON_CNT = 8;
struct PassengerInfo{
    int start;
    int end;
    bool entered;
};

enum statusValue{STOPPED,OPENED,UPWARD,DOWNWARD};
enum commandValue{STOP,OPEN,ENTER,EXIT,CLOSE,UP,DOWN,YET};

map<string,statusValue> STATUS;
map<commandValue,string> COMMAND;
vector<string> PREV_COMMAND;

map<int,PassengerInfo> PASSENGER;

string user = "whrlgus";
string problem = "2";
int ele_cnt = 4;

struct Command {
    int elevator_id;
    string command;
    vector<int> call_ids;
    
    Command(int _elevator_id, string _command, vector<int> _call_ids){
        elevator_id = _elevator_id;
        command = _command;
        call_ids = _call_ids;
    }
};

bool shouldExit(Value& passenger,int floor){
    for(Value& v:passenger.GetArray()){
        int call_id = v["id"].GetInt();
        if(floor==PASSENGER[call_id].end) return true;
    }
    return false;
}
bool shouldEnter(Value& calls,int floor,bool dir, bool anyDir){
    for(Value& call:calls.GetArray()){
        int call_id = call["id"].GetInt();
        if(PASSENGER[call_id].entered)continue;
        if(PASSENGER[call_id].start!=floor)continue;
        if(anyDir) return true;
        if(dir==PASSENGER[call_id].start<PASSENGER[call_id].end) return true;
    }
    return false;
}
bool shouldMove(Value& calls,int floor,bool& dir){
    for(Value& call:calls.GetArray()){
        int call_id = call["id"].GetInt();
        if(PASSENGER[call_id].entered)continue;
        dir = floor<PASSENGER[call_id].start;
        return true;
    }
    return false;
}
void doExit(Value& passenger,int floor, vector<int>& call_ids, string& command){
    for(Value& v:passenger.GetArray()){
        int call_id = v["id"].GetInt();
        if(floor==PASSENGER[call_id].end) call_ids.push_back(call_id);
    }
    command = COMMAND[EXIT];
}
void doEnter(Value& passenger,Value& calls,int floor, vector<int>& call_ids, string& command, int passCnt){
    bool usable=false;
    bool dir;
    if(passCnt!=0){
        int id = passenger.Begin()->GetObject()["id"].GetInt();
        dir = PASSENGER[id].start<PASSENGER[id].end;
        usable=true;
    }
    for(Value& v:calls.GetArray()){
        if(passCnt+call_ids.size()==MAX_PERSON_CNT)break;
        
        int call_id = v["id"].GetInt();
        if(PASSENGER[call_id].entered)continue;
        if(floor!=PASSENGER[call_id].start)continue;
        
        bool tmp = PASSENGER[call_id].start<PASSENGER[call_id].end;
        if(!usable){
            dir = tmp;
            usable=true;
            call_ids.push_back(call_id);
            PASSENGER[call_id].entered=true;

            continue;
        }
        if(dir!=tmp)continue;
        call_ids.push_back(call_id);
        PASSENGER[call_id].entered=true;



    }
    command = COMMAND[ENTER];
}


void createDocument(vector<Command>& Commands,Document& d){
    Document::AllocatorType& a = d.GetAllocator();
    Value v(kArrayType);
    
    for(auto c:Commands){
        Value obj(kObjectType);
        obj.AddMember("elevator_id", c.elevator_id, a);
        Value tmp(kStringType);
        tmp.SetString(c.command.c_str(), c.command.length(),a);
        obj.AddMember("command", tmp, a);
        if(c.call_ids.size()!=0){
            Value ids(kArrayType);
            for(auto id:c.call_ids) ids.PushBack(id, a);
            obj.AddMember("call_ids", ids, a);
        }
        v.PushBack(obj, a);
    }
    d.AddMember("commands", v, a);
}

void makePlan(Value& calls, Value& elevators, StringBuffer& jsonBuf){
    Document d(kObjectType);
    vector<Command> Commands;
    
    for(auto& ele:elevators.GetArray()){

        int elevator_id = ele["id"].GetInt();
        string command=COMMAND[YET];
        vector<int> call_ids;
        call_ids.clear();
        
        
        int floor = ele["floor"].GetInt();
        int passengerCnt = ele["passengers"].GetArray().Size();
        string prevCommand = PREV_COMMAND[elevator_id];
        
        switch (STATUS[ele["status"].GetString()]) {
            case STOPPED:{
                if(prevCommand==COMMAND[CLOSE]){// 방금 완료하거나 멈춰있었음, 올라가거나 내려가야함
                    if(passengerCnt==0){
                        bool dir;
                        if(shouldMove(calls, floor, dir))
                            command = dir?COMMAND[UP]:COMMAND[DOWN];
                        else command = COMMAND[STOP];
                    }else{
                        int id = ele["passengers"].Begin()->GetObject()["id"].GetInt();
                        command = PASSENGER[id].start<PASSENGER[id].end?COMMAND[UP]:COMMAND[DOWN];
                    }
                }else{ // 멈춰 있는 상태였음,
                    if(passengerCnt==0){ //탈 사람 혹은 콜 없음
                        if(shouldEnter(calls, floor, true, true))
                            command = COMMAND[OPEN];
                        else {
                            bool dir;
                            if(shouldMove(calls, floor, dir))
                                command = dir?COMMAND[UP]:COMMAND[DOWN];
                            else command = COMMAND[STOP];
                        }
                    }else{ // 내리거나 탈 사람 있음
                        command = COMMAND[OPEN];
                    }
                }
                
                
                break;
            }
            case OPENED:{
                // exit or enter or close
                if(passengerCnt==0){// 태울라고
                    if(shouldEnter(calls, floor, true,true))
                        doEnter(ele["passengers"], calls, floor, call_ids, command, passengerCnt);
                    else command = COMMAND[CLOSE];
                }else{// 내리거나 태울라고
                    
                    int id = ele["passengers"].Begin()->GetObject()["id"].GetInt();
                    bool dir = PASSENGER[id].start<PASSENGER[id].end;
                    
                    if(shouldExit(ele["passengers"], floor))
                        doExit(ele["passengers"], floor, call_ids,command);
                    else if(shouldEnter(calls, floor, dir,false)){
                        if(passengerCnt==MAX_PERSON_CNT) command = COMMAND[CLOSE];
                        else doEnter(ele["passengers"], calls, floor, call_ids, command, passengerCnt);
                    }else command = COMMAND[CLOSE];
                    
                }
                
                break;
            }
            default:{
                if(floor==25||floor==1){
                    command = COMMAND[STOP];
                    break;
                }
                // 승객을 태운 경우 안태운 경우
                if(passengerCnt==0){
                    if(shouldEnter(ele["passengers"],floor,true,true)){
                        command = COMMAND[STOP];
                    }else{
                        bool dir;
                        if(shouldMove(calls, floor, dir)){
                            command = dir?COMMAND[UP]:COMMAND[DOWN];
                            if(command!=prevCommand)command = COMMAND[STOP];
                        }
                        else command = COMMAND[STOP];
                    }
                }else{
                    //stop - 타거나 내릴 승객이 존재
                    //현재 층에서 내리거나 탈 사람 체크
                    bool dir = prevCommand==COMMAND[UP]?true:false;
                    if(shouldExit(ele["passengers"],floor)||shouldEnter(ele["passengers"],floor,dir,false))
                        command = COMMAND[STOP];
                    else command = prevCommand;
                }
                break;
            }
            
        }
        
        PREV_COMMAND[elevator_id]=command;
        Commands.push_back(Command(elevator_id,command,call_ids));
        call_ids.clear();
    }
    
    createDocument(Commands, d);
    Writer<StringBuffer> writer(jsonBuf);
    d.Accept(writer);
}
void func(){
    
    string token = Kakao::start(user,problem,ele_cnt);
    
    //반복문
    while(true){
        Document d;
        Kakao::oncalls(token,d);
        
        if(d["is_end"].GetBool()) return;
        
        StringBuffer b;
        Writer<StringBuffer> w(b);
        d.Accept(w);
        cout<<b.GetString()<<endl;
        
        int tt=0;
        
        cout<<d["calls"].GetArray().Size()<<"\n\n";
        // 매번 oncall 때마다, 승객 정보 갱신
        for(auto& call:d["calls"].GetArray()){
            int id =call["id"].GetInt();
            
            if(PASSENGER.count(id)>0)continue;
            
            int start = call["start"].GetInt();
            int end = call["end"].GetInt();
            PassengerInfo tmp = {start,end,false};
            PASSENGER[id] = tmp;
        }
        
        StringBuffer jsonBuf;
        
        makePlan(d["calls"], d["elevators"], jsonBuf);
        cout<<jsonBuf.GetString()<<endl;
        Kakao::action(token, jsonBuf.GetString());
    }
}




int main(){
    STATUS["STOPPED"]=STOPPED;
    STATUS["OPENED"]=OPENED;
    STATUS["UPWARD"]=UPWARD;
    STATUS["DOWNWARD"]=DOWNWARD;

    COMMAND[STOP]="STOP";
    COMMAND[OPEN]="OPEN";
    COMMAND[ENTER]="ENTER";
    COMMAND[EXIT]="EXIT";
    COMMAND[CLOSE]="CLOSE";
    COMMAND[UP]="UP";
    COMMAND[DOWN]="DOWN";
    
    for(int i=0;i<ele_cnt;++i)
        PREV_COMMAND.push_back(COMMAND[STOP]);
    
    
    func();
    
    
    return 0;
}
