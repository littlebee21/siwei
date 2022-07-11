// bmcweb日志开发-获取想要内容的日志-填充数据
// 当前其实内部的路径还是有问题的


static int fillBMCJournalLogEntryHaveContentJson(const std::string& bmcJournalLogEntryID,
                                      sd_journal* journal,
                                      nlohmann::json& bmcJournalLogEntryJson
                                      const std::string& content)
{
    // Get the Log Entry contents
    int ret = 0;

    std::string message;
    std::string_view syslogID;

    //获取 syslog 标识符
    //输出无法读取 syslog ，或者说就没有
    ret = getJournalMetadata(journal, "SYSLOG_IDENTIFIER", syslogID);
    if (ret < 0)
    {
        BMCWEB_LOG_ERROR << "Failed to read SYSLOG_IDENTIFIER field: "
                         << strerror(-ret);
    }
    if (!syslogID.empty())
    {
        message += std::string(syslogID) + ": ";
    }

    //对于日志的内容进行判断
    std::string_view msg;
    ret = getJournalMetadata(journal, "MESSAGE", msg);
    if (ret < 0)
    {
        BMCWEB_LOG_ERROR << "Failed to read MESSAGE field: " << strerror(-ret);
        return 1;
    }
    message += std::string(msg);
    //对信号内部的内容进行判单
    if(message.find(content)==string::npos){
        return 1;
    }


    // Get the severity from the PRIORITY field
    // 就是优先级字段
    // Jul 07 02:31:48 palmetto bmcweb[233]: (2022-07-07 02:31:47) 
    // [DEBUG "log_services.hpp":3457] DBUS response error generic:113
    long int severity = 8; // Default to an invalid priority
    ret = getJournalMetadata(journal, "PRIORITY", 10, severity);
    if (ret < 0)
    {
        BMCWEB_LOG_ERROR << "Failed to read PRIORITY field: " << strerror(-ret);
    }

    // Get the Created time from the timestamp
    std::string entryTimeStr;
    if (!getEntryTimestamp(journal, entryTimeStr))
    {
        return 1;
    }

    // Fill in the log entry with the gathered data
    bmcJournalLogEntryJson = {
        {"@odata.type", "#LogEntry.v1_8_0.LogEntry"},
        {"@odata.id", "/redfish/v1/Managers/bmc/LogServices/Journal/Entries/" +
                          bmcJournalLogEntryID},
        {"Name", "BMC Journal Entry"},
        {"Id", bmcJournalLogEntryID},
        {"Message", std::move(message)},
        {"EntryType", "Oem"},
        {"Severity", severity <= 2   ? "Critical"
                     : severity <= 4 ? "Warning"
                                     : "OK"},
        {"OemRecordFormat", "BMC Journal Entry"},
        {"Created", std::move(entryTimeStr)}};
    return 0;
}