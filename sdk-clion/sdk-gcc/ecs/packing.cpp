//
// Created by txink on 18-3-23.
//

#include "packing.h"
#include <cstring>

double score1,score2;
double score_one[3];
int serverType;
//get Ai
std::vector<double> getA(std::map<int, int> predict_data, std::map<int,Vm> vm_info)
{
    std::vector<double> A(2,0);
    int count = 0;
    for(auto &t: predict_data)
    {
        A[0] += (vm_info[t.first].core * t.second);
        A[1] += (vm_info[t.first].mem * t.second);
        count += t.second;
    }
    A[0] = A[0] * 1.0 / count;
    A[1] = A[1] * 1.0 / count;
    return A;
}

std::vector<std::map<int,int>> packing(std::map<int,Vm> vm_info, Server server, std::map<int, int> predict_data,std::vector<Server> &allocate_result){
    std::vector<std::map<int,int>>result_record_1;
    std::vector<std::map<int,int>>result_record_2;

    result_record_1 = packing(vm_info, server, predict_data, 1, allocate_result);
    result_record_2 = packing(vm_info, server, predict_data, 2, allocate_result);

    if(score1 >= score2){
        return result_record_2;
    }else{
        return  result_record_1;
    }
}




//根据当前的server参数和vm数量进行背包，返回分配方案，资源利用率
std::map<int,int> onepacking(std::map<int, Vm> vm_info, Server server, std::map<int,int> predict_data){
//    //归一化系数
//    std::vector<double> paramA = getA(predict_data,vm_info);

    //用来保存虚拟机的剩余数量，反向对应，比如tmp_vm_num[18]对应vm1，tmp_vm_num[1]对应vm18
    std::vector<int> tmp_vm_num(19,0);
    bool is_vm_empty;
    for(int i=1; i<=18; i++){
        std::map<int, int>::iterator iter;
        iter = predict_data.find(19-i);
        if(iter == predict_data.end()){
            tmp_vm_num[i] = 0;
        }else {
            tmp_vm_num[i] = iter->second;
        }
    }
    std::map<int ,int >::iterator current_flavor = predict_data.begin();
    std::map<int,int> new_record;
    std::vector<std::vector <int> > dp (server.core+1, std::vector<int>(server.mem+1,0));
    std::vector<std::vector<std::vector<int> > > used(19, std::vector<std::vector<int> >(server.core+1, std::vector<int>(server.mem+1,0)));


    //一次二维多重背包循环,pos表示前pos个物品
    for(int pos = 1; pos <= 18; pos++){
        //获取当前虚拟机的CPU和MEM限制，同时当前虚拟机id为current_flavor->first
        std::map<int, Vm>::iterator current_flavor_info;
        current_flavor_info =  vm_info.find(19-pos);
        int core_need = current_flavor_info->second.core;
        int mem_need = current_flavor_info->second.mem;
        int item_value= core_need + mem_need;//物品价值;

        int item_num = tmp_vm_num[pos];//可用的物品数量

        //void MultiplePack(int C, int D, int U, int V, int W, int M);
        //C表示物品费用1，D物品费用2，U背包费用1容量，V背包费用2容量，W物品价值，M物品数量
        MultiplePack(dp, used, core_need, mem_need, server.core, server.mem, item_value, item_num, pos);
    }
    std::vector<int> choose_vm_num;
    choose_vm_num = get_path(used, vm_info, server.core, server.mem);
    //处理数据，tmp_vm_num数据更新，同时对predict_data数据更新
    for(int i=1; i<=18; i++){
        if(choose_vm_num[19 - i] != 0){
            new_record[i] = choose_vm_num[19 - i];
        }
    }
    score1 = get_score_one(server,new_record,vm_info);
    return new_record;
}

//一次背包，从不同服务器类型中选出最好一个
std::map<int,int> onepacking(std::map<int, Vm> vm_info, std::map<int, Server> servers, std::map<int,int> predict_data) {

    std::map<int,int> record1;
    std::map<int,int> record;
    double max = 0;
    int index = -1;
    for(int i=0; i<3; i++){
        record1 = onepacking(vm_info,servers[i],predict_data);
        if(score1 > max){
            max = score1;
            index = i;
            record = record1;
        }
    }
    //printf("choose score:%f   serverType:%d\n",max,index);
    if(index == 0){
        serverType = 0;
        return record;
    }else if(index == 1){
        serverType = 1;
        return  record;
    } else if(index == 2){
        serverType = 2;
        return  record;
    }
}


