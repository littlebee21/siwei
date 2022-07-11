
//被 获取全部的日志调用
//获取全部的实体内容， 被调用的函数内容
// 本质还是从 
inline void requestRoutesBMCJournalLogEntry(App& app)
{
    //这回的路径是带有字符串的通配的内容
    //直接的请求是没有办法访问的
    //内容被重定向，所有的内容都是
    BMCWEB_ROUTE(app,

                 "/redfish/v1/Managers/bmc/LogServices/Journal/Entries/<str>/")
        .privileges(redfish::privileges::getLogEntry)
        .methods(boost::beast::http::verb::get)(
            [&app](const crow::Request& req,
                   const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& entryID) {
        //所有的内容都是在这里创建实体，然后进行创建
        if (!redfish::setUpRedfishRoute(app, req, asyncResp->res))
        {
            return;
        }
        // 将时间戳 转换为ID 去找到一个实体
        // Convert the unique ID back to a timestamp to find the entry
        uint64_t ts = 0;
        uint64_t index = 0;
        if (!getTimestampFromID(asyncResp, entryID, ts, index))
        {
            return;
        }

        // 打开对应的日志记录
        sd_journal* journalTmp = nullptr;
        int ret = sd_journal_open(&journalTmp, SD_JOURNAL_LOCAL_ONLY);
        if (ret < 0)
        {
            BMCWEB_LOG_ERROR << "failed to open journal: " << strerror(-ret);
            messages::internalError(asyncResp->res);
            return;
        }

        // 创建journal的声明，等待之后的使用
        std::unique_ptr<sd_journal, decltype(&sd_journal_close)> journal(
            journalTmp, sd_journal_close);
        journalTmp = nullptr;

        // 获取日志当中的实时的时间用作当前的ID
        // Go to the timestamp in the log and move to the entry at the
        // index tracking the unique ID
        std::string idStr;
        bool firstEntry = true;
        ret = sd_journal_seek_realtime_usec(journal.get(), ts);
        if (ret < 0)
        {
            BMCWEB_LOG_ERROR << "failed to seek to an entry in journal"
                             << strerror(-ret);
            messages::internalError(asyncResp->res);
            return;
        }

        //按照前面获取的index 进行遍历
        //找到我们想要的代码实体
        for (uint64_t i = 0; i <= index; i++)
        {
            sd_journal_next(journal.get());
            //获取所有的 ID
            if (!getUniqueEntryID(journal.get(), idStr, firstEntry))
            {
                messages::internalError(asyncResp->res);
                return;
            }
            if (firstEntry)
            {
                firstEntry = false;
            }
        }
        // 确定 entry ID 是我们请求的内容
        // Confirm that the entry ID matches what was requested
        if (idStr != entryID)
        {
            messages::resourceMissingAtURI(asyncResp->res, req.urlView);
            return;
        }

        //将对应的实体内容
        if (fillBMCJournalLogEntryJson(entryID, journal.get(),
                                       asyncResp->res.jsonValue) != 0)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        });
}