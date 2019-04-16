#pragma once
#include <steem/plugins/json_rpc/utility.hpp>
#include <steem/protocol/types.hpp>

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

struct get_reviewer_return
{
    string   reviewer1;
    string   reviewer2;
    string   reviewer3;
    string   reviewer4;
    string   reviewer5;
    string   reviewer6;
};

struct get_reviewer_args
{
    uint32_t   arg;
};


class demo_api
{
private:
    std::unique_ptr<detail::demo_api_impl> my;

public:
    demo_api();
    ~demo_api();

    DECLARE_API(
        (get_reviewer)
        )
};



}
}
}

FC_REFLECT(steem::plugins::demo::get_reviewer_args,(arg))
FC_REFLECT(steem::plugins::demo::get_reviewer_return,(reviewer1)(reviewer2)(reviewer3)(reviewer4)(reviewer5)(reviewer6))