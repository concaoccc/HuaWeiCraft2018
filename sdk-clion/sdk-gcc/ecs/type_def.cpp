//
// Created by ruiy on 18-3-18.
//

#include "type_def.h"
//bool operator< (const Allocat_server &server1, const Allocat_server &server2)
//{
//    if (server1.target == 0)
//    {
//        if( server1.core > server2.core)
//        {
//            return true;
//        }
//        else if (server1.core == server2.core)
//        {
//            if(server1.id > server2.id)
//            {
//                return true;
//            }
//        }
//        else
//        {
//            return false;
//        }
//    }
//    else
//    {
//        if(server1.mem > server2.mem)
//        {
//            return true;
//        }
//        else if (server1.mem == server2.mem)
//        {
//            if(server1.id > server2.id)
//            {
//                return true;
//            }
//        }
//        else
//        {
//            return false;
//        }
//    }
//}

bool operator< (const Node_index &index1, const Node_index &index2)
{
    if(index1.value < index2.value)
    {
        return true;
    }
    else
    {
        return false;
    }

}