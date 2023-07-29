#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

#include "spdlog.hpp"
#include "spdlog2.hpp"
#include "spdlog_shared.hpp"
#include "spdlog_static.hpp"

// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/synchronous_factory.h>

#include <mutex>
#include <string>

namespace spdlog {

// callbacks type
typedef std::function<void(const details::log_msg &msg)> custom_log_callback;

namespace sinks {
/*
 * Trivial callback sink, gets a callback function and calls it on each log
 */
template<typename Mutex>
class callback_sink final : public base_sink<Mutex>
{
public:
    explicit callback_sink(custom_log_callback callback)
        : callback_{std::move(callback)}
    {}

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        callback_(msg);
    }
    void flush_() override{};

private:
    custom_log_callback callback_;
};

using callback_sink_mt = callback_sink<std::mutex>;
using callback_sink_st = callback_sink<details::null_mutex>;

} // namespace sinks



//
// factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_mt(const std::string &logger_name, const custom_log_callback &callback)
{
    return Factory::template create<sinks::callback_sink_mt>(logger_name, callback);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> callback_logger_st(const std::string &logger_name, const custom_log_callback &callback)
{
    return Factory::template create<sinks::callback_sink_st>(logger_name, callback);
}

} // namespace spdlog



TEST_CASE("Test logging across compilation units")
{
  std::vector<std::string> messages;

  spdlog::set_level(spdlog::level::trace);
  auto logger = spdlog::callback_logger_mt("custom_callback_logger", [&messages](const spdlog::details::log_msg &msg) {
      messages.emplace_back(msg.payload.data(), msg.payload.size());
    });

  spdlog::set_default_logger(logger);
  spdlog::set_pattern("%v");

  test_logger();
  test_logger_2();
  test_logger_shared_lib();
  test_logger_static_lib();

  REQUIRE(messages.size() == 4);
  CHECK(messages[0] == "test_logger()");
  CHECK(messages[1] == "test_logger_2()");
  CHECK(messages[2] == "test_logger_shared_lib()");
  CHECK(messages[3] == "test_logger_static_lib()");

}


