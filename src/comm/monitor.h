/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */
#ifndef _MONITOR_H_
#define _MONITOR_H_
#include "commdefs.h"

namespace bm
{
#pragma pack(1)
    typedef struct tagIntfStat
    {
        int8_t staFlag;
        int64_t execKey;
        int64_t staIndex;
        double minTime;
        double maxTime;
        double p90Time;
        double p99Time;
        double p999Time;
        double totalTime;
        size_t costTimes[10];
        size_t succCount;
        size_t failCount;
        size_t totalCount;
        size_t totalSendBytes;
        size_t totalRecvBytes;
        int8_t retCount[1024];
        int8_t endFlag;

        tagIntfStat()
        {
            clear();
        }

        void clear()
        {
            memset(this, 0, sizeof(struct tagIntfStat));
            staFlag = STA_FLAG;
            endFlag = END_FLAG;
            minTime = 1000.00;
        }

        inline tagIntfStat &operator+=(const tagIntfStat &src)
        {
            for (size_t i = 0; i < 10; i++)
            {
                costTimes[i] += src.costTimes[i];
            }

            int32_t totalCnt = totalCount + src.totalCount;
            if ((totalCount + src.totalCount) > 0)
            {
                p90Time = (p90Time * totalCount + src.p90Time * src.totalCount) / totalCnt;
                p99Time = (p99Time * totalCount + src.p99Time * src.totalCount) / totalCnt;
                p999Time = (p999Time * totalCount + src.p999Time * src.totalCount) / totalCnt;
            }

            totalTime += src.totalTime;
            totalCount += src.totalCount;
            failCount += src.failCount;
            succCount += src.succCount;
            totalSendBytes += src.totalSendBytes;
            totalRecvBytes += src.totalRecvBytes;
            maxTime = std::max<double>(src.maxTime, maxTime);
            minTime = std::min<double>(src.minTime, minTime);
            return *this;
        }

    } IntfStat;

    typedef struct tagStatCache
    {
        volatile uint32_t max_cnt;
        volatile uint32_t head;
        volatile uint32_t tail;
        IntfStat item_list[0];
    } StatCache;
#pragma pack()
    class Monitor
    {
    public:
        Monitor() : _workmode(MODEL_FIXED){};
        virtual ~Monitor(){};

        /**
         * @brief  实例化
         *
         */
        static Monitor *getInstance()
        {
            static Monitor *m = NULL;
            if (m == NULL)
            {
                m = new Monitor;
            }
            return m;
        }

        /**
         * @brief  初始化
         *
         */
        int initialize(int shm_key = 0x19453959, int shm_size = 1024 * 1024);

        /**
         * @brief  清除数据
         *
         */
        void clear();

        /**
         * @brief  接口上报
         *
         * @param ret_code   返回码
         * @param cost_time  消耗的时间
         * @param snd_time   上报的时间点
         */
        void report(int ret_code);
        void report(int ret_code, int cost_time);
        void reportSend(int64_t snd_time, int snd_bytes);
        void reportRecv(int64_t rcv_time, int rcv_bytes);

        /**
         * @brief  状态同步
         *
         * @param cur_time      当前时间
         * @param time_out      超时时间
         *
         * @return bool  是否出现大量超时
         */
        bool syncStat(int64_t cur_time, int64_t time_out = 0);

        /**
         * @brief  获取上报的数据
         *
         * @param item_list  返回的数据
         *
         * @return bool
         */
        bool fetch(vector<IntfStat> &item_list);

        /**
         * @brief  计算千分比
         *
         * @param percent  分子数
         */
        double calcPercent(size_t percent);

        /**
         * @brief  计算下一次请求时间
         *
         * @param intval     时间间隔
         * @param cons       连接数
         *
         * @return size_t 时间间隔
         */
        size_t calcInterval(size_t intval, size_t cons);

    private:
        /**
         * @brief  自动适配速率
         *
         * @param cur_time      当前时间
         * @param time_out    超时时间
         */
        void adapteSpeed(int64_t cur_time, int64_t time_out);

    private:
        int _workmode;
        char *_data_base;
        size_t _set_speed;
        IntfStat _cache_stat;
        vector<int> _queue_cost;
        map<int, int> _count_ret;
    };
}; // namespace bm
#endif
