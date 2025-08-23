#include "Krpcconfig.h"
#include "memory" //智能指针

/**
 * 处理配置文件
 */

//将配置文件加载到config_map
void KrpcConfig::LoadConfigFile(const char *config_file)
{
    //unique_ptr 管理FILE类型变量, 自定义使用fclose()销毁对象, decltype是类型推导
    std::unique_ptr<FILE, decltype(&fclose)> pf(fopen(config_file, "r"), &fclose);
    if (pf == nullptr) exit(EXIT_FAILURE);
    //获取每一行内容的指针, 其中智能指针的get()返回管理对象的原始指针(并非智能指针)
    char buf[1024];
    while (fgets(buf, 1024, pf.get()) != nullptr) 
    {
        //读取每一行, 转换为string
        std::string read_buf(buf);
        Trim(read_buf);

        //跳过注释和空行
        if (read_buf[0] == '#' || read_buf.empty()) continue;
        
        //取前半部分key
        int index = read_buf.find('='); //index is position of '='
        if (index = -1) continue;
        std::string key = read_buf.substr(0, index); //substr(start, len)
        Trim(key);

        //去后半部分value
        int endindex = read_buf.find('\n', index); // start from position index
        std::string value = read_buf.substr(index + 1, endindex - index -1);
        Trim(value);

        //写入unordered_map
        config_map.insert({key, value});
    }
    
}

//在config_map中根据key查找value
std::string KrpcConfig::Load(const std::string &key)
{
    std::unordered_map<std::string, std::string>::iterator it = config_map.find(key);
    if (it == config_map.end()) return "";
    return it->second;
}

//去除字符串前后空格
void KrpcConfig::Trim(std::string &read_buf)
{
    int index = read_buf.find_first_not_of(' ');
    if (index != -1){
        read_buf = read_buf.substr(index, read_buf.size() - index);
    }

    // 去掉字符串后面的空格
    index = read_buf.find_last_not_of(' ');
    if (index != -1) {  // 如果找到非空格字符
        read_buf = read_buf.substr(0, index + 1);  // 截取字符串
    }
}
