//问题
//如何对代码的公共的内容进行调用


//获取 所有的请求日志
inline void requestRoutesBMCJournalLogEntryCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/bmc/LogServices/Journal/Entries/")
        .privileges(redfish::privileges::getLogEntryCollection)
        .methods(boost::beast::http::verb::get)(
            //lambda表达式进行请求
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp) {
        // 请求参数的容器
        query_param::QueryCapabilities capabilities = {
            .canDelegateTop = true,
            .canDelegateSkip = true,
        };
        // 请求参数的代理请求
        query_param::Query delegatedQuery;
        // 如果设定路由通过代理不接受请求
        if (!redfish::setUpRedfishRouteWithDelegation(
                app, req, asyncResp->res, delegatedQuery, capabilities))
        {
            return;
        }

        // 没有静态数据通过子路由的方式进行添加
        // Collections don't include the static data added by SubRoute
        // 因为一个成员有身体
        // because it has a duplicate entry for members
        asyncResp->res.jsonValue["@odata.type"] =
            "#LogEntryCollection.LogEntryCollection";
        asyncResp->res.jsonValue["@odata.id"] =
            "/redfish/v1/Managers/bmc/LogServices/Journal/Entries";
        asyncResp->res.jsonValue["Name"] = "Open BMC Journal Entries";
        asyncResp->res.jsonValue["Description"] =
            "Collection of BMC Journal Entries";
        nlohmann::json& logEntryArray = asyncResp->res.jsonValue["Members"];
        //json变成array的容器
        logEntryArray = nlohmann::json::array();

        
        //根据时间戳创建ID
        // Go through the journal and use the timestamp to create a
        //每个实体一个ID
        // unique ID for each entry
        
        //可以理解这是一个类
        //通过打开的方式形成了一个有实体的内容
        sd_journal* journalTmp = nullptr;
        int ret = sd_journal_open(&journalTmp, SD_JOURNAL_LOCAL_ONLY);
        if (ret < 0)
        {
            BMCWEB_LOG_ERROR << "failed to open journal: " << strerror(-ret);
            messages::internalError(asyncResp->res);
            return;
        }
        //创建返回智能指针类型的 函数 journal, 但是这个函数没有函数体，应该是一个声明，在他处进行了实现，应该就是
        //参数是一个指针对象， 一个journal close的宏函数，可能是用完了就会关闭的功能，使用回调函数的方式
        std::unique_ptr<sd_journal, decltype(&sd_journal_close)> journal(
            journalTmp, sd_journal_close);

        //清空journ
        journalTmp = nullptr;
        uint64_t entryCount = 0;


        // 重置特定的ID，在第一个实体上
        // Reset the unique ID on the first entry
        bool firstEntry = true;
        SD_JOURNAL_FOREACH(journal.get())
        {
            entryCount++;

            // 用skip的分页标识进行分页显示
            // Handle paging using skip (number of entries to skip from
            // the start) and top (number of entries to display)
            if (entryCount <= delegatedQuery.skip ||
                entryCount > delegatedQuery.skip + delegatedQuery.top)
            {
                continue;
            }

            // 获取entry的id
            std::string idStr;
            if (!getUniqueEntryID(journal.get(), idStr, firstEntry))
            {
                continue;
            }

            //将第一个entry设置为false
            if (firstEntry)
            {
                firstEntry = false;
            }

            // 容器的结尾放空的
            logEntryArray.push_back({});
            nlohmann::json& bmcJournalLogEntry = logEntryArray.back();

            // 将json内容 id entry放入到里面进行填充
            if (fillBMCJournalLogEntryJson(idStr, journal.get(),
                                           bmcJournalLogEntry) != 0)
            {
                messages::internalError(asyncResp->res);
                return;
            }
        }

        // 填写 entrycount的数目
        asyncResp->res.jsonValue["Members@odata.count"] = entryCount;
        //根据 数目决定最后跳转的 连接的链接
        if (delegatedQuery.skip + delegatedQuery.top < entryCount)
        {
            asyncResp->res.jsonValue["Members@odata.nextLink"] =
                "/redfish/v1/Managers/bmc/LogServices/Journal/Entries?$skip=" +
                std::to_string(delegatedQuery.skip + delegatedQuery.top);
        }
        });
}



json array 数组使用的 demo  https://json.nlohmann.me/api/basic_json/array/#notes