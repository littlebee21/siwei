bmcweb日志开发-获取全部的日志-填充数据


static int fillBMCJournalLogEntryJson(const std::string& bmcJournalLogEntryID,
                                      sd_journal* journal,
                                      nlohmann::json& bmcJournalLogEntryJson)
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

    std::string_view msg;
    ret = getJournalMetadata(journal, "MESSAGE", msg);
    if (ret < 0)
    {
        BMCWEB_LOG_ERROR << "Failed to read MESSAGE field: " << strerror(-ret);
        return 1;
    }
    message += std::string(msg);

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

//获取元数据后对内容进行判断：
//设计成get接口 还是post接口

//前端界面应该合成一个，上层应该是少的，使用单个的变量的形式，然后在底层根据内容来进行不同的过滤和实现，前端是一个数组的形式，
//也可以是一个手动输入的形式
//应该添加一个总的页面进行定位
//post的字符串的内容应该
//使用post接口，然后根据请求的内容对日志的内容进行过滤刷新


//在后端进行效率的提升：
//数据库又没有直接提供进行检索的工具：再去确认，目前没有，先用遍历进行查找





//获取元数据
//四个参数的内容：
//还是通过四个参数的方法
inline static int getJournalMetadata(sd_journal* journal,   //日志对象
                                     const std::string_view& field,   //字段
                                     const int& base,      //控制转化成进制的基数
                                     long int& contents)   //内容元数据的返回内容
{
    int ret = 0;
    std::string_view metadata;
    // Get the metadata from the requested field of the journal entry
    ret = getJournalMetadata(journal, field, metadata);
    if (ret < 0)
    {
        return ret;
    }
    contents = strtol(metadata.data(), nullptr, base);   //其实是严重的级别，级别输出，将base
    return ret;
}



//什么是field数据内容：
//获取日志的内部的数据内容：
//三个参数的内容
inline static int getJournalMetadata(sd_journal* journal,
                                     const std::string_view& field,
                                     std::string_view& contents)
{
    const char* data = nullptr;
    size_t length = 0;
    int ret = 0;
    //我没能解决的数据转化的问题
    // Get the metadata from the requested field of the journal entry
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const void** dataVoid = reinterpret_cast<const void**>(&data);

    ret = sd_journal_get_data(journal, field.data(), dataVoid, &length);
    if (ret < 0)
    {
        return ret;
    }
    contents = std::string_view(data, length);
    // Only use the content after the "=" character.
    contents.remove_prefix(std::min(contents.find('=') + 1, contents.size()));
    return ret;
}

//sd_journal_get_data 对应关键关键字的使用
// sd_journal_get_data()从当前日记帐分录中获取与特定字段关联的数据对象。
//s它有四个参数：
int sd_journal_get_data(	sd_journal *j,
 	const char *field,
 	const void **data,
 	size_t *length);
1，日志上下文对象，
2，一个带有要请求的字段名称的字符串，
3，加上一对指向指针/大小变量的指针，
4,数据对象及其大小应存储在其中。


// 字段标准的说明
字段名称应该是一个条目字段名称. 
众所周知的字段名称在 systemd.journal-fields (7)中列出，
// https://www.freedesktop.org/software/systemd/man/systemd.journal-fields.html#
但可以指定任何字段。


//返回的数据下一个调用可用
返回的数据位于只读内存映射中，
并且仅在下一次调用
sd_journal_get_data(), 
sd_journal_enumerate_data(), 
sd_journal_enumerate_available_data()或读取指针被更改之前有效。
请注意，返回的数据将以字段名称和“="。
另请注意，默认情况下，大于 64K 的数据字段可能会被截断为 64K。此阈值可能会更改并关闭



// Syslog 兼容性字段包含设施（格式为十进制字符串）、标识符字符串（即“标签”）、客户端 PID 和原始数据报中指定的时间戳。（注意，标签通常来自 glibc 的 program_invocation_short_name变量，参见 program_invocation_short_name (3)。）
// 请注意，日志服务不会验证名称不带下划线前缀的任何结构化日志字段的值，这包括任何与系统日志相关的字段，例如这些。因此，提供设施、PID 或日志级别的应用程序应采用正确格式，即格式化为十进制字符串的数字整数。