#pragma once
#include <steem/plugins/json_rpc/utility.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>
#include <fc/api.hpp>

namespace steem{
namespace plugins{
namespace demo{

namespace detail{
    class demo_api_impl;
}

struct get_sum_return
{
    int64_t sum;
};

struct get_sum_args
{
    std::vector<int64_t> nums;
};


class demo_api
{
private:
    std::unique_ptr<detail::demo_api_impl> my;

public:
    demo_api();
    ~demo_api();

    DECLARE_API(
        (get_sum)
        )
};



}
}
}

FC_REFLECT(steem::plugins::demo::get_sum_args,(nums))
FC_REFLECT(steem::plugins::demo::get_sum_return,(sum))