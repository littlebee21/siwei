bmcweb日志开发-entry日志json格式


static int fillBMCJournalLogEntryJson(const std::string& bmcJournalLogEntryID,  //输入的参数id
                                      sd_journal* journal,                      //对应日志的条目  
                                      nlohmann::json& bmcJournalLogEntryJson)   //输入的  json格式的内容

// 参考：
    //看json文件的缺少的内容
    // Fill in the log entry with the gathered data
    bmcJournalLogEntryJson = {
        {"@odata.type", "#LogEntry.v1_8_0.LogEntry"},
        {"@odata.id", "/redfish/v1/Managers/bmc/LogServices/Journal/Entries/" +
                          bmcJournalLogEntryID},    //缺少id
        {"Name", "BMC Journal Entry"},
        {"Id", bmcJournalLogEntryID},    //也是id
        {"Message", std::move(message)},    //message内容
        {"EntryType", "Oem"},
        {"Severity", severity <= 2   ? "Critical"  //对应内容的评级
                     : severity <= 4 ? "Warning"
                                     : "OK"},
        {"OemRecordFormat", "BMC Journal Entry"},  
        {"Created", std::move(entryTimeStr)}};   //创建的时间戳