std::vector<std::map<int,int>> packing_ad(std::map<int,Vm> vm_info, std::map<int, Server> servers, std::map<int, int> predict_data, std::vector<Server> &allocate_result){
    std::map<int, int> predict_data_tmp = predict_data;
    //首先确定优化目标
    //保存最终结果的map，vector中的ID对应编号为多少的分配结果
    std::vector<std::map<int,int>>result_record;
    // 初始化服务器节点
    int server_number = 0;
//    //归一化系数
//    std::vector<double> paramA = getA(predict_data,vm_info);

    //用来保存虚拟机的剩余数量，反向对应，比如tmp_vm_num[18]对应vm1，tmp_vm_num[1]对应vm18

    bool is_vm_empty;


    is_vm_empty = check_vmnum_empty(predict_data);

    while(!is_vm_empty)
    {

        //清除预测数据中value为0的项
        for(int i = 18; i>0; i--) {
            std::map<int, int>::iterator iter;
            iter = predict_data.find(i);
            if(iter == predict_data.end()){
                continue;
            }else if(iter->second == 0) {
                predict_data.erase(iter);
            }
        }


        std::map<int,int> new_record = onepacking(vm_info, servers, predict_data);

        bool isRepeat = true;
        //处理数据，tmp_vm_num数据更新，同时对predict_data数据更新
        while(isRepeat){
            for(int i=1; i<=18; i++) {
                std::map<int, int>::iterator iter;
                iter = new_record.find(i);
                if (iter == new_record.end()) {
                    continue;
                } else if (iter->second != 0) {
                    std::map<int, int>::iterator itert;
                    itert = predict_data.find(i);
                    itert->second -= iter->second;
                    if(itert->second < iter->second)
                        isRepeat = false;
                }
            }
            //首先初始化一个节点
            Server sv = servers[serverType];
            Server new_server = allocate_oneserver(sv.type, sv.core, sv.mem);
            allocate_result.push_back(new_server);
            server_number++;
            result_record.push_back(new_record);
        }
        is_vm_empty = check_vmnum_empty(predict_data);
    }
    get_scores_p(predict_data_tmp, allocate_result, vm_info);
    return result_record;
}


std::vector<std::map<int,int>> packing(std::map<int,Vm> vm_info, Server server, std::map<int, int> predict_data, int value_type,std::vector<Server> &allocate_result) {
    std::map<int, int> predict_data_tmp = predict_data;
    //首先确定优化目标
    //保存最终结果的map，vector中的ID对应编号为多少的分配结果
    std::vector<std::map<int,int>>result_record;
    // 初始化服务器节点
    int server_number = 0;
//    //归一化系数
//    std::vector<double> paramA = getA(predict_data,vm_info);

    //用来保存虚拟机的剩余数量，反向对应，比如tmp_vm_num[18]对应vm1，tmp_vm_num[1]对应vm18

    bool is_vm_empty;
    is_vm_empty = check_vmnum_empty(predict_data);

    while(!is_vm_empty)
    {

        //清除预测数据中value为0的项
        for(int i = 18; i>0; i--) {
            std::map<int, int>::iterator iter;
            iter = predict_data.find(i);
            if(iter == predict_data.end()){
                continue;
            }else if(iter->second == 0) {
                predict_data.erase(iter);
            }
        }


        std::map<int,int> new_record = onepacking(vm_info, server, predict_data);
        bool isRepeat = true;
        //处理数据，tmp_vm_num数据更新，同时对predict_data数据更新
        while(isRepeat){
            for(int i=1; i<=18; i++) {
                std::map<int, int>::iterator iter;
                iter = new_record.find(i);
                if (iter == new_record.end()) {
                    continue;
                } else if (iter->second != 0) {
                    std::map<int, int>::iterator itert;
                    itert = predict_data.find(i);
                    itert->second -= iter->second;
                    if(itert->second < iter->second)
                        isRepeat = false;
                }
            }
            //首先初始化一个节点
            Server sv = server;
            Server new_server = allocate_oneserver(sv.type, sv.core, sv.mem);
            allocate_result.push_back(new_server);
            server_number++;
            result_record.push_back(new_record);
        }


        is_vm_empty = check_vmnum_empty(predict_data);
    }
    get_scores_p(predict_data_tmp, allocate_result, vm_info);
    return result_record;
}



bool check_vmnum_empty(std::map<int, int> predict_data){
    bool isempty = true;
    for(int i=1; i<=18; i++) {
        std::map<int, int>::iterator iter;
        iter = predict_data.find(i);
        if (iter == predict_data.end()) {
            continue;
        } else{
           if(iter->second != 0){
               isempty = false;
           }
        }
    }
    return isempty;
}

std::vector<int> get_path(std::vector<std::vector<std::vector<int> > > &used, std::map<int,Vm> vminfo, int U, int V)
{
    //18种物品选择了哪些
    std::vector<int> choose_num(19, 0);
    std::map<int, Vm>::iterator current_flavor_info;
    for(int i=1; i<=18; i++){
        choose_num[i] = used[i][U][V];
    }
    return choose_num;
}

