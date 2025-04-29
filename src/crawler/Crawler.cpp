#include "Crawler.hpp"

// CURL写回调
// 什么是写入数据的回调函数？
// 写入数据的回调函数是一个函数指针，用于处理从服务器接收到的数据。
// 它接受四个参数：
// 1. void* contents：指向接收到的数据的指针。
// 2. size_t size：每个数据块的大小。
// 3. size_t nmemb：数据块的数量。
// 4. std::string* output：用于存储接收到的数据的字符串。   
// 谁调用了这个函数？
// 在 CURL 库中，当 CURL 库从服务器接收数据时，会调用这个回调函数。
// 这个回调函数会将接收到的数据存储到 output 字符串中。
// 为什么要实现这个回调函数？
// 因为 CURL 库需要一个回调函数来处理从服务器接收到的数据。
static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// 获取天气信息的函数
std::string fetch_weather(const std::string& city) {
    // curl 是什么？
    // curl 是一个命令行工具，用于传输数据，支持多种协议，包括 HTTP、HTTPS、FTP 等。
    // 它可以通过命令行或脚本调用，方便地进行网络操作。
    // 在 C/C++ 中，curl 库可以用于在程序中实现网络请求。   
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        // curl_easy_escape 是什么？
        // curl_easy_escape 是 CURL 库中的一个函数，用于将字符串转换为 URL 编码。
        // 它接受一个 CURL 句柄、一个字符串和一个长度，返回一个 URL 编码的字符串。
        // 为什么要使用 URL 编码？
        // 因为 URL 编码可以将特殊字符转换为 URL 编码，方便在 URL 中传输。
        // 例如，空格转换为 %20，等号转换为 %3D，等等。
        char* encoded = curl_easy_escape(curl, city.c_str(), city.length());
        // ostringstream 是什么？
        // ostringstream 是 C++ 标准库中的一个类，用于将数据转换为字符串。
        // 它继承自 ostream 类，可以像流一样使用。
        // 使用 ostringstream 可以将各种数据类型转换为字符串，方便进行字符串操作。  
        std::ostringstream url;
        url << "https://www.mxnzp.com/api/weather/current/" << std::string(encoded)  << "?app_id=ngeorpqtkeijibqu&app_secret=ZWJlWXFzc21KNjYzVG9iakdBT3cydz09"; 
        curl_free(encoded);
        // curl_easy_setopt 是什么？
        // curl_easy_setopt 是 CURL 库中的一个函数，用于设置 CURL 选项。
        // 它接受一个 CURL 句柄、一个选项常量和一个值，用于配置网络请求的各种参数。 
        // option 是什么？  
        // option 是 CURL 库中的一个选项常量，用于配置网络请求的各种参数。
        // 例如，CURLOPT_URL 选项用于设置请求的 URL，
        // CURLOPT_WRITEFUNCTION 选项用于设置写入数据的回调函数，
        // CURLOPT_WRITEDATA 选项用于设置写入数据的缓冲区。
        
        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);  // 防止卡死
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        // curl_easy_perform 是什么？
        // curl_easy_perform 是 CURL 库中的一个函数，用于执行网络请求。
        // 它接受一个 CURL 句柄，并返回一个 CURLcode 类型的值，表示请求的结果。
        // 如果请求成功，返回 CURLE_OK，否则返回其他错误码。    
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "Error fetching weather: " + std::string(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    } else {
        response = "Failed to init curl";
    }

    return response;
}

