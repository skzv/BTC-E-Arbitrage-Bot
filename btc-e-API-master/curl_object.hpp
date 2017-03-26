#ifndef CURL_OBJECT_HPP
#define CURL_OBJECT_HPP

#include <curl/curl.h>
#include "uncopyable.hpp"

class curl_object: public uncopyable {
    static CURL* curl_handle;
    curl_object() {}
public:
    ~curl_object() {
        curl_easy_cleanup(curl_handle);
        curl_handle = nullptr;
    }

    static CURL* get_instance() {
        if (curl_handle == nullptr) {
            curl_handle = curl_easy_init();
        }
        return curl_handle;
    }
};
CURL* curl_object::curl_handle = nullptr;

#endif // CURL_OBJECT_HPP
