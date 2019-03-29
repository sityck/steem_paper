#pragma once
#include <steem/plugins/json_rpc/utility.hpp>
#include <steem/plugins/follow/follow_objects.hpp>
#include <steem/plugins/database_api/database_api_objects.hpp>
#include <steem/plugins/account_history_api/account_history_api.hpp>

#include <steem/plugins/condenser_api/condenser_api_legacy_objects.hpp>

#include <steem/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>
#include <fc/api.hpp>

namespace steem{
namespace plugins{
namespace get{


namespace detail{
    class get_api_impl;
}

struct api_operation_object
{
   api_operation_object() {}

   template< typename T >
   api_operation_object( const T& op_obj ) :
      trx_id( op_obj.trx_id ),
      block( op_obj.block ),
      trx_in_block( op_obj.trx_in_block ),
      virtual_op( op_obj.virtual_op ),
      timestamp( op_obj.timestamp )
   {
      op = fc::raw::unpack_from_buffer< steem::protocol::operation >( op_obj.serialized_op );
   }

   steem::protocol::transaction_id_type trx_id;
   uint32_t                               block = 0;
   uint32_t                               trx_in_block = 0;
   uint32_t                               op_in_trx = 0;
   uint32_t                               virtual_op = 0;
   fc::time_point_sec                     timestamp;
   steem::protocol::operation             op;

   bool operator<( const api_operation_object& obj ) const
   {
      return std::tie( block, trx_in_block, op_in_trx, virtual_op ) < std::tie( obj.block, obj.trx_in_block, obj.op_in_trx, obj.virtual_op );
   }
};

struct get_sum_return
{
    int64_t sum;
};

struct get_sum_args
{
    std::vector<int64_t> nums;
};

struct get_count_return
{
    steem::protocol::account_name_type account;
    uint32_t          follower_count = 0;
    uint32_t          following_count = 0;
    uint32_t          totle = 0;
};

struct get_count_args
{
    steem::protocol::account_name_type account;
};

struct get_feed_entries_args
{
   steem::protocol::account_name_type account;
   uint32_t          start_entry_id = 0;
   uint32_t          limit = 500;
};

struct comment_blog_entry
{
   database_api::api_comment_object comment;
   string                           blog;
   time_point_sec                   reblog_on;
   uint32_t                         entry_id = 0;
};

typedef get_feed_entries_args get_blog_args;

struct get_blog_return
{
   vector< comment_blog_entry > blog;
};

struct get_message_args
{
    steem::protocol::account_name_type account;
};

struct get_message_return
{
   std::map< uint32_t, api_operation_object > messages;
};

class get_api
{
private:
    std::unique_ptr<detail::get_api_impl> my;

public:
    get_api();
    ~get_api();

    DECLARE_API(
        (get_sum)
        (get_count)
        (get_blog)
        (get_message)
        )
};



}
}
}

FC_REFLECT(steem::plugins::get::get_sum_args,(nums))
FC_REFLECT(steem::plugins::get::get_sum_return,(sum))
//sum

FC_REFLECT( steem::plugins::get::get_count_args,
            (account) );
FC_REFLECT( steem::plugins::get::get_count_return,
            (account)(follower_count)(following_count)(totle) );
//get_follow

FC_REFLECT( steem::plugins::get::get_feed_entries_args,
            (account)(start_entry_id)(limit) );
FC_REFLECT( steem::plugins::get::comment_blog_entry,
            (comment)(blog)(reblog_on)(entry_id) );
FC_REFLECT( steem::plugins::get::get_blog_return,
            (blog) );
//get_blog

FC_REFLECT( steem::plugins::get::get_message_args,
            (account) );
FC_REFLECT( steem::plugins::get::api_operation_object,
             (trx_id)(block)(trx_in_block)(op_in_trx)(virtual_op)(timestamp)(op) )
FC_REFLECT( steem::plugins::get::get_message_return,
            (messages) );