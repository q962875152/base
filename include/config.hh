#ifndef CONFIG_HH_
#define CONFIG_HH_

#include <bits/types/struct_sched_param.h>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include "../include/rapidjson/document.h"
#include <fstream>
#include <iostream>
#include "rapidjson/prettywriter.h"  // for stringify JSON
#include "rapidjson/ostreamwrapper.h"
#include <string_view>
#include <thread>
#include <sys/inotify.h>
#include <unistd.h>
#include "../MWUtils/singleton.h"
#include <string_view>
#include <sys/param.h>
#include "../src/log.hh"

const std::string config_file_path = "/home/chengjun/cpp/code/radioservice.json";

using namespace rapidjson;

class ConfigParse {
public:
    virtual int getIntValue(const std::string& key) = 0;
    virtual bool setValue(const std::string& key, const int value) = 0;  // 可不可以用模板替代
    virtual bool configIsExist(const std::string& key) = 0;
    virtual bool saveToFile() = 0;
    virtual ~ConfigParse() = default;
};

class JsonConfigParse : public ConfigParse {
public:
    JsonConfigParse(std::string_view filePath) {
        file_path_ = std::move(filePath);
        std::ifstream ifile(file_path_);
        if (!ifile.is_open()) {
            std::cout << "open radioservice.json failed" << std::endl;
            std::cerr << "errno: " << strerror(errno) << std::endl;
            return;
            exit(EXIT_FAILURE);
        }

        json = std::string((std::istreambuf_iterator<char>(ifile)), (std::istreambuf_iterator<char>()));

        parser_.Parse(json.c_str());
        if (parser_.HasParseError() || !parser_.IsObject()) {
            std::cout << "Parse radioservice.json failed" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    int getIntValue(const std::string& key) override {
        auto itr = parser_.FindMember(key.c_str());
        if (itr != parser_.MemberEnd()) {
            return itr->value.GetInt();
        }

        return -1;
    }

    bool setValue(const std::string& key, const int value) override {
        Value& object = parser_[key.c_str()];
        object.SetInt(value);
        return 0;
    }

    bool configIsExist(const std::string& key) override {
        auto itr = parser_.FindMember(key.c_str());
        if (itr == parser_.MemberEnd()) {
            return false;
        }

        return true;
    }

    bool saveToFile() override {
        std::ofstream ofile(file_path_);
        if (!ofile.is_open()) {
            std::cout << "open radioservice.json failed" << std::endl;
            std::cerr << "errno: " << strerror(errno) << std::endl;
            return false;
        }

        rapidjson::OStreamWrapper osw(ofile);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        parser_.Accept(writer);
        return true;
    }

    ~JsonConfigParse() {
        saveToFile();
    }

    /************************************TEST***********************************/
    void print_test() {
        for (auto pvalue = parser_.MemberBegin(); pvalue != parser_.MemberEnd(); ++pvalue) {
            std::cout << pvalue->value.GetInt() << std::endl;
        }
    }

private:
    Document parser_;
    std::string json;
    std::string file_path_;
};

class jsonConfigFactore {
public:
    std::unique_ptr<JsonConfigParse> createJsonConfigParse(std::string_view file_path) {
        return std::make_unique<JsonConfigParse>(file_path);
    }
};

class MementoConfig {
public:
    MementoConfig(int _rssi, int _snr, int _usn) :
        rssi_(_rssi),
        snr_(_snr),
        usn_(_usn) {
    }

    void save() {

    }
private:
    int rssi_ = 0;
    int snr_ = 0;
    int usn_ = 0;
};

class Config : public Singleton<Config> {
    friend Singleton<Config>;
public:
    void init(std::string_view file_path) {
        auto pfac = std::make_unique<jsonConfigFactore>();
        p_config_parse_ = pfac->createJsonConfigParse(file_path);
        file_path_ = file_path;
        inotify_fd = inotify_init();
        if (inotify_fd == -1) {
            std::cout << "create inotify is failed, errno: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        iwatch_fd = inotify_add_watch(inotify_fd, file_path_.c_str(), IN_MODIFY);
    }

    int getRssi() {
        return p_config_parse_->getIntValue("rssi");
    }

    void setRssi(int value) {
        p_config_parse_->setValue("rssi", value);
    }

    int getSnr() {
        return p_config_parse_->getIntValue("snr");
    }

    void setSnr(int value) {
        p_config_parse_->setValue("snr", value);
    }

    int getUsn() {
        return p_config_parse_->getIntValue("usn");
    }

    void setUsn(int value) {
        p_config_parse_->setValue("usn", value);
    }

    void saveToFile() {
        p_config_parse_->saveToFile();
    }

    void update_config() {
        auto f = [this] {
            while (true) {
                uint8_t buff[sizeof(inotify_event) + NAME_MAX + 1];
                std::memset(buff, 0, sizeof(buff));
                int numRead = read(inotify_fd, &buff, sizeof(buff));
                if (numRead == -1) {
                    LOG("read inotify_fd is failed, errno:", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                for (uint8_t* p = buff; p < buff + numRead;) {
                    inotify_event* event = reinterpret_cast<inotify_event*>(buff);
                    if (event->wd == iwatch_fd) {
                        // 重新读取配置文件，并启动更新流程
                        LOG("enter update is success!");
                        inotify_test(event);
                        break;
                    }
                    p += sizeof(inotify_event) + event->len;
                }
            }
        };

        handler = std::thread(f);
    }
    ~Config() {
        if (handler.joinable()) {
            handler.join();
        }
    }

    /*******************************TEST************************************/
    void print_test() {
        LOG(getRssi());
        LOG(getSnr());
        LOG(getUsn());
    }

    void inotify_test(inotify_event* event) {
        LOG(event->wd, event->mask, event->cookie, event->len, event->name);
    }
    /***********************************************************************/
 
 private:
    Config() :
        p_config_parse_(nullptr),
        inotify_fd(-1),
        iwatch_fd((-1)) {
    }
    std::unique_ptr<ConfigParse> p_config_parse_;
    std::string file_path_;
    int inotify_fd;
    int iwatch_fd;
    std::thread handler;
};

#define ConfigProcess Singleton<Config>::Instance()

#endif