void CompletePack(std::vector<std::vector <int> > &dp, std::vector<std::vector<std::vector<int> > > &used, int C, int D, int U, int V, int W, int pos)
{

    for(int u=C; u<=U; u++)
    {
        for(int v=D; v<=V; v++){
            if(C <= u && D <= v){
                //第i个物品可放入
                if(dp[u][v] < dp[u-C][v-D]+W){
                    dp[u][v] =  dp[u-C][v-D]+W;
                    for(int k=1; k<=18; k++){
                        used[k][u][v] = used[k][u-C][v-D];
                    }
                    used[pos][u][v] += 1;
                }
            }
        }

    }

}

void ZeroOnePack(std::vector<std::vector <int> > &dp, std::vector<std::vector<std::vector<int> > > &used, int C, int D, int U, int V, int W, int M, int pos)
{
    //  std::cout<<"ZeroPack("<<C*M<<", "<<D*M<<", "<<W*M<<", "<<pos<<")"<<std::endl;
    for(int u=U; u>=C*M; u--)
    {
        for(int v=V; v>=D*M; v--){
            if(C*M <= u && D*M <= v) {
                //第i个物品可放入
                if (dp[u][v] < dp[u - C * M][v - D * M] + W * M) {
                    dp[u][v] = dp[u - C * M][v - D * M] + W * M;
                    for(int k=1; k<=18; k++){
                        used[k][u][v] = used[k][u-C*M][v-D*M];
                    }
                    used[pos][u][v] += M;
                }
            }
        }

    }


}

void MultiplePack(std::vector<std::vector <int> > &dp, std::vector<std::vector<std::vector<int> > > &used, int C, int D, int U, int V, int W, int M, int pos)
{
    //物品容量大于背包容量，直接返回
    if(C > U || D >  V)
        return;

    //物品数量为0，直接返回
    if(M <= 0)
        return;


    if(C*M >= U && D*M >= V){
        CompletePack(dp, used, C, D, U, V, W, pos);
        return;
    }

    for(int k=1; k<=M; k*=2)
    {
        ZeroOnePack(dp, used, C, D, U, V, W, k, pos);
        M = M - k;
    }
    if(M){
        ZeroOnePack(dp, used, C, D, U, V, W, M, pos);
    }
}




Server allocate_oneserver(int type, int core, int mem)
{
    Server new_allocate;
    new_allocate.type = type;
    new_allocate.core = core;
    new_allocate.mem = mem;
    return new_allocate;
}


double get_score_one(Server server, std::map<int,int> record, std::map<int, Vm> vm_info){
    int total_need_cpu = 0;
    int total_need_mem = 0;
    for (int i = 1; i<=18; i++)
    {
        std::map<int ,int >::iterator iter;
        iter = record.find(i);
        if (iter == record.end())
        {
            continue;
        }
        else
        {
            int target_need_cpu = 0;
            int target_need_mem = 0;
            std::map<int, Vm>::iterator current_flavor_info;
            current_flavor_info =  vm_info.find(i);;
            target_need_cpu = current_flavor_info->second.core;
            target_need_mem = current_flavor_info->second.mem;

            total_need_cpu += iter->second * target_need_cpu;
            total_need_mem += iter->second * target_need_mem;

        }
    }
    double percent = ((total_need_cpu+0.0)/server.core + (total_need_mem+0.0)/server.mem)*0.5;
    //printf("allocate score = %f\n", percent);
    return percent;

}

double get_scores_p(std::map<int, int>predict_data, std::vector<Server> servers, std::map<int, Vm> vm_info)
{
    int total_allocate_cpu = 0;
    int total_allocate_mem = 0;
    for(int i=0; i<servers.size(); i++){
        total_allocate_cpu += servers[i].core;
        total_allocate_mem += servers[i].mem;
    }

    int total_need_cpu = 0;
    int total_need_mem = 0;
    for (int i = 1; i<=18; i++)
    {
        std::map<int ,int >::iterator iter;
        iter = predict_data.find(i);
        if (iter == predict_data.end())
        {
            continue;
        }
        else
        {
            int target_need_cpu = 0;
            int target_need_mem = 0;
            std::map<int, Vm>::iterator current_flavor_info;
            current_flavor_info =  vm_info.find(i);;

            target_need_cpu = current_flavor_info->second.core;
            target_need_mem = current_flavor_info->second.mem;

            total_need_cpu += iter->second * target_need_cpu;
            total_need_mem += iter->second * target_need_mem;
        }
    }
    double percent = ((total_need_cpu+0.0)/total_allocate_cpu + (total_need_mem+0.0)/total_allocate_mem)*0.5;
    printf("Final allocate score = %f\n", percent);
    return percent;
}
