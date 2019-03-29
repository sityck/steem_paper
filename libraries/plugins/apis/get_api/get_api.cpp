#include <steem/plugins/get_api/get_api.hpp>
#include <steem/plugins/get_api/get_api_plugin.hpp>

#include <steem/plugins/follow/follow_objects.hpp>
#include <steem/plugins/account_history_api/account_history_api_plugin.hpp>
#include <steem/plugins/account_history_api/account_history_api.hpp>

#include <steem/utilities/git_revision.hpp>

#include <steem/chain/util/reward.hpp>
#include <steem/chain/util/uint256.hpp>
#include <steem/protocol/types.hpp>
#include <steem/protocol/types_fwd.hpp>

#include <fc/git_revision.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/thread/future.hpp>
#include <boost/thread/lock_guard.hpp>

#define CHECK_ARG_SIZE( s ) \
   FC_ASSERT( args.size() == s, "Expected #s argument(s), was ${n}", ("n", args.size()) );

namespace steem{
namespace plugins{
namespace get{

namespace detail{
    class get_api_impl {
        public:

        get_api_impl() : _db( appbase::app().get_plugin< steem::plugins::chain::chain_plugin >().db() ) {}
        ~get_api_impl() {}

        chain::database& _db;
        std::shared_ptr< account_history::account_history_api >           _account_history_api;

        DECLARE_API_IMPL(
            (get_count)
            (get_blog)
            (get_message)
        )

        get_sum_return get_sum(const get_sum_args &args) const {
            get_sum_return final{0};
            for(auto num : args.nums)
            {
                final.sum += num;
            }
            return final;
        }

    };

DEFINE_API_IMPL( get_api_impl, get_count )
{
   get_count_return result;
   auto itr = _db.find< follow::follow_count_object, follow::by_account >( args.account );

   if( itr != nullptr )
    { 
      uint32_t totle = itr->factor_count;
      result = get_count_return{ itr->account, itr->follower_count, itr->following_count, totle};
    }
   else
      result.account = args.account;

   return result;
}

DEFINE_API_IMPL( get_api_impl, get_blog )
{
   FC_ASSERT( args.limit <= 500, "Cannot retrieve more than 500 blog entries at a time." );

   auto entry_id = args.start_entry_id == 0 ? ~0 : args.start_entry_id;

   get_blog_return result;
   result.blog.reserve( args.limit );

   const auto& blog_idx = _db.get_index< follow::blog_index >().indices().get< follow::by_blog >();
   auto itr = blog_idx.lower_bound( boost::make_tuple( args.account, entry_id ) );

   while( itr != blog_idx.end() && itr->account == args.account && result.blog.size() < args.limit )
   {
      const auto& comment = _db.get( itr->comment );
      comment_blog_entry entry;
      entry.comment = database_api::api_comment_object( comment, _db );
      entry.blog = args.account;
      entry.reblog_on = itr->reblogged_on;
      entry.entry_id = itr->blog_feed_id;

      result.blog.push_back( entry );
      ++itr;
   }

   return result;
}

DEFINE_API_IMPL( get_api_impl, get_message )
{

   return _db.with_read_lock( [&]()
   {
        const auto& idx = _db.get_index< chain::account_history_index, chain::by_account >();
        auto itr = idx.lower_bound( boost::make_tuple( args.account, 100 ) );
        uint32_t n = 0;

        get_message_return result;
        while( true )
        {
            if( itr == idx.end() )
                break;
            if( itr->account != args.account )
                break;
            if( n >= 100 )
                break;
            result.messages[ itr->sequence ] = _db.get( itr->op );
            ++itr;
            ++n;
        }

    return result;
   });

}

}

   get_api::get_api() : my(new detail::get_api_impl()) {
       JSON_RPC_REGISTER_API(STEEM_GET_API_PLUGIN_NAME);
   }

   get_api::~get_api() {}

   void get_api_plugin::plugin_initialize(const appbase::variables_map &options) {
       api = std::make_shared<get_api>();
   }

   DEFINE_LOCKLESS_APIS(get_api,(get_sum))

   DEFINE_READ_APIS(get_api,
       (get_count)
       (get_blog)
       (get_message)
   )
}
}